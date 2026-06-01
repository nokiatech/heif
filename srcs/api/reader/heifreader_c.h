/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled
 * by Nokia Corporation and/or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

/**
 * @file heifreader_c.h
 *
 * Pure-C reader facade over `HEIF::Reader`.
 *
 * **What this is.**  A minimum-viable, `extern "C"` reader surface
 * sufficient for a binding (Python `cffi`, Rust `bindgen`, Go `cgo`,
 * .NET P/Invoke, Swift module-map, ...) to open a HEIF/HEIC file,
 * enumerate its master image items, and pull a decoder-ready
 * bitstream out of each one.
 *
 * **Architectural rules.**  Together these define the
 * binding contract for *every* future entry point on this surface.
 *
 *   1. **Opaque handles.**  `heif_reader_t*` is a pointer to an
 *      incomplete struct; the layout is never exposed in this header.
 *      Callers may only pass the pointer back into other entry
 *      points on this surface.
 *
 *   2. **ID types as `uint32_t`.**  `HEIF::ImageId`, `HEIF::SequenceId`,
 *      etc. are POD wrappers around a single `uint32_t` (see
 *      `heifid.h::IdType`).  The C surface uses raw `uint32_t` and
 *      reconstructs the wrapper at the boundary, so a future change
 *      to the C++ wrapper shape is invisible to bindings.
 *
 *   3. **FourCC as caller-owned `char[5]`.**  Output FourCCs are
 *      written into a 5-byte buffer (4 bytes + terminating NUL) the
 *      caller owns, matching `HEIF::FourCC::value` byte-for-byte.
 *      Input FourCCs are passed as `const char*` of length >= 4.
 *
 *   4. **`Array<T>` is FFI-poison.**  `HEIF::Array<T>` has a
 *      `virtual ~Array()` and so cannot cross any C boundary safely.
 *      Replaced uniformly by a `(get_X_count, get_X_at)` pair: one
 *      call returns the count, repeated calls fetch elements by
 *      index.  Indexing is bounds-checked at the boundary.
 *
 *   5. **Owning byte payloads use the `snprintf(3)`-style size-probe
 *      convention.**  The hot path
 *      (`heif_reader_get_item_data_with_decoder_parameters`) accepts
 *      `(buf, *size_inout)`: pass `(NULL, 0)` to ask for the required
 *      size (returned via `*size_inout` and `BUFFER_SIZE_TOO_SMALL`),
 *      then allocate caller-side, then call again.  This matches the
 *      existing C++ contract exactly, so there is zero impedance
 *      mismatch and zero need for an `heif_reader_free_*` companion
 *      on this hot path.
 *      **The probe-and-fetch protocol is used consistently across
 *      all layers (raw C++ <-> HEIFPP <-> C facade <-> JNI <-> bindings).**
 *
 *   6. **Owning string returns use paired-free** (property objects
 *      that surface user-visible strings).
 *
 *   7. **Errors drain through the error surface.**  Every entry point returns a
 *      `heif_error_code_t` (alias of `int32_t`, value range matches
 *      `HEIF::ErrorCode`).  On non-OK, the embedder calls the
 *      `heif_get_last_error_*` family (in `heif_error_c.h`) to pull
 *      the structured detail.  No error info lives on the return
 *      value path itself.
 *
 *   8. **Caller-allocated POD struct + size-on-entry.**
 *      For property payloads with more than ~3 scalars
 *      (`heif_pixi_channel_t`, the upcoming `heif_clli_t` /
 *      `heif_mdcv_t` / `heif_gainmap_t` / `heif_grid_t` ...), the C
 *      surface declares a flat POD struct of `<stdint.h>` fixed-width
 *      types only -- no `bool`, no bare `enum`, no bitfields, no
 *      `long`/`size_t`/`long double`, no FAMs, no function pointers.
 *      Fields are ordered descending alignment, with explicit
 *      `_reserved[N]` padding so the binary layout is decided by the
 *      source, not by the compiler, and a `HEIF_STATIC_ASSERT(sizeof
 *      == N)` next to every declaration prevents silent ABI drift.
 *      Each such struct begins with a `uint32_t struct_size` field
 *      that the caller MUST set to `sizeof(*out)` before the call;
 *      the library uses the value to gate forward / backward binary
 *      compatibility (smaller-than-current -> fill leading prefix
 *      only; larger-than-current -> fill known fields, zero the rest,
 *      return `OK`; zero / insane -> `INVALID_FUNCTION_PARAMETER`).
 *      Evolution rule: append-only; never reorder, resize, or
 *      repurpose a field.  Bindings regenerate `cdef` against the
 *      latest header and stay binary-compatible across minor library
 *      bumps for free.
 *
 *   9. **Nested arrays follow the `(_count, _at)` shape, not
 *      flexible array members.**  When a property contains a
 *      variable-length list of records (pixi channels, gainmap
 *      per-channel data, sample groups, ...), expose it as a
 *      `_count(...)` plus a `_at(..., index, &out_record)` pair.
 *      Each record is itself a POD struct.  Variable-length
 *      strings inside a record are split out into their own
 *      `_at_label`-style accessor that uses the size-probe.
 *      No FAMs, no `T*` pointers in shared structs, no walk-the-list
 *      contracts.  This keeps every binding (cffi, JNA, bindgen,
 *      cgo, P/Invoke) on the same trivial cdef shape.
 *
 * **Thread-safety (normative).**  A `heif_reader_t*` instance is
 * **not** safe for concurrent use from multiple threads -- each
 * consumer thread should hold its own instance.  Multiple
 * instances may exist concurrently.  `heif_reader_create` /
 * `heif_reader_destroy` are themselves thread-safe and may be
 * called from any thread.  The error slot drained by
 * `heif_error_c.h` is `thread_local`, so a failure on thread A is
 * invisible to thread B; a non-OK return must be inspected on the
 * *same* thread that observed it.  `heif_reader_initialize_stream`
 * stream callbacks are invoked only from the thread that called
 * the reader method, never concurrently on a single instance --
 * see the dedicated note on `heif_stream_callbacks_t` below.
 *
 * **Relationship to the C++ vtable.**  Every entry point on this
 * surface forwards 1:1 to a single `HEIF::Reader` virtual.  The C
 * surface adds no behavior of its own beyond the marshalling rules
 * above; if a binding observes a divergence between the C surface
 * and the C++ surface, that is a bug in the marshalling layer, not
 * a feature.
 */

#ifndef HEIF_READER_C_H
#define HEIF_READER_C_H

#include <stdint.h>

#include "heifexport.h"

/**
 * Portable compile-time assertion macro for ABI-pinning struct
 * sizes.  Resolves to `static_assert` in C++11+, to
 * `_Static_assert` in C11+, and to a tag-typedef trick on older
 * dialects.  Used at file scope next to every wire-struct
 * declaration so a contributor who reorders or resizes a field
 * gets a build break, not a silent ABI drift.
 */
#if defined(__cplusplus) && (__cplusplus >= 201103L)
#  define HEIF_STATIC_ASSERT(cond_, msg_) static_assert((cond_), msg_)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#  define HEIF_STATIC_ASSERT(cond_, msg_) _Static_assert((cond_), msg_)
#else
#  define HEIF_STATIC_ASSERT_PASTE_(a_, b_) a_##b_
#  define HEIF_STATIC_ASSERT_NAME_(line_)   HEIF_STATIC_ASSERT_PASTE_(heif_sa_, line_)
#  define HEIF_STATIC_ASSERT(cond_, msg_) \
       typedef char HEIF_STATIC_ASSERT_NAME_(__LINE__)[(cond_) ? 1 : -1]
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * `heif_error_code_t` is the integer-tier mirror of `HEIF::ErrorCode`
 * (which is `: std::uint8_t`).  Widened to `int32_t` on the C
 * surface so a future ABI bump can grow the enum without changing
 * the signature of every reader entry point.  Numeric values are
 * stable: `0` is always `OK`.
 *
 * After a non-OK return, call `heif_get_last_error_*` from
 * `heif_error_c.h` to retrieve the structured failure context.
 */
typedef int32_t heif_error_code_t;

/**
 * Opaque reader handle.  Created by `heif_reader_create`, released by
 * `heif_reader_destroy`.  The struct is intentionally never defined in
 * this header -- bindings see only the pointer.
 */
typedef struct heif_reader heif_reader_t;

/* ------------------------------------------------------------------ */
/*  Lifecycle                                                          */
/* ------------------------------------------------------------------ */

/**
 * Allocate a fresh reader instance.  Returns `NULL` on allocation
 * failure (the only failure mode); the C++ counterpart
 * `HEIF::Reader::Create()` would `throw std::bad_alloc`, but throwing
 * across an `extern "C"` boundary is undefined, so we catch the
 * allocation failure and surface it as `NULL` instead.
 *
 * The returned handle is *not* yet attached to a file -- call
 * `heif_reader_initialize_file` (or a future stream variant) before
 * any accessor.
 *
 * The library does not retain any reference to the returned handle;
 * the caller MUST release it via `heif_reader_destroy`.
 */
HEIF_DLL_FN_EXPORT heif_reader_t* heif_reader_create(void);

/**
 * Release a reader handle previously returned by `heif_reader_create`.
 * Passing `NULL` is a no-op (matches `free(3)` convention).  Passing
 * a pointer not returned by `heif_reader_create`, or passing the same
 * pointer twice, is undefined behavior -- same contract as `free(3)`.
 * Never throws; safe to call from any context including error
 * handlers and FFI finaliser threads.
 */
HEIF_DLL_FN_EXPORT void heif_reader_destroy(heif_reader_t* reader);

/**
 * Open a file and read its header.  Mirrors
 * `HEIF::Reader::initialize(const char*)`.  `path` is interpreted by
 * the underlying file API (typically system-default encoding); on
 * Windows, prefer feeding UTF-8 paths to the future stream variant
 * once it lands.
 *
 * @return `0` (`OK`) on success; non-zero `HEIF::ErrorCode` value on
 *         failure (e.g. `FILE_OPEN_ERROR`, `FILE_READ_ERROR`,
 *         `FILE_HEADER_ERROR`).  Drain `heif_get_last_error_*` for
 *         the structured context.
 *
 * @pre `reader` was returned by `heif_reader_create` and has not yet
 *      been initialised, OR has been destroyed and re-created.
 *      Calling `initialize_file` twice on the same handle is *not*
 *      currently supported by the C++ surface; callers should
 *      destroy and re-create.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_initialize_file(heif_reader_t* reader, const char* path);

/* ------------------------------------------------------------------ */
/*  File-level accessors                                               */
/* ------------------------------------------------------------------ */

/**
 * Write the file's major brand into a caller-owned 5-byte buffer
 * (4 brand bytes + terminating NUL).  Mirrors
 * `HEIF::Reader::getMajorBrand(FourCC&)`.
 *
 * @param      reader     The reader handle.
 * @param [out] out_brand  Pointer to a 5-byte buffer.  On `OK`,
 *                         contains the 4-character brand followed by
 *                         a NUL terminator (e.g. `"heic\0"`,
 *                         `"mif1\0"`).  On non-OK, the buffer
 *                         contents are unspecified.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED` if the reader has
 *         not been attached to a file yet.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_major_brand(const heif_reader_t* reader, char out_brand[5]);

/**
 * Minor version of the file (the 32-bit `minor_version` field of the
 * `ftyp` box).  Mirrors `HEIF::Reader::getMinorVersion(uint32_t&)`.
 *
 * The value is opaque to the library -- bindings should treat it as
 * a vendor-defined integer that may be displayed or logged but
 * should not gate any code path on it.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED` if the reader has
 *         not been attached to a file yet.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_minor_version(const heif_reader_t* reader,
                                                                uint32_t*           out_minor_version);

/**
 * Count of compatible brands declared in the `ftyp` box.  Mirrors
 * the size of the `Array<FourCC>` returned by
 * `HEIF::Reader::getCompatibleBrands(Array<FourCC>&)` -- replacing
 * the `Array<T>` return shape with the `(_count, _at)` pair
 * pattern.
 *
 * The library caches the brand list on first access; subsequent
 * `_at` calls do not re-walk the file.  The cache is implicitly
 * invalidated by destroying the handle.
 *
 * @param      reader     The reader handle.
 * @param [out] out_count  Number of compatible brands.
 * @return `0` (`OK`) on success; `UNINITIALIZED` if the reader has
 *         not been attached to a file yet.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_compatible_brand_count(const heif_reader_t* reader,
                                                                         uint32_t*           out_count);

/**
 * Fetch the compatible-brand FourCC at the given index.  `index`
 * MUST be `< heif_reader_get_compatible_brand_count(...)`;
 * out-of-range indices return `INVALID_FUNCTION_PARAMETER`.
 *
 * @param      reader     The reader handle.
 * @param      index      Zero-based index into the compatible-brand list.
 * @param [out] out_brand  5-byte buffer (4 bytes + terminating NUL),
 *                         same convention as
 *                         `heif_reader_get_major_brand`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_compatible_brand_at(const heif_reader_t* reader,
                                                                      uint32_t            index,
                                                                      char                out_brand[5]);

/* ------------------------------------------------------------------ */
/*  Primary item                                                       */
/* ------------------------------------------------------------------ */

/**
 * Item id of the file's primary item.  Mirrors
 * `HEIF::Reader::getPrimaryItem(ImageId&)`.  The primary item is
 * the one a viewer should display by default; bindings that present
 * a single image (rather than a gallery) should select this id and
 * decode its bitstream via
 * `heif_reader_get_item_data_with_decoder_parameters`.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `PRIMARY_ITEM_NOT_SET` (the file declares no primary).
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_primary_item(const heif_reader_t* reader,
                                                               uint32_t*           out_image_id);

/* ------------------------------------------------------------------ */
/*  Item enumeration by type / by reference                            */
/* ------------------------------------------------------------------ */

