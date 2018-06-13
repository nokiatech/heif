/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

class DecoderConfigurationRecord
{
public:
    enum DecoderParameterType
    {
        AVC_SPS=0,
        AVC_PPS,
        HEVC_VPS,
        HEVC_SPS,
        HEVC_PPS,
        AudioSpecificConfig
    };

    typedef Map<DecoderParameterType, Vector<std::uint8_t>> ConfigurationMap;

    DecoderConfigurationRecord()          = default;
    virtual ~DecoderConfigurationRecord() = default;

    /* @brief Returns configuration parameter map for this record */
    virtual void getConfigurationMap(ConfigurationMap& aMap) const = 0;
};

#endif /* end of include guard: DECODERCONFIGRECORD_HPP*/
