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
import android.opengl.GLES11Ext;
import android.opengl.GLES20;

import com.nokia.heif.AuxiliaryProperty;
import com.nokia.heif.CleanApertureProperty;
import com.nokia.heif.Exception;
import com.nokia.heif.ImageItem;
import com.nokia.heif.MirrorProperty;
import com.nokia.heif.RotateProperty;
import com.nokia.heif.TransformativeProperty;
import com.nokia.heif.VideoTrack;

import java.util.ArrayList;
import java.util.List;

public abstract class DrawableSurface
{
    private static final String TAG = "DrawableSurface";
    public float mRenderWidth, mRenderHeight;

    protected List<DrawableChild> mChildren = new ArrayList<>();
    protected RenderTarget mRenderTarget = null;
    protected float[] mTextureMatrix = new float[4 * 4];
    protected ImageItem mImageItem = null;
    protected VideoTrack mVideoTrack = null;
    protected GLResources mResources = null;

    private RenderTarget mAlphaRenderTarget = null;
    private ImageItem mAlphaImage = null;
    private DrawableSurface mAlphaDrawableSurface = null;
    private boolean mOesTexture;
    private VertexBuffer mVertexBuffer = null;

    protected Shader mColorOES = null;
    protected Shader mColor = null;
    protected Shader mColorAlphaOES = null;
    protected Shader mColorAlpha = null;

    private PointF mZoomCenter = new PointF(0.5f, 0.5f);
    private float mZoomFactor = 1.0f;

    public void zoom(PointF zoomCenter, float zoomFactor)
    {
        mZoomFactor = zoomFactor;
        mZoomCenter = zoomCenter;
    }

    public void dispose()
    {
        //final cleanup.
        releaseShaders();
        if (mVertexBuffer != null)
        {
            mResources.removeResource(toString() + "_VB");
            mVertexBuffer = null;
        }
        if (mRenderTarget != null)
        {
            mResources.removeResource(toString() + "_Color");
            mRenderTarget = null;
        }
        if (mAlphaRenderTarget != null)
        {
            mResources.removeResource(toString() + "_Alpha");
            mAlphaRenderTarget = null;
        }
        if (mAlphaDrawableSurface != null)
        {
            mAlphaDrawableSurface.dispose();
        }
        mAlphaDrawableSurface = null;
        for (DrawableChild child : mChildren)
        {
            child.surface.dispose();
        }
        mTextureMatrix = null;
        mChildren = null;
        mImageItem = null;
        mVideoTrack = null;
        mAlphaImage = null;
        mResources = null;
    }

    public void preDraw()
    {
        if ((mAlphaDrawableSurface != null) && (mAlphaRenderTarget != null))
        {
            mAlphaDrawableSurface.preDraw();
        }
        for (DrawableChild child : mChildren)
        {
            child.surface.preDraw();
        }
    }

    public void refreshContent()
            throws Exception
    {
        //refresh gl resources.
        refreshVertexBuffer();

        //Re-create render target.
        int width = 0;
        int height = 0;
        if (mImageItem != null)
        {
            width = mImageItem.getSize().width;
            height = mImageItem.getSize().height;
        }
        else if (mVideoTrack != null)
        {
            width = mVideoTrack.getDisplaySize().width;
            height = mVideoTrack.getDisplaySize().height;
        }
        if ((mRenderTarget == null) || ((width != mRenderTarget.width) || (height != mRenderTarget.height)))
        {
            if (mRenderTarget != null)
            {
                mResources.removeResource(toString() + "_Color");
                mRenderTarget = null;
            }
            mRenderTarget = new RenderTarget(width, height);
            mResources.addResource(toString() + "_Color", mRenderTarget);
        }

        initShaders();


        //Refresh all child content.
        for (DrawableChild child : mChildren)
        {
            child.surface.refreshContent();
        }

        //Create alpha texture/surface if needed
        if (mImageItem != null)
        {
            handleItemAlpha();
        }
    }

