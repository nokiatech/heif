/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled
 * by Nokia Corporation and/or its subsidiaries. All rights are reserved.
 */

#include "heifreader_c.h"


#include <cstring>
#include <mutex>
#include <new>
#include <unordered_set>

#include "heifcommondatatypes.h"
#include "heiferrorinfo.h"
#include "heifreader.h"
#include "heifreaderdatatypes.h"
#include "heifstreaminterface.h"
#include <cstdint>
#include <stddef.h>

namespace
{
    struct ReaderHandle
    {
        ::HEIF::Reader* reader = nullptr;

        // Lazy caches; *Valid flags track population, freed with the handle.
        ::HEIF::Array<::HEIF::ImageId> masterImages;
        bool                           masterImagesValid = false;

        ::HEIF::Array<::HEIF::FourCC>  compatibleBrands;
        bool                           compatibleBrandsValid = false;

        // Single-entry cache for getItemListByType, keyed on the FourCC.
        ::HEIF::FourCC                 cachedItemListType{};
        ::HEIF::Array<::HEIF::ImageId> cachedItemList;
        bool                           cachedItemListValid = false;

        // Keyed on (from_image_id, reference_type).
        ::HEIF::ImageId                cachedRefFromId{0};
        ::HEIF::FourCC                 cachedRefFromType{};
        ::HEIF::Array<::HEIF::ImageId> cachedRefFromList;
        bool                           cachedRefFromValid = false;

        // Keyed on (to_image_id, reference_type).
        ::HEIF::ImageId                cachedRefToId{0};
        ::HEIF::FourCC                 cachedRefToType{};
        ::HEIF::Array<::HEIF::ImageId> cachedRefToList;
        bool                           cachedRefToValid = false;

        // Single-entry cache for getItemProperties(image_id), keyed on image_id.
        ::HEIF::ImageId                       cachedPropListImageId{0};
        ::HEIF::Array<::HEIF::ItemPropertyInfo> cachedPropList;
        bool                                  cachedPropListValid = false;

        // Keyed on PropertyId -- a single image may carry multiple pixi
        // descriptors under the tile-with-different-bit-depth pattern.
        ::HEIF::PropertyId             cachedPixiPropertyId{0};
        ::HEIF::PixelInformation       cachedPixi;
        bool                           cachedPixiValid = false;

        ::HEIF::PropertyId             cachedOinfPropertyId{0};
        ::HEIF::OperatingPointsInfo    cachedOinf;
        bool                           cachedOinfValid = false;

        ::HEIF::FileInformation        cachedFileInfo;
        bool                           cachedFileInfoValid = false;

        // Owned by the handle iff initialize_stream was used; kept alive
        // across the reader's lifetime per the StreamInterface contract.
        struct StreamCallbackBridge*   streamBridge = nullptr;
    };

    // Process-wide live-handle registry: every entry point checks
    // membership before dereferencing so a stale pointer past destroy
    // (Python finally double-close, Rust Drop racing a manual destroy,
    // Java GC after close()) returns UNINITIALIZED instead of SIGSEGV.
    // The mutex is held only for the O(1) lookup; once liveness is
    // confirmed the lock is released before calling into the C++ surface
    // so there is no contention on parallel reader workloads. This does
    // not make a single handle thread-safe -- only protects against
    // use-after-destroy from a different execution slot than the one
    // that destroyed it.
    std::mutex& liveHandlesMutex()
    {
        // Function-local static -- initialised on first use, never
        // destroyed, so a late-running entry point that races shutdown
        // still sees a live mutex.
        static std::mutex* m = new std::mutex();
        return *m;
    }

    std::unordered_set<ReaderHandle*>& liveHandles()
    {
        // Leak-on-purpose so shutdown-time deallocations do not dispatch
        // on a dead set.
        static auto* s = new std::unordered_set<ReaderHandle*>();
        return *s;
    }

    // True iff h is currently registered as live.
    bool isLiveHandle(ReaderHandle* h) noexcept
    {
        if (h == nullptr)
        {
            return false;
        }
        try
        {
            std::lock_guard<std::mutex> lk(liveHandlesMutex());
            return liveHandles().count(h) != 0U;
        }
        catch (...)
        {
            // lock_guard / unordered_set::count are noexcept in practice;
            // on a hypothetical alloc-throwing rebuild, fail closed rather
            // than report liveness.
            return false;
        }
    }

    // ReaderHandle is heif_reader_t at the binary level (incomplete-type
    // forward declaration on the C side); the cast through the C handle
    // type is the documented form.
    inline ReaderHandle* asHandle(heif_reader_t* h) noexcept
    {
        return reinterpret_cast<ReaderHandle*>(h);
    }
    inline const ReaderHandle* asHandle(const heif_reader_t* h) noexcept
    {
        return reinterpret_cast<const ReaderHandle*>(h);
    }
    inline heif_reader_t* asOpaque(ReaderHandle* h) noexcept
    {
        return reinterpret_cast<heif_reader_t*>(h);
    }

    // Centralised so a future widening of ErrorCode beyond uint8_t
    // needs only one cast site changed.
    inline heif_error_code_t toCode(::HEIF::ErrorCode c) noexcept
    {
        return static_cast<heif_error_code_t>(c);
    }

    // HEIF::FourCC::value is char[5] and already NUL-terminated by every
    // constructor, so a 5-byte memcpy is the entire contract.
    inline void copyFourCC(const ::HEIF::FourCC& src, char dst[5]) noexcept
    {
        std::memcpy(dst, src.value, 5U);
    }

    // Map ItemPropertyType to a 4-byte FourCC the binding can dispatch on.
    // FourCC keeps the C surface numerically stable across a possible
    // underlying-type widening of the C++ enum.
    //
    // "auxC" preserves the box-header capitalisation; INVALID maps to
    // "\0\0\0\0" and RAW to "raw " so a binding can detect both sentinels
    // unambiguously.
    inline void propertyTypeToFourCC(::HEIF::ItemPropertyType type, char dst[5]) noexcept
    {
        const char* s = nullptr;
        switch (type)
        {
            case ::HEIF::ItemPropertyType::INVALID: s = "\0\0\0\0"; break;
            case ::HEIF::ItemPropertyType::RAW:     s = "raw ";     break;
            case ::HEIF::ItemPropertyType::ALTT:    s = "altt";     break;
            case ::HEIF::ItemPropertyType::AUXC:    s = "auxC";     break;
            case ::HEIF::ItemPropertyType::AVCC:    s = "avcC";     break;
            case ::HEIF::ItemPropertyType::CLAP:    s = "clap";     break;
            case ::HEIF::ItemPropertyType::COLR:    s = "colr";     break;
            case ::HEIF::ItemPropertyType::CRTT:    s = "crtt";     break;
            case ::HEIF::ItemPropertyType::HVCC:    s = "hvcC";     break;
            case ::HEIF::ItemPropertyType::IMIR:    s = "imir";     break;
            case ::HEIF::ItemPropertyType::IROT:    s = "irot";     break;
            case ::HEIF::ItemPropertyType::ISCL:    s = "iscl";     break;
            case ::HEIF::ItemPropertyType::ISPE:    s = "ispe";     break;
            case ::HEIF::ItemPropertyType::JPGC:    s = "jpgC";     break;
            case ::HEIF::ItemPropertyType::LHVC:    s = "lhvC";     break;
            case ::HEIF::ItemPropertyType::LSEL:    s = "lsel";     break;
            case ::HEIF::ItemPropertyType::MDFT:    s = "mdft";     break;
            case ::HEIF::ItemPropertyType::PASP:    s = "pasp";     break;
            case ::HEIF::ItemPropertyType::PIXI:    s = "pixi";     break;
            case ::HEIF::ItemPropertyType::RLOC:    s = "rloc";     break;
            case ::HEIF::ItemPropertyType::RREF:    s = "rref";     break;
            case ::HEIF::ItemPropertyType::UDES:    s = "udes";     break;
            case ::HEIF::ItemPropertyType::VVCC:    s = "vvcC";     break;
            case ::HEIF::ItemPropertyType::AMVE:    s = "amve";     break;
            case ::HEIF::ItemPropertyType::CLLI:    s = "clli";     break;
            case ::HEIF::ItemPropertyType::CCLV:    s = "cclv";     break;
            case ::HEIF::ItemPropertyType::MDCV:    s = "mdcv";     break;
            case ::HEIF::ItemPropertyType::NDWT:    s = "ndwt";     break;
            case ::HEIF::ItemPropertyType::REVE:    s = "reve";     break;
            case ::HEIF::ItemPropertyType::TOLS:    s = "tols";     break;
            case ::HEIF::ItemPropertyType::OINF:    s = "oinf";     break;
            case ::HEIF::ItemPropertyType::SUBS:    s = "subs";     break;
            // No default: a future enum addition surfaces as INVALID
            // and the binding's dispatch falls through.
        }
        if (s == nullptr)
        {
            std::memset(dst, 0, 5U);
            return;
        }
        std::memcpy(dst, s, 4U);
        dst[4] = '\0';
    }

    // Records a structured ErrorInfo and returns the matching code so
    // a guard can stay one line at the call site:
    //
    //     return recordGuard(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
    //                        "fn_name: NULL handle");
    //
    // Origin is Reader_ItemValidation -- these are caller-side contract
    // violations, not parse failures -- with the per-site detail in the
    // message string.
    inline heif_error_code_t recordGuard(::HEIF::ErrorCode code,
                                         const char*       message,
                                         const char*       file,
                                         std::size_t       line) noexcept
    {
        try
        {
            ::HEIF::ErrorInfo info;
            info.setCode   (code);
            info.setOrigin (::HEIF::ErrorOrigin::Reader_ItemValidation);
            info.setMessage(message);
            info.setFile   (file);
            info.setLine   (static_cast<std::uint32_t>(line));
            info.setValid  (true);
            ::HEIF::setErrorInfo(info);
        }
        catch (...)
        {
            // setErrorInfo allocates (dupOrNull strdup); OOM here must
            // not promote a logical guard to a crash. The slot may be
            // empty in this rare case but the return code still reflects
            // the contract violation.
        }
        return toCode(code);
    }

#define HEIF_C_GUARD_RETURN(code_, message_) \
    return recordGuard((code_), (message_), __FILE__, __LINE__)

    // Populate h->cachedPixi for property pid, parsing on cache miss.
    // No error-info slot writes -- the caller owns guard messaging.
    inline ::HEIF::ErrorCode ensurePixiCached(ReaderHandle* h, ::HEIF::PropertyId pid) noexcept
    {
        if (h->cachedPixiValid && (h->cachedPixiPropertyId == pid))
        {
            return ::HEIF::ErrorCode::OK;
        }
        const auto err = h->reader->getProperty(pid, h->cachedPixi);
        if (err != ::HEIF::ErrorCode::OK)
        {
            h->cachedPixiValid = false;
            return err;
        }
        h->cachedPixiPropertyId = pid;
        h->cachedPixiValid      = true;
        return ::HEIF::ErrorCode::OK;
    }

    // Populate h->cachedOinf for property pid, parsing on cache miss.
    inline ::HEIF::ErrorCode ensureOinfCached(ReaderHandle* h, ::HEIF::PropertyId pid) noexcept
    {
        if (h->cachedOinfValid && (h->cachedOinfPropertyId == pid))
        {
            return ::HEIF::ErrorCode::OK;
        }
        const auto err = h->reader->getProperty(pid, h->cachedOinf);
        if (err != ::HEIF::ErrorCode::OK)
        {
            h->cachedOinfValid = false;
            return err;
        }
        h->cachedOinfPropertyId = pid;
        h->cachedOinfValid      = true;
        return ::HEIF::ErrorCode::OK;
    }

    // Populate h->cachedFileInfo on demand. Safe to call on a freshly-
    // initialised reader (forwards the underlying error if initialize()
    // never ran). No error-info slot writes -- caller owns guard messaging.
    inline ::HEIF::ErrorCode ensureFileInfoCached(ReaderHandle* h) noexcept
    {
        if (h->cachedFileInfoValid)
        {
            return ::HEIF::ErrorCode::OK;
        }
        const auto err = h->reader->getFileInformation(h->cachedFileInfo);
        if (err != ::HEIF::ErrorCode::OK)
        {
            h->cachedFileInfoValid = false;
            return err;
        }
        h->cachedFileInfoValid = true;
        return ::HEIF::ErrorCode::OK;
    }

    // Forward-declared so heif_reader_destroy can invoke the destructor
    // regardless of declaration order; the definition lives near the
    // end of the file alongside the stream-init entry point.
    struct StreamCallbackBridge;
    void destroyStreamBridge(StreamCallbackBridge* b) noexcept;
}  // namespace

