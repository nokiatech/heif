/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#pragma once

#include <DecoderConfiguration.h>
#include "heifcommondatatypes.h"
#include <cstdint>

namespace HEIFPP
{
    /// Decoder configuration for L-HEVC (Layered HEVC) items.
    ///
    /// Mirrors HEVCDecoderConfiguration but carries the four-cc "lhv1"
    /// instead of "hvc1". The NAL unit payload (VPS/SPS/PPS) is parsed
    /// and serialised identically -- the difference is the item type and
    /// the fact that the decoder configuration record is an
    /// LHEVCDecoderConfigurationRecord (ISO/IEC 14496-15 Section 9.4) rather
    /// than an HEVCDecoderConfigurationRecord (Section 8.3.3.1). At the
    /// Annex-B NAL level the parameter sets share the same structure.
    class LHEVCDecoderConfiguration : public HEIFPP::DecoderConfig
    {
    public:
        explicit LHEVCDecoderConfiguration(Heif* aHeif);
        LHEVCDecoderConfiguration(Heif* aHeif, const HEIF::FourCC& aType);
        ~LHEVCDecoderConfiguration() override = default;

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

    public:
        LHEVCDecoderConfiguration& operator=(const LHEVCDecoderConfiguration&) = delete;
        LHEVCDecoderConfiguration& operator=(LHEVCDecoderConfiguration&&) = delete;
        LHEVCDecoderConfiguration(const LHEVCDecoderConfiguration&)       = delete;
        LHEVCDecoderConfiguration(LHEVCDecoderConfiguration&&)            = delete;
        LHEVCDecoderConfiguration()                                       = delete;
    };
}  // namespace HEIFPP
