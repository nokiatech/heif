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

#include "mp4audiodecoderconfigrecord.hpp"
#include "elementarystreamdescriptorbox.hpp"

MP4AudioDecoderConfigurationRecord::MP4AudioDecoderConfigurationRecord(ElementaryStreamDescriptorBox& aBox)
    : mESDBox(aBox)
{
}

void MP4AudioDecoderConfigurationRecord::getConfigurationMap(ConfigurationMap& aMap) const
{
    Vector<std::uint8_t> decoderSpecInfo;
    aMap.clear();
    if (mESDBox.getOneParameterSet(decoderSpecInfo))
    {
        // Only handle AudioSpecificConfig from 1.6.2.1 AudioSpecificConfig of ISO/IEC 14496-3:200X(E) subpart 1
        aMap.insert({DecoderParameterType::AudioSpecificConfig, std::move(decoderSpecInfo)});
    }
}