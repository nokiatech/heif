/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 */

#include "heiferrorinfo.h"

#include <atomic>
#include <cstdio>
#include <cstring>

namespace HEIF
{
    //  ErrorInfo -- public layout does not contain std::string /
    //  std::exception. Heap storage for the message strings is owned by
    //  the library: setMessage / setCauseWhat allocate via plain new[]
    //  inside the shared library, and ~ErrorInfo deallocates via delete[]
    //  also inside the shared library, so a mixed-CRT MSVC consumer
    //  cannot free across CRT boundaries.
    namespace
    {
        // Local helper: deep-copy `src` into a freshly-allocated NUL-
        // terminated buffer, or return nullptr for null/empty input.
        // We store nullptr for the empty case so the common "no cause"
        // path costs zero allocations.
        char* dupOrNull(const char* src)
        {
            if ((src == nullptr) || (src[0] == '\0'))
            {
                return nullptr;
            }
            const std::size_t n = std::strlen(src);
            char* out = new char[n + 1U];
            std::memcpy(out, src, n + 1U);  // includes terminating NUL
            return out;
        }
    }  // namespace

    ErrorInfo::ErrorInfo() noexcept = default;

    ErrorInfo::~ErrorInfo()
    {
        delete[] m_message;
        delete[] m_causeWhat;
    }

    ErrorInfo::ErrorInfo(const ErrorInfo& other)
        : m_code(other.m_code)
        , m_origin(other.m_origin)
        , m_file(other.m_file)
        , m_line(other.m_line)
        , m_valid(other.m_valid)
        , m_message(dupOrNull(other.m_message))
        , m_causeWhat(dupOrNull(other.m_causeWhat))
    {
    }

    ErrorInfo::ErrorInfo(ErrorInfo&& other) noexcept
        : m_code(other.m_code)
        , m_origin(other.m_origin)
        , m_file(other.m_file)
        , m_line(other.m_line)
        , m_valid(other.m_valid)
        , m_message(other.m_message)
        , m_causeWhat(other.m_causeWhat)
    {
        other.m_message   = nullptr;
        other.m_causeWhat = nullptr;
        other.m_valid     = false;
    }

    ErrorInfo& ErrorInfo::operator=(const ErrorInfo& other)
    {
        if (this != &other)
        {
            // Copy-and-swap-light: allocate new buffers first so that an
            // allocation failure leaves *this unchanged (basic exception
            // safety with strong guarantee on the strings).
            char* newMsg   = dupOrNull(other.m_message);
            char* newCause = dupOrNull(other.m_causeWhat);
            delete[] m_message;
            delete[] m_causeWhat;
            m_code      = other.m_code;
            m_origin    = other.m_origin;
            m_file      = other.m_file;
            m_line      = other.m_line;
            m_valid     = other.m_valid;
            m_message   = newMsg;
            m_causeWhat = newCause;
        }
        return *this;
    }

    ErrorInfo& ErrorInfo::operator=(ErrorInfo&& other) noexcept
    {
        if (this != &other)
        {
            delete[] m_message;
            delete[] m_causeWhat;
            m_code            = other.m_code;
            m_origin          = other.m_origin;
            m_file            = other.m_file;
            m_line            = other.m_line;
            m_valid           = other.m_valid;
            m_message         = other.m_message;
            m_causeWhat       = other.m_causeWhat;
            other.m_message   = nullptr;
            other.m_causeWhat = nullptr;
            other.m_valid     = false;
        }
        return *this;
    }

    const char* ErrorInfo::message() const noexcept
    {
        return (m_message != nullptr) ? m_message : "";
    }

    const char* ErrorInfo::causeWhat() const noexcept
    {
        return (m_causeWhat != nullptr) ? m_causeWhat : "";
    }

    void ErrorInfo::setMessage(const char* msg)
    {
        char* newMsg = dupOrNull(msg);
        delete[] m_message;
        m_message = newMsg;
    }

