/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 */

#include "ErrorContext.h"

#include <atomic>
#include <cstdio>
#include <mutex>
#include <utility>

namespace HEIFPP
{
    namespace
    {
        // Per-thread slot. thread_local guarantees that an error reported
        // on thread A cannot be observed from thread B, which would cause
        // wildly misleading diagnostics under any concurrent use of the
        // library (e.g. an embedder loading multiple files in parallel).
        //
        // The slot is initialised to the default-constructed ErrorContext
        // (valid==false) so a getLastError() call before any error has
        // happened returns a well-defined "no error here" record rather
        // than reading uninitialised memory.
        thread_local ErrorContext tLastError{};

        // Optional sink (added 3.8.0). Unlike the C-side sink we cannot
        // store an std::function in std::atomic (not trivially-copyable),
        // so we guard it with a mutex. Install/uninstall is rare; dispatch
        // takes the mutex briefly in the failure path which is already
        // expensive, so the contention cost is negligible. We also keep a
        // separate atomic<bool> for the fast "is anything installed?"
        // check so the no-sink fast path does not touch the mutex at all.
        std::mutex            gSinkMutex;
        ErrorSink             gSink;            // protected by gSinkMutex
        std::atomic<bool>     gSinkInstalled{false};
    }

    void setLastError(const ErrorContext& ctx)
    {
        tLastError = ctx;
        // Notify the optional sink AFTER the slot has been populated so a
        // sink that itself calls getLastError() observes the same record
        // it was just notified about. The atomic check makes the no-sink
        // path lock-free.
        if (gSinkInstalled.load(std::memory_order_acquire))
        {
            ErrorSink local;
            {
                std::lock_guard<std::mutex> lk(gSinkMutex);
                local = gSink;
            }
            if (local)
            {
                local(ctx);
            }
        }
    }

    void clearLastError()
    {
        tLastError = ErrorContext{};
    }

    const ErrorContext& getLastError()
    {
        return tLastError;
    }

    bool hasLastError()
    {
        return tLastError.valid;
    }

    const char* errorSourceName(ErrorSource source)
    {
        switch (source)
        {
            case ErrorSource::Unknown:              return "Unknown";

            case ErrorSource::Reader_FileIO:        return "Reader_FileIO";
            case ErrorSource::Reader_BoxParse:      return "Reader_BoxParse";
            case ErrorSource::Reader_VvcParser:     return "Reader_VvcParser";
            case ErrorSource::Reader_HevcParser:    return "Reader_HevcParser";
            case ErrorSource::Reader_AvcParser:     return "Reader_AvcParser";
            case ErrorSource::Reader_Grid:          return "Reader_Grid";
            case ErrorSource::Reader_Derivation:    return "Reader_Derivation";
            case ErrorSource::Reader_DecoderConfig: return "Reader_DecoderConfig";
            case ErrorSource::Reader_Mini:          return "Reader_Mini";

            case ErrorSource::Writer_FileIO:        return "Writer_FileIO";
            case ErrorSource::Writer_BoxBuild:      return "Writer_BoxBuild";
            case ErrorSource::Writer_Encoder:       return "Writer_Encoder";
            case ErrorSource::Writer_Mini:          return "Writer_Mini";

            case ErrorSource::Api_Validation:       return "Api_Validation";
            case ErrorSource::Api_State:            return "Api_State";
            case ErrorSource::Api_NotFound:         return "Api_NotFound";

            case ErrorSource::Internal_Invariant:   return "Internal_Invariant";
        }
        return "Unknown";
    }

