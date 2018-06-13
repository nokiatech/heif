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

#include "JPEGCodedImageItem.h"
#include <heifreader.h>
#include <heifwriter.h>
#include <cstring>

using namespace HEIFPP;

#if 0
JPEGDecoderConfiguration::JPEGDecoderConfiguration(Heif* aHeif) : DecoderConfiguration(aHeif,HEIF::MediaFormat::JPEG)
{
}
HEIF::ErrorCode JPEGDecoderConfiguration::convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig, std::uint8_t*& aData, std::uint32_t& aSize) const
{
    aSize = 0;
    for (int i = 0; i < aConfig.size; i++)
    {
        if (aConfig[i].decSpecInfoType != HEIF::DecoderSpecInfoType::JPEG)
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
        aSize += (uint32_t)aConfig[i].decSpecInfoData.size;
    }
    std::uint8_t* d = aData = new std::uint8_t[aSize];
    for (int i = 0; i < aConfig.size; i++)
    {
        std::memcpy(d, aConfig[i].decSpecInfoData.begin(), aConfig[i].decSpecInfoData.size);
        d += aConfig[i].decSpecInfoData.size;
    }
    return HEIF::ErrorCode::OK;
}
HEIF::ErrorCode JPEGDecoderConfiguration::convertFromRawData(const std::uint8_t* aData, std::uint32_t aSize)
{
    //TODO: how to verify that the data contains valid JPEG prefix/config code?
    mConfig.decoderSpecificInfo = HEIF::Array<HEIF::DecoderSpecificInfo>(1);
    mConfig.decoderSpecificInfo[0].decSpecInfoType = HEIF::DecoderSpecInfoType::JPEG;
    mConfig.decoderSpecificInfo[0].decSpecInfoData = HEIF::Array<uint8_t>(aSize);
    std::memcpy(mConfig.decoderSpecificInfo[0].decSpecInfoData.elements, aData,aSize);
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
#endif
JPEGCodedImageItem::JPEGCodedImageItem(Heif* aHeif)
    : CodedImageItem(aHeif, HEIF::FourCC("jpeg"), HEIF::MediaFormat::JPEG)
{
    mMandatoryConfiguration = false;
}
HEIF::ErrorCode JPEGCodedImageItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    return CodedImageItem::load(aReader, aId);
}
HEIF::ErrorCode JPEGCodedImageItem::save(HEIF::Writer* aWriter)
{
    return CodedImageItem::save(aWriter);
}

bool JPEGCodedImageItem::getBitstream(uint8_t*& aData, std::uint64_t& aSize)
{
    // TODO: nothing to do?
    aSize = mBufferSize;
    aData = new std::uint8_t[mBufferSize];
    std::memcpy(aData, mBuffer, aSize);
    return true;
}
