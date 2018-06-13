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
 * Defines the size of an image
 */
public final class Size
{
    /** Width in pixels */
    public final int width;

    /** Height in pixels */
    public final int height;

    public Size(int width, int height)
    {
        this.width = width;
        this.height = height;
    }

    @Override
    public boolean equals(Object obj)
    {
        if (obj instanceof Size)
        {
            Size comparing = (Size)obj;
            return (this.width == comparing.width) && (this.height == comparing.height);
        }
        else
        {
            return false;
        }
    }
}
