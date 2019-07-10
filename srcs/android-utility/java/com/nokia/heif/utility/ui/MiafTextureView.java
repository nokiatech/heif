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

import android.content.Context;
import android.graphics.PointF;
import android.graphics.SurfaceTexture;
import android.opengl.GLSurfaceView;
import android.opengl.GLUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.view.TextureView;
import android.view.View;

import com.nokia.heif.Base;
import com.nokia.heif.ImageItem;
import com.nokia.heif.Track;
import com.nokia.heif.VideoTrack;

import java.util.List;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL;
import javax.microedition.khronos.opengles.GL10;

public class MiafTextureView extends TextureView implements TextureView.SurfaceTextureListener
{
    private GLSurfaceView.Renderer mRenderer = null;
    private GLThread mGlThread;
    private final static String TAG = "MiafTextureView";
    private GLResources mResources;
    private Object mSync = new Object();
    private boolean mRequestRender = true;
    private SequencePlayerThread mSequencePlayerThread = null;

    protected ImageItem mCurrentImageItem;
    protected VideoTrack mCurrentVideoTrack;

    // Main Thread
    public MiafTextureView(Context context)
    {
        this(context, null);
    }

    // Main Thread
    public MiafTextureView(Context context, AttributeSet attrs)
    {
        this(context, attrs, 0);
    }

    // Main Thread
    public MiafTextureView(Context context, AttributeSet attrs, int defStyleAttr)
    {
        super(context, attrs, defStyleAttr);
        setSurfaceTextureListener(this);
        mResources = new GLResources();
        mResources.Initialize();
    }

    public void zoom(PointF zoomCenter, float zoomFactor)
    {
        ((MiafRenderer) mRenderer).zoom(zoomCenter, zoomFactor);
    }

    @Override
    protected void onVisibilityChanged(View changedView, int visibility)
    {
        super.onVisibilityChanged(changedView, visibility);
    }

