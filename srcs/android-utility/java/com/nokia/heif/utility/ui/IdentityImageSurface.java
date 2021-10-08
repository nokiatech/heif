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

import android.graphics.Rect;

import com.nokia.heif.Exception;
import com.nokia.heif.IdentityImageItem;

public final class IdentityImageSurface extends DrawableSurface
{
    public IdentityImageSurface(IdentityImageItem image, GLResources aResources)
            throws Exception
    {
        super(image, aResources);
        refreshContent();
    }

    @Override
    public void refreshContent()
            throws Exception
    {
        mChildren.clear();
        super.refreshContent();

        Rect targetRect = new Rect(0, 0, mRenderTarget.width, mRenderTarget.height);
        IdentityImageItem image = (IdentityImageItem) mImageItem;
        mChildren.add(new DrawableChild(DrawableSurfaceFactory.createSurface(image.getImage(), mResources), targetRect));

        // Refresh all child content.
        for (DrawableChild child : mChildren)
        {
            child.surface.refreshContent();
        }
    }

    @Override
    public void draw(RenderTarget renderTarget, Rect rect)
    {
        super.draw(mRenderTarget, new Rect(0, 0, mRenderTarget.width, mRenderTarget.height));

        // Draw "self" to given target.
        DrawQuad(renderTarget, rect, mRenderTarget.textureId);
    }

}
