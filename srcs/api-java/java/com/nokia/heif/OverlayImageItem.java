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

/**
 * A DerivedImageItem which contains several ImageItems overlayed on top of each other.
 */
public final class OverlayImageItem extends DerivedImageItem
{

    /**
     * Areas not covered by an ImageItem will be filled with the given background color
     * Value range for the colors is from 0 to 65535
     * Defaults to opaque black
     */
    public static class BackgroundColour
    {
        public static final int MAX_VALUE = 65535;

        public BackgroundColour()
        {
            this.r = 0;
            this.g = 0;
            this.b = 0;
            this.a = MAX_VALUE;
        }

        public BackgroundColour(int r, int g, int b, int a)
        {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = a;
        }

        @Override
        public boolean equals(Object obj)
        {
            if (obj instanceof BackgroundColour)
            {
                BackgroundColour compareTo = (BackgroundColour) obj;
                return compareTo.r == this.r
                        && compareTo.b == this.b
                        && compareTo.g == this.g
                        && compareTo.a == this.a;

            }
            else
            {
                return false;
            }
        }

        public final int r;
        public final int g;
        public final int b;
        public final int a;
    }


    /**
     * Holds the ImageItem which belongs to the OverlayImageItem and its position within the OverlayImageItem
     */
    public class OverlayedImage
    {
        public OverlayedImage(ImageItem image, int horizontalOffset, int verticalOffset)
        {
            this.image = image;
            this.horizontalOffset = horizontalOffset;
            this.verticalOffset = verticalOffset;
        }

        /** ImageItem */
        public final ImageItem image;

        /** Horizontal offset in pixels */
        public final int horizontalOffset;

        /** Vertical offset in pixels */
        public final int verticalOffset;
    }

    /**
     * Creates an OverlayImageItem to the given HEIF instance
     * @param heif The parent HEIF instance for the new object
     * @param size The display size of the overlay image
     * @throws Exception Thrown if the parent HEIF instance is invalid
     */
    public OverlayImageItem(HEIF heif, Size size)
            throws Exception
    {
        super(heif);
        mNativeHandle = createContextNative(heif);
        setSize(size);
    }

    /**
     * Protected constructor, should be called only by the HEIF instance
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected OverlayImageItem(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }

    /**
     * Returns a list of images that are part of the OverlayImageItem
     * The drawing order for the images should be the same as the order of list
     * @return List containing OverlayedImages
     * @throws Exception
     */
    public List<OverlayedImage> getOverlayedImages()
            throws Exception
    {
        checkState();
        int imageCount = imageCountNative();
        List<OverlayedImage> images = new ArrayList<>(imageCount);
        for (int index = 0; index < imageCount; index++)
        {
            images.add(getImageNative(index));
        }
        return images;
    }

    /**
     * Sets a background colour for the OverlayImageItem
     * @param colour The background colour
     * @throws Exception
     */
    public void setBackgroundColour(BackgroundColour colour)
            throws Exception
    {
        checkState();
        setBackgroundColourNative(
                clamptoRange(colour.r),
                clamptoRange(colour.g),
                clamptoRange(colour.b),
                clamptoRange(colour.a));
    }

    /**
     * Sets a background colour for the OverlayImageItem
     * @param r Red channel ranging from 0 to 65535
     * @param g Green channel ranging from 0 to 65535
     * @param b Blue channel ranging from 0 to 65535
     * @param a Alpha channel ranging from 0 to 65535
     * @throws Exception
     */
    public void setBackgroundColour(int r, int g, int b, int a)
            throws Exception
    {
        checkState();
        setBackgroundColour(new BackgroundColour(clamptoRange(r),
                                                 clamptoRange(g),
                                                 clamptoRange(b),
                                                 clamptoRange(a)));
    }

    /**
     * The Background colour of the OverlayImageItem
     * @return The background colour
     * @throws Exception
     */
    public OverlayImageItem.BackgroundColour getBackgroundColour()
            throws Exception
    {
        checkState();
        return getBackgroundColourNative();
    }

    /**
     * Adds an ImageItem for the OverlayImageItem.
     * The added image goes to the end of the list, meaning it will be drawn last
     * @param image Image item to be added.
     * @param horizontalOffset The horizontal offset of the image inside the OverlayImageItem in pixels
     * @param verticalOffset The vertical offset of the image inside the OverlayImageItem in pixels
     * @throws Exception
     */
    public void addImage(ImageItem image, int horizontalOffset, int verticalOffset)
            throws Exception
    {
        checkState();
        checkParameter(image);
        addImageNative(image, horizontalOffset, verticalOffset);
    }

    /**
     * Sets an image to the given index in the OverlayImageItem.
     * @param orderIndex Index in the list, defines the drawing order of the images
     * @param image The image to be added
     * @param horizontalOffset The horizontal offset of the image inside the OverlayImageItem in pixels
     * @param verticalOffset The vertical offset of the image inside the OverlayImageItem in pixels
     * @throws Exception
     */
    public void setImage(int orderIndex, ImageItem image, int horizontalOffset, int verticalOffset)
            throws Exception
    {
        checkState();
        checkParameter(image);
        setImageNative(orderIndex, image, horizontalOffset, verticalOffset);
    }

    /**
     * Removes an image from OverlayImageItem with the given index.
     * @param index Index of the image to be removed
     * @throws Exception
     */
    public void removeImage(int index)
            throws Exception
    {
        checkState();
        removeImageByIndexNative(index);
    }

    /**
     * Removes the image from the OverlayImageItem.
     * All references of the image are removed from the OverlayImageItem so if it has been added multiple times to
     * different layers, all of them will be removed
     * @param image The image to be removed
     * @throws Exception
     */
    public void removeImage(ImageItem image)
            throws Exception
    {
        checkState();
        checkParameter(image);
        removeImageNative(image);
    }

    private OverlayImageItem.BackgroundColour createBackgroundColour(
            int r, int g, int b, int a)
    {
        return new BackgroundColour(r, g, b, a);
    }

    private int clamptoRange(int value)
    {
        return Math.max(0, Math.min(BackgroundColour.MAX_VALUE, value));
    }


    private OverlayImageItem.OverlayedImage createOverlayedImage(ImageItem image,
                                                                 int horizontalOffset,
                                                                 int verticalOffset)
    {
        return new OverlayedImage(image, horizontalOffset, verticalOffset);
    }

    private native long createContextNative(HEIF heif);

    private native int imageCountNative();

    private native OverlayedImage getImageNative(int index);

    private native BackgroundColour getBackgroundColourNative();

    private native void setBackgroundColourNative(int r, int g, int b, int a);

    private native void addImageNative(ImageItem image, int horizontalOffset, int verticalOffset);

    private native void setImageNative(int index, ImageItem image, int horizontalOffset, int verticalOffset);

    private native void removeImageByIndexNative(int index);

    private native void removeImageNative(ImageItem image);


}