    private void handleItemAlpha() throws Exception
    {
        List<ImageItem> auxlist = mImageItem.getAuxiliaries();
        ImageItem alphaImage = null;
        for (int t = 0; t < auxlist.size(); t++)
        {
            ImageItem aux = auxlist.get(t);
            AuxiliaryProperty ap = aux.getAuxiliaryProperty();
            if (ap != null)
            {
                final String type = ap.getType();
                if ((type.equals("urn:mpeg:hevc:2015:auxid:1")) ||
                    (type.equals("urn:mpeg:avc:2015:auxid:1")) ||
                    (type.equals("urn:mpeg:mpegB:cicp:systems:auxiliary:alpha")))
                {
                    alphaImage = aux;
                }
            }
        }

        if (alphaImage != null && !alphaImage.equals(mAlphaImage))
        {
            //Release old alpha drawable since alpha plane has changed.
            if (mAlphaDrawableSurface != null)
            {
                mAlphaDrawableSurface.dispose();
            }
            mAlphaDrawableSurface = null;
            if (alphaImage != null)
            {
                //Create alpha drawable.
                mAlphaDrawableSurface = DrawableSurfaceFactory.createSurface(alphaImage, mResources);
                mAlphaDrawableSurface.refreshContent();
            }
            mAlphaImage = alphaImage;
        }

        //Destroy/Re-create the render target if needed.
        if (mAlphaDrawableSurface != null)
        {
            //create / re-size alpha render target if needed.
            if ((mAlphaRenderTarget == null) || ((mAlphaImage.getSize().width != mAlphaRenderTarget.width) || (mAlphaImage.getSize().height != mAlphaRenderTarget.height)))
            {
                if (mAlphaRenderTarget != null)
                {
                    mResources.removeResource(toString() + "_Alpha");
                    mAlphaRenderTarget = null;
                }
                mAlphaRenderTarget = new RenderTarget(mAlphaImage.getSize().width, mAlphaImage.getSize().height);
                mResources.addResource(toString() + "_Alpha", mAlphaRenderTarget);
            }
        }
        else
        {
            //alpha render target not needed anymore.
            if (mAlphaRenderTarget != null)
            {
                mResources.removeResource(toString() + "_Alpha");
                mAlphaRenderTarget = null;
            }
        }
    }

    public DrawableSurface(ImageItem image, GLResources aResources)
            throws Exception
    {
        mResources = aResources;
        mImageItem = image;

        //Fill the texture matrix with identity. we need this since the decoded texture could be and most likely is flipped.
        for (int i = 0; i < 4 * 4; i++)
        {
            mTextureMatrix[i] = 0.f;
        }
        mTextureMatrix[0 + 0 * 4] = mTextureMatrix[1 + 1 * 4] = mTextureMatrix[2 + 2 * 4] = mTextureMatrix[3 + 3 * 4] = 1.f;
    }

    public DrawableSurface(VideoTrack track, GLResources aResources)
            throws Exception
    {
        mResources = aResources;
        mVideoTrack = track;

        //Fill the texture matrix with identity. we need this since the decoded texture could be and most likely is flipped.
        for (int i = 0; i < 4 * 4; i++)
        {
            mTextureMatrix[i] = 0.f;
        }
        mTextureMatrix[0 + 0 * 4] = mTextureMatrix[1 + 1 * 4] = mTextureMatrix[2 + 2 * 4] = mTextureMatrix[3 + 3 * 4] = 1.f;
    }

    public void draw(RenderTarget renderTarget, Rect rect)
    {
        if ((mAlphaDrawableSurface != null) && (mAlphaRenderTarget != null))
        {
            //Okay we have a alpha aux surface.
            //render that first to our alpha texture
            mAlphaDrawableSurface.draw(mAlphaRenderTarget, new Rect(0, 0, mAlphaRenderTarget.width, mAlphaRenderTarget.height));
        }

        for (DrawableChild child : mChildren)
        {
            child.surface.draw(mRenderTarget, child.targetRect);
        }
    }

    public boolean readyToDraw()
    {
        if ((mAlphaDrawableSurface != null) && (mAlphaRenderTarget != null))
        {
            if (mAlphaDrawableSurface.readyToDraw() == false)
            {
                return false;
            }
        }
        for (DrawableChild child : mChildren)
        {
            if (!child.surface.readyToDraw())
            {
                return false;
            }
        }
        return true;
    }

    private float[] QUAD = {
            -1.0f, -1.0f, 0.0f, 1.0f,      //X Y  U V
            1.0f, -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 0.0f};

