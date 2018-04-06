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

import java.util.ArrayList;
import java.util.List;

/**
 * DerivedImageItems are ImageItems which are derived from one or more other ImageItems
 * DerivedImageItem is abstract and is implemented by the following classes
 * @see GridImageItem
 * @see OverlayImageItem
 * @see IdentityImageItem
 */
public abstract class DerivedImageItem extends ImageItem
{

    /**
     * Constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @exception Exception Thrown if the parent HEIF instance is invalid
     */
    protected DerivedImageItem(HEIF heif)
            throws Exception
    {
        super(heif);
    }

    /**
     * Protected constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected DerivedImageItem(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns all the source images of the DerivedImageItem.
     * If an image is added several times to the DerivedImageItem to multiple positions,
     * it is listed multiple times as a source image
     * @return List of all the source images of this DerivedImageItem
     * @throws Exception
     */
    public List<ImageItem> getSourceImages()
            throws Exception
    {
        checkState();
        int count = getSourceImageCountNative();
        List<ImageItem> images = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            images.add(getSourceImageNative(index));
        }
        return images;
    }

    /**
     * Sets the size of the image
     * @param size The size of the image
     * @throws Exception
     */
    public void setSize(Size size)
            throws Exception
    {
        super.setSize(size);
    }

    private native int getSourceImageCountNative();

    private native ImageItem getSourceImageNative(int index);
}
