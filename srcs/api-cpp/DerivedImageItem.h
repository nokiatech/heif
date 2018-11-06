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

#include <ImageItem.h>

namespace HEIFPP
{
    class DerivedImageItem : public HEIFPP::ImageItem
    {
    public:
        ~DerivedImageItem();

        /** Returns count of source images associated with this derived image.
         * @return count of source images*/
        std::uint32_t getSourceImageCount() const;

        /** Returns indexed source image associated with this derived image.
         * @param [in] aIndex: Index of the image
         * @return ImageItem*/
        ImageItem* getSourceImage(std::uint32_t aIndex);
        const ImageItem* getSourceImage(std::uint32_t aIndex) const;

        /** Removes the image from the derived image
         * @param [in] aImage: Image to be removed */
        virtual Result removeImage(ImageItem* aImage) = 0;

    protected:
        void reserveSourceImages(std::uint32_t cnt);

        // Replaces specified sourceimage with aImage
        void setSourceImage(std::uint32_t aId, ImageItem* aImage);
        // Replaces all instances of aOldImage with aNewImage
        // returns false if no instance of aOldImage is found.
        bool setSourceImage(ImageItem* aOldImage, ImageItem* aNewImage);

        // Adds a new image to the end of the list
        void addSourceImage(ImageItem* aImage);

        // Removes all instances of aImage from sources.
        bool removeSourceImage(ImageItem* aImage);
        // Removes image specified by index.
        void removeSourceImage(std::uint32_t aId);

        std::vector<ImageItem*> mSourceImages;

        DerivedImageItem(Heif* aHeif, const HEIF::FourCC&);

        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId) override;

    private:
        DerivedImageItem& operator=(const DerivedImageItem&) = delete;
        DerivedImageItem& operator=(DerivedImageItem&&)      = delete;
        DerivedImageItem(const DerivedImageItem&)            = delete;
        DerivedImageItem(DerivedImageItem&&)                 = delete;
        DerivedImageItem()                                   = delete;
    };

}  // namespace HEIFPP
