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

import android.graphics.PointF;
import android.graphics.Rect;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.util.Log;

import com.nokia.heif.Exception;
import com.nokia.heif.HEIF;
import com.nokia.heif.ImageItem;
import com.nokia.heif.Item;
import com.nokia.heif.VideoTrack;

import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MiafRenderer implements GLSurfaceView.Renderer
{
    private static final String TAG = "MiafRenderer";
    private GLResources mResources;
    private RenderTarget mRenderTarget;
    private Rect mRenderRect;
    private DrawableSurface mDrawableSurface;
    private ImageItem mImage;
    private ImageItem mCurImage = null;
    private boolean mBusy = false;
    private MiafTextureView mTextureView = null;

    private VideoTrack mTrack;
    private VideoTrack mCurTrack = null;

    public MiafRenderer(MiafTextureView aView, GLResources aResources)
    {
        mTextureView = aView;
        mResources = aResources;
        mRenderTarget = new RenderTarget(true);
        mRenderRect = new Rect(0, 0, 0, 0);
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config)
    {
        // Need to create all shaders, decoders, texture, etc.
        mResources.reserve();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height)
    {
        mRenderTarget.width = width;
        mRenderTarget.height = height;
    }

    Object mLock = new Object();

    boolean setImage(ImageItem aImage)
    {
        synchronized (mLock)
        {
            if (!mBusy)
            {
                mBusy = true;
                mImage = aImage;
                return true;
            }
        }
        return false;
    }

    boolean setTrack(VideoTrack aTrack)
    {
        synchronized (mLock)
        {
            if (!mBusy)
            {
                mBusy = true;
                mTrack = aTrack;
                return true;
            }
        }
        return false;
    }

    public void zoom(PointF zoomCenter, float zoomFactor)
    {
        if (mDrawableSurface != null)
        {
            mDrawableSurface.zoom(zoomCenter, zoomFactor);
        }
    }

    private void createDrawableSurface(GLResources aResources)
    {
        try
        {
            if (mImage != null)
            {
                DrawableSurface surface = (DrawableSurface) mImage.getUserData();

                if (surface == null)
                {
                    surface = DrawableSurfaceFactory.createSurface(mImage, aResources);
                    mImage.setUserData(surface);
                }

                mDrawableSurface = surface;
            }
            else if (mTrack != null)
            {
                DrawableSurface surface = (DrawableSurface) mTrack.getUserData();

                if (surface == null)
                {
                    surface = new CodedImageSurface(mTrack, aResources);
                    mTrack.setUserData(surface);
                }

                mDrawableSurface = surface;
            }

            // render 1:1 scale. (clipping as needed)
            if (mImage != null)
            {
                mRenderRect = new Rect(0, 0, mImage.getSize().width, mImage.getSize().height);
            }
            else
            {
                mRenderRect = new Rect(0, 0, mTrack.getDisplaySize().width, mTrack.getDisplaySize().height);
            }
        }
        catch (Exception e)
        {
            Log.e(TAG, "Drawable surface creation failed");
            mImage = null;
        }
    }

    public void dispose()
    {
        HEIF h = null;
        if (mImage != null) h = mImage.getParentHEIF();
        else if (mTrack != null) h = mTrack.getParentHEIF();

        try
        {
            List<ImageItem> images = h.getImages();
            for (int i = 0; i < images.size(); i++)
            {
                DrawableSurface surf = (DrawableSurface) images.get(i).getUserData();
                if (surf != null)
                {
                    surf.dispose();
                    surf = null;
                    images.get(i).setUserData(null);
                }
            }
            List<Item> items = h.getItems();
            for (int i = 0; i < items.size(); i++)
            {
                DrawableSurface surf = (DrawableSurface) items.get(i).getUserData();
                if (surf != null)
                {
                    surf.dispose();
                    surf = null;
                    items.get(i).setUserData(null);
                }
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    @Override
    public void onDrawFrame(GL10 gl)
    {
        synchronized (mLock)
        {
            GLES20.glDisable(GLES20.GL_SCISSOR_TEST);
            GLES20.glClearColor(0.f, 0.f, 0.f, 0.f);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
            if (mImage != null && mImage != mCurImage)
            {
                createDrawableSurface(mResources);
                mCurImage = mImage;
            }

            if (mTrack != null && mTrack != mCurTrack)
            {
                createDrawableSurface(mResources);
                mCurTrack = mTrack;
            }

            mResources.predraw();
            if (mDrawableSurface != null)
            {
                mDrawableSurface.preDraw(); // update textures if needed..
                if (mDrawableSurface.readyToDraw())
                {
                    // direct to screen
                    mRenderTarget.fboId = 0;
                    mRenderTarget.textureId = 0;
                    float sx = (float) mRenderTarget.width / mDrawableSurface.mRenderWidth;
                    float sy = (float) mRenderTarget.height / mDrawableSurface.mRenderHeight;

                    float s = sy;
                    if (sx < sy)
                    {
                        s = sx;
                    }
                    float scaledWidth = mDrawableSurface.mRenderWidth * s;
                    float scaledHeight = mDrawableSurface.mRenderHeight * s;
                    float centerPointX = (mRenderTarget.width - scaledWidth) / 2;
                    float centerPointY = (mRenderTarget.height - scaledHeight) / 2;

                    mRenderRect = new Rect((int) centerPointX, (int) centerPointY, (int) (centerPointX + scaledWidth), (int) (centerPointY + scaledHeight));
                    mDrawableSurface.draw(mRenderTarget, mRenderRect);
                    mBusy = false;
                }
                else
                {
                    // try again
                    if (mTextureView != null)
                    {
                        mTextureView.requestRender();
                    }
                }
            }
        }
    }

    public void showFrame(int frameIndex)
    {
        if (mDrawableSurface != null)
        {
            ((CodedImageSurface) mDrawableSurface).decodeFrame(frameIndex);
        }
        if (mTextureView != null)
        {
            mTextureView.requestRender();
        }
    }
}
