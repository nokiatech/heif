/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#pragma once

#include <Item.h>

namespace HEIFPP
{
    /** @brief Image item abstraction*/
    class ImageItem : public HEIFPP::Item
    {
        friend class IdentityImageItem;
        friend class GridImageItem;
        friend class OverlayImageItem;
        friend class CodedImageItem;

    public:
        ~ImageItem();

        /** Returns if the image is a derived image. */
        bool isDerivedImage() const;

        /** Returns if the image is a coded image. */
        bool isCodedImage() const;

        /** Returns if the image is a prederived image. */
        bool isPreDerivedImage() const;

        /** Returns if the image is a primary image. */
        bool isPrimaryImage() const;

        /** Returns if the image is a cover image. */
        bool isCoverImage() const;

        /** Returns if the image is hidden. */
        bool isHidden() const;

        /** Returns if the image is a thumbnail. */
        bool isThumbnailImage() const;

        /** Returns if the image is an auxiliary image. */
        bool isAuxiliaryImage() const;

        /** Returns if the image is a master image. */
        bool isMasterImage() const;

        /** Returns if the image is a source image. */
        bool isSourceImage() const;

        /** Returns if the image is a base image. */
        bool isBaseImage() const;

        /** Returns if the image has linked thumbnails. */
        bool HasLinkedThumbnails() const;

        /** Returns if the image has a linked auxiliary image. */
        bool hasLinkedAuxiliaryImage() const;

        /** Returns if the image has a linked derived image. */
        bool hasLinkedDerivedImage() const;

        /** Returns if the image has a linked precomputed derived image. */
        bool hasLinkedPreComputedDerivedImage() const;

        /** Returns if the image has a linked metadata. */
        bool hasLinkedMetadata() const;

        /** Sets the hidden property on the image
         * @param [in] aHidden: The new value for the hidden property. */
        void setHidden(bool aHidden);

        /** Returns the count of transformative properties (0-3) */
        std::uint32_t transformativePropertyCount() const;

        /** Returns a transformative property with the given index
         * @param [in] id: The index for the item */
        TransformativeProperty* getTransformativeProperty(uint32_t id);

        /** Returns a transformative property with the given index
         * @param [in] id: The index for the item */
        const TransformativeProperty* getTransformativeProperty(uint32_t id) const;

        /** Returns the width of the image in pixels */
        std::uint32_t width() const;

        /** Returns the height of the image in pixels */
        std::uint32_t height() const;

        /** Sets the size of the image
         * @param [in] aWidth: The width of the image in pixels.
         * @param [in] aHeight: The height of the image in pixels. */
        void setSize(uint32_t aWidth, std::uint32_t aHeight);

        /** Returns the Pixel Aspect Ratio property if it exists */
        PixelAspectRatioProperty* pixelAspectRatio();
        const PixelAspectRatioProperty* pixelAspectRatio() const;

        /** Returns the Colour Information property if it exists */
        ColourInformationProperty* colourInformation();
        const ColourInformationProperty* colourInformation() const;

        /** Returns the Pixel Information property if it exists */
        PixelInformationProperty* pixelInformation();
        const PixelInformationProperty* pixelInformation() const;

        /** Returns the Relative Location property if it exists */
        RelativeLocationProperty* relativeLocation();
        const RelativeLocationProperty* relativeLocation() const;

        /** Returns the Auxiliary property if it exists */
        AuxiliaryProperty* aux();
        const AuxiliaryProperty* aux() const;

        /** Returns the amount of metadata items */
        std::uint32_t getMetadataCount() const;

        /** Gets a Meta item with the given index
         * @param [in] aIndex: The index of the meta item. */
        MetaItem* getMetadata(uint32_t aIndex);
        const MetaItem* getMetadata(uint32_t aIndex) const;

        /** Adds a Metadata item to the image
         * @param [in] aItem: MetaItem to be added */
        void addMetadata(MetaItem* aItem);

        /** Removes a Metadata item from the image
         * @param [in] aItem: MetaItem to be added */
        void removeMetadata(MetaItem* aItem);

        /** Return count of thumbnails associated with this image. */
        std::uint32_t getThumbnailCount() const;

        /** Gets the thumbnail with the given index.
         * @param [in] aIndex: Index of the thumbnail. */
        ImageItem* getThumbnail(uint32_t aIndex);
        const ImageItem* getThumbnail(uint32_t) const;

        /** Adds a thumbnail for the image
         * @param [in] aThumbnail: Thumbnail to be added. */
        void addThumbnail(ImageItem* aThumbnail);

        /** Removes a thumbnail from the image
         * @param [in] aThumbnail: Thumbnail to be removed. */
        void removeThumbnail(ImageItem* aThumbnail);

        /** Return count of auxiliary images associated with this image. */
        std::uint32_t getAuxCount() const;

        /** Gets an auxiliary image from the image.
         * @param [in] aIndex: Index of the auxiliary image. */
        ImageItem* getAux(uint32_t aIndex);
        const ImageItem* getAux(uint32_t aIndex) const;

        /** Adds an auxiliary image for the image
         * @param [in] aImage: The auxiliary image to be added. */
        void addAuxImage(ImageItem* aImage);

        /** Removes an auxiliary image for the image
         * @param [in] aImage: The auxiliary image to be removed. */
        void removeAuxImage(ImageItem* aImage);

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

        ImageItem(Heif* aHeif, const HEIF::FourCC& aType, bool aDerived, bool aCoded);

        void addSourceLink(ImageItem* aSource, DerivedImageItem* aTarget);
        bool removeSourceLink(ImageItem* aSource, DerivedImageItem* aTarget);

        void addBaseLink(ImageItem* aSource, CodedImageItem* aTarget);
        bool removeBaseLink(ImageItem* aSource, CodedImageItem* aTarget);

    private:
        ItemProperty* getFirstPropertyOfType(HEIF::ItemPropertyType aType) const;
        // Reference counted links.
        LinkArray<ImageItem*> mIsThumbnailTo;
        LinkArray<ImageItem*> mIsAuxiliaryTo;
        LinkArray<CodedImageItem*> mIsBaseImageTo;
        LinkArray<DerivedImageItem*> mIsSourceImageTo;
        bool mIsHidden;
        bool mIsDerived;
        bool mIsCoded;

        std::vector<ImageItem*> mThumbnailImages;
        std::vector<ImageItem*> mAuxImages;
        std::vector<MetaItem*> mMetaItems;

        std::uint32_t mWidth;
        std::uint32_t mHeight;


        ImageItem& operator=(const ImageItem&) = delete;
        ImageItem& operator=(ImageItem&&) = delete;
        ImageItem(const ImageItem&)       = delete;
        ImageItem(ImageItem&&)            = delete;
        ImageItem()                       = delete;
    };
}  // namespace HEIFPP
