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

public abstract class ImageItem extends Item
{

    /**
     * Constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @exception Exception Thrown if the parent HEIF instance is invalid
     */
    protected ImageItem(HEIF heif)
            throws Exception
    {
        super(heif);
    }

    /**
     * Protected constructor, abstract class so not called directly
     * @param heif The parent HEIF instance for the new object
     * @param nativeHandle A handle to the corresponding C++ object
     */
    protected ImageItem(HEIF heif, long nativeHandle)
    {
        super(heif, nativeHandle);
    }


    /**
     * Returns the size of the image
     * @return Size of the image
     * @throws Exception
     */
    public Size getSize()
            throws Exception
    {
        checkState();
        return new Size(getWidthNative(), getHeightNative());
    }

    /**
     * Tells if the image is set as hidden
     * @return If the image is hidden
     * @throws Exception
     */
    public boolean getHidden()
            throws Exception
    {
        checkState();
        return getHiddenNative();
    }

    /**
     * Sets the hidden flag for the image
     * @param hidden If the image should be hidden or not
     * @throws Exception
     */
    public void setHidden(boolean hidden)
            throws Exception
    {
        checkState();
        setHiddenNative(hidden);
    }

    /**
     * Returns the thumbnails of the image
     * @return List of ImageItems
     * @throws Exception
     */
    public List<ImageItem> getThumbnails()
            throws Exception
    {
        checkState();
        int thumbnailCount = getThumbnailCountNative();
        List<ImageItem> thumbnails = new ArrayList<>(thumbnailCount);
        for (int index = 0; index < thumbnailCount; index++)
        {
            thumbnails.add(getThumbnailNative(index));
        }
        return thumbnails;
    }

    /**
     * Adds a thumbnail for the Image
     * @param thumbnail ImageItem to be added as a thumbnail
     * @throws Exception
     */
    public void addThumbnail(ImageItem thumbnail)
            throws Exception
    {
        checkState();
        checkParameter(thumbnail);
        addThumbnailNative(thumbnail);
    }

    /**
     * Removes a thumbnail from the Image
     * @param thumbnail The thumbnail image to be removed
     * @throws Exception
     */
    public void removeThumbnail(ImageItem thumbnail)
            throws Exception
    {
        checkState();
        checkParameter(thumbnail);
        removeThumbnailNative(thumbnail);
    }

    /**
     * Returns a list of the auxiliary images for this image
     * @return List of ImageItems which are auxiliaries to this image
     * @throws Exception
     */
    public List<ImageItem> getAuxiliaries()
            throws Exception
    {
        checkState();
        int auxCount = getAuxiliaryCountNative();
        List<ImageItem> auxes = new ArrayList<>(auxCount);
        for (int index = 0; index < auxCount; index++)
        {
            auxes.add(getAuxiliaryNative(index));
        }
        return auxes;
    }

    /**
     * Adds an auxiliary image for the ImageItem
     * @param auxiliaryImage The auxiliary image to be added
     * @throws Exception
     */
    public void addAuxiliaryImage(ImageItem auxiliaryImage)
            throws Exception
    {
        checkState();
        checkParameter(auxiliaryImage);
        addAuxiliaryImageNative(auxiliaryImage);
    }

    /**
     * Removes an auxiliary image from the ImageItem
     * @param auxiliaryImage Auxiliary image to be removed
     * @throws Exception
     */
    public void removeAuxiliaryImage(ImageItem auxiliaryImage)
            throws Exception
    {
        checkState();
        checkParameter(auxiliaryImage);
        removeAuxiliaryImageNative(auxiliaryImage);
    }