    // Main Thread
    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height)
    {
        Log.v(TAG, "onSurfaceTextureAvailable " + width + " " + height);
        mGlThread = new GLThread(surface);
        mGlThread.start();
    }

    // Main Thread
    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height)
    {
        Log.v(TAG, "onSurfaceTextureSizeChanged " + width + " " + height);
        mGlThread.onWindowResize(width, height);
    }

    // Main Thread
    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface)
    {
        Log.v(TAG, "onSurfaceTextureDestroyed");
        mGlThread.finish();
        try
        {
            Log.v(TAG, "onSurfaceTextureDestroyed join");
            mGlThread.join();
            mGlThread = null;
        }
        catch (InterruptedException e)
        {
            Log.e(TAG, "onSurfaceTextureDestroyed interrupted " + e.getMessage());
            e.printStackTrace();
        }
        Log.v(TAG, "onSurfaceTextureDestroyed done");
        return false;   // if return false then need to manually release
    }

    // Main Thread
    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface)
    {
        //Log.v(TAG, "onSurfaceTextureUpdated");
    }

    private class GLThread extends Thread
    {
        static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
        static final int EGL_OPENGL_ES2_BIT = 4;

        private final SurfaceTexture mSurface;

        private EGL10 mEgl;
        private EGLDisplay mEglDisplay;
        private EGLConfig mEglConfig;
        private EGLContext mEglContext;
        private EGLSurface mEglSurface;
        private GL mGl;
        private int mWidth = getWidth();
        private int mHeight = getHeight();

        private volatile boolean mFinished = false;
        private volatile boolean mSizeChanged = false;

        GLThread(SurfaceTexture surface)
        {
            super("GLThread");
            this.mSurface = surface;
        }

        @Override
        public void run()
        {
            GL10 gl10 = null;
            for (; ; )
            {
                synchronized (mSync)
                {
                    if (mFinished)
                    {
                        gl10 = null;
                        ((MiafRenderer) mRenderer).dispose();

                        finishGL();
                        mSurface.release();
                        break;
                    }
                    if (gl10 == null)
                    {
                        boolean failed = !initGL();// TODO: initGL could fail, and when it does all rendering breaks!
                        if (failed)
                        {
                            // so just terminate thread on failure
                            mFinished = true;
                            continue;
                        }
                        gl10 = (GL10) mGl;
                        mRenderer.onSurfaceCreated(gl10, mEglConfig);
                    }

                    if (mSizeChanged) // Size of surface/surface texture has changes, update the EGL surface
                    {
                        boolean failed = !createSurface();// TODO: createSurface could fail, and when it does all rendering breaks!
                        if (failed)
                        {
                            // so just terminate thread on failure
                            // TODO: It might be possible to recover from this error, by just trying again.
                            mFinished = true;
                            continue;
                        }
                        mRenderer.onSurfaceChanged(gl10, mWidth, mHeight);
                        mSizeChanged = false;
                    }

                    // Make sure the correct context is active. (unless mRenderer changes the context it is always correct)
                    if (!mEglContext.equals(mEgl.eglGetCurrentContext()) || !mEglSurface.equals(mEgl.eglGetCurrentSurface(EGL10.EGL_DRAW)) || !mEglSurface.equals(mEgl.eglGetCurrentSurface(EGL10.EGL_READ)))
                    {
                        int error = mEgl.eglGetError();
                        if (error != EGL10.EGL_SUCCESS)
                        {
                            // There was a pending error.
                            Log.e(TAG, "EGL error = " + GLUtils.getEGLErrorString(error));
                        }
                        if (!mEgl.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext))
                        {
                            Log.e(TAG, "eglMakeCurrent failed " + GLUtils.getEGLErrorString(mEgl.eglGetError()));
                            continue;
                        }
                    }

                    mRenderer.onDrawFrame(gl10);
                    if (!mEgl.eglSwapBuffers(mEglDisplay, mEglSurface))
                    {
                        Log.e(TAG, "Cannot swap buffers: " + GLUtils.getEGLErrorString(mEgl.eglGetError()));
                    }

                    try
                    {
                        if (!mRequestRender)
                        {
                            Log.e(TAG, "Waiting");
                            mSync.wait();
                            Log.e(TAG, "Done");
                        }
                        mRequestRender = false;
                    }
                    catch (java.lang.InterruptedException e)
                    {
                        // Interrupted exception means we should shut down.
                        Log.e(TAG, "Interrupted");
                        mFinished = true;
                    }
                }
            }
            Log.v(TAG, "Finished loop");

        }


        public boolean createSurface()
        {
            if (mEglSurface != null && mEglSurface != EGL10.EGL_NO_SURFACE)
            {
                mEgl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
                mEgl.eglDestroySurface(mEglDisplay, mEglSurface);
                mEglSurface = null;
            }

            try
            {
                mEglSurface = mEgl.eglCreateWindowSurface(mEglDisplay, mEglConfig, mSurface, null);
            }
            catch (IllegalArgumentException e)
            {
                // This exception indicates that the surface flinger surface
                // is not valid. This can happen if the surface flinger surface has
                // been torn down, but the application has not yet been
                // notified via SurfaceHolder.Callback.surfaceDestroyed.
                // In theory the application should be notified first,
                // but in practice sometimes it is not. See b/4588890
                Log.e(TAG, "eglCreateWindowSurface failed! ", e);
                return false;
            }

            if (mEglSurface == null || mEglSurface == EGL10.EGL_NO_SURFACE)
            {
                int error = mEgl.eglGetError();
                if (error == EGL10.EGL_BAD_NATIVE_WINDOW)
                {
                    Log.e(TAG, "createWindowSurface returned EGL_BAD_NATIVE_WINDOW.");
                }
                else
                {
                    Log.e(TAG, "createWindowSurface returned " + GLUtils.getEGLErrorString(mEgl.eglGetError()));
                }
                return false;
            }

            if (!mEgl.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext))
            {
                Log.e(TAG, "eglMakeCurrent failed " + GLUtils.getEGLErrorString(mEgl.eglGetError()));
                return false;
            }

            return true;
        }

        private void finishGL()
        {
            synchronized (mSync)
            {
                mRenderer = null;
                if (mResources != null)
                {
                    mResources.release();
                }
                mResources = null;
                if (mEglDisplay != null && mEglDisplay != EGL10.EGL_NO_DISPLAY)
                {
                    mEgl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
                    if (mEglContext != null && mEglContext != EGL10.EGL_NO_CONTEXT)
                    {
                        mEgl.eglDestroyContext(mEglDisplay, mEglContext);
                    }
                    if (mEglSurface != null && mEglSurface != EGL10.EGL_NO_SURFACE)
                    {
                        mEgl.eglDestroySurface(mEglDisplay, mEglSurface);
                    }
                    mEgl.eglTerminate(mEglDisplay);
                }
                mEglContext = null;
                mEglSurface = null;
                mEglDisplay = null;
            }
        }

        private boolean initGL()
        {
            mEgl = (EGL10) EGLContext.getEGL();

            mEglDisplay = mEgl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
            if (mEglDisplay == EGL10.EGL_NO_DISPLAY)
            {
                Log.e(TAG, "eglGetDisplay failed " + GLUtils.getEGLErrorString(mEgl.eglGetError()));
                return false;
            }

            int[] version = new int[2];
            if (!mEgl.eglInitialize(mEglDisplay, version))
            {
                Log.e(TAG, "eglInitialize failed " + GLUtils.getEGLErrorString(mEgl.eglGetError()));
                return false;
            }

            int[] configsCount = new int[1];
            EGLConfig[] configs = new EGLConfig[1];
            int[] configSpec = new int[]
                    {
                            EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                            EGL10.EGL_RED_SIZE, 8,
                            EGL10.EGL_GREEN_SIZE, 8,
                            EGL10.EGL_BLUE_SIZE, 8,
                            EGL10.EGL_ALPHA_SIZE, 8,
                            EGL10.EGL_DEPTH_SIZE, 0,
                            EGL10.EGL_STENCIL_SIZE, 0,
                            EGL10.EGL_NONE
                    };
            if (!mEgl.eglChooseConfig(mEglDisplay, configSpec, configs, 1, configsCount))
            {
                Log.e(TAG, "eglChooseConfig failed " + GLUtils.getEGLErrorString(mEgl.eglGetError()));
                return false;
            }
            mEglConfig = configs[0];

            int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE};
            mEglContext = mEgl.eglCreateContext(mEglDisplay, mEglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);

            mGl = mEglContext.getGL();
            mSizeChanged = true;
            return true;
        }

        void finish()
        {
            synchronized (mSync)
            {
                mFinished = true;
                mSync.notifyAll();
            }
        }

        public void onWindowResize(int w, int h)
        {
            synchronized (mSync)
            {
                mWidth = w;
                mHeight = h;
                mSizeChanged = true;
                mRequestRender = true;
                mSync.notifyAll();
            }
        }

    }

    public void requestRender()
    {
        synchronized (mSync)
        {
            mRequestRender = true;
            mSync.notifyAll();
        }
    }

    public boolean setContent(Base content)
    {
        if (mRenderer == null)
        {
            mRenderer = new MiafRenderer(this, mResources);
        }

        boolean result = false;
        if (content instanceof ImageItem)
        {
            ImageItem image = (ImageItem)content;
            result = ((MiafRenderer) this.mRenderer).setImage(image);
            if (result)
            {
                mCurrentImageItem = image;
            }
        }
        if (content instanceof VideoTrack)
        {
            VideoTrack track = (VideoTrack)content;
            result = ((MiafRenderer) this.mRenderer).setTrack(track);
            if (result)
            {
                mCurrentVideoTrack = track;
                mSequencePlayerThread = new SequencePlayerThread((MiafRenderer) this.mRenderer, track);
                mSequencePlayerThread.start();
            }
        }
        if (result)
        {
            requestRender();
        }

        return false;
    }

    public void togglePlayback()
    {
        mSequencePlayerThread.togglePlayback();
    }

    private class SequencePlayerThread extends Thread
    {
        MiafRenderer mRenderer;
        private List<Track.TimeStamp> mTimeStamps;
        private int mTimeStampIndex = 0;
        private double mPlaybackDuration = 0;

        private volatile boolean mRunning = true;
        private volatile boolean mPaused = true; // Start paused
        private final Object mPauseLock = new Object();

        SequencePlayerThread(MiafRenderer renderer, VideoTrack track)
        {
            this.setName("SequencePlayerThread");
            this.mRenderer = renderer;
            try
            {
                this.mTimeStamps = track.getTimestamps();
                this.mPlaybackDuration = track.getDuration();
            }
            catch (Exception e)
            {
                Log.e(TAG, "Getting timestamps for track failed.");
            }
        }

        public void run()
        {
            while (mRunning)
            {
                synchronized (mPauseLock)
                {
                    // Terminate immediately if stopped during synchronization.
                    if (!mRunning)
                    {
                        break;
                    }
                    if (mPaused)
                    {
                        try
                        {
                            mPauseLock.wait();
                        }
                        catch (InterruptedException ex)
                        {
                            break;
                        }
                        // Terminate immediately if stopped during pause wait.
                        if (!mRunning)
                        {
                            break;
                        }
                    }
                }

                final Track.TimeStamp timestamp = mTimeStamps.get(mTimeStampIndex);
                mRenderer.showFrame(timestamp.sampleIndex);

                mTimeStampIndex++;
                long sampleDuration;
                if (mTimeStampIndex < mTimeStamps.size())
                {
                    sampleDuration = mTimeStamps.get(mTimeStampIndex).timeStamp - timestamp.timeStamp;
                }
                else // calculate last sample duration from track duration
                {
                    sampleDuration = (long)(mPlaybackDuration * 1000) - timestamp.timeStamp;
                }

                if (mTimeStampIndex == mTimeStamps.size())
                {
                    mTimeStampIndex = 0;
                }

                try
                {
                    sleep(sampleDuration);
                }
                catch (InterruptedException ex)
                {
                    break;
                }
            }
        }

        public void continuePlayback()
        {
            synchronized (mPauseLock)
            {
                mPaused = false;
                mPauseLock.notifyAll();
            }
        }

        public void stopPlayback()
        {
            mRunning = false;
            continuePlayback();
        }

        public void pausePlayback()
        {
            mPaused = true;
        }

        public void togglePlayback()
        {
            if (mPaused)
            {
                continuePlayback();
            }
            else
            {
                pausePlayback();
            }
        }
    }
}
