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

import android.opengl.GLES20;
import android.util.Log;


public class Shader implements GLResource
{
    int mShader;
    int POS_ATTRIB, TEX_ATTRIB;
    int TRANSFORM_UNIFORM;
    int COLOR_TYPE, ALPHA_TYPE;
    private String mVertex, mFragment;

    Shader(String vertex, String fragment)
    {
        mVertex = vertex;
        mFragment = fragment;
    }

    public void reserve()
    {
        int COLOR_UNIFORM, ALPHA_UNIFORM, OVERLAY_UNIFORM;
        if (mShader != 0)
        {
            release(false);
        }
        int vs = compileShader(mVertex, GLES20.GL_VERTEX_SHADER);
        int fs = compileShader(mFragment, GLES20.GL_FRAGMENT_SHADER); // color only
        mShader = linkProgram(vs, fs);
        GLES20.glDeleteShader(fs);
        GLES20.glDeleteShader(vs);
        POS_ATTRIB = GLES20.glGetAttribLocation(mShader, "pos");
        TEX_ATTRIB = GLES20.glGetAttribLocation(mShader, "tex");
        COLOR_UNIFORM = GLES20.glGetUniformLocation(mShader, "u_texture");
        ALPHA_UNIFORM = GLES20.glGetUniformLocation(mShader, "u_texture_A");
        OVERLAY_UNIFORM = GLES20.glGetUniformLocation(mShader, "u_texture_O");
        TRANSFORM_UNIFORM = GLES20.glGetUniformLocation(mShader, "u_matrix");
        GLES20.glUseProgram(mShader);
        if (COLOR_UNIFORM > -1) GLES20.glUniform1i(COLOR_UNIFORM, 0); // Bind color to texture unit 0
        if (ALPHA_UNIFORM > -1) GLES20.glUniform1i(ALPHA_UNIFORM, 1); // Bind alpha to texture unit 1
        if (OVERLAY_UNIFORM > -1)
            GLES20.glUniform1i(OVERLAY_UNIFORM, 2); // Bind overlay to texture unit 2
        GLES20.glUseProgram(0);
        // TODO: fetch sampler type with glGetActiveUniform?
    }

    public void release(boolean aLostContext)
    {
        if (!aLostContext)
        {
            if (mShader != 0)
            {
                GLES20.glDeleteProgram(mShader);
            }
        }
        mShader = 0;
    }

    private int compileShader(String shader, int shaderType)
    {
        final String TAG = "ShaderCompile";
        int id = GLES20.glCreateShader(shaderType);
        GLES20.glShaderSource(id, shader);
        GLES20.glCompileShader(id);
        int[] status = new int[1];
        GLES20.glGetShaderiv(id, GLES20.GL_COMPILE_STATUS, status, 0);
        if (status[0] == 0)
        {
            Log.e(TAG, "Could not compile shader " + shaderType + ":");
            Log.e(TAG, GLES20.glGetShaderInfoLog(id));
            GLES20.glDeleteShader(id);
            id = 0;
        }
        return id;
    }

    private int linkProgram(int vs, int fs)
    {
        final String TAG = "ShaderLink";
        int id = GLES20.glCreateProgram();
        GLES20.glAttachShader(id, vs);
        GLES20.glAttachShader(id, fs);
        GLES20.glLinkProgram(id);
        int[] status = new int[1];
        GLES20.glGetProgramiv(id, GLES20.GL_LINK_STATUS, status, 0);
        if (status[0] == 0)
        {
            Log.e(TAG, "Could not link program:");
            Log.e(TAG, GLES20.glGetProgramInfoLog(id));
            GLES20.glDeleteProgram(id);
            id = 0;
        }
        return id;
    }
}