/**
 * Count of items in the file whose item-type FourCC equals `type`.
 * Mirrors the size of the `Array<ImageId>` returned by
 * `HEIF::Reader::getItemListByType(FourCC, Array<ImageId>&)`.
 *
 * `type` is a NUL-terminated C string of length >= 4; only the first
 * 4 bytes are consumed.  Common values: `"hvc1"`, `"avc1"`,
 * `"vvc1"`, `"jpeg"`, `"grid"`, `"iovl"`, `"iden"`, `"tmap"`,
 * `"Exif"`, `"mime"`.
 *
 * The library maintains a single-entry cache keyed on `type`: if
 * the next `_at` call uses the same type, the cached array is
 * reused.  A different `type` invalidates the cache.  This keeps
 * the typical (count, at_0, at_1, ..., at_n-1) loop O(n) total.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `INVALID_FUNCTION_PARAMETER` (NULL `type` or `out_count`).
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_item_list_by_type_count(const heif_reader_t* reader,
                                                                          const char*         type,
                                                                          uint32_t*           out_count);

/**
 * Fetch the image id at the given index in the type-filtered item
 * list.  `index` MUST be
 * `< heif_reader_get_item_list_by_type_count(reader, type, ...)`,
 * AND `type` MUST equal the FourCC most recently passed to
 * `_count` (otherwise the library re-runs the query under the new
 * key -- correct, but slower).
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_item_list_by_type_at(const heif_reader_t* reader,
                                                                       const char*         type,
                                                                       uint32_t            index,
                                                                       uint32_t*           out_image_id);

/**
 * Count of items that the given `from_image_id` references with a
 * reference of the given `reference_type` (e.g. `"thmb"` to find
 * thumbnails of an image, `"cdsc"` to find Exif/XMP siblings,
 * `"auxl"` for auxiliary alpha, `"dimg"` for derived image
 * dependencies, `"base"` for predictively-coded base layers).
 * Mirrors `HEIF::Reader::getReferencedFromItemListByType(...)`.
 *
 * Single-entry cache keyed on `(from_image_id, reference_type)`;
 * the typical (count, at_0, ...) loop stays O(n).
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`, `INVALID_ITEM_ID`
 *         (the from-id is not present in the file).
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_referenced_from_item_list_by_type_count(
    const heif_reader_t* reader,
    uint32_t            from_image_id,
    const char*         reference_type,
    uint32_t*           out_count);

/**
 * Fetch the referenced-from item id at the given index.  Follow
 * the same single-entry-cache contract as `_count`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_referenced_from_item_list_by_type_at(
    const heif_reader_t* reader,
    uint32_t            from_image_id,
    const char*         reference_type,
    uint32_t            index,
    uint32_t*           out_image_id);

/**
 * Inverse of `_referenced_from_*`: count of items that reference
 * `to_image_id` with a reference of `reference_type`.  Mirrors
 * `HEIF::Reader::getReferencedToItemListByType(...)`.  Useful for
 * "given this thumbnail, what is its parent?" queries.
 *
 * Cache shape and contract identical to the from-direction.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_referenced_to_item_list_by_type_count(
    const heif_reader_t* reader,
    uint32_t            to_image_id,
    const char*         reference_type,
    uint32_t*           out_count);

/**
 * Fetch the referenced-to item id at the given index.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_referenced_to_item_list_by_type_at(
    const heif_reader_t* reader,
    uint32_t            to_image_id,
    const char*         reference_type,
    uint32_t            index,
    uint32_t*           out_image_id);

/* ------------------------------------------------------------------ */
/*  Master-image enumeration                                           */
/* ------------------------------------------------------------------ */

/**
 * Count of master image items in the file.  Mirrors the size of the
 * `Array<ImageId>` returned by
 * `HEIF::Reader::getMasterImages(Array<ImageId>&)`, replacing the
 * `Array<T>` return shape with the `(_count, _at)` pair pattern.
 *
 * @param      reader     The reader handle.
 * @param [out] out_count  Number of master image items.  Written on
 *                         `OK`; left untouched on non-OK.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED` if the reader has
 *         not been attached to a file yet.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_master_image_count(const heif_reader_t* reader,
                                                                     uint32_t*           out_count);

/**
 * Fetch the master image item id at the given index.  `index` MUST
 * be `< heif_reader_get_master_image_count(...)`; out-of-range
 * indices return `INVALID_FUNCTION_PARAMETER`.
 *
 * The returned `image_id` is the raw `uint32_t` payload of
 * `HEIF::ImageId` -- feed it back into any other
 * `heif_reader_*(image_id, ...)` entry point unchanged.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED` if not yet
 *         attached, `INVALID_FUNCTION_PARAMETER` on out-of-range
 *         index.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_master_image_at(const heif_reader_t* reader,
                                                                  uint32_t            index,
                                                                  uint32_t*           out_image_id);

/* ------------------------------------------------------------------ */
/*  Per-image accessors                                                */
/* ------------------------------------------------------------------ */

/**
 * Pixel width of an image item.  Mirrors
 * `HEIF::Reader::getWidth(const ImageId&, uint32_t&)`.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`, `INVALID_ITEM_ID`,
 *         or codec-specific failure on non-OK.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_width(const heif_reader_t* reader,
                                                        uint32_t            image_id,
                                                        uint32_t*           out_width);

/**
 * Pixel height of an image item.  Mirrors
 * `HEIF::Reader::getHeight(const ImageId&, uint32_t&)`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_height(const heif_reader_t* reader,
                                                         uint32_t            image_id,
                                                         uint32_t*           out_height);

/**
 * FourCC item type of an image item (e.g. `"hvc1"`, `"avc1"`,
 * `"vvc1"`, `"jpeg"`, `"grid"`, `"iovl"`, `"iden"`, `"tmap"`).
 * Mirrors `HEIF::Reader::getItemType(const ImageId&, FourCC&)`.
 *
 * @param      reader     The reader handle.
 * @param      image_id   The image item identifier.
 * @param [out] out_type  5-byte buffer (4 bytes + terminating NUL),
 *                        same convention as `heif_reader_get_major_brand`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_item_type(const heif_reader_t* reader,
                                                            uint32_t            image_id,
                                                            char                out_type[5]);

/* ------------------------------------------------------------------ */
/*  Decoder-ready bitstream extraction                                 */
/* ------------------------------------------------------------------ */

/**
 * Extract a decoder-ready bitstream for an encoded image item
 * (typically `'hvc1'`, `'avc1'`, `'vvc1'`, `'jpeg'`, ...).  Mirrors
 * `HEIF::Reader::getItemDataWithDecoderParameters(const ImageId&,
 * uint8_t*, uint64_t&)`.  On success the buffer contains the decoder
 * configuration record(s) followed by the item data with bytestream
 * NAL headers (`0x000001`) substituted in place of `nal_length`
 * fields where applicable.
 *
 * **Size-probe convention**.  Recommended pattern:
 *
 * @code{.c}
 *     uint64_t needed = 0;
 *     heif_error_code_t err = heif_reader_get_item_data_with_decoder_parameters(
 *         reader, image_id, NULL, &needed);
 *     if (err != 0 && err != HEIF_ERR_BUFFER_SIZE_TOO_SMALL) goto fail;
 *     uint8_t* buf = (uint8_t*)malloc((size_t)needed);
 *     err = heif_reader_get_item_data_with_decoder_parameters(
 *         reader, image_id, buf, &needed);
 *     // On OK, `needed` is the actual byte count written into `buf`.
 * @endcode
 *
 * The probe call returns `BUFFER_SIZE_TOO_SMALL` and writes the
 * required byte count into `*size_inout`.  The fetch call returns
 * `OK` and writes the actual byte count (<= the originally-supplied
 * capacity) into `*size_inout`.  Passing `buffer == NULL` with
 * `*size_inout > 0` is undefined behavior; pass `(NULL, 0)` to
 * probe.
 *
 * @param [in]      reader      Reader handle.
 * @param [in]      image_id    Item id (raw `uint32_t`).
 * @param [out]     buffer      Destination buffer, or `NULL` for a
 *                              size probe.  Caller-owned; library
 *                              never frees it.
 * @param [in,out]  size_inout  On entry: capacity of `buffer` in
 *                              bytes (0 for a probe).  On exit: on
 *                              `OK` the bytes actually written; on
 *                              `BUFFER_SIZE_TOO_SMALL` the bytes
 *                              required.
 *
 * @return `0` (`OK`) on success; `BUFFER_SIZE_TOO_SMALL` on probe
 *         (caller should re-try with a larger buffer);
 *         `UNINITIALIZED`, `INVALID_ITEM_ID`, `PROTECTED_ITEM`,
 *         `UNSUPPORTED_CODE_TYPE` on permanent failure.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_item_data_with_decoder_parameters(
    heif_reader_t* reader,
    uint32_t      image_id,
    uint8_t*      buffer,
    uint64_t*     size_inout);

/**
 * Extract raw item bytes for any item (e.g. `'Exif'`, `'mime'` /
 * XMP, or an encoded image item if the caller wants the bytestream
 * *without* decoder configuration records prepended).  Mirrors
 * `HEIF::Reader::getItemData(const ImageId&, uint8_t*, uint64_t&,
 * bool bytestreamHeaders=true)`.
 *
 * For non-image items (Exif, XMP) the result is the on-wire payload
 * as written into the file.  For coded image items this returns
 * the item data without the decoder configuration record prefix --
 * use `heif_reader_get_item_data_with_decoder_parameters` instead
 * if you intend to decode the bitstream.
 *
 * Uses the same `(buf, *size_inout)` size-probe convention as
 * `heif_reader_get_item_data_with_decoder_parameters`; see that
 * function's documentation for the recommended call pattern.
 *
 * @param [in]      reader      Reader handle.
 * @param [in]      image_id    Item id (raw `uint32_t`).
 * @param [out]     buffer      Destination buffer, or `NULL` for a
 *                              size probe.
 * @param [in,out]  size_inout  On entry: capacity of `buffer` in
 *                              bytes (0 for a probe).  On exit: on
 *                              `OK` the bytes actually written; on
 *                              `BUFFER_SIZE_TOO_SMALL` the bytes
 *                              required.
 *
 * @return `0` (`OK`) on success; `BUFFER_SIZE_TOO_SMALL` on probe;
 *         `UNINITIALIZED`, `INVALID_ITEM_ID`, `FILE_READ_ERROR`,
 *         `PROTECTED_ITEM` on permanent failure.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_item_data(heif_reader_t* reader,
                                                            uint32_t      image_id,
                                                            uint8_t*      buffer,
                                                            uint64_t*     size_inout);

/* ------------------------------------------------------------------ */
/*  Image properties (irot / imir / ispe / pixi / colr / pasp)            */
/* ------------------------------------------------------------------ */

/**
 * Property enumeration: get the number of properties associated
 * with an image item.  Mirrors the size of the
 * `Array<ItemPropertyInfo>` returned by
 * `HEIF::Reader::getItemProperties(const ImageId&,
 * Array<ItemPropertyInfo>&)`.
 *
 * Cached single-entry per `(reader, image_id)` so the typical
 * `(_count, _at[0..n))` loop is O(n) entry-point calls.
 *
 * @param      reader    The reader handle.
 * @param      image_id  The image item identifier.
 * @param [out] out_count Number of properties associated with the item.
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `INVALID_ITEM_ID`, or `INVALID_FUNCTION_PARAMETER`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_item_property_count(const heif_reader_t* reader,
                                                                      uint32_t            image_id,
                                                                      uint32_t*           out_count);

/**
 * Property enumeration: fetch the property descriptor at the given
 * index.  Returns the `(PropertyId, FourCC type, essential)` triple
 * matching `HEIF::ItemPropertyInfo`.
 *
 * The `out_type` FourCC is derived from
 * `HEIF::ItemPropertyType` and is the natural key bindings filter
 * on (e.g. `"irot"`, `"ispe"`, `"colr"`).  Properties whose type
 * is `RAW` (an unrecognised box) surface as the FourCC `"raw "`;
 * properties whose type is `INVALID` (should never happen on a
 * well-formed file) surface as `"\\0\\0\\0\\0"` and the call still
 * returns `OK` so the binding can decide what to do.
 *
 * @param      reader           The reader handle.
 * @param      image_id         The image item identifier.
 * @param      index            Zero-based index into the property list,
 *                              `< heif_reader_get_item_property_count(...)`.
 * @param [out] out_property_id  Raw `uint32_t` payload of
 *                               `HEIF::PropertyId`; pass back into
 *                               any `heif_reader_get_property_*`
 *                               typed accessor unchanged.
 * @param [out] out_type         5-byte FourCC buffer (4 bytes + NUL).
 * @param [out] out_essential    Non-zero iff the property is marked
 *                               essential for the item.  Bindings
 *                               that cannot honour an essential
 *                               unknown property MUST refuse the
 *                               item per ISO/IEC 23008-12.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `INVALID_ITEM_ID`, or `INVALID_FUNCTION_PARAMETER`
 *         (NULL out / out-of-range index).
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_item_property_at(const heif_reader_t* reader,
                                                                   uint32_t            image_id,
                                                                   uint32_t            index,
                                                                   uint32_t*           out_property_id,
                                                                   char                out_type[5],
                                                                   int*                out_essential);

/**
 * Image Spatial Extents (`'ispe'`) -- the encoded image dimensions
 * before any irot/imir/iscl transform is applied.  Mirrors
 * `HEIF::Reader::getProperty(const PropertyId&, SpatialExtents&)`.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `INVALID_PROPERTY_INDEX` (id is not an ispe property).
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_ispe(const heif_reader_t* reader,
                                                                uint32_t            property_id,
                                                                uint32_t*           out_width,
                                                                uint32_t*           out_height);

/**
 * Image Rotation (`'irot'`) -- counter-clockwise rotation angle in
 * degrees.  Valid values per ISO/IEC 23008-12 are `0`, `90`, `180`,
 * `270`; the library does not enforce this on read.  Mirrors
 * `HEIF::Reader::getProperty(const PropertyId&, Rotate&)`.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `INVALID_PROPERTY_INDEX`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_irot(const heif_reader_t* reader,
                                                                uint32_t            property_id,
                                                                uint32_t*           out_angle_ccw);

/**
 * Image Mirror (`'imir'`) -- the axis to mirror across.  Mirrors
 * `HEIF::Reader::getProperty(const PropertyId&, Mirror&)`.
 *
 * @param      reader       The reader handle.
 * @param      property_id  The property identifier returned by
 *                          `heif_reader_get_item_property_at`.
 * @param [out] out_horizontal_axis  Non-zero iff the mirror axis is
 *                                   horizontal (top<->bottom flip);
 *                                   zero iff vertical (left<->right).
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `INVALID_PROPERTY_INDEX`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_imir(const heif_reader_t* reader,
                                                                uint32_t            property_id,
                                                                int*                out_horizontal_axis);

/**
 * Pixel Information (`'pixi'`): channel count.  The expected pattern
 * is `pixi_channel_count(p)` -> `pixi_bits_at(p, 0..n-1)` for
 * per-channel bit depths.  Mirrors the size of `bitsPerChannel`
 * from `HEIF::PixelInformation`.
 *
 * The C facade caches the parsed `PixelInformation` per
 * `(reader, property_id)` so the count-then-at loop is O(n)
 * entry-point calls.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `INVALID_PROPERTY_INDEX`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_pixi_channel_count(const heif_reader_t* reader,
                                                                              uint32_t            property_id,
                                                                              uint32_t*           out_count);

/**
 * Pixel Information (`'pixi'`): bits-per-channel for the channel at
 * `channel_index`.  Index MUST be `< pixi_channel_count(...)`.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `INVALID_PROPERTY_INDEX`, `INVALID_FUNCTION_PARAMETER`
 *         (out-of-range index).
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_pixi_bits_at(const heif_reader_t* reader,
                                                                        uint32_t            property_id,
                                                                        uint32_t            channel_index,
                                                                        uint8_t*            out_bits);

/**
 * Colour Information (`'colr'`): the colour-type FourCC.  Common
 * values: `"nclx"` (numeric primaries/transfer/matrix triple),
 * `"rICC"` (restricted ICC profile), `"prof"` (full ICC profile).
 * Bindings dispatch on this value to decide whether to call
 * `_colr_nclx` or `_colr_icc`.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `INVALID_PROPERTY_INDEX`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_colr_type(const heif_reader_t* reader,
                                                                     uint32_t            property_id,
                                                                     char                out_type[5]);

/**
 * Colour Information (`'colr'`) -- `nclx` payload: the numeric
 * primaries / transfer characteristics / matrix coefficients triple
 * plus the full-range flag.  Only meaningful when the colour type
 * is `"nclx"`; for `"rICC"` / `"prof"` the underlying C++ struct
 * still has these fields populated to zero, which the C facade
 * forwards verbatim -- bindings should branch on
 * `_colr_type` first.
 *
 * @param      reader        The reader handle.
 * @param      property_id   The property identifier returned by
 *                           `heif_reader_get_item_property_at`.
 * @param [out] out_primaries  ITU-T Rec. H.273 ColourPrimaries code.
 * @param [out] out_transfer   ITU-T Rec. H.273 TransferCharacteristics code.
 * @param [out] out_matrix     ITU-T Rec. H.273 MatrixCoefficients code.
 * @param [out] out_full_range Non-zero iff full-range (PC-range)
 *                             samples; zero iff TV-range.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `INVALID_PROPERTY_INDEX`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_colr_nclx(const heif_reader_t* reader,
                                                                     uint32_t            property_id,
                                                                     uint16_t*           out_primaries,
                                                                     uint16_t*           out_transfer,
                                                                     uint16_t*           out_matrix,
                                                                     int*                out_full_range);

/**
 * Colour Information (`'colr'`) -- ICC profile payload bytes.  Uses
 * the same `(buf, *size_inout)` size-probe convention as
 * `heif_reader_get_item_data`.  Only meaningful when the
 * colour type is `"rICC"` or `"prof"`; for `"nclx"` the size is 0
 * and the call returns `OK` with `*size_inout == 0`.
 *
 * @return `0` (`OK`) on success or zero-size payload;
 *         `BUFFER_SIZE_TOO_SMALL` on probe;
 *         `UNINITIALIZED`, `INVALID_PROPERTY_INDEX`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_colr_icc(const heif_reader_t* reader,
                                                                    uint32_t            property_id,
                                                                    uint8_t*            buffer,
                                                                    uint64_t*           size_inout);

/**
 * Pixel Aspect Ratio (`'pasp'`): the (relativeWidth, relativeHeight)
 * fraction.  A value of (1, 1) means square pixels.  Mirrors
 * `HEIF::Reader::getProperty(const PropertyId&, PixelAspectRatio&)`.
 *
 * Common on phone cameras shooting in non-1:1 sensor modes; bindings
 * that resample to display geometry MUST honour this property.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `INVALID_PROPERTY_INDEX`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_pasp(const heif_reader_t* reader,
                                                                uint32_t            property_id,
                                                                uint32_t*           out_relative_width,
                                                                uint32_t*           out_relative_height);

/* ------------------------------------------------------------------ */
/*  pixi ChannelData (rules 8 & 9: caller-allocated POD               */
/*             struct + size-on-entry + (_count, _at) for nested      */
/*             arrays).                                               */
/* ------------------------------------------------------------------ */

