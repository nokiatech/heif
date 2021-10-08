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

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

class VertexBuffer implements GLResource
{
    int mVb;

    VertexBuffer()
    {
    }

    public void reserve()
    {
        if (mVb != 0)
        {
            release(false);
        }
        int[] id = new int[1];
        GLES20.glGenBuffers(1, id, 0);
        mVb = id[0];
    }

    public void release(boolean aLostContext)
    {
        if (mVb != 0)
        {
            if (!aLostContext)
            {
                int[] id = new int[1];
                id[0] = mVb;
                GLES20.glDeleteBuffers(1, id, 0);
            }
        }
        mVb = 0;
    }

    public void update(float[] data)
    {
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, mVb);
        ByteBuffer bb = ByteBuffer.allocateDirect(data.length * 4);
        bb.order(ByteOrder.nativeOrder());
        FloatBuffer vertexBuffer = bb.asFloatBuffer();
        vertexBuffer.put(data);
        vertexBuffer.position(0);
        GLES20.glBufferData(GLES20.GL_ARRAY_BUFFER, bb.limit(), bb, GLES20.GL_STATIC_DRAW);
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, 0);
    }
}
