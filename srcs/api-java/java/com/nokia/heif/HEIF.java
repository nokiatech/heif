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

import java.util.ArrayList;
import java.util.List;

public final class HEIF
{

    private static final String TAG = "HEIF";

    public static final FourCC FOURCC_AVC = new FourCC("avc1", true);
    public static final FourCC FOURCC_HEVC = new FourCC("hvc1", true);
    public static final FourCC FOURCC_JPEG = new FourCC("jpeg", true);
    public static final FourCC FOURCC_IDENTITY = new FourCC("iden", true);
    public static final FourCC FOURCC_OVERLAY = new FourCC("iovl", true);
    public static final FourCC FOURCC_GRID = new FourCC("grid", true);
    public static final FourCC FOURCC_EXIF = new FourCC("Exif", true);

    public static final FourCC BRAND_MIF1 = new FourCC("mif1", true);
    public static final FourCC BRAND_HEIC = new FourCC("heic", true);
    public static final FourCC BRAND_HEIX = new FourCC("heix", true);
    public static final FourCC BRAND_HEVC = new FourCC("hevc", true);
    public static final FourCC BRAND_AVCS = new FourCC("avcs", true);
    public static final FourCC BRAND_AVIC = new FourCC("avic", true);
    public static final FourCC BRAND_JPEG = new FourCC("jpeg", true);

    /**
     * Creates a HEIF instance which can be used to read and write HEIF files
     */
    public HEIF()
    {
        createInstanceNative();
    }

    /**
     * Releases the resources held by the HEIF instance. This will invalidate every child item of the instance
     */
    public void release()
    {
        if (mNativeHandle != 0)
        {
            destroyInstanceNative();
        }
    }

    @Override
    protected void finalize()
            throws Throwable
    {
        if (mNativeHandle != 0)
        {
            destroyInstanceNative();
        }
        super.finalize();
    }

    protected long mNativeHandle;

    /**
     * Loads a HEIF file. Can be called only once per instance.
     * @param filename Filename including the path
     * @throws Exception
     */
    public void load(String filename)
            throws Exception
    {
        checkState();
        checkParameter(filename);
        loadNative(filename);
    }

    /**
     * Saves the HEIF instance to a file. Can be called multiple times with the same instance.
     * Checks are performed to ensure that the file will be a valid HEIF file so the instance should have at least
     * a displayable primary image and a major and compatible brands set.
     * @param filename Filename including the path
     * @throws Exception
     */
    public void save(String filename)
            throws Exception
    {
        checkState();
        checkParameter(filename);
        saveNative(filename);
    }

    /**
     * Returns all objects of Item type in this HEIF instance.
     * @return List of all items
     * @throws Exception
     */
    public List<Item> getItems()
            throws Exception
    {
        checkState();
        int itemCount = getItemCountNative();
        List<Item> items = new ArrayList<>(itemCount);
        for (int index = 0; index < itemCount; index++)
        {
            items.add(getItemNative(index));
        }
        return items;
    }

    /**
     * Returns all objects of ImageItem type in this HEIF instance
     * @return List of all ImageItems
     * @throws Exception
     */
    public List<ImageItem> getImages()
            throws Exception
    {
        checkState();
        int itemCount = getImageCountNative();
        List<ImageItem> items = new ArrayList<>(itemCount);
        for (int index = 0; index < itemCount; index++)
        {
            items.add(getImageNative(index));
        }
        return items;
    }

    /**
     * Returns all master images of the HEIF instance.
     * Master images are all ImageItems except those that are either thumbnails or auxiliary images.
     * @return List of all master images
     * @throws Exception
     */
    public List<ImageItem> getMasterImages()
            throws Exception
    {
        checkState();
        int itemCount = getMasterImageCountNative();
        List<ImageItem> items = new ArrayList<>(itemCount);
        for (int index = 0; index < itemCount; index++)
        {
            items.add(getMasterImageNative(index));
        }
        return items;
    }

    /**
     * Returns all Items of the given type
     * @param typeFourCC FourCC code of the wanted type
     * @return List of all Items with the given type
     * @throws Exception
     */
    public List<Item> getItemsOfType(FourCC typeFourCC)
            throws Exception
    {
        checkState();
        checkParameter(typeFourCC);
        int itemCount = getItemsOfTypeCountNative(typeFourCC.toString());
        List<Item> items = new ArrayList<>(itemCount);
        for (int index = 0; index < itemCount; index++)
        {
            items.add(getItemOfTypeNative(typeFourCC.toString(), index));
        }
        return items;
    }

    /**
     * Returns the primary item of the HEIF instance. Every valid HEIF file should have a primary image.
     * The Primary image can be either a coded image or a derived image.
     * @return The Primary image for this HEIF file
     * @throws Exception
     */
    public ImageItem getPrimaryImage()
            throws Exception
    {
        checkState();
        return getPrimaryItemNative();
    }

