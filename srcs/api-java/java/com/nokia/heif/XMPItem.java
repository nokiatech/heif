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
 * Item containing XMP metadata
 */
public final class XMPItem extends MimeItem
{
    /**
     * Creates a new XMPItem to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @exception Exception Thrown if the parent HEIF instance is invalid
     */
    public XMPItem(HEIF heif)
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
    protected XMPItem(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    private native long createContextNative(HEIF heif);
}
