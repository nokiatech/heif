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


public final class RotateProperty extends TransformativeProperty
{

    /**
     * Creates a RotateProperty to the given HEIF instance with the given rotation value
     * @param heif The parent HEIF instance for the new object
     * @param rotation The rotation value
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public RotateProperty(HEIF heif, Rotation rotation)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        setRotation(rotation);
    }

    /**
     * Protected constructor, used to create an object from the native side
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected RotateProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    public enum Rotation
    {
        DEGREES_0(0),
        DEGREES_90(90),
        DEGREES_180(180),
        DEGREES_270(270);

        private int value;

        Rotation(int value)
        {
            this.value = value;
        }

        public int getValue()
        {
            return value;
        }
    }


    public void setRotation(Rotation rotation)
            throws Exception
    {
        checkState();
        setRotationNative(rotation.getValue());
    }

    public Rotation getRotation()
            throws Exception
    {
        checkState();
        int rotation = getRotationNative();

        if (rotation == Rotation.DEGREES_90.getValue())
        {
            return Rotation.DEGREES_90;
        }
        else
        {
            if (rotation == Rotation.DEGREES_180.getValue())
            {
                return Rotation.DEGREES_180;
            }
            else
            {
                if (rotation == Rotation.DEGREES_270.getValue())
                {
                    return Rotation.DEGREES_270;
                }
                else
                {
                    return Rotation.DEGREES_0;
                }
            }
        }
    }

    private native long createContextNative(HEIF heif);

    private native void setRotationNative(int rotation);

    private native int getRotationNative();

}