    namespace
    {
        // Local stringifier for HEIFPP::Result. Kept private to this TU
        // because the canonical "name of an integer code" lookup the Java
        // side uses lives in ErrorHandler.nativeErrorToString; this is the
        // C++ counterpart used only by formatErrorContext below.
        const char* resultName(Result r)
        {
            switch (r)
            {
                case Result::OK:                              return "OK";
                case Result::INDEX_OUT_OF_BOUNDS:             return "INDEX_OUT_OF_BOUNDS";
                case Result::ALREADY_INITIALIZED:             return "ALREADY_INITIALIZED";
                case Result::PRIMARY_ITEM_NOT_SET:            return "PRIMARY_ITEM_NOT_SET";
                case Result::BRANDS_NOT_SET:                  return "BRANDS_NOT_SET";
                case Result::HIDDEN_PRIMARY_ITEM:             return "HIDDEN_PRIMARY_ITEM";
                case Result::INVALID_HANDLE:                  return "INVALID_HANDLE";
                case Result::ALREADY_IN_GROUP:                return "ALREADY_IN_GROUP";
                case Result::ALREADY_SET:                     return "ALREADY_SET";
                case Result::INVALID_CONFIG:                  return "INVALID_CONFIG";

                case Result::FILE_OPEN_ERROR:                 return "FILE_OPEN_ERROR";
                case Result::FILE_READ_ERROR:                 return "FILE_READ_ERROR";
                case Result::FILE_HEADER_ERROR:               return "FILE_HEADER_ERROR";
                case Result::BUFFER_SIZE_TOO_SMALL:           return "BUFFER_SIZE_TOO_SMALL";
                case Result::BYTESTREAM_READ_ERROR:           return "BYTESTREAM_READ_ERROR";
                case Result::MEMORY_TOO_SMALL_BUFFER:         return "MEMORY_TOO_SMALL_BUFFER";

                case Result::UNINITIALIZED:                   return "UNINITIALIZED";
                case Result::NOT_APPLICABLE:                  return "NOT_APPLICABLE";
                case Result::FTYP_ALREADY_WRITTEN:            return "FTYP_ALREADY_WRITTEN";
                case Result::ALLOCATOR_ALREADY_SET:           return "ALLOCATOR_ALREADY_SET";

                case Result::INVALID_FUNCTION_PARAMETER:      return "INVALID_FUNCTION_PARAMETER";
                case Result::INVALID_ITEM_ID:                 return "INVALID_ITEM_ID";
                case Result::INVALID_GROUP_ID:                return "INVALID_GROUP_ID";
                case Result::INVALID_MEDIADATA_ID:            return "INVALID_MEDIADATA_ID";
                case Result::INVALID_METADATAITEM_ID:         return "INVALID_METADATAITEM_ID";
                case Result::INVALID_PROPERTY_INDEX:          return "INVALID_PROPERTY_INDEX";
                case Result::INVALID_REFERENCE_COUNT:         return "INVALID_REFERENCE_COUNT";
                case Result::INVALID_SAMPLE_DESCRIPTION_INDEX:return "INVALID_SAMPLE_DESCRIPTION_INDEX";
                case Result::INVALID_SEGMENT:                 return "INVALID_SEGMENT";
                case Result::INVALID_SEQUENCE_ID:             return "INVALID_SEQUENCE_ID";
                case Result::INVALID_DECODER_CONFIG_ID:       return "INVALID_DECODER_CONFIG_ID";
                case Result::INVALID_SEQUENCE_IMAGE_ID:       return "INVALID_SEQUENCE_IMAGE_ID";

                case Result::MEDIA_PARSING_ERROR:             return "MEDIA_PARSING_ERROR";
                case Result::INVALID_MEDIA_FORMAT:            return "INVALID_MEDIA_FORMAT";
                case Result::UNSUPPORTED_CODE_TYPE:           return "UNSUPPORTED_CODE_TYPE";
                case Result::DECODER_CONFIGURATION_ERROR:     return "DECODER_CONFIGURATION_ERROR";
                case Result::NO_MEDIA:                        return "NO_MEDIA";
                case Result::CONTENT_TYPE_NOT_SET:            return "CONTENT_TYPE_NOT_SET";
                case Result::PROTECTED_ITEM:                  return "PROTECTED_ITEM";
                case Result::UNPROTECTED_ITEM:                return "UNPROTECTED_ITEM";

                case Result::TOO_MANY_ITEMS:                  return "TOO_MANY_ITEMS";
                case Result::MISSING_PROPERTIES:              return "MISSING_PROPERTIES";
                case Result::MULTIPLE_GAINMAPS:               return "MULTIPLE_GAINMAPS";
                case Result::INVALID_EXIF_ORIENTATION_VALUE:  return "INVALID_EXIF_ORIENTATION_VALUE";
                case Result::VVC_DISABLED:                    return "VVC_DISABLED";

                case Result::ERROR_UNDEFINED:                 return "ERROR_UNDEFINED";
            }
            return "Result(?)";
        }
    }  // namespace

    std::size_t formatErrorContext(const ErrorContext& ctx, char* buf, std::size_t n)
    {
        char  sentinel = 0;
        char* dst      = (buf != nullptr) ? buf : &sentinel;
        const std::size_t cap = (buf != nullptr) ? n : 1;

        if (!ctx.valid)
        {
            const int written = std::snprintf(dst, cap, "<no error>");
            return (written < 0) ? 0u : static_cast<std::size_t>(written);
        }
        const char* sourceName = errorSourceName(ctx.source);
        const char* codeName   = resultName(ctx.code);
        const char* file       = (ctx.file != nullptr) ? ctx.file : "?";
        const int   codeInt    = static_cast<int>(ctx.code);

        int written;
        if (ctx.causeWhat.empty())
        {
            written = std::snprintf(dst, cap, "[%s@%s:%d] %s(%d): %s",
                                    sourceName, file, ctx.line,
                                    codeName, codeInt,
                                    ctx.message.c_str());
        }
        else
        {
            written = std::snprintf(dst, cap, "[%s@%s:%d] %s(%d): %s | cause: %s",
                                    sourceName, file, ctx.line,
                                    codeName, codeInt,
                                    ctx.message.c_str(),
                                    ctx.causeWhat.c_str());
        }
        return (written < 0) ? 0u : static_cast<std::size_t>(written);
    }

    std::string formatErrorContext(const ErrorContext& ctx)
    {
        char small[256];
        const std::size_t need = formatErrorContext(ctx, small, sizeof small);
        if (need < sizeof small)
        {
            return std::string(small, need);
        }
        std::string out;
        out.resize(need);
        formatErrorContext(ctx, &out[0], need + 1);
        return out;
    }

    void setErrorSink(ErrorSink sink)
    {
        std::lock_guard<std::mutex> lk(gSinkMutex);
        gSink = std::move(sink);
        // Publish the "is something installed?" flag AFTER the storage has
        // been written; the matching acquire-load in setLastError ensures
        // a concurrent dispatcher that observes the flag also observes
        // the new value of gSink. (The mutex inside dispatch makes the
        // load itself safe; the atomic is purely an early-out for the
        // overwhelmingly common no-sink path.)
        gSinkInstalled.store(static_cast<bool>(gSink), std::memory_order_release);
    }

    bool hasErrorSink()
    {
        return gSinkInstalled.load(std::memory_order_acquire);
    }

    void stderrErrorSink(const ErrorContext& ctx)
    {
        char buf[512];
        formatErrorContext(ctx, buf, sizeof buf);
        std::fputs("[heifpp] ", stderr);
        std::fputs(buf, stderr);
        std::fputc('\n', stderr);
    }
}  // namespace HEIFPP