    private void refreshVertexBuffer()
            throws Exception
    {

        if (mVertexBuffer == null)
        {
            mVertexBuffer = new VertexBuffer();
            mResources.addResource(toString() + "_VB", mVertexBuffer);
        }
        //Now refresh all our content..
        mOesTexture = (this instanceof CodedImageSurface);

        float tw;
        float th;
        if (mImageItem != null)
        {
            tw = mImageItem.getSize().width;
            th = mImageItem.getSize().height;
        }
        else // if (mVideoTrack != null)
        {
            tw = mVideoTrack.getDisplaySize().width;
            th = mVideoTrack.getDisplaySize().height;
        }

        mRenderWidth = tw;
        mRenderHeight = th;
        float[][] textureCoordinates = {{0, 0}, {tw, 0}, {0, th}, {tw, th}};

        if (mImageItem != null)
        {
            textureCoordinates = applyTransformativeProperties(tw, th, textureCoordinates);
        }
        //convert from pixels to OpenGL normalized...
        //Note the texel center offset.
        for (int i = 0; i < 4; i++)
        {
            textureCoordinates[i][0] = (textureCoordinates[i][0] / (tw + 1.f)) + (1.f / tw) * 0.5f;
            textureCoordinates[i][1] = (textureCoordinates[i][1] / (th + 1.f)) + (1.f / th) * 0.5f;
            textureCoordinates[i][1] = 1.f - textureCoordinates[i][1];
        }

        QUAD[0 * 4 + 2 + 0] = textureCoordinates[2][0];
        QUAD[0 * 4 + 2 + 1] = textureCoordinates[2][1];//bottom left
        QUAD[1 * 4 + 2 + 0] = textureCoordinates[3][0];
        QUAD[1 * 4 + 2 + 1] = textureCoordinates[3][1];//bottom right
        QUAD[2 * 4 + 2 + 0] = textureCoordinates[0][0];
        QUAD[2 * 4 + 2 + 1] = textureCoordinates[0][1];//top left

        QUAD[3 * 4 + 2 + 0] = textureCoordinates[3][0];
        QUAD[3 * 4 + 2 + 1] = textureCoordinates[3][1];//bottom right
        QUAD[4 * 4 + 2 + 0] = textureCoordinates[1][0];
        QUAD[4 * 4 + 2 + 1] = textureCoordinates[1][1];//top right
        QUAD[5 * 4 + 2 + 0] = textureCoordinates[0][0];
        QUAD[5 * 4 + 2 + 1] = textureCoordinates[0][1];//top left
    }

