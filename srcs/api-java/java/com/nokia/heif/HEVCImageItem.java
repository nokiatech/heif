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
 * A CodedImageItem which contains HEVC encoded data.
 * Works just like a CodedImageItem.
 */
public final class HEVCImageItem extends CodedImageItem
{

    /**
     * Creates a new HEVCImageItem to the given HEIF instance with the given image data
     * Also creates a corresponding HEVCDecoderConfig object with the given decoder config
     *
     * @param heif The parent HEIF instance for the new object
     * @param size Size of the image
     * @param decoderConfig Byte array containing the decoder config
     * @param imageData Byte array containing the image data as an encoded HEVC frame
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public HEVCImageItem(HEIF heif, Size size, byte[] decoderConfig, byte[] imageData)
            throws Exception
    {
        this(heif, size);
        try
        {
            DecoderConfig decConfig = new HEVCDecoderConfig(heif, decoderConfig);

            setDecoderConfig(decConfig);
            setItemData(imageData);
        }
        catch (Exception ex)
        {
            destroy();
            throw ex;
        }
    }

    /**
     * Creates a new HEVCImageItem to the given HEIF instance with the given image data and HEVCDecoderConfig
     *
     * @param heif The parent HEIF instance for the new object
     * @param decoderConfig DecoderConfig for the image
     * @param imageData Byte array containing the image data as an encoded HEVC frame
     * @throws Exception Thrown if the parent HEIF instance or the decoderconfig is invalid
     */
    public HEVCImageItem(HEIF heif, Size size, HEVCDecoderConfig decoderConfig, byte[] imageData)
            throws Exception
    {
        this(heif, size);
        try
        {
            setDecoderConfig(decoderConfig);
            setItemData(imageData);
        }
        catch (Exception ex)
        {
            destroy();
            throw ex;
        }
    }

    /**
     * Creates a new HEVCImageItem to the given HEIF instance
     *
     * @param heif The parent HEIF instance for the new object
     * @param size Size of the image
     * @throws Exception
     */
    private HEVCImageItem(HEIF heif, Size size)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        try
        {
            setSize(size);
        }
        catch (Exception ex)
        {
            destroy();
            throw ex;
        }
    }

    /**
     * Protected constructor, used to create an object from the native side
     *
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    public HEVCImageItem(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the decoder config of the image
     *
     * @return HEVCDecoderConfig
     * @throws Exception
     */
    public HEVCDecoderConfig getDecoderConfig()
            throws Exception
    {
        return (HEVCDecoderConfig) super.getDecoderConfig();
    }

    @Override
    protected boolean checkDecoderConfigType(DecoderConfig config)
    {
        return config instanceof HEVCDecoderConfig;
    }

    private native long createContextNative(HEIF heif);
}
