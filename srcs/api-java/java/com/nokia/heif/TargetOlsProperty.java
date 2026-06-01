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
 * Target Output Layer Set property ('tols') -- selects which Output
 * Layer Set enumerated by an associated 'oinf' table the multi-layer
 * image item targets.  Single 16-bit target_ols_index payload
 * (ISO/IEC 23008-12).
 */
public final class TargetOlsProperty extends DescriptiveProperty
{

    /**
     * Creates a TargetOlsProperty for the given HEIF instance.
     * @param heif           The parent HEIF instance for the new object
     * @param targetOlsIndex The OLS index to target
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public TargetOlsProperty(HEIF heif, int targetOlsIndex)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        setTargetOlsIndex(targetOlsIndex);
    }

    /**
     * Protected constructor, used to create an object from the native side.
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected TargetOlsProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the target Output Layer Set index.
     * @return The 16-bit target_ols_index value
     * @throws Exception
     */
    public int getTargetOlsIndex()
            throws Exception
    {
        checkState();
        return getTargetOlsIndexNative();
    }

    /**
     * Sets the target Output Layer Set index.
     * @param targetOlsIndex The 16-bit target_ols_index value
     * @throws Exception
     */
    public void setTargetOlsIndex(int targetOlsIndex)
            throws Exception
    {
        checkState();
        setTargetOlsIndexNative(targetOlsIndex);
    }

    private native long createContextNative(HEIF heif);

    private native int getTargetOlsIndexNative();

    private native void setTargetOlsIndexNative(int targetOlsIndex);
}
