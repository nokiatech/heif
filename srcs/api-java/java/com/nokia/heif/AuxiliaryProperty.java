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
 * AuxiliaryProperty defines how an AuxiliaryImage relates to its parent
 * AuxiliaryProperties can be defined by the user
 */
public final class AuxiliaryProperty extends DescriptiveProperty
{

    /**
     * Creates a new AuxiliaryProperty to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public AuxiliaryProperty(HEIF heif)
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
    protected AuxiliaryProperty(HEIF heif, long nativeHandle)
    {
        super(heif,nativeHandle);
    }

    /**
     * Sets the type of the auxiliary property
     * @param type Type as a String
     * @throws Exception
     */
    public void setType(String type)
            throws Exception
    {
        checkState();
        setTypeNative(type);
    }

    /**
     * Gets the type of the auxiliary property
     * @return The type as a String
     * @throws Exception
     */
    public String getType()
            throws Exception
    {
        checkState();
        return getTypeNative();
    }

    /**
     * Sets the subtype of the auxliary property.
     * The subtype is a freely defined blob of data which is processed based on the type of the property
     * @param subtype Subtype as a byte array
     * @throws Exception
     */
    public void setSubType(byte[] subtype)
            throws Exception
    {
        checkState();
        setSubTypeNative(subtype);
    }

    /**
     * Gets the subtype of the auxiliary property as a byte array.
     * The subtype is a freely defined blob of data which is processed based on the type of the property
     * Note that this method does a copy of the data. Getting the ByteBuffer is more optimized
     * @return The subtype as a byte array
     * @throws Exception
     */
    public byte[] getSubTypeAsArray()
            throws Exception
    {
        checkState();
        ByteBuffer dataBuffer = getSubType();
        byte[] dataArray = new byte[dataBuffer.remaining()];
        dataBuffer.get(dataArray);
        return dataArray;
    }

    /**
     * Gets the subtype of the auxiliary property as a ByteBuffer
     * The subtype is a freely defined blob of data which is processed based on the type of the property.
     * Note that the data in the ByteBuffer is owned by the native object so the ByteBuffer is invalidated if this
     * object or its parent HEIF instance is released
     * @return Subtype as a ByteBuffer
     * @throws Exception
     */
    public ByteBuffer getSubType()
            throws Exception
    {
        checkState();
        return getSubTypeNative();
    }

    native private long createContextNative(HEIF heif);

    native private void setTypeNative(String type);

    native private String getTypeNative();

    native private void setSubTypeNative(byte[] subType);

    native private ByteBuffer getSubTypeNative();


}
