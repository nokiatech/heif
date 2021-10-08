/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 *
 */

package com.nokia.heif;

/**
 * ImageSequence, a special case of a VideoTrack
 */
public final class ImageSequence extends VideoTrack
{
    /**
     * Creates a new ImageSequence to the given HEIF instance
     *
     * @param heif The parent HEIF instance for the new object
     * @param timescale The timescale of the track. How many timescale units are per second.
     * @throws Exception
     */
    public ImageSequence(HEIF heif, int timescale)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        try
        {
            setTimescale(timescale);
        }
        catch (Exception ex)
        {
            destroy();
            throw ex;
        }
    }

    /**
     * Protected constructor, used to create an object from the native side
     *
     * @param heif         The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected ImageSequence(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Get the auxiliary type identifier from an optional AuxiliaryTypeInfoBox.
     * @return URN used to identify the typo of the auxiliary images. Null if no AuxiliaryTypeInfoBox
     * was present in the sample entry.
     */
    public String getAuxTypeInfo()
    {
        return getAuxiliaryTypeInfoNative();
    }

    /**
     * Set auxiliary type identifier to an optional AuxiliaryTypeInfoBox in the sample entry.
     * @param aux UTF-8 character string of the URN used to identify the type of the auxiliary images
     *            Null if removing the AuxiliaryTypeInfoBox.
     * @throws Exception
     */
    public void setAux(String aux)
            throws Exception
    {
        checkState();
        setAuxNative(aux);
    }

    private native long createContextNative(HEIF heif);
    native private String getAuxiliaryTypeInfoNative();
    native private void setAuxNative(String auxType);
}
