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

public final class NCLXColourProperty extends ColourInformationProperty
{

    /**
     * Creates an NCLXColourProperty to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public NCLXColourProperty(HEIF heif)
            throws Exception
    {
        super(heif);
        setColourTypeNative(ColourInformationProperty.NCLX_PROFILE.toString());
    }

    /**
     * Protected constructor, used to create an object from the native side
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected NCLXColourProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Gets the colour primaries
     * @return Colour primaries as an integer
     * @throws Exception
     */
    public int getColourPrimaries()
            throws Exception
    {
        checkState();
        return getColourPrimariesNative();
    }

    /**
     * Sets the colour primaries
     * @param primaries Colour primaries as an integer
     * @throws Exception
     */
    public void setColourPrimaries(int primaries)
            throws Exception
    {
        checkState();
        setColourPrimariesNative(primaries);
    }

    /**
     * Gets the transfer characteristics
     * @return Transfer characteristics as an integer
     * @throws Exception
     */
    public int getTransferCharacteristics()
            throws Exception
    {
        checkState();
        return getTransferCharacteristicsNative();
    }

    /**
     * Sets the transfer characteristics
     * @param characteristics Transfer characteristics as an integer
     * @throws Exception
     */
    public void setTransferCharacteristics(int characteristics)
            throws Exception
    {
        checkState();
        setTransferCharacteristicsNative(characteristics);
    }

    /**
     * Get the matrix coefficients
     * @return Matrix coefficients as an integer
     * @throws Exception
     */
    public int getMatrixCoefficients()
            throws Exception
    {
        checkState();
        return getMatrixCoefficientsNative();
    }

    /**
     * Sets the matrix coefficients
     * @param matrixCoefficients Matrix coefficients as an integer
     * @throws Exception
     */
    public void setMatrixCoefficients(int matrixCoefficients)
            throws Exception
    {
        checkState();
        setMatrixCoefficientsNative(matrixCoefficients);
    }

    /**
     * Gets the full range flag
     * @return Full range flag as a boolean
     * @throws Exception
     */
    public boolean getFullRangeFlag()
            throws Exception
    {
        checkState();
        return getFullRangeFlagNative();
    }

    /**
     * Sets the full range flag
     * @param flag Full range flag as a boolean
     * @throws Exception
     */
    public void setFullRangeFlag(boolean flag)
            throws Exception
    {
        checkState();
        setFullRangeFlagNative(flag);
    }

}
