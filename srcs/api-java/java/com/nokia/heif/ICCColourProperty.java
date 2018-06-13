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


import java.nio.ByteBuffer;

public final class ICCColourProperty extends ColourInformationProperty
{
    /**
     * Creates a new ICC ColourProperty to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @param restricted If the restricted ICC profile should be used
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public ICCColourProperty(HEIF heif, boolean restricted)
            throws Exception
    {
        super(heif);
        if (restricted)
        {
            setColourTypeNative(ColourInformationProperty.ICC_RESTRICTED_PROFILE.toString());
        }
        else
        {
            setColourTypeNative(ColourInformationProperty.ICC_UNRESTRICTED_PROFILE.toString());
        }
    }

    /**
     * Protected constructor, used to create an object from the native side
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected ICCColourProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Gets the ICC profile
     * @return ICC profile as a byte array
     * @throws Exception
     */
    public byte[] getProfileAsByteArray()
            throws Exception
    {
        checkState();
        ByteBuffer dataAsBuffer = getProfile();
        byte[] dataAsArray = new byte[dataAsBuffer.remaining()];
        dataAsBuffer.get(dataAsArray);
        return dataAsArray;
    }

    /**
     * Gets the ICC profile
     * @return ICC profile as a byte buffer
     * @throws Exception
     */
    public ByteBuffer getProfile()
            throws Exception
    {
        checkState();
        return getICCProfileNative();
    }

    /**
     * Sets the ICC profile
     * @param profile ICC profile as a byte array
     * @throws Exception
     */
    public void setProfile(byte[] profile)
            throws Exception
    {
        checkState();
        checkParameter(profile);
        setICCProfileNative(profile);
    }
}
