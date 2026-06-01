/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef DECODERCONFIGRECORD_HPP
#define DECODERCONFIGRECORD_HPP

#include <cstdint>

#include "customallocator.hpp"
#include "bitstream.hpp"

class DecoderConfigurationRecord
{
public:
    enum DecoderParameterType : std::uint8_t
    {
        AVC_SPS = 0,
        AVC_PPS,
        HEVC_VPS,
        HEVC_SPS,
        HEVC_PPS,
        VVC_APS,
        VVC_SPS,
        VVC_PPS,
        VVC_VPS,
        AudioSpecificConfig,
        JPEG
    };

    typedef MultiMap<DecoderParameterType, Vector<std::uint8_t>> ConfigurationMap;

    DecoderConfigurationRecord()          = default;
    virtual ~DecoderConfigurationRecord() = default;

    /* @brief Returns configuration parameter map for this record */
    virtual void getConfigurationMap(ConfigurationMap& aMap, const uint16_t& nuh_layer_id = 0) const = 0;

    virtual void writeDecConfigRecord(ISOBMFF::BitStream& bitstr) const = 0;

    //A totally healthy workaround to allow calling this method for
    //compact vvc decoderconfiguration only.
    virtual void writeCompactDecConfigRecord(ISOBMFF::BitStream& bitstr) const {
        //This could also raise an error, but probably wont be used outside of vvcC mini usage
        writeDecConfigRecord(bitstr);
    }
};

#endif /* end of include guard: DECODERCONFIGRECORD_HPP*/
