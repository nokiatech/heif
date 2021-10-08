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
import com.nokia.heif.GridImageItem;
import com.nokia.heif.ImageItem;

import java.util.ArrayList;
import java.util.List;

public class GridImageSurface extends DrawableSurface
{
    private int mRowCount;
    private int mColumnCount;

    public GridImageSurface(GridImageItem gridImage, GLResources aResources)
            throws Exception
    {
        super(gridImage, aResources);
        refreshContent();
    }

    public void refreshContent()
            throws Exception
    {
        mChildren.clear();
        GridImageItem grid = (GridImageItem) mImageItem;
        mRowCount = grid.getRowCount();
        mColumnCount = grid.getColumnCount();
        mChildren = new ArrayList<>(mRowCount * mColumnCount);
        List<ImageItem> sourceImages = grid.getSourceImages();
        super.refreshContent();

        for (int index = 0; index < sourceImages.size(); index++)
        {
            mChildren.add(new DrawableChild(DrawableSurfaceFactory.createSurface(sourceImages.get(index), mResources),
                    createRectForChild(index)));
        }

        // Refresh all child content.
        for (DrawableChild child : mChildren)
        {
            child.surface.refreshContent();
        }
    }

    private Rect createRectForChild(int listIndex)
    {
        int rowIndex = listIndex / mColumnCount;
        int columnIndex = listIndex % mColumnCount;
        int cellWidth = mRenderTarget.width / mColumnCount;
        int cellHeight = mRenderTarget.height / mRowCount;
        Rect targetRect = new Rect();
        targetRect.top = cellHeight * rowIndex;
        targetRect.bottom = cellHeight * rowIndex + cellHeight;
        targetRect.left = cellWidth * columnIndex;
        targetRect.right = cellWidth * columnIndex + cellWidth;
        return targetRect;
    }

    @Override
    public void draw(RenderTarget renderTarget, Rect rect)
    {
        // Draw self.
        super.draw(mRenderTarget, new Rect(0, 0, mRenderTarget.width, mRenderTarget.height));
        // Draw "self" to given target.
        DrawQuad(renderTarget, rect, mRenderTarget.textureId);
    }

}
