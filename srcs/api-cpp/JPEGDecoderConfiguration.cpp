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

#include "JPEGDecoderConfiguration.h"

#include <algorithm>

using namespace HEIFPP;

JPEGDecoderConfiguration::JPEGDecoderConfiguration(Heif* aHeif, const HEIF::FourCC& aType)
    : DecoderConfig(aHeif, aType)
{
}

HEIF::ErrorCode JPEGDecoderConfiguration::convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig,
                                                           std::uint8_t*& aData,
                                                           std::uint32_t& aSize) const
{
    aSize = 0;
    for (size_t i = 0; i < aConfig.size; i++)
    {
        if (aConfig[i].decSpecInfoType != HEIF::DecoderSpecInfoType::JPEG)
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
        aSize += static_cast<uint32_t>(aConfig[i].decSpecInfoData.size);
    }
    std::uint8_t* d = aData = new std::uint8_t[aSize];
    for (size_t i = 0; i < aConfig.size; i++)
    {
        std::copy(aConfig[i].decSpecInfoData.begin(), aConfig[i].decSpecInfoData.end(), d);
        d += aConfig[i].decSpecInfoData.size;
    }
    return HEIF::ErrorCode::OK;
}

HEIF::ErrorCode JPEGDecoderConfiguration::convertFromRawData(const std::uint8_t* aData, std::uint32_t aSize)
{
    // How to verify that the data contains valid JPEG prefix/config code? The only realistic way
    // is to run a custom JPEG decoder against the prefix and determine if it fails before the
    // end-of-data. Not feasible.
    mConfig.decoderSpecificInfo                    = HEIF::Array<HEIF::DecoderSpecificInfo>(1);
    mConfig.decoderSpecificInfo[0].decSpecInfoType = HEIF::DecoderSpecInfoType::JPEG;
    mConfig.decoderSpecificInfo[0].decSpecInfoData = HEIF::Array<uint8_t>(aSize);
    std::copy_n(aData, aSize, mConfig.decoderSpecificInfo[0].decSpecInfoData.elements);
    return HEIF::ErrorCode::OK;
}

HEIF::ErrorCode JPEGDecoderConfiguration::setConfig(const std::uint8_t* aData, std::uint32_t aSize)
{
    return convertFromRawData(aData, aSize);
}

void JPEGDecoderConfiguration::getConfig(uint8_t*& aData, std::uint32_t& aSize) const
{
    aData = mBuffer;
    aSize = mBufferSize;
}
