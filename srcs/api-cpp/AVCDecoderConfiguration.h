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
    class AVCDecoderConfiguration : public HEIFPP::DecoderConfig
    {
    public:
        AVCDecoderConfiguration(Heif* aHeif);
        AVCDecoderConfiguration(Heif* aHeif, const HEIF::FourCC& aType);
        ~AVCDecoderConfiguration() = default;

        /** Returns the whole configuration as a block
         * @param [out] data: Reference to where the data should be copied
         * @param [out] size: The size of the data copied */
        void getConfig(std::uint8_t*& data, std::uint32_t& size) const override;

        /** Sets the decoder data as a block
         * @param [in] data: Pointer to the data
         * @param [in] size: The size of the data */
        HEIF::ErrorCode setConfig(const std::uint8_t* data, std::uint32_t size) override;

    protected:
        HEIF::ErrorCode convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig,
                                         std::uint8_t*& aData,
                                         std::uint32_t& aSize) const override;
        HEIF::ErrorCode convertFromRawData(const std::uint8_t* aData, std::uint32_t aSize) override;

    private:
        AVCDecoderConfiguration& operator=(const AVCDecoderConfiguration&) = delete;
        AVCDecoderConfiguration& operator=(AVCDecoderConfiguration&&)      = delete;
        AVCDecoderConfiguration(const AVCDecoderConfiguration&)            = delete;
        AVCDecoderConfiguration(AVCDecoderConfiguration&&)                 = delete;
        AVCDecoderConfiguration()                                          = delete;
    };

}  // namespace HEIFPP