    private float[][] applyTransformativeProperties(float tw, float th, float[][] textureCoordinates) throws Exception
    {
        float[] tmp;
        int angle = 0;
        boolean flip = false;
        int mirror = 0;
        List<TransformativeProperty> transforms = mImageItem.getTransformativeProperties();
        for (int i = 0; i < transforms.size(); i++)
        {
            if (transforms.get(i) instanceof RotateProperty)
            {
                RotateProperty prop = (RotateProperty) transforms.get(i);
                angle = prop.getRotation().getValue();
                if ((angle == 90) || (angle == 270))
                {
                    flip = true;
                    tmp = textureCoordinates[0];
                    textureCoordinates[0] = textureCoordinates[1];
                    textureCoordinates[1] = textureCoordinates[3];
                    textureCoordinates[3] = textureCoordinates[2];
                    textureCoordinates[2] = tmp;
                    float t = mRenderWidth;
                    mRenderWidth = mRenderHeight;
                    mRenderHeight = t;
                }
                if ((angle == 180) || (angle == 270))
                {
                    tmp = textureCoordinates[0];
                    textureCoordinates[0] = textureCoordinates[3];
                    textureCoordinates[3] = tmp;
                    tmp = textureCoordinates[1];
                    textureCoordinates[1] = textureCoordinates[2];
                    textureCoordinates[2] = tmp;
                }
            }
            if (transforms.get(i) instanceof MirrorProperty)
            {
                MirrorProperty prop = (MirrorProperty) transforms.get(i);
                if (prop.getMirror() == MirrorProperty.Mirror.HORIZONTAL)
                {
                    mirror = 1;
                    tmp = textureCoordinates[0];
                    textureCoordinates[0] = textureCoordinates[1];
                    textureCoordinates[1] = tmp;
                    tmp = textureCoordinates[2];
                    textureCoordinates[2] = textureCoordinates[3];
                    textureCoordinates[3] = tmp;
                }
                else
                {
                    mirror = 2;
                    tmp = textureCoordinates[0];
                    textureCoordinates[0] = textureCoordinates[2];
                    textureCoordinates[2] = tmp;
                    tmp = textureCoordinates[1];
                    textureCoordinates[1] = textureCoordinates[3];
                    textureCoordinates[3] = tmp;
                }
                if (flip)
                {
                    //flipmirror (rotated first)
                    mirror += 2;
                }
            }
            if (transforms.get(i) instanceof CleanApertureProperty)
            {
                CleanApertureProperty prop = (CleanApertureProperty) transforms.get(i);
                float dw = prop.getWidth().numerator / (float) prop.getWidth().denominator;
                float dh = prop.getHeight().numerator / (float) prop.getHeight().denominator;
                float dx = prop.getHorizontalOffset().numerator / (float) prop.getHorizontalOffset().denominator;
                float dy = prop.getVerticalOffset().numerator / (float) prop.getVerticalOffset().denominator;
                int angle_index = 0;
                switch (angle)
                {
                    case 0:
                        angle_index = 0;
                        break;
                    case 90:
                        angle_index = 1;
                        break;
                    case 180:
                        angle_index = 2;
                        break;
                    case 270:
                        angle_index = 3;
                        break;
                }
                int[/*mirror*/][/*angle*/] types =
                        {
                                {1, 2, 3, 4}, //no mirror
                                {6, 8, 5, 7}, //horizontal mirror    (rotate after mirror)
                                {5, 7, 6, 8}, //vertical mirror      (rotate after mirror)
                                {6, 7, 5, 8}, //horizontal mirror    (rotate before mirror)
                                {5, 8, 6, 7}  //vertical mirror      (rotate before mirror)
                        };
                //Select correct texture coordinates based on mirror and rotation.
                switch (types[mirror][angle_index])
                {
                    case 1:
                    {
                        textureCoordinates = new float[][]{{dx, dy}, {dx + dw, dy}, {dx, dy + dh}, {dx + dw, dy + dh}};
                        break;
                    }
                    case 2:
                    {
                        textureCoordinates = new float[][]{{tw - dy, dx}, {tw - dy, dx + dw}, {tw - dh - dy, dx}, {tw - dh - dy, dx + dw}};
                        break;
                    }
                    case 3:
                    {
                        textureCoordinates = new float[][]{{tw - dx, th - dy}, {tw - dw - dx, th - dy}, {tw - dx, th - dh - dy}, {tw - dw - dx, th - dh - dy}};
                        break;
                    }
                    case 4:
                    {
                        textureCoordinates = new float[][]{{dy, th - dx}, {dy, th - dw - dx}, {dh + dy, th - dx}, {dh + dy, th - dw - dx}};
                        break;
                    }
                    case 5:
                    {
                        textureCoordinates = new float[][]{{dx, th - dy}, {dx + dw, th - dy}, {dx, th - dy - dh}, {dx + dw, th - dy - dh}};
                        break;
                    }
                    case 6:
                    {
                        textureCoordinates = new float[][]{{tw - dx, dy}, {tw - dx - dw, dy}, {tw - dx, dy + dh}, {tw - dx - dw, dy + dh}};
                        break;
                    }
                    case 7:
                    {
                        textureCoordinates = new float[][]{{tw - dy, th - dx}, {tw - dy, th - dx - dw}, {tw - dh - dy, th - dx}, {tw - dh - dy, th - dx - dw}};
                        break;
                    }
                    case 8:
                    {
                        textureCoordinates = new float[][]{{dy, dx}, {dy, dx + dw}, {dh + dy, dx}, {dh + dy, dx + dw}};
                        break;
                    }
                }
                mRenderWidth = dw;
                mRenderHeight = dh;
            }
        }
        return textureCoordinates;
    }

