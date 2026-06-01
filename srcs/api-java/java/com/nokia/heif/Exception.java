/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

package com.nokia.heif;

/**
 * Custom exception thrown by the HEIF library.
 *
 * <p>In addition to the legacy {@code errorCode + info} pair, exceptions
 * raised by the native side now carry rich diagnostic context populated by
 * the C++ {@code ErrorContext} thread-local: a coarse <b>source category</b>
 * (which subsystem failed - file I/O, parser, validation, ...), the
 * <b>native source location</b> ({@code __FILE__:__LINE__}) where the
 * failure was recorded, and an optional <b>cause message</b> captured from
 * the underlying C++ {@code std::exception::what()} when the failure was
 * triggered by a thrown exception.
 *
 * <p>For backward compatibility with existing client code, all the new
 * fields are optional; the legacy two-arg constructor still works and
 * returns {@code SOURCE_UNKNOWN}, an empty native location, and no cause.
 *
 * <p>The default {@link #getMessage()} now renders the full diagnostic
 * record on a single line so a stack-trace dump from any logging framework
 * (Android Log, java.util.logging, slf4j) shows the embedder everything
 * they need to file a bug or render a meaningful UI message.
 */
public final class Exception extends java.lang.Exception
{
    private static final long serialVersionUID = 2L;

    private static String TAG = "Exception";

    /** Source category sentinel for "no source recorded" / pre-2026 callers. */
    public static final String SOURCE_UNKNOWN = "Unknown";

    /** Error code which maps to the C++ errors, additional Java errors use negative values */
    private final int mNativeErrorCode;

    /** Coarse subsystem the failure originated from (see {@code HEIFPP::ErrorSource}). */
    private final String mSource;

    /** Native source file ({@code __FILE__}) where the error was recorded; "" if unknown. */
    private final String mNativeFile;

    /** Native source line ({@code __LINE__}) where the error was recorded; 0 if unknown. */
    private final int mNativeLine;

    /**
     * Original {@code std::exception::what()} string captured when the failure
     * was triggered by a C++ exception. Empty when the failure path was a
     * plain "guard returned an error" with no upstream exception.
     */
    private final String mCauseMessage;

    /**
     * Legacy two-arg constructor, preserved so older callers compile
     * unchanged. The new diagnostic fields default to "unknown".
     */
    public Exception(int nativeErrorCode, String info)
    {
        this(nativeErrorCode, info, SOURCE_UNKNOWN, "", 0, "");
    }

    /**
     * Full constructor used by {@link ErrorHandler#throwException(int, String,
     * String, String, int, String)} when the native side has populated rich
     * context via {@code HEIFPP::setLastError}.
     */
    public Exception(int nativeErrorCode,
                     String info,
                     String source,
                     String nativeFile,
                     int nativeLine,
                     String causeMessage)
    {
        super(buildMessage(nativeErrorCode, info, source, nativeFile, nativeLine, causeMessage));
        mNativeErrorCode = nativeErrorCode;
        mSource          = (source == null) ? SOURCE_UNKNOWN : source;
        mNativeFile      = (nativeFile == null) ? "" : nativeFile;
        mNativeLine      = nativeLine;
        mCauseMessage    = (causeMessage == null) ? "" : causeMessage;
    }

    /** Native error code (see constants in {@link ErrorHandler}). */
    public int getErrorCode()
    {
        return mNativeErrorCode;
    }

    /**
     * Coarse subsystem name (e.g. {@code "Reader_VvcParser"},
     * {@code "Api_Validation"}). Returns {@link #SOURCE_UNKNOWN} for legacy
     * callers that constructed via the two-arg form.
     */
    public String getSource()
    {
        return mSource;
    }

    /**
     * Native source file ({@code __FILE__}) where the error was recorded.
     * Empty string when not recorded.
     */
    public String getNativeFile()
    {
        return mNativeFile;
    }

    /**
     * Native source line ({@code __LINE__}) where the error was recorded.
     * 0 when not recorded.
     */
    public int getNativeLine()
    {
        return mNativeLine;
    }

    /**
     * Original {@code std::exception::what()} message from the underlying C++
     * exception, when the failure was caused by one. Empty otherwise.
     */
    public String getCauseMessage()
    {
        return mCauseMessage;
    }

    /**
     * Render the diagnostic record as a single human-readable line. Used as
     * the {@code Throwable.message} so default stack-trace dumpers show
     * everything the embedder needs.
     *
     * Format:
     *   "<info> code=<NAME>(<n>) | source=<src> | at <file>:<line> | cause=<...>"
     *
     * Trailing fields are omitted when not populated, so legacy two-arg
     * exceptions still produce a clean "<info> <NAME>" string.
     */
    private static String buildMessage(int nativeErrorCode,
                                       String info,
                                       String source,
                                       String nativeFile,
                                       int nativeLine,
                                       String causeMessage)
    {
        StringBuilder sb = new StringBuilder();
        if (info != null && !info.isEmpty())
        {
            sb.append(info).append(' ');
        }
        sb.append("code=")
          .append(ErrorHandler.nativeErrorToString(nativeErrorCode))
          .append('(').append(nativeErrorCode).append(')');
        if (source != null && !source.isEmpty() && !SOURCE_UNKNOWN.equals(source))
        {
            sb.append(" | source=").append(source);
        }
        if (nativeFile != null && !nativeFile.isEmpty())
        {
            sb.append(" | at ").append(nativeFile);
            if (nativeLine > 0)
            {
                sb.append(':').append(nativeLine);
            }
        }
        if (causeMessage != null && !causeMessage.isEmpty())
        {
            sb.append(" | cause=").append(causeMessage);
        }
        return sb.toString();
    }
}
