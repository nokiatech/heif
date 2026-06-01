/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 */

#pragma once

#include <cstddef>
#include <functional>
#include <string>

#include "ErrorCodes.h"
#include "heiferrorinfo.h"
#include <cstdint>

/**
 * Rich error context for HEIFPP / JNI integration.
 *
 * BACKGROUND
 * ----------
 * HEIFPP API methods historically returned only a `Result` enum on failure
 * (e.g. `Result::FILE_READ_ERROR`). The enum value carries no information
 * about *where* in the pipeline the failure originated, *which* low-level
 * exception (if any) caused it, or *why* it happened (which field was bad,
 * which offset, which item id). When a non-OK result was forwarded across
 * the JNI boundary the only thing the Java caller saw was the integer code
 * plus a generic call-site string like "Loading failed".
 *
 * For an embedder integrating this library as a dependency, a result of
 * `FILE_READ_ERROR` could mean "file does not exist", "stream truncated",
 * "VVC parser found an out-of-range profile_idc", "decoder config NAL had
 * empty payload", etc. They all collapse to the same code with no breadcrumb.
 *
 * DESIGN
 * ------
 * Rather than rewriting every Result-returning function to produce a richer
 * type (which would break source compatibility for every existing client),
 * we add a *thread-local side-channel*: when HEIFPP is about to return a
 * non-OK Result, it ALSO calls `setLastError(...)` with as much context as
 * it has. The next call to `checkError` from the JNI layer reads (and
 * clears) this context, packaging it into the Java `com.nokia.heif.Exception`
 * along with the integer error code that already crosses the boundary.
 *
 * The thread-local makes this safe under concurrent use - each calling
 * thread gets its own slot, so an error reported on thread A is never
 * observable from thread B.
 *
 * The macros HEIFPP_FAIL / HEIFPP_FAIL_CAUSED_BY capture `__FILE__` and
 * `__LINE__` automatically so the produced Java exception can point at the
 * exact native source location.
 */
namespace HEIFPP
{
    /**
     * Coarse classification of *where* in the pipeline a failure originated.
     * Embedders can switch on this to localise UI messages or pick recovery
     * strategies (e.g. "ask user for a different file" for File_IO,
     * "report bug" for Internal_Invariant).
     */
    enum class ErrorSource : std::uint8_t
    {
        Unknown             = 0,

        // Reading pipeline
        Reader_FileIO       = 10,  ///< Could not open / read underlying stream.
        Reader_BoxParse     = 11,  ///< ISOBMFF box-level parse failure.
        Reader_VvcParser    = 12,  ///< VVC bitstream / config parser.
        Reader_HevcParser   = 13,  ///< HEVC bitstream / config parser.
        Reader_AvcParser    = 14,  ///< AVC bitstream / config parser.
        Reader_Grid         = 15,  ///< Grid composition.
        Reader_Derivation   = 16,  ///< Derived image (overlay / identity / ...).
        Reader_DecoderConfig = 17, ///< Decoder configuration record.
        Reader_Mini         = 18,  ///< MINI (low-overhead HEIF) parsing.

        // Writing pipeline
        Writer_FileIO       = 30,  ///< Could not open / write underlying stream.
        Writer_BoxBuild     = 31,  ///< Box construction failure.
        Writer_Encoder      = 32,  ///< Encoder configuration / sample assembly.
        Writer_Mini         = 33,  ///< MINI writer.

        // HEIFPP wrapper
        Api_Validation      = 50,  ///< Caller passed bad arguments.
        Api_State           = 51,  ///< Operation invalid for current state.
        Api_NotFound        = 52,  ///< Requested item / id does not exist.

        // Internal
        Internal_Invariant  = 90,  ///< HEIF_ASSERT / "should never happen".
    };

    /**
     * Human-readable name of a source for logging.
     * Returns a static C string; never returns nullptr.
     */
    const char* errorSourceName(ErrorSource source);

    /**
     * Snapshot of the most recent error.
     *
     * `valid==false` means the slot is empty (no error has been recorded
     * since the last `clearLastError`).
     *
     * `causeWhat` is empty when the failure was not triggered by a
     * std::exception (i.e. a plain "guard returned an error" path).
     *
     * `file` is a pointer into static storage (always `__FILE__`); copying
     * the struct by value is safe.
     */
    struct ErrorContext
    {
        Result      code        = Result::OK;
        ErrorSource source      = ErrorSource::Unknown;
        std::string message;          ///< HEIFPP-level explanation.
        std::string causeWhat;        ///< Original std::exception::what(), if any.
        const char* file        = "";
        int         line        = 0;
        bool        valid       = false;
    };