    void ErrorInfo::setCauseWhat(const char* what)
    {
        char* newCause = dupOrNull(what);
        delete[] m_causeWhat;
        m_causeWhat = newCause;
    }

    namespace
    {
        // Per-thread slot. `thread_local` guarantees that a failure
        // recorded by thread A cannot be observed from thread B; without
        // this, concurrent decoder/encoder usage by an embedder would
        // produce wildly misleading diagnostics (a Java exception thrown
        // from one worker could carry breadcrumbs from another worker's
        // unrelated parse error).
        //
        // Default-constructed (valid() == false) so an unconditional
        // read before any error has been reported returns a well-defined
        // "no error here" record rather than reading uninitialised
        // memory.
        thread_local ErrorInfo tLastError{};

        // Optional sink callback (added in 3.8.0). Process-wide, NOT
        // per-thread: sinks are typically attached to a single embedder
        // logger which already serialises its own writes. Using
        // std::atomic gives us a torn-write-free publish without paying
        // for a mutex on the (overwhelmingly common) "no sink installed"
        // fast path - one relaxed atomic load per setErrorInfo.
        //
        // We store sink + user as two separate atomics rather than one
        // packed struct because (a) a 16-byte CAS is not portable across
        // 32-bit targets the library still supports, and (b) install is
        // rare while dispatch is hot, so we publish user first and sink
        // second (release order on store), and load sink first and user
        // second (acquire order on load). A racing observer that sees
        // the new sink with the OLD user is impossible because the
        // sink-store happens-after the user-store.
        std::atomic<ErrorSink> gSink{nullptr};
        std::atomic<void*>     gSinkUser{nullptr};
    }  // namespace

    void setErrorInfo(const ErrorInfo& info)
    {
        tLastError = info;
        // Notify the optional sink AFTER the slot has been populated so a
        // sink that itself calls getErrorInfo() observes the same record
        // it was just notified about.
        if (auto sink = gSink.load(std::memory_order_acquire))
        {
            void* user = gSinkUser.load(std::memory_order_acquire);
            sink(info, user);
        }
    }

    void clearErrorInfo()
    {
        tLastError = ErrorInfo{};
    }

    const ErrorInfo& getErrorInfo()
    {
        return tLastError;
    }

    bool hasErrorInfo()
    {
        return tLastError.valid();
    }

    const char* errorOriginName(ErrorOrigin origin)
    {
        switch (origin)
        {
            case ErrorOrigin::Unknown:               return "Unknown";

            case ErrorOrigin::Reader_FileOpen:       return "Reader_FileOpen";
            case ErrorOrigin::Reader_FileRead:       return "Reader_FileRead";
            case ErrorOrigin::Reader_BoxParse:       return "Reader_BoxParse";
            case ErrorOrigin::Reader_VvcParser:      return "Reader_VvcParser";
            case ErrorOrigin::Reader_HevcParser:     return "Reader_HevcParser";
            case ErrorOrigin::Reader_AvcParser:      return "Reader_AvcParser";
            case ErrorOrigin::Reader_DecoderConfig:  return "Reader_DecoderConfig";
            case ErrorOrigin::Reader_ItemValidation: return "Reader_ItemValidation";
            case ErrorOrigin::Reader_Mini:           return "Reader_Mini";
            case ErrorOrigin::Reader_Grid:           return "Reader_Grid";
            case ErrorOrigin::Reader_Derivation:     return "Reader_Derivation";

            case ErrorOrigin::Writer_FileIO:         return "Writer_FileIO";
            case ErrorOrigin::Writer_BoxBuild:       return "Writer_BoxBuild";
            case ErrorOrigin::Writer_DecoderConfig:  return "Writer_DecoderConfig";
            case ErrorOrigin::Writer_Mini:           return "Writer_Mini";
            case ErrorOrigin::Writer_State:          return "Writer_State";
            case ErrorOrigin::Writer_Validation:     return "Writer_Validation";

            case ErrorOrigin::Parser_Vvc:            return "Parser_Vvc";
            case ErrorOrigin::Parser_Jpeg:           return "Parser_Jpeg";
            case ErrorOrigin::Parser_VvcSps:         return "Parser_VvcSps";
            case ErrorOrigin::Parser_VvcPps:         return "Parser_VvcPps";
            case ErrorOrigin::Parser_VvcVps:         return "Parser_VvcVps";
            case ErrorOrigin::Parser_VvcOpi:         return "Parser_VvcOpi";
            case ErrorOrigin::Parser_VvcRbsp:        return "Parser_VvcRbsp";

            case ErrorOrigin::Common_BitstreamRead:  return "Common_BitstreamRead";
            case ErrorOrigin::Internal_Invariant:    return "Internal_Invariant";
        }
        return "Unknown";
    }

