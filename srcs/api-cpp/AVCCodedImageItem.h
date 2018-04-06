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

#include <CodedImageItem.h>

namespace HEIFPP
{
    class AVCDecoderConfiguration : public HEIFPP::DecoderConfiguration
    {
    public:
        AVCDecoderConfiguration(Heif* aHeif);
        virtual ~AVCDecoderConfiguration() = default;

        /** Returns the whole configuration as a block
         * @param [out] data: Reference to where the data should be copied
         * @param [out] size: The size of the data copied */
        void getConfig(uint8_t*& data, uint32_t& size) const;

        /** Sets the decoder data as a block
         * @param [in] data: Pointer to the data
         * @param [in] size: The size of the data */
        HEIF::ErrorCode setConfig(const uint8_t* data, uint32_t size);

    protected:
        HEIF::ErrorCode convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig,
                                         uint8_t*& aData,
                                         uint32_t& aSize) const;
        HEIF::ErrorCode convertFromRawData(const uint8_t* aData, uint32_t aSize);

    private:
        AVCDecoderConfiguration& operator=(const AVCDecoderConfiguration&) = delete;
        AVCDecoderConfiguration(const AVCDecoderConfiguration&)            = delete;
        AVCDecoderConfiguration(AVCDecoderConfiguration&&)                 = delete;
        AVCDecoderConfiguration()                                          = delete;
    };

    class AVCCodedImageItem : public HEIFPP::CodedImageItem
    {
    public:
        AVCCodedImageItem(Heif* aHeif);
        virtual ~AVCCodedImageItem() = default;

    protected:
        // serialization
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

        void getBitstream(uint8_t*& aData, uint64_t& aSize);

    private:
        AVCCodedImageItem& operator=(const AVCCodedImageItem&) = delete;
        AVCCodedImageItem(const AVCCodedImageItem&)            = delete;
        AVCCodedImageItem(AVCCodedImageItem&&)                 = delete;
        AVCCodedImageItem()                                    = delete;
    };
}  // namespace HEIFPP