/**
 * Per-channel descriptor for the v3 `pixi` Pixel Information
 * property's `ChannelData` array.  Mirrors the
 * `HEIF::PixelInformation::ChannelEntry` + `ChannelInfo` records
 * from the C++ surface.
 *
 * **Architectural (caller-allocated POD struct + size-on-
 * entry).**  The caller allocates the struct (stack or heap),
 * sets `struct_size = sizeof(heif_pixi_channel_t)` before the
 * call, and passes a pointer to the library.  The library uses
 * `struct_size` to gate forward / backward binary compatibility:
 *
 *   * If the caller's `struct_size` matches the library's
 *     `sizeof`, the library fills every field.
 *   * If the caller's value is *smaller* than the library's
 *     `sizeof` (old caller, new library), the library fills only
 *     the leading prefix the caller knows about.
 *   * If the caller's value is *larger* (new caller, old library),
 *     the library fills the fields it knows and zeros the rest;
 *     the call still returns `OK`.
 *   * If the caller's value is `0` or smaller than the minimum
 *     supported version, the call returns
 *     `INVALID_FUNCTION_PARAMETER`.
 *
 * **Field ordering** is descending-alignment, with explicit
 * `_reserved` padding so the binary layout is fixed by source,
 * not by the compiler -- the trailing `HEIF_STATIC_ASSERT(sizeof
 * == 12)` pins this and breaks the build if a future contributor
 * reorders or resizes a field.
 *
 * **Evolution rule:** append-only.  Never reorder, resize, or
 * repurpose a field; bump the trailing `_reserved` size whenever
 * fields are added to keep the struct rounded to 8 bytes.
 *
 * For the variable-length `ChannelLabel` UTF-8 string, see
 * `heif_reader_get_property_pixi_channel_label` (size-probe).
 */
typedef struct heif_pixi_channel
{
    /** MUST equal `sizeof(heif_pixi_channel_t)` on entry. */
    uint32_t struct_size;
    /** Channel identifier code (ISO/IEC 23008-12 v3 Table A.x):
     *  0 = Unused, 1 = Unspecified, 2 = Y / R, 3 = U (Cb) / G,
     *  4 = V (Cr) / B, 5 = Alpha, 6 = Depth, 7 = Fourth (K). */
    uint8_t  channel_idc;
    /** ComponentFormat from the v3 ChannelInfo (codec-defined). */
    uint8_t  component_format;
    /** Non-zero iff `subsampling_type` / `subsampling_location`
     *  carry meaningful values (i.e. the v3 SubSamplingFlag bit
     *  was set in the parsed box).  When zero, both fields are
     *  zeroed and bindings should treat the channel as
     *  unsubsampled. */
    uint8_t  subsampling_flag;
    /** Non-zero iff a `ChannelLabel` UTF-8 string is present;
     *  fetch via `heif_reader_get_property_pixi_channel_label`.
     *  When zero, no label is associated with this channel. */
    uint8_t  channel_label_flag;
    /** SubSamplingType (codec-defined, e.g. YCbCr 4:2:0/4:2:2/4:4:4
     *  encoding).  Zero when `subsampling_flag == 0`. */
    uint8_t  subsampling_type;
    /** SubSamplingLocation (codec-defined chroma siting).
     *  Zero when `subsampling_flag == 0`. */
    uint8_t  subsampling_location;
    /** Reserved bytes; MUST be ignored on read.  Library writes
     *  zero here.  Present so the struct is exactly 12 bytes on
     *  every supported target (alignment). */
    uint8_t  _reserved[2];
} heif_pixi_channel_t;

HEIF_STATIC_ASSERT(sizeof(heif_pixi_channel_t) == 12,
                   "heif_pixi_channel_t ABI is exactly 12 bytes; "
                   "do not reorder/resize fields, append only");

/**
 * Pixel Information (`'pixi'`): the spec `numChannels` byte from
 * the v3 box header.  On a well-formed file this equals both
 * `heif_reader_get_property_pixi_channel_count` (the size of
 * `bitsPerChannel`) and the size of the `ChannelData` array
 * walked by `_pixi_channel_at`; the three accessors are surfaced
 * separately so a binding can detect a size-mismatch in a
 * malformed file.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `INVALID_PROPERTY_INDEX`, `INVALID_FUNCTION_PARAMETER`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_pixi_num_channels(const heif_reader_t* reader,
                                                                             uint32_t            property_id,
                                                                             uint8_t*            out_num_channels);

/**
 * Pixel Information (`'pixi'`): fetch the v3 `ChannelData[index]`
 * record into a caller-allocated `heif_pixi_channel_t`.
 *
 * Caller MUST set `out->struct_size = sizeof(heif_pixi_channel_t)`
 * before calling.  Index MUST be `< heif_reader_get_property_pixi_channel_count`
 * -- but note the bound is the size of `ChannelData`, which on a
 * well-formed v3 pixi equals `bitsPerChannel.size`; on a v0/v1
 * pixi without `ChannelData` this call returns
 * `INVALID_FUNCTION_PARAMETER` for any index because there are no
 * v3 channel records to fetch.
 *
 * @return `0` (`OK`) on success; `UNINITIALIZED`,
 *         `INVALID_PROPERTY_INDEX`, `INVALID_FUNCTION_PARAMETER`
 *         (NULL out / bad `struct_size` / out-of-range index).
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_pixi_channel_at(const heif_reader_t* reader,
                                                                           uint32_t            property_id,
                                                                           uint32_t            channel_index,
                                                                           heif_pixi_channel_t* out_channel);

/**
 * Pixel Information (`'pixi'`): fetch the v3 ChannelLabel UTF-8
 * string for the channel at `channel_index`.  Uses the
 * `(buf, *size_inout)` size-probe convention: pass
 * `(NULL, 0)` to ask for the required size including the
 * terminating NUL, then allocate caller-side, then call again.
 *
 * Channels without a label (`channel_label_flag == 0` in the
 * matching `heif_pixi_channel_t`) return `OK` with
 * `*size_inout == 0` and an unmodified buffer.
 *
 * @return `0` (`OK`) on success or absent label;
 *         `BUFFER_SIZE_TOO_SMALL` on probe; `UNINITIALIZED`,
 *         `INVALID_PROPERTY_INDEX`, `INVALID_FUNCTION_PARAMETER`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_pixi_channel_label(const heif_reader_t* reader,
                                                                              uint32_t            property_id,
                                                                              uint32_t            channel_index,
                                                                              char*               buffer,
                                                                              uint64_t*           size_inout);

/* ------------------------------------------------------------------ */
/*  HDR static metadata properties                                     */
/*    clli / mdcv / cclv / amve / reve / ndwt                          */
/*    All wire structs follow (caller-allocated POD struct +    */
/*    leading uint32_t struct_size).  All payload arrays in the C++    */
/*    structs are spec-fixed at 3 elements (RGB primaries) and are     */
/*    surfaced as fixed-size uint16_t[3] / uint32_t[3] arrays here.    */
/*    A file whose Array<> size diverges from the spec returns         */
/*    FILE_HEADER_ERROR rather than corrupting the binding.            */
/* ------------------------------------------------------------------ */

/**
 * Content Light Level Information (`'clli'`) -- peak / frame-average
 * luminance from the SEI message.  Required for HDR10 tone-mapping.
 * 8 bytes.
 */
typedef struct heif_clli
{
    uint32_t struct_size;            /**< MUST be sizeof(heif_clli_t). */
    uint16_t max_content_light_level;
    uint16_t max_pic_average_light_level;
} heif_clli_t;
HEIF_STATIC_ASSERT(sizeof(heif_clli_t) == 8, "heif_clli_t ABI is 8 bytes");

/**
 * Mastering Display Colour Volume (`'mdcv'`) -- primaries + white
 * point + luminance bounds of the mastering monitor.  Required for
 * Dolby Vision / HDR10 / HDR10+ rendering pipelines.  Primaries are
 * R, G, B in array index order 0..2 (file order is preserved
 * verbatim).  All x/y values are in chromaticity units of 0.00002
 * per ISO/IEC 23001-13 / SMPTE ST 2086.  Luminance is in 0.0001
 * cd/m^2.  40 bytes.
 */
typedef struct heif_mdcv
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_mdcv_t). */
    uint16_t display_primaries_x[3];            /**< R, G, B chromaticity x. */
    uint16_t display_primaries_y[3];            /**< R, G, B chromaticity y. */
    uint16_t white_point_x;
    uint16_t white_point_y;
    uint8_t  _reserved[4];                      /**< Pad to 4-byte alignment. */
    uint32_t max_display_mastering_luminance;
    uint32_t min_display_mastering_luminance;
} heif_mdcv_t;
HEIF_STATIC_ASSERT(sizeof(heif_mdcv_t) == 32, "heif_mdcv_t ABI is 32 bytes");

/**
 * Content Colour Volume (`'cclv'`) -- extended HDR metadata.  The
 * four `*_present` flags gate which trailing fields are
 * meaningful (the C++ struct stores `bool`; the C wire surface
 * uses `uint8_t` (0 or 1) per the size-prefix convention).  Primaries arrays mirror
 * `mdcv` ordering.  40 bytes.
 */
typedef struct heif_cclv
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_cclv_t). */
    uint8_t  primaries_present;
    uint8_t  min_luminance_present;
    uint8_t  max_luminance_present;
    uint8_t  avg_luminance_present;
    uint32_t primaries_x[3];                    /**< Valid iff primaries_present. */
    uint32_t primaries_y[3];                    /**< Valid iff primaries_present. */
    uint32_t min_luminance_value;
    uint32_t max_luminance_value;
    uint32_t avg_luminance_value;
} heif_cclv_t;
HEIF_STATIC_ASSERT(sizeof(heif_cclv_t) == 44, "heif_cclv_t ABI is 44 bytes");

/**
 * Ambient Viewing Environment (`'amve'`) -- illuminance + ambient
 * light chromaticity at the intended viewing location.  16 bytes.
 */
typedef struct heif_amve
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_amve_t). */
    uint32_t ambient_illuminance;
    uint16_t ambient_light_x;
    uint16_t ambient_light_y;
} heif_amve_t;
HEIF_STATIC_ASSERT(sizeof(heif_amve_t) == 12, "heif_amve_t ABI is 12 bytes");

/**
 * Reference Viewing Environment (`'reve'`) -- surround + periphery
 * luminance / chromaticity for the colour-grading reference
 * monitor.  24 bytes.
 */
typedef struct heif_reve
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_reve_t). */
    uint32_t surround_luminance;
    uint16_t surround_light_x;
    uint16_t surround_light_y;
    uint32_t periphery_luminance;
    uint16_t periphery_light_x;
    uint16_t periphery_light_y;
} heif_reve_t;
HEIF_STATIC_ASSERT(sizeof(heif_reve_t) == 20, "heif_reve_t ABI is 20 bytes");

/**
 * Nominal Diffuse White (`'ndwt'`) -- display luminance that should
 * map to "diffuse white" in the rendered image.  8 bytes.
 */
