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
 * @file heif_error_c.h
 *
 * Pure-C error-inspection surface over the per-thread `HEIF::ErrorInfo`
 * slot.
 *
 * **Why a separate C header?**  `heiferrorinfo.h` exposes a C++ class
 * (`HEIF::ErrorInfo`) that owns heap storage and uses C++ namespacing.
 * That cannot cross a stable FFI boundary: ctypes / cffi / cgo /
 * P-Invoke / bindgen consumers need plain `extern "C"` functions with
 * POD parameter types and caller-owned buffers.  This header provides
 * exactly that -- six small wrappers that read the same `thread_local`
 * slot the C++ macros (`HEIF_FAIL`, `HEIF_RECORD`, ...) populate.
 *
 * **Guarantees.**
 *   - All functions are `noexcept` (C linkage; no exception ever escapes
 *     into the embedder ABI).
 *   - All functions are thread-safe and read the calling thread's slot
 *     only -- a failure recorded by thread A is never visible from
 *     thread B.
 *   - String accessors that return `const char*` point into static or
 *     library-thread-local storage; the embedder MUST NOT call `free()`
 *     on them.  Lifetime is "valid until the next mutating call on this
 *     thread" (`heif_clear_last_error`, or any HEIF API entry that
 *     records a new error).
 *   - The buffer-fill accessor (`heif_get_last_error_message`) follows
 *     the standard `snprintf(3)` convention: always NUL-terminates the
 *     output buffer when `len > 0`, and returns the number of bytes
 *     that *would* have been written (excluding the terminator), so a
 *     caller can size the buffer with a `(NULL, 0)` probe call.
 *
 * **Mapping to the C++ slot.**
 *
 *     C-API entry point                  C++ equivalent
 *     ---------------------------------  ------------------------------
 *     heif_get_last_error_code           HEIF::getErrorInfo().code()
 *     heif_get_last_error_origin         HEIF::getErrorInfo().origin()
 *     heif_get_last_error_origin_name    HEIF::errorOriginName(...)
 *     heif_get_last_error_code_name      HEIF::errorCodeName(...)
 *     heif_get_last_error_file           HEIF::getErrorInfo().file()
 *     heif_get_last_error_line           HEIF::getErrorInfo().line()
 *     heif_get_last_error_message        HEIF::getErrorInfo().message()
 *     heif_get_last_error_cause          HEIF::getErrorInfo().causeWhat()
 *     heif_has_last_error                HEIF::hasErrorInfo()
 *     heif_clear_last_error              HEIF::clearErrorInfo()
 *
 * No allocator coupling is required by this surface: the only owning
 * return value is the `(buf, len)` truncation idiom, where the caller
 * owns the buffer.  Allocator-paired free functions
 * (`heif_*_free_*` companions) are provided on the reader and
 * writer surfaces that return library-allocated payloads.
 */

#ifndef HEIF_ERROR_C_H
#define HEIF_ERROR_C_H

#include <stddef.h>
#include <stdint.h>

#include "heifexport.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Numeric value of `HEIF::ErrorCode` for the calling thread's most
 * recent error.  Returns `0` (the value of `HEIF::ErrorCode::OK`) when
 * no error has been recorded since the last `heif_clear_last_error`.
 *
 * Wider than the underlying `std::uint8_t` so that a future ABI bump
 * can grow the enum without changing this signature.  Callers that
 * round-trip the value into the C++ enum should narrow explicitly.
 */
HEIF_DLL_FN_EXPORT int32_t heif_get_last_error_code(void);

/**
 * Numeric value of `HEIF::ErrorOrigin` for the calling thread's most
 * recent error.  Returns `0` (`ErrorOrigin::Unknown`) when no error
 * has been recorded.
 */
HEIF_DLL_FN_EXPORT int32_t heif_get_last_error_origin(void);

/**
 * Human-readable name of the most recent `ErrorOrigin` (e.g.
 * `"Reader_FileOpen"`).  Always non-null; points into static storage
 * inside the library.  MUST NOT be freed by the caller.
 */
HEIF_DLL_FN_EXPORT const char* heif_get_last_error_origin_name(void);

/**
 * Human-readable name of the most recent `ErrorCode` (e.g.
 * `"FILE_READ_ERROR"`, or `"ErrorCode(<n>)"` for codes outside the
 * documented enum).  Always non-null; points into static storage
 * inside the library.  MUST NOT be freed by the caller.
 */