    /**
     * Sets the primary image for the HEIF instance.
     * Will replace any existing primary image. Old primary image will still remain in the instance as a master image
     * @param primaryImage The ImageItem to be set as primary image
     * @throws Exception
     */
    public void setPrimaryImage(ImageItem primaryImage)
            throws Exception
    {
        checkState();
        checkParameterAllowNull(primaryImage);
        setPrimaryItemNative(primaryImage);
    }

    /**
     * Returns all compatible brands of the HEIF instance.
     * @return List containing the compatible brands as Strings
     * @throws Exception
     */
    public List<FourCC> getCompatibleBrands()
            throws Exception
    {
        checkState();
        int brandCount = getCompatibleBrandsCountNative();
        List<FourCC> brands = new ArrayList<>(brandCount);
        for (int index = 0; index < brandCount; index++)
        {
            brands.add(new FourCC(getCompatibleBrandNative(index), true));
        }
        return brands;
    }

    /**
     * Adds a compatible brand for the HEIF instance
     * @param brand The brand FourCC as a String
     * @throws Exception
     */
    public void addCompatibleBrand(FourCC brand)
            throws Exception
    {
        checkState();
        checkParameter(brand);
        addCompatibleBrandNative(brand.toString());
    }

    /**
     * Removes a compatible brand from the HEIF instance
     * @param brand The brand FourCC as a String
     * @throws Exception
     */
    public void removeComptibleBrand(FourCC brand)
            throws Exception
    {
        checkState();
        checkParameter(brand);
        removeCompatibleBrandNative(brand.toString());
    }

    /**
     * Returns the major brand of the HEIF instance
     * Every valid HEIF file must have the major brand set
     * @return The brand FourCC as a String
     * @throws Exception
     */
    public FourCC getMajorBrand()
            throws Exception
    {
        checkState();
        return new FourCC(getMajorBrandNative(), true);
    }

    /**
     * Sets the major brand for the HEIF instance.
     * Every valid HEIF file must have a major brand so one must be set before saving a HEIF instance to file
     * @param brand The brand FourCC as a String
     * @throws Exception
     */
    public void setMajorBrand(FourCC brand)
            throws Exception
    {
        checkState();
        checkParameter(brand);
        setMajorBrandNative(brand.toString());
    }

    /**
     * Returns all the ItemProperties in the HEIF instance
     * @return List of all the ItemProperties
     * @throws Exception
     */
    public List<ItemProperty> getItemProperties()
            throws Exception
    {
        checkState();
        int itemPropertyCount = getPropertyCountNative();
        List<ItemProperty> properties = new ArrayList<>(itemPropertyCount);
        for (int index = 0; index < itemPropertyCount; index++)
        {
            properties.add(getPropertyNative(index));
        }
        return properties;
    }

    /**
     * Verifies that the HEIF instance hasn't already been released and throws an exception if it has been.
     * @throws Exception
     */
    private void checkState()
            throws Exception
    {
        if (mNativeHandle == 0)
        {
            ErrorHandler.checkError(ErrorHandler.INVALID_HANDLE, "Object already deleted");
        }
    }

    private void checkParameter(Object parameter)
            throws Exception
    {
        if (parameter == null)
        {
            ErrorHandler.checkError(ErrorHandler.INVALID_HANDLE, "Parameter is null");
        }
        else if (parameter instanceof Base)
        {
            Base parameterAsBase = (Base) parameter;
            if (parameterAsBase.mNativeHandle == 0)
            {
                ErrorHandler.checkError(ErrorHandler.INVALID_HANDLE, "Object already deleted");
            }
            else if (parameterAsBase.getParentHEIF() != this)
            {
                ErrorHandler.checkError(ErrorHandler.WRONG_HEIF_INSTANCE, "Incorrect HEIF instance");
            }
        }
    }

    private void checkParameterAllowNull(Object parameter)
            throws Exception
    {
        if (parameter != null)
        {
            checkParameter(parameter);
        }
    }

    private native void createInstanceNative();

    private native void destroyInstanceNative();

    private native void loadNative(String filename);

    private native void saveNative(String filename);

    private native int getItemCountNative();

    private native Item getItemNative(int itemIndex);

    private native int getImageCountNative();

    private native ImageItem getImageNative(int itemIndex);

    private native int getMasterImageCountNative();

    private native ImageItem getMasterImageNative(int itemIndex);

    private native int getItemsOfTypeCountNative(String type);

    private native Item getItemOfTypeNative(String type, int itemIndex);

    private native ImageItem getPrimaryItemNative();

    private native void setPrimaryItemNative(ImageItem item);

    private native String getMajorBrandNative();

    private native void setMajorBrandNative(String brand);

    private native int getCompatibleBrandsCountNative();

    private native String getCompatibleBrandNative(int index);

    private native void addCompatibleBrandNative(String brand);

    private native void removeCompatibleBrandNative(String brand);

    private native int getPropertyCountNative();

    private native ItemProperty getPropertyNative(int index);

}
