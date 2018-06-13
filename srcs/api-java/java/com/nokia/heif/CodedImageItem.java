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

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * ImageItem which contains an encoded image in it.
 * Parent class to codec specific CodedImageItems
 */
public abstract class CodedImageItem extends ImageItem
{

    /**
     * Constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @exception Exception Thrown if the parent HEIF instance is invalid
     */
    protected CodedImageItem(HEIF heif)
            throws Exception
    {
        super(heif);
    }

    /**
     * Protected constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected CodedImageItem(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns the decoder code type of the image
     * @return Decoder type as a String
     * @throws Exception
     */
    public FourCC getDecoderCodeType()
            throws Exception
    {
        checkState();
        return new FourCC(getDecoderCodeTypeNative(),true);
    }

    /**
     * Returns the image data as a byte array.
     * Note that this does a memory copy from the ByteBuffer so using the ByteBuffer getter is more optimized
     * @return A ByteArray containing the image data
     * @throws Exception
     */
    public byte[] getItemDataAsArray()
            throws Exception
    {
        checkState();
        ByteBuffer output = getItemData();
        byte[] data = new byte[output.remaining()];
        output.get(data);
        return data;
    }

    /**
     * Returns the image data as a ByteBuffer. The buffer is a DirectBuffer with the data being owned by the native
     * object so the CodedImageItem or its parent HEIF instance is destroyed, the ByteBuffer is invalidated
     * @return The image data in a ByteBuffer
     * @throws Exception
     */
    public ByteBuffer getItemData()
            throws Exception
    {
        checkState();
        ByteBuffer output = getItemDataNative();
        return output.asReadOnlyBuffer();
    }

    /**
     * Sets the image data for the CodedImageItem. The byte arrays contents are copied in the native code so
     * the byte array can be released after calling the setter.
     * @param data ImageData as a byte array
     * @throws Exception
     */
    public void setItemData(byte[] data)
            throws Exception
    {
        checkState();
        checkParameter(data);
        setItemDataNative(data);
    }

    /**
     * Sets the DecoderConfig for the CodedImageItem
     * The DecoderConfig's type must match the type of the CodedImageItem
     * @param config The decoder config
     * @throws Exception
     */
    public void setDecoderConfig(DecoderConfig config)
            throws Exception
    {
        checkState();
        checkParameter(config);
        if (!checkDecoderConfigType(config))
        {
            throw new Exception(ErrorHandler.INVALID_PARAMETER, "Incorrect decoder config type");
        }
        setDecoderConfigNative(config);
    }

    /**
     * Returns the DecoderConfig of the CodedImageItem.
     * @return The DecoderConfig
     * @throws Exception
     */
    public DecoderConfig getDecoderConfig()
            throws Exception
    {
        checkState();
        return getDecoderConfigNative();
    }

    /**
     * Returns a list of the base images for this coded image.
     * A base image is an image that was used to construct the CodedImageItem.
     * @return
     * @throws Exception
     */
    public List<ImageItem> getBaseImages()
            throws Exception
    {
        checkState();
        int imageCount = getBaseImageCountNative();
        List<ImageItem> images = new ArrayList<>(imageCount);
        for (int index = 0; index < imageCount; index++)
        {
            images.add(getBaseImageNative(index));
        }
        return images;
    }

    /**
     * Adds a base image for the CodedImageItem
     * @param image New base image
     * @throws Exception
     */
    public void addBaseImage(ImageItem image)
            throws Exception
    {
        checkState();
        checkParameter(image);
        addBaseImageNative(image);
    }

    /**
     * Removes the image from the list of base images
     * @param image Image to be removed
     * @throws Exception
     */
    public void removeBaseImage(ImageItem image)
            throws Exception
    {
        checkState();
        checkParameter(image);
        removeBaseImageNative(image);
    }

    /**
     * Removes a base image using an index
     * @param index Index of the image to be removed
     * @throws Exception
     */
    public void removeBaseImageByIndex(int index)
            throws Exception
    {
        checkState();
        removeBaseImageByIndexNative(index);
    }

    /**
     * Adds a base image to a certain index
     * @param index index
     * @param image Image to be added
     * @throws Exception
     */
    public void setBaseImageByIndex(int index, ImageItem image)
            throws Exception
    {
        checkState();
        checkParameter(image);
        setBaseImageByIndexNative(index, image);
    }

    /**
     * Replaces an existing base image with another one
     * @param oldImage Image to be replaced
     * @param newImage Image to be added
     * @throws Exception
     */
    public void setBaseImage(ImageItem oldImage, ImageItem newImage)
            throws Exception
    {
        checkState();
        checkParameter(oldImage);
        checkParameter(newImage);
        setBaseImageNative(oldImage, newImage);
    }

    abstract protected boolean checkDecoderConfigType(DecoderConfig config);

    private native String getDecoderCodeTypeNative();

    private native ByteBuffer getItemDataNative();

    private native void setItemDataNative(byte[] data);

    private native void setDecoderConfigNative(DecoderConfig config);

    private native DecoderConfig getDecoderConfigNative();


    private native int getBaseImageCountNative();

    private native ImageItem getBaseImageNative(int index);

    private native void addBaseImageNative(ImageItem image);

    private native void removeBaseImageNative(ImageItem image);

    private native void removeBaseImageByIndexNative(int index);

    private native void setBaseImageByIndexNative(int index, ImageItem image);

    private native void setBaseImageNative(ImageItem oldImage, ImageItem newImage);

    private native void reserveBaseImagesNative(int count);
}
