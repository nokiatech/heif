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
    class DecoderConfig;
    class CodedImageItem : public HEIFPP::ImageItem
    {
    public:
        ~CodedImageItem();

        /** Returns the DecoderConfiguration of the image */
        DecoderConfig* getDecoderConfiguration();
        const DecoderConfig* getDecoderConfiguration() const;

        /** Sets a decoder configuration for the image
         * @param [in] aConfig: The decoder configuration to be added. */
        Result setDecoderConfiguration(DecoderConfig* aConfig);

        /** Sets the item data for the image
         * @param [in] aData: A pointer to the data.
         * @param [in] aLength: The amount of data. */
        void setItemData(const std::uint8_t* aData, std::uint64_t aLength);

        /** Returns the size of the item data */
        std::uint64_t getItemDataSize() const;

        /** Returns a pointer to the item data */
        const std::uint8_t* getItemData();

        /** Returns the decoder code type of the image. */
        const HEIF::FourCC& getDecoderCodeType() const;

        /** Returns the media format of the image. */
        HEIF::MediaFormat getMediaFormat() const;

        /** Returns the amount of base images associated with this coded image. */
        std::uint32_t getBaseImageCount() const;

        /** Get indexed base image associated with this coded image.
         * @param [in] aIndex: The index of the base image.*/
        ImageItem* getBaseImage(std::uint32_t aIndex);
        const ImageItem* getBaseImage(std::uint32_t aIndex) const;

        /** Adds new base image reference to aImage
         * @param [in] aImage: The image to be added. */
        void addBaseImage(ImageItem* aImage);

        /** Removes all base image references to aImage.
         * @param [in] aImage: The image to be removed. */
        void removeBaseImage(ImageItem* aImage);

        /** Removes the indexed base image reference.
         * @param [in] aIndex: Index to be removed. */
        void removeBaseImage(std::uint32_t aIndex);

        /** Sets the indexed base image reference.
         * @param [in] aIndex: The index where to set the image
         * @param [in] aImage: The image to be set. */
        void setBaseImage(std::uint32_t aIndex, ImageItem* aImage);

        /** Sets the base image reference of aOldImage to aNewImage
         * @param [in] aOldImage: Image to be removed.
         * @param [in] aNewImage: Image  to be added. */
        void setBaseImage(ImageItem* aOldImage, ImageItem* aNewImage);

        /** Reserves room for base images
         * @param [in] aCount: The amount of images to be reserved. */
        void reserveBaseImages(std::uint32_t aCount);

    protected:
        // serialization
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

        virtual bool getBitstream(std::uint8_t*& aData, std::uint64_t& aSize) = 0;
        CodedImageItem(Heif* aHeif, const HEIF::FourCC& aType, const HEIF::MediaFormat& aFormat);
        HEIF::MediaFormat mFormat;
        DecoderConfig* mConfig;
        std::uint64_t mBufferSize;
        std::uint8_t* mBuffer;
        std::vector<ImageItem*> mBaseImages;
        bool mMandatoryConfiguration;

    private:
        HEIF::ErrorCode loadItemData();

    private:
        CodedImageItem& operator=(const CodedImageItem&) = delete;
        CodedImageItem& operator=(CodedImageItem&&) = delete;
        CodedImageItem(const CodedImageItem&)       = delete;
        CodedImageItem(CodedImageItem&&)            = delete;
        CodedImageItem()                            = delete;
    };
}  // namespace HEIFPP
