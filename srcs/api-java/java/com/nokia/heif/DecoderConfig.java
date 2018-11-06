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
 * DecoderConfig contains the decoder specific information attached to a CodedImageItem
 */
public class DecoderConfig extends Base
{

    /**
     * Constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @exception Exception Thrown if the parent HEIF instance is invalid
     */
    protected DecoderConfig(HEIF heif)
            throws Exception
    {
        super(heif);
    }

    /**
     * Protected constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected DecoderConfig(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Sets the decoder specific information
     * @param config Config as a byte array
     * @throws Exception
     */
    public void setConfig(byte[] config)
            throws Exception
    {
        checkState();
        checkParameter(config);
        setConfigNative(config);
    }

    /**
     * Returns the decoder specific information
     * @return The decoder specific information as a byte array
     * @throws Exception
     */
    public byte[] getConfig()
            throws Exception
    {
        checkState();
        return getConfigNative();
    }

    @Override
    protected void destroyNative()
    {
        destroyContextNative();
    }


    private native void destroyContextNative();

    private native void setConfigNative(byte[] config);

    private native byte[] getConfigNative();
}