typedef struct heif_ndwt
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_ndwt_t). */
    uint32_t diffuse_white_luminance;
} heif_ndwt_t;
HEIF_STATIC_ASSERT(sizeof(heif_ndwt_t) == 8, "heif_ndwt_t ABI is 8 bytes");

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_clli(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                heif_clli_t*         out_clli);
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_mdcv(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                heif_mdcv_t*         out_mdcv);
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_cclv(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                heif_cclv_t*         out_cclv);
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_amve(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                heif_amve_t*         out_amve);
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_reve(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                heif_reve_t*         out_reve);
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_ndwt(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                heif_ndwt_t*         out_ndwt);

/* ------------------------------------------------------------------ */
/*  Derived items: Grid and Overlay                                    */
/*    Grid:   getItem(ImageId, Grid&)                                  */
/*    Overlay: getItem(ImageId, Overlay&)                              */
/*    Both expose the scalar header via a POD struct, and the  */
/*    nested image-id / offset list via the count-at (_count, _at) pair.*/
/* ------------------------------------------------------------------ */

typedef struct heif_grid
{
    uint32_t struct_size;            /**< MUST be sizeof(heif_grid_t). */
    uint32_t output_width;
    uint32_t output_height;
    uint32_t columns;
    uint32_t rows;
} heif_grid_t;
HEIF_STATIC_ASSERT(sizeof(heif_grid_t) == 20, "heif_grid_t ABI is 20 bytes");

/**
 * Fetch the grid header (output dimensions + grid shape).  The
 * constituent image-ID list is fetched separately via
 * `heif_reader_get_grid_image_count` + `_at`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_grid(const heif_reader_t* reader,
                                                       uint32_t             image_id,
                                                       heif_grid_t*         out_grid);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_grid_image_count(const heif_reader_t* reader,
                                                                   uint32_t             image_id,
                                                                   uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_grid_image_at(const heif_reader_t* reader,
                                                                uint32_t             image_id,
                                                                uint32_t             index,
                                                                uint32_t*            out_image_id);

typedef struct heif_overlay
{
    uint32_t struct_size;            /**< MUST be sizeof(heif_overlay_t). */
    uint16_t fill_r;                 /**< sRGB fill colour for empty canvas. */
    uint16_t fill_g;
    uint16_t fill_b;
    uint16_t fill_a;                 /**< 0 = transparent, 65535 = opaque. */
    uint32_t output_width;
    uint32_t output_height;
} heif_overlay_t;
HEIF_STATIC_ASSERT(sizeof(heif_overlay_t) == 20, "heif_overlay_t ABI is 20 bytes");

/**
 * Per-image entry in an overlay derivation: the image-id + its
 * (signed) top-left offset relative to the canvas origin.
 */