    const char* errorCodeName(ErrorCode code)
    {
        switch (code)
        {
            case ErrorCode::OK:                              return "OK";
            case ErrorCode::ALLOCATOR_ALREADY_SET:           return "ALLOCATOR_ALREADY_SET";
            case ErrorCode::ALREADY_INITIALIZED:             return "ALREADY_INITIALIZED";
            case ErrorCode::BRANDS_NOT_SET:                  return "BRANDS_NOT_SET";
            case ErrorCode::BUFFER_SIZE_TOO_SMALL:           return "BUFFER_SIZE_TOO_SMALL";
            case ErrorCode::CONTENT_TYPE_NOT_SET:            return "CONTENT_TYPE_NOT_SET";
            case ErrorCode::DECODER_CONFIGURATION_ERROR:     return "DECODER_CONFIGURATION_ERROR";
            case ErrorCode::FILE_HEADER_ERROR:               return "FILE_HEADER_ERROR";
            case ErrorCode::FILE_OPEN_ERROR:                 return "FILE_OPEN_ERROR";
            case ErrorCode::FILE_READ_ERROR:                 return "FILE_READ_ERROR";
            case ErrorCode::FTYP_ALREADY_WRITTEN:            return "FTYP_ALREADY_WRITTEN";
            case ErrorCode::HIDDEN_PRIMARY_ITEM:             return "HIDDEN_PRIMARY_ITEM";
            case ErrorCode::INVALID_FUNCTION_PARAMETER:      return "INVALID_FUNCTION_PARAMETER";
            case ErrorCode::INVALID_GROUP_ID:                return "INVALID_GROUP_ID";
            case ErrorCode::INVALID_ITEM_ID:                 return "INVALID_ITEM_ID";
            case ErrorCode::INVALID_MEDIADATA_ID:            return "INVALID_MEDIADATA_ID";
            case ErrorCode::INVALID_METADATAITEM_ID:         return "INVALID_METADATAITEM_ID";
            case ErrorCode::INVALID_MEDIA_FORMAT:            return "INVALID_MEDIA_FORMAT";
            case ErrorCode::INVALID_PROPERTY_INDEX:          return "INVALID_PROPERTY_INDEX";
            case ErrorCode::INVALID_REFERENCE_COUNT:         return "INVALID_REFERENCE_COUNT";
            case ErrorCode::INVALID_SAMPLE_DESCRIPTION_INDEX:return "INVALID_SAMPLE_DESCRIPTION_INDEX";
            case ErrorCode::INVALID_SEGMENT:                 return "INVALID_SEGMENT";
            case ErrorCode::INVALID_SEQUENCE_ID:             return "INVALID_SEQUENCE_ID";
            case ErrorCode::INVALID_DECODER_CONFIG_ID:       return "INVALID_DECODER_CONFIG_ID";
            case ErrorCode::INVALID_SEQUENCE_IMAGE_ID:       return "INVALID_SEQUENCE_IMAGE_ID";
            case ErrorCode::MEDIA_PARSING_ERROR:             return "MEDIA_PARSING_ERROR";
            case ErrorCode::MEMORY_TOO_SMALL_BUFFER:         return "MEMORY_TOO_SMALL_BUFFER";
            case ErrorCode::NOT_APPLICABLE:                  return "NOT_APPLICABLE";
            case ErrorCode::NO_MEDIA:                        return "NO_MEDIA";
            case ErrorCode::PRIMARY_ITEM_NOT_SET:            return "PRIMARY_ITEM_NOT_SET";
            case ErrorCode::PROTECTED_ITEM:                  return "PROTECTED_ITEM";
            case ErrorCode::UNINITIALIZED:                   return "UNINITIALIZED";
            case ErrorCode::UNPROTECTED_ITEM:                return "UNPROTECTED_ITEM";
            case ErrorCode::UNSUPPORTED_CODE_TYPE:           return "UNSUPPORTED_CODE_TYPE";
            case ErrorCode::TMI:                             return "TMI";
            case ErrorCode::MISSING_PROPERTIES:              return "MISSING_PROPERTIES";
            case ErrorCode::MULTIPLE_GAINMAPS:               return "MULTIPLE_GAINMAPS";
            case ErrorCode::INVALID_EXIF_ORIENTATION_VALUE:  return "INVALID_EXIF_ORIENTATION_VALUE";
            case ErrorCode::BYTESTREAM_READ_ERROR:           return "BYTESTREAM_READ_ERROR";
            case ErrorCode::VVC_DISABLED:                    return "VVC_DISABLED";
            case ErrorCode::UNKNOWN_ERROR:                   return "UNKNOWN_ERROR";
        }
        return "ErrorCode(?)";
    }

