/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef HEIFERRORINFO_H
#define HEIFERRORINFO_H

#include <cstdint>
#include <string>

#include "heifcommondatatypes.h"
#include "heifexport.h"
#include <cstddef>

/**
 * Rich error context for the HEIF base library (reader / writer / common /
 * parsers).
 *
 * BACKGROUND
 * ----------
 * The base library reports failures through the integer enum
 * `HEIF::ErrorCode`. The enum has only a few dozen values and is
 * deliberately coarse: many distinct internal failure modes collapse onto
 * the same code (e.g. `FILE_READ_ERROR` is returned from ~38 sites in the
 * reader, `DECODER_CONFIGURATION_ERROR` from ~21 sites in the writer).
 *
 * In production this means the integer return tells the embedder *that* a
 * failure happened but provides no breadcrumb pointing at *which* one.
 * Worse, several reader catch sites use `catch(...)` and silently overwrite
 * the diagnostic with a generic value, dropping the original
 * `std::exception::what()` on the floor.
 *
 * DESIGN
 * ------
 * Rather than redesign every signature in the public API (which would break
 * source compatibility for every existing client), we add a *thread-local
 * side-channel*. When a base-library function is about to return a non-OK
 * `ErrorCode`, it ALSO calls `HEIF::setErrorInfo(...)` with as much
 * contextual information as it has: where in the pipeline the failure
 * originated, which source file and line raised it, a short message, and
 * (when applicable) the upstream `std::exception::what()` that triggered
 * it.
 *
 * The HEIFPP wrapper layer drains this slot at its boundary
 * (`HEIFPP::convertErrorCode`) and folds it into the higher-level
 * `HEIFPP::ErrorContext`, which in turn surfaces in the Java
 * `com.nokia.heif.Exception` thrown across the JNI boundary.
 *
 * THREADING
 * ---------
 * The slot is `thread_local`. An error reported on thread A is never
 * observable from thread B. This makes the side-channel safe under any
 * concurrent use of the library (e.g. an embedder loading multiple HEIFs
 * in parallel).
 *
 * MEMORY
 * ------
 * Implemented with `std::string` so messages are not truncated. On the
 * pathological "we are reporting OOM" path the assignment may itself fail;
 * that is acceptable - the integer ErrorCode still propagates correctly,
 * we just lose the human-readable message.
 *
 * STABILITY
 * ---------
 * This API is *additive*. No existing public function changed signature.
 * Embedders that ignore the side-channel get exactly the behavior they
 * had before.
 */
namespace HEIF
{
    /**
     * Coarse classification of *where* in the base library a failure
     * originated. Phase B/C/D will grow this enum as additional sites are
     * retrofitted; values are explicitly numbered so additions are
     * non-breaking and binary diff-friendly.
     */
    enum class ErrorOrigin : std::uint8_t
    {
        Unknown                  = 0,

        // Reader pipeline -------------------------------------------------
        Reader_FileOpen          = 10,  ///< Could not open the underlying stream.
        Reader_FileRead          = 11,  ///< Stream open but read / seek failed.
        Reader_BoxParse          = 12,  ///< ISOBMFF box-level parse failure.
        Reader_VvcParser         = 13,  ///< VVC bitstream / config parser.
        Reader_HevcParser        = 14,  ///< HEVC bitstream / config parser.
        Reader_AvcParser         = 15,  ///< AVC bitstream / config parser.
        Reader_DecoderConfig     = 16,  ///< Decoder configuration record extraction.
        Reader_ItemValidation    = 17,  ///< Item-id sanity / properties / references.
        Reader_Mini              = 18,  ///< MINI (low-overhead HEIF) parsing.
        Reader_Grid              = 19,  ///< Grid / overlay composition.
        Reader_Derivation        = 20,  ///< Derived image (identity / overlay / iden).

        // Writer pipeline -------------------------------------------------
        Writer_FileIO            = 30,  ///< Could not open / write underlying stream.
        Writer_BoxBuild          = 31,  ///< Box construction failure.
        Writer_DecoderConfig     = 32,  ///< Decoder configuration / parameter sets.
        Writer_Mini              = 33,  ///< MINI writer.
        Writer_State             = 34,  ///< Operation invalid for current writer state.
        Writer_Validation        = 35,  ///< Caller passed a bad argument to the writer.