typedef struct heif_overlay_image
{
    uint32_t struct_size;            /**< MUST be sizeof(heif_overlay_image_t). */
    uint32_t image_id;
    int32_t  horizontal_offset;
    int32_t  vertical_offset;
} heif_overlay_image_t;
HEIF_STATIC_ASSERT(sizeof(heif_overlay_image_t) == 16, "heif_overlay_image_t ABI is 16 bytes");

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_overlay(const heif_reader_t* reader,
                                                          uint32_t             image_id,
                                                          heif_overlay_t*      out_overlay);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_overlay_image_count(const heif_reader_t* reader,
                                                                      uint32_t             image_id,
                                                                      uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_overlay_image_at(const heif_reader_t*  reader,
                                                                   uint32_t              image_id,
                                                                   uint32_t              index,
                                                                   heif_overlay_image_t* out_image);

/* ------------------------------------------------------------------ */
/*  Apple-style gain-map HDR (Tonemap derived item)                    */
/*    `tmap` items wrap a base SDR image + a gain map + GainMapData    */
/*    metadata used to reconstruct an HDR rendering at display time.   */
/* ------------------------------------------------------------------ */

typedef struct heif_gainmap
{
    uint32_t struct_size;                          /**< MUST be sizeof(heif_gainmap_t). */
    uint16_t minimum_version;
    uint16_t writer_version;
    uint8_t  is_multichannel;                      /**< 0/1. */
    uint8_t  use_base_colour_space;                /**< 0/1. */
    uint8_t  _reserved[2];
    int32_t  channel_count;                        /**< Per spec, 1 or 3. */
    uint32_t base_hdr_headroom_numerator;
    uint32_t base_hdr_headroom_denominator;
    uint32_t alternate_hdr_headroom_numerator;
    uint32_t alternate_hdr_headroom_denominator;
} heif_gainmap_t;
HEIF_STATIC_ASSERT(sizeof(heif_gainmap_t) == 32, "heif_gainmap_t ABI is 32 bytes");

typedef struct heif_gainmap_channel
{
    uint32_t struct_size;                /**< MUST be sizeof(heif_gainmap_channel_t). */
    int32_t  gain_map_min_numerator;
    uint32_t gain_map_min_denominator;
    int32_t  gain_map_max_numerator;
    uint32_t gain_map_max_denominator;
    uint32_t gamma_numerator;
    uint32_t gamma_denominator;
    int32_t  base_offset_numerator;
    uint32_t base_offset_denominator;
    int32_t  alternate_offset_numerator;
    uint32_t alternate_offset_denominator;
} heif_gainmap_channel_t;
HEIF_STATIC_ASSERT(sizeof(heif_gainmap_channel_t) == 44, "heif_gainmap_channel_t ABI is 44 bytes");

/**
 * Fetch the GainMapData header from a `tmap` derived item.
 * Per-channel records are reachable via
 * `_tonemap_gainmap_channel_count` + `_at`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_tonemap_gainmap(const heif_reader_t* reader,
                                                                  uint32_t             image_id,
                                                                  heif_gainmap_t*      out_gainmap);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_tonemap_gainmap_channel_count(const heif_reader_t* reader,
                                                                                uint32_t             image_id,
                                                                                uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_tonemap_gainmap_channel_at(const heif_reader_t*    reader,
                                                                             uint32_t                image_id,
                                                                             uint32_t                index,
                                                                             heif_gainmap_channel_t* out_channel);

/**
 * The image-id list referenced by the `tmap` item (the `dimg`
 * references -- typically the base SDR image followed by the gain
 * map).  Indexed via the standard (_count, _at) pair.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_tonemap_image_count(const heif_reader_t* reader,
                                                                      uint32_t             image_id,
                                                                      uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_tonemap_image_at(const heif_reader_t* reader,
                                                                   uint32_t             image_id,
                                                                   uint32_t             index,
                                                                   uint32_t*            out_image_id);

/* ------------------------------------------------------------------ */
/*  File / item / metabox feature bitmasks + decode helpers             */
/*    These answer "what kind of file/item is this?" cheaply, without   */
/*    requiring the binding to walk every property of every item.      */
/* ------------------------------------------------------------------ */

/**
 * File-level feature bitmask.  Bits map 1:1 to
 * `HEIF::FileFeatureEnum::Feature`:
 *
 *   bit 0 (0x01) -- HasSingleImage
 *   bit 1 (0x02) -- HasImageCollection
 *   bit 2 (0x04) -- HasImageSequence
 *   bit 3 (0x08) -- HasRootLevelMetaBox
 *   bit 4 (0x10) -- HasAlternateTracks
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_file_features(const heif_reader_t* reader,
                                                                uint32_t*            out_features);

/**
 * Root-metabox feature bitmask.  Bits map 1:1 to
 * `HEIF::MetaBoxFeatureEnum::Feature` (9 bits: IsSingleImage,
 * IsImageCollection, HasMasterImages, HasThumbnails,
 * HasAuxiliaryImages, HasDerivedImages,
 * HasPreComputedDerivedImages, HasHiddenImages, HasGroupLists).
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_metabox_features(const heif_reader_t* reader,
                                                                   uint32_t*            out_features);

/**
 * Per-item feature bitmask.  Bits map 1:1 to
 * `HEIF::ItemFeatureEnum::Feature` (23 bits including
 * IsMasterImage, IsThumbnailImage, IsAuxiliaryImage,
 * IsPrimaryImage, IsDerivedImage, IsHiddenImage, IsCoverImage,
 * IsProtected, HasLinkedThumbnails, IsExifItem, IsXMPItem,
 * IsGainmapImage, ...).  Returned in O(1) after the first call --
 * the parsed `FileInformation` is cached on the handle.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_item_features(const heif_reader_t* reader,
                                                                uint32_t             image_id,
                                                                uint32_t*            out_features);

/**
 * Decoder code type FourCC: e.g. `"hvc1"` vs `"hev1"`.  The
 * difference matters at the bytestream level (NAL unit prefix
 * format) -- bindings that pre-pend start codes need to branch
 * on this.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_decoder_code_type(const heif_reader_t* reader,
                                                                    uint32_t             image_id,
                                                                    char                 out_type[5]);

/**
 * Decode-dependency list: image-IDs whose decoded samples must be
 * available before this item can be decoded (predictively-coded
 * items, multi-layer VVC, ...).  Most still-image items have no
 * dependencies -> count returns 0.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_decode_dependency_count(const heif_reader_t* reader,
                                                                          uint32_t             image_id,
                                                                          uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_decode_dependency_at(const heif_reader_t* reader,
                                                                       uint32_t             image_id,
                                                                       uint32_t             index,
                                                                       uint32_t*            out_dep_image_id);

/**
 * Decoder parameter set count (VPS / SPS / PPS / APS).  Each entry
 * is a (type, payload) pair; `_decoder_param_set_type_at` returns
 * the `HEIF::DecoderSpecInfoType` numeric value (HEVC_VPS = 32,
 * HEVC_SPS = 33, HEVC_PPS = 34, VVC_VPS = 14, ...) and
 * `_decoder_param_set_data_at` fetches the payload bytes via the
 * size-probe convention.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_decoder_param_set_count(const heif_reader_t* reader,
                                                                          uint32_t             image_id,
                                                                          uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_decoder_param_set_type_at(const heif_reader_t* reader,
                                                                            uint32_t             image_id,
                                                                            uint32_t             index,
                                                                            uint8_t*             out_type);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_decoder_param_set_data_at(const heif_reader_t* reader,
                                                                            uint32_t             image_id,
                                                                            uint32_t             index,
                                                                            uint8_t*             buffer,
                                                                            uint64_t*            size_inout);

/**
 * As above but for the all-layers parameter-set bundle of a
 * multi-layer VVC item.  For non-VVC or single-layer items the
 * behavior is identical to the layer-specific accessors.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_decoder_param_set_all_layers_count(const heif_reader_t* reader,
                                                                                     uint32_t             image_id,
                                                                                     uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_decoder_param_set_all_layers_type_at(const heif_reader_t* reader,
                                                                                       uint32_t             image_id,
                                                                                       uint32_t             index,
                                                                                       uint8_t*             out_type);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_decoder_param_set_all_layers_data_at(const heif_reader_t* reader,
                                                                                       uint32_t             image_id,
                                                                                       uint32_t             index,
                                                                                       uint8_t*             buffer,
                                                                                       uint64_t*            size_inout);

/**
 * Item Protection Scheme box (`'sinf'`) for a protected item, as
 * raw bytes via the size-probe convention.  Only meaningful
 * when the item's `IsProtected` feature bit is set; otherwise
 * returns `UNPROTECTED_ITEM`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_item_protection_scheme(heif_reader_t* reader,
                                                                         uint32_t       image_id,
                                                                         uint8_t*       buffer,
                                                                         uint64_t*      size_inout);

/**
 * Compatible-brand combinations (`getCompatibleBrandCombinations`).
 * Each combination is itself a list of FourCC strings.  Walked via
 * a nested (_count, _at_count, _at_at) trio.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_compatible_brand_combo_count(const heif_reader_t* reader,
                                                                               uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_compatible_brand_combo_brand_count(const heif_reader_t* reader,
                                                                                     uint32_t             combo_index,
                                                                                     uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_compatible_brand_combo_brand_at(const heif_reader_t* reader,
                                                                                  uint32_t             combo_index,
                                                                                  uint32_t             brand_index,
                                                                                  char                 out_brand[5]);

/**
 * The 9-element file-level transformation matrix (movie-level mvhd
 * matrix) as a uint32_t count + index accessor.  Returns
 * `INVALID_FUNCTION_PARAMETER` on out-of-range index.  Element
 * order is the standard ISO/IEC 14496-12 (a, b, u, c, d, v, x, y, w).
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_matrix_at(const heif_reader_t* reader,
                                                            uint32_t             index,
                                                            int32_t*             out_value);

/* ------------------------------------------------------------------ */
/*  Stream-input initialize                                             */
/*    Lets a binding plug in an arbitrary read/seek/size source        */
/*    (network, BytesIO, zip-member, S3 object, ...) without having to   */
/*    materialise the whole file on disk first.                        */
/* ------------------------------------------------------------------ */

/**
 * Caller-side stream callback table.  This is a *callback* table,
 * not a wire payload, so the ban on function pointers in shared
 * structs does NOT apply (callback tables are an accepted pattern).
 * The library invokes these
 * callbacks while parsing the file and never retains the table
 * past the `heif_reader_initialize_stream` call's lifetime.
 *
 * All callbacks are called with the `user_data` pointer the
 * caller supplied here.  Callbacks MUST be thread-safe with
 * respect to *each other* on the same handle (the library does
 * not call them concurrently on a single reader).
 */
typedef struct heif_stream_callbacks
{
    uint32_t struct_size;            /**< MUST be sizeof(heif_stream_callbacks_t). */

    /** Read up to `size` bytes into `buffer`; return bytes read,
     *  0 on EOF, negative on error. */
    int64_t (*read)(void* user_data, char* buffer, int64_t size);

    /** Seek to absolute offset.  Return non-zero on success, 0 on
     *  failure.  Seeking past end of stream is permitted (per
     *  HEIF::StreamInterface contract); the next read returns 0. */
    int (*absolute_seek)(void* user_data, int64_t offset);

    /** Return current absolute offset, or negative on error. */
    int64_t (*tell)(void* user_data);

    /** Return total stream size, or `0x7fffffffffffffff` for an
     *  indeterminate size (network stream). */
    int64_t (*size)(void* user_data);

    /** Opaque caller pointer passed to every callback. */
    void* user_data;
} heif_stream_callbacks_t;
HEIF_STATIC_ASSERT(sizeof(heif_stream_callbacks_t) >= 4 + 5 * sizeof(void*),
                   "heif_stream_callbacks_t at least header + 5 pointers");

/**
 * Initialize a reader from an arbitrary stream described by a
 * caller-owned `heif_stream_callbacks_t` table.  The library
 * builds an internal `HEIF::StreamInterface` shim that forwards
 * each virtual call to the corresponding C callback; the shim is
 * destroyed by `heif_reader_destroy`.
 *
 * The callback table is *copied* into the reader during this
 * call -- the caller may free it immediately afterwards.
 * `user_data` is held by reference for the lifetime of the reader
 * and MUST remain valid until `heif_reader_destroy`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_initialize_stream(heif_reader_t*                  reader,
                                                                const heif_stream_callbacks_t* callbacks);

/* ------------------------------------------------------------------ */
/*  Long-tail descriptive / transformative properties                  */
/*    clap / iscl / rloc / auxC / rref / udes / altt / crtt / mdft /   */
/*    RawProperty.  All scalar property accessors plus size-probe string  */
/*    accessors for the variable-length fields.                       */
/* ------------------------------------------------------------------ */

typedef struct heif_clap
{
    uint32_t struct_size;            /**< MUST be sizeof(heif_clap_t). */
    uint32_t width_n;
    uint32_t width_d;
    uint32_t height_n;
    uint32_t height_d;
    int32_t  horizontal_offset_n;
    uint32_t horizontal_offset_d;
    int32_t  vertical_offset_n;
    uint32_t vertical_offset_d;
} heif_clap_t;
HEIF_STATIC_ASSERT(sizeof(heif_clap_t) == 36, "heif_clap_t ABI is 36 bytes");

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_clap(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                heif_clap_t*         out_clap);

/** Image scaling (`'iscl'`) -- target_width = N/D, target_height = N/D. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_iscl(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                uint16_t*            out_target_width_n,
                                                                uint16_t*            out_target_width_d,
                                                                uint16_t*            out_target_height_n,
                                                                uint16_t*            out_target_height_d);

/** Layer selector (`'lsel'`) -- selects a single layer of a multi-layer
 *  (e.g. L-HEVC) item.  Single 16-bit `layer_id` payload (ISO/IEC 23008-12). */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_lsel(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                uint16_t*            out_layer_id);

/** Target Output Layer Set (`'tols'`) -- selects which Output Layer Set
 *  enumerated by an associated `'oinf'` table the multi-layer
 *  (e.g. L-HEVC) image item targets.  Single 16-bit `target_ols_index`
 *  payload (ISO/IEC 23008-12). */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_tols(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                uint16_t*            out_target_ols_index);

/** Profile-Tier-Level entry from an `'oinf'` Operating-Points-Info property
 *  (ISO/IEC 23008-12, L-HEVC multi-layer).  Rule-8 POD. */
typedef struct heif_oinf_ptl
{
    uint32_t struct_size;
    uint8_t  general_profile_space;
    uint8_t  general_tier_flag;
    uint8_t  general_profile_idc;
    uint8_t  general_level_idc;
    uint32_t general_profile_compatibility_flags;
    uint64_t general_constraint_indicator_flags;
} heif_oinf_ptl_t;

HEIF_STATIC_ASSERT(sizeof(heif_oinf_ptl_t) == 24,
                   "heif_oinf_ptl_t ABI is exactly 24 bytes; "
                   "do not reorder/resize fields, append only");

/** Single layer entry inside an operating point (POD). */
typedef struct heif_oinf_op_layer
{
    uint32_t struct_size;
    uint8_t  ptl_idx;
    uint8_t  layer_id;
    uint8_t  is_output_layer;
    uint8_t  is_alternate_output_layer;
} heif_oinf_op_layer_t;

HEIF_STATIC_ASSERT(sizeof(heif_oinf_op_layer_t) == 8,
                   "heif_oinf_op_layer_t ABI is exactly 8 bytes; "
                   "do not reorder/resize fields, append only");

/** Operating point from an `'oinf'` property (POD).
 *  Nested layer entries are accessed via
 *  `heif_reader_get_property_oinf_op_layer_count` /
 *  `heif_reader_get_property_oinf_op_layer_at`. */
typedef struct heif_oinf_op
{
    uint32_t struct_size;
    uint32_t max_bit_rate;
    uint32_t avg_bit_rate;
    uint16_t output_layer_set_idx;
    uint16_t min_pic_width;
    uint16_t min_pic_height;
    uint16_t max_pic_width;
    uint16_t max_pic_height;
    uint16_t avg_frame_rate;
    uint8_t  max_temporal_id;
    uint8_t  max_chroma_format;
    uint8_t  max_bit_depth_minus8;
    uint8_t  frame_rate_info_flag;
    uint8_t  bit_rate_info_flag;
    uint8_t  constant_frame_rate;
} heif_oinf_op_t;

HEIF_STATIC_ASSERT(sizeof(heif_oinf_op_t) == 32,
                   "heif_oinf_op_t ABI is exactly 32 bytes; "
                   "do not reorder/resize fields, append only");

/** Operating-Points-Info (`'oinf'`) -- multi-layer operating point table
 *  (ISO/IEC 23008-12).
 *
 *  The top-level scalability mask and the four nested arrays are
 *  accessed through 13 entry points:
 *
 *  - `heif_reader_get_property_oinf` -- returns `scalability_mask`.
 *  - PTL array: `_oinf_ptl_count`, `_oinf_ptl_at`.
 *  - Operating-point array: `_oinf_op_count`, `_oinf_op_at`.
 *  - OP->layer (doubly nested): `_oinf_op_layer_count`, `_oinf_op_layer_at`.
 *  - Layer-info array: `_oinf_layer_count`, `_oinf_layer_at` (returns
 *    `dependent_layer_id`).
 *  - Layer-info->dependency IDs: `_oinf_layer_dep_count`, `_oinf_layer_dep_at`.
 *  - Layer-info->dimension IDs: `_oinf_layer_dim_count`, `_oinf_layer_dim_at`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_oinf(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                uint16_t*            out_scalability_mask);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_oinf_ptl_count(const heif_reader_t* reader,
                                                                          uint32_t             property_id,
                                                                          uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_oinf_ptl_at(const heif_reader_t* reader,
                                                                       uint32_t             property_id,
                                                                       uint32_t             index,
                                                                       heif_oinf_ptl_t*     out_ptl);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_oinf_op_count(const heif_reader_t* reader,
                                                                         uint32_t             property_id,
                                                                         uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_oinf_op_at(const heif_reader_t* reader,
                                                                      uint32_t             property_id,
                                                                      uint32_t             index,
                                                                      heif_oinf_op_t*      out_op);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_oinf_op_layer_count(const heif_reader_t* reader,
                                                                               uint32_t             property_id,
                                                                               uint32_t             op_index,
                                                                               uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_oinf_op_layer_at(const heif_reader_t* reader,
                                                                            uint32_t             property_id,
                                                                            uint32_t             op_index,
                                                                            uint32_t             layer_index,
                                                                            heif_oinf_op_layer_t* out_layer);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_oinf_layer_count(const heif_reader_t* reader,
                                                                            uint32_t             property_id,
                                                                            uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_oinf_layer_at(const heif_reader_t* reader,
                                                                         uint32_t             property_id,
                                                                         uint32_t             index,
                                                                         uint8_t*             out_dependent_layer_id);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_oinf_layer_dep_count(const heif_reader_t* reader,
                                                                                uint32_t             property_id,
                                                                                uint32_t             layer_index,
                                                                                uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_oinf_layer_dep_at(const heif_reader_t* reader,
                                                                             uint32_t             property_id,
                                                                             uint32_t             layer_index,
                                                                             uint32_t             dep_index,
                                                                             uint8_t*             out_dep_layer_id);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_oinf_layer_dim_count(const heif_reader_t* reader,
                                                                                uint32_t             property_id,
                                                                                uint32_t             layer_index,
                                                                                uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_oinf_layer_dim_at(const heif_reader_t* reader,
                                                                             uint32_t             property_id,
                                                                             uint32_t             layer_index,
                                                                             uint32_t             dim_index,
                                                                             uint8_t*             out_dimension_id);

/* ------------------------------------------------------------------ */
/* 'subs' -- Sub-sample Information                                     */
/* ------------------------------------------------------------------ */

/** Sub-sample entry returned by `heif_reader_get_property_subs_at`. */
typedef struct heif_subs_entry_t
{
    /** Size of this struct in bytes (caller sets to `sizeof`). */
    uint32_t struct_size;

    /** Byte count of this sub-sample. */
    uint32_t subsample_size;
    /** Lower values are more important. */
    uint8_t  subsample_priority;
    /** Non-zero if this sub-sample may be discarded. */
    uint8_t  discardable;

    uint8_t  _reserved_pad0;
    uint8_t  _reserved_pad1;

    /** Codec-defined semantics. */
    uint32_t codec_specific_parameters;
} heif_subs_entry_t;
HEIF_STATIC_ASSERT(sizeof(heif_subs_entry_t) == 16, "heif_subs_entry_t size pin");

/** @return Number of sub-sample entries in a `'subs'` property. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_subs_count(const heif_reader_t* reader,
                                                                         uint32_t             property_id,
                                                                         uint32_t*            out_count);

/** @return The sub-sample entry at @p index. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_subs_at(const heif_reader_t* reader,
                                                                      uint32_t             property_id,
                                                                      uint32_t             index,
                                                                      heif_subs_entry_t*   out_entry);

/** Relative location (`'rloc'`) -- pixel offset of a tile within its base. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_rloc(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                uint32_t*            out_horizontal_offset,
                                                                uint32_t*            out_vertical_offset);

/** Auxiliary type (`'auxC'`) -- type URN string (e.g.
 *  "urn:mpeg:hevc:2015:auxid:1" for alpha) and codec-specific subtype bytes.
 *  Both use the size-probe convention.  The type buffer is
 *  NUL-terminated; the subtype buffer is raw bytes (NOT NUL-terminated). */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_auxc_type(const heif_reader_t* reader,
                                                                     uint32_t             property_id,
                                                                     char*                buffer,
                                                                     uint64_t*            size_inout);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_auxc_subtype(const heif_reader_t* reader,
                                                                        uint32_t             property_id,
                                                                        uint8_t*             buffer,
                                                                        uint64_t*            size_inout);

/** Required reference types (`'rref'`) -- list of FourCC reference types
 *  the reader must understand to decode this item. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_rref_count(const heif_reader_t* reader,
                                                                      uint32_t             property_id,
                                                                      uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_rref_at(const heif_reader_t* reader,
                                                                   uint32_t             property_id,
                                                                   uint32_t             index,
                                                                   char                 out_type[5]);

/** User description (`'udes'`) -- four UTF-8 string fields:
 *  field index 0 = lang, 1 = name, 2 = description, 3 = tags. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_udes_field(const heif_reader_t* reader,
                                                                      uint32_t             property_id,
                                                                      uint32_t             field_index,
                                                                      char*                buffer,
                                                                      uint64_t*            size_inout);

/** Accessibility text (`'altt'`) -- 0 = text, 1 = lang. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_altt_field(const heif_reader_t* reader,
                                                                      uint32_t             property_id,
                                                                      uint32_t             field_index,
                                                                      char*                buffer,
                                                                      uint64_t*            size_inout);

/** Creation time (`'crtt'`) -- microseconds since 1904-01-01 UTC. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_crtt(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                uint64_t*            out_time_us);

/** Modification time (`'mdft'`) -- microseconds since 1904-01-01 UTC. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_mdft(const heif_reader_t* reader,
                                                                uint32_t             property_id,
                                                                uint64_t*            out_time_us);

/** Raw / unrecognised property -- its FourCC type and full box payload
 *  (including the box header).  Use after `_get_item_property_at` returns
 *  the `"raw "` sentinel for the FourCC. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_raw_type(const heif_reader_t* reader,
                                                                    uint32_t             property_id,
                                                                    char                 out_type[5]);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_property_raw_data(const heif_reader_t* reader,
                                                                    uint32_t             property_id,
                                                                    uint8_t*             buffer,
                                                                    uint64_t*            size_inout);

/* ------------------------------------------------------------------ */
/*  Entity groups                                                       */
/*    `GroupsListBox` payload: stereo pairs, alternate groups,         */
/*    equivalence groups, etc.  Walked via the standard (_count, _at)  */
/*    pair plus a per-group entity-id list.                            */
/* ------------------------------------------------------------------ */

typedef struct heif_entity_group
{
    uint32_t struct_size;            /**< MUST be sizeof(heif_entity_group_t). */
    uint32_t group_id;
    char     type[5];                /**< 4-CC + NUL. */
    uint8_t  _reserved[3];           /**< Pad to 4-byte alignment. */
} heif_entity_group_t;
HEIF_STATIC_ASSERT(sizeof(heif_entity_group_t) == 16, "heif_entity_group_t ABI is 16 bytes");

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_entity_group_count(const heif_reader_t* reader,
                                                                     uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_entity_group_at(const heif_reader_t* reader,
                                                                  uint32_t             index,
                                                                  heif_entity_group_t* out_group);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_entity_group_entity_count(const heif_reader_t* reader,
                                                                            uint32_t             group_index,
                                                                            uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_entity_group_entity_at(const heif_reader_t* reader,
                                                                         uint32_t             group_index,
                                                                         uint32_t             entity_index,
                                                                         uint32_t*            out_entity_id);

/** Entity-group properties (analogue of `_get_item_property_count` but
 *  keyed on a group_id from `heif_entity_group_t.group_id`). */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_group_property_count(const heif_reader_t* reader,
                                                                       uint32_t             group_id,
                                                                       uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_group_property_at(const heif_reader_t* reader,
                                                                    uint32_t             group_id,
                                                                    uint32_t             index,
                                                                    uint32_t*            out_property_id,
                                                                    char                 out_type[5],
                                                                    int*                 out_essential);

/* ------------------------------------------------------------------ */
/*  Track / sample surface                                             */
/*    Mirrors HEIF::Reader's sequence/track virtuals                   */
/*    (getTrackInformations, getDisplayWidth/Height,                   */
/*    getPlaybackDurationInSecs, getItemTimestamps,                    */
/*    getTimestampsOfItem, getSegmentIndex).                           */
/*    PODs are caller-allocated POD structs (leading struct_size).  */
/*    Nested Array<...> members of the C++ source structs are reached  */
/*    via standard (_count, _at) pairs.                       */
/*    Per-grouping POD names mirror the four-CC verbatim               */
/*    (heif_sample_eqiv_t, _stmi_t, _refs_t, _trif_t, _spor_t,         */
/*    _sulm_t, _minp_t).  SampleFlags is exposed via the spec-bit      */
/*    inline accessors below (the C++ bitfield layout is               */
/*    implementation-defined and not safe to share across compilers).  */
/* ------------------------------------------------------------------ */

/**
 * Per-track scalars.  Mirrors the non-`Array<...>` fields of
 * `HEIF::TrackInformation` plus the leading `TrackTypeInformation`
 * scalars (major brand + minor version).  Companion accessors for
 * `compatible_brands`, `alternate_track_ids`, `reference_track_ids`,
 * `track_group_ids`, `sample_groups`, sample iteration, edit list,
 * and the seven sample-groupings exist as separate
 * `_count`/`_at` pairs.
 *
 * Snapshot semantics.  Reflects whatever the reader has parsed so
 * far.  For non-fragmented input every field is final after
 * `heif_reader_initialize_*`.  For the fragmented streaming flow
 * (`parseInitializationSegment` + `parseSegment`, currently only
 * reachable via the C++ `Reader` API) the sample table and the
 * per-sample fields on `heif_sample_info_t` grow as media segments
 * are fed.
 */
typedef struct heif_track_info
{
    uint32_t struct_size;                /**< MUST be sizeof(heif_track_info_t). */
    uint32_t alternate_group_id;
    uint64_t max_sample_size;            /**< Largest sample byte size; useful for buffer sizing. */
    uint32_t track_id;
    uint32_t features;                   /**< Bitmask of HEIF::TrackFeatureEnum::Feature. */
    uint32_t time_scale;                 /**< Track-level timescale (ticks/sec). */
    uint32_t frame_rate_num;             /**< Library-derived approximate frame rate; numerator (HEIF::Rational::num).
                                          *   See `HEIF::TrackInformation::frameRate` for the full contract.
                                          *   Summary: not a spec field (ISOBMFF stores per-sample
                                          *   durations, not fps); computed as `mdhd.timescale /
                                          *   avg(sample_duration)`; numerator left unreduced so the
                                          *   source clock survives round-tripping; edit-list rate
                                          *   scaling is NOT folded in (raw media-rate); `{0, 0}`
                                          *   means the library could not derive a value (e.g. empty
                                          *   track).  For variable-rate tracks this is an average;
                                          *   per-sample truth is in `heif_sample_info_t.sample_duration_ts`. */
    uint32_t frame_rate_den;             /**< Library-derived frame-rate denominator (HEIF::Rational::den).
                                          *   See `frame_rate_num` for the full contract. */
    uint32_t minor_version;              /**< From TrackTypeInformation; 0 if !has_type_information. */
    uint8_t  has_type_information;       /**< 0/1: TrackTypeInformation fields are populated. */
    uint8_t  _reserved[3];
    char     major_brand[5];             /**< 4-CC + NUL; zeroed if !has_type_information. */
    uint8_t  _reserved2[7];
} heif_track_info_t;
HEIF_STATIC_ASSERT(sizeof(heif_track_info_t) == 56,
                   "heif_track_info_t ABI is 56 bytes; append-only");

/**
 * Per-sample scalars.  Mirrors `HEIF::SampleInformation` field-for-
 * field; `coding_constraints` packs the three-field
 * `HEIF::CodingConstraints` struct into a single byte (bit 0 =
 * allRefPicsIntra, bit 1 = intraPredUsed, bits 7..2 = maxRefPerPic
 * clamped to 0..63 -- value 15 in the C++ enum still surfaces as the
 * literal byte 15 because the spec's "15 = any" sentinel is < 64).
 *
 * `sample_flags` is the verbatim 32-bit `SampleFlags::flagsAsUInt`
 * value as written in the file (ISO/IEC 14496-12 Section 8.8.3.1); use the
 * `heif_sample_flags_*` inline accessors below to extract individual
 * fields portably.
 *
 * Per-segment fields (`segment_id`, `earliest_timestamp`,
 * `earliest_timestamp_ts`, `sample_flags`) only carry useful data
 * for samples drawn from a parsed media segment in the fragmented
 * streaming flow.  Samples sourced from a non-fragmented moov
 * sample table report `segment_id == 0` and zeroed earliest-PTS
 * fields; derive a timeline from `sample_duration_ts` accumulation
 * if you need one.
 */
typedef struct heif_sample_info
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_sample_info_t). */
    uint32_t sample_id;                         /**< SequenceImageId (sample-table entry order). */
    uint64_t sample_duration_ts;                /**< Sample duration in track timescale units. */
    uint64_t size;                              /**< Sample data byte size. */
    uint64_t earliest_timestamp;                /**< Earliest timestamp (milliseconds). */
    uint64_t earliest_timestamp_ts;             /**< Earliest timestamp in track timescale units. */
    int64_t  sample_composition_offset_ts;      /**< Composition-time offset in timescale units. */
    uint32_t sample_description_index;          /**< 1-based index into the sample-description box. */
    uint32_t segment_id;                        /**< In which segment the sample is located. */
    uint32_t sample_flags;                      /**< Verbatim SampleFlags::flagsAsUInt; see heif_sample_flags_*. */
    uint8_t  sample_type;                       /**< HEIF::SampleType enum (0..2). */
    uint8_t  has_clap;                          /**< 0/1: CleanApertureBox present in sample entry. */
    uint8_t  has_auxi;                          /**< 0/1: AuxiliaryTypeInfoBox present in sample entry. */
    uint8_t  coding_constraints;                /**< Packed CodingConstraints (see field doc). */
    char     sample_entry_type[5];              /**< codingname FourCC + NUL. */
    uint8_t  _reserved[3];
} heif_sample_info_t;
HEIF_STATIC_ASSERT(sizeof(heif_sample_info_t) == 72,
                   "heif_sample_info_t ABI is 72 bytes; append-only");

/**
 * Single edit-list unit.  Mirrors `HEIF::EditUnit`; the wrapper
 * scalars (`looping`, `repetitions`) live on the EditList itself
 * and are surfaced separately via
 * `heif_reader_get_track_edit_list_meta`.
 */
typedef struct heif_edit_unit
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_edit_unit_t). */
    uint8_t  edit_type;                         /**< HEIF::EditType enum (EMPTY=0, DWELL=1, SHIFT=2, RAW=3). */
    uint8_t  _reserved[3];
    int64_t  media_time_in_track_ts;            /**< Edit-time in track timescale units. */
    uint64_t duration_in_movie_ts;              /**< Edit length in movie timescale units (writer uses 1000). */
    int16_t  media_rate_integer;                /**< Integer part of media_rate (16.16 fixed-point). */
    int16_t  media_rate_fraction;               /**< Fraction part of media_rate (16.16 fixed-point). */
    uint8_t  _reserved2[4];
} heif_edit_unit_t;
HEIF_STATIC_ASSERT(sizeof(heif_edit_unit_t) == 32,
                   "heif_edit_unit_t ABI is 32 bytes; append-only");

