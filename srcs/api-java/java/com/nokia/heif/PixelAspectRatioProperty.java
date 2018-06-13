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
 * PixelAspectRationProperty
 */
public final class PixelAspectRatioProperty extends DescriptiveProperty
{

    /**
     * Creates a PixelAspectRatioProperty to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @param relativeWidth The relative width
     * @param relativeHeight The relative height
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public PixelAspectRatioProperty(HEIF heif, int relativeWidth, int relativeHeight)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        setRelativeWidth(relativeWidth);
        setRelativeHeight(relativeHeight);
    }

    /**
     * Protected constructor, used to create an object from the native side
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected PixelAspectRatioProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the relative width of the property
     * @return Relative width in pixels
     * @throws Exception
     */
    public int getRelativeWidth()
            throws Exception
    {
        checkState();
        return getRelativeWidthNative();
    }

    /**
     * Gets the relative height of the property
     * @return Relative height in pixels
     * @throws Exception
     */
    public int getRelativeHeight()
            throws Exception
    {
        checkState();
        return getRelativeHeightNative();
    }

    /**
     * Sets the relative width for the property
     * @param relativeWidth Relative width in pixels
     * @throws Exception
     */
    public void setRelativeWidth(int relativeWidth)
            throws Exception
    {
        checkState();
        setRelativeWidthNative(relativeWidth);
    }

    /**
     * Sets the relative height in pixels
     * @param relativeHeight Relative height in pixels
     * @throws Exception
     */
    public void setRelativeHeight(int relativeHeight)
            throws Exception
    {
        checkState();
        setRelativeHeightNative(relativeHeight);
    }

    private native long createContextNative(HEIF heif);

    private native int getRelativeWidthNative();

    private native int getRelativeHeightNative();

    private native void setRelativeWidthNative(int relativeWidth);

    private native void setRelativeHeightNative(int relativeHeight);
}
