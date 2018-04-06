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
 * ColourInformationProperty
 */
public abstract class ColourInformationProperty extends DescriptiveProperty
{

    public final static FourCC NCLX_PROFILE = new FourCC("nclx", true);
    public final static FourCC ICC_RESTRICTED_PROFILE = new FourCC("rICC", true);
    public final static FourCC ICC_UNRESTRICTED_PROFILE = new FourCC("prof", true);

    /**
     * Constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @exception Exception Thrown if the parent HEIF instance is invalid
     */
    protected ColourInformationProperty(HEIF heif)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
    }


    /**
     * Protected constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected ColourInformationProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Gets the colour type
     * @return Colour type as a String
     * @throws Exception
     */
    public FourCC getColourType()
            throws Exception
    {
        checkState();
        return new FourCC(getColourTypeNative(), true);
    }

    private native long createContextNative(HEIF heif);

    protected native String getColourTypeNative();

    protected native void setColourTypeNative(String type);

    protected native int getColourPrimariesNative();

    protected native void setColourPrimariesNative(int colourPrimaries);

    protected native int getTransferCharacteristicsNative();

    protected native void setTransferCharacteristicsNative(int characteristics);

    protected native int getMatrixCoefficientsNative();

    protected native void setMatrixCoefficientsNative(int matrixCoefficients);

    protected native boolean getFullRangeFlagNative();

    protected native void setFullRangeFlagNative(boolean flag);

    protected native ByteBuffer getICCProfileNative();

    protected native void setICCProfileNative(byte[] profile);
}