        // Standalone parsers ---------------------------------------------
        Parser_Vvc               = 50,  ///< VVC NAL parser silent-swallow recovery (legacy/generic).
        Parser_Jpeg              = 51,  ///< JPEG marker parser silent-break recovery.
        Parser_VvcSps            = 52,  ///< VVC SPS NAL parser.
        Parser_VvcPps            = 53,  ///< VVC PPS NAL parser.
        Parser_VvcVps            = 54,  ///< VVC VPS NAL parser.
        Parser_VvcOpi            = 55,  ///< VVC OPI NAL parser.
        Parser_VvcRbsp           = 56,  ///< VVC RBSP bit reader (NAL header / EPB strip).

        // Internal --------------------------------------------------------
        Common_BitstreamRead     = 80,  ///< BitStream read past end / overflow.
        Internal_Invariant       = 90,  ///< HEIF_ASSERT or "should never happen".
    };

    /**
     * Snapshot of the most recent base-library error on the calling thread.
     *
     * `valid() == false` indicates the slot is empty (no error has been
     * recorded since the last `clearErrorInfo`).
     *
     * `causeWhat()` returns an empty C-string (`""`, never null) when the
     * failure was not caused by a `std::exception` (i.e. a plain "guard
     * returned non-OK" path).
     *
     * `file()` points into static storage (always `__FILE__`); the integer,
     * code and origin getters are noexcept and return by value.
     *
     * This type is a class with library-owned heap storage for the message
     * strings, so the public binary layout does not contain `std::string`
     * / `std::exception` (which are not ABI-stable across MSVC CRTs and
     * cannot cross a C FFI boundary).
     * Migration: replace `info.message` with `info.message()`,
     * `info.code = X` with `info.setCode(X)`, etc. The HEIF_FAIL /
     * HEIF_RECORD macros are unchanged at call sites; only their internal
     * expansion uses setters now.
     */
    class HEIF_DLL_PUBLIC ErrorInfo
    {
    public:
        ErrorInfo() noexcept;
        ~ErrorInfo();
        ErrorInfo(const ErrorInfo& other);
        ErrorInfo(ErrorInfo&& other) noexcept;
        ErrorInfo& operator=(const ErrorInfo& other);
        ErrorInfo& operator=(ErrorInfo&& other) noexcept;

        // ---- Accessors (noexcept, return by value / static C-string) ----
        ErrorCode   code()      const noexcept { return m_code;   }
        ErrorOrigin origin()    const noexcept { return m_origin; }
        const char* file()      const noexcept { return (m_file != nullptr) ? m_file : ""; }
        int         line()      const noexcept { return m_line;   }
        bool        valid()     const noexcept { return m_valid;  }

        /// HEIF-level explanation, set at the failure site. Never null;
        /// empty string ("") when no message was attached. Pointer is
        /// stable until the next mutating call on this instance.
        const char* message()   const noexcept;

        /// Original `std::exception::what()` if the failure was caught
        /// from a thrown exception, or the literal "<unknown exception>"
        /// for catch(...) sites. Never null; empty string ("") on the
        /// pure guard-return path. Pointer is stable until the next
        /// mutating call on this instance.
        const char* causeWhat() const noexcept;

        // ---- Mutators (used by HEIF_FAIL / HEIF_RECORD macro family) ----
        void setCode  (ErrorCode   c) noexcept { m_code   = c; }
        void setOrigin(ErrorOrigin o) noexcept { m_origin = o; }
        void setFile  (const char* f) noexcept { m_file   = f; }
        void setLine  (int         l) noexcept { m_line   = l; }
        void setValid (bool        v) noexcept { m_valid  = v; }

        /// Deep-copies `msg` (or the empty string if `msg == nullptr`)
        /// into library-owned storage. Replaces any previous message.
        /// Allocation is performed inside the HEIF shared library so the
        /// matching deallocation in ~ErrorInfo also runs against the
        /// library's CRT -- safe for mixed-CRT MSVC consumers.
        void setMessage  (const char* msg);

        /// Same as setMessage but for the upstream `what()` slot.
        void setCauseWhat(const char* what);

    private:
        // POD members -- safe in public layout. Order is part of the
        // binary contract; do not rearrange without an ABI baseline diff.
        ErrorCode   m_code   = ErrorCode::OK;
        ErrorOrigin m_origin = ErrorOrigin::Unknown;
        const char* m_file   = "";
        int         m_line   = 0;
        bool        m_valid  = false;
        char*       m_message   = nullptr;  // library-allocated; freed by ~ErrorInfo
        char*       m_causeWhat = nullptr;  // library-allocated; freed by ~ErrorInfo
    };

