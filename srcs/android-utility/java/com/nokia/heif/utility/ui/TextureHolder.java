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

import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.util.Log;
import android.view.Surface;

import com.nokia.heif.CodedImageItem;
import com.nokia.heif.DecoderConfig;
import com.nokia.heif.Exception;
import com.nokia.heif.Sample;
import com.nokia.heif.VideoSample;
import com.nokia.heif.VideoTrack;
import com.nokia.heif.utility.codec.Decoder;

import java.nio.ByteBuffer;
import java.util.LinkedList;
import java.util.List;

public class TextureHolder implements GLResource, Decoder.DecodeClient
{
    private static final String TAG = "TextureHolder";

    public float[] transform = new float[4 * 4];
    public boolean decoded, available, updated;
    private int mTextureId;

    private CodedImageItem mImage = null;
    private Surface mSurface;
    private SurfaceTexture mSurfaceTexture;
    private GLResources mResources = null;

    // These are used only with video tracks
    private VideoTrack mVideoTrack = null;
    private Decoder mDecoder = null;
    private List<VideoSample> mSamples = null;

    public TextureHolder(CodedImageItem image, GLResources aResources)
    {
        Log.d(TAG, "TextureHolder for CodedImageItem created");
        mResources = aResources;
        mImage = image;
    }

    public TextureHolder(VideoTrack track, GLResources aResources)
            throws Exception
    {
        Log.d(TAG, "TextureHolder for VideoTrack created");
        mResources = aResources;
        mVideoTrack = track;

        mSamples = mVideoTrack.getVideoSamples();
    }

    @Override
    public Surface getOutputSurface()
    {
        Log.d(TAG, "getOutputSurface()");
        return mSurface;
    }

    @Override
    public void setDefaultSize(int width, int height)
    {
        mSurfaceTexture.setDefaultBufferSize(width, height);
    }

    @Override
    public ByteBuffer getItemData()
            throws Exception
    {
        if (mImage != null)
        {
            return mImage.getItemData();
        }
        return null;
    }

    @Override
    public void notifyDecodeReady()
    {
        updated = false;
        decoded = true;
    }

    @Override
    public int getTextureId()
    {
        return mTextureId;
    }

    public void setAvailable(boolean status)
    {
        available = status;
    }

    public void reserve()
    {
        Log.d(TAG, "reserve()");
        if (mTextureId != 0)
        {
            Log.d(TAG, "mTextureId != 0 => release(false);");
            release(false);
        }
        try
        {
            int[] textureIds = new int[1];
            GLES20.glGenTextures(1, textureIds, 0);
            mTextureId = textureIds[0];
            // NOTE: we need to use GL_TEXTURE_EXTERNAL_OES since thats what SurfaceTexture/Surface/VideoDecoder uses.
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mTextureId);
            GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

            available = false;
            decoded = false;
            updated = false;
            mSurfaceTexture = new SurfaceTexture(mTextureId);
            mSurface = new Surface(mSurfaceTexture);

            mSurfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener()
            {
                @Override
                public void onFrameAvailable(SurfaceTexture surfaceTexture)
                {
                    notifyDecodeReady();
                    available = true;
                }
            });

            if (mImage != null)
            {
                mDecoder = mResources.getCodecUtility().getDecoder(mImage);
                mDecoder.queueDecode(this);
            }
            else
            {
                mDecoder = mResources.getCodecUtility().getDecoder(mVideoTrack);
                mDecoder.queueDecode(this, mSamples.get(0), true);
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
            Log.e(TAG, "Error during reserve()");
        }
    }

    /**
     * Return full decoding dependencies of a sample, including the sample itself.
     */
    private List<Sample> getFramesToDecode(VideoSample sample)
    {
        assert (mVideoTrack != null);
        List<Sample> fullDependencies = new LinkedList<>();
        try
        {
            List<Sample> dependencies = sample.getDecodeDependencies();
            if (dependencies.size() == 0) {
                dependencies.add(sample);
                return dependencies;
            }

            for (Sample dependency : dependencies)
            {
                fullDependencies.addAll(getFramesToDecode((VideoSample)dependency));
            }
        }
        catch (Exception e)
        {
            Log.e(TAG, "getFramesToDecode() failed.");
            return null;
        }
        fullDependencies.add(sample);

        return fullDependencies;
    }

    public void decodeFrame(int frameIndex)
    {
        assert (mVideoTrack != null);

        final List<Sample> decodeQueue = getFramesToDecode(mSamples.get(frameIndex));
        for (int i = 0; i < decodeQueue.size(); i++)
        {
            mDecoder.queueDecode(this, decodeQueue.get(i), i == decodeQueue.size() - 1);
        }
    }

    public void release(boolean aLostContext)
    {
        Log.d(TAG, "release(" + aLostContext + ")");
        updated = false;
        decoded = false;
        available = false;
        if (mSurfaceTexture != null)
        {
            mSurfaceTexture.release();
            mSurfaceTexture = null;
        }
        if (mSurface != null)
        {
            mSurface.release();
            mSurface = null;
        }
        if (!aLostContext)
        {
            if (mTextureId != 0)
            {
                int[] textureIds = new int[1];
                textureIds[0] = mTextureId;
                GLES20.glDeleteTextures(1, textureIds, 0);
            }
        }
        mTextureId = 0;
    }

    public void updateTexture()
    {
        Log.d(TAG, "updateTexture()");
        if (!updated)
        {
            mSurfaceTexture.updateTexImage();
            mSurfaceTexture.getTransformMatrix(transform);
            updated = true;
        }
    }
}