    /**
     * Returns a list of TransformativeProperties.
     * TransformativeProperties must be applied in the order they are listed
     * The properties can be:
     * @see RotateProperty
     * @see MirrorProperty
     * @see CleanApertureProperty
     * @return A list of the transformative properties
     * @throws Exception
     */
    public List<TransformativeProperty> getTransformativeProperties()
            throws Exception
    {
        checkState();
        int count = transformativePropertyCountNative();
        List<TransformativeProperty> properties = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            properties.add(getTransformativePropertyNative(index));

        }
        return properties;
    }

    /**
     * Returns the PixelAspectRatioProperty of the ImageItem
     * @return The PixelAspectRatioProperty if it exists, null otherwise
     * @throws Exception
     */
    public PixelAspectRatioProperty getPixelAspectRatioProperty()
            throws Exception
    {
        checkState();
        return getPixelAspectRatioPropertyNative();
    }

    /**
     * Returns the ColourInformationProperty of the ImageItem
     * @return The ColourInformationProperty if it exists, null otherwise
     * @throws Exception
     */
    public ColourInformationProperty getColourInformationProperty()
            throws Exception
    {
        checkState();
        return getColourInformationPropertyNative();
    }

    /**
     * Returns the PixelInformationProperty of the ImageItem
     * @return The PixelInformationProperty if it exists, null otherwise
     * @throws Exception
     */
    public PixelInformationProperty getPixelInformationProperty()
            throws Exception
    {
        checkState();
        return getPixelInformationPropertyNative();
    }

    /**
     * Returns the RelativeLocationProperty of the ImageItem
     * @return The RelativeLocationProperty if it exists, null otherwise
     * @throws Exception
     */
    public RelativeLocationProperty getRelativeLocationProperty()
            throws Exception
    {
        checkState();
        return getRelativeLocationPropertyNative();
    }

    /**
     * Returns the AuxiliaryProperty of the ImageItem
     * @return The AuxiliaryProperty if it exists, null otherwise
     * @throws Exception
     */
    public AuxiliaryProperty getAuxiliaryProperty()
            throws Exception
    {
        checkState();
        return getAuxiliaryPropertyNative();
    }

    /**
     * Adds a property for the ImageItem
     * @param property Property to be added
     * @param essential
     * @throws Exception
     */
    public void addProperty(ItemProperty property, boolean essential)
            throws Exception
    {
        checkState();
        checkParameter(property);
        addPropertyNative(property, essential);
    }

    /**
     * Removes a property from the ImageItem
     * @param property The property to be removed
     * @throws Exception
     */
    public void removeProperty(ItemProperty property)
            throws Exception
    {
        checkState();
        checkParameter(property);
        removePropertyNative(property);
    }

    /**
     * Returns the MetaItems of the ImageItem
     * @return List of the metadata items
     * @throws Exception
     */
    public List<MetaItem> getMetadatas()
            throws Exception
    {
        checkState();
        int count = getMetadataCountNative();
        List<MetaItem> items = new ArrayList<>(count);
        for (int index = 0; index < count; index++)
        {
            items.add(getMetadataNative(index));
        }
        return items;
    }

    /**
     * Adds a metadata item for the ImageItem
     * @param item Item to be added
     * @throws Exception
     */
    public void addMetadata(MetaItem item)
            throws Exception
    {
        checkState();
        checkParameter(item);
        addMetadataNative(item);
    }

    /**
     * Removes a metadata item from the ImageItem
     * @param item Item to be removed
     * @throws Exception
     */
    public void removeMetadata(MetaItem item)
            throws Exception
    {
        checkState();
        removeMetadataNative(item);
    }

    /**
     * Sets the size of the image
     * @param size The size of the image
     * @throws Exception
     */
    protected void setSize(Size size)
            throws Exception
    {
        checkState();
        checkParameter(size);
        setSizeNative(size.width, size.height);
    }

    native private int getWidthNative();

    native private int getHeightNative();

    native private void setSizeNative(int width, int height);

    native private boolean getHiddenNative();

    native private void setHiddenNative(boolean hidden);

    native private int getThumbnailCountNative();

    native private ImageItem getThumbnailNative(int index);

    native private void addThumbnailNative(ImageItem image);

    native private void removeThumbnailNative(ImageItem image);

    native private int getAuxiliaryCountNative();

    native private ImageItem getAuxiliaryNative(int index);

    native private void addAuxiliaryImageNative(ImageItem auxiliaryImage);

    native private void removeAuxiliaryImageNative(ImageItem auxliaryImage);

    native private PixelAspectRatioProperty getPixelAspectRatioPropertyNative();

    native private ColourInformationProperty getColourInformationPropertyNative();

    native private PixelInformationProperty getPixelInformationPropertyNative();

    native private RelativeLocationProperty getRelativeLocationPropertyNative();

    native private AuxiliaryProperty getAuxiliaryPropertyNative();

    native private int transformativePropertyCountNative();

    native private TransformativeProperty getTransformativePropertyNative(int index);

    native private void addPropertyNative(ItemProperty property, boolean essential);

    native private void removePropertyNative(ItemProperty property);

    native private int getMetadataCountNative();

    native private MetaItem getMetadataNative(int index);

    native private void addMetadataNative(MetaItem item);

    native private void removeMetadataNative(MetaItem item);


}
