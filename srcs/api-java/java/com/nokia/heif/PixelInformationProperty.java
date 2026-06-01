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

import java.nio.ByteBuffer;

/**
 * PixelInformationProperty (pixi) describes pixel information for image channels.
 * Contains:
 * - numChannels: Number of channels in the image
 * - bitsPerChannel: Array of bit depths for each channel (e.g., {8, 8, 8} for 8-bit RGB)
 * - Per-channel data including component format, subsampling info, and channel labels
 */
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
     * Gets the number of channels
     * @return Number of channels
     * @throws Exception
     */
    public int getNumChannels()
            throws Exception
    {
        checkState();
        return getNumChannelsNative();
    }

    /**
     * Gets the bits per channel information.
     * Each element in the returned array represents the bit depth of a channel.
     * For example, {8, 8, 8} for an 8-bit RGB image.
     * @return Bits per channel as a byte array
     * @throws Exception
     */
    public byte[] getBitsPerChannel()
            throws Exception
    {
        checkState();
        ByteBuffer dataAsBuffer = getBitsPerChannelNative();
        if (dataAsBuffer == null || dataAsBuffer.remaining() == 0)
        {
            return new byte[0];
        }
        byte[] dataAsArray = new byte[dataAsBuffer.remaining()];
        dataAsBuffer.get(dataAsArray);
        return dataAsArray;
    }

    /**
     * Sets the bits per channel information.
     * Each element in the array represents the bit depth of a channel.
     * This also sets numChannels to the length of the array.
     * For example, {8, 8, 8} for an 8-bit RGB image, or {10, 10, 10} for 10-bit.
     * @param bitsPerChannel Array of bit depths for each channel
     * @throws Exception
     */
    public void setBitsPerChannel(byte[] bitsPerChannel)
            throws Exception
    {
        checkState();
        checkParameter(bitsPerChannel);
        setBitsPerChannelNative(bitsPerChannel);
    }

    /**
     * Gets the component format for a specific channel.
     * @param channelIndex The channel index (0-based)
     * @return Component format value
     * @throws Exception
     */
    public int getComponentFormat(int channelIndex)
            throws Exception
    {
        checkState();
        return getComponentFormatNative(channelIndex);
    }

    /**
     * Sets the component format for a specific channel.
     * @param channelIndex The channel index (0-based)
     * @param componentFormat Component format value
     * @throws Exception
     */
    public void setComponentFormat(int channelIndex, int componentFormat)
            throws Exception
    {
        checkState();
        setComponentFormatNative(channelIndex, componentFormat);
    }

    /**
     * Gets the subsampling flag for a specific channel.
     * @param channelIndex The channel index (0-based)
     * @return true if subsampling is enabled
     * @throws Exception
     */
    public boolean getSubSamplingFlag(int channelIndex)
            throws Exception
    {
        checkState();
        return getSubSamplingFlagNative(channelIndex);
    }

    /**
     * Sets the subsampling flag for a specific channel.
     * @param channelIndex The channel index (0-based)
     * @param flag true to enable subsampling
     * @throws Exception
     */
    public void setSubSamplingFlag(int channelIndex, boolean flag)
            throws Exception
    {
        checkState();
        setSubSamplingFlagNative(channelIndex, flag);
    }

    /**
     * Gets the channel label flag for a specific channel.
     * @param channelIndex The channel index (0-based)
     * @return true if channel label is present
     * @throws Exception
     */
    public boolean getChannelLabelFlag(int channelIndex)
            throws Exception
    {
        checkState();
        return getChannelLabelFlagNative(channelIndex);
    }

    /**
     * Sets the channel label flag for a specific channel.
     * @param channelIndex The channel index (0-based)
     * @param flag true if channel label is present
     * @throws Exception
     */
    public void setChannelLabelFlag(int channelIndex, boolean flag)
            throws Exception
    {
        checkState();
        setChannelLabelFlagNative(channelIndex, flag);
    }

    /**
     * Gets the subsampling type for a specific channel.
     * @param channelIndex The channel index (0-based)
     * @return Subsampling type value
     * @throws Exception
     */
    public int getSubSamplingType(int channelIndex)
            throws Exception
    {
        checkState();
        return getSubSamplingTypeNative(channelIndex);
    }

    /**
     * Sets the subsampling type for a specific channel.
     * @param channelIndex The channel index (0-based)
     * @param type Subsampling type value
     * @throws Exception
     */
    public void setSubSamplingType(int channelIndex, int type)
            throws Exception
    {
        checkState();
        setSubSamplingTypeNative(channelIndex, type);
    }

    /**
     * Gets the subsampling location for a specific channel.
     * @param channelIndex The channel index (0-based)
     * @return Subsampling location value
     * @throws Exception
     */
    public int getSubSamplingLocation(int channelIndex)
            throws Exception
    {
        checkState();
        return getSubSamplingLocationNative(channelIndex);
    }

    /**
     * Sets the subsampling location for a specific channel.
     * @param channelIndex The channel index (0-based)
     * @param location Subsampling location value
     * @throws Exception
     */
    public void setSubSamplingLocation(int channelIndex, int location)
            throws Exception
    {
        checkState();
        setSubSamplingLocationNative(channelIndex, location);
    }

    /**
     * Gets the channel label for a specific channel.
     * @param channelIndex The channel index (0-based)
     * @return Channel label string
     * @throws Exception
     */
    public String getChannelLabel(int channelIndex)
            throws Exception
    {
        checkState();
        return getChannelLabelNative(channelIndex);
    }

    /**
     * Sets the channel label for a specific channel.
     * @param channelIndex The channel index (0-based)
     * @param label Channel label string
     * @throws Exception
     */
    public void setChannelLabel(int channelIndex, String label)
            throws Exception
    {
        checkState();
        setChannelLabelNative(channelIndex, label);
    }

    // Legacy methods for backwards compatibility
    /**
     * @deprecated Use {@link #getBitsPerChannel()} instead
     */
    @Deprecated
    public byte[] getPixelInformation()
            throws Exception
    {
        return getBitsPerChannel();
    }

    /**
     * @deprecated Use {@link #setBitsPerChannel(byte[])} instead
     */
    @Deprecated
    public void setPixelInformation(byte[] pixelInformation)
            throws Exception
    {
        setBitsPerChannel(pixelInformation);
    }

    private native long createContextNative(HEIF heif);

    private native int getNumChannelsNative();

    private native ByteBuffer getBitsPerChannelNative();

    private native void setBitsPerChannelNative(byte[] data);

    private native int getComponentFormatNative(int channelIndex);

    private native void setComponentFormatNative(int channelIndex, int format);

    private native boolean getSubSamplingFlagNative(int channelIndex);

    private native void setSubSamplingFlagNative(int channelIndex, boolean flag);

    private native boolean getChannelLabelFlagNative(int channelIndex);

    private native void setChannelLabelFlagNative(int channelIndex, boolean flag);

    private native int getSubSamplingTypeNative(int channelIndex);

    private native void setSubSamplingTypeNative(int channelIndex, int type);

    private native int getSubSamplingLocationNative(int channelIndex);

    private native void setSubSamplingLocationNative(int channelIndex, int location);

    private native String getChannelLabelNative(int channelIndex);

    private native void setChannelLabelNative(int channelIndex, String label);
}