    /**
     * Record an error on the calling thread's slot. Overwrites any
     * previously-recorded error on this thread. Prefer the HEIF_FAIL /
     * HEIF_FAIL_FROM macros at the failure site, which capture
     * __FILE__/__LINE__ automatically.
     */
    HEIF_DLL_PUBLIC void setErrorInfo(const ErrorInfo& info);

    /**
     * Clear this thread's slot. Public API entry points should call this on
     * entry so that a stale context from an earlier failure (which the
     * embedder may have ignored) cannot surface as a misleading diagnostic
     * for a later, unrelated success.
     */
    HEIF_DLL_PUBLIC void clearErrorInfo();

    /**
     * Read this thread's slot. Returned reference is valid until the next
     * setErrorInfo / clearErrorInfo on the same thread.
     */
    HEIF_DLL_PUBLIC const ErrorInfo& getErrorInfo();

    /**
     * True when the calling thread has a recorded error that has not yet
     * been cleared.
     */
    HEIF_DLL_PUBLIC bool hasErrorInfo();

    /**
     * Stable, human-readable name for an ErrorOrigin (e.g. for log lines).
     * Always returns a non-null pointer into static storage.
     */
    HEIF_DLL_PUBLIC const char* errorOriginName(ErrorOrigin origin);

    /**
     * Stable, human-readable name for an ErrorCode (e.g. "FILE_READ_ERROR").
     * Always returns a non-null pointer into static storage.
     * For codes outside the documented enum returns "ErrorCode(<n>)".
     */
    HEIF_DLL_PUBLIC const char* errorCodeName(ErrorCode code);

    // ---------------------------------------------------------------------
    //  Formatter / Sink helpers (added in 3.8.0)
    //
    //  Background: the slot above is a structured *data model*, not a
    //  presentation layer. Embedders should not have to write their own
    //  formatter or wire the slot into their logger by hand. Industry
    //  practice (POSIX strerror, libcurl curl_easy_strerror, OpenSSL
    //  ERR_error_string_n, Vulkan VkDebugUtilsMessenger, FFmpeg av_log,
    //  SQLite sqlite3_errmsg) converged on the trio:
    //
    //      1. A pure stringifier that writes into the caller's buffer
    //         (no I/O, no globals, thread-safe, predictable size).
    //      2. An optional sink callback the embedder installs once and
    //         the library invokes from setErrorInfo(). Default: none
    //         installed -> library is silent. Embedders who want stderr
    //         opt in explicitly via stderrErrorSink below.
    //      3. A reference stderr sink so demos and CLI tools get useful
    //         output for free, without forcing log pollution on
    //         production embedders.
    //
    //  This trio composes cleanly: production embedders install a sink
    //  routing to spdlog/glog/Android Log/etc.; CLI tools install
    //  stderrErrorSink; library wrappers (HEIFPP, JNI) install their own
    //  sinks and re-emit through their own channels; embedders that want
    //  total silence install nothing and the library never touches I/O.
    // ---------------------------------------------------------------------

    /**
     * @since 3.8.0
     * Format an ErrorInfo into a single human-readable line in the caller's
     * buffer. snprintf-style: returns the number of characters that would
     * have been written had the buffer been large enough (excluding the
     * trailing NUL). Always NUL-terminates when n > 0; truncates safely
     * when the formatted message exceeds the buffer.
     *
     * Output format:
     *   - valid slot, no cause:
     *       "[<ORIGIN>@<file>:<line>] <CODE_NAME>(<n>): <message>"
     *   - valid slot, with cause:
     *       "[<ORIGIN>@<file>:<line>] <CODE_NAME>(<n>): <message> | cause: <what>"
     *   - empty slot (info.valid == false):
     *       "<no error>"
     *
     * Pure function: no I/O, no globals, thread-safe.
     */
    HEIF_DLL_PUBLIC std::size_t formatErrorInfo(const ErrorInfo& info,
                                                char* buf,
                                                std::size_t n);

    /**
     * @since 3.8.0
     * std::string convenience overload. Allocates exactly the required
     * storage (one snprintf measurement + one allocation). Identical
     * format to the buffer overload above.
     */
    HEIF_DLL_PUBLIC std::string formatErrorInfo(const ErrorInfo& info);

