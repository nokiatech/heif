/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
 * Operating-Points-Info property ('oinf') -- multi-layer operating
 * point table (ISO/IEC 23008-12).  Enumerates the output layer sets,
 * profile-tier-level entries, and layer dependency information for a
 * layered (e.g. L-HEVC) image item.
 */
public final class OperatingPointsProperty extends DescriptiveProperty
{

    /**
     * Creates an OperatingPointsProperty for the given HEIF instance.
     * @param heif The parent HEIF instance for the new object
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public OperatingPointsProperty(HEIF heif)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
    }

    /**
     * Protected constructor, used to create an object from the native side.
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected OperatingPointsProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the scalability mask from the oinf property.
     * @return The 16-bit scalability_mask value
     * @throws Exception
     */
    public int getScalabilityMask()
            throws Exception
    {
        checkState();
        return getScalabilityMaskNative();
    }

    /**
     * Returns the number of operating points.
     * @return Operating point count
     * @throws Exception
     */
    public int getOperatingPointCount()
            throws Exception
    {
        checkState();
        return getOperatingPointCountNative();
    }

    /**
     * Returns the number of profile-tier-level entries.
     * @return PTL entry count
     * @throws Exception
     */
    public int getProfileTierLevelCount()
            throws Exception
    {
        checkState();
        return getProfileTierLevelCountNative();
    }

    /**
     * Returns the number of layer-info entries.
     * @return Layer count
     * @throws Exception
     */
    public int getLayerCount()
            throws Exception
    {
        checkState();
        return getLayerCountNative();
    }

    private native long createContextNative(HEIF heif);

    private native int getScalabilityMaskNative();

    private native int getOperatingPointCountNative();

    private native int getProfileTierLevelCountNative();

    private native int getLayerCountNative();
}
