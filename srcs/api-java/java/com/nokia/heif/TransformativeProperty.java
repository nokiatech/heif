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
 * Abstract parent class for TransformativeProperties
 */
public abstract class TransformativeProperty extends ItemProperty
{

    /**
     * Constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @exception Exception Thrown if the parent HEIF instance is invalid
     */
    protected TransformativeProperty(HEIF heif)
            throws Exception
    {
        super(heif);
    }

    /**
     * Protected constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected TransformativeProperty(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }
}
