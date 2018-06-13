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
 * Mirror property, defines if the ImageItem should be mirrored along a certain axis
 */
public final class MirrorProperty extends TransformativeProperty
{

    /**
     * Enum describing the possible mirroring options.
     */
    public enum Mirror
    {
        VERTICAL(false),
        HORIZONTAL(true);

        private boolean value;

        private Mirror(boolean value)
        {
            this.value = value;
        }

        protected boolean getValue()
        {
            return value;
        }
    }

    /**
     * Creates an MirrorProperty to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @param
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public MirrorProperty(HEIF heif, Mirror direction)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        setMirror(direction);
    }

    /**
     * Protected constructor, used to create an object from the native side
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected MirrorProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Sets a mirroring
     * @param mirror Along which axis the image should be mirrored
     */
    public void setMirror(Mirror mirror)
    {
        setMirrorNative(mirror.getValue());
    }

    /**
     * Gets the mirroring
     * @return Along which axis the image should be mirrored
     * @throws Exception
     */
    public Mirror getMirror()
            throws Exception
    {
        checkState();
        boolean mirror = getMirrorNative();
        if (mirror)
        {
            return Mirror.HORIZONTAL;
        }
        else
        {
            return Mirror.VERTICAL;
        }
    }

    private native long createContextNative(HEIF heif);

    private native boolean getMirrorNative();

    private native void setMirrorNative(boolean mirror);

}
