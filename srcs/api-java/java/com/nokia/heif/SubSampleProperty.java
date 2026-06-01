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
 * Sub-sample information property ('subs') -- describes byte-range
 * sub-divisions within a coded image item's data, mapping each
 * sub-sample to a priority and codec-specific role (ISO/IEC 14496-12 Section 8.7.7).
 * Zero or more instances may be associated with an HEVC or AVC image item.
 */
public final class SubSampleProperty extends DescriptiveProperty
{

    /**
     * Creates a SubSampleProperty for the given HEIF instance.
     * @param heif The parent HEIF instance for the new object
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public SubSampleProperty(HEIF heif)
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
    protected SubSampleProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the number of sub-sample entries.
     * @return The sub-sample count
     * @throws Exception
     */
    public int getSubSampleCount()
            throws Exception
    {
        checkState();
        return getSubSampleCountNative();
    }

    private native long createContextNative(HEIF heif);

    private native int getSubSampleCountNative();
}
