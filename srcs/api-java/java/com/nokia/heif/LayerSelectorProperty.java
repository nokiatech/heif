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
 * Layer selector property ('lsel') -- selects a single layer of a
 * multi-layer (e.g. L-HEVC) image item.  Single 16-bit layer_id
 * payload (ISO/IEC 23008-12).
 */
public final class LayerSelectorProperty extends DescriptiveProperty
{

    /**
     * Creates a LayerSelectorProperty for the given HEIF instance.
     * @param heif    The parent HEIF instance for the new object
     * @param layerId The layer identifier to select
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public LayerSelectorProperty(HEIF heif, int layerId)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        setLayerId(layerId);
    }

    /**
     * Protected constructor, used to create an object from the native side.
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected LayerSelectorProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the selected layer identifier.
     * @return The 16-bit layer_id value
     * @throws Exception
     */
    public int getLayerId()
            throws Exception
    {
        checkState();
        return getLayerIdNative();
    }

    /**
     * Sets the layer identifier.
     * @param layerId The 16-bit layer_id value
     * @throws Exception
     */
    public void setLayerId(int layerId)
            throws Exception
    {
        checkState();
        setLayerIdNative(layerId);
    }

    private native long createContextNative(HEIF heif);

    private native int getLayerIdNative();

    private native void setLayerIdNative(int layerId);
}
