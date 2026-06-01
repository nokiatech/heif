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
 * @file heif_version_c.h
 *
 * Pure-C runtime-version surface.  Lets a binding perform a load-time
 * version handshake without going back through embind / C++ name
 * mangling just to call `HEIF::Reader::GetVersion()`.
 *
 * **Why a separate header?**  `heifexport.h` is macros only and
 * `heif_error_c.h` is the error-inspection slot -- neither is the
 * right place for an unrelated version accessor.  This keeps the
 * concern siloed and lets bindings include exactly what they need.
 *
 * **Numbering contract.**  The `major` value returned here is the
 * same integer the library publishes as its `SOVERSION` (see
 * `srcs/reader/CMakeLists.txt` `SOVERSION ${heif_VERSION_MAJOR}`),
 * so a binding that built against major N can refuse to load against
 * a runtime whose `major != N` and have that match the dynamic
 * loader's own SONAME pin.
 *
 * **Thread-safety.**  All entry points are pure reads of compile-time
 * constants embedded in the library; safe to call from any thread,
 * any time, including before `heif_reader_create`.
 *
 * **Mapping to the existing C++ surface.**
 *
 *     C-API entry point               C++ equivalent
 *     ------------------------------  --------------------------------
 *     heif_get_runtime_version        (HEIF_VERSION_MAJOR / _MINOR / _PATCH)
 *     heif_get_runtime_version_string HEIF::Reader::GetVersion() / Writer::GetVersion()
 *     heif_get_runtime_version_number HEIF_VERSION_NUMBER
 *
 * The macro-mirror trio (`HEIF_VERSION_MAJOR` etc., declared in
 * `buildinfo.hpp`) remains the authoritative *build-time* answer for
 * call sites that want preprocessor-time guards.  The accessors here
 * are the authoritative *runtime* answer for embedders that link
 * dynamically (or load via `dlopen` / Emscripten `Module`).
 */

#ifndef HEIF_VERSION_C_H
#define HEIF_VERSION_C_H

#include <stdint.h>

#include "heifexport.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Fetch the library's runtime semver triple.  Each out-pointer may
 * be `NULL` to skip that component (e.g. a binding that only cares
 * about `major` for the SOVERSION handshake passes `NULL` for the
 * other two).  Never fails; never returns an error code.
 *
 * The values returned are baked into the library at build time from
 * `PROJECT_VERSION_*` (see `srcs/buildinfo/buildinfo.hpp.in`) and so
 * always agree with the SONAME of the consuming `libheif_shared.so`
 * / `libheif_writer_shared.so`.
 */
HEIF_DLL_FN_EXPORT void heif_get_runtime_version(uint32_t* out_major,
                                                 uint32_t* out_minor,
                                                 uint32_t* out_patch);

/**
 * Returns the library version as a NUL-terminated string of the form
 * `"<major>.<minor>.<patch>"` (e.g. `"4.0.0"`), optionally suffixed
 * with a `+<git-sha>` provenance tag when the build was not made
 * from a release tag.  The pointer is to static storage owned by the
 * library; MUST NOT be `free`'d by the caller.  Never returns NULL.
 *
 * This is the C-callable analogue of `HEIF::Reader::GetVersion()` /
 * `HEIF::Writer::GetVersion()`; the returned string is the same one
 * those C++ accessors return.
 */
HEIF_DLL_FN_EXPORT const char* heif_get_runtime_version_string(void);

/**
 * Returns the library version as a single packed 32-bit integer in
 * the same encoding as the `HEIF_VERSION_NUMBER` build-time macro:
 *   `(major << 16) | (minor << 8) | patch`
 * suitable for direct numeric comparison against a build-time
 * constant a binding embeds at compile time.  Cheaper than the
 * triple-out variant when the binding only wants to do
 * `runtime >= MY_MIN`.
 */
HEIF_DLL_FN_EXPORT uint32_t heif_get_runtime_version_number(void);

/**
 * @brief Evolvable ABI-handshake POD.
 *
 * Size-prefixed struct carrying the library's runtime
 * identity.  Designed for FFI / wasm consumers that want a single
 * call returning a flat, memcpy-safe record instead of three
 * separate out-pointer calls.
 *
 * **struct_size handshake.**  The caller sets `struct_size =
 * sizeof(heif_abi_handshake_t)` before calling
 * `heif_get_abi_handshake`.  The library fills the leading prefix
 * the caller knows about; if the caller's struct is larger than
 * the library's, the library zeros the trailing bytes.  This lets
 * old callers talk to new libraries and vice versa.
 *
 * **Field ordering.**  All fields are `uint32_t` so alignment is
 * uniform; no padding is needed.
 */
typedef struct heif_abi_handshake
{
    /** MUST equal `sizeof(heif_abi_handshake_t)` on entry. */
    uint32_t struct_size;
    /** SOVERSION-aligned library major version. */
    uint32_t major;
    /** Library minor version. */
    uint32_t minor;
    /** Library patch version. */
    uint32_t patch;
    /** Reserved for future feature flags (build-config bitmap).
     *  Currently always zero.  Consumers MUST ignore unknown bits. */
    uint32_t flags;
} heif_abi_handshake_t;

#ifdef __cplusplus
static_assert(sizeof(heif_abi_handshake_t) == 20,
              "heif_abi_handshake_t ABI is exactly 20 bytes");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(heif_abi_handshake_t) == 20,
               "heif_abi_handshake_t ABI is exactly 20 bytes");
#endif

/**
 * Fill a caller-provided `heif_abi_handshake_t` with the library's
 * runtime identity.  The caller MUST set `out->struct_size =
 * sizeof(heif_abi_handshake_t)` before the call.
 *
 * @param[in,out] out  Pointer to a caller-allocated handshake POD.
 *                     Must not be NULL; `struct_size` must be >= 4.
 * @return 0 on success, negative on error (NULL pointer or
 *         struct_size too small).
 */
HEIF_DLL_FN_EXPORT int32_t heif_get_abi_handshake(heif_abi_handshake_t* out);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // HEIF_VERSION_C_H
