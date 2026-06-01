/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 *
 *
 */

package com.nokia.heif;

import java.util.HashMap;
import java.util.function.Consumer;

public final class ErrorHandler
{
    // Additional Java errors
    static final public int INVALID_PARAMETER = -2;
    static final public int WRONG_HEIF_INSTANCE = -3;
    static final public int OBJECT_ALREADY_DELETED = -4;
    static final public int INVALID_STRUCTURE = -5;
    static final public int INCORRECT_SUPERCLASS = -6;
    static final public int TYPE_ALREADY_EXISTS = -7;

    // Native C++ error codes. Mirrors HEIFPP::Result in
    // srcs/api-cpp/ErrorCodes.h. Codes 0..9 are STABLE and predate the
    // 2026 expansion; codes 10..99 were added so that Java callers can
    // distinguish e.g. "file not found" from "file corrupt" from "ID does
    // not exist". Before the expansion every parser/IO failure surfaced
    // as UNDEFINED_ERROR (999), which gave embedders no way to render a
    // useful message.

    // Stable codes (0..9)
    static final public int OK = 0;
    static final public int INDEX_OUT_OF_BOUNDS = 1;
    static final public int ALREADY_INITIALIZED = 2;
    static final public int PRIMARY_ITEM_NOT_SET = 3;
    static final public int BRANDS_NOT_SET = 4;
    static final public int HIDDEN_PRIMARY_ITEM = 5;
    static final public int INVALID_HANDLE = 6;
    static final public int ALREADY_IN_GROUP = 7;
    static final public int ALREADY_SET = 8;
    static final public int INVALID_CONFIG = 9;

    // File / IO
    static final public int FILE_OPEN_ERROR = 10;
    static final public int FILE_READ_ERROR = 11;
    static final public int FILE_HEADER_ERROR = 12;
    static final public int BUFFER_SIZE_TOO_SMALL = 13;
    static final public int BYTESTREAM_READ_ERROR = 14;
    static final public int MEMORY_TOO_SMALL_BUFFER = 15;

    // State
    static final public int UNINITIALIZED = 20;
    static final public int NOT_APPLICABLE = 21;
    static final public int FTYP_ALREADY_WRITTEN = 22;
    static final public int ALLOCATOR_ALREADY_SET = 23;

    // Bad parameters / IDs
    static final public int INVALID_FUNCTION_PARAMETER = 30;
    static final public int INVALID_ITEM_ID = 31;
    static final public int INVALID_GROUP_ID = 32;
    static final public int INVALID_MEDIADATA_ID = 33;
    static final public int INVALID_METADATAITEM_ID = 34;
    static final public int INVALID_PROPERTY_INDEX = 35;
    static final public int INVALID_REFERENCE_COUNT = 36;
    static final public int INVALID_SAMPLE_DESCRIPTION_INDEX = 37;
    static final public int INVALID_SEGMENT = 38;
    static final public int INVALID_SEQUENCE_ID = 39;
    static final public int INVALID_DECODER_CONFIG_ID = 40;
    static final public int INVALID_SEQUENCE_IMAGE_ID = 41;

    // Content
    static final public int MEDIA_PARSING_ERROR = 50;
    static final public int INVALID_MEDIA_FORMAT = 51;
    static final public int UNSUPPORTED_CODE_TYPE = 52;
    static final public int DECODER_CONFIGURATION_ERROR = 53;
    static final public int NO_MEDIA = 54;
    static final public int CONTENT_TYPE_NOT_SET = 55;
    static final public int PROTECTED_ITEM = 56;
    static final public int UNPROTECTED_ITEM = 57;

    // Mini / extension
    static final public int TOO_MANY_ITEMS = 60;
    static final public int MISSING_PROPERTIES = 61;
    static final public int MULTIPLE_GAINMAPS = 62;
    static final public int INVALID_EXIF_ORIENTATION_VALUE = 63;
    static final public int VVC_DISABLED = 64;

    // Catch-all
    static final public int UNDEFINED_ERROR = 999;

    private static final HashMap<Integer, String> ERROR_MESSAGES =
            new HashMap<Integer, String>();

