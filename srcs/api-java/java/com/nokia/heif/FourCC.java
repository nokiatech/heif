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
 * Class containing a FourCC code
 */
public class FourCC
{

    private String mContent;

    /**
     * Constructs a new FourCC code from the String
     * @param code FourCC code as a string. Must be 4 characters long
     * @throws Exception Thrown if the given String is not 4 characters long
     */
    public FourCC(String code)
            throws Exception
    {
        if (code.length() != 4)
        {
            throw new Exception(ErrorHandler.INVALID_PARAMETER, "Not a valid FourCC");
        }
        mContent = code;
    }

    /**
     * Protected constructor which doesn't check the length of the string
     * @param code FourCC code as a String
     * @param force Dummy parameter to separate the two constructors
     */
    protected FourCC(String code, boolean force)
    {
        mContent = code;
    }

    @Override
    public String toString()
    {
        return mContent;
    }

    @Override
    public boolean equals(Object obj)
    {
        if (obj instanceof FourCC || obj instanceof String )
        {
            return mContent.equals(obj.toString());
        }
        return false;
    }

}