    private void applyZoom()
    {
        // if (mZoomFactor != 1.0f)
        {
            float scaledSize = 1.0f / mZoomFactor;
            float left = mZoomCenter.x - (scaledSize / 2);
            float right = left + scaledSize;
            float bottom = (1.0f - mZoomCenter.y) - (scaledSize / 2);
            float top = bottom + scaledSize;

            QUAD[0 * 4 + 2 + 0] = left;
            QUAD[0 * 4 + 2 + 1] = bottom;//bottom left
            QUAD[1 * 4 + 2 + 0] = right;
            QUAD[1 * 4 + 2 + 1] = bottom;//bottom right
            QUAD[2 * 4 + 2 + 0] = left;
            QUAD[2 * 4 + 2 + 1] = top;//top left

            QUAD[3 * 4 + 2 + 0] = right;
            QUAD[3 * 4 + 2 + 1] = bottom;//bottom right
            QUAD[4 * 4 + 2 + 0] = right;
            QUAD[4 * 4 + 2 + 1] = top;//top right
            QUAD[5 * 4 + 2 + 0] = left;
            QUAD[5 * 4 + 2 + 1] = top;//top left

        }
    }

    protected void DrawQuad(RenderTarget fbo, Rect r, int mTid)
    {
        int mAlphaTid = 0;
        if ((mAlphaDrawableSurface != null) && (mAlphaRenderTarget != null))
        {
            mAlphaTid = mAlphaRenderTarget.textureId;
        }

        applyZoom();

        mVertexBuffer.update(QUAD);

        float x = r.left;
        float y = r.top;
        float xscl = r.width() / mRenderWidth;
        float yscl = r.height() / mRenderHeight;
        float rw = mRenderWidth * xscl;
        float rh = mRenderHeight * yscl;

        fbo.bind();
        GLES20.glEnable(GLES20.GL_SCISSOR_TEST);
        GLES20.glScissor((int) x, (int) (fbo.height - (y + rh)), (int) rw, (int) rh);
        GLES20.glViewport((int) x, (int) (fbo.height - (y + rh)), (int) rw, (int) rh);
        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, mVertexBuffer.mVb);
        Shader shd = null;
        if (mOesTexture)
        {
            if (mAlphaTid != 0)
            {
                shd = mColorAlphaOES;
            }
            else
            {
                shd = mColorOES;
            }
        }
        else
        {
            if (mAlphaTid != 0)
            {
                shd = mColorAlpha;
            }
            else
            {
                shd = mColor;
            }
        }
        GLES20.glUseProgram(shd.mShader);
        GLES20.glUniformMatrix4fv(shd.TRANSFORM_UNIFORM, 1, false, mTextureMatrix, 0);
        GLES20.glVertexAttribPointer(shd.POS_ATTRIB, 2, GLES20.GL_FLOAT, false, 4 * 4, 0);
        GLES20.glVertexAttribPointer(shd.TEX_ATTRIB, 2, GLES20.GL_FLOAT, false, 4 * 4, 2 * 4);
        GLES20.glEnableVertexAttribArray(shd.POS_ATTRIB);
        GLES20.glEnableVertexAttribArray(shd.TEX_ATTRIB);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(shd.COLOR_TYPE, mTid);
        GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
        if (mAlphaTid != 0)
        {
            GLES20.glEnable(GLES20.GL_BLEND);
            GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);
            GLES20.glBindTexture(shd.ALPHA_TYPE, mAlphaTid);
        }

        GLES20.glDrawArrays(GLES20.GL_TRIANGLES, 0, 6);

        GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, 0);
        if (mAlphaTid != 0)
        {
            GLES20.glDisable(GLES20.GL_BLEND);
            GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        }

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        if (mOesTexture)
        {
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, 0);
        }
        else
        {
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        }

        fbo.unbind();
    }

    protected void initShaders()
    {
        //Instantiate shaders only once.
        mColor = (Shader) mResources.getResource("Color");
        if (mColor == null)
        {
            mColor = new Shader(VERTEX_SHADER, FRAGMENT_PREFIX + FRAGMENT_SHADER_C);
            mColor.COLOR_TYPE = GLES20.GL_TEXTURE_2D;
            mColor.ALPHA_TYPE = GLES20.GL_TEXTURE_2D;
            mResources.addResource("Color", mColor);
        }
        mColorOES = (Shader) mResources.getResource("ColorOES");
        if (mColorOES == null)
        {
            mColorOES = new Shader(VERTEX_SHADER, FRAGMENT_PREFIX_OES + FRAGMENT_SHADER_C);
            mColorOES.COLOR_TYPE = GLES11Ext.GL_TEXTURE_EXTERNAL_OES;
            mColorOES.ALPHA_TYPE = GLES20.GL_TEXTURE_2D;
            mResources.addResource("ColorOES", mColorOES);
        }
        mColorAlpha = (Shader) mResources.getResource("ColorAlpha");
        if (mColorAlpha == null)
        {
            mColorAlpha = new Shader(VERTEX_SHADER, FRAGMENT_PREFIX + FRAGMENT_SHADER_CA);
            mColorAlpha.COLOR_TYPE = GLES20.GL_TEXTURE_2D;
            mColorAlpha.ALPHA_TYPE = GLES20.GL_TEXTURE_2D;
            mResources.addResource("ColorAlpha", mColorAlpha);
        }
        mColorAlphaOES = (Shader) mResources.getResource("ColorAlphaOES");
        if (mColorAlphaOES == null)
        {
            mColorAlphaOES = new Shader(VERTEX_SHADER, FRAGMENT_PREFIX_OES + FRAGMENT_SHADER_CA);
            mColorAlphaOES.COLOR_TYPE = GLES11Ext.GL_TEXTURE_EXTERNAL_OES;
            mColorAlphaOES.ALPHA_TYPE = GLES20.GL_TEXTURE_2D;
            mResources.addResource("ColorAlphaOES", mColorAlphaOES);
        }
    }

    protected void releaseShaders()
    {
        if (mColor != null)
        {
            mColor = null;
            mResources.removeResource("Color");
        }
        if (mColorOES != null)
        {
            mColorOES = null;
            mResources.removeResource("ColorOES");
        }
        if (mColorAlpha != null)
        {
            mColorAlpha = null;
            mResources.removeResource("ColorAlpha");
        }
        if (mColorAlphaOES != null)
        {
            mColorAlphaOES = null;
            mResources.removeResource("ColorAlphaOES");
        }
    }

    private static final String VERTEX_PREFIX = "precision mediump float;\n"
            + "attribute vec2 pos;\n"
            + "attribute vec2 tex;\n"
            + "uniform mat4 u_matrix;\n"          //texture transform matrix.
            + "varying vec2 texCoord;\n"
            + "varying vec2 texCoordA;\n"
            + "varying vec2 texCoordO;\n";

    private static final String FRAGMENT_PREFIX = "precision mediump float;\n"
            + "uniform sampler2D u_texture;\n"      //Color (rgb) or Luma (Y) (r)
            + "uniform sampler2D u_texture_A;\n"    //Alpha (r)
            + "uniform sampler2D u_texture_O;\n"    //Overlay (rgb)
            + "varying vec2 texCoord;\n"
            + "varying vec2 texCoordA;\n"
            + "varying vec2 texCoordO;\n";

    private static final String FRAGMENT_PREFIX_OES = "#extension GL_OES_EGL_image_external : require\n"
            + "precision mediump float;\n"
            + "uniform samplerExternalOES u_texture;\n" //Color (rgb) or Luma (Y) (r)
            + "uniform sampler2D u_texture_A;\n"        //Alpha (r)
            + "uniform sampler2D u_texture_O;\n"        //Overlay (rgb)
            + "varying vec2 texCoord;\n"
            + "varying vec2 texCoordA;\n"
            + "varying vec2 texCoordO;\n";

    private static final String VERTEX_SHADER = VERTEX_PREFIX
            + "void main()\n"
            + "{\n"
            + "  texCoord = (u_matrix*vec4(tex,0.0,1.0)).xy;\n"
            + "  texCoordA = (u_matrix*vec4(tex,0.0,1.0)).xy;\n"
            + "  texCoordO = (u_matrix*vec4(tex,0.0,1.0)).xy;\n"
            + "  texCoordA.y = texCoordA.y;\n"
            + "  gl_Position = vec4(pos,1.0,1.0);\n"
            + "}";


    private static final String FRAGMENT_SHADER_C =
            "void main()\n"
            + "{\n"
            + "  gl_FragColor = texture2D(u_texture, texCoord);\n"
            + "}";

    private static final String FRAGMENT_SHADER_CA =
            "void main()\n"
            + "{\n"
            + "  gl_FragColor = vec4(texture2D(u_texture, texCoord).rgb,texture2D(u_texture_A, texCoordA).r);\n"
            + "}";
}
