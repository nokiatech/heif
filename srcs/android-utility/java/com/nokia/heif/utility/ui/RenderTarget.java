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

import android.opengl.GLES20;
import android.util.Log;

import static android.opengl.GLES20.GL_FRAMEBUFFER_COMPLETE;

public class RenderTarget implements GLResource
{
    public int textureId;
    public int fboId;
    public int width;
    public int height;
    private int surfaceWidth, surfaceHeight;
    private boolean mDefaultFBO;

    public RenderTarget(boolean w)
    {
        mDefaultFBO = true;
    }

    public RenderTarget(int w, int h)
    {
        mDefaultFBO = false;
        width = w;
        height = h;
    }

    public void reserve()
    {
        if ((textureId != 0) || (fboId != 0))
        {
            release(false);
        }
        if (mDefaultFBO)
        {
            textureId = 0;
            fboId = 0;
            surfaceWidth = width;
            surfaceHeight = height;

        }
        else
        {
            int[] id = new int[1];
            GLES20.glGenTextures(1, id, 0);
            textureId = id[0];
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textureId);

            float[] color = {0.0f, 0.0f, 0.0f, 0.0f};
            /*GLES20.glTexParameterfv(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_BORDER_COLOR, color);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_BORDER);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_BORDER);*/
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA/*.GL_RGBA8*/, width, height, 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, null);
            surfaceWidth = width;
            surfaceHeight = height;

            GLES20.glGenFramebuffers(1, id, 0);
            fboId = id[0];
            GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, fboId);
            GLES20.glFramebufferTexture2D(GLES20.GL_FRAMEBUFFER, GLES20.GL_COLOR_ATTACHMENT0, GLES20.GL_TEXTURE_2D, textureId, 0);


            int status = GLES20.glCheckFramebufferStatus(GLES20.GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE)
            {
                Log.e("RenderTarget", "glCheckFramebufferStatus failed.");
                return;
            }

            float r = (float) Math.random();
            float g = (float) Math.random();
            float b = (float) Math.random();
            GLES20.glClearColor(r, g, b, 1.f);
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT);
            GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
        }

    }

    public void release(boolean aLostContext)
    {
        if (!aLostContext)
        {
            int[] id = new int[1];
            if (textureId != 0)
            {
                id[0] = textureId;
                GLES20.glDeleteTextures(1, id, 0);
            }
            if (fboId != 0)
            {
                id[0] = fboId;
                GLES20.glDeleteFramebuffers(1, id, 0);
            }
        }
        textureId = 0;
        fboId = 0;

    }

    public void resize(float w, float h)
    {
        width = (int) Math.ceil(w);
        height = (int) Math.ceil(h);
    }


    public void bind()
    {
        if ((surfaceWidth != width) || (surfaceHeight != height))
        {
            surfaceWidth = width;
            surfaceHeight = height;
        }
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, fboId);
        GLES20.glScissor(0, 0, width, height);
        GLES20.glViewport(0, 0, width, height);
    }

    public void unbind()
    {
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
    }
}
