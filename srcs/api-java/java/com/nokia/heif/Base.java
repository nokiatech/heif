/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 *
 *
 */

package com.nokia.heif;

import java.lang.ref.WeakReference;

/**
 * Common base class for all objects belonging to the HEIF instance (except the HEIF instance itself)
 */
public abstract class Base
{

    /** Handle to the C++ object */
    protected long mNativeHandle;

    /** Handle to the parent HEIF instance */
    protected WeakReference<HEIF> mHEIF;

    /** Freely definable userdata in the Java context. Exists only runtime, not saved to file */
    protected Object mUserData;


    /**
     * Constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @exception Exception Thrown if the parent HEIF instance is invalid
     */
    protected Base(HEIF heif)
            throws Exception
    {
        if (heif == null)
        {
            throw new Exception(ErrorHandler.INVALID_PARAMETER, "HEIF can't be null");
        }
        else if (heif.mNativeHandle == 0)
        {
            throw new Exception(ErrorHandler.OBJECT_ALREADY_DELETED, "HEIF instance already deleted");
        }
        mHEIF = new WeakReference<HEIF>(heif);
    }

    /**
     * Protected constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected Base(HEIF heif, long nativeHandle)
    {
        mHEIF = new WeakReference<HEIF>(heif);
        mNativeHandle = nativeHandle;
    }

    @Override
    public boolean equals(Object obj)
    {
        if (obj instanceof Base)
        {
            return ((Base)obj).mNativeHandle == this.mNativeHandle;
        }
        else
        {
            return false;
        }
    }

    /**
     * Removes this object from its parent HEIF and releases the native resources
     */
    public void destroy()
    {
        if (mNativeHandle != 0)
        {
            destroyNative();
            releaseHandles();
        }
    }

    /**
     * Releases the Java handles
     */
    protected void releaseHandles()
    {
        mHEIF = null;
        mNativeHandle = 0;
    }

    /**
     * Returns the parent HEIF of the object
     * @return The parent HEIF object
     */
    public HEIF getParentHEIF()
    {
        return mHEIF.get();
    }

    /**
     * Checks that the internal state of the object is still valid and throws an exception if it's not.
     * @throws Exception
     */
    protected void checkState()
            throws Exception
    {
        if (mNativeHandle == 0)
        {
            throw new Exception(ErrorHandler.OBJECT_ALREADY_DELETED, "Object already deleted");
        }
    }

    /**
     * Sets userdata for the item. Can be freely defined and stored only runtime, not saved to file
     * @param userData Generic Java object containing the user data
     */
    public void setUserData(Object userData)
    {
        mUserData = userData;
    }

    /**
     * Returns the user data
     * @return Generic Java object containing the user data
     */
    public Object getUserData()
    {
        return mUserData;
    }

    /**
     * Checks that the given parameter is valid
     * @param handle Object being passed as a parameter
     * @throws Exception Thrown if the object is either null, deleted or belongs to a different HEIF instance
     */
    protected void checkParameter(Object handle)
            throws Exception
    {
        if (handle == null)
        {
            throw new Exception(ErrorHandler.INVALID_PARAMETER, "Object is null");
        }

        if (handle instanceof Base)
        {
            Base baseHandle = (Base) handle;
            if (baseHandle.mNativeHandle == 0)
            {
                throw new Exception(ErrorHandler.OBJECT_ALREADY_DELETED, "Object already deleted");
            }
            else if (baseHandle.getParentHEIF() != mHEIF.get())
            {
                throw new Exception(ErrorHandler.WRONG_HEIF_INSTANCE, "HEIF instances don't match");
            }
        }

    }

    /**
     * Checks that the given parameter is null or valid
     * @param handle Object being passed as a parameter
     * @throws Exception Thrown if the object is deleted or belongs to a different HEIF instance
     */
    protected void checkParameterAllowNull(Base handle)
            throws Exception
    {
        if (handle != null)
        {
            checkParameter(handle);
        }
    }

    abstract protected void destroyNative();

}
