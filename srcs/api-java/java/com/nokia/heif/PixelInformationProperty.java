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

public final class PixelInformationProperty extends DescriptiveProperty
{

    /**
     * Creates a PixelInformationProperty to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public PixelInformationProperty(HEIF heif)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
    }

    /**
     * Protected constructor, used to create an object from the native side
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected PixelInformationProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Gets the Pixel information
     * @return Pixel information as a byte array
     * @throws Exception
     */
    public byte[] getPixelInformation()
            throws Exception
    {
        checkState();
        ByteBuffer dataAsBuffer = getPixelInformationNative();
        byte[] dataAsArray = new byte[dataAsBuffer.remaining()];
        dataAsBuffer.get(dataAsArray);
        return dataAsArray;
    }

    /**
     * Sets the pixel information
     * @param pixelInformation Pixel information as a byte array
     * @throws Exception
     */
    public void setPixelInformation(byte[] pixelInformation)
            throws Exception
    {
        checkState();
        checkParameter(pixelInformation);
        setPixelInformationNative(pixelInformation);
    }

    private native long createContextNative(HEIF heif);

    private native ByteBuffer getPixelInformationNative();

    private native void setPixelInformationNative(byte[] data);
}
