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

import java.nio.ByteBuffer;

/**
 * ExifItem, contains Exif metadata
 */
public final class ExifItem extends MetaItem
{

    /**
     * Creates a new ExifItem to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @exception Exception Thrown if the parent HEIF instance is invalid
     */
    public ExifItem(HEIF heif)
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
    protected ExifItem(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }


    /**
     * Returns the item data as a byte array
     * @return The data as a byte array
     * @throws Exception
     */
    public byte[] getDataAsArray()
            throws Exception
    {
        checkState();
        ByteBuffer buffer = getData();
        byte[] data = new byte[buffer.remaining()];
        buffer.get(data);
        return data;
    }

    /**
     * Returns the item data as a BufferBuffer
     * @return The data as a ByteBuffer
     * @throws Exception
     */
    public ByteBuffer getData()
            throws Exception
    {
        checkState();
        return getItemDataNative();
    }

    /**
     * Sets the ite data
     * @param data Data as a byte array
     * @throws Exception
     */
    public void setData(byte[] data)
            throws Exception
    {
        checkState();
        setItemDataNative(data);
    }

    private native long createContextNative(HEIF heif);

    private native ByteBuffer getItemDataNative();
    private native void setItemDataNative(byte[] data);


}
