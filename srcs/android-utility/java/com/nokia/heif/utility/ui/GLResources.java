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

import android.support.annotation.Nullable;
import android.util.Log;
import android.util.Pair;

import com.nokia.heif.utility.codec.CodecUtility;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

public class GLResources
{
    //ref counted resources.
    private class RefCount
    {
        int count;
        GLResource object;
        boolean released;

        public RefCount(GLResource a)
        {
            released = true;
            object = a;
            count = 1;
        }

        public int addRef()
        {
            count++;
            return count;
        }

        public int decRef()
        {
            count--;
            if (count == 0)
            {
                object.release(false);
                released = true;
                object = null;
            }
            return count;
        }

        public void release(boolean aLostContext)
        {
            if (released)
            {
                Log.e("GLResources", "release called on object that has been released!");
            }
            else
            {
                released = true;
                object.release(aLostContext);
            }
        }

        public void reserve()
        {
            if (!released)
            {
                Log.e("GLResources", "reserve called on object that has not been released!");
            }
            else
            {
                released = false;
                object.reserve();
            }
        }
    }

    private final CodecUtility mCodecUtility = new CodecUtility();

    private final ArrayList<Runnable> mEventQueue = new ArrayList<>();
    private final Map<String, RefCount> mResources = new TreeMap<>();
    private final List<RefCount> mReserveQueue = new ArrayList<>();
    private final List<Pair<String, RefCount>> mRemoveQueue = new ArrayList<>();
    private long mGLThreadId = 0;

    private static final GLResources mGlobalResources = new GLResources();

    //use for decoders/encoders
    synchronized public static GLResources GlobalInstance()
    {
        return mGlobalResources;
    }

    synchronized public void Initialize()
    {
        mResources.clear();
        mReserveQueue.clear();
    }

    public CodecUtility getCodecUtility()
    {
        return mCodecUtility;
    }

    public void finalize()
    {
        release();
    }

    synchronized public void reserve()
    {
        Log.e("GLResources", "RESERVE! " + mResources.size());
        long curThreadId = Thread.currentThread().getId();
        if (mGLThreadId == 0)
        {
            mGLThreadId = curThreadId;
        }
        else
        {
            //make sure the thread is correct.
            if (mGLThreadId != curThreadId)
            {
                Log.e("GLResources", "GLResources reserve called from incorrect thread? " + mGLThreadId + " " + curThreadId);
            }
        }
        for (Map.Entry<String, RefCount> entry : mResources.entrySet())
        {
            RefCount val = entry.getValue();
            val.reserve();
            if (mReserveQueue.contains(val))
            {
                val.decRef();
                mReserveQueue.remove(val);
            }
        }
    }

    boolean inRelease = false;

    synchronized public void release()
    {
        Log.e("GLResources", "RELEASE! " + mResources.size());
        if (mGLThreadId == 0)
        {
            Log.e("GLResources", "GLResources release called without being reserved,finalize?) " + mGLThreadId);
            mResources.clear();
            mReserveQueue.clear();
        }
        else
        {
            long curThreadId = Thread.currentThread().getId();
            //make sure the thread is correct.
            if (mGLThreadId != curThreadId)
            {
                Log.e("GLResources", "GLResources release called from incorrect thread? " + mGLThreadId + " " + curThreadId);
            }
            inRelease = true;
            for (Map.Entry<String, RefCount> entry : mResources.entrySet())
            {
                RefCount val = entry.getValue();
                val.release(true);
            }
            inRelease = false;
            for (Pair<String, RefCount> key : mRemoveQueue)
            {
                mResources.remove(key.first, key.second);
            }
            mRemoveQueue.clear();
            mReserveQueue.clear();
            mGLThreadId = 0;
        }
        mCodecUtility.releaseResources();
    }

    synchronized public boolean hasResource(String key)
    {
        return mResources.containsKey(key);
    }

    @Nullable
    synchronized public GLResource getResource(String key)
    {
        RefCount res;
        res = mResources.get(key);
        if (res != null)
        {
            res.addRef();
            return res.object;
        }
        return null;
    }

    synchronized public void addResource(String key, GLResource resource)
    {
        RefCount ref = new RefCount(resource);
        mResources.put(key, ref);
        //add to queue if NOT in GLthread
        long curThreadId = Thread.currentThread().getId();
        //make sure the thread is correct.
        if (mGLThreadId != curThreadId)
        {
            ref.addRef();
            mReserveQueue.add(ref);
        }
        else
        {
            ref.reserve();
        }
    }

    synchronized public void removeResource(String key)
    {
        RefCount res = mResources.get(key);
        if (res != null)
        {
            if (0 == res.decRef())
            {
                if (inRelease)
                {
                    mRemoveQueue.add(new Pair<String, RefCount>(key, res));
                }
                else
                {
                    mResources.remove(key, res);
                }
            }
        }
        if (mResources.size() == 0)
        {
            Log.e("GLResources", "All resources removed");
        }
    }

    synchronized public void queueEvent(Runnable r)
    {
        mEventQueue.add(r);
    }

    synchronized public void predraw()
    {
        while (!mEventQueue.isEmpty())
        {
            Runnable event = mEventQueue.remove(0);
            event.run();
        }

        for (int i = 0; i < mReserveQueue.size(); i++)
        {
            mReserveQueue.get(i).reserve();
            mReserveQueue.get(i).decRef();
        }
        mReserveQueue.clear();
    }

    synchronized static public void cleanup()
    {
        //Just a helper to force GC.
        System.runFinalization();
        System.gc();
    }
}
