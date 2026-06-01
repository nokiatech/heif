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

/**
 * DecoderConfig for L-HEVC (Layered HEVC) data.
 *
 * Mirrors {@link HEVCDecoderConfig} but carries the "lhv1" four-cc
 * for enhancement-layer items defined by ISO/IEC 14496-15 Section 9.6.
 */
public final class LHEVCDecoderConfig extends DecoderConfig
{
    /**
     * Creates a new LHEVCDecoderConfig to the given HEIF instance
     *
     * @param heif The parent HEIF instance for the new object
     * @param config Byte array containing the config data
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public LHEVCDecoderConfig(HEIF heif, byte[] config)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        try
        {
            setConfig(config);
        }
        catch (Exception ex)
        {
            destroy();
            throw ex;
        }
    }

    /**
     * Protected constructor, used to create an object from the native side
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected LHEVCDecoderConfig(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }


    private native long createContextNative(HEIF heif);
}