    // ---------------------------------------------------------------------
    //  Formatter / Sink (3.8.0)
    // ---------------------------------------------------------------------

    std::size_t formatErrorInfo(const ErrorInfo& info, char* buf, std::size_t n)
    {
        // Defensive: an embedder calling with (nullptr, 0) is asking
        // "how many bytes do I need?" - mirror snprintf's contract.
        // We cannot pass a null buffer to std::snprintf with n>0, so we
        // route through a throwaway one-byte sink in that case.
        char  sentinel = 0;
        char* dst      = (buf != nullptr) ? buf : &sentinel;
        const std::size_t cap = (buf != nullptr) ? n : 1;

        if (!info.valid())
        {
            // Stable sentinel for "slot is empty" so embedders writing
            //     fmt::format("heif: {}", formatErrorInfo(info))
            // never produce confusingly-blank log lines.
            const int written = std::snprintf(dst, cap, "<no error>");
            return (written < 0) ? 0u : static_cast<std::size_t>(written);
        }

        const char* originName = errorOriginName(info.origin());
        const char* codeName   = errorCodeName(info.code());
        const char* file       = info.file();  // never null per contract
        const int   codeInt    = static_cast<int>(info.code());
        const char* msg        = info.message();    // never null
        const char* cause      = info.causeWhat();  // never null; "" when none

        int written;
        if (cause[0] == '\0')
        {
            written = std::snprintf(dst, cap, "[%s@%s:%d] %s(%d): %s",
                                    originName, file, info.line(),
                                    codeName, codeInt,
                                    msg);
        }
        else
        {
            written = std::snprintf(dst, cap, "[%s@%s:%d] %s(%d): %s | cause: %s",
                                    originName, file, info.line(),
                                    codeName, codeInt,
                                    msg, cause);
        }
        return (written < 0) ? 0u : static_cast<std::size_t>(written);
    }

    std::string formatErrorInfo(const ErrorInfo& info)
    {
        // Two-pass to avoid both truncation and an exception: measure first
        // (snprintf returns required size), then allocate exactly enough.
        // This costs one extra format on the typical path but keeps the
        // helper allocation-correct even for pathological message lengths.
        char small[256];
        const std::size_t need = formatErrorInfo(info, small, sizeof small);
        if (need < sizeof small)
        {
            return std::string(small, need);
        }
        std::string out;
        out.resize(need);
        // need + 1 because snprintf wants room for the trailing NUL.
        formatErrorInfo(info, &out[0], need + 1);
        return out;
    }

