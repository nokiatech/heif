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
#if 0
    class JPEGDecoderConfiguration : public HEIFPP::DecoderConfiguration
    {
    public:
        JPEGDecoderConfiguration(Heif* aHeif);
        virtual ~JPEGDecoderConfiguration() = default;
        virtual void getConfig(uint8_t*& data, uint32_t& size) const;
        virtual HEIF::ErrorCode  setConfig(const uint8_t* data, uint32_t size);
    protected:
        HEIF::ErrorCode convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig, uint8_t*& aData, uint32_t& aSize) const;
        HEIF::ErrorCode convertFromRawData(const uint8_t* aData, uint32_t aSize);       
    private:
        JPEGDecoderConfiguration & operator=(const JPEGDecoderConfiguration&) = delete;
        JPEGDecoderConfiguration(const JPEGDecoderConfiguration&) = delete;
        JPEGDecoderConfiguration(JPEGDecoderConfiguration&&) = delete;
        JPEGDecoderConfiguration() = delete;
};
#endif
    class JPEGCodedImageItem : public HEIFPP::CodedImageItem
    {
    public:
        JPEGCodedImageItem(Heif* aHeif);
        virtual ~JPEGCodedImageItem() = default;

    protected:
        // serialization
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

        virtual void getBitstream(uint8_t*& aData, uint64_t& aSize);

    private:
        JPEGCodedImageItem& operator=(const JPEGCodedImageItem&) = delete;
        JPEGCodedImageItem(const JPEGCodedImageItem&)            = delete;
        JPEGCodedImageItem(JPEGCodedImageItem&&)                 = delete;
        JPEGCodedImageItem()                                     = delete;
    };
}  // namespace HEIFPP
