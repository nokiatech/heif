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

#include <DecoderConfiguration.h>

namespace HEIFPP
{
    class HEVCDecoderConfiguration : public HEIFPP::DecoderConfig
    {
    public:
        HEVCDecoderConfiguration(Heif* aHeif);
        HEVCDecoderConfiguration(Heif* aHeif, const HEIF::FourCC& aType);
        ~HEVCDecoderConfiguration() = default;

        /** Returns the config data
         * @param [out] data: Reference to the data pointer where the data is stored.
         * @param [out] size: Reference to the variable where the size will be stored. */
        void getConfig(std::uint8_t*& data, std::uint32_t& size) const override;

        /** Sets the config data
         * @param [in] data: Pointer to the data.
         * @param [in] size: Size of the data. */
        HEIF::ErrorCode setConfig(const std::uint8_t* data, std::uint32_t size) override;

    protected:
        HEIF::ErrorCode convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig,
                                         std::uint8_t*& aData,
                                         std::uint32_t& aSize) const override;
        HEIF::ErrorCode convertFromRawData(const std::uint8_t* aData, std::uint32_t aSize) override;

    private:
        HEVCDecoderConfiguration& operator=(const HEVCDecoderConfiguration&) = delete;
        HEVCDecoderConfiguration& operator=(HEVCDecoderConfiguration&&)      = delete;
        HEVCDecoderConfiguration(const HEVCDecoderConfiguration&)            = delete;
        HEVCDecoderConfiguration(HEVCDecoderConfiguration&&)                 = delete;
        HEVCDecoderConfiguration()                                           = delete;
    };
}  // namespace HEIFPP