    void setErrorSink(ErrorSink sink, void* user)
    {
        // Publish user FIRST so that a concurrent dispatch that loads sink
        // and observes it as non-null can never observe a stale user paired
        // with the new sink. See note above gSink/gSinkUser.
        gSinkUser.store(user, std::memory_order_release);
        gSink.store(sink, std::memory_order_release);
    }

    bool hasErrorSink()
    {
        return gSink.load(std::memory_order_acquire) != nullptr;
    }

    void stderrErrorSink(const ErrorInfo& info, void* user)
    {
        // Cast user back to FILE*. The library never dereferences user
        // itself; this sink is the only thing that interprets it. nullptr
        // -> stderr is the documented fallback so embedders can install
        // the sink with a default user pointer.
        std::FILE* fp = (user != nullptr) ? static_cast<std::FILE*>(user) : stderr;

        // Format into a stack buffer first so a single fputs writes the
        // whole record - reduces interleaving when multiple threads emit
        // simultaneously (most stdio implementations are line-buffered
        // per-call, not per-byte).
        char buf[512];
        formatErrorInfo(info, buf, sizeof buf);

        std::fputs("[heif] ", fp);
        std::fputs(buf, fp);
        std::fputc('\n', fp);
        // Intentionally no fflush: embedders who need synchronous output
        // can install their own sink. stderr is unbuffered on POSIX by
        // default; stdout is line-buffered when isatty(stdout); both are
        // fine for the demo / CLI use-case this sink targets.
    }

    // ------------------------------------------------------------------
    //  detail::heifFail* — helpers behind the HEIF_FAIL* macros.
    //
    //  Kept as plain functions (not inline / not lambdas) so the public
    //  macros expand to a single source line: __LINE__ then sits on the
    //  same physical line as the macro name in the caller's source and
    //  reports the call-site line on every conforming preprocessor
    //  (GCC, Clang, and MSVC behave identically). The previous lambda-
    //  in-macro form left __LINE__ implementation-defined for multi-line
    //  invocations - MSVC reported the closing-paren line, GCC the
    //  opening-paren line.
    // ------------------------------------------------------------------
    namespace detail
    {
        ErrorCode heifFail(ErrorCode    code,
                           ErrorOrigin  origin,
                           const char*  message,
                           const char*  file,
                           int          line)
        {
            ErrorInfo info;
            info.setCode(code);
            info.setOrigin(origin);
            info.setMessage(message);
            info.setFile(file);
            info.setLine(line);
            info.setValid(true);
            setErrorInfo(info);
            return code;
        }

        ErrorCode heifFailFrom(ErrorCode    code,
                               ErrorOrigin  origin,
                               const char*  message,
                               const char*  causeWhat,
                               const char*  file,
                               int          line)
        {
            ErrorInfo info;
            info.setCode(code);
            info.setOrigin(origin);
            info.setMessage(message);
            info.setCauseWhat((causeWhat != nullptr) ? causeWhat : "");
            info.setFile(file);
            info.setLine(line);
            info.setValid(true);
            setErrorInfo(info);
            return code;
        }

        ErrorCode heifFailFromUnknown(ErrorCode    code,
                                      ErrorOrigin  origin,
                                      const char*  message,
                                      const char*  file,
                                      int          line)
        {
            ErrorInfo info;
            info.setCode(code);
            info.setOrigin(origin);
            info.setMessage(message);
            info.setCauseWhat("<unknown exception>");
            info.setFile(file);
            info.setLine(line);
            info.setValid(true);
            setErrorInfo(info);
            return code;
        }
    }  // namespace detail
}  // namespace HEIF
