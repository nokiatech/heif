/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

public final class ErrorHandler
{
    // Additional Java errors
    static final public int INVALID_PARAMETER = -2;
    static final public int WRONG_HEIF_INSTANCE = -3;
    static final public int OBJECT_ALREADY_DELETED = -4;
    static final public int INVALID_STRUCTURE = -5;
    static final public int INCORRECT_SUPERCLASS = -6;
    static final public int TYPE_ALREADY_EXISTS = -7;

    // Native C++ error codes
    static final public int OK = 0;
    static final public int INDEX_OUT_OF_BOUNDS = 1;
    static final public int ALREADY_INITIALIZED = 2;
    static final public int PRIMARY_ITEM_NOT_SET = 3;
    static final public int BRANDS_NOT_SET = 4;
    static final public int HIDDEN_PRIMARY_ITEM = 5;
    static final public int INVALID_HANDLE = 6;
    static final public int ALREADY_IN_GROUP = 7;
    static final public int UNDEFINED_ERROR = 999;

    private static final HashMap<Integer, String> ERROR_MESSAGES =
            new HashMap<Integer, String>();

    static
    {
        ERROR_MESSAGES.put(INVALID_PARAMETER,
                           "INVALID_PARAMETER");
        ERROR_MESSAGES.put(WRONG_HEIF_INSTANCE,
                           "WRONG_HEIF_INSTANCE");
        ERROR_MESSAGES.put(OBJECT_ALREADY_DELETED,
                           "OBJECT_ALREADY_DELETED");
        ERROR_MESSAGES.put(INVALID_STRUCTURE,
                           "INVALID_STRUCTURE");

        ERROR_MESSAGES.put(OK,
                           "OK");
        ERROR_MESSAGES.put(INDEX_OUT_OF_BOUNDS,
                           "INDEX_OUT_OF_BOUNDS");
        ERROR_MESSAGES.put(ALREADY_INITIALIZED,
                           "ALREADY_INITIALIZED");
        ERROR_MESSAGES.put(PRIMARY_ITEM_NOT_SET,
                           "PRIMARY_ITEM_NOT_SET");
        ERROR_MESSAGES.put(BRANDS_NOT_SET,
                           "BRANDS_NOT_SET");
        ERROR_MESSAGES.put(HIDDEN_PRIMARY_ITEM,
                           "HIDDEN_PRIMARY_ITEM");
        ERROR_MESSAGES.put(INVALID_HANDLE,
                           "INVALID_HANDLE");
        ERROR_MESSAGES.put(ALREADY_IN_GROUP,
                           "ALREADY_IN_GROUP");



        ERROR_MESSAGES.put(UNDEFINED_ERROR,
                           "UNDEFINED_ERROR");
    }

    ;

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
     * Throws an exception with the given information
     * @param errorCode Error code
     * @param errorInfo Additional information to be printed with the exception
     * @throws Exception
     */
    static void throwException(int errorCode, String errorInfo)
            throws Exception
    {
        throw new Exception(errorCode, errorInfo);
    }
}
