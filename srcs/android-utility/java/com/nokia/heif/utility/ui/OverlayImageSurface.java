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
import android.opengl.GLES20;

import com.nokia.heif.Exception;
import com.nokia.heif.OverlayImageItem;

import java.util.List;

public class OverlayImageSurface extends DrawableSurface
{
    private float mR, mG, mB, mA;

    public OverlayImageSurface(OverlayImageItem image, GLResources aResources)
            throws Exception
    {
        super(image, aResources);
        refreshContent();

    }

    public void refreshContent()
            throws Exception
    {
        mChildren.clear();
        super.refreshContent();

        OverlayImageItem image = (OverlayImageItem) mImageItem;
        List<OverlayImageItem.OverlayedImage> overlaidImages = image.getOverlayedImages();
        for (OverlayImageItem.OverlayedImage overlaidImage : overlaidImages)
        {
            Rect targetRect = createRectForChild(overlaidImage);
            mChildren.add(new DrawableChild(DrawableSurfaceFactory.createSurface(overlaidImage.image, mResources), targetRect));
        }
        mR = image.getBackgroundColour().r / 65535.0f;
        mG = image.getBackgroundColour().g / 65535.0f;
        mB = image.getBackgroundColour().b / 65535.0f;
        mA = image.getBackgroundColour().a / 65535.0f;
        // Refresh all child content.
        for (DrawableChild child : mChildren)
        {
            child.surface.refreshContent();
        }

    }

    private Rect createRectForChild(OverlayImageItem.OverlayedImage image)
            throws Exception
    {
        Rect targetRect = new Rect();

        targetRect.top = image.verticalOffset;
        targetRect.bottom = image.verticalOffset + image.image.getSize().height;
        targetRect.left = image.horizontalOffset;
        targetRect.right = image.horizontalOffset + image.image.getSize().width;
        return targetRect;
    }

    @Override
    public void draw(RenderTarget renderTarget, Rect rect)
    {
        // Draw self.
        // First clear it to specified color.
        mRenderTarget.bind();
        GLES20.glClearColor(mR, mG, mB, mA);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        mRenderTarget.unbind();
        // Now render all the layers.
        super.draw(mRenderTarget, new Rect(0, 0, mRenderTarget.width, mRenderTarget.height));
        // Draw "self" to given target.
        DrawQuad(renderTarget, rect, mRenderTarget.textureId);
    }
}
