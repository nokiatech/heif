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

public abstract class MimeItem extends MetaItem
{
    /**
     * Constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @exception Exception Thrown if the parent HEIF instance is invalid
     */
    protected MimeItem(HEIF heif)
            throws Exception
    {
        super(heif);
    }
    /**
     * Protected constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected MimeItem(HEIF heif, long nativeHandle)
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
        byte[] output = new byte[buffer.remaining()];
        buffer.get(output);
        return output;
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

    private native ByteBuffer getItemDataNative();
    private native void setItemDataNative(byte[] data);
}
