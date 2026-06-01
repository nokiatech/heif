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

public final class VVCSample extends VideoSample
{
    /**
     * Creates a new VVCSample and an VVCDecoderConfig for the sample
     *
     * @param heif The parent HEIF instance for the new object.
     * @param decoderConfig Byte array containing the decoder config.
     * @param data Byte array containing the sample data as an encoded VVC frame
     * @param duration The duration of the sample in timescale units
     * @throws Exception
     */
    public VVCSample(HEIF heif, byte[] decoderConfig, byte[] data, long duration)
            throws Exception
    {
        this(heif);
        try
        {
            DecoderConfig decConfig = new VVCDecoderConfig(heif, decoderConfig);

            setDecoderConfig(decConfig);
            setSampleData(data);
            setDuration(duration);
        }
        catch (Exception ex)
        {
            destroy();
            throw ex;
        }
    }

    /**
     * Creates a new VVCSample
     *
     * @param heif The parent HEIF instance for the new object.
     * @param config The decoder config for the new object.
     * @param data The sample data
     * @param duration The duration of the sample in timescale units
     * @throws Exception
     */
    public VVCSample(HEIF heif, VVCDecoderConfig config, byte[] data, long duration)
            throws Exception
    {
        this(heif);
        try
        {
            setDecoderConfig(config);
            setSampleData(data);
            setDuration(duration);
        }
        catch (Exception ex)
        {
            destroy();
            throw ex;
        }
    }

    /**
     * Private constructor
     *
     * @param heif The parent HEIF instance for the new object.
     * @throws Exception
     */
    private VVCSample(HEIF heif)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
    }

    /**
     * Protected constructor, abstract class so not called directly
     *
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected VVCSample(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the decoder config of the sample
     *
     * @return VVCDecoderConfig
     * @throws Exception
     */
    public VVCDecoderConfig getDecoderConfig()
            throws Exception
    {
        return (VVCDecoderConfig) super.getDecoderConfig();
    }

    @Override
    protected boolean checkDecoderConfigType(DecoderConfig config)
    {
        return config instanceof VVCDecoderConfig;
    }

    private native long createContextNative(HEIF heif);
}
