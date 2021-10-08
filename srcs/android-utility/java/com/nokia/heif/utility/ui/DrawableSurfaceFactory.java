/*
 * This file is part of Nokia HEIF applications
 *
 * Copyright (c) 2018-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be disclosed to others without the prior written consent of Nokia.
 *
 */

package com.nokia.heif.utility.ui;

import com.nokia.heif.CodedImageItem;
import com.nokia.heif.Exception;
import com.nokia.heif.GridImageItem;
import com.nokia.heif.IdentityImageItem;
import com.nokia.heif.ImageItem;
import com.nokia.heif.OverlayImageItem;

public class DrawableSurfaceFactory
{

    static public DrawableSurface createSurface(ImageItem image, GLResources aResources)
            throws Exception
    {
        if (image instanceof GridImageItem)
        {
            return new GridImageSurface((GridImageItem) image, aResources);
        }
        else if (image instanceof CodedImageItem)
        {
            return new CodedImageSurface((CodedImageItem) image, aResources);
        }
        else if (image instanceof OverlayImageItem)
        {
            return new OverlayImageSurface((OverlayImageItem) image, aResources);
        }
        else if (image instanceof IdentityImageItem)
        {
            return new IdentityImageSurface((IdentityImageItem) image, aResources);
        }
        else
        {
            assert (false);
            return null;
        }
    }

}
