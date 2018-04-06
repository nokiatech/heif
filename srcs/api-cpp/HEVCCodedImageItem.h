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
    class HEVCDecoderConfiguration : public HEIFPP::DecoderConfiguration
    {
    public:
        HEVCDecoderConfiguration(Heif* aHeif);
        virtual ~HEVCDecoderConfiguration() = default;
        /** Returns the config data
         * @param [out] data: Reference to the data pointer where the data is stored.
         * @param [out] size: Reference to the variable where the size will be stored. */
        void getConfig(uint8_t*& data, uint32_t& size) const;

        /** Sets the config data
         * @param [in] data: Pointer to the data.
         * @param [in] size: Size of the data. */
        HEIF::ErrorCode setConfig(const uint8_t* data, uint32_t size);

    protected:
        HEIF::ErrorCode convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig,
                                         uint8_t*& aData,
                                         uint32_t& aSize) const;
        HEIF::ErrorCode convertFromRawData(const uint8_t* aData, uint32_t aSize);

    private:
        HEVCDecoderConfiguration& operator=(const HEVCDecoderConfiguration&) = delete;
        HEVCDecoderConfiguration(const HEVCDecoderConfiguration&)            = delete;
        HEVCDecoderConfiguration(HEVCDecoderConfiguration&&)                 = delete;
        HEVCDecoderConfiguration()                                           = delete;
    };

    class HEVCCodedImageItem : public HEIFPP::CodedImageItem
    {
    public:
        HEVCCodedImageItem(Heif* aHeif);
        virtual ~HEVCCodedImageItem() = default;
        // serialization
    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

        void getBitstream(uint8_t*& aData, uint64_t& aSize);

    private:
        HEVCCodedImageItem& operator=(const HEVCCodedImageItem&) = delete;
        HEVCCodedImageItem(const HEVCCodedImageItem&)            = delete;
        HEVCCodedImageItem(HEVCCodedImageItem&&)                 = delete;
        HEVCCodedImageItem()                                     = delete;
    };
}  // namespace HEIFPP