    static
    {
        ERROR_MESSAGES.put(INVALID_PARAMETER,             "INVALID_PARAMETER");
        ERROR_MESSAGES.put(WRONG_HEIF_INSTANCE,           "WRONG_HEIF_INSTANCE");
        ERROR_MESSAGES.put(OBJECT_ALREADY_DELETED,        "OBJECT_ALREADY_DELETED");
        ERROR_MESSAGES.put(INVALID_STRUCTURE,             "INVALID_STRUCTURE");
        ERROR_MESSAGES.put(INCORRECT_SUPERCLASS,          "INCORRECT_SUPERCLASS");
        ERROR_MESSAGES.put(TYPE_ALREADY_EXISTS,           "TYPE_ALREADY_EXISTS");

        ERROR_MESSAGES.put(OK,                            "OK");
        ERROR_MESSAGES.put(INDEX_OUT_OF_BOUNDS,           "INDEX_OUT_OF_BOUNDS");
        ERROR_MESSAGES.put(ALREADY_INITIALIZED,           "ALREADY_INITIALIZED");
        ERROR_MESSAGES.put(PRIMARY_ITEM_NOT_SET,          "PRIMARY_ITEM_NOT_SET");
        ERROR_MESSAGES.put(BRANDS_NOT_SET,                "BRANDS_NOT_SET");
        ERROR_MESSAGES.put(HIDDEN_PRIMARY_ITEM,           "HIDDEN_PRIMARY_ITEM");
        ERROR_MESSAGES.put(INVALID_HANDLE,                "INVALID_HANDLE");
        ERROR_MESSAGES.put(ALREADY_IN_GROUP,              "ALREADY_IN_GROUP");
        ERROR_MESSAGES.put(ALREADY_SET,                   "ALREADY_SET");
        ERROR_MESSAGES.put(INVALID_CONFIG,                "INVALID_CONFIG");

        ERROR_MESSAGES.put(FILE_OPEN_ERROR,               "FILE_OPEN_ERROR");
        ERROR_MESSAGES.put(FILE_READ_ERROR,               "FILE_READ_ERROR");
        ERROR_MESSAGES.put(FILE_HEADER_ERROR,             "FILE_HEADER_ERROR");
        ERROR_MESSAGES.put(BUFFER_SIZE_TOO_SMALL,         "BUFFER_SIZE_TOO_SMALL");
        ERROR_MESSAGES.put(BYTESTREAM_READ_ERROR,         "BYTESTREAM_READ_ERROR");
        ERROR_MESSAGES.put(MEMORY_TOO_SMALL_BUFFER,       "MEMORY_TOO_SMALL_BUFFER");

        ERROR_MESSAGES.put(UNINITIALIZED,                 "UNINITIALIZED");
        ERROR_MESSAGES.put(NOT_APPLICABLE,                "NOT_APPLICABLE");
        ERROR_MESSAGES.put(FTYP_ALREADY_WRITTEN,          "FTYP_ALREADY_WRITTEN");
        ERROR_MESSAGES.put(ALLOCATOR_ALREADY_SET,         "ALLOCATOR_ALREADY_SET");

        ERROR_MESSAGES.put(INVALID_FUNCTION_PARAMETER,    "INVALID_FUNCTION_PARAMETER");
        ERROR_MESSAGES.put(INVALID_ITEM_ID,               "INVALID_ITEM_ID");
        ERROR_MESSAGES.put(INVALID_GROUP_ID,              "INVALID_GROUP_ID");
        ERROR_MESSAGES.put(INVALID_MEDIADATA_ID,          "INVALID_MEDIADATA_ID");
        ERROR_MESSAGES.put(INVALID_METADATAITEM_ID,       "INVALID_METADATAITEM_ID");
        ERROR_MESSAGES.put(INVALID_PROPERTY_INDEX,        "INVALID_PROPERTY_INDEX");
        ERROR_MESSAGES.put(INVALID_REFERENCE_COUNT,       "INVALID_REFERENCE_COUNT");
        ERROR_MESSAGES.put(INVALID_SAMPLE_DESCRIPTION_INDEX, "INVALID_SAMPLE_DESCRIPTION_INDEX");
        ERROR_MESSAGES.put(INVALID_SEGMENT,               "INVALID_SEGMENT");
        ERROR_MESSAGES.put(INVALID_SEQUENCE_ID,           "INVALID_SEQUENCE_ID");
        ERROR_MESSAGES.put(INVALID_DECODER_CONFIG_ID,     "INVALID_DECODER_CONFIG_ID");
        ERROR_MESSAGES.put(INVALID_SEQUENCE_IMAGE_ID,     "INVALID_SEQUENCE_IMAGE_ID");

        ERROR_MESSAGES.put(MEDIA_PARSING_ERROR,           "MEDIA_PARSING_ERROR");
        ERROR_MESSAGES.put(INVALID_MEDIA_FORMAT,          "INVALID_MEDIA_FORMAT");
        ERROR_MESSAGES.put(UNSUPPORTED_CODE_TYPE,         "UNSUPPORTED_CODE_TYPE");
        ERROR_MESSAGES.put(DECODER_CONFIGURATION_ERROR,   "DECODER_CONFIGURATION_ERROR");
        ERROR_MESSAGES.put(NO_MEDIA,                      "NO_MEDIA");
        ERROR_MESSAGES.put(CONTENT_TYPE_NOT_SET,          "CONTENT_TYPE_NOT_SET");
        ERROR_MESSAGES.put(PROTECTED_ITEM,                "PROTECTED_ITEM");
        ERROR_MESSAGES.put(UNPROTECTED_ITEM,              "UNPROTECTED_ITEM");

        ERROR_MESSAGES.put(TOO_MANY_ITEMS,                "TOO_MANY_ITEMS");
        ERROR_MESSAGES.put(MISSING_PROPERTIES,            "MISSING_PROPERTIES");
        ERROR_MESSAGES.put(MULTIPLE_GAINMAPS,             "MULTIPLE_GAINMAPS");
        ERROR_MESSAGES.put(INVALID_EXIF_ORIENTATION_VALUE,"INVALID_EXIF_ORIENTATION_VALUE");
        ERROR_MESSAGES.put(VVC_DISABLED,                  "VVC_DISABLED");

        ERROR_MESSAGES.put(UNDEFINED_ERROR,               "UNDEFINED_ERROR");
    }