    /**
     * @since 3.8.0
     * Optional sink callback signature. Installed via setErrorSink, invoked
     * from setErrorInfo *after* the slot has been written, on the same
     * thread that produced the failure. The `info` reference is the same
     * value that was just stored in the slot; embedders may copy fields out
     * of it freely but must not retain the reference.
     *
     * `user` is the opaque pointer passed to setErrorSink at install time;
     * the library never dereferences it.
     *
     * Reentrancy: a sink that itself calls back into HEIF and triggers a
     * new setErrorInfo will overwrite the slot it was just notified about.
     * Sinks should generally be passive (formatting + forwarding to a
     * logger) rather than initiating new HEIF calls.
     */
    using ErrorSink = void (*)(const ErrorInfo& info, void* user);

    /**
     * @since 3.8.0
     * Install (or replace) the process-wide error sink. Pass nullptr to
     * uninstall. Default state: no sink installed -> library does not print
     * or forward anything when errors occur (it still populates the slot,
     * which embedders may poll via getErrorInfo()).
     *
     * Thread-safe: the sink + user pair is published atomically. Calls in
     * flight at install time may observe either the old or the new sink,
     * but never a torn pair.
     */
    HEIF_DLL_PUBLIC void setErrorSink(ErrorSink sink, void* user);

    /**
     * @since 3.8.0
     * True iff a sink is currently installed. Embedders writing both a
     * sink AND an inline polling check use this to deduplicate so the
     * same failure isn't logged twice.
     */
    HEIF_DLL_PUBLIC bool hasErrorSink();

    /**
     * @since 3.8.0
     * Reference sink implementation: writes "[heif] <formatErrorInfo>\n"
     * to the FILE* passed as `user` (typically stderr or stdout). Provided
     * as a convenience for CLI tools and demos. NOT installed automatically;
     * embedders opt in explicitly:
     *
     *     HEIF::setErrorSink(&HEIF::stderrErrorSink, stderr);
     *
     * Uses fputs/fputc rather than std::cerr so it is usable even from
     * environments that have replaced their iostreams allocator (relevant
     * for embedders using HEIF::setCustomAllocator).
     *
     * If `user` is nullptr, falls back to stderr.
     */
    HEIF_DLL_PUBLIC void stderrErrorSink(const ErrorInfo& info, void* user);

    /**
     * @since 4.0.0
     * Trace-sink callback signature. Receives one newline-stripped,
     * NUL-terminated line per dispatch. `user` is the opaque pointer
     * passed to setTraceSink at install time.
     *
     * The trace channel carries verbose developer output from the legacy
     * Log facility (logInfo / logWarning / logError / logPanic); structured
     * failure records still flow through ErrorSink above.
     */
    using TraceSink = void (*)(const char* line, void* user);

    /**
     * @since 4.0.0
     * Install (or replace) the process-wide trace sink. Pass nullptr to
     * uninstall and restore the cout/cerr fallback. Default state: no
     * sink installed; combined with the default Log::LogLevel::NONE this
     * means out-of-the-box embedders see nothing on stdout/stderr from
     * the library.
     *
     * Thread-safe: same atomic-publish contract as setErrorSink.
     */
    HEIF_DLL_PUBLIC void setTraceSink(TraceSink sink, void* user);

    /**
     * @since 4.0.0
     * True iff a trace sink is currently installed.
     */
    HEIF_DLL_PUBLIC bool hasTraceSink();

    /**
     * @since 4.0.0
     * Reference trace sink: writes "[heif:trace] <line>\n" to the FILE*
     * passed as `user`. Nullptr `user` falls back to stderr.
     */
    HEIF_DLL_PUBLIC void stderrTraceSink(const char* line, void* user);
}  // namespace HEIF

namespace HEIF
{
    namespace detail
    {
        /**
         * Helper used by HEIF_FAIL / HEIF_FAIL_FROM / HEIF_FAIL_FROM_UNKNOWN.
         *
         * Why a helper instead of an immediately-invoked lambda?
         *
         *   The C/C++ standards leave __LINE__'s value implementation-defined
         *   when the token appears inside the body of a multi-line function-
         *   like macro. GCC/Clang collapse all expanded tokens to the macro
         *   invocation's first line; MSVC keeps each token on whichever
         *   physical line of the expansion it sits, so a 3-line HEIF_FAIL
         *   call would report the closing-paren line, not the call-site line.
         *
         *   By making the public macros one-liners that just forward
         *   __FILE__ / __LINE__ as arguments, the __LINE__ token always
         *   appears on the SAME physical line as the macro name in the
         *   user's source -> identical, correct behaviour on every compiler.
         */
        HEIF_DLL_PUBLIC ErrorCode heifFail(ErrorCode    code,
                                           ErrorOrigin  origin,
                                           const char*  message,
                                           const char*  file,
                                           int          line);