extern "C"
{

heif_reader_t* heif_reader_create(void)
{
    // Reader::Create() throws only std::bad_alloc; firewall it and surface
    // as a NULL return so bindings see MemoryError / None instead of an
    // aborted process.
    try
    {
        auto* h = new (std::nothrow) ReaderHandle();
        if (h == nullptr)
        {
            return nullptr;
        }
        h->reader = ::HEIF::Reader::Create();
        if (h->reader == nullptr)
        {
            // Defensive guard -- Create() is documented to throw rather
            // than return nullptr; honour both contracts.
            delete h;
            return nullptr;
        }
        // Register the handle so subsequent entry points can detect
        // use-after-destroy. If registration itself OOMs, treat it as a
        // create failure -- a facade that cannot police its own handles
        // is worse than no facade.
        try
        {
            std::lock_guard<std::mutex> lk(liveHandlesMutex());
            liveHandles().insert(h);
        }
        catch (...)
        {
            ::HEIF::Reader::Destroy(h->reader);
            delete h;
            return nullptr;
        }
        return asOpaque(h);
    }
    catch (...)
    {
        return nullptr;
    }
}

void heif_reader_destroy(heif_reader_t* reader)
{
    if (reader == nullptr)
    {
        return;  // free(3) convention
    }
    auto* h = asHandle(reader);
    // Unregister BEFORE destroying so a racing entry point on another
    // thread sees "no longer live" rather than "live, but mid-destruction".
    // A double-destroy of an already-freed pointer no-ops the erase and
    // bails without re-freeing.
    {
        try
        {
            std::lock_guard<std::mutex> lk(liveHandlesMutex());
            if (liveHandles().erase(h) == 0U)
            {
                // Either a double-destroy or a pointer that never came
                // from heif_reader_create. Either way: do not double-free.
                return;
            }
        }
        catch (...)
        {
            // Cannot consult the registry -> cannot prove ownership;
            // bail rather than risk a double-free.
            return;
        }
    }
    try
    {
        if (h->reader != nullptr)
        {
            ::HEIF::Reader::Destroy(h->reader);
            h->reader = nullptr;
        }
    }
    catch (...)
    {
        // Reader::Destroy is documented noexcept; even if a future
        // refactor breaks that, the exception must not escape into C-land.
    }
    // Tear down the stream-callback shim AFTER Reader::Destroy because
    // the Reader holds a pointer to it.
    if (h->streamBridge != nullptr)
    {
        destroyStreamBridge(h->streamBridge);
        h->streamBridge = nullptr;
    }
    delete h;
}

heif_error_code_t heif_reader_initialize_file(heif_reader_t* reader, const char* path)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_initialize_file: reader handle is NULL");
    }
    if (path == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_initialize_file: path is NULL");
    }
    auto* h = asHandle(reader);
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_initialize_file: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_initialize_file: underlying Reader is NULL (handle not properly created)");
    }
    try
    {
        return toCode(h->reader->initialize(path));
    }
    catch (...)
    {
        // The C++ surface's HEIF_TRY/HEIF_CATCH macros convert every
        // escape into an ErrorCode + ErrorInfo. Any escape past that is
        // a library bug; surface as UNKNOWN_ERROR rather than crashing
        // the embedder.
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_major_brand(const heif_reader_t* reader, char out_brand[5])
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_major_brand: reader handle is NULL");
    }
    if (out_brand == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_major_brand: out_brand is NULL");
    }
    auto* mh = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(mh))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_major_brand: reader handle is not live (use-after-destroy or never created)");
    }
    if (mh->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_major_brand: underlying Reader is NULL");
    }
    try
    {
        ::HEIF::FourCC brand;
        const auto     err = mh->reader->getMajorBrand(brand);
        if (err == ::HEIF::ErrorCode::OK)
        {
            copyFourCC(brand, out_brand);
        }
        return toCode(err);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_master_image_count(const heif_reader_t* reader, uint32_t* out_count)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_master_image_count: reader handle is NULL");
    }
    if (out_count == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_master_image_count: out_count is NULL");
    }
    // The lazy cache is mutable state on a logically-const handle;
    // Reader::getMasterImages is itself const, so casting away const here
    // is safe.
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_master_image_count: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_master_image_count: underlying Reader is NULL");
    }
    try
    {
        if (!h->masterImagesValid)
        {
            const auto err = h->reader->getMasterImages(h->masterImages);
            if (err != ::HEIF::ErrorCode::OK)
            {
                return toCode(err);
            }
            h->masterImagesValid = true;
        }
        *out_count = static_cast<uint32_t>(h->masterImages.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_master_image_at(const heif_reader_t* reader,
                                                  uint32_t            index,
                                                  uint32_t*           out_image_id)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_master_image_at: reader handle is NULL");
    }
    if (out_image_id == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_master_image_at: out_image_id is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_master_image_at: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_master_image_at: underlying Reader is NULL");
    }
    try
    {
        // Populate the cache on first access -- same path as the count
        // accessor -- so a binding can call _at directly without a leading
        // _count if it discovers the count some other way.
        if (!h->masterImagesValid)
        {
            const auto err = h->reader->getMasterImages(h->masterImages);
            if (err != ::HEIF::ErrorCode::OK)
            {
                return toCode(err);
            }
            h->masterImagesValid = true;
        }
        if (index >= h->masterImages.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_master_image_at: index >= master_image_count");
        }
        *out_image_id = h->masterImages.elements[index].get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_width(const heif_reader_t* reader, uint32_t image_id, uint32_t* out_width)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_width: reader handle is NULL");
    }
    if (out_width == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_width: out_width is NULL");
    }
    auto* mh = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(mh))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_width: reader handle is not live (use-after-destroy or never created)");
    }
    if (mh->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_width: underlying Reader is NULL");
    }
    try
    {
        return toCode(mh->reader->getWidth(::HEIF::ImageId(image_id), *out_width));
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_height(const heif_reader_t* reader, uint32_t image_id, uint32_t* out_height)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_height: reader handle is NULL");
    }
    if (out_height == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_height: out_height is NULL");
    }
    auto* mh = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(mh))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_height: reader handle is not live (use-after-destroy or never created)");
    }
    if (mh->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_height: underlying Reader is NULL");
    }
    try
    {
        return toCode(mh->reader->getHeight(::HEIF::ImageId(image_id), *out_height));
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_item_type(const heif_reader_t* reader, uint32_t image_id, char out_type[5])
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_type: reader handle is NULL");
    }
    if (out_type == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_type: out_type is NULL");
    }
    auto* mh = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(mh))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_type: reader handle is not live (use-after-destroy or never created)");
    }
    if (mh->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_type: underlying Reader is NULL");
    }
    try
    {
        ::HEIF::FourCC type;
        const auto     err = mh->reader->getItemType(::HEIF::ImageId(image_id), type);
        if (err == ::HEIF::ErrorCode::OK)
        {
            copyFourCC(type, out_type);
        }
        return toCode(err);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_item_data_with_decoder_parameters(heif_reader_t* reader,
                                                                    uint32_t      image_id,
                                                                    uint8_t*      buffer,
                                                                    uint64_t*     size_inout)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_data_with_decoder_parameters: reader handle is NULL");
    }
    if (size_inout == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_data_with_decoder_parameters: size_inout is NULL");
    }
    // Probe-call shape: (NULL, 0) is legal. (non-NULL, 0) and (NULL, >0)
    // are both undefined per the header contract; surface them as
    // INVALID_FUNCTION_PARAMETER rather than gambling on the C++
    // implementation tolerating them.
    if ((buffer == nullptr) && (*size_inout != 0U))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_data_with_decoder_parameters: buffer is NULL but *size_inout > 0 (probe call requires *size_inout == 0)");
    }
    auto* h = asHandle(reader);
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_data_with_decoder_parameters: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_data_with_decoder_parameters: underlying Reader is NULL");
    }
    try
    {
        // size_inout is in/out: the C++ contract writes the required
        // size on BUFFER_SIZE_TOO_SMALL and the actual size on OK.
        return toCode(
            h->reader->getItemDataWithDecoderParameters(::HEIF::ImageId(image_id), buffer, *size_inout));
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_minor_version(const heif_reader_t* reader, uint32_t* out_minor_version)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_minor_version: reader handle is NULL");
    }
    if (out_minor_version == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_minor_version: out_minor_version is NULL");
    }
    auto* mh = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(mh))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_minor_version: reader handle is not live (use-after-destroy or never created)");
    }
    if (mh->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_minor_version: underlying Reader is NULL");
    }
    try
    {
        return toCode(mh->reader->getMinorVersion(*out_minor_version));
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_compatible_brand_count(const heif_reader_t* reader, uint32_t* out_count)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_compatible_brand_count: reader handle is NULL");
    }
    if (out_count == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_compatible_brand_count: out_count is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_compatible_brand_count: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_compatible_brand_count: underlying Reader is NULL");
    }
    try
    {
        if (!h->compatibleBrandsValid)
        {
            const auto err = h->reader->getCompatibleBrands(h->compatibleBrands);
            if (err != ::HEIF::ErrorCode::OK)
            {
                return toCode(err);
            }
            h->compatibleBrandsValid = true;
        }
        *out_count = static_cast<uint32_t>(h->compatibleBrands.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_compatible_brand_at(const heif_reader_t* reader,
                                                      uint32_t            index,
                                                      char                out_brand[5])
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_compatible_brand_at: reader handle is NULL");
    }
    if (out_brand == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_compatible_brand_at: out_brand is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_compatible_brand_at: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_compatible_brand_at: underlying Reader is NULL");
    }
    try
    {
        if (!h->compatibleBrandsValid)
        {
            const auto err = h->reader->getCompatibleBrands(h->compatibleBrands);
            if (err != ::HEIF::ErrorCode::OK)
            {
                return toCode(err);
            }
            h->compatibleBrandsValid = true;
        }
        if (index >= h->compatibleBrands.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_compatible_brand_at: index >= compatible_brand_count");
        }
        copyFourCC(h->compatibleBrands.elements[index], out_brand);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_primary_item(const heif_reader_t* reader, uint32_t* out_image_id)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_primary_item: reader handle is NULL");
    }
    if (out_image_id == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_primary_item: out_image_id is NULL");
    }
    auto* mh = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(mh))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_primary_item: reader handle is not live (use-after-destroy or never created)");
    }
    if (mh->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_primary_item: underlying Reader is NULL");
    }
    try
    {
        ::HEIF::ImageId id;
        const auto      err = mh->reader->getPrimaryItem(id);
        if (err == ::HEIF::ErrorCode::OK)
        {
            *out_image_id = id.get();
        }
        return toCode(err);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_item_list_by_type_count(const heif_reader_t* reader,
                                                          const char*         type,
                                                          uint32_t*           out_count)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_list_by_type_count: reader handle is NULL");
    }
    if (type == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_list_by_type_count: type is NULL");
    }
    if (out_count == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_list_by_type_count: out_count is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_list_by_type_count: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_list_by_type_count: underlying Reader is NULL");
    }
    try
    {
        const ::HEIF::FourCC requested(type);
        if (!h->cachedItemListValid || (h->cachedItemListType != requested))
        {
            const auto err = h->reader->getItemListByType(requested, h->cachedItemList);
            if (err != ::HEIF::ErrorCode::OK)
            {
                h->cachedItemListValid = false;
                return toCode(err);
            }
            h->cachedItemListType  = requested;
            h->cachedItemListValid = true;
        }
        *out_count = static_cast<uint32_t>(h->cachedItemList.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_item_list_by_type_at(const heif_reader_t* reader,
                                                       const char*         type,
                                                       uint32_t            index,
                                                       uint32_t*           out_image_id)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_list_by_type_at: reader handle is NULL");
    }
    if (type == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_list_by_type_at: type is NULL");
    }
    if (out_image_id == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_list_by_type_at: out_image_id is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_list_by_type_at: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_list_by_type_at: underlying Reader is NULL");
    }
    try
    {
        const ::HEIF::FourCC requested(type);
        if (!h->cachedItemListValid || (h->cachedItemListType != requested))
        {
            const auto err = h->reader->getItemListByType(requested, h->cachedItemList);
            if (err != ::HEIF::ErrorCode::OK)
            {
                h->cachedItemListValid = false;
                return toCode(err);
            }
            h->cachedItemListType  = requested;
            h->cachedItemListValid = true;
        }
        if (index >= h->cachedItemList.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_item_list_by_type_at: index >= item_list_by_type_count");
        }
        *out_image_id = h->cachedItemList.elements[index].get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_referenced_from_item_list_by_type_count(const heif_reader_t* reader,
                                                                          uint32_t            from_image_id,
                                                                          const char*         reference_type,
                                                                          uint32_t*           out_count)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_referenced_from_item_list_by_type_count: reader handle is NULL");
    }
    if (reference_type == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_referenced_from_item_list_by_type_count: reference_type is NULL");
    }
    if (out_count == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_referenced_from_item_list_by_type_count: out_count is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_referenced_from_item_list_by_type_count: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_referenced_from_item_list_by_type_count: underlying Reader is NULL");
    }
    try
    {
        const ::HEIF::ImageId fromId(from_image_id);
        const ::HEIF::FourCC  refType(reference_type);
        if (!h->cachedRefFromValid
            || (h->cachedRefFromId != fromId)
            || (h->cachedRefFromType != refType))
        {
            const auto err = h->reader->getReferencedFromItemListByType(fromId, refType, h->cachedRefFromList);
            if (err != ::HEIF::ErrorCode::OK)
            {
                h->cachedRefFromValid = false;
                return toCode(err);
            }
            h->cachedRefFromId    = fromId;
            h->cachedRefFromType  = refType;
            h->cachedRefFromValid = true;
        }
        *out_count = static_cast<uint32_t>(h->cachedRefFromList.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_referenced_from_item_list_by_type_at(const heif_reader_t* reader,
                                                                       uint32_t            from_image_id,
                                                                       const char*         reference_type,
                                                                       uint32_t            index,
                                                                       uint32_t*           out_image_id)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_referenced_from_item_list_by_type_at: reader handle is NULL");
    }
    if (reference_type == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_referenced_from_item_list_by_type_at: reference_type is NULL");
    }
    if (out_image_id == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_referenced_from_item_list_by_type_at: out_image_id is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_referenced_from_item_list_by_type_at: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_referenced_from_item_list_by_type_at: underlying Reader is NULL");
    }
    try
    {
        const ::HEIF::ImageId fromId(from_image_id);
        const ::HEIF::FourCC  refType(reference_type);
        if (!h->cachedRefFromValid
            || (h->cachedRefFromId != fromId)
            || (h->cachedRefFromType != refType))
        {
            const auto err = h->reader->getReferencedFromItemListByType(fromId, refType, h->cachedRefFromList);
            if (err != ::HEIF::ErrorCode::OK)
            {
                h->cachedRefFromValid = false;
                return toCode(err);
            }
            h->cachedRefFromId    = fromId;
            h->cachedRefFromType  = refType;
            h->cachedRefFromValid = true;
        }
        if (index >= h->cachedRefFromList.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_referenced_from_item_list_by_type_at: index >= referenced_from_count");
        }
        *out_image_id = h->cachedRefFromList.elements[index].get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_referenced_to_item_list_by_type_count(const heif_reader_t* reader,
                                                                        uint32_t            to_image_id,
                                                                        const char*         reference_type,
                                                                        uint32_t*           out_count)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_referenced_to_item_list_by_type_count: reader handle is NULL");
    }
    if (reference_type == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_referenced_to_item_list_by_type_count: reference_type is NULL");
    }
    if (out_count == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_referenced_to_item_list_by_type_count: out_count is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_referenced_to_item_list_by_type_count: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_referenced_to_item_list_by_type_count: underlying Reader is NULL");
    }
    try
    {
        const ::HEIF::ImageId toId(to_image_id);
        const ::HEIF::FourCC  refType(reference_type);
        if (!h->cachedRefToValid
            || (h->cachedRefToId != toId)
            || (h->cachedRefToType != refType))
        {
            const auto err = h->reader->getReferencedToItemListByType(toId, refType, h->cachedRefToList);
            if (err != ::HEIF::ErrorCode::OK)
            {
                h->cachedRefToValid = false;
                return toCode(err);
            }
            h->cachedRefToId    = toId;
            h->cachedRefToType  = refType;
            h->cachedRefToValid = true;
        }
        *out_count = static_cast<uint32_t>(h->cachedRefToList.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_referenced_to_item_list_by_type_at(const heif_reader_t* reader,
                                                                     uint32_t            to_image_id,
                                                                     const char*         reference_type,
                                                                     uint32_t            index,
                                                                     uint32_t*           out_image_id)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_referenced_to_item_list_by_type_at: reader handle is NULL");
    }
    if (reference_type == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_referenced_to_item_list_by_type_at: reference_type is NULL");
    }
    if (out_image_id == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_referenced_to_item_list_by_type_at: out_image_id is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_referenced_to_item_list_by_type_at: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_referenced_to_item_list_by_type_at: underlying Reader is NULL");
    }
    try
    {
        const ::HEIF::ImageId toId(to_image_id);
        const ::HEIF::FourCC  refType(reference_type);
        if (!h->cachedRefToValid
            || (h->cachedRefToId != toId)
            || (h->cachedRefToType != refType))
        {
            const auto err = h->reader->getReferencedToItemListByType(toId, refType, h->cachedRefToList);
            if (err != ::HEIF::ErrorCode::OK)
            {
                h->cachedRefToValid = false;
                return toCode(err);
            }
            h->cachedRefToId    = toId;
            h->cachedRefToType  = refType;
            h->cachedRefToValid = true;
        }
        if (index >= h->cachedRefToList.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_referenced_to_item_list_by_type_at: index >= referenced_to_count");
        }
        *out_image_id = h->cachedRefToList.elements[index].get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_item_data(heif_reader_t* reader,
                                            uint32_t      image_id,
                                            uint8_t*      buffer,
                                            uint64_t*     size_inout)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_data: reader handle is NULL");
    }
    if (size_inout == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_data: size_inout is NULL");
    }
    if ((buffer == nullptr) && (*size_inout != 0U))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_data: buffer is NULL but *size_inout > 0 (probe call requires *size_inout == 0)");
    }
    auto* h = asHandle(reader);
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_data: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_data: underlying Reader is NULL");
    }
    try
    {
        // Forward with bytestreamHeaders=true (the C++ default). Bindings
        // wanting raw container bytes for an encoded item should use
        // _with_decoder_parameters and strip the prefix themselves.
        return toCode(
            h->reader->getItemData(::HEIF::ImageId(image_id), buffer, *size_inout, /*bytestreamHeaders=*/true));
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_item_property_count(const heif_reader_t* reader,
                                                      uint32_t            image_id,
                                                      uint32_t*           out_count)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_property_count: reader handle is NULL");
    }
    if (out_count == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_property_count: out_count is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_property_count: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_property_count: underlying Reader is NULL");
    }
    try
    {
        const ::HEIF::ImageId id(image_id);
        if (!h->cachedPropListValid || (h->cachedPropListImageId != id))
        {
            const auto err = h->reader->getItemProperties(id, h->cachedPropList);
            if (err != ::HEIF::ErrorCode::OK)
            {
                h->cachedPropListValid = false;
                return toCode(err);
            }
            h->cachedPropListImageId = id;
            h->cachedPropListValid   = true;
        }
        *out_count = static_cast<uint32_t>(h->cachedPropList.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_item_property_at(const heif_reader_t* reader,
                                                   uint32_t            image_id,
                                                   uint32_t            index,
                                                   uint32_t*           out_property_id,
                                                   char                out_type[5],
                                                   int*                out_essential)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_property_at: reader handle is NULL");
    }
    if (out_property_id == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_property_at: out_property_id is NULL");
    }
    if (out_type == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_property_at: out_type is NULL");
    }
    if (out_essential == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_item_property_at: out_essential is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_property_at: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_item_property_at: underlying Reader is NULL");
    }
    try
    {
        const ::HEIF::ImageId id(image_id);
        if (!h->cachedPropListValid || (h->cachedPropListImageId != id))
        {
            const auto err = h->reader->getItemProperties(id, h->cachedPropList);
            if (err != ::HEIF::ErrorCode::OK)
            {
                h->cachedPropListValid = false;
                return toCode(err);
            }
            h->cachedPropListImageId = id;
            h->cachedPropListValid   = true;
        }
        if (index >= h->cachedPropList.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_item_property_at: index >= item_property_count");
        }
        const auto& info  = h->cachedPropList.elements[index];
        *out_property_id  = info.index.get();
        propertyTypeToFourCC(info.type, out_type);
        *out_essential    = info.essential ? 1 : 0;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

// Each typed-accessor entry point below performs the same NULL/liveness
// checks. Expanded inline rather than via a helper because each site
// needs a unique fn-name token in its diagnostic message and a helper
// would either require __FUNCTION__ string-passing per call or lose the
// per-site message specificity.

heif_error_code_t heif_reader_get_property_ispe(const heif_reader_t* reader,
                                                uint32_t            property_id,
                                                uint32_t*           out_width,
                                                uint32_t*           out_height)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_ispe: reader handle is NULL");
    }
    if ((out_width == nullptr) || (out_height == nullptr))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_ispe: out_width or out_height is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_ispe: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_ispe: underlying Reader is NULL");
    }
    try
    {
        ::HEIF::SpatialExtents ispe{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), ispe);
        if (err == ::HEIF::ErrorCode::OK)
        {
            *out_width  = ispe.mImageWidth;
            *out_height = ispe.mImageHeight;
        }
        return toCode(err);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_property_irot(const heif_reader_t* reader,
                                                uint32_t            property_id,
                                                uint32_t*           out_angle_ccw)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_irot: reader handle is NULL");
    }
    if (out_angle_ccw == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_irot: out_angle_ccw is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_irot: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_irot: underlying Reader is NULL");
    }
    try
    {
        ::HEIF::Rotate irot{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), irot);
        if (err == ::HEIF::ErrorCode::OK)
        {
            *out_angle_ccw = irot.angle;
        }
        return toCode(err);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_property_imir(const heif_reader_t* reader,
                                                uint32_t            property_id,
                                                int*                out_horizontal_axis)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_imir: reader handle is NULL");
    }
    if (out_horizontal_axis == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_imir: out_horizontal_axis is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_imir: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_imir: underlying Reader is NULL");
    }
    try
    {
        ::HEIF::Mirror imir{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), imir);
        if (err == ::HEIF::ErrorCode::OK)
        {
            *out_horizontal_axis = imir.horizontalAxis ? 1 : 0;
        }
        return toCode(err);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_property_pixi_channel_count(const heif_reader_t* reader,
                                                              uint32_t            property_id,
                                                              uint32_t*           out_count)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_pixi_channel_count: reader handle is NULL");
    }
    if (out_count == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_pixi_channel_count: out_count is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_pixi_channel_count: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_pixi_channel_count: underlying Reader is NULL");
    }
    try
    {
        const auto err = ensurePixiCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK)
        {
            return toCode(err);
        }
        *out_count = static_cast<uint32_t>(h->cachedPixi.bitsPerChannel.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_property_pixi_bits_at(const heif_reader_t* reader,
                                                        uint32_t            property_id,
                                                        uint32_t            channel_index,
                                                        uint8_t*            out_bits)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_pixi_bits_at: reader handle is NULL");
    }
    if (out_bits == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_pixi_bits_at: out_bits is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_pixi_bits_at: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_pixi_bits_at: underlying Reader is NULL");
    }
    try
    {
        const auto err = ensurePixiCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK)
        {
            return toCode(err);
        }
        if (channel_index >= h->cachedPixi.bitsPerChannel.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_pixi_bits_at: channel_index >= pixi_channel_count");
        }
        *out_bits = h->cachedPixi.bitsPerChannel.elements[channel_index];
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_property_colr_type(const heif_reader_t* reader,
                                                     uint32_t            property_id,
                                                     char                out_type[5])
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_colr_type: reader handle is NULL");
    }
    if (out_type == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_colr_type: out_type is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_colr_type: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_colr_type: underlying Reader is NULL");
    }
    try
    {
        ::HEIF::ColourInformation colr{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), colr);
        if (err == ::HEIF::ErrorCode::OK)
        {
            copyFourCC(colr.colourType, out_type);
        }
        return toCode(err);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_property_colr_nclx(const heif_reader_t* reader,
                                                     uint32_t            property_id,
                                                     uint16_t*           out_primaries,
                                                     uint16_t*           out_transfer,
                                                     uint16_t*           out_matrix,
                                                     int*                out_full_range)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_colr_nclx: reader handle is NULL");
    }
    if ((out_primaries == nullptr) || (out_transfer == nullptr)
        || (out_matrix == nullptr) || (out_full_range == nullptr))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_colr_nclx: one or more out pointers are NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_colr_nclx: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_colr_nclx: underlying Reader is NULL");
    }
    try
    {
        ::HEIF::ColourInformation colr{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), colr);
        if (err == ::HEIF::ErrorCode::OK)
        {
            *out_primaries  = colr.colourPrimaries;
            *out_transfer   = colr.transferCharacteristics;
            *out_matrix     = colr.matrixCoefficients;
            *out_full_range = colr.fullRangeFlag ? 1 : 0;
        }
        return toCode(err);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_property_colr_icc(const heif_reader_t* reader,
                                                    uint32_t            property_id,
                                                    uint8_t*            buffer,
                                                    uint64_t*           size_inout)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_colr_icc: reader handle is NULL");
    }
    if (size_inout == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_colr_icc: size_inout is NULL");
    }
    if ((buffer == nullptr) && (*size_inout != 0U))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_colr_icc: buffer is NULL but *size_inout > 0 (probe call requires *size_inout == 0)");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_colr_icc: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_colr_icc: underlying Reader is NULL");
    }
    try
    {
        // Two-step probe-or-fetch: fetch the ColourInformation (cheap)
        // then implement the (buf, *size_inout) protocol on the
        // iccProfile array ourselves. ColourInformation is small and the
        // typical access pattern is "type then nclx OR icc" -- caching
        // would be unused.
        ::HEIF::ColourInformation colr{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), colr);
        if (err != ::HEIF::ErrorCode::OK)
        {
            return toCode(err);
        }
        const auto needed = static_cast<uint64_t>(colr.iccProfile.size);
        if (buffer == nullptr)
        {
            // Probe call. Return BUFFER_SIZE_TOO_SMALL with the required
            // byte count. A 0-byte ICC profile (nclx-typed colr) returns
            // OK with size 0 so the binding does not allocate-and-retry.
            *size_inout = needed;
            if (needed == 0U)
            {
                return toCode(::HEIF::ErrorCode::OK);
            }
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::BUFFER_SIZE_TOO_SMALL,
                                "heif_reader_get_property_colr_icc: probe -- caller buffer is NULL");
        }
        // Fetch call.
        if (*size_inout < needed)
        {
            *size_inout = needed;
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::BUFFER_SIZE_TOO_SMALL,
                                "heif_reader_get_property_colr_icc: caller buffer too small");
        }
        if (needed > 0U)
        {
            std::memcpy(buffer, colr.iccProfile.elements, static_cast<std::size_t>(needed));
        }
        *size_inout = needed;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_property_pasp(const heif_reader_t* reader,
                                                uint32_t            property_id,
                                                uint32_t*           out_relative_width,
                                                uint32_t*           out_relative_height)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_pasp: reader handle is NULL");
    }
    if ((out_relative_width == nullptr) || (out_relative_height == nullptr))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_pasp: out_relative_width or out_relative_height is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_pasp: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_pasp: underlying Reader is NULL");
    }
    try
    {
        ::HEIF::PixelAspectRatio pasp{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), pasp);
        if (err == ::HEIF::ErrorCode::OK)
        {
            *out_relative_width  = pasp.relativeWidth;
            *out_relative_height = pasp.relativeHeight;
        }
        return toCode(err);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

