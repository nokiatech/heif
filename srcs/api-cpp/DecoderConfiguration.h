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

#include <Heif.h>

namespace HEIFPP
{
    class DecoderConfig
    {
        friend class Sample;
        friend class Heif;
        friend class CodedImageItem;

    public:
        virtual ~DecoderConfig();

        /** Returns the id of the decoder config. */
        const HEIF::DecoderConfigId& getId() const;

        /** Returns the media format of the decoder configuration */
        HEIF::MediaFormat getMediaFormat() const;

        /** Returns the media type (fourcc) of the decoder configuration */
        const HEIF::FourCC& getMediaType() const;

        /** Returns configurations as an array  */
        const HEIF::Array<HEIF::DecoderSpecificInfo>& getConfig() const;

        /** Sets the configurations as an array
         * @param [in] aConfig: The configurations as an array */
        HEIF::ErrorCode setConfig(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig);

        /** Returns the whole configuration as a block
         * @param [out] data: Reference to where the data should be copied
         * @param [out] size: The size of the data copied */
        virtual void getConfig(std::uint8_t*& data, std::uint32_t& size) const = 0;

        /** Sets the decoder data as a block
         * @param [in] data: Pointer to the data
         * @param [in] size: The size of the data */
        virtual HEIF::ErrorCode setConfig(const std::uint8_t* data, std::uint32_t size) = 0;

        /** Sets the custom user data
         *  @param [in] aContext Pointer to the custom data */
        void setContext(const void* aContext);

        /** Returns the custom user data */
        const void* getContext() const;

    protected:
        HEIF::ErrorCode save(HEIF::Writer* aWriter);
        void link(CodedImageItem* aImage);
        void unlink(CodedImageItem* aImage);
        void link(Sample* aImage);
        void unlink(Sample* aImage);

        void setId(const HEIF::DecoderConfigId& aId);

        virtual HEIF::ErrorCode convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig,
                                                 std::uint8_t*& aData,
                                                 std::uint32_t& aSize) const                       = 0;
        virtual HEIF::ErrorCode convertFromRawData(const std::uint8_t* aData, std::uint32_t aSize) = 0;

        DecoderConfig(Heif* aHeif, const HEIF::FourCC& aType);
        Heif* mHeif;
        const void* mContext;
        HEIF::FourCC mType;
        HEIF::DecoderConfiguration mConfig;
        LinkArray<CodedImageItem*> mLinks;
        LinkArray<Sample*> mSampleLinks;
        std::uint8_t* mBuffer;
        std::uint32_t mBufferSize;

    private:
        DecoderConfig& operator=(const DecoderConfig&) = delete;
        DecoderConfig& operator=(DecoderConfig&&)      = delete;
        DecoderConfig(const DecoderConfig&)            = delete;
        DecoderConfig(DecoderConfig&&)                 = delete;
        DecoderConfig()                                       = delete;
    };

}  // namespace HEIFPP
