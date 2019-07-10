/*
 * This file is part of Nokia HEIF applications
 *
 * Copyright (c) 2018-2019 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

import com.nokia.heif.CodedImageItem;
import com.nokia.heif.Exception;
import com.nokia.heif.VideoTrack;

public class CodedImageSurface extends DrawableSurface
{
    TextureHolder mTextureHolder;

    public CodedImageSurface(CodedImageItem image, GLResources aResources)
            throws Exception
    {
        super(image, aResources);
        mTextureHolder = (TextureHolder) mResources.getResource(mImageItem.toString() + "_Texture");

        if (mTextureHolder == null)
        {
            mTextureHolder = new TextureHolder(image, aResources);
            mResources.addResource(mImageItem.toString() + "_Texture", mTextureHolder);
        }
        else
        {
            mTextureMatrix = mTextureHolder.transform;
        }
        refreshContent();
    }


    public CodedImageSurface(VideoTrack track, GLResources aResources)
            throws Exception
    {
        super(track, aResources);
        mTextureHolder = (TextureHolder) mResources.getResource(mVideoTrack.toString() + "_Texture");

        if (mTextureHolder == null)
        {
            mTextureHolder = new TextureHolder(track, aResources);
            mResources.addResource(mVideoTrack.toString() + "_Texture", mTextureHolder);
        }
        else
        {
            mTextureMatrix = mTextureHolder.transform;
        }
        refreshContent();
    }

    public void dispose()
    {
        if (mTextureHolder != null)
        {
            mResources.removeResource(mImageItem.toString() + "_Texture");
            mTextureHolder = null;
        }
        super.dispose();
    }

    @Override
    public void refreshContent()
            throws Exception
    {
        super.refreshContent();
    }

    @Override
    public void preDraw()
    {
        super.preDraw();
        if (mTextureHolder.decoded)
        {
            if (mTextureHolder.available)
            {
                if (!mTextureHolder.updated)
                {
                    mTextureHolder.updateTexture();
                    mTextureMatrix = mTextureHolder.transform;
                }
            }
        }
    }

    @Override
    public void draw(RenderTarget renderTarget, Rect rect)
    {
        super.draw(mRenderTarget, new Rect(0, 0, mRenderTarget.width, mRenderTarget.height));
        DrawQuad(renderTarget, rect, mTextureHolder.getTextureId());
    }

    @Override
    public boolean readyToDraw()
    {
        if (super.readyToDraw())
        {
            return mTextureHolder.updated;
        }
        return false;
    }

    public void decodeFrame(int frameIndex)
    {
        mTextureHolder.decodeFrame(frameIndex);
    }
}