/* Minimum sensible struct_size: the struct_size field plus the single
 * byte field present since the first version of heif_pixi_channel_t.
 * A caller passing less is using a struct definition older than this
 * facade ever supported and is treated as a contract violation. */
static constexpr uint32_t kPixiChannelMinStructSize =
    static_cast<uint32_t>(sizeof(uint32_t) + sizeof(uint8_t));

/* Write the current fields of heif_pixi_channel_t into the caller's
 * buffer, honouring the prefix-fill semantics. effective is
 * min(caller_struct_size, sizeof(heif_pixi_channel_t)); if the caller
 * asked for more than we know, the trailing bytes are zeroed so the
 * caller never observes uninitialised memory. */
static inline void fillPixiChannel(heif_pixi_channel_t*                          out,
                                   const ::HEIF::PixelInformation::ChannelEntry& entry,
                                   uint32_t                                      caller_struct_size) noexcept
{
    const uint32_t our_size  = static_cast<uint32_t>(sizeof(heif_pixi_channel_t));
    const uint32_t effective = (caller_struct_size < our_size) ? caller_struct_size : our_size;

    /* Build a local copy of the current layout, then memcpy exactly
     * `effective` bytes. Avoids per-field offsetof gymnastics and stays
     * append-safe: a future field added at the tail is filled iff the
     * caller's struct_size covers it. */
    heif_pixi_channel_t tmp{};
    tmp.struct_size          = our_size;
    tmp.channel_idc          = entry.channelIdc;
    tmp.component_format     = entry.info.ComponentFormat;
    tmp.subsampling_flag     = entry.info.SubSamplingFlag ? static_cast<uint8_t>(1U) : static_cast<uint8_t>(0U);
    tmp.channel_label_flag   = entry.info.ChannelLabelFlag ? static_cast<uint8_t>(1U) : static_cast<uint8_t>(0U);
    tmp.subsampling_type     = entry.info.SubSamplingType;
    tmp.subsampling_location = entry.info.SubSamplingLocation;
    /* tmp._reserved already zeroed by value-init. */

    std::memcpy(out, &tmp, effective);

    /* Forward compatibility: caller knows about a longer struct than
     * this library can populate. Zero the bytes past our sizeof so the
     * caller sees deterministic zero, never garbage. */
    if (caller_struct_size > our_size)
    {
        auto* tail = reinterpret_cast<uint8_t*>(out) + our_size;
        std::memset(tail, 0, static_cast<std::size_t>(caller_struct_size - our_size));
    }
}

heif_error_code_t heif_reader_get_property_pixi_num_channels(const heif_reader_t* reader,
                                                             uint32_t            property_id,
                                                             uint8_t*            out_num_channels)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_pixi_num_channels: reader handle is NULL");
    }
    if (out_num_channels == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_pixi_num_channels: out_num_channels is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_pixi_num_channels: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_pixi_num_channels: underlying Reader is NULL");
    }
    try
    {
        const auto err = ensurePixiCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK)
        {
            return toCode(err);
        }
        *out_num_channels = h->cachedPixi.numChannels;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_property_pixi_channel_at(const heif_reader_t* reader,
                                                           uint32_t            property_id,
                                                           uint32_t            channel_index,
                                                           heif_pixi_channel_t* out_channel)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_pixi_channel_at: reader handle is NULL");
    }
    if (out_channel == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_pixi_channel_at: out_channel is NULL");
    }
    /* Read the caller's struct_size BEFORE touching any other field so a
     * caller who passed a too-small struct never has trailing bytes
     * scribbled. */
    const uint32_t caller_struct_size = out_channel->struct_size;
    if (caller_struct_size < kPixiChannelMinStructSize)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_pixi_channel_at: out_channel->struct_size is 0 "
                            "or smaller than the minimum supported version (struct_size violation; caller "
                            "must set struct_size = sizeof(heif_pixi_channel_t) before the call)");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_pixi_channel_at: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_pixi_channel_at: underlying Reader is NULL");
    }
    try
    {
        const auto err = ensurePixiCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK)
        {
            return toCode(err);
        }
        if (channel_index >= h->cachedPixi.ChannelData.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_pixi_channel_at: channel_index >= ChannelData.size "
                                "(v0/v1 pixi without ChannelData -> size is 0; use _pixi_bits_at instead)");
        }
        fillPixiChannel(out_channel,
                        h->cachedPixi.ChannelData.elements[channel_index],
                        caller_struct_size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_property_pixi_channel_label(const heif_reader_t* reader,
                                                              uint32_t            property_id,
                                                              uint32_t            channel_index,
                                                              char*               buffer,
                                                              uint64_t*           size_inout)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_pixi_channel_label: reader handle is NULL");
    }
    if (size_inout == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_pixi_channel_label: size_inout is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_pixi_channel_label: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_property_pixi_channel_label: underlying Reader is NULL");
    }
    try
    {
        const auto err = ensurePixiCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK)
        {
            return toCode(err);
        }
        if (channel_index >= h->cachedPixi.ChannelData.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_pixi_channel_label: channel_index >= ChannelData.size");
        }
        const auto& info  = h->cachedPixi.ChannelData.elements[channel_index].info;
        const auto& label = info.ChannelLabel;

        /* Channel without a label: return OK with size 0. */
        if (!info.ChannelLabelFlag || (label.size == 0U))
        {
            *size_inout = 0U;
            return toCode(::HEIF::ErrorCode::OK);
        }

        /* Required size includes the terminating NUL. ChannelLabel is a
         * UTF-8 byte sequence; we always NUL-terminate at the boundary
         * so the C caller can use it as a C string. */
        const uint64_t required = static_cast<uint64_t>(label.size) + 1U;
        const uint64_t cap      = *size_inout;
        *size_inout             = required;
        if ((buffer == nullptr) || (cap < required))
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::BUFFER_SIZE_TOO_SMALL,
                                "heif_reader_get_property_pixi_channel_label: caller buffer too small "
                                "for ChannelLabel (size-probe; *size_inout now holds required bytes)");
        }
        std::memcpy(buffer, label.elements, static_cast<std::size_t>(label.size));
        buffer[label.size] = '\0';
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

}  // extern "C" -- the helper namespace below MUST be at C++ linkage;
   // we re-open extern "C" for the actual entry points.

namespace
{
    // Write the current layout into out honouring the caller's
    // struct_size handshake. caller_size must already have been validated
    // to be >= the minimum supported version.
    template <typename T>
    inline void writeRule8Struct(T* out, const T& source, uint32_t caller_size) noexcept
    {
        const uint32_t our_size = static_cast<uint32_t>(sizeof(T));
        const uint32_t effective = (caller_size < our_size) ? caller_size : our_size;
        std::memcpy(out, &source, effective);
        if (caller_size > our_size)
        {
            auto* tail = reinterpret_cast<uint8_t*>(out) + our_size;
            std::memset(tail, 0, static_cast<std::size_t>(caller_size - our_size));
        }
    }

    // Minimum sensible struct_size: the struct_size field itself (4 bytes).
    // Per-struct accessors may add stricter bounds.
    constexpr uint32_t kRule8MinStructSize = static_cast<uint32_t>(sizeof(uint32_t));
}

extern "C"
{

#define HEIF_C_GUARD_PROPERTY(fn_name_, out_ptr_)                                            \
    if (reader == nullptr)                                                                   \
    {                                                                                        \
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,                   \
                            fn_name_ ": reader handle is NULL");                             \
    }                                                                                        \
    if ((out_ptr_) == nullptr)                                                               \
    {                                                                                        \
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,                   \
                            fn_name_ ": out pointer is NULL");                               \
    }                                                                                        \
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));                                   \
    if (!isLiveHandle(h))                                                                    \
    {                                                                                        \
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,                                \
                            fn_name_ ": reader handle is not live (use-after-destroy or never created)"); \
    }                                                                                        \
    if (h->reader == nullptr)                                                                \
    {                                                                                        \
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,                                \
                            fn_name_ ": underlying Reader is NULL");                         \
    }                                                                                        \
    do { } while (0)

#define HEIF_C_GUARD_RULE8(fn_name_, struct_member_, min_size_)                              \
    do {                                                                                     \
        if ((struct_member_) < (min_size_))                                                  \
        {                                                                                    \
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,               \
                                fn_name_ ": out->struct_size is 0 or smaller than the "     \
                                "minimum supported version (struct_size violation; caller must " \
                                "set struct_size = sizeof(out_struct_t) before the call)"); \
        }                                                                                    \
    } while (0)

