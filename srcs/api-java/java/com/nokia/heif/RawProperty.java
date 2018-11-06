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
 * Rawproperty
 */
public final class RawProperty extends ItemProperty
{

    /**
     * Creates a RawProperty to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public RawProperty(HEIF heif, FourCC type)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        try
        {
            setRawType(type);
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
    protected RawProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the type of the property
     * @return Type as a FourCC
     * @throws Exception
     */
    public FourCC getRawType()
            throws Exception
    {
        checkState();
        return new FourCC(getRawTypeNative());
    }

    public byte[] getDataAsByteArray()
            throws Exception
    {
        ByteBuffer buffer = getData();
        byte[] byteArray = new byte[buffer.remaining()];
        buffer.get(byteArray);
        return byteArray;
    }

    public ByteBuffer getData()
            throws Exception
    {
        checkState();
        ByteBuffer output = getDataNative();
        return output.asReadOnlyBuffer();
    }

    public void setData(byte[] data)
            throws Exception
    {
        checkState();
        checkParameter(data);
        setDataNative(data);
    }

    private void setRawType(FourCC type)
            throws Exception
    {
        checkState();
        checkParameter(type);
        setRawTypeNative(type.toString());
    }

    private native long createContextNative(HEIF heif);

    private native String getRawTypeNative();

    private native void setRawTypeNative(String type);

    private native ByteBuffer getDataNative();

    private native void setDataNative(byte[] data);



}