HEIF_DLL_FN_EXPORT const char* heif_get_last_error_code_name(void);

/**
 * Source file where the most recent failure was recorded.  Points into
 * static storage (always `__FILE__`); MUST NOT be freed by the caller.
 * Returns `""` (never null) when no error has been recorded.
 */
HEIF_DLL_FN_EXPORT const char* heif_get_last_error_file(void);

/**
 * Source line where the most recent failure was recorded.  Returns `0`
 * when no error has been recorded.
 */
HEIF_DLL_FN_EXPORT uint32_t heif_get_last_error_line(void);

/**
 * Copy the most recent error message into a caller-owned buffer using
 * the standard `snprintf(3)` truncation convention:
 *
 *   - When `len > 0`, writes up to `len - 1` bytes of message text plus
 *     a terminating NUL into `buf`.  `buf` is always NUL-terminated on
 *     return when `len > 0`.
 *   - When `len == 0`, `buf` may be `NULL`; nothing is written.  Use
 *     this form to query the size: `heif_get_last_error_message(NULL, 0)`
 *     returns the number of bytes that would be required (excluding
 *     the terminator), and the caller can then allocate `result + 1`
 *     bytes and call again.
 *
 * Returns the length of the full message in bytes (excluding the
 * terminating NUL), regardless of whether truncation occurred.  When
 * no error has been recorded, returns `0` and writes a single NUL byte
 * into `buf` (when `len > 0`).
 *
 * Thread-safe: reads only the calling thread's slot.
 */
HEIF_DLL_FN_EXPORT size_t heif_get_last_error_message(char* buf, size_t len);

/**
 * Same as `heif_get_last_error_message` but for the upstream
 * `std::exception::what()` string captured at the failure site (if
 * any).  When the failure was a plain "guard returned non-OK" (no
 * exception involved), the cause is empty and this returns `0`.
 */
HEIF_DLL_FN_EXPORT size_t heif_get_last_error_cause(char* buf, size_t len);

/**
 * `1` when the calling thread has a recorded error that has not yet
 * been cleared, `0` otherwise.  Equivalent to
 * `HEIF::getErrorInfo().valid()`.
 */
HEIF_DLL_FN_EXPORT int32_t heif_has_last_error(void);

/**
 * Clear the calling thread's slot.  Public API entry points inside the
 * library already clear on entry; this is exposed so embedders can
 * defensively reset between unrelated operations they pipeline on the
 * same thread, or after they have observed and reported an error and
 * want subsequent successful calls to surface a clean slot.
 */
HEIF_DLL_FN_EXPORT void heif_clear_last_error(void);

/**
 * Translate an arbitrary `heif_error_code_t` value into its canonical
 * name (e.g. `0` -> `"OK"`, `7` -> `"FILE_READ_ERROR"`).  Unlike
 * `heif_get_last_error_code_name`, this entry point is a pure
 * stateless lookup -- it does NOT consult the per-thread error slot,
 * so it is safe to call when no error has been recorded, on a
 * different thread from the one that raised an error, or to map any
 * code returned by any reader / writer entry point directly to a
 * string for logging.
 *
 * Forwards to `HEIF::errorCodeName(HEIF::ErrorCode)` so the C facade
 * and the C++ surface cannot drift.  Bindings that previously
 * maintained their own switch-statement translation table can drop
 * it and call this instead.
 *
 * Always returns a non-null pointer to static storage owned by the
 * library; MUST NOT be `free`'d by the caller.  Codes outside the
 * documented enum surface as a sentinel string of the form
 * `"ErrorCode(<n>)"` (the same convention `errorCodeName` already
 * uses internally), so a forward-compatible switch in a binding
 * never has to deal with NULL.
 *
 * Thread-safe and re-entrant: pure read of a static lookup table.
 */
HEIF_DLL_FN_EXPORT const char* heif_error_code_name(int32_t code);

/**
 * Symmetric to `heif_error_code_name` for `HEIF::ErrorOrigin` values.
 * Surfaces `Reader_*` / `Writer_*` / `Unknown` etc. so a binding can
 * log both halves of `(origin, code)` from any context -- including a
 * non-error one -- without going through the per-thread slot.
 *
 * Same lifetime / non-null / thread-safety contract as
 * `heif_error_code_name`.  Unknown origins surface as
 * `"ErrorOrigin(<n>)"`.
 */
HEIF_DLL_FN_EXPORT const char* heif_error_origin_name(int32_t origin);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // HEIF_ERROR_C_H