heif_error_code_t heif_reader_get_property_clli(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                heif_clli_t*         out_clli)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_clli", out_clli);
    const uint32_t cs = out_clli->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_property_clli", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::ContentLightLevel src{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), src);
        if (err != ::HEIF::ErrorCode::OK)
        {
            return toCode(err);
        }
        heif_clli_t tmp{};
        tmp.struct_size                  = static_cast<uint32_t>(sizeof(heif_clli_t));
        tmp.max_content_light_level      = src.maxContentLightLevel;
        tmp.max_pic_average_light_level  = src.maxPicAverageLightLevel;
        writeRule8Struct(out_clli, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_mdcv(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                heif_mdcv_t*         out_mdcv)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_mdcv", out_mdcv);
    const uint32_t cs = out_mdcv->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_property_mdcv", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::MasteringDisplayColorVolume src{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), src);
        if (err != ::HEIF::ErrorCode::OK)
        {
            return toCode(err);
        }
        if ((src.displayPrimariesX.size != 3U) || (src.displayPrimariesY.size != 3U))
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::FILE_HEADER_ERROR,
                                "heif_reader_get_property_mdcv: malformed mdcv (primaries arrays "
                                "have unexpected length; spec requires exactly 3)");
        }
        heif_mdcv_t tmp{};
        tmp.struct_size = static_cast<uint32_t>(sizeof(heif_mdcv_t));
        for (int i = 0; i < 3; ++i)
        {
            tmp.display_primaries_x[i] = src.displayPrimariesX.elements[i];
            tmp.display_primaries_y[i] = src.displayPrimariesY.elements[i];
        }
        tmp.white_point_x                    = src.whitePointX;
        tmp.white_point_y                    = src.whitePointY;
        tmp.max_display_mastering_luminance  = src.maxDisplayMasteringLuminance;
        tmp.min_display_mastering_luminance  = src.minDisplayMasteringLuminance;
        writeRule8Struct(out_mdcv, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_cclv(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                heif_cclv_t*         out_cclv)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_cclv", out_cclv);
    const uint32_t cs = out_cclv->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_property_cclv", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::ContentColorVolume src{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), src);
        if (err != ::HEIF::ErrorCode::OK)
        {
            return toCode(err);
        }
        heif_cclv_t tmp{};
        tmp.struct_size              = static_cast<uint32_t>(sizeof(heif_cclv_t));
        tmp.primaries_present        = src.ccvPrimariesPresent ? 1U : 0U;
        tmp.min_luminance_present    = src.ccvMinLuminanceValuePresent ? 1U : 0U;
        tmp.max_luminance_present    = src.ccvMaxLuminanceValuePresent ? 1U : 0U;
        tmp.avg_luminance_present    = src.ccvAvgLuminanceValuePresent ? 1U : 0U;
        if (src.ccvPrimariesPresent)
        {
            if ((src.ccvPrimariesX.size != 3U) || (src.ccvPrimariesY.size != 3U))
            {
                HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::FILE_HEADER_ERROR,
                                    "heif_reader_get_property_cclv: malformed cclv "
                                    "(primaries arrays length != 3)");
            }
            for (int i = 0; i < 3; ++i)
            {
                tmp.primaries_x[i] = src.ccvPrimariesX.elements[i];
                tmp.primaries_y[i] = src.ccvPrimariesY.elements[i];
            }
        }
        tmp.min_luminance_value = src.ccvMinLuminanceValue;
        tmp.max_luminance_value = src.ccvMaxLuminanceValue;
        tmp.avg_luminance_value = src.ccvAvgLuminanceValue;
        writeRule8Struct(out_cclv, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_amve(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                heif_amve_t*         out_amve)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_amve", out_amve);
    const uint32_t cs = out_amve->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_property_amve", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::AmbientViewingEnvironment src{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), src);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        heif_amve_t tmp{};
        tmp.struct_size         = static_cast<uint32_t>(sizeof(heif_amve_t));
        tmp.ambient_illuminance = src.ambientIlluminance;
        tmp.ambient_light_x     = src.ambientLightX;
        tmp.ambient_light_y     = src.ambientLightY;
        writeRule8Struct(out_amve, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_reve(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                heif_reve_t*         out_reve)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_reve", out_reve);
    const uint32_t cs = out_reve->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_property_reve", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::ReferenceViewingEnvironment src{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), src);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        heif_reve_t tmp{};
        tmp.struct_size          = static_cast<uint32_t>(sizeof(heif_reve_t));
        tmp.surround_luminance   = src.surroundLuminance;
        tmp.surround_light_x     = src.surroundLightX;
        tmp.surround_light_y     = src.surroundLightY;
        tmp.periphery_luminance  = src.peripheryLuminance;
        tmp.periphery_light_x    = src.peripheryLightX;
        tmp.periphery_light_y    = src.peripheryLightY;
        writeRule8Struct(out_reve, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_ndwt(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                heif_ndwt_t*         out_ndwt)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_ndwt", out_ndwt);
    const uint32_t cs = out_ndwt->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_property_ndwt", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::NominalDiffuseWhite src{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), src);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        heif_ndwt_t tmp{};
        tmp.struct_size              = static_cast<uint32_t>(sizeof(heif_ndwt_t));
        tmp.diffuse_white_luminance  = src.diffuseWhiteLuminance;
        writeRule8Struct(out_ndwt, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_grid(const heif_reader_t* reader,
                                       uint32_t             image_id,
                                       heif_grid_t*         out_grid)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_grid", out_grid);
    const uint32_t cs = out_grid->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_grid", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::Grid g{};
        const auto err = h->reader->getItem(::HEIF::ImageId(image_id), g);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        heif_grid_t tmp{};
        tmp.struct_size   = static_cast<uint32_t>(sizeof(heif_grid_t));
        tmp.output_width  = g.outputWidth;
        tmp.output_height = g.outputHeight;
        tmp.columns       = g.columns;
        tmp.rows          = g.rows;
        writeRule8Struct(out_grid, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_grid_image_count(const heif_reader_t* reader,
                                                   uint32_t             image_id,
                                                   uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_grid_image_count", out_count);
    try
    {
        ::HEIF::Grid g{};
        const auto err = h->reader->getItem(::HEIF::ImageId(image_id), g);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(g.imageIds.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_grid_image_at(const heif_reader_t* reader,
                                                uint32_t             image_id,
                                                uint32_t             index,
                                                uint32_t*            out_image_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_grid_image_at", out_image_id);
    try
    {
        ::HEIF::Grid g{};
        const auto err = h->reader->getItem(::HEIF::ImageId(image_id), g);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= g.imageIds.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_grid_image_at: index >= grid image count");
        }
        *out_image_id = g.imageIds.elements[index].get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_overlay(const heif_reader_t* reader,
                                          uint32_t             image_id,
                                          heif_overlay_t*      out_overlay)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_overlay", out_overlay);
    const uint32_t cs = out_overlay->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_overlay", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::Overlay o{};
        const auto err = h->reader->getItem(::HEIF::ImageId(image_id), o);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        heif_overlay_t tmp{};
        tmp.struct_size   = static_cast<uint32_t>(sizeof(heif_overlay_t));
        tmp.fill_r        = o.r;
        tmp.fill_g        = o.g;
        tmp.fill_b        = o.b;
        tmp.fill_a        = o.a;
        tmp.output_width  = o.outputWidth;
        tmp.output_height = o.outputHeight;
        writeRule8Struct(out_overlay, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_overlay_image_count(const heif_reader_t* reader,
                                                      uint32_t             image_id,
                                                      uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_overlay_image_count", out_count);
    try
    {
        ::HEIF::Overlay o{};
        const auto err = h->reader->getItem(::HEIF::ImageId(image_id), o);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(o.imageIds.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_overlay_image_at(const heif_reader_t*  reader,
                                                   uint32_t              image_id,
                                                   uint32_t              index,
                                                   heif_overlay_image_t* out_image)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_overlay_image_at", out_image);
    const uint32_t cs = out_image->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_overlay_image_at", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::Overlay o{};
        const auto err = h->reader->getItem(::HEIF::ImageId(image_id), o);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if ((index >= o.imageIds.size) || (index >= o.offsets.size))
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_overlay_image_at: index >= overlay image count");
        }
        heif_overlay_image_t tmp{};
        tmp.struct_size       = static_cast<uint32_t>(sizeof(heif_overlay_image_t));
        tmp.image_id          = o.imageIds.elements[index].get();
        tmp.horizontal_offset = o.offsets.elements[index].horizontal;
        tmp.vertical_offset   = o.offsets.elements[index].vertical;
        writeRule8Struct(out_image, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_tonemap_gainmap(const heif_reader_t* reader,
                                                  uint32_t             image_id,
                                                  heif_gainmap_t*      out_gainmap)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_tonemap_gainmap", out_gainmap);
    const uint32_t cs = out_gainmap->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_tonemap_gainmap", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::Tonemap t{};
        const auto err = h->reader->getItem(::HEIF::ImageId(image_id), t);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto& gm = t.gain_map_metadata;
        heif_gainmap_t tmp{};
        tmp.struct_size                        = static_cast<uint32_t>(sizeof(heif_gainmap_t));
        tmp.minimum_version                    = gm.minimumVersion;
        tmp.writer_version                     = gm.writerVersion;
        tmp.is_multichannel                    = gm.isMultichannel ? 1U : 0U;
        tmp.use_base_colour_space              = gm.useBaseColourSpace ? 1U : 0U;
        tmp.channel_count                      = gm.channelCount;
        tmp.base_hdr_headroom_numerator        = gm.baseHdrHeadroomNumerator;
        tmp.base_hdr_headroom_denominator      = gm.baseHdrHeadroomDenominator;
        tmp.alternate_hdr_headroom_numerator   = gm.alternateHdrHeadroomNumerator;
        tmp.alternate_hdr_headroom_denominator = gm.alternateHdrHeadroomDenominator;
        writeRule8Struct(out_gainmap, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_tonemap_gainmap_channel_count(const heif_reader_t* reader,
                                                                uint32_t             image_id,
                                                                uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_tonemap_gainmap_channel_count", out_count);
    try
    {
        ::HEIF::Tonemap t{};
        const auto err = h->reader->getItem(::HEIF::ImageId(image_id), t);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(t.gain_map_metadata.channels.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_tonemap_gainmap_channel_at(const heif_reader_t*    reader,
                                                             uint32_t                image_id,
                                                             uint32_t                index,
                                                             heif_gainmap_channel_t* out_channel)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_tonemap_gainmap_channel_at", out_channel);
    const uint32_t cs = out_channel->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_tonemap_gainmap_channel_at", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::Tonemap t{};
        const auto err = h->reader->getItem(::HEIF::ImageId(image_id), t);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= t.gain_map_metadata.channels.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_tonemap_gainmap_channel_at: index >= channel count");
        }
        const auto& ch = t.gain_map_metadata.channels.elements[index];
        heif_gainmap_channel_t tmp{};
        tmp.struct_size                  = static_cast<uint32_t>(sizeof(heif_gainmap_channel_t));
        tmp.gain_map_min_numerator       = ch.gainMapMinNumerator;
        tmp.gain_map_min_denominator     = ch.gainMapMinDenominator;
        tmp.gain_map_max_numerator       = ch.gainMapMaxNumerator;
        tmp.gain_map_max_denominator     = ch.gainMapMaxDenominator;
        tmp.gamma_numerator              = ch.gammaNumerator;
        tmp.gamma_denominator            = ch.gammaDenominator;
        tmp.base_offset_numerator        = ch.baseOffsetNumerator;
        tmp.base_offset_denominator      = ch.baseOffsetDenominator;
        tmp.alternate_offset_numerator   = ch.alternateOffsetNumerator;
        tmp.alternate_offset_denominator = ch.alternateOffsetDenominator;
        writeRule8Struct(out_channel, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_tonemap_image_count(const heif_reader_t* reader,
                                                      uint32_t             image_id,
                                                      uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_tonemap_image_count", out_count);
    try
    {
        ::HEIF::Tonemap t{};
        const auto err = h->reader->getItem(::HEIF::ImageId(image_id), t);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(t.imageIds.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_tonemap_image_at(const heif_reader_t* reader,
                                                   uint32_t             image_id,
                                                   uint32_t             index,
                                                   uint32_t*            out_image_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_tonemap_image_at", out_image_id);
    try
    {
        ::HEIF::Tonemap t{};
        const auto err = h->reader->getItem(::HEIF::ImageId(image_id), t);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= t.imageIds.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_tonemap_image_at: index out of range");
        }
        *out_image_id = t.imageIds.elements[index].get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_file_features(const heif_reader_t* reader,
                                                uint32_t*            out_features)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_file_features", out_features);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_features = static_cast<uint32_t>(h->cachedFileInfo.features);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_metabox_features(const heif_reader_t* reader,
                                                   uint32_t*            out_features)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_metabox_features", out_features);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_features = static_cast<uint32_t>(h->cachedFileInfo.rootMetaBoxInformation.features);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_item_features(const heif_reader_t* reader,
                                                uint32_t             image_id,
                                                uint32_t*            out_features)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_item_features", out_features);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto& items = h->cachedFileInfo.rootMetaBoxInformation.itemInformations;
        for (size_t i = 0; i < items.size; ++i)
        {
            if (items.elements[i].itemId.get() == image_id)
            {
                *out_features = static_cast<uint32_t>(items.elements[i].features);
                return toCode(::HEIF::ErrorCode::OK);
            }
        }
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_ITEM_ID,
                            "heif_reader_get_item_features: image_id not found in metabox");
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_decoder_code_type(const heif_reader_t* reader,
                                                    uint32_t             image_id,
                                                    char                 out_type[5])
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_decoder_code_type", out_type);
    try
    {
        ::HEIF::FourCC fourcc;
        const auto err = h->reader->getDecoderCodeType(::HEIF::ImageId(image_id), fourcc);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        copyFourCC(fourcc, out_type);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_decode_dependency_count(const heif_reader_t* reader,
                                                          uint32_t             image_id,
                                                          uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_decode_dependency_count", out_count);
    try
    {
        ::HEIF::Array<::HEIF::ImageId> deps;
        const auto err = h->reader->getDecodeDependencies(::HEIF::ImageId(image_id), deps);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(deps.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_decode_dependency_at(const heif_reader_t* reader,
                                                       uint32_t             image_id,
                                                       uint32_t             index,
                                                       uint32_t*            out_dep_image_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_decode_dependency_at", out_dep_image_id);
    try
    {
        ::HEIF::Array<::HEIF::ImageId> deps;
        const auto err = h->reader->getDecodeDependencies(::HEIF::ImageId(image_id), deps);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= deps.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_decode_dependency_at: index out of range");
        }
        *out_dep_image_id = deps.elements[index].get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

namespace
{
    // Shared body for the 6 decoder-param-set accessors
    // (count/type/data x layer-specific/all-layers).
    inline ::HEIF::ErrorCode fetchDecoderConfig(ReaderHandle*                 h,
                                                uint32_t                      image_id,
                                                bool                          allLayers,
                                                ::HEIF::DecoderConfiguration& out) noexcept
    {
        try
        {
            return allLayers
                ? h->reader->getDecoderParameterSetsForAllLayers(::HEIF::ImageId(image_id), out)
                : h->reader->getDecoderParameterSets(::HEIF::ImageId(image_id), out);
        }
        catch (...)
        {
            return ::HEIF::ErrorCode::UNKNOWN_ERROR;
        }
    }
}

static heif_error_code_t decoderParamSetCount(const heif_reader_t* reader,
                                              uint32_t             image_id,
                                              uint32_t*            out_count,
                                              bool                 allLayers,
                                              const char*          fn_name)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "decoder param set count: reader handle is NULL");
    }
    if (out_count == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "decoder param set count: out_count is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h) || (h->reader == nullptr))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "decoder param set count: reader handle is not live or Reader is NULL");
    }
    (void)fn_name;
    ::HEIF::DecoderConfiguration cfg;
    const auto err = fetchDecoderConfig(h, image_id, allLayers, cfg);
    if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
    *out_count = static_cast<uint32_t>(cfg.decoderSpecificInfo.size);
    return toCode(::HEIF::ErrorCode::OK);
}

static heif_error_code_t decoderParamSetTypeAt(const heif_reader_t* reader,
                                               uint32_t             image_id,
                                               uint32_t             index,
                                               uint8_t*             out_type,
                                               bool                 allLayers)
{
    if ((reader == nullptr) || (out_type == nullptr))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "decoder param set type_at: NULL parameter");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h) || (h->reader == nullptr))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "decoder param set type_at: handle not live");
    }
    ::HEIF::DecoderConfiguration cfg;
    const auto err = fetchDecoderConfig(h, image_id, allLayers, cfg);
    if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
    if (index >= cfg.decoderSpecificInfo.size)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "decoder param set type_at: index out of range");
    }
    *out_type = static_cast<uint8_t>(cfg.decoderSpecificInfo.elements[index].decSpecInfoType);
    return toCode(::HEIF::ErrorCode::OK);
}

static heif_error_code_t decoderParamSetDataAt(const heif_reader_t* reader,
                                               uint32_t             image_id,
                                               uint32_t             index,
                                               uint8_t*             buffer,
                                               uint64_t*            size_inout,
                                               bool                 allLayers)
{
    if ((reader == nullptr) || (size_inout == nullptr))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "decoder param set data_at: NULL parameter");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h) || (h->reader == nullptr))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "decoder param set data_at: handle not live");
    }
    ::HEIF::DecoderConfiguration cfg;
    const auto err = fetchDecoderConfig(h, image_id, allLayers, cfg);
    if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
    if (index >= cfg.decoderSpecificInfo.size)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "decoder param set data_at: index out of range");
    }
    const auto& payload = cfg.decoderSpecificInfo.elements[index].decSpecInfoData;
    const uint64_t required = static_cast<uint64_t>(payload.size);
    const uint64_t cap      = *size_inout;
    *size_inout             = required;
    if (required == 0U)
    {
        return toCode(::HEIF::ErrorCode::OK);
    }
    if ((buffer == nullptr) || (cap < required))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::BUFFER_SIZE_TOO_SMALL,
                            "decoder param set data_at: caller buffer too small (size-probe)");
    }
    std::memcpy(buffer, payload.elements, static_cast<std::size_t>(required));
    return toCode(::HEIF::ErrorCode::OK);
}

heif_error_code_t heif_reader_get_decoder_param_set_count(const heif_reader_t* reader,
                                                          uint32_t image_id, uint32_t* out_count)
{ return decoderParamSetCount(reader, image_id, out_count, false, "decoder_param_set_count"); }
heif_error_code_t heif_reader_get_decoder_param_set_type_at(const heif_reader_t* reader,
                                                            uint32_t image_id, uint32_t index, uint8_t* out_type)
{ return decoderParamSetTypeAt(reader, image_id, index, out_type, false); }
heif_error_code_t heif_reader_get_decoder_param_set_data_at(const heif_reader_t* reader,
                                                            uint32_t image_id, uint32_t index,
                                                            uint8_t* buffer, uint64_t* size_inout)
{ return decoderParamSetDataAt(reader, image_id, index, buffer, size_inout, false); }
heif_error_code_t heif_reader_get_decoder_param_set_all_layers_count(const heif_reader_t* reader,
                                                                     uint32_t image_id, uint32_t* out_count)
{ return decoderParamSetCount(reader, image_id, out_count, true, "decoder_param_set_all_layers_count"); }
heif_error_code_t heif_reader_get_decoder_param_set_all_layers_type_at(const heif_reader_t* reader,
                                                                       uint32_t image_id, uint32_t index, uint8_t* out_type)
{ return decoderParamSetTypeAt(reader, image_id, index, out_type, true); }
heif_error_code_t heif_reader_get_decoder_param_set_all_layers_data_at(const heif_reader_t* reader,
                                                                       uint32_t image_id, uint32_t index,
                                                                       uint8_t* buffer, uint64_t* size_inout)
{ return decoderParamSetDataAt(reader, image_id, index, buffer, size_inout, true); }

heif_error_code_t heif_reader_get_item_protection_scheme(heif_reader_t* reader,
                                                         uint32_t       image_id,
                                                         uint8_t*       buffer,
                                                         uint64_t*      size_inout)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_item_protection_scheme", size_inout);
    try
    {
        uint64_t cap = *size_inout;
        const auto err = h->reader->getItemProtectionScheme(::HEIF::ImageId(image_id),
                                                            buffer, cap);
        *size_inout = cap;
        return toCode(err);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_compatible_brand_combo_count(const heif_reader_t* reader,
                                                               uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_compatible_brand_combo_count", out_count);
    try
    {
        ::HEIF::Array<::HEIF::Array<::HEIF::FourCC>> combos;
        const auto err = h->reader->getCompatibleBrandCombinations(combos);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(combos.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_compatible_brand_combo_brand_count(const heif_reader_t* reader,
                                                                     uint32_t             combo_index,
                                                                     uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_compatible_brand_combo_brand_count", out_count);
    try
    {
        ::HEIF::Array<::HEIF::Array<::HEIF::FourCC>> combos;
        const auto err = h->reader->getCompatibleBrandCombinations(combos);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (combo_index >= combos.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "compatible_brand_combo_brand_count: combo_index out of range");
        }
        *out_count = static_cast<uint32_t>(combos.elements[combo_index].size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_compatible_brand_combo_brand_at(const heif_reader_t* reader,
                                                                  uint32_t             combo_index,
                                                                  uint32_t             brand_index,
                                                                  char                 out_brand[5])
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_compatible_brand_combo_brand_at", out_brand);
    try
    {
        ::HEIF::Array<::HEIF::Array<::HEIF::FourCC>> combos;
        const auto err = h->reader->getCompatibleBrandCombinations(combos);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (combo_index >= combos.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "compatible_brand_combo_brand_at: combo_index out of range");
        }
        const auto& brands = combos.elements[combo_index];
        if (brand_index >= brands.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "compatible_brand_combo_brand_at: brand_index out of range");
        }
        copyFourCC(brands.elements[brand_index], out_brand);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_matrix_at(const heif_reader_t* reader,
                                            uint32_t             index,
                                            int32_t*             out_value)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_matrix_at", out_value);
    try
    {
        ::HEIF::Array<int32_t> m;
        const auto err = h->reader->getMatrix(m);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= m.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_matrix_at: index out of range (matrix has 9 entries)");
        }
        *out_value = m.elements[index];
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_clap(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                heif_clap_t*         out_clap)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_clap", out_clap);
    const uint32_t cs = out_clap->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_property_clap", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::CleanAperture src{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), src);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        heif_clap_t tmp{};
        tmp.struct_size         = static_cast<uint32_t>(sizeof(heif_clap_t));
        tmp.width_n             = src.widthN;
        tmp.width_d             = src.widthD;
        tmp.height_n            = src.heightN;
        tmp.height_d            = src.heightD;
        tmp.horizontal_offset_n = src.horizontalOffsetN;
        tmp.horizontal_offset_d = src.horizontalOffsetD;
        tmp.vertical_offset_n   = src.verticalOffsetN;
        tmp.vertical_offset_d   = src.verticalOffsetD;
        writeRule8Struct(out_clap, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_iscl(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                uint16_t*            out_w_n,
                                                uint16_t*            out_w_d,
                                                uint16_t*            out_h_n,
                                                uint16_t*            out_h_d)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_iscl", out_w_n);
    if ((out_w_d == nullptr) || (out_h_n == nullptr) || (out_h_d == nullptr))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_iscl: NULL out parameter");
    }
    try
    {
        ::HEIF::Scale s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_w_n = s.targetWidthN;
        *out_w_d = s.targetWidthD;
        *out_h_n = s.targetHeightN;
        *out_h_d = s.targetHeightD;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_lsel(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                uint16_t*            out_layer_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_lsel", out_layer_id);
    try
    {
        ::HEIF::LayerSelector s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_layer_id = s.layerId;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_tols(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                uint16_t*            out_target_ols_index)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_tols", out_target_ols_index);
    try
    {
        ::HEIF::TargetOls s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_target_ols_index = s.targetOlsIndex;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_oinf(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                uint16_t*            out_scalability_mask)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_oinf", out_scalability_mask);
    try
    {
        const auto err = ensureOinfCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_scalability_mask = h->cachedOinf.scalabilityMask;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_oinf_ptl_count(const heif_reader_t* reader,
                                                          uint32_t             property_id,
                                                          uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_oinf_ptl_count", out_count);
    try
    {
        const auto err = ensureOinfCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(h->cachedOinf.profileTierLevels.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_oinf_ptl_at(const heif_reader_t* reader,
                                                       uint32_t             property_id,
                                                       uint32_t             index,
                                                       heif_oinf_ptl_t*     out_ptl)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_oinf_ptl_at", out_ptl);
    const uint32_t cs = out_ptl->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_property_oinf_ptl_at", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureOinfCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= h->cachedOinf.profileTierLevels.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_oinf_ptl_at: index out of range");
        }
        const auto& src = h->cachedOinf.profileTierLevels[index];
        heif_oinf_ptl_t tmp{};
        tmp.struct_size                          = static_cast<uint32_t>(sizeof(heif_oinf_ptl_t));
        tmp.general_profile_space                = src.generalProfileSpace;
        tmp.general_tier_flag                    = src.generalTierFlag;
        tmp.general_profile_idc                  = src.generalProfileIdc;
        tmp.general_level_idc                    = src.generalLevelIdc;
        tmp.general_profile_compatibility_flags  = src.generalProfileCompatibilityFlags;
        tmp.general_constraint_indicator_flags   = src.generalConstraintIndicatorFlags;
        writeRule8Struct(out_ptl, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_oinf_op_count(const heif_reader_t* reader,
                                                         uint32_t             property_id,
                                                         uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_oinf_op_count", out_count);
    try
    {
        const auto err = ensureOinfCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(h->cachedOinf.operatingPoints.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_oinf_op_at(const heif_reader_t* reader,
                                                      uint32_t             property_id,
                                                      uint32_t             index,
                                                      heif_oinf_op_t*      out_op)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_oinf_op_at", out_op);
    const uint32_t cs = out_op->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_property_oinf_op_at", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureOinfCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= h->cachedOinf.operatingPoints.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_oinf_op_at: index out of range");
        }
        const auto& src = h->cachedOinf.operatingPoints[index];
        heif_oinf_op_t tmp{};
        tmp.struct_size           = static_cast<uint32_t>(sizeof(heif_oinf_op_t));
        tmp.output_layer_set_idx  = src.outputLayerSetIdx;
        tmp.max_temporal_id       = src.maxTemporalId;
        tmp.min_pic_width         = src.minPicWidth;
        tmp.min_pic_height        = src.minPicHeight;
        tmp.max_pic_width         = src.maxPicWidth;
        tmp.max_pic_height        = src.maxPicHeight;
        tmp.max_chroma_format     = src.maxChromaFormat;
        tmp.max_bit_depth_minus8  = src.maxBitDepthMinus8;
        tmp.frame_rate_info_flag  = src.frameRateInfoFlag;
        tmp.bit_rate_info_flag    = src.bitRateInfoFlag;
        tmp.avg_frame_rate        = src.avgFrameRate;
        tmp.constant_frame_rate   = src.constantFrameRate;
        tmp.max_bit_rate          = src.maxBitRate;
        tmp.avg_bit_rate          = src.avgBitRate;
        writeRule8Struct(out_op, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_oinf_op_layer_count(const heif_reader_t* reader,
                                                               uint32_t             property_id,
                                                               uint32_t             op_index,
                                                               uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_oinf_op_layer_count", out_count);
    try
    {
        const auto err = ensureOinfCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (op_index >= h->cachedOinf.operatingPoints.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_oinf_op_layer_count: op_index out of range");
        }
        *out_count = static_cast<uint32_t>(h->cachedOinf.operatingPoints[op_index].layers.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_oinf_op_layer_at(const heif_reader_t* reader,
                                                            uint32_t             property_id,
                                                            uint32_t             op_index,
                                                            uint32_t             layer_index,
                                                            heif_oinf_op_layer_t* out_layer)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_oinf_op_layer_at", out_layer);
    const uint32_t cs = out_layer->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_property_oinf_op_layer_at", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureOinfCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (op_index >= h->cachedOinf.operatingPoints.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_oinf_op_layer_at: op_index out of range");
        }
        const auto& layers = h->cachedOinf.operatingPoints[op_index].layers;
        if (layer_index >= layers.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_oinf_op_layer_at: layer_index out of range");
        }
        const auto& src = layers[layer_index];
        heif_oinf_op_layer_t tmp{};
        tmp.struct_size                = static_cast<uint32_t>(sizeof(heif_oinf_op_layer_t));
        tmp.ptl_idx                    = src.ptlIdx;
        tmp.layer_id                   = src.layerId;
        tmp.is_output_layer            = src.isOutputLayer;
        tmp.is_alternate_output_layer  = src.isAlternateOutputLayer;
        writeRule8Struct(out_layer, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_oinf_layer_count(const heif_reader_t* reader,
                                                            uint32_t             property_id,
                                                            uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_oinf_layer_count", out_count);
    try
    {
        const auto err = ensureOinfCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(h->cachedOinf.layers.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_oinf_layer_at(const heif_reader_t* reader,
                                                         uint32_t             property_id,
                                                         uint32_t             index,
                                                         uint8_t*             out_dependent_layer_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_oinf_layer_at", out_dependent_layer_id);
    try
    {
        const auto err = ensureOinfCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= h->cachedOinf.layers.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_oinf_layer_at: index out of range");
        }
        *out_dependent_layer_id = h->cachedOinf.layers[index].dependentLayerId;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_oinf_layer_dep_count(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                uint32_t             layer_index,
                                                                uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_oinf_layer_dep_count", out_count);
    try
    {
        const auto err = ensureOinfCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (layer_index >= h->cachedOinf.layers.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_oinf_layer_dep_count: layer_index out of range");
        }
        *out_count = static_cast<uint32_t>(h->cachedOinf.layers[layer_index].dependsOnLayerIds.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_oinf_layer_dep_at(const heif_reader_t* reader,
                                                             uint32_t             property_id,
                                                             uint32_t             layer_index,
                                                             uint32_t             dep_index,
                                                             uint8_t*             out_dep_layer_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_oinf_layer_dep_at", out_dep_layer_id);
    try
    {
        const auto err = ensureOinfCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (layer_index >= h->cachedOinf.layers.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_oinf_layer_dep_at: layer_index out of range");
        }
        const auto& deps = h->cachedOinf.layers[layer_index].dependsOnLayerIds;
        if (dep_index >= deps.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_oinf_layer_dep_at: dep_index out of range");
        }
        *out_dep_layer_id = deps[dep_index];
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_oinf_layer_dim_count(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                uint32_t             layer_index,
                                                                uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_oinf_layer_dim_count", out_count);
    try
    {
        const auto err = ensureOinfCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (layer_index >= h->cachedOinf.layers.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_oinf_layer_dim_count: layer_index out of range");
        }
        *out_count = static_cast<uint32_t>(h->cachedOinf.layers[layer_index].dimensionIdentifiers.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_oinf_layer_dim_at(const heif_reader_t* reader,
                                                             uint32_t             property_id,
                                                             uint32_t             layer_index,
                                                             uint32_t             dim_index,
                                                             uint8_t*             out_dimension_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_oinf_layer_dim_at", out_dimension_id);
    try
    {
        const auto err = ensureOinfCached(h, ::HEIF::PropertyId(property_id));
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (layer_index >= h->cachedOinf.layers.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_oinf_layer_dim_at: layer_index out of range");
        }
        const auto& dims = h->cachedOinf.layers[layer_index].dimensionIdentifiers;
        if (dim_index >= dims.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_oinf_layer_dim_at: dim_index out of range");
        }
        *out_dimension_id = dims[dim_index];
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_subs_count(const heif_reader_t* reader,
                                                      uint32_t             property_id,
                                                      uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_subs_count", out_count);
    try
    {
        ::HEIF::SubSampleInfo s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(s.subSamples.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_subs_at(const heif_reader_t* reader,
                                                   uint32_t             property_id,
                                                   uint32_t             index,
                                                   heif_subs_entry_t*   out_entry)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_subs_at", out_entry);
    const uint32_t cs = out_entry->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_property_subs_at", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::SubSampleInfo s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= s.subSamples.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_subs_at: index out of range");
        }
        heif_subs_entry_t tmp{};
        tmp.struct_size              = sizeof(heif_subs_entry_t);
        tmp.subsample_size           = s.subSamples[index].subsampleSize;
        tmp.subsample_priority       = s.subSamples[index].subsamplePriority;
        tmp.discardable              = s.subSamples[index].discardable;
        tmp.codec_specific_parameters = s.subSamples[index].codecSpecificParameters;
        writeRule8Struct(out_entry, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_rloc(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                uint32_t*            out_h,
                                                uint32_t*            out_v)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_rloc", out_h);
    if (out_v == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_rloc: out_v is NULL");
    }
    try
    {
        ::HEIF::RelativeLocation s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_h = s.horizontalOffset;
        *out_v = s.verticalOffset;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

}  // extern "C" -- close so the next anonymous namespace is C++ linkage

namespace
{
    // Generic Array<T> -> caller buffer with size-probe convention.
    // T must be a POD of size 1 (uint8_t / char) so the byte size
    // == element count.  Returns OK if buffer is large enough,
    // BUFFER_SIZE_TOO_SMALL on probe.  When `nul_terminate` is
    // true, an extra NUL is appended after `arr.size` bytes (caller
    // must size for arr.size + 1).
    template <typename T>
    inline heif_error_code_t copyArrayToBuffer(const ::HEIF::Array<T>& arr,
                                               void*                   buffer,
                                               uint64_t*               size_inout,
                                               bool                    nul_terminate,
                                               const char*             diag) noexcept
    {
        const uint64_t required = static_cast<uint64_t>(arr.size) + (nul_terminate ? 1U : 0U);
        const uint64_t cap      = *size_inout;
        *size_inout             = required;
        if (required == 0U)
        {
            return toCode(::HEIF::ErrorCode::OK);
        }
        if ((buffer == nullptr) || (cap < required))
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::BUFFER_SIZE_TOO_SMALL, diag);
        }
        std::memcpy(buffer, arr.elements, static_cast<std::size_t>(arr.size));
        if (nul_terminate)
        {
            static_cast<char*>(buffer)[arr.size] = '\0';
        }
        return toCode(::HEIF::ErrorCode::OK);
    }
}  // namespace

extern "C"
{

heif_error_code_t heif_reader_get_property_auxc_type(const heif_reader_t* reader,
                                                     uint32_t             property_id,
                                                     char*                buffer,
                                                     uint64_t*            size_inout)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_auxc_type", size_inout);
    try
    {
        ::HEIF::AuxiliaryType s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        return copyArrayToBuffer(s.auxType, buffer, size_inout, true,
                                 "heif_reader_get_property_auxc_type: caller buffer too small");
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_auxc_subtype(const heif_reader_t* reader,
                                                        uint32_t             property_id,
                                                        uint8_t*             buffer,
                                                        uint64_t*            size_inout)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_auxc_subtype", size_inout);
    try
    {
        ::HEIF::AuxiliaryType s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        return copyArrayToBuffer(s.subType, buffer, size_inout, false,
                                 "heif_reader_get_property_auxc_subtype: caller buffer too small");
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_rref_count(const heif_reader_t* reader,
                                                      uint32_t             property_id,
                                                      uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_rref_count", out_count);
    try
    {
        ::HEIF::RequiredReferenceTypes s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(s.referenceTypes.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_rref_at(const heif_reader_t* reader,
                                                   uint32_t             property_id,
                                                   uint32_t             index,
                                                   char                 out_type[5])
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_rref_at", out_type);
    try
    {
        ::HEIF::RequiredReferenceTypes s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= s.referenceTypes.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_property_rref_at: index out of range");
        }
        copyFourCC(s.referenceTypes.elements[index], out_type);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_udes_field(const heif_reader_t* reader,
                                                      uint32_t             property_id,
                                                      uint32_t             field_index,
                                                      char*                buffer,
                                                      uint64_t*            size_inout)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_udes_field", size_inout);
    if (field_index > 3U)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_udes_field: field_index must be 0..3");
    }
    try
    {
        ::HEIF::UserDescription s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const ::HEIF::Array<char>* fields[4] = { &s.lang, &s.name, &s.description, &s.tags };
        return copyArrayToBuffer(*fields[field_index], buffer, size_inout, true,
                                 "heif_reader_get_property_udes_field: caller buffer too small");
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_altt_field(const heif_reader_t* reader,
                                                      uint32_t             property_id,
                                                      uint32_t             field_index,
                                                      char*                buffer,
                                                      uint64_t*            size_inout)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_altt_field", size_inout);
    if (field_index > 1U)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_property_altt_field: field_index must be 0 or 1");
    }
    try
    {
        ::HEIF::AccessibilityText s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const ::HEIF::Array<char>* fields[2] = { &s.text, &s.lang };
        return copyArrayToBuffer(*fields[field_index], buffer, size_inout, true,
                                 "heif_reader_get_property_altt_field: caller buffer too small");
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_crtt(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                uint64_t*            out_time_us)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_crtt", out_time_us);
    try
    {
        ::HEIF::CreationTimeInformation s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_time_us = s.time;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_mdft(const heif_reader_t* reader,
                                                uint32_t             property_id,
                                                uint64_t*            out_time_us)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_mdft", out_time_us);
    try
    {
        ::HEIF::ModificationTimeInformation s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_time_us = s.time;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_raw_type(const heif_reader_t* reader,
                                                    uint32_t             property_id,
                                                    char                 out_type[5])
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_raw_type", out_type);
    try
    {
        ::HEIF::RawProperty s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        copyFourCC(s.type, out_type);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_property_raw_data(const heif_reader_t* reader,
                                                    uint32_t             property_id,
                                                    uint8_t*             buffer,
                                                    uint64_t*            size_inout)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_property_raw_data", size_inout);
    try
    {
        ::HEIF::RawProperty s{};
        const auto err = h->reader->getProperty(::HEIF::PropertyId(property_id), s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        return copyArrayToBuffer(s.data, buffer, size_inout, false,
                                 "heif_reader_get_property_raw_data: caller buffer too small");
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_entity_group_count(const heif_reader_t* reader,
                                                     uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_entity_group_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(
            h->cachedFileInfo.rootMetaBoxInformation.entityGroupings.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_entity_group_at(const heif_reader_t* reader,
                                                  uint32_t             index,
                                                  heif_entity_group_t* out_group)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_entity_group_at", out_group);
    const uint32_t cs = out_group->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_entity_group_at", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto& groups = h->cachedFileInfo.rootMetaBoxInformation.entityGroupings;
        if (index >= groups.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_entity_group_at: index out of range");
        }
        heif_entity_group_t tmp{};
        tmp.struct_size = static_cast<uint32_t>(sizeof(heif_entity_group_t));
        tmp.group_id    = groups.elements[index].groupId.get();
        copyFourCC(groups.elements[index].type, tmp.type);
        writeRule8Struct(out_group, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_entity_group_entity_count(const heif_reader_t* reader,
                                                            uint32_t             group_index,
                                                            uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_entity_group_entity_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto& groups = h->cachedFileInfo.rootMetaBoxInformation.entityGroupings;
        if (group_index >= groups.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "entity_group_entity_count: group_index out of range");
        }
        *out_count = static_cast<uint32_t>(groups.elements[group_index].entityIds.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_entity_group_entity_at(const heif_reader_t* reader,
                                                         uint32_t             group_index,
                                                         uint32_t             entity_index,
                                                         uint32_t*            out_entity_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_entity_group_entity_at", out_entity_id);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto& groups = h->cachedFileInfo.rootMetaBoxInformation.entityGroupings;
        if (group_index >= groups.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "entity_group_entity_at: group_index out of range");
        }
        const auto& ids = groups.elements[group_index].entityIds;
        if (entity_index >= ids.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "entity_group_entity_at: entity_index out of range");
        }
        *out_entity_id = ids.elements[entity_index];
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_group_property_count(const heif_reader_t* reader,
                                                       uint32_t             group_id,
                                                       uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_group_property_count", out_count);
    try
    {
        ::HEIF::Array<::HEIF::ItemPropertyInfo> props;
        const auto err = h->reader->getItemProperties(::HEIF::GroupId(group_id), props);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(props.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_group_property_at(const heif_reader_t* reader,
                                                    uint32_t             group_id,
                                                    uint32_t             index,
                                                    uint32_t*            out_property_id,
                                                    char                 out_type[5],
                                                    int*                 out_essential)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_group_property_at", out_property_id);
    if ((out_type == nullptr) || (out_essential == nullptr))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_group_property_at: NULL out parameter");
    }
    try
    {
        ::HEIF::Array<::HEIF::ItemPropertyInfo> props;
        const auto err = h->reader->getItemProperties(::HEIF::GroupId(group_id), props);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= props.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_group_property_at: index out of range");
        }
        const auto& info = props.elements[index];
        *out_property_id = info.index.get();
        propertyTypeToFourCC(info.type, out_type);
        *out_essential = info.essential ? 1 : 0;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

}  // extern "C"

namespace
{
    // StreamInterface subclass forwarding every virtual call to the
    // caller-provided C callback table. Owned by ReaderHandle::streamBridge
    // and freed in heif_reader_destroy AFTER the Reader is torn down (the
    // Reader holds a raw pointer to us via initialize(StreamInterface*)).
    struct StreamCallbackBridge final : public ::HEIF::StreamInterface
    {
        heif_stream_callbacks_t cb;  // copied from caller

        ::HEIF::StreamInterface::offset_t read(char* buffer, ::HEIF::StreamInterface::offset_t size) override
        {
            if (cb.read == nullptr) { return 0; }
            const int64_t n = cb.read(cb.user_data, buffer, static_cast<int64_t>(size));
            return (n < 0) ? 0 : static_cast<::HEIF::StreamInterface::offset_t>(n);
        }
        bool absoluteSeek(::HEIF::StreamInterface::offset_t offset) override
        {
            if (cb.absolute_seek == nullptr) { return false; }
            return cb.absolute_seek(cb.user_data, static_cast<int64_t>(offset)) != 0;
        }
        ::HEIF::StreamInterface::offset_t tell() override
        {
            if (cb.tell == nullptr) { return 0; }
            const int64_t n = cb.tell(cb.user_data);
            return (n < 0) ? 0 : static_cast<::HEIF::StreamInterface::offset_t>(n);
        }
        ::HEIF::StreamInterface::offset_t size() override
        {
            if (cb.size == nullptr) { return ::HEIF::StreamInterface::IndeterminateSize; }
            const int64_t n = cb.size(cb.user_data);
            return static_cast<::HEIF::StreamInterface::offset_t>(n);
        }
    };

    void destroyStreamBridge(StreamCallbackBridge* b) noexcept
    {
        delete b;
    }
}

extern "C"
{

heif_error_code_t heif_reader_initialize_stream(heif_reader_t*                 reader,
                                                const heif_stream_callbacks_t* callbacks)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_initialize_stream", callbacks);
    if (callbacks->struct_size < kRule8MinStructSize)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_initialize_stream: callbacks->struct_size has an invalid struct_size");
    }
    if ((callbacks->read == nullptr) || (callbacks->absolute_seek == nullptr) ||
        (callbacks->tell == nullptr) || (callbacks->size == nullptr))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_initialize_stream: read/seek/tell/size callbacks must all be non-NULL");
    }
    if (h->streamBridge != nullptr)
    {
        // Re-init: tear down the previous bridge first.
        destroyStreamBridge(h->streamBridge);
        h->streamBridge = nullptr;
    }
    try
    {
        auto* bridge = new StreamCallbackBridge();
        bridge->cb = *callbacks;
        const auto err = h->reader->initialize(bridge);
        if (err != ::HEIF::ErrorCode::OK)
        {
            destroyStreamBridge(bridge);
            return toCode(err);
        }
        h->streamBridge = bridge;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

}  // extern "C"

// Opaque segment-stream handle.  Caller-owned, distinct from the
// single ReaderHandle::streamBridge installed by initialize_stream;
// the segmented-playback flow needs concurrent stream sources alive
// at once (init segment + N media segments).  The struct definition
// lives here in the .cpp; the public header declares it as opaque.
struct heif_stream
{
    StreamCallbackBridge bridge;
};

namespace
{
    inline ::HEIF::StreamInterface* asStreamInterface(heif_stream_t* s) noexcept
    {
        return (s == nullptr) ? nullptr : static_cast<::HEIF::StreamInterface*>(&s->bridge);
    }
}

extern "C"
{

heif_stream_t* heif_stream_create(const heif_stream_callbacks_t* callbacks)
{
    if (callbacks == nullptr) { return nullptr; }
    if (callbacks->struct_size < kRule8MinStructSize) { return nullptr; }
    if ((callbacks->read == nullptr) || (callbacks->absolute_seek == nullptr) ||
        (callbacks->tell == nullptr) || (callbacks->size == nullptr))
    {
        return nullptr;
    }
    try
    {
        auto* s   = new heif_stream();
        s->bridge.cb = *callbacks;
        return s;
    }
    catch (...) { return nullptr; }
}

void heif_stream_destroy(heif_stream_t* stream)
{
    delete stream;  // delete on nullptr is well-defined no-op
}

heif_error_code_t heif_reader_parse_initialization_segment(heif_reader_t* reader,
                                                           heif_stream_t* stream)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_parse_initialization_segment", stream);
    try { return toCode(h->reader->parseInitializationSegment(asStreamInterface(stream))); }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_parse_segment(heif_reader_t* reader,
                                            heif_stream_t* stream,
                                            uint32_t       segment_id,
                                            uint64_t       earliest_pts_in_ts)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_parse_segment", stream);
    try
    {
        return toCode(h->reader->parseSegment(asStreamInterface(stream),
                                              ::HEIF::SegmentId(segment_id),
                                              earliest_pts_in_ts));
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_invalidate_segment(heif_reader_t* reader, uint32_t segment_id)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_invalidate_segment: reader handle is NULL");
    }
    auto* h = const_cast<ReaderHandle*>(asHandle(reader));
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_invalidate_segment: reader handle is not live");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_invalidate_segment: underlying Reader is NULL");
    }
    try { return toCode(h->reader->invalidateSegment(::HEIF::SegmentId(segment_id))); }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_parse_segment_index(heif_reader_t*       reader,
                                                  heif_stream_t*       stream,
                                                  heif_segment_info_t* out_segments,
                                                  uint32_t*            count_inout)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_parse_segment_index", count_inout);
    if (stream == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_parse_segment_index: stream is NULL");
    }
    try
    {
        ::HEIF::Array<::HEIF::SegmentInformation> segs;
        const auto err = h->reader->parseSegmentIndex(asStreamInterface(stream), segs);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const uint32_t needed = static_cast<uint32_t>(segs.size);
        // Probe path: NULL out buffer or too-small capacity
        if (out_segments == nullptr)
        {
            *count_inout = needed;
            return toCode(::HEIF::ErrorCode::OK);
        }
        if (*count_inout < needed)
        {
            *count_inout = needed;
            return toCode(::HEIF::ErrorCode::BUFFER_SIZE_TOO_SMALL);
        }
        for (uint32_t i = 0; i < needed; ++i)
        {
            const uint32_t cs = out_segments[i].struct_size;
            HEIF_C_GUARD_RULE8("heif_reader_parse_segment_index", cs, kRule8MinStructSize);
            const auto& s = segs.elements[i];
            heif_segment_info_t src{};
            src.struct_size         = cs;
            src.segment_id          = s.segmentId.get();
            src.earliest_pts_in_ts  = s.earliestPTSinTS;
            src.start_data_offset   = s.startDataOffset;
            src.reference_id        = s.referenceId;
            src.timescale           = s.timescale;
            src.duration_in_ts      = s.durationInTS;
            src.data_size           = s.dataSize;
            src.reference_type      = s.referenceType ? 1U : 0U;
            src.starts_with_sap     = s.startsWithSAP ? 1U : 0U;
            src.sap_type            = s.SAPType;
            writeRule8Struct(&out_segments[i], src, cs);
        }
        *count_inout = needed;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

}  // extern "C" -- track-surface helpers below need C++ linkage; re-open later.

namespace
{
    // Locate the cached TrackInformation entry for track_id.  Records a
    // structured guard with INVALID_SEQUENCE_ID on miss and returns null.
    // Caller must already have validated the handle and warmed the
    // FileInformation cache (caller-side consistency mirrors the existing
    // ensureFileInfoCached helper above).
    inline const ::HEIF::TrackInformation* findTrackInfo(const ReaderHandle* h,
                                                         uint32_t            track_id,
                                                         const char*         fn_name) noexcept
    {
        const auto& tracks = h->cachedFileInfo.trackInformation;
        for (std::size_t i = 0; i < tracks.size; ++i)
        {
            if (tracks.elements[i].trackId.get() == track_id)
            {
                return &tracks.elements[i];
            }
        }
        // Mirror the canonical guard idiom for unknown sequence IDs.
        // Origin Reader_ItemValidation matches the rest of the C facade.
        try
        {
            ::HEIF::ErrorInfo info;
            info.setCode   (::HEIF::ErrorCode::INVALID_SEQUENCE_ID);
            info.setOrigin (::HEIF::ErrorOrigin::Reader_ItemValidation);
            info.setMessage(fn_name);
            info.setFile   (__FILE__);
            info.setLine   (static_cast<std::uint32_t>(__LINE__));
            info.setValid  (true);
            ::HEIF::setErrorInfo(info);
        }
        catch (...)
        {
            // OOM in the structured-error slot must not promote to a crash.
        }
        return nullptr;
    }

    // Pack the three-field HEIF::CodingConstraints into a single byte:
    //   bit 0 : allRefPicsIntra
    //   bit 1 : intraPredUsed
    //   bits 7..2 : maxRefPerPic clamped to 0..63 (spec value 15 = "any"
    //               survives unchanged because it fits in 6 bits)
    inline uint8_t packCodingConstraints(const ::HEIF::CodingConstraints& cc) noexcept
    {
        const uint8_t maxRef = (cc.maxRefPerPic > 63U)
                                   ? static_cast<uint8_t>(63U)
                                   : static_cast<uint8_t>(cc.maxRefPerPic);
        return static_cast<uint8_t>((cc.allRefPicsIntra ? 0x1U : 0U) |
                                    (cc.intraPredUsed   ? 0x2U : 0U) |
                                    static_cast<unsigned>(maxRef << 2));
    }

    // Build a heif_sample_info_t from the C++ source.  Used by the
    // per-sample _at accessors and (in spirit) by anything iterating the
    // sampleProperties array.
    inline void fillSampleInfo(const ::HEIF::SampleInformation& src, heif_sample_info_t& dst) noexcept
    {
        // struct_size already set by caller; preserved here.
        dst.sample_id                    = src.sampleId.get();
        dst.sample_duration_ts           = src.sampleDurationTS;
        dst.size                         = src.size;
        dst.earliest_timestamp           = src.earliestTimestamp;
        dst.earliest_timestamp_ts        = src.earliestTimestampTS;
        dst.sample_composition_offset_ts = src.sampleCompositionOffsetTs;
        dst.sample_description_index     = src.sampleDescriptionIndex;
        dst.segment_id                   = src.segmentId.get();
        dst.sample_flags                 = src.sampleFlags.flagsAsUInt;
        dst.sample_type                  = static_cast<uint8_t>(src.sampleType);
        dst.has_clap                     = src.hasClap ? 1U : 0U;
        dst.has_auxi                     = src.hasAuxi ? 1U : 0U;
        dst.coding_constraints           = packCodingConstraints(src.codingConstraints);
        std::memcpy(dst.sample_entry_type, src.sampleEntryType.value, 5U);
        std::memset(dst._reserved, 0, sizeof(dst._reserved));
    }
}  // namespace

extern "C"
{

heif_error_code_t heif_reader_get_track_count(const heif_reader_t* reader, uint32_t* out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(h->cachedFileInfo.trackInformation.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_id_at(const heif_reader_t* reader,
                                              uint32_t             index,
                                              uint32_t*            out_track_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_id_at", out_track_id);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= h->cachedFileInfo.trackInformation.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_id_at: index >= track_count");
        }
        *out_track_id = h->cachedFileInfo.trackInformation.elements[index].trackId.get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_info(const heif_reader_t* reader,
                                             uint32_t             track_id,
                                             heif_track_info_t*   out_info)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_info", out_info);
    const uint32_t cs = out_info->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_track_info", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_info");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }

        heif_track_info_t src{};
        src.struct_size            = static_cast<uint32_t>(sizeof(heif_track_info_t));
        src.alternate_group_id     = ti->alternateGroupId;
        src.max_sample_size        = ti->maxSampleSize;
        src.track_id               = ti->trackId.get();
        src.features               = ti->features;
        src.time_scale             = ti->timeScale;
        src.frame_rate_num         = static_cast<uint32_t>(ti->frameRate.num);
        src.frame_rate_den         = static_cast<uint32_t>(ti->frameRate.den);
        src.minor_version          = ti->hasTypeInformation ? ti->type.minorVersion : 0U;
        src.has_type_information   = ti->hasTypeInformation ? 1U : 0U;
        if (ti->hasTypeInformation)
        {
            std::memcpy(src.major_brand, ti->type.majorBrand.value, 5U);
        }
        // out's struct_size is preserved by writeRule8Struct, so stamp it on src too.
        src.struct_size = cs;
        writeRule8Struct(out_info, src, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_display_width(const heif_reader_t* reader,
                                                      uint32_t             track_id,
                                                      uint32_t*            out_width)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_display_width", out_width);
    try
    {
        return toCode(h->reader->getDisplayWidth(::HEIF::SequenceId(track_id), *out_width));
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_display_height(const heif_reader_t* reader,
                                                       uint32_t             track_id,
                                                       uint32_t*            out_height)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_display_height", out_height);
    try
    {
        return toCode(h->reader->getDisplayHeight(::HEIF::SequenceId(track_id), *out_height));
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_playback_duration_secs(const heif_reader_t* reader,
                                                               uint32_t             track_id,
                                                               double*              out_secs)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_playback_duration_secs", out_secs);
    try
    {
        return toCode(h->reader->getPlaybackDurationInSecs(::HEIF::SequenceId(track_id), *out_secs));
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

//Track-scoped reference / group / brand lists
heif_error_code_t heif_reader_get_track_alternate_track_count(const heif_reader_t* reader,
                                                              uint32_t             track_id,
                                                              uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_alternate_track_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_alternate_track_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        *out_count = static_cast<uint32_t>(ti->alternateTrackIds.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_alternate_track_at(const heif_reader_t* reader,
                                                           uint32_t             track_id,
                                                           uint32_t             index,
                                                           uint32_t*            out_alt_track_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_alternate_track_at", out_alt_track_id);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_alternate_track_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (index >= ti->alternateTrackIds.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_alternate_track_at: index >= count");
        }
        *out_alt_track_id = ti->alternateTrackIds.elements[index].get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

namespace
{
    // Sum the trackIds.size across every FourCCToIds entry.  Both
    // referenceTrackIds and trackGroupIds use the same Array<FourCCToIds>
    // shape, so this helper covers both.
    inline uint32_t sumFourCCToIds(const ::HEIF::Array<::HEIF::FourCCToIds>& arr) noexcept
    {
        uint64_t total = 0;
        for (std::size_t i = 0; i < arr.size; ++i)
        {
            total += static_cast<uint64_t>(arr.elements[i].trackIds.size);
        }
        // Cap defensively at uint32 max; well-formed files are nowhere near.
        return (total > 0xFFFFFFFFULL) ? 0xFFFFFFFFU : static_cast<uint32_t>(total);
    }

    // Locate the (type, track_id) pair at the flattened index.
    // Returns false on out-of-range; on success writes through both outs.
    inline bool fourCCToIdsAt(const ::HEIF::Array<::HEIF::FourCCToIds>& arr,
                              uint32_t                                  flat_index,
                              char                                      out_type[5],
                              uint32_t&                                 out_track_id) noexcept
    {
        uint32_t cursor = 0;
        for (std::size_t i = 0; i < arr.size; ++i)
        {
            const auto& entry = arr.elements[i];
            const uint32_t span = static_cast<uint32_t>(entry.trackIds.size);
            if (flat_index < cursor + span)
            {
                const uint32_t local = flat_index - cursor;
                std::memcpy(out_type, entry.type.value, 5U);
                out_track_id = entry.trackIds.elements[local].get();
                return true;
            }
            cursor += span;
        }
        return false;
    }
}  // namespace

heif_error_code_t heif_reader_get_track_reference_count(const heif_reader_t* reader,
                                                        uint32_t             track_id,
                                                        uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_reference_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_reference_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        *out_count = sumFourCCToIds(ti->referenceTrackIds);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_reference_at(const heif_reader_t* reader,
                                                     uint32_t             track_id,
                                                     uint32_t             index,
                                                     char                 out_reference_type[5],
                                                     uint32_t*            out_referenced_track_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_reference_at", out_referenced_track_id);
    if (out_reference_type == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_track_reference_at: out_reference_type is NULL");
    }
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_reference_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (!fourCCToIdsAt(ti->referenceTrackIds, index, out_reference_type, *out_referenced_track_id))
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_reference_at: index >= count");
        }
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_group_count(const heif_reader_t* reader,
                                                    uint32_t             track_id,
                                                    uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_group_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_group_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        *out_count = sumFourCCToIds(ti->trackGroupIds);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_group_at(const heif_reader_t* reader,
                                                 uint32_t             track_id,
                                                 uint32_t             index,
                                                 char                 out_group_type[5],
                                                 uint32_t*            out_member_track_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_group_at", out_member_track_id);
    if (out_group_type == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_track_group_at: out_group_type is NULL");
    }
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_group_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (!fourCCToIdsAt(ti->trackGroupIds, index, out_group_type, *out_member_track_id))
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_group_at: index >= count");
        }
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_compatible_brand_count(const heif_reader_t* reader,
                                                               uint32_t             track_id,
                                                               uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_compatible_brand_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_compatible_brand_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        *out_count = ti->hasTypeInformation
                         ? static_cast<uint32_t>(ti->type.compatibleBrands.size)
                         : 0U;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_compatible_brand_at(const heif_reader_t* reader,
                                                            uint32_t             track_id,
                                                            uint32_t             index,
                                                            char                 out_brand[5])
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_compatible_brand_at", out_brand);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_compatible_brand_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        const auto& brands = ti->type.compatibleBrands;
        if (!ti->hasTypeInformation || (index >= brands.size))
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_compatible_brand_at: index >= count");
        }
        copyFourCC(brands.elements[index], out_brand);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

//Sample iteration
heif_error_code_t heif_reader_get_track_sample_count(const heif_reader_t* reader,
                                                     uint32_t             track_id,
                                                     uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        *out_count = static_cast<uint32_t>(ti->sampleProperties.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_at(const heif_reader_t* reader,
                                                  uint32_t             track_id,
                                                  uint32_t             index,
                                                  heif_sample_info_t*  out_info)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_at", out_info);
    const uint32_t cs = out_info->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_track_sample_at", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (index >= ti->sampleProperties.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_at: index >= sample_count");
        }
        heif_sample_info_t src{};
        src.struct_size = cs;
        fillSampleInfo(ti->sampleProperties.elements[index], src);
        writeRule8Struct(out_info, src, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_data_with_decoder_parameters(heif_reader_t* reader,
                                                                            uint32_t       track_id,
                                                                            uint32_t       sample_id,
                                                                            uint8_t*       buffer,
                                                                            uint64_t*      size_inout)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_track_sample_data_with_decoder_parameters: reader handle is NULL");
    }
    if (size_inout == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_track_sample_data_with_decoder_parameters: size_inout is NULL");
    }
    if ((buffer == nullptr) && (*size_inout != 0U))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_track_sample_data_with_decoder_parameters: buffer is NULL but *size_inout > 0 (probe call requires *size_inout == 0)");
    }
    auto* h = asHandle(reader);
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_track_sample_data_with_decoder_parameters: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_track_sample_data_with_decoder_parameters: underlying Reader is NULL");
    }
    try
    {
        return toCode(h->reader->getItemDataWithDecoderParameters(
            ::HEIF::SequenceId(track_id), ::HEIF::SequenceImageId(sample_id), buffer, *size_inout));
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_track_sample_data(heif_reader_t* reader,
                                                    uint32_t       track_id,
                                                    uint32_t       sample_id,
                                                    uint8_t*       buffer,
                                                    uint64_t*      size_inout)
{
    if (reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_track_sample_data: reader handle is NULL");
    }
    if (size_inout == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_track_sample_data: size_inout is NULL");
    }
    if ((buffer == nullptr) && (*size_inout != 0U))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_track_sample_data: buffer is NULL but *size_inout > 0 (probe call requires *size_inout == 0)");
    }
    auto* h = asHandle(reader);
    if (!isLiveHandle(h))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_track_sample_data: reader handle is not live (use-after-destroy or never created)");
    }
    if (h->reader == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::UNINITIALIZED,
                            "heif_reader_get_track_sample_data: underlying Reader is NULL");
    }
    try
    {
        // Forward with bytestreamHeaders=true (the C++ default).
        return toCode(h->reader->getItemData(::HEIF::SequenceId(track_id),
                                             ::HEIF::SequenceImageId(sample_id),
                                             buffer,
                                             *size_inout,
                                             /*bytestreamHeaders=*/true));
    }
    catch (...)
    {
        return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR);
    }
}

heif_error_code_t heif_reader_get_track_sample_decoder_code_type(const heif_reader_t* reader,
                                                                 uint32_t             track_id,
                                                                 uint32_t             sample_id,
                                                                 char                 out_type[5])
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_decoder_code_type", out_type);
    try
    {
        ::HEIF::FourCC fourcc;
        const auto err = h->reader->getDecoderCodeType(::HEIF::SequenceId(track_id),
                                                       ::HEIF::SequenceImageId(sample_id),
                                                       fourcc);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        copyFourCC(fourcc, out_type);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_item_type(const heif_reader_t* reader,
                                                         uint32_t             track_id,
                                                         uint32_t             sample_id,
                                                         char                 out_type[5])
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_item_type", out_type);
    try
    {
        ::HEIF::FourCC fourcc;
        const auto err = h->reader->getItemType(::HEIF::SequenceId(track_id),
                                                ::HEIF::SequenceImageId(sample_id),
                                                fourcc);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        copyFourCC(fourcc, out_type);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

namespace
{
    // Shared body for the 3 track-sample decoder-param-set accessors
    // (count / type_at / data_at).  Mirrors the item-side
    // fetchDecoderConfig helper above but for SequenceId+SequenceImageId.
    inline ::HEIF::ErrorCode fetchTrackSampleDecoderConfig(
        ReaderHandle*                 h,
        uint32_t                      track_id,
        uint32_t                      sample_id,
        ::HEIF::DecoderConfiguration& out) noexcept
    {
        try
        {
            return h->reader->getDecoderParameterSets(::HEIF::SequenceId(track_id),
                                                      ::HEIF::SequenceImageId(sample_id),
                                                      out);
        }
        catch (...) { return ::HEIF::ErrorCode::UNKNOWN_ERROR; }
    }
}

heif_error_code_t heif_reader_get_track_sample_decoder_param_set_count(const heif_reader_t* reader,
                                                                       uint32_t             track_id,
                                                                       uint32_t             sample_id,
                                                                       uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_decoder_param_set_count", out_count);
    ::HEIF::DecoderConfiguration cfg;
    const auto err = fetchTrackSampleDecoderConfig(h, track_id, sample_id, cfg);
    if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
    *out_count = static_cast<uint32_t>(cfg.decoderSpecificInfo.size);
    return toCode(::HEIF::ErrorCode::OK);
}

heif_error_code_t heif_reader_get_track_sample_decoder_param_set_type_at(const heif_reader_t* reader,
                                                                         uint32_t             track_id,
                                                                         uint32_t             sample_id,
                                                                         uint32_t             index,
                                                                         uint8_t*             out_type)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_decoder_param_set_type_at", out_type);
    ::HEIF::DecoderConfiguration cfg;
    const auto err = fetchTrackSampleDecoderConfig(h, track_id, sample_id, cfg);
    if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
    if (index >= cfg.decoderSpecificInfo.size)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_track_sample_decoder_param_set_type_at: index out of range");
    }
    *out_type = static_cast<uint8_t>(cfg.decoderSpecificInfo.elements[index].decSpecInfoType);
    return toCode(::HEIF::ErrorCode::OK);
}

heif_error_code_t heif_reader_get_track_sample_decoder_param_set_data_at(const heif_reader_t* reader,
                                                                         uint32_t             track_id,
                                                                         uint32_t             sample_id,
                                                                         uint32_t             index,
                                                                         uint8_t*             buffer,
                                                                         uint64_t*            size_inout)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_decoder_param_set_data_at", size_inout);
    ::HEIF::DecoderConfiguration cfg;
    const auto err = fetchTrackSampleDecoderConfig(h, track_id, sample_id, cfg);
    if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
    if (index >= cfg.decoderSpecificInfo.size)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_track_sample_decoder_param_set_data_at: index out of range");
    }
    const auto& payload     = cfg.decoderSpecificInfo.elements[index].decSpecInfoData;
    const uint64_t required = static_cast<uint64_t>(payload.size);
    const uint64_t cap      = *size_inout;
    *size_inout             = required;
    if (required == 0U) { return toCode(::HEIF::ErrorCode::OK); }
    if ((buffer == nullptr) || (cap < required))
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::BUFFER_SIZE_TOO_SMALL,
                            "heif_reader_get_track_sample_decoder_param_set_data_at: caller buffer too small (size-probe)");
    }
    std::memcpy(buffer, payload.elements, static_cast<std::size_t>(required));
    return toCode(::HEIF::ErrorCode::OK);
}

heif_error_code_t heif_reader_get_track_decoding_order_count(const heif_reader_t* reader,
                                                             uint32_t             track_id,
                                                             uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_decoding_order_count", out_count);
    try
    {
        ::HEIF::Array<::HEIF::TimestampIDPair> order;
        const auto err = h->reader->getItemsInDecodingOrder(::HEIF::SequenceId(track_id), order);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(order.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_decoding_order_at(const heif_reader_t* reader,
                                                          uint32_t             track_id,
                                                          uint32_t             index,
                                                          int64_t*             out_timestamp_ms,
                                                          uint32_t*            out_sample_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_decoding_order_at", out_timestamp_ms);
    if (out_sample_id == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_track_decoding_order_at: out_sample_id is NULL");
    }
    try
    {
        ::HEIF::Array<::HEIF::TimestampIDPair> order;
        const auto err = h->reader->getItemsInDecodingOrder(::HEIF::SequenceId(track_id), order);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= order.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_decoding_order_at: index >= count");
        }
        *out_timestamp_ms = order.elements[index].timeStamp;
        *out_sample_id    = order.elements[index].itemId.get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_decode_dependency_count(const heif_reader_t* reader,
                                                                       uint32_t             track_id,
                                                                       uint32_t             sample_id,
                                                                       uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_decode_dependency_count", out_count);
    try
    {
        ::HEIF::Array<::HEIF::SequenceImageId> deps;
        const auto err = h->reader->getDecodeDependencies(::HEIF::SequenceId(track_id),
                                                          ::HEIF::SequenceImageId(sample_id),
                                                          deps);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(deps.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_decode_dependency_at(const heif_reader_t* reader,
                                                                    uint32_t             track_id,
                                                                    uint32_t             sample_id,
                                                                    uint32_t             index,
                                                                    uint32_t*            out_dep_sample_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_decode_dependency_at", out_dep_sample_id);
    try
    {
        ::HEIF::Array<::HEIF::SequenceImageId> deps;
        const auto err = h->reader->getDecodeDependencies(::HEIF::SequenceId(track_id),
                                                          ::HEIF::SequenceImageId(sample_id),
                                                          deps);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= deps.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_decode_dependency_at: index out of range");
        }
        *out_dep_sample_id = deps.elements[index].get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

// Per-sample coded width / height (track-side overloads of getWidth /
// getHeight).  Thin pass-through; the C++ accessors validate ids.
heif_error_code_t heif_reader_get_track_sample_width(const heif_reader_t* reader,
                                                     uint32_t             track_id,
                                                     uint32_t             sample_id,
                                                     uint32_t*            out_width)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_width", out_width);
    try
    {
        return toCode(h->reader->getWidth(::HEIF::SequenceId(track_id),
                                          ::HEIF::SequenceImageId(sample_id),
                                          *out_width));
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_height(const heif_reader_t* reader,
                                                      uint32_t             track_id,
                                                      uint32_t             sample_id,
                                                      uint32_t*            out_height)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_height", out_height);
    try
    {
        return toCode(h->reader->getHeight(::HEIF::SequenceId(track_id),
                                           ::HEIF::SequenceImageId(sample_id),
                                           *out_height));
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

// Per-track 9-entry display matrix; same shape as the file-level
// _matrix_at bridge but scoped to a SequenceId.
heif_error_code_t heif_reader_get_track_matrix_at(const heif_reader_t* reader,
                                                  uint32_t             track_id,
                                                  uint32_t             index,
                                                  int32_t*             out_value)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_matrix_at", out_value);
    try
    {
        ::HEIF::Array<int32_t> m;
        const auto err = h->reader->getMatrix(::HEIF::SequenceId(track_id), m);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= m.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_matrix_at: index out of range (matrix has 9 entries)");
        }
        *out_value = m.elements[index];
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

// Master-images list of a track.  No cross-call cache here: each
// _at re-runs the accessor.  See header for the rationale.
heif_error_code_t heif_reader_get_track_master_image_count(const heif_reader_t* reader,
                                                           uint32_t             track_id,
                                                           uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_master_image_count", out_count);
    try
    {
        ::HEIF::Array<::HEIF::SequenceImageId> ids;
        const auto err = h->reader->getMasterImages(::HEIF::SequenceId(track_id), ids);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(ids.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_master_image_at(const heif_reader_t* reader,
                                                        uint32_t             track_id,
                                                        uint32_t             index,
                                                        uint32_t*            out_sample_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_master_image_at", out_sample_id);
    try
    {
        ::HEIF::Array<::HEIF::SequenceImageId> ids;
        const auto err = h->reader->getMasterImages(::HEIF::SequenceId(track_id), ids);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= ids.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_master_image_at: index >= master_image_count");
        }
        *out_sample_id = ids.elements[index].get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

// Sample-id list filtered by TrackSampleType classification
// (out_ref / out_non_ref / non_out_ref / display / samples).
heif_error_code_t heif_reader_get_track_sample_list_by_type_count(const heif_reader_t* reader,
                                                                  uint32_t             track_id,
                                                                  uint8_t              sample_type,
                                                                  uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_list_by_type_count", out_count);
    try
    {
        ::HEIF::Array<::HEIF::SequenceImageId> ids;
        const auto err = h->reader->getItemListByType(::HEIF::SequenceId(track_id),
                                                      static_cast<::HEIF::TrackSampleType>(sample_type),
                                                      ids);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(ids.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_list_by_type_at(const heif_reader_t* reader,
                                                               uint32_t             track_id,
                                                               uint8_t              sample_type,
                                                               uint32_t             index,
                                                               uint32_t*            out_sample_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_list_by_type_at", out_sample_id);
    try
    {
        ::HEIF::Array<::HEIF::SequenceImageId> ids;
        const auto err = h->reader->getItemListByType(::HEIF::SequenceId(track_id),
                                                      static_cast<::HEIF::TrackSampleType>(sample_type),
                                                      ids);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= ids.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_list_by_type_at: index >= count");
        }
        *out_sample_id = ids.elements[index].get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

// Track-side 'clap' from a sample-description entry; reuses the
// shared heif_clap_t POD and size-prefix size handshake.
heif_error_code_t heif_reader_get_track_sample_clap(const heif_reader_t* reader,
                                                    uint32_t             track_id,
                                                    uint32_t             sample_description_index,
                                                    heif_clap_t*         out_clap)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_clap", out_clap);
    const uint32_t cs = out_clap->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_track_sample_clap", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::CleanAperture src{};
        const auto err = h->reader->getProperty(::HEIF::SequenceId(track_id),
                                                sample_description_index,
                                                src);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        heif_clap_t tmp{};
        tmp.struct_size         = static_cast<uint32_t>(sizeof(heif_clap_t));
        tmp.width_n             = src.widthN;
        tmp.width_d             = src.widthD;
        tmp.height_n            = src.heightN;
        tmp.height_d            = src.heightD;
        tmp.horizontal_offset_n = src.horizontalOffsetN;
        tmp.horizontal_offset_d = src.horizontalOffsetD;
        tmp.vertical_offset_n   = src.verticalOffsetN;
        tmp.vertical_offset_d   = src.verticalOffsetD;
        writeRule8Struct(out_clap, tmp, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

// Track-side 'auxi' type / subtype.  Mirrors the item-side
// _property_auxc_* shape; the underlying C++ POD (AuxiliaryType) is
// the same, only the lookup is keyed on (SequenceId, sdi).
heif_error_code_t heif_reader_get_track_sample_auxi_type(const heif_reader_t* reader,
                                                         uint32_t             track_id,
                                                         uint32_t             sample_description_index,
                                                         char*                buffer,
                                                         uint64_t*            size_inout)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_auxi_type", size_inout);
    try
    {
        ::HEIF::AuxiliaryType s{};
        const auto err = h->reader->getProperty(::HEIF::SequenceId(track_id),
                                                sample_description_index,
                                                s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        return copyArrayToBuffer(s.auxType, buffer, size_inout, true,
                                 "heif_reader_get_track_sample_auxi_type: caller buffer too small");
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_auxi_subtype(const heif_reader_t* reader,
                                                            uint32_t             track_id,
                                                            uint32_t             sample_description_index,
                                                            uint8_t*             buffer,
                                                            uint64_t*            size_inout)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_auxi_subtype", size_inout);
    try
    {
        ::HEIF::AuxiliaryType s{};
        const auto err = h->reader->getProperty(::HEIF::SequenceId(track_id),
                                                sample_description_index,
                                                s);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        return copyArrayToBuffer(s.subType, buffer, size_inout, false,
                                 "heif_reader_get_track_sample_auxi_subtype: caller buffer too small");
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

//Item timestamps
heif_error_code_t heif_reader_get_track_item_timestamp_count(const heif_reader_t* reader,
                                                             uint32_t             track_id,
                                                             uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_item_timestamp_count", out_count);
    try
    {
        ::HEIF::Array<::HEIF::TimestampIDPair> ts;
        const auto err = h->reader->getItemTimestamps(::HEIF::SequenceId(track_id), ts);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(ts.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_item_timestamp_at(const heif_reader_t* reader,
                                                          uint32_t             track_id,
                                                          uint32_t             index,
                                                          int64_t*             out_timestamp_ms,
                                                          uint32_t*            out_sample_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_item_timestamp_at", out_timestamp_ms);
    if (out_sample_id == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_track_item_timestamp_at: out_sample_id is NULL");
    }
    try
    {
        ::HEIF::Array<::HEIF::TimestampIDPair> ts;
        const auto err = h->reader->getItemTimestamps(::HEIF::SequenceId(track_id), ts);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= ts.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_item_timestamp_at: index >= count");
        }
        *out_timestamp_ms = ts.elements[index].timeStamp;
        *out_sample_id    = ts.elements[index].itemId.get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_item_timestamp_count(const heif_reader_t* reader,
                                                       uint32_t             track_id,
                                                       uint32_t             sample_id,
                                                       uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_item_timestamp_count", out_count);
    try
    {
        ::HEIF::Array<int64_t> ts;
        const auto err = h->reader->getTimestampsOfItem(
            ::HEIF::SequenceId(track_id), ::HEIF::SequenceImageId(sample_id), ts);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(ts.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_item_timestamp_at(const heif_reader_t* reader,
                                                    uint32_t             track_id,
                                                    uint32_t             sample_id,
                                                    uint32_t             index,
                                                    int64_t*             out_timestamp_ms)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_item_timestamp_at", out_timestamp_ms);
    try
    {
        ::HEIF::Array<int64_t> ts;
        const auto err = h->reader->getTimestampsOfItem(
            ::HEIF::SequenceId(track_id), ::HEIF::SequenceImageId(sample_id), ts);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= ts.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_item_timestamp_at: index >= count");
        }
        *out_timestamp_ms = ts.elements[index];
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

//Edit list
heif_error_code_t heif_reader_get_track_edit_list_meta(const heif_reader_t* reader,
                                                       uint32_t             track_id,
                                                       int*                 out_looping,
                                                       double*              out_repetitions)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_edit_list_meta", out_looping);
    if (out_repetitions == nullptr)
    {
        HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                            "heif_reader_get_track_edit_list_meta: out_repetitions is NULL");
    }
    // Defensive zero before any cache read or branch: callers reading
    // the outputs after a non-OK return get well-defined values rather
    // than indeterminate memory.
    *out_looping     = 0;
    *out_repetitions = 0.0;
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_edit_list_meta");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if ((ti->features & ::HEIF::TrackFeatureEnum::HasEditList) == 0U)
        {
            // No edit list present.  Outputs already zeroed above; surface
            // NOT_APPLICABLE so callers can distinguish "absent" from
            // "present with looping=0, repetitions=0.0".
            return toCode(::HEIF::ErrorCode::NOT_APPLICABLE);
        }
        const int looping = ti->editList.looping ? 1 : 0;
        *out_looping      = looping;
        // C++ contract (heifcommondatatypes.h): EditList::repetitions is
        // "Only affects when looping is set true".  When looping is
        // false the underlying value is meaningless; normalise to 0.0
        // here so plain-C / FFI callers do not need the
        // `repetitions = looping ? value : 0` workaround at the
        // binding boundary.
        *out_repetitions = looping ? ti->editList.repetitions : 0.0;
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_edit_unit_count(const heif_reader_t* reader,
                                                        uint32_t             track_id,
                                                        uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_edit_unit_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_edit_unit_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        *out_count = static_cast<uint32_t>(ti->editList.editUnits.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_edit_unit_at(const heif_reader_t* reader,
                                                     uint32_t             track_id,
                                                     uint32_t             index,
                                                     heif_edit_unit_t*    out_unit)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_edit_unit_at", out_unit);
    const uint32_t cs = out_unit->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_track_edit_unit_at", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_edit_unit_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (index >= ti->editList.editUnits.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_edit_unit_at: index >= edit_unit_count");
        }
        const auto& eu = ti->editList.editUnits.elements[index];
        heif_edit_unit_t src{};
        src.struct_size              = cs;
        src.edit_type                = static_cast<uint8_t>(eu.editType);
        src.media_time_in_track_ts   = eu.mediaTimeInTrackTS;
        src.duration_in_movie_ts     = eu.durationInMovieTS;
        src.media_rate_integer       = eu.mediaRateInteger;
        src.media_rate_fraction      = eu.mediaRateFraction;
        writeRule8Struct(out_unit, src, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

//Sample groupings: 'eqiv' (SampleVisualEquivalence)
heif_error_code_t heif_reader_get_track_sample_eqiv_count(const heif_reader_t* reader,
                                                          uint32_t             track_id,
                                                          uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_eqiv_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_eqiv_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        *out_count = static_cast<uint32_t>(ti->equivalences.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_eqiv_at(const heif_reader_t* reader,
                                                       uint32_t             track_id,
                                                       uint32_t             index,
                                                       heif_sample_eqiv_t*  out_eqiv)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_eqiv_at", out_eqiv);
    const uint32_t cs = out_eqiv->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_track_sample_eqiv_at", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_eqiv_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (index >= ti->equivalences.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_eqiv_at: index >= count");
        }
        const auto& src_e = ti->equivalences.elements[index];
        heif_sample_eqiv_t src{};
        src.struct_size                    = cs;
        src.sample_group_description_index = src_e.sampleGroupDescriptionIndex;
        src.time_offset                    = src_e.timeOffset;
        src.timescale_multiplier           = src_e.timescaleMultiplier;
        writeRule8Struct(out_eqiv, src, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

//Sample groupings: 'stmi' (SampleToMetadataItem)
heif_error_code_t heif_reader_get_track_sample_stmi_count(const heif_reader_t* reader,
                                                          uint32_t             track_id,
                                                          uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_stmi_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_stmi_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        *out_count = static_cast<uint32_t>(ti->metadatas.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_stmi_at(const heif_reader_t* reader,
                                                       uint32_t             track_id,
                                                       uint32_t             index,
                                                       heif_sample_stmi_t*  out_stmi)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_stmi_at", out_stmi);
    const uint32_t cs = out_stmi->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_track_sample_stmi_at", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_stmi_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (index >= ti->metadatas.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_stmi_at: index >= count");
        }
        heif_sample_stmi_t src{};
        src.struct_size                    = cs;
        src.sample_group_description_index = ti->metadatas.elements[index].sampleGroupDescriptionIndex;
        writeRule8Struct(out_stmi, src, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_stmi_metadata_item_count(const heif_reader_t* reader,
                                                                        uint32_t             track_id,
                                                                        uint32_t             stmi_index,
                                                                        uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_stmi_metadata_item_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_stmi_metadata_item_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (stmi_index >= ti->metadatas.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_stmi_metadata_item_count: stmi_index >= count");
        }
        *out_count = static_cast<uint32_t>(ti->metadatas.elements[stmi_index].metadataItemIds.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_stmi_metadata_item_at(const heif_reader_t* reader,
                                                                     uint32_t             track_id,
                                                                     uint32_t             stmi_index,
                                                                     uint32_t             item_index,
                                                                     uint32_t*            out_metadata_item_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_stmi_metadata_item_at", out_metadata_item_id);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_stmi_metadata_item_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (stmi_index >= ti->metadatas.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_stmi_metadata_item_at: stmi_index >= count");
        }
        const auto& items = ti->metadatas.elements[stmi_index].metadataItemIds;
        if (item_index >= items.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_stmi_metadata_item_at: item_index >= count");
        }
        *out_metadata_item_id = items.elements[item_index].get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

//Sample groupings: 'refs' (DirectReferenceSamples)
heif_error_code_t heif_reader_get_track_sample_refs_count(const heif_reader_t* reader,
                                                          uint32_t             track_id,
                                                          uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_refs_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_refs_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        *out_count = static_cast<uint32_t>(ti->referenceSamples.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_refs_at(const heif_reader_t* reader,
                                                       uint32_t             track_id,
                                                       uint32_t             index,
                                                       heif_sample_refs_t*  out_refs)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_refs_at", out_refs);
    const uint32_t cs = out_refs->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_track_sample_refs_at", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_refs_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (index >= ti->referenceSamples.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_refs_at: index >= count");
        }
        const auto& src_r = ti->referenceSamples.elements[index];
        heif_sample_refs_t src{};
        src.struct_size                    = cs;
        src.sample_group_description_index = src_r.sampleGroupDescriptionIndex;
        src.sample_id                      = src_r.sampleId;
        writeRule8Struct(out_refs, src, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_refs_reference_item_count(const heif_reader_t* reader,
                                                                         uint32_t             track_id,
                                                                         uint32_t             refs_index,
                                                                         uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_refs_reference_item_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_refs_reference_item_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (refs_index >= ti->referenceSamples.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_refs_reference_item_count: refs_index >= count");
        }
        *out_count = static_cast<uint32_t>(ti->referenceSamples.elements[refs_index].referenceItemIds.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_refs_reference_item_at(const heif_reader_t* reader,
                                                                      uint32_t             track_id,
                                                                      uint32_t             refs_index,
                                                                      uint32_t             item_index,
                                                                      uint32_t*            out_reference_item_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_refs_reference_item_at", out_reference_item_id);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_refs_reference_item_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (refs_index >= ti->referenceSamples.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_refs_reference_item_at: refs_index >= count");
        }
        const auto& items = ti->referenceSamples.elements[refs_index].referenceItemIds;
        if (item_index >= items.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_refs_reference_item_at: item_index >= count");
        }
        *out_reference_item_id = items.elements[item_index].get();
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

//Sample groupings: 'trif' (RectangularRegion)
heif_error_code_t heif_reader_get_track_sample_trif_count(const heif_reader_t* reader,
                                                          uint32_t             track_id,
                                                          uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_trif_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_trif_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        *out_count = static_cast<uint32_t>(ti->rectangularRegions.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_trif_at(const heif_reader_t* reader,
                                                       uint32_t             track_id,
                                                       uint32_t             index,
                                                       heif_sample_trif_t*  out_trif)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_trif_at", out_trif);
    const uint32_t cs = out_trif->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_track_sample_trif_at", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_trif_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (index >= ti->rectangularRegions.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_trif_at: index >= count");
        }
        const auto& src_r = ti->rectangularRegions.elements[index];
        heif_sample_trif_t src{};
        src.struct_size                    = cs;
        src.sample_group_description_index = src_r.sampleGroupDescriptionIndex;
        src.group_id                       = src_r.trif.groupId;
        src.independent_idc                = src_r.trif.independentIdc;
        src.horizontal_offset              = src_r.trif.horizontalOffset;
        src.vertical_offset                = src_r.trif.verticalOffset;
        src.region_width                   = src_r.trif.regionWidth;
        src.region_height                  = src_r.trif.regionHeight;
        src.rect_region_flag               = src_r.trif.rectRegionFlag ? 1U : 0U;
        src.full_picture                   = src_r.trif.fullPicture    ? 1U : 0U;
        src.filtering_disabled             = src_r.trif.filteringDisabled ? 1U : 0U;
        writeRule8Struct(out_trif, src, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_trif_dependency_count(const heif_reader_t* reader,
                                                                     uint32_t             track_id,
                                                                     uint32_t             trif_index,
                                                                     uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_trif_dependency_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_trif_dependency_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (trif_index >= ti->rectangularRegions.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_trif_dependency_count: trif_index >= count");
        }
        *out_count = static_cast<uint32_t>(
            ti->rectangularRegions.elements[trif_index].trif.dependencyRectRegionGroupIds.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_trif_dependency_at(const heif_reader_t* reader,
                                                                  uint32_t             track_id,
                                                                  uint32_t             trif_index,
                                                                  uint32_t             dep_index,
                                                                  uint16_t*            out_dependency_group_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_trif_dependency_at", out_dependency_group_id);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_trif_dependency_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (trif_index >= ti->rectangularRegions.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_trif_dependency_at: trif_index >= count");
        }
        const auto& deps = ti->rectangularRegions.elements[trif_index].trif.dependencyRectRegionGroupIds;
        if (dep_index >= deps.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_trif_dependency_at: dep_index >= count");
        }
        *out_dependency_group_id = deps.elements[dep_index];
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

//Sample groupings: 'spor' (VvcSubpicOrder)
heif_error_code_t heif_reader_get_track_sample_spor_count(const heif_reader_t* reader,
                                                          uint32_t             track_id,
                                                          uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_spor_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_spor_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        *out_count = static_cast<uint32_t>(ti->vvcSubpicOrders.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_spor_at(const heif_reader_t* reader,
                                                       uint32_t             track_id,
                                                       uint32_t             index,
                                                       heif_sample_spor_t*  out_spor)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_spor_at", out_spor);
    const uint32_t cs = out_spor->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_track_sample_spor_at", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_spor_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (index >= ti->vvcSubpicOrders.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_spor_at: index >= count");
        }
        const auto& src_s = ti->vvcSubpicOrders.elements[index];
        heif_sample_spor_t src{};
        src.struct_size                    = cs;
        src.sample_group_description_index = src_s.sampleGroupDescriptionIndex;
        src.subpic_id_bit_pos              = src_s.subpicIdBitPos;
        src.subpic_id_info_flag            = src_s.subpicIdInfoFlag ? 1U : 0U;
        src.subpicd_len_minus1             = src_s.subpicdLenMinus1;
        src.start_code_emul_flag           = src_s.startCodeEmulFlag ? 1U : 0U;
        src.pps_sps_subpic_id_flag         = src_s.ppsSpsSubpicIdFlag ? 1U : 0U;
        src.pps_id                         = src_s.ppsId;
        src.sps_id                         = src_s.spsId;
        writeRule8Struct(out_spor, src, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_spor_subp_track_ref_idx_count(const heif_reader_t* reader,
                                                                             uint32_t             track_id,
                                                                             uint32_t             spor_index,
                                                                             uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_spor_subp_track_ref_idx_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id,
                                       "heif_reader_get_track_sample_spor_subp_track_ref_idx_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (spor_index >= ti->vvcSubpicOrders.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_spor_subp_track_ref_idx_count: spor_index >= count");
        }
        *out_count = static_cast<uint32_t>(ti->vvcSubpicOrders.elements[spor_index].subpTrackRefIdx.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_spor_subp_track_ref_idx_at(const heif_reader_t* reader,
                                                                          uint32_t             track_id,
                                                                          uint32_t             spor_index,
                                                                          uint32_t             ref_idx,
                                                                          uint16_t*            out_subp_track_ref_idx)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_spor_subp_track_ref_idx_at", out_subp_track_ref_idx);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id,
                                       "heif_reader_get_track_sample_spor_subp_track_ref_idx_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (spor_index >= ti->vvcSubpicOrders.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_spor_subp_track_ref_idx_at: spor_index >= count");
        }
        const auto& arr = ti->vvcSubpicOrders.elements[spor_index].subpTrackRefIdx;
        if (ref_idx >= arr.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_spor_subp_track_ref_idx_at: ref_idx >= count");
        }
        *out_subp_track_ref_idx = arr.elements[ref_idx];
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

//Sample groupings: 'sulm' (VvcSubpicLayoutMap)
heif_error_code_t heif_reader_get_track_sample_sulm_count(const heif_reader_t* reader,
                                                          uint32_t             track_id,
                                                          uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_sulm_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_sulm_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        *out_count = static_cast<uint32_t>(ti->vvcSubpicLayoutMaps.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_sulm_at(const heif_reader_t* reader,
                                                       uint32_t             track_id,
                                                       uint32_t             index,
                                                       heif_sample_sulm_t*  out_sulm)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_sulm_at", out_sulm);
    const uint32_t cs = out_sulm->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_track_sample_sulm_at", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_sulm_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (index >= ti->vvcSubpicLayoutMaps.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_sulm_at: index >= count");
        }
        const auto& src_s = ti->vvcSubpicLayoutMaps.elements[index];
        heif_sample_sulm_t src{};
        src.struct_size                    = cs;
        src.sample_group_description_index = src_s.sampleGroupDescriptionIndex;
        std::memcpy(src.group_id_info_4cc, src_s.groupIdInfo4cc.value, 5U);
        writeRule8Struct(out_sulm, src, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_sulm_group_id_count(const heif_reader_t* reader,
                                                                   uint32_t             track_id,
                                                                   uint32_t             sulm_index,
                                                                   uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_sulm_group_id_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_sulm_group_id_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (sulm_index >= ti->vvcSubpicLayoutMaps.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_sulm_group_id_count: sulm_index >= count");
        }
        *out_count = static_cast<uint32_t>(ti->vvcSubpicLayoutMaps.elements[sulm_index].groupIds.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_sulm_group_id_at(const heif_reader_t* reader,
                                                                uint32_t             track_id,
                                                                uint32_t             sulm_index,
                                                                uint32_t             group_idx,
                                                                uint16_t*            out_group_id)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_sulm_group_id_at", out_group_id);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_sulm_group_id_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (sulm_index >= ti->vvcSubpicLayoutMaps.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_sulm_group_id_at: sulm_index >= count");
        }
        const auto& arr = ti->vvcSubpicLayoutMaps.elements[sulm_index].groupIds;
        if (group_idx >= arr.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_sulm_group_id_at: group_idx >= count");
        }
        *out_group_id = arr.elements[group_idx];
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

//Sample groupings: 'minp' (VvcMixedNalUnitTypePic)
heif_error_code_t heif_reader_get_track_sample_minp_count(const heif_reader_t* reader,
                                                          uint32_t             track_id,
                                                          uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_minp_count", out_count);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_minp_count");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        *out_count = static_cast<uint32_t>(ti->vvcMixedNalUnitTypePics.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_minp_at(const heif_reader_t* reader,
                                                       uint32_t             track_id,
                                                       uint32_t             index,
                                                       heif_sample_minp_t*  out_minp)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_minp_at", out_minp);
    const uint32_t cs = out_minp->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_track_sample_minp_at", cs, kRule8MinStructSize);
    try
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, "heif_reader_get_track_sample_minp_at");
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (index >= ti->vvcMixedNalUnitTypePics.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_track_sample_minp_at: index >= count");
        }
        const auto& src_m = ti->vvcMixedNalUnitTypePics.elements[index];
        heif_sample_minp_t src{};
        src.struct_size                       = cs;
        src.sample_group_description_index    = src_m.sampleGroupDescriptionIndex;
        src.pps_mix_nalu_types_in_pic_bit_pos = src_m.ppsMixNaluTypesInPicBitPos;
        src.pps_id                            = src_m.ppsId;
        writeRule8Struct(out_minp, src, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

namespace
{
    // Choose mixSubpTrackIdx1 vs idx2 with a single helper.
    inline const ::HEIF::Array<uint16_t>* minpMixArr(const ::HEIF::VvcMixedNalUnitTypePic& src,
                                                     int                                   which) noexcept
    {
        return (which == 1) ? &src.mixSubpTrackIdx1 : &src.mixSubpTrackIdx2;
    }

    inline heif_error_code_t minpMixCount(ReaderHandle*       h,
                                          uint32_t            track_id,
                                          uint32_t            minp_index,
                                          int                 which,
                                          const char*         fn_name,
                                          uint32_t*           out_count)
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, fn_name);
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (minp_index >= ti->vvcMixedNalUnitTypePics.size)
        {
            return recordGuard(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                               fn_name, __FILE__, __LINE__);
        }
        const auto* arr = minpMixArr(ti->vvcMixedNalUnitTypePics.elements[minp_index], which);
        *out_count = static_cast<uint32_t>(arr->size);
        return toCode(::HEIF::ErrorCode::OK);
    }

    inline heif_error_code_t minpMixAt(ReaderHandle* h,
                                       uint32_t      track_id,
                                       uint32_t      minp_index,
                                       uint32_t      idx,
                                       int           which,
                                       const char*   fn_name,
                                       uint16_t*     out_value)
    {
        const auto err = ensureFileInfoCached(h);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        const auto* ti = findTrackInfo(h, track_id, fn_name);
        if (ti == nullptr) { return toCode(::HEIF::ErrorCode::INVALID_SEQUENCE_ID); }
        if (minp_index >= ti->vvcMixedNalUnitTypePics.size)
        {
            return recordGuard(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                               fn_name, __FILE__, __LINE__);
        }
        const auto* arr = minpMixArr(ti->vvcMixedNalUnitTypePics.elements[minp_index], which);
        if (idx >= arr->size)
        {
            return recordGuard(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                               fn_name, __FILE__, __LINE__);
        }
        *out_value = arr->elements[idx];
        return toCode(::HEIF::ErrorCode::OK);
    }
}  // namespace

heif_error_code_t heif_reader_get_track_sample_minp_mix_subp_track_idx1_count(const heif_reader_t* reader,
                                                                              uint32_t             track_id,
                                                                              uint32_t             minp_index,
                                                                              uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_minp_mix_subp_track_idx1_count", out_count);
    try { return minpMixCount(h, track_id, minp_index, 1,
                              "heif_reader_get_track_sample_minp_mix_subp_track_idx1_count", out_count); }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_minp_mix_subp_track_idx1_at(const heif_reader_t* reader,
                                                                           uint32_t             track_id,
                                                                           uint32_t             minp_index,
                                                                           uint32_t             idx1_index,
                                                                           uint16_t*            out_value)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_minp_mix_subp_track_idx1_at", out_value);
    try { return minpMixAt(h, track_id, minp_index, idx1_index, 1,
                           "heif_reader_get_track_sample_minp_mix_subp_track_idx1_at", out_value); }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_minp_mix_subp_track_idx2_count(const heif_reader_t* reader,
                                                                              uint32_t             track_id,
                                                                              uint32_t             minp_index,
                                                                              uint32_t*            out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_minp_mix_subp_track_idx2_count", out_count);
    try { return minpMixCount(h, track_id, minp_index, 2,
                              "heif_reader_get_track_sample_minp_mix_subp_track_idx2_count", out_count); }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_track_sample_minp_mix_subp_track_idx2_at(const heif_reader_t* reader,
                                                                           uint32_t             track_id,
                                                                           uint32_t             minp_index,
                                                                           uint32_t             idx2_index,
                                                                           uint16_t*            out_value)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_track_sample_minp_mix_subp_track_idx2_at", out_value);
    try { return minpMixAt(h, track_id, minp_index, idx2_index, 2,
                           "heif_reader_get_track_sample_minp_mix_subp_track_idx2_at", out_value); }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

//Segment index
heif_error_code_t heif_reader_get_segment_index_count(heif_reader_t* reader, uint32_t* out_count)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_segment_index_count", out_count);
    try
    {
        ::HEIF::Array<::HEIF::SegmentInformation> segs;
        const auto err = h->reader->getSegmentIndex(segs);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        *out_count = static_cast<uint32_t>(segs.size);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

heif_error_code_t heif_reader_get_segment_index_at(heif_reader_t*       reader,
                                                   uint32_t             index,
                                                   heif_segment_info_t* out_segment)
{
    HEIF_C_GUARD_PROPERTY("heif_reader_get_segment_index_at", out_segment);
    const uint32_t cs = out_segment->struct_size;
    HEIF_C_GUARD_RULE8("heif_reader_get_segment_index_at", cs, kRule8MinStructSize);
    try
    {
        ::HEIF::Array<::HEIF::SegmentInformation> segs;
        const auto err = h->reader->getSegmentIndex(segs);
        if (err != ::HEIF::ErrorCode::OK) { return toCode(err); }
        if (index >= segs.size)
        {
            HEIF_C_GUARD_RETURN(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                                "heif_reader_get_segment_index_at: index >= count");
        }
        const auto& s = segs.elements[index];
        heif_segment_info_t src{};
        src.struct_size         = cs;
        src.segment_id          = s.segmentId.get();
        src.earliest_pts_in_ts  = s.earliestPTSinTS;
        src.start_data_offset   = s.startDataOffset;
        src.reference_id        = s.referenceId;
        src.timescale           = s.timescale;
        src.duration_in_ts      = s.durationInTS;
        src.data_size           = s.dataSize;
        src.reference_type      = s.referenceType ? 1U : 0U;
        src.starts_with_sap     = s.startsWithSAP ? 1U : 0U;
        src.sap_type            = s.SAPType;
        writeRule8Struct(out_segment, src, cs);
        return toCode(::HEIF::ErrorCode::OK);
    }
    catch (...) { return toCode(::HEIF::ErrorCode::UNKNOWN_ERROR); }
}

}  // extern "C"
