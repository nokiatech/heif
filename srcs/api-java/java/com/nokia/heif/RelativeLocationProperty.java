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
 * RelativeLocationProperty
 */
public final class RelativeLocationProperty extends DescriptiveProperty
{

    /**
     * Creates a RelativeLocationProperty to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public RelativeLocationProperty(HEIF heif, int horizontalOffset, int verticalOffset)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        setVerticallOffset(verticalOffset);
        setHorizontalOffset(horizontalOffset);
    }

    /**
     * Protected constructor, used to create an object from the native side
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected RelativeLocationProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Gets the horizontal offset
     * @return Horizontal offset in pixels
     * @throws Exception
     */
    public int getHorizontalOffset()
            throws Exception
    {
        checkState();
        return getHorizontalOffsetNative();
    }

    /**
     * Gets the vertical offset
     * @return Vertical offset in pixels
     * @throws Exception
     */
    public int getVerticalOffset()
            throws Exception
    {
        checkState();
        return getVerticalOffsetNative();
    }

    /**
     * Sets the horizontal offset
     * @param offset Horizontal offset in pixels
     * @throws Exception
     */
    public void setHorizontalOffset(int offset)
            throws Exception
    {
        checkState();
        setHorizontalOffsetNative(offset);
    }

    /**
     * Sets the vertical offset
     * @param offset Vertical offset in pixels
     * @throws Exception
     */
    public void setVerticallOffset(int offset)
            throws Exception
    {
        checkState();
        setVerticalOffsetNative(offset);
    }


    private native long createContextNative(HEIF heif);

    private native int getHorizontalOffsetNative();

    private native int getVerticalOffsetNative();

    private native void setHorizontalOffsetNative(int offset);

    private native void setVerticalOffsetNative(int offset);
}