    /**
     * Converts a native error code integer to a descriptive String
     * @param errorCode Error code to be converted
     * @return Human readable text matching the error
     */
    public static String nativeErrorToString(int errorCode)
    {
        return ERROR_MESSAGES.get(errorCode);
    }

    /**
     * Legacy entry point - throws an exception with just code + info, no
     * native source location or cause. Still used by Java-side throwers and
     * by callers compiled against pre-2026 native libraries.
     *
     * <p>Notifies the registered error sink (if any) BEFORE throwing, so
     * embedders observing failures via {@link #setErrorSink} see the same
     * Exception instance that will then unwind the stack.
     *
     * @param errorCode Error code
     * @param errorInfo Additional information to be printed with the exception
     * @throws Exception
     */
    static void throwException(int errorCode, String errorInfo)
            throws Exception
    {
        Exception e = new Exception(errorCode, errorInfo);
        dispatchToSink(e);
        throw e;
    }

    /**
     * Rich entry point used by the JNI layer when the native side has
     * populated {@code HEIFPP::ErrorContext}. The extra parameters survive
     * the JNI hop so embedders see "where" and "why" alongside "what".
     *
     * <p>Notifies the registered error sink (if any) BEFORE throwing, so
     * embedders observing failures via {@link #setErrorSink} see the same
     * Exception instance that will then unwind the stack.
     *
     * <p>Kept package-private and called only from native code; client code
     * should not invoke this directly.
     *
     * @param errorCode    native error code (see constants above)
     * @param errorInfo    JNI-call-site message (e.g. "Loading failed")
     * @param source       coarse subsystem name from {@code errorSourceName()}
     * @param nativeFile   {@code __FILE__} of the C++ failure site
     * @param nativeLine   {@code __LINE__} of the C++ failure site
     * @param causeMessage upstream {@code std::exception::what()}, or "" if none
     */
    static void throwException(int errorCode,
                               String errorInfo,
                               String source,
                               String nativeFile,
                               int nativeLine,
                               String causeMessage)
            throws Exception
    {
        Exception e = new Exception(errorCode, errorInfo, source, nativeFile, nativeLine, causeMessage);
        dispatchToSink(e);
        throw e;
    }

    // ---------------------------------------------------------------------
    //  Error sink (added 3.8.0)
    //
    //  Mirror of HEIF::setErrorSink / HEIFPP::setErrorSink at the Java
    //  layer. Embedders install a Consumer<Exception> once at startup;
    //  the JNI layer invokes it whenever HEIFPP records a non-OK result,
    //  BEFORE the corresponding Exception is thrown into the Java stack.
    //
    //  Why "before throw":
    //    - Embedders that try/catch the Exception still see exactly one
    //      structured record (sink fires, then they catch).
    //    - Embedders that let the Exception propagate also see the record
    //      (sink fires, then JVM stack-traces the throw).
    //    - Embedders that pre-empt the Exception (catch and swallow) at
    //      least get the diagnostic logged once, instead of silently
    //      losing it.
    //
    //  Why a Consumer<Exception> rather than a custom listener interface:
    //    - Java 8 lambdas / method refs are the idiomatic shape:
    //          ErrorHandler.setErrorSink(e -> Log.e(TAG, format(e)));
    //          ErrorHandler.setErrorSink(System.err::println);
    //    - The Exception object the sink receives is the SAME instance
    //      that will then be thrown, so any state attached to it (cause,
    //      stack trace, suppressed exceptions) is consistent between the
    //      sink notification and the catch-block view.
    //
    //  Default state: no sink installed -> library is silent at the Java
    //  layer (an Exception still gets thrown for the embedder's catch).
    // ---------------------------------------------------------------------