/**
 * Per-segment information from a DASH ISOBMFF On-Demand profile
 * `sidx` box.  Mirrors `HEIF::SegmentInformation`.
 */
typedef struct heif_segment_info
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_segment_info_t). */
    uint32_t segment_id;
    uint64_t earliest_pts_in_ts;                /**< Earliest PTS in `timescale` units. */
    uint64_t start_data_offset;                 /**< Byte offset of the segment from file start. */
    uint32_t reference_id;                      /**< Stream ID for the reference stream. */
    uint32_t timescale;                         /**< Ticks per second for earliestPTS / duration. */
    uint32_t duration_in_ts;                    /**< Byte-range duration in `timescale` units. */
    uint32_t data_size;                         /**< Distance in bytes from start_data_offset to segment end. */
    uint8_t  reference_type;                    /**< 0/1: 1 = other sidx box, 0 = media content. */
    uint8_t  starts_with_sap;                   /**< 0/1: segment starts with a Stream Access Point. */
    uint8_t  sap_type;                          /**< SAP type per ISO/IEC 14496-12 Section 8.16.3.3. */
    uint8_t  _reserved[5];
} heif_segment_info_t;
HEIF_STATIC_ASSERT(sizeof(heif_segment_info_t) == 48,
                   "heif_segment_info_t ABI is 48 bytes; append-only");

/**
 * `'eqiv'` sample-grouping entry -- mirror of
 * `HEIF::SampleVisualEquivalence`.  No nested arrays.
 */
typedef struct heif_sample_eqiv
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_sample_eqiv_t). */
    uint32_t sample_group_description_index;
    int16_t  time_offset;
    uint16_t timescale_multiplier;              /**< 8.8 fixed-point timeOffset multiplier. */
} heif_sample_eqiv_t;
HEIF_STATIC_ASSERT(sizeof(heif_sample_eqiv_t) == 12,
                   "heif_sample_eqiv_t ABI is 12 bytes; append-only");

/**
 * `'stmi'` sample-grouping scalars.  The variable-length
 * `metadataItemIds` array is reachable via
 * `heif_reader_get_track_sample_stmi_metadata_item_count` / `_at`.
 */
typedef struct heif_sample_stmi
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_sample_stmi_t). */
    uint32_t sample_group_description_index;
} heif_sample_stmi_t;
HEIF_STATIC_ASSERT(sizeof(heif_sample_stmi_t) == 8,
                   "heif_sample_stmi_t ABI is 8 bytes; append-only");

/**
 * `'refs'` sample-grouping scalars.  The variable-length
 * `referenceItemIds` array is reachable via
 * `heif_reader_get_track_sample_refs_reference_item_count` / `_at`.
 */
typedef struct heif_sample_refs
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_sample_refs_t). */
    uint32_t sample_group_description_index;
    uint32_t sample_id;                         /**< 0 for non-reference samples. */
    uint8_t  _reserved[4];
} heif_sample_refs_t;
HEIF_STATIC_ASSERT(sizeof(heif_sample_refs_t) == 16,
                   "heif_sample_refs_t ABI is 16 bytes; append-only");

/**
 * `'trif'` sample-grouping (RectangularRegionGroupEntry) scalars.
 * Mirror of `HEIF::Trif` minus the trailing
 * `dependencyRectRegionGroupIds` array, which is reachable via
 * `heif_reader_get_track_sample_trif_dependency_count` / `_at`.
 */
typedef struct heif_sample_trif
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_sample_trif_t). */
    uint32_t sample_group_description_index;
    uint16_t group_id;
    uint16_t independent_idc;
    uint16_t horizontal_offset;
    uint16_t vertical_offset;
    uint16_t region_width;
    uint16_t region_height;
    uint8_t  rect_region_flag;                  /**< 0/1. */
    uint8_t  full_picture;                      /**< 0/1. */
    uint8_t  filtering_disabled;                /**< 0/1. */
    uint8_t  _reserved;
} heif_sample_trif_t;
HEIF_STATIC_ASSERT(sizeof(heif_sample_trif_t) == 24,
                   "heif_sample_trif_t ABI is 24 bytes; append-only");

/**
 * `'spor'` sample-grouping (VvcSubpicOrderEntry) scalars.  The
 * variable-length `subpTrackRefIdx` array is reachable via
 * `heif_reader_get_track_sample_spor_subp_track_ref_idx_count` / `_at`.
 */
typedef struct heif_sample_spor
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_sample_spor_t). */
    uint32_t sample_group_description_index;
    uint16_t subpic_id_bit_pos;
    uint8_t  subpic_id_info_flag;               /**< 0/1. */
    uint8_t  subpicd_len_minus1;
    uint8_t  start_code_emul_flag;              /**< 0/1; meaningful iff subpic_id_info_flag. */
    uint8_t  pps_sps_subpic_id_flag;            /**< 0/1; meaningful iff subpic_id_info_flag. */
    uint8_t  pps_id;                            /**< Meaningful iff subpic_id_info_flag && pps_sps_subpic_id_flag. */
    uint8_t  sps_id;                            /**< Meaningful iff subpic_id_info_flag && !pps_sps_subpic_id_flag. */
} heif_sample_spor_t;
HEIF_STATIC_ASSERT(sizeof(heif_sample_spor_t) == 16,
                   "heif_sample_spor_t ABI is 16 bytes; append-only");

/**
 * `'sulm'` sample-grouping (VvcSubpicLayoutMapEntry) scalars.  The
 * variable-length `groupIds` array is reachable via
 * `heif_reader_get_track_sample_sulm_group_id_count` / `_at`.
 */
typedef struct heif_sample_sulm
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_sample_sulm_t). */
    uint32_t sample_group_description_index;
    char     group_id_info_4cc[5];              /**< FourCC + NUL. */
    uint8_t  _reserved[3];
} heif_sample_sulm_t;
HEIF_STATIC_ASSERT(sizeof(heif_sample_sulm_t) == 16,
                   "heif_sample_sulm_t ABI is 16 bytes; append-only");

/**
 * `'minp'` sample-grouping (VvcMixedNALUnitTypePicEntry) scalars.
 * The two variable-length `mixSubpTrackIdx1` / `mixSubpTrackIdx2`
 * arrays are reachable via the matching
 * `_track_sample_minp_mix_subp_track_idx{1,2}_count` / `_at` pairs.
 */
typedef struct heif_sample_minp
{
    uint32_t struct_size;                       /**< MUST be sizeof(heif_sample_minp_t). */
    uint32_t sample_group_description_index;
    uint16_t pps_mix_nalu_types_in_pic_bit_pos;
    uint8_t  pps_id;
    uint8_t  _reserved;
} heif_sample_minp_t;
HEIF_STATIC_ASSERT(sizeof(heif_sample_minp_t) == 12,
                   "heif_sample_minp_t ABI is 12 bytes; append-only");

/* ------------------------------------------------------------------ */
/*  SampleFlags inline accessors                                       */
/*    Spec bit positions per ISO/IEC 14496-12 Section 8.8.3.1 (sample_flags   */
/*    field of TrackRunBox / TrackFragmentHeaderBox).  The 32-bit      */
/*    word, MSB-first, decomposes as:                                  */
/*      bits 31..28 (4) : reserved                                     */
/*      bits 27..26 (2) : is_leading                                   */
/*      bits 25..24 (2) : sample_depends_on                            */
/*      bits 23..22 (2) : sample_is_depended_on                        */
/*      bits 21..20 (2) : sample_has_redundancy                        */
/*      bits 19..17 (3) : sample_padding_value                         */
/*      bit  16     (1) : sample_is_non_sync_sample                    */
/*      bits 15..0  (16): sample_degradation_priority                  */
/*    These accessors operate on the spec-defined wire format          */
/*    (heif_sample_info_t::sample_flags); the C++ SampleFlagsType      */
/*    bitfield layout is implementation-defined and MUST NOT be        */
/*    assumed by bindings.                                             */
/* ------------------------------------------------------------------ */

static inline uint32_t heif_sample_flags_is_leading(uint32_t flags)
{
    return (flags >> 26) & 0x3u;
}
static inline uint32_t heif_sample_flags_sample_depends_on(uint32_t flags)
{
    return (flags >> 24) & 0x3u;
}
static inline uint32_t heif_sample_flags_sample_is_depended_on(uint32_t flags)
{
    return (flags >> 22) & 0x3u;
}
static inline uint32_t heif_sample_flags_sample_has_redundancy(uint32_t flags)
{
    return (flags >> 20) & 0x3u;
}
static inline uint32_t heif_sample_flags_sample_padding_value(uint32_t flags)
{
    return (flags >> 17) & 0x7u;
}
static inline uint32_t heif_sample_flags_sample_is_non_sync_sample(uint32_t flags)
{
    return (flags >> 16) & 0x1u;
}
static inline uint32_t heif_sample_flags_sample_degradation_priority(uint32_t flags)
{
    return flags & 0xFFFFu;
}

