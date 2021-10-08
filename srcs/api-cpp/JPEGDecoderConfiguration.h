/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#pragma once

#include <DecoderConfiguration.h>

namespace HEIFPP
{
    class JPEGDecoderConfiguration : public HEIFPP::DecoderConfig
    {
    public:
        JPEGDecoderConfiguration(Heif* aHeif, const HEIF::FourCC& aType);
        ~JPEGDecoderConfiguration() override = default;
        void getConfig(std::uint8_t*& data, std::uint32_t& size) const override;
        HEIF::ErrorCode setConfig(const std::uint8_t* data, std::uint32_t size) override;

    protected:
        HEIF::ErrorCode convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig,
                                         std::uint8_t*& aData,
                                         std::uint32_t& aSize) const override;
        HEIF::ErrorCode convertFromRawData(const std::uint8_t* aData, std::uint32_t aSize) override;

    private:
        JPEGDecoderConfiguration& operator=(const JPEGDecoderConfiguration&) = delete;
        JPEGDecoderConfiguration& operator=(JPEGDecoderConfiguration&&) = delete;
        JPEGDecoderConfiguration(const JPEGDecoderConfiguration&)       = delete;
        JPEGDecoderConfiguration(JPEGDecoderConfiguration&&)            = delete;
        JPEGDecoderConfiguration()                                      = delete;
    };
}  // namespace HEIFPP