        HEIF_DLL_PUBLIC ErrorCode heifFailFrom(ErrorCode    code,
                                               ErrorOrigin  origin,
                                               const char*  message,
                                               const char*  causeWhat,
                                               const char*  file,
                                               int          line);

        HEIF_DLL_PUBLIC ErrorCode heifFailFromUnknown(ErrorCode    code,
                                                      ErrorOrigin  origin,
                                                      const char*  message,
                                                      const char*  file,
                                                      int          line);
    }  // namespace detail
}  // namespace HEIF

/**
 * Record an error and yield it as the function's return value.
 *
 *     if (!filename) {
 *         return HEIF_FAIL(HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
 *                          HEIF::ErrorOrigin::Writer_Validation,
 *                          "WriterImpl::initialize: filename is null");
 *     }
 *
 * Single-line expansion guarantees __FILE__/__LINE__ point at the call
 * site on every conforming compiler (see detail::heifFail rationale).
 */
#define HEIF_FAIL(code_, origin_, message_) \
    (::HEIF::detail::heifFail((code_), (origin_), (message_), __FILE__, __LINE__))

/**
 * Same as HEIF_FAIL but also captures the upstream std::exception::what()
 * so the original message survives through the catch block. Use inside a
 * catch:
 *
 *     try { box.parseBox(stream); }
 *     catch (const std::exception& e) {
 *         return HEIF_FAIL_FROM(HEIF::ErrorCode::FILE_READ_ERROR,
 *                               HEIF::ErrorOrigin::Reader_BoxParse,
 *                               "HeifReaderImpl::readStream: box parse threw",
 *                               e);
 *     }
 */
#define HEIF_FAIL_FROM(code_, origin_, message_, exception_) \
    (::HEIF::detail::heifFailFrom((code_), (origin_), (message_), (exception_).what(), __FILE__, __LINE__))

/**
 * Same as HEIF_FAIL but for catch(...) sites where we have no
 * std::exception to read what() from. Records "<unknown exception>" as the
 * cause so downstream consumers can distinguish "guarded return" (causeWhat
 * empty) from "exception was eaten" (causeWhat == "<unknown exception>").
 */
#define HEIF_FAIL_FROM_UNKNOWN(code_, origin_, message_) \
    (::HEIF::detail::heifFailFromUnknown((code_), (origin_), (message_), __FILE__, __LINE__))

/**
 * Firewall macros for wrapping a public method body that must NEVER let a
 * std::exception escape across the library boundary. Use as:
 *
 *     ErrorCode WriterImpl::finalize()
 *     {
 *         HEIF_TRY
 *         {
 *             ... existing body ...
 *             return ErrorCode::OK;
 *         }
 *         HEIF_CATCH_AND_RETURN(ErrorOrigin::Writer_BoxBuild,
 *                               "WriterImpl::finalize")
 *     }
 *
 * The catch arm returns ErrorCode::UNKNOWN_ERROR (the only generic catch-all
 * code in the enum) and records the upstream what() / "<unknown exception>"
 * sentinel in the side-channel. ErrorCode::UNKNOWN_ERROR was previously
 * comment-marked "Shouldn't be allowed" precisely because, before the
 * firewall, an exception escape was undefined behavior; the firewall is
 * the first legitimate use.
 *
 * IMPORTANT: place the firewall around the WHOLE body so all existing
 * `return ErrorCode::X;` statements inside continue to return their codes
 * normally. The firewall ONLY catches escapes; well-typed returns are
 * untouched.
 */
#define HEIF_TRY  try

