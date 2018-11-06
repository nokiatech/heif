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

/**
 * Custom exception used with the HEIF library
 */
public final class Exception extends java.lang.Exception
{
    private static String TAG = "Exception";

    /** Error code which maps to the C++ errors, additional Java errors use negative values */
    private final int mNativeErrorCode;

    /**
     * Constructor
     * @param nativeErrorCode Error code received from the native side (or an additional Java error code)
     * @param info Extra information to be printed with the exception
     */
    public Exception(int nativeErrorCode, String info)
    {
        super(info + " " + ErrorHandler.nativeErrorToString(nativeErrorCode));
        mNativeErrorCode = nativeErrorCode;
    }

    /**
     * Returns the native error code
     * @return Error as integer
     */
    public int getErrorCode()
    {
        return mNativeErrorCode;
    }
}
