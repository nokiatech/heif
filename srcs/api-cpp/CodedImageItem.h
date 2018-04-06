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
    class DecoderConfiguration
    {
        friend class Heif;
        friend class CodedImageItem;

    public:
        virtual ~DecoderConfiguration();

        /** Returns the id of the decoder config. */
        const HEIF::DecoderConfigId& getId() const;

        /** Returns the media format of the decoder configuration */
        const HEIF::MediaFormat& getMediaFormat() const;

        /** Returns configurations as an array  */
        const HEIF::Array<HEIF::DecoderSpecificInfo>& getConfig() const;

        /** Sets the configurations as an array
         * @param [in] aConfig: The configurations as an array */
        HEIF::ErrorCode setConfig(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig);

        /** Returns the whole configuration as a block
         * @param [out] data: Reference to where the data should be copied
         * @param [out] size: The size of the data copied */
        virtual void getConfig(uint8_t*& data, uint32_t& size) const = 0;

        /** Sets the decoder data as a block
         * @param [in] data: Pointer to the data
         * @param [in] size: The size of the data */
        virtual HEIF::ErrorCode setConfig(const uint8_t* data, uint32_t size) = 0;

        /** Sets the custom user data
         *  @param [in] aContext Pointer to the custom data */
        void setContext(const void* aContext);

        /** Returns the custom user data */
        const void* getContext() const;

    protected:
        HEIF::ErrorCode save(HEIF::Writer* aWriter);
        void link(CodedImageItem* aImage);
        void unlink(CodedImageItem* aImage);

        void setId(const HEIF::DecoderConfigId& aId);

        virtual HEIF::ErrorCode convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig,
                                                 uint8_t*& aData,
                                                 uint32_t& aSize) const                  = 0;
        virtual HEIF::ErrorCode convertFromRawData(const uint8_t* aData, uint32_t aSize) = 0;

        DecoderConfiguration(Heif* aHeif, const HEIF::MediaFormat& aFormat);
        Heif* mHeif;
        const void* mContext;
        HEIF::MediaFormat mFormat;
        HEIF::DecoderConfiguration mConfig;
        LinkArray<CodedImageItem*> mLinks;
        uint8_t* mBuffer;
        uint32_t mBufferSize;

    private:
        DecoderConfiguration& operator=(const DecoderConfiguration&) = delete;
        DecoderConfiguration(const DecoderConfiguration&)            = delete;
        DecoderConfiguration(DecoderConfiguration&&)                 = delete;
        DecoderConfiguration()                                       = delete;
    };

    // Actual image type item (hvc1,jpeg,avc1) ie. Coded Image (do we want/need classes for hevc/jpeg/avc encoded images
    // or is this enough?)
    class CodedImageItem : public HEIFPP::ImageItem
    {
    public:
        virtual ~CodedImageItem();

        /** Returns the DecoderConfiguration of the image */
        DecoderConfiguration* getDecoderConfiguration();
        const DecoderConfiguration* getDecoderConfiguration() const;

        /** Sets a decoder configuration for the image
         * @param [in] aConfig: The decoder configuration to be added. */
        void setDecoderConfiguration(DecoderConfiguration* aConfig);

        /** Sets the item data for the image
         * @param [in] aData: A pointer to the data.
         * @param [in] aLength: The amount of data. */
        void setItemData(const uint8_t* aData, uint64_t aLength);

        /** Returns the size of the item data */
        uint64_t getItemDataSize() const;

        /** Returns a pointer to the item data */
        const uint8_t* getItemData() const;

        /** Returns the decoder code type of the image. */
        const HEIF::FourCC& getDecoderCodeType() const;

        /** Returns the media format of the image. */
        const HEIF::MediaFormat& getMediaFormat() const;

        /** Returns the amount of base images associated with this coded image. */
        uint32_t getBaseImageCount() const;

        /** Get indexed base image associated with this coded image.
         * @param [in] aIndex: The index of the base image.*/
        ImageItem* getBaseImage(uint32_t aIndex);
        const ImageItem* getBaseImage(uint32_t aIndex) const;

        /** Adds new base image reference to aImage
         * @param [in] aImage: The image to be added. */
        void addBaseImage(ImageItem* aImage);

        /** Removes all base image references to aImage.
         * @param [in] aImage: The image to be removed. */
        void removeBaseImage(ImageItem* aImage);

        /** Removes the indexed base image reference.
         * @param [in] aIndex: Index to be removed. */
        void removeBaseImage(uint32_t aIndex);

        /** Sets the indexed base image reference.
         * @param [in] aIndex: The index where to set the image
         * @param [in] aImage: The image to be set. */
        void setBaseImage(uint32_t aIndex, ImageItem* aImage);

        /** Sets the base image reference of aOldImage to aNewImage
         * @param [in] aOldImage: Image to be removed.
         * @param [in] aNewImage: Image  to be added. */
        void setBaseImage(ImageItem* aOldImage, ImageItem* aNewImage);

        /** Reserves room for base images
         * @param [in] aCount: The amount of images to be reserved. */
        void reserveBaseImages(uint32_t aCount);

    protected:
        // serialization
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

        virtual void getBitstream(uint8_t*& aData, uint64_t& aSize) = 0;
        CodedImageItem(Heif* aHeif, const HEIF::FourCC& aType, const HEIF::MediaFormat& aFormat);
        HEIF::MediaFormat mFormat;
        DecoderConfiguration* mConfig;
        uint64_t mBufferSize;
        uint8_t* mBuffer;
        std::vector<ImageItem*> mBaseImages;
        bool mMandatoryConfiguration;


    private:
        CodedImageItem& operator=(const CodedImageItem&) = delete;
        CodedImageItem(const CodedImageItem&)            = delete;
        CodedImageItem(CodedImageItem&&)                 = delete;
        CodedImageItem()                                 = delete;
    };
}  // namespace HEIFPP