    /**
     * Process-wide sink. We don't bother with thread locals - the same
     * reasoning as the C++ side applies: typical sinks are a single
     * embedder logger which already serialises its own writes. {@code
     * volatile} is sufficient because we only ever assign a fully-
     * constructed reference (publish) or null (uninstall), and
     * Consumer.accept is not subject to torn reads.
     */
    private static volatile Consumer<Exception> sErrorSink = null;

    /**
     * Install (or replace) the process-wide HEIF error sink. Pass {@code
     * null} to uninstall. Default state: no sink installed.
     *
     * <p>The sink is invoked from the JNI layer immediately before the
     * {@link Exception} is thrown into the Java stack, on the thread that
     * triggered the failure. The same {@code Exception} instance the sink
     * receives is the one subsequently thrown.
     *
     * <p>Sinks must not throw; any RuntimeException escaping the sink is
     * swallowed by the JNI layer to preserve the original failure path.
     *
     * <p>Typical use:
     * <pre>{@code
     * ErrorHandler.setErrorSink(e ->
     *     Log.e("heif", ErrorHandler.format(e)));
     * }</pre>
     *
     * @param sink consumer invoked per failure; {@code null} to uninstall
     */
    public static void setErrorSink(Consumer<Exception> sink)
    {
        sErrorSink = sink;
    }

    /** True iff a sink is currently installed. */
    public static boolean hasErrorSink()
    {
        return sErrorSink != null;
    }

    /**
     * Internal entry point invoked from the JNI layer immediately before
     * the matching exception is thrown. Package-private; embedders should
     * use {@link #setErrorSink} instead of calling this directly.
     *
     * <p>Wrapped in a try/catch so a misbehaving sink cannot prevent the
     * Exception from being thrown into the embedder's catch block - that
     * would silently swallow errors, which is strictly worse than a
     * dropped log line.
     */
    static void dispatchToSink(Exception e)
    {
        Consumer<Exception> sink = sErrorSink;   // single volatile read
        if (sink == null)
        {
            return;
        }
        try
        {
            sink.accept(e);
        }
        catch (Throwable ignored)
        {
            // Sink misbehaved. We deliberately swallow rather than
            // propagate: if we let this throw, it would either replace
            // the original Exception (losing the diagnostic) or terminate
            // the process. Neither is what an embedder asked for by
            // installing a sink.
        }
    }

    /**
     * Format a {@link Exception} as a single human-readable line matching
     * the C++ {@code HEIF::formatErrorInfo} layout. Embedders can use this
     * uniformly across stderr / android.util.Log / slf4j without each
     * embedder reinventing the format.
     *
     * <p>Format:
     * <pre>
     *   "[heif] code=&lt;NAME&gt;(&lt;n&gt;) | source=&lt;src&gt; | at &lt;file&gt;:&lt;line&gt; | cause=&lt;...&gt; | info=&lt;...&gt;"
     * </pre>
     * Trailing fields are omitted when empty so legacy 2-arg exceptions
     * still produce a clean record.
     *
     * @param e the exception to format; null produces "&lt;null&gt;"
     */
    public static String format(Exception e)
    {
        if (e == null)
        {
            return "<null>";
        }
        StringBuilder sb = new StringBuilder("[heif] code=");
        sb.append(nativeErrorToString(e.getErrorCode()))
          .append('(').append(e.getErrorCode()).append(')');

        if (e.getSource() != null
            && !e.getSource().isEmpty()
            && !Exception.SOURCE_UNKNOWN.equals(e.getSource()))
        {
            sb.append(" | source=").append(e.getSource());
        }
        if (e.getNativeFile() != null && !e.getNativeFile().isEmpty())
        {
            sb.append(" | at ").append(e.getNativeFile());
            if (e.getNativeLine() > 0)
            {
                sb.append(':').append(e.getNativeLine());
            }
        }
        if (e.getCauseMessage() != null && !e.getCauseMessage().isEmpty())
        {
            sb.append(" | cause=").append(e.getCauseMessage());
        }
        // Exception.getMessage() already includes most of the above (it's
        // built by Exception.buildMessage). Surface the bare info string
        // separately so embedders who want it can have it. We extract the
        // leading "<info>" prefix from getMessage if there is one.
        String msg = e.getMessage();
        if (msg != null)
        {
            int firstCode = msg.indexOf(" code=");
            if (firstCode > 0)
            {
                String info = msg.substring(0, firstCode).trim();
                if (!info.isEmpty())
                {
                    sb.append(" | info=").append(info);
                }
            }
        }
        return sb.toString();
    }

    /**
     * Convenience sink that writes {@link #format(Exception)} to {@code
     * System.err}. Provided so demos and JVM CLI tools can opt in with
     * one line:
     * <pre>{@code
     * ErrorHandler.setErrorSink(ErrorHandler.STDERR_SINK);
     * }</pre>
     */
    public static final Consumer<Exception> STDERR_SINK =
            e -> System.err.println(format(e));
}