/** Number of tracks recognised by the reader.  See `getTrackInformations`
 *  doc on heifreader.h for the four supported handler types. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_count(const heif_reader_t* reader,
                                                              uint32_t*            out_count);

/** Track ID at the given index (0 .. count-1).  The returned
 *  `track_id` is the raw `uint32_t` payload of `HEIF::SequenceId`. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_id_at(const heif_reader_t* reader,
                                                              uint32_t             index,
                                                              uint32_t*            out_track_id);

/** Per-track scalars.  Caller MUST set
 *  `out_info->struct_size = sizeof(heif_track_info_t)` before the call.
 *  All `Array<...>` members of `HEIF::TrackInformation` are reached
 *  via the companion `_count`/`_at` accessors below.
 *
 *  Backed by the cached `getFileInformation` snapshot; see
 *  `heif_track_info_t` doc for snapshot semantics. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_info(const heif_reader_t* reader,
                                                             uint32_t             track_id,
                                                             heif_track_info_t*   out_info);

/** Maximum display width from the track header.  Mirrors
 *  `HEIF::Reader::getDisplayWidth`. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_display_width(const heif_reader_t* reader,
                                                                      uint32_t             track_id,
                                                                      uint32_t*            out_width);

/** Maximum display height from the track header.  Mirrors
 *  `HEIF::Reader::getDisplayHeight`. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_display_height(const heif_reader_t* reader,
                                                                       uint32_t             track_id,
                                                                       uint32_t*            out_height);

/** Playback duration in seconds, honouring the edit list.  Mirrors
 *  `HEIF::Reader::getPlaybackDurationInSecs`. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_playback_duration_secs(const heif_reader_t* reader,
                                                                               uint32_t             track_id,
                                                                               double*              out_secs);

/** Count + at: alternate track IDs sharing this track's `alternateGroupId`. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_alternate_track_count(const heif_reader_t* reader,
                                                                              uint32_t             track_id,
                                                                              uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_alternate_track_at(const heif_reader_t* reader,
                                                                           uint32_t             track_id,
                                                                           uint32_t             index,
                                                                           uint32_t*            out_alt_track_id);

/** Count + at: `(reference_type, referenced_track_id)` pairs flattened
 *  across `TrackInformation::referenceTrackIds`.  Pair order matches
 *  the C++ array order (group-by-type, then per-type ID order). */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_reference_count(const heif_reader_t* reader,
                                                                        uint32_t             track_id,
                                                                        uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_reference_at(const heif_reader_t* reader,
                                                                     uint32_t             track_id,
                                                                     uint32_t             index,
                                                                     char                 out_reference_type[5],
                                                                     uint32_t*            out_referenced_track_id);

/** Count + at: `(group_type, member_track_id)` pairs flattened across
 *  `TrackInformation::trackGroupIds`.  Same shape as `_track_reference_*`. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_group_count(const heif_reader_t* reader,
                                                                    uint32_t             track_id,
                                                                    uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_group_at(const heif_reader_t* reader,
                                                                 uint32_t             track_id,
                                                                 uint32_t             index,
                                                                 char                 out_group_type[5],
                                                                 uint32_t*            out_member_track_id);

/** Count + at: per-track compatible brands from `TrackTypeInformation`.
 *  Returns count = 0 when `has_type_information` is 0. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_compatible_brand_count(const heif_reader_t* reader,
                                                                               uint32_t             track_id,
                                                                               uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_compatible_brand_at(const heif_reader_t* reader,
                                                                            uint32_t             track_id,
                                                                            uint32_t             index,
                                                                            char                 out_brand[5]);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_count(const heif_reader_t* reader,
                                                                     uint32_t             track_id,
                                                                     uint32_t*            out_count);

/** Caller MUST set `out_info->struct_size = sizeof(heif_sample_info_t)`
 *  before the call.  Backed by the cached `getFileInformation`
 *  snapshot; see `heif_sample_info_t` doc for the per-segment-field
 *  caveat in the fragmented streaming flow. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_at(const heif_reader_t* reader,
                                                                  uint32_t             track_id,
                                                                  uint32_t             index,
                                                                  heif_sample_info_t*  out_info);

/**
 * Extract a decoder-ready bitstream for one sample of an image
 * sequence track.  Mirrors `HEIF::Reader::getItemDataWithDecoderParameters(
 * const SequenceId&, const SequenceImageId&, uint8_t*, uint64_t&)`.
 * On success the buffer contains the decoder configuration record(s)
 * for the sample's description entry followed by the sample payload
 * with bytestream NAL headers (`0x000001`) substituted for
 * `nal_length` fields.  Use this when feeding a hardware/software
 * H.264 / H.265 / VVC decoder.
 *
 * `track_id` is a raw `SequenceId` (matches `heif_track_info_t.track_id`
 * and `heif_reader_get_track_id_at`).  `sample_id` is a raw
 * `SequenceImageId` from sample-table order -- it is **not** an item
 * id from the `'meta'` box and the two id spaces are disjoint.  Use
 * `heif_reader_get_track_sample_at` to enumerate valid sample ids.
 *
 * Same `(buffer, *size_inout)` size-probe convention as
 * `heif_reader_get_item_data_with_decoder_parameters` -- see that
 * function's documentation for the recommended call pattern.
 * **Note** -- the underlying C++ track accessor surfaces the probe
 * miss as `MEMORY_TOO_SMALL_BUFFER` rather than the item-side
 * spelling `BUFFER_SIZE_TOO_SMALL`; treat both as equivalent on
 * this entry point.  As an alternative to probing, callers may
 * pre-allocate using `heif_sample_info_t.size` plus a few KiB of
 * headroom for the inlined decoder configuration record(s) and
 * skip the probe entirely.
 *
 * @param [in]      reader      Reader handle.
 * @param [in]      track_id    Sequence (track) id.
 * @param [in]      sample_id   Sample id within the track.
 * @param [out]     buffer      Destination buffer, or `NULL` for a
 *                              size probe.  Caller-owned; library
 *                              never frees it.
 * @param [in,out]  size_inout  On entry: capacity of `buffer` in
 *                              bytes (0 for a probe).  On exit: on
 *                              `OK` the bytes actually written; on
 *                              `MEMORY_TOO_SMALL_BUFFER` the bytes
 *                              required.
 *
 * @return `0` (`OK`) on success; `MEMORY_TOO_SMALL_BUFFER` (or
 *         `BUFFER_SIZE_TOO_SMALL`) on probe; `UNINITIALIZED`,
 *         `INVALID_SEQUENCE_ID`, `INVALID_SEQUENCE_IMAGE_ID`,
 *         `UNSUPPORTED_CODE_TYPE` on permanent failure.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_data_with_decoder_parameters(
    heif_reader_t* reader,
    uint32_t       track_id,
    uint32_t       sample_id,
    uint8_t*       buffer,
    uint64_t*      size_inout);

/**
 * Extract raw sample bytes for one sample of an image sequence track.
 * Mirrors `HEIF::Reader::getItemData(const SequenceId&,
 * const SequenceImageId&, uint8_t*, uint64_t&, bool bytestreamHeaders=true)`.
 * Returns the on-wire sample payload **without** the decoder
 * configuration record prefix; for `'hvc1'` / `'avc1'` the
 * `nal_length` fields are substituted with bytestream NAL headers
 * (`0x000001`) by default.  For protected samples the raw bytes are
 * returned as-is; consult the track's protection-scheme info for
 * decryption metadata.
 *
 * Use `heif_reader_get_track_sample_data_with_decoder_parameters`
 * instead when you intend to decode the bitstream -- that variant
 * prepends the decoder configuration record(s) the sample's
 * description entry references.
 *
 * Uses the same `(buffer, *size_inout)` size-probe convention as
 * `heif_reader_get_item_data_with_decoder_parameters`; see that
 * function's documentation for the recommended call pattern.
 * **Note** -- the underlying C++ implementation surfaces the probe
 * miss as `MEMORY_TOO_SMALL_BUFFER` rather than the item-side
 * spelling `BUFFER_SIZE_TOO_SMALL`; treat both as equivalent on
 * this entry point.  The required size **is** propagated into
 * `*size_inout` on probe (unlike the `_with_decoder_parameters`
 * sibling), so probe-and-fetch works end-to-end here.  As an
 * alternative, callers may pre-allocate using
 * `heif_sample_info_t.size` and skip the probe entirely.
 *
 * @param [in]      reader      Reader handle.
 * @param [in]      track_id    Sequence (track) id.
 * @param [in]      sample_id   Sample id within the track.
 * @param [out]     buffer      Destination buffer, or `NULL` for a
 *                              size probe.
 * @param [in,out]  size_inout  On entry: capacity of `buffer` in
 *                              bytes (0 for a probe).  On exit: on
 *                              `OK` the bytes actually written; on
 *                              `MEMORY_TOO_SMALL_BUFFER` the bytes
 *                              required.
 *
 * @return `0` (`OK`) on success; `MEMORY_TOO_SMALL_BUFFER` (or
 *         `BUFFER_SIZE_TOO_SMALL`) on probe; `UNINITIALIZED`,
 *         `INVALID_SEQUENCE_ID`, `INVALID_SEQUENCE_IMAGE_ID` on
 *         permanent failure.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_data(heif_reader_t* reader,
                                                                    uint32_t       track_id,
                                                                    uint32_t       sample_id,
                                                                    uint8_t*       buffer,
                                                                    uint64_t*      size_inout);

/**
 * Decoder code type FourCC for one sample of an image sequence
 * track: e.g. `"hvc1"` vs `"hev1"`, `"avc1"` vs `"avc3"`.  Mirrors
 * the item-side `heif_reader_get_decoder_code_type`.  A sequence
 * may carry samples that share a sample-entry FourCC but differ in
 * decoder code type (e.g. mid-stream `'hvc1'` / `'hev1'` switch);
 * call this per sample if you intend to dispatch on it.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_decoder_code_type(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             sample_id,
    char                 out_type[5]);

/**
 * Sample-description-entry FourCC for one sample of an image
 * sequence track.  Mirrors `Reader::getItemType(SequenceId,
 * SequenceImageId, FourCC&)`.  This is the per-sample sample-entry
 * type from the sample-table sample-description box (e.g. `"hvc1"`,
 * `"avc1"`, `"vvc1"`, `"jpeg"`); contrast with
 * `heif_reader_get_track_sample_decoder_code_type`, which returns
 * the sample's decoder-code-type FourCC and may differ when the
 * sample-entry uses a configuration alias (`"hvc1"` sample-entry
 * carrying `"hev1"` decoder-config, etc.).  Bindings dispatching on
 * the sample-entry box type (e.g. choosing a parser for the
 * decoder configuration record) want this; bindings dispatching on
 * the bitstream framing want `_decoder_code_type`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_item_type(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             sample_id,
    char                 out_type[5]);

/**
 * Decoder parameter set count for one sample of an image sequence
 * track.  Mirrors the item-side
 * `heif_reader_get_decoder_param_set_count` trio.  Each entry is a
 * `(type, payload)` pair; `_type_at` returns the
 * `HEIF::DecoderSpecInfoType` numeric value and `_data_at` fetches
 * the payload bytes via the size-probe convention.
 *
 * Use this when feeding parameter sets out-of-band (e.g.
 * WebCodecs `VideoDecoder.configure({description: ...})`) instead
 * of relying on the inline-prefix bitstream returned by
 * `heif_reader_get_track_sample_data_with_decoder_parameters`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_decoder_param_set_count(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             sample_id,
    uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_decoder_param_set_type_at(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             sample_id,
    uint32_t             index,
    uint8_t*             out_type);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_decoder_param_set_data_at(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             sample_id,
    uint32_t             index,
    uint8_t*             buffer,
    uint64_t*            size_inout);

/**
 * Decoding-order list for an image sequence track.  Mirrors
 * `HEIF::Reader::getItemsInDecodingOrder`.  Sample-table order
 * (returned by `heif_reader_get_track_sample_at`) is presentation
 * order; samples must be **decoded** in the order returned here,
 * which differs whenever B-frames or non-zero sample composition
 * offsets exist.  Each entry is a `(timestamp_ms, sample_id)` pair
 * -- same shape as `heif_reader_get_track_item_timestamp_*`, just
 * walking the decoding-order list instead of the
 * presentation-order list.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_decoding_order_count(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_decoding_order_at(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             index,
    int64_t*             out_timestamp_ms,
    uint32_t*            out_sample_id);

/**
 * Decode-dependency list for one sample of an image sequence
 * track: sample ids whose decoded output must be available before
 * this sample can be decoded.  Mirrors the item-side
 * `heif_reader_get_decode_dependency_*`.  Required for seek/scrub:
 * after seeking to sample N, feed every sample reported here into
 * the decoder before sample N itself.  Independently-decodable
 * samples (typically sync samples) return count = 0 (or count = 1
 * containing the sample id itself, mirroring the C++ contract).
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_decode_dependency_count(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             sample_id,
    uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_decode_dependency_at(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             sample_id,
    uint32_t             index,
    uint32_t*            out_dep_sample_id);

/** Per-sample coded width / height of a sequence sample.  Mirrors the
 *  C++ overloads `Reader::getWidth(SequenceId, SequenceImageId, &)` /
 *  `Reader::getHeight(SequenceId, SequenceImageId, &)`.  This is the
 *  coded frame dimension of the individual sample (which may differ
 *  from the track-level display dimensions reported by
 *  `heif_reader_get_track_display_width/height` once the track header
 *  matrix / clean-aperture / scaling are applied). */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_width(const heif_reader_t* reader,
                                                                        uint32_t             track_id,
                                                                        uint32_t             sample_id,
                                                                        uint32_t*            out_width);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_height(const heif_reader_t* reader,
                                                                         uint32_t             track_id,
                                                                         uint32_t             sample_id,
                                                                         uint32_t*            out_height);

/** Per-track 9-entry `tkhd` display matrix (rotation / mirror / scale
 *  presentation transform).  Mirrors `Reader::getMatrix(SequenceId,
 *  Array<int32_t>&)`; layout matches the ISOBMFF 3x3 matrix
 *  (a, b, u, c, d, v, x, y, w) with `index` in [0, 9).  A binding that
 *  needs the full matrix calls this nine times. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_matrix_at(const heif_reader_t* reader,
                                                                     uint32_t             track_id,
                                                                     uint32_t             index,
                                                                     int32_t*             out_value);

/** Master images of a track -- the count and per-index sample-id of
 *  samples flagged as sync / random-access points.  Mirrors
 *  `Reader::getMasterImages(SequenceId, Array<SequenceImageId>&)`.
 *  Bindings use this for thumbnail-strip generation, seek anchors,
 *  and cover-art selection.
 *
 *  Note: each `_at` call re-runs the underlying accessor; bindings
 *  that walk the full list should fetch the count once and cache the
 *  per-index sample-ids if profiling shows the cost matters. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_master_image_count(const heif_reader_t* reader,
                                                                              uint32_t             track_id,
                                                                              uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_master_image_at(const heif_reader_t* reader,
                                                                           uint32_t             track_id,
                                                                           uint32_t             index,
                                                                           uint32_t*            out_sample_id);

/** Sample-id list filtered by `TrackSampleType` classification
 *  (`out_ref` = output reference frames, `display` = displayable
 *  samples in display order, `samples` = all samples in track-entry
 *  order, etc. -- see `HEIF::TrackSampleType`).  Mirrors
 *  `Reader::getItemListByType(SequenceId, TrackSampleType,
 *  Array<SequenceImageId>&)`.  `sample_type` accepts the integer
 *  values exposed via `heif_sample_info_t::sample_type`.
 *
 *  Bindings use this for e.g. "give me only the displayable
 *  samples in display order" without walking every sample-info. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_list_by_type_count(const heif_reader_t* reader,
                                                                                     uint32_t             track_id,
                                                                                     uint8_t              sample_type,
                                                                                     uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_list_by_type_at(const heif_reader_t* reader,
                                                                                  uint32_t             track_id,
                                                                                  uint8_t              sample_type,
                                                                                  uint32_t             index,
                                                                                  uint32_t*            out_sample_id);

/** Clean-aperture ('clap') from a sample-description entry of a
 *  track.  Mirrors `Reader::getProperty(SequenceId, sdi,
 *  CleanAperture&)`.  `sample_description_index` is 1-based per
 *  ISOBMFF (not the sample id).  Reuses the `heif_clap_t` POD shared
 *  with the item-side bridge. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_clap(const heif_reader_t* reader,
                                                                       uint32_t             track_id,
                                                                       uint32_t             sample_description_index,
                                                                       heif_clap_t*         out_clap);

/** Auxiliary-type-info ('auxi') type / subtype from a sample-
 *  description entry of a track.  Mirrors
 *  `Reader::getProperty(SequenceId, sdi, AuxiliaryType&)`.  Both
 *  outputs follow the standard `(buf, *size_inout)` size-probe
 *  convention; the type buffer is NUL-terminated, the subtype buffer
 *  is raw bytes.  `sample_description_index` is 1-based per ISOBMFF. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_auxi_type(const heif_reader_t* reader,
                                                                            uint32_t             track_id,
                                                                            uint32_t             sample_description_index,
                                                                            char*                buffer,
                                                                            uint64_t*            size_inout);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_auxi_subtype(const heif_reader_t* reader,
                                                                               uint32_t             track_id,
                                                                               uint32_t             sample_description_index,
                                                                               uint8_t*             buffer,
                                                                               uint64_t*            size_inout);

/** Count + at for the per-track timestamp list returned by
 *  `HEIF::Reader::getItemTimestamps`.  Each entry is a
 *  `(timestamp_ms, sample_id)` pair. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_item_timestamp_count(const heif_reader_t* reader,
                                                                             uint32_t             track_id,
                                                                             uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_item_timestamp_at(const heif_reader_t* reader,
                                                                          uint32_t             track_id,
                                                                          uint32_t             index,
                                                                          int64_t*             out_timestamp_ms,
                                                                          uint32_t*            out_sample_id);

/** Count + at for the per-sample timestamp list returned by
 *  `HEIF::Reader::getTimestampsOfItem`.  An image may be displayed
 *  multiple times under an edit list, hence the list. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_item_timestamp_count(const heif_reader_t* reader,
                                                                       uint32_t             track_id,
                                                                       uint32_t             sample_id,
                                                                       uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_item_timestamp_at(const heif_reader_t* reader,
                                                                    uint32_t             track_id,
                                                                    uint32_t             sample_id,
                                                                    uint32_t             index,
                                                                    int64_t*             out_timestamp_ms);

/** EditList wrapper scalars.  `out_looping` is 0/1.  `out_repetitions`
 *  semantics:
 *    - `out_looping == 1` and `out_repetitions == 0.0` -> infinite
 *      looping;
 *    - `out_looping == 1` and `out_repetitions > 0.0` -> finite,
 *      repeated that many times;
 *    - `out_looping == 0` -> bridge writes `0.0` regardless of the
 *      underlying value (the C++ `EditList::repetitions` field is
 *      defined as meaningless when `looping` is false; the bridge
 *      enforces that contract so plain-C / FFI callers do not need
 *      a `repetitions = looping ? value : 0` workaround).
 *  Returns `NOT_APPLICABLE` when the track has no edit list (the
 *  `HasEditList` feature bit is clear); on that path both outputs
 *  are written as defensive zeros so callers reading after a
 *  non-OK return get well-defined values. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_edit_list_meta(const heif_reader_t* reader,
                                                                       uint32_t             track_id,
                                                                       int*                 out_looping,
                                                                       double*              out_repetitions);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_edit_unit_count(const heif_reader_t* reader,
                                                                        uint32_t             track_id,
                                                                        uint32_t*            out_count);

/** Caller MUST set `out_unit->struct_size = sizeof(heif_edit_unit_t)`
 *  before the call. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_edit_unit_at(const heif_reader_t* reader,
                                                                     uint32_t             track_id,
                                                                     uint32_t             index,
                                                                     heif_edit_unit_t*    out_unit);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_eqiv_count(const heif_reader_t* reader,
                                                                          uint32_t             track_id,
                                                                          uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_eqiv_at(const heif_reader_t* reader,
                                                                       uint32_t             track_id,
                                                                       uint32_t             index,
                                                                       heif_sample_eqiv_t*  out_eqiv);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_stmi_count(const heif_reader_t* reader,
                                                                          uint32_t             track_id,
                                                                          uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_stmi_at(const heif_reader_t* reader,
                                                                       uint32_t             track_id,
                                                                       uint32_t             index,
                                                                       heif_sample_stmi_t*  out_stmi);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_stmi_metadata_item_count(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             stmi_index,
    uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_stmi_metadata_item_at(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             stmi_index,
    uint32_t             item_index,
    uint32_t*            out_metadata_item_id);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_refs_count(const heif_reader_t* reader,
                                                                          uint32_t             track_id,
                                                                          uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_refs_at(const heif_reader_t* reader,
                                                                       uint32_t             track_id,
                                                                       uint32_t             index,
                                                                       heif_sample_refs_t*  out_refs);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_refs_reference_item_count(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             refs_index,
    uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_refs_reference_item_at(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             refs_index,
    uint32_t             item_index,
    uint32_t*            out_reference_item_id);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_trif_count(const heif_reader_t* reader,
                                                                          uint32_t             track_id,
                                                                          uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_trif_at(const heif_reader_t* reader,
                                                                       uint32_t             track_id,
                                                                       uint32_t             index,
                                                                       heif_sample_trif_t*  out_trif);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_trif_dependency_count(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             trif_index,
    uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_trif_dependency_at(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             trif_index,
    uint32_t             dep_index,
    uint16_t*            out_dependency_group_id);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_spor_count(const heif_reader_t* reader,
                                                                          uint32_t             track_id,
                                                                          uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_spor_at(const heif_reader_t* reader,
                                                                       uint32_t             track_id,
                                                                       uint32_t             index,
                                                                       heif_sample_spor_t*  out_spor);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_spor_subp_track_ref_idx_count(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             spor_index,
    uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_spor_subp_track_ref_idx_at(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             spor_index,
    uint32_t             ref_idx,
    uint16_t*            out_subp_track_ref_idx);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_sulm_count(const heif_reader_t* reader,
                                                                          uint32_t             track_id,
                                                                          uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_sulm_at(const heif_reader_t* reader,
                                                                       uint32_t             track_id,
                                                                       uint32_t             index,
                                                                       heif_sample_sulm_t*  out_sulm);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_sulm_group_id_count(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             sulm_index,
    uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_sulm_group_id_at(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             sulm_index,
    uint32_t             group_idx,
    uint16_t*            out_group_id);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_minp_count(const heif_reader_t* reader,
                                                                          uint32_t             track_id,
                                                                          uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_minp_at(const heif_reader_t* reader,
                                                                       uint32_t             track_id,
                                                                       uint32_t             index,
                                                                       heif_sample_minp_t*  out_minp);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_minp_mix_subp_track_idx1_count(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             minp_index,
    uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_minp_mix_subp_track_idx1_at(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             minp_index,
    uint32_t             idx1_index,
    uint16_t*            out_value);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_minp_mix_subp_track_idx2_count(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             minp_index,
    uint32_t*            out_count);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_track_sample_minp_mix_subp_track_idx2_at(
    const heif_reader_t* reader,
    uint32_t             track_id,
    uint32_t             minp_index,
    uint32_t             idx2_index,
    uint16_t*            out_value);

HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_segment_index_count(heif_reader_t* reader,
                                                                      uint32_t*      out_count);

/** Caller MUST set `out_segment->struct_size = sizeof(heif_segment_info_t)`
 *  before the call. */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_get_segment_index_at(heif_reader_t*       reader,
                                                                   uint32_t             index,
                                                                   heif_segment_info_t* out_segment);

/* Fragmented / DASH-style segmented playback bridges the four
 * remaining streaming virtuals on `Reader`: parseInitializationSegment,
 * parseSegment, invalidateSegment, parseSegmentIndex.  Companion to
 * the single-stream initialise surface; designed for the multi-stream
 * case where the init segment and one or more media segments live
 * behind separate I/O cursors (network, ranged fetch, ISO BMFF
 * On-Demand profile). */

/**
 * Opaque segment-stream handle.  Wraps a caller-side
 * `heif_stream_callbacks_t` table in a `HEIF::StreamInterface`
 * shim suitable for handing to the segment-parsing entry points
 * below.  Distinct from `heif_reader_initialize_stream` because
 * the streaming flow needs MULTIPLE concurrent stream sources
 * alive at once (one init segment + one or more media segments),
 * which a single stream installed on the reader cannot model.
 *
 * **Lifetime contract** (mirrors `HEIF::StreamInterface*` rules):
 *  - For `parse_initialization_segment`: the library retains a
 *    raw pointer to the underlying shim until
 *    `heif_reader_destroy`.  The caller MUST keep the matching
 *    `heif_stream_t*` alive for the entire reader lifetime.
 *  - For `parse_segment`: the library retains the pointer until
 *    the matching `invalidate_segment(segment_id)`.  The caller
 *    MUST NOT destroy the `heif_stream_t*` before that call.
 *  - For `parse_segment_index`: the library uses the stream only
 *    for the duration of the call; the caller may destroy the
 *    handle immediately after the function returns.
 *
 * Bindings that own streams from a garbage-collected runtime
 * (Python, Java, JavaScript) MUST hold a strong reference to
 * each `heif_stream_t` for the appropriate window above; a GC
 * of the underlying `user_data` mid-parse will crash the library.
 * The recommended binding pattern is for the reader wrapper to
 * keep strong references to every stream it has been handed,
 * dropping each one only when its lifetime window above closes.
 */
typedef struct heif_stream heif_stream_t;

/**
 * Build a segment-stream handle from a caller-owned callback
 * table.  The callback table is *copied* into the handle; the
 * caller may free the `heif_stream_callbacks_t` immediately
 * after this call.  `user_data` is held by reference for the
 * lifetime of the handle (until `heif_stream_destroy`).
 *
 * Returns NULL on allocation failure or if `callbacks` is NULL,
 * if `callbacks->struct_size` has an invalid struct_size, or if any of the
 * four required callbacks (read / absolute_seek / tell / size)
 * is NULL.  The library does not populate the structured error
 * slot on NULL returns from constructor functions (callers detect
 * by pointer).
 */
HEIF_DLL_FN_EXPORT heif_stream_t* heif_stream_create(const heif_stream_callbacks_t* callbacks);

/**
 * Destroy a segment-stream handle.  MUST NOT be called while
 * any reader still references the underlying stream -- see the
 * lifetime contract on `heif_stream_t` above.  Calling
 * destroy on a NULL pointer is a no-op.
 */
HEIF_DLL_FN_EXPORT void heif_stream_destroy(heif_stream_t* stream);

/**
 * Parse an initialization segment from `stream`.  Mirrors
 * `Reader::parseInitializationSegment(StreamInterface*)`.
 *
 * @return OK on success.  The library retains the underlying
 *         stream pointer until `heif_reader_destroy` -- see the
 *         `heif_stream_t` lifetime contract.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_parse_initialization_segment(heif_reader_t* reader,
                                                                           heif_stream_t* stream);

/**
 * Parse a media segment from `stream`.  Mirrors
 * `Reader::parseSegment(StreamInterface*, SegmentId, uint64_t)`.
 *
 * `segment_id` MUST be globally unique across this reader and
 * MUST differ from any init segment ID.
 *
 * `earliest_pts_in_ts` is optional: pass `UINT64_MAX` to mean
 * "unset" (the C++ default-argument behavior); only meaningful
 * when feeding a partial segment without an `sidx` box.
 *
 * The library retains the underlying stream pointer until the
 * matching `heif_reader_invalidate_segment(segment_id)` call --
 * see the `heif_stream_t` lifetime contract.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_parse_segment(heif_reader_t* reader,
                                                            heif_stream_t* stream,
                                                            uint32_t       segment_id,
                                                            uint64_t       earliest_pts_in_ts);

/**
 * Invalidate a previously-parsed media segment.  Mirrors
 * `Reader::invalidateSegment(SegmentId)`.  After this returns
 * OK the caller MAY destroy the matching `heif_stream_t*`.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_invalidate_segment(heif_reader_t* reader,
                                                                 uint32_t       segment_id);

/**
 * Parse an `sidx` box from `stream` and return the resulting
 * segment index using the size-probe convention.
 * Mirrors `Reader::parseSegmentIndex(StreamInterface*,
 * Array<SegmentInformation>&)`.
 *
 * Two-call usage:
 *   1. Probe: pass `out_segments = NULL`, `*count_inout = 0`.
 *      The function returns OK with `*count_inout` set to the
 *      required entry count (or an error if parsing failed).
 *   2. Fetch: allocate `out_segments[*count_inout]`, set each
 *      element's `struct_size = sizeof(heif_segment_info_t)`,
 *      then call again.  On exit `*count_inout` is narrowed to
 *      the number of entries actually written.
 *
 * Returns `BUFFER_SIZE_TOO_SMALL` if `*count_inout` < required.
 *
 * The stream is used only for the duration of this call; the
 * caller MAY destroy it immediately afterwards.
 */
HEIF_DLL_FN_EXPORT heif_error_code_t heif_reader_parse_segment_index(heif_reader_t*       reader,
                                                                  heif_stream_t*       stream,
                                                                  heif_segment_info_t* out_segments,
                                                                  uint32_t*            count_inout);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* HEIF_READER_C_H */