/**
 * Public-entry variant of HEIF_TRY: drains the calling thread's diagnostic
 * slot before opening the try block. Use on EVERY method that an integrator
 * can call directly (every override of HEIF::Reader::* and HEIF::Writer::*).
 *
 * Why: an embedder polling HEIF::getLastError() after a successful call must
 * see "no error" - not stale data left over from a previous failure on the
 * same thread that the embedder may have ignored. The HEIFPP layer already
 * does this via clearLastError() at the top of each Heif::* entry method;
 * this macro brings the C-tier reader/writer impls to parity.
 *
 * Do NOT use on internal helpers (e.g. HeifReaderImpl::handleMeta,
 * HeifReaderImpl::readBox). Those run inside an outer entry method's
 * scope and must NOT clear the slot - if a deeper helper has already
 * populated the slot via HEIF_FAIL, the outer entry method's clear would
 * destroy the diagnostic the integrator is about to read. Internal
 * helpers keep using plain HEIF_TRY.
 */
#define HEIF_ENTRY_TRY                                                          \
    ::HEIF::clearErrorInfo();                                                   \
    try

#define HEIF_CATCH_AND_RETURN(origin_, message_)                                \
    catch (const std::exception& _heif_e)                                       \
    {                                                                           \
        return HEIF_FAIL_FROM(::HEIF::ErrorCode::UNKNOWN_ERROR,                 \
                              (origin_), (message_), _heif_e);                  \
    }                                                                           \
    catch (...)                                                                 \
    {                                                                           \
        return HEIF_FAIL_FROM_UNKNOWN(::HEIF::ErrorCode::UNKNOWN_ERROR,         \
                                      (origin_), (message_));                   \
    }

/**
 * Record-only family. Used by helper layers (e.g. the standalone VVC NAL
 * parsers in srcs/vvc-parser/) whose failure-path returns a value type
 * rather than an ErrorCode. The macro records the diagnostic in the
 * thread-local slot but does NOT issue a `return`, so it can sit anywhere
 * inside a function whose return type is not ErrorCode.
 *
 *     if (!reader.prepare(buf)) {
 *         HEIF_RECORD(ErrorCode::INVALID_FUNCTION_PARAMETER,
 *                     ErrorOrigin::Parser_VvcRbsp,
 *                     "VvcSpsParser::parse: NAL header / EPB strip failed");
 *         return {};   // zeroed Info struct, contract preserved
 *     }
 *
 * The recorded ErrorCode is whatever the caller chooses; for parser-internal
 * recoveries `INVALID_FUNCTION_PARAMETER` is the closest fit (the embedder
 * passed bytes the parser cannot interpret). The slot only matters when the
 * upstream public-API call ultimately returns non-OK; well-behaved callers
 * follow the documented protocol of consulting the slot AFTER a non-OK
 * return.
 */
#define HEIF_RECORD(code_, origin_, message_)                                   \
    do {                                                                        \
        ::HEIF::ErrorInfo _heif_info;                                           \
        _heif_info.setCode   ((code_));                                         \
        _heif_info.setOrigin ((origin_));                                       \
        _heif_info.setMessage((message_));                                      \
        _heif_info.setFile   (__FILE__);                                        \
        _heif_info.setLine   (__LINE__);                                        \
        _heif_info.setValid  (true);                                            \
        ::HEIF::setErrorInfo(_heif_info);                                       \
    } while (0)

#define HEIF_RECORD_FROM(code_, origin_, message_, exception_)                  \
    do {                                                                        \
        ::HEIF::ErrorInfo _heif_info;                                           \
        _heif_info.setCode     ((code_));                                       \
        _heif_info.setOrigin   ((origin_));                                     \
        _heif_info.setMessage  ((message_));                                    \
        _heif_info.setCauseWhat((exception_).what() ? (exception_).what() : ""); \
        _heif_info.setFile     (__FILE__);                                      \
        _heif_info.setLine     (__LINE__);                                      \
        _heif_info.setValid    (true);                                          \
        ::HEIF::setErrorInfo(_heif_info);                                       \
    } while (0)

#define HEIF_RECORD_FROM_UNKNOWN(code_, origin_, message_)                      \
    do {                                                                        \
        ::HEIF::ErrorInfo _heif_info;                                           \
        _heif_info.setCode     ((code_));                                       \
        _heif_info.setOrigin   ((origin_));                                     \
        _heif_info.setMessage  ((message_));                                    \
        _heif_info.setCauseWhat("<unknown exception>");                         \
        _heif_info.setFile     (__FILE__);                                      \
        _heif_info.setLine     (__LINE__);                                      \
        _heif_info.setValid    (true);                                          \
        ::HEIF::setErrorInfo(_heif_info);                                       \
    } while (0)

#endif  // HEIFERRORINFO_H