    /**
     * Record an error on the current thread's slot. Overwrites any
     * previously-recorded error on this thread.
     */
    void setLastError(const ErrorContext& ctx);

    /**
     * Clear this thread's slot. Call at the entry of public API functions
     * to avoid surfacing a stale context from an earlier call that the
     * embedder forgot to drain.
     */
    void clearLastError();

    /**
     * Read this thread's slot. The returned reference is valid until the
     * next `setLastError` / `clearLastError` on the same thread.
     */
    const ErrorContext& getLastError();

    /**
     * Convenience predicate.
     */
    bool hasLastError();

    // ---------------------------------------------------------------------
    //  Formatter / Sink helpers (added in 3.8.0)
    //
    //  Mirror of the same trio in HEIF::heiferrorinfo.h, scoped to the
    //  HEIFPP wrapper layer. Embedders writing C++ code directly against
    //  HEIFPP install a HEIFPP::ErrorSink; embedders going through JNI
    //  install a Java sink (com.nokia.heif.ErrorHandler.setErrorSink)
    //  which is bridged into a HEIFPP sink internally.
    //
    //  The two layers' sinks are independent on purpose: a HEIFPP-level
    //  embedder (using try/catch) does not necessarily care about the
    //  base-library slot, and a base-library embedder does not get
    //  HEIFPP context. Wrappers that DO care about both can install one
    //  sink at each level.
    // ---------------------------------------------------------------------

    /**
     * @since 3.8.0
     * Format an ErrorContext into a single human-readable line in the
     * caller's buffer. snprintf-style: returns required length, always
     * NUL-terminates when n>0, truncates safely. Pure function; no I/O,
     * no globals, thread-safe.
     *
     * Output format mirrors HEIF::formatErrorInfo:
     *   - valid:   "[<SOURCE>@<file>:<line>] Result(<n>): <message>[ | cause: <what>]"
     *   - invalid: "<no error>"
     */
    std::size_t formatErrorContext(const ErrorContext& ctx, char* buf, std::size_t n);

    /// std::string overload of formatErrorContext.
    std::string formatErrorContext(const ErrorContext& ctx);

    /**
     * @since 3.8.0
     * Optional sink callback signature. Invoked from setLastError after
     * the slot has been populated, on the failing thread. The reference
     * is valid for the duration of the call only; sinks must copy any
     * fields they need to retain.
     *
     * Sinks may capture state by lambda; we use std::function rather than
     * a raw function pointer so HEIFPP embedders (who are already in C++,
     * not C-ABI land) can route to spdlog/glog-style loggers naturally.
     */
    using ErrorSink = std::function<void(const ErrorContext& ctx)>;

    /**
     * @since 3.8.0
     * Install (or replace) the process-wide HEIFPP error sink. Pass an
     * empty std::function to uninstall. Default state: no sink installed
     * -> library does not print or forward HEIFPP errors when they occur
     * (it still populates the slot, polled via getLastError()).
     */
    void setErrorSink(ErrorSink sink);

    /// True iff a HEIFPP sink is currently installed.
    bool hasErrorSink();

    /**
     * @since 3.8.0
     * Reference sink: writes "[heifpp] <formatErrorContext>\n" to stderr.
     * Provided as a convenience for CLI tools and demos. NOT installed
     * automatically; embedders opt in:
     *     HEIFPP::setErrorSink(&HEIFPP::stderrErrorSink);
     */
    void stderrErrorSink(const ErrorContext& ctx);
}  // namespace HEIFPP

/**
 * Record an error and yield it as the function's return value. Use as:
 *
 *     if (!aFilename) {
 *         return HEIFPP_FAIL(HEIFPP::Result::INVALID_FUNCTION_PARAMETER,
 *                            HEIFPP::ErrorSource::Api_Validation,
 *                            "Heif::load: filename is null");
 *     }
 *
 * The macro expands to a comma-expression so it can sit on the right of a
 * `return` and still capture __FILE__/__LINE__ at the failure site.
 */
#define HEIFPP_FAIL(code_, source_, message_)                                  \
    ([&]() -> ::HEIFPP::Result {                                               \
        ::HEIFPP::ErrorContext _ctx;                                           \
        _ctx.code    = (code_);                                                \
        _ctx.source  = (source_);                                              \
        _ctx.message = (message_);                                             \
        _ctx.file    = __FILE__;                                               \
        _ctx.line    = __LINE__;                                               \
        _ctx.valid   = true;                                                   \
        ::HEIFPP::setLastError(_ctx);                                          \
        return (code_);                                                        \
    }())

