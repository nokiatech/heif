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
 * IdentityImageItem adds transformative properties to another existing ImageItem
 */
public final class IdentityImageItem extends DerivedImageItem
{

    /**
     * Creates an IdentityImageItem to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @param imageItem The imageItem this Identity image is based on
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public IdentityImageItem(HEIF heif, ImageItem imageItem)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        try
        {
            setImage(imageItem);
            setSize(imageItem.getSize());
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
    protected IdentityImageItem(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the ImageItem on which this IdentityImageItem is linked
     * @return The linked ImageItem
     * @throws Exception
     */
    public ImageItem getImage()
            throws Exception
    {
        checkState();
        return getImageNative();
    }

    /**
     * Sets the image item for which this IdentityImageItem is based
     * An IdentityImageItem can be linked only to a single ImageItem
     * @param image The ImageItem to be linked
     * @throws Exception
     */
    public void setImage(ImageItem image)
            throws Exception
    {
        checkState();
        checkParameter(image);
        setImageNative(image);
    }

    private native long createContextNative(HEIF heif);

    private native ImageItem getImageNative();
    private native void setImageNative(ImageItem image);
}