/**
 * Same as HEIFPP_FAIL but also captures the upstream std::exception::what()
 * so the original message survives the hop from the lower-level layer
 * (reader / writer / parser) into HEIFPP. Use inside a catch block:
 *
 *     try { mReader->initialize(...); }
 *     catch (const std::exception& e) {
 *         return HEIFPP_FAIL_CAUSED_BY(
 *             HEIFPP::Result::FILE_READ_ERROR,
 *             HEIFPP::ErrorSource::Reader_FileIO,
 *             "Heif::load: reader initialization threw",
 *             e);
 *     }
 */
#define HEIFPP_FAIL_CAUSED_BY(code_, source_, message_, exception_)            \
    ([&]() -> ::HEIFPP::Result {                                               \
        ::HEIFPP::ErrorContext _ctx;                                           \
        _ctx.code      = (code_);                                              \
        _ctx.source    = (source_);                                            \
        _ctx.message   = (message_);                                           \
        _ctx.causeWhat = (exception_).what() ? (exception_).what() : "";       \
        _ctx.file      = __FILE__;                                             \
        _ctx.line      = __LINE__;                                             \
        _ctx.valid     = true;                                                 \
        ::HEIFPP::setLastError(_ctx);                                          \
        return (code_);                                                        \
    }())

/**
 * Record context from a converted lower-level HEIF::ErrorCode. Use right
 * before returning a `convertErrorCode(error)` so the Java side sees the
 * source/file/line that produced it. The result is the converted Result.
 */
#define HEIFPP_FAIL_CONVERTED(heifErrorCode_, source_, message_)               \
    ([&]() -> ::HEIFPP::Result {                                               \
        const ::HEIFPP::Result _r = ::HEIFPP::convertErrorCode(heifErrorCode_);\
        if (_r != ::HEIFPP::Result::OK)                                        \
        {                                                                      \
            ::HEIFPP::ErrorContext _ctx;                                       \
            _ctx.code    = _r;                                                 \
            _ctx.source  = (source_);                                          \
            _ctx.message = (message_);                                         \
            _ctx.file    = __FILE__;                                           \
            _ctx.line    = __LINE__;                                           \
            _ctx.valid   = true;                                               \
            /* Phase E bridge: if the lower-level HEIF base library has    */ \
            /* populated its own thread-local ErrorInfo (Phase B/C/D       */ \
            /* retrofits), fold the breadcrumb into our HEIFPP context so  */ \
            /* it survives the boundary into the JNI layer instead of      */ \
            /* being silently dropped.                                     */ \
            /*                                                             */ \
            /* file/line are KEPT at the HEIFPP boundary (the conversion   */ \
            /* call site) so embedders inspecting ctx.file always see      */ \
            /* "Heif.cpp" or whichever wrapper invoked the base call. The  */ \
            /* deeper HEIF site is encoded into causeWhat together with    */ \
            /* the HEIF-level message and any upstream what(), giving the  */ \
            /* full breadcrumb without breaking the existing               */ \
            /* "ctx.file == wrapper-source" contract.                      */ \
            if (::HEIF::hasErrorInfo())                                        \
            {                                                                  \
                const ::HEIF::ErrorInfo& _hei = ::HEIF::getErrorInfo();        \
                const char* _heiCause = _hei.causeWhat();                      \
                std::string _bread = std::string("[")                          \
                    + ::HEIF::errorOriginName(_hei.origin()) + "@"             \
                    + _hei.file() + ":"                                        \
                    + std::to_string(_hei.line()) + "] "                       \
                    + _hei.message();                                          \
                if (_heiCause[0] != '\0')                                      \
                {                                                              \
                    _bread += " | cause: ";                                    \
                    _bread += _heiCause;                                       \
                }                                                              \
                _ctx.causeWhat = _bread;                                       \
                ::HEIF::clearErrorInfo();                                      \
            }                                                                  \
            ::HEIFPP::setLastError(_ctx);                                      \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            /* Drain any stale slot left over from a recovered-from        */ \
            /* internal failure so the next non-OK return doesn't pick up  */ \
            /* misleading context from this successful call.               */ \
            ::HEIF::clearErrorInfo();                                          \
        }                                                                      \
        return _r;                                                             \
    }())
