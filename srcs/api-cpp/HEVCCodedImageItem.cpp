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

#include "HEVCCodedImageItem.h"
#include <heifreader.h>
#include <heifwriter.h>
#include "H26xTools.h"
using namespace HEIFPP;

HEVCDecoderConfiguration::HEVCDecoderConfiguration(Heif* aHeif)
    : DecoderConfiguration(aHeif, HEIF::MediaFormat::HEVC)
{
}
HEIF::ErrorCode HEVCDecoderConfiguration::convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig,
                                                           uint8_t*& aData,
                                                           uint32_t& aSize) const
{
    aSize = 0;
    for (size_t i = 0; i < aConfig.size; i++)
    {
        if ((aConfig[i].decSpecInfoType != HEIF::DecoderSpecInfoType::HEVC_VPS) &&
            (aConfig[i].decSpecInfoType != HEIF::DecoderSpecInfoType::HEVC_SPS) &&
            (aConfig[i].decSpecInfoType != HEIF::DecoderSpecInfoType::HEVC_PPS))
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }


        aSize += (uint32_t) aConfig[i].decSpecInfoData.size;
    }
    uint8_t* d = aData = new uint8_t[aSize];
    for (size_t i = 0; i < aConfig.size; i++)
    {
        memcpy(d, aConfig[i].decSpecInfoData.begin(), aConfig[i].decSpecInfoData.size);
        d += aConfig[i].decSpecInfoData.size;
    }
    return HEIF::ErrorCode::OK;
}
HEIF::ErrorCode HEVCDecoderConfiguration::convertFromRawData(const uint8_t* aData, uint32_t aSize)
{
    // ISO / IEC 14496 - 15:2017 8.3.3.1 HEVC decoder configuration record :
    // It is recommended that the arrays be in the order VPS, SPS, PPS, prefix SEI, suffix SEI.

    mConfig.decoderSpecificInfo = HEIF::Array<HEIF::DecoderSpecificInfo>(3);

    // NOTE: only VPS,PPS,SPS is saved here, and we expect all three to exist.
    NAL_State d;
    uint32_t flags;
    d.init_parse(aData, aSize);
    flags = 0;
    for (;;)
    {
        const uint8_t* nal_data;
        uint64_t nal_len;
        if (!d.parse_byte_stream(nal_data, nal_len))
        {
            break;
        }
        HEIF::DecoderSpecInfoType type;
        type           = (HEIF::DecoderSpecInfoType)((nal_data[0] >> 1) & 0x3f);
        uint32_t index = 0;
        if (type == HEIF::DecoderSpecInfoType::HEVC_VPS)
            index = 0;
        else if (type == HEIF::DecoderSpecInfoType::HEVC_SPS)
            index = 1;
        else if (type == HEIF::DecoderSpecInfoType::HEVC_PPS)
            index = 2;
        else
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
        if ((flags & (1u << index)) != 0)
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
        flags |= 1u << index;
        mConfig.decoderSpecificInfo[index].decSpecInfoType    = type;
        mConfig.decoderSpecificInfo[index].decSpecInfoData    = HEIF::Array<uint8_t>(nal_len + 4);
        mConfig.decoderSpecificInfo[index].decSpecInfoData[0] = mConfig.decoderSpecificInfo[index].decSpecInfoData[1] =
            mConfig.decoderSpecificInfo[index].decSpecInfoData[2] = 0;
        mConfig.decoderSpecificInfo[index].decSpecInfoData[3]     = 1;
        memcpy(mConfig.decoderSpecificInfo[index].decSpecInfoData.elements + 4, nal_data, nal_len);
    }
    if (flags != 7)
    {
        return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
    }
    return HEIF::ErrorCode::OK;
}

HEIF::ErrorCode HEVCDecoderConfiguration::setConfig(const uint8_t* aData, uint32_t aSize)
{
    delete[] mBuffer;
    mBuffer     = 0;
    mBufferSize = 0;
    mBuffer     = new uint8_t[aSize];
    memcpy(mBuffer, aData, aSize);
    mBufferSize = aSize;
    return convertFromRawData(mBuffer, mBufferSize);
}

void HEVCDecoderConfiguration::getConfig(uint8_t*& aData, uint32_t& aSize) const
{
    aData = mBuffer;
    aSize = mBufferSize;
}

HEVCCodedImageItem::HEVCCodedImageItem(Heif* aHeif)
    : CodedImageItem(aHeif, HEIF::FourCC("hvc1"), HEIF::MediaFormat::HEVC)
{
}
HEIF::ErrorCode HEVCCodedImageItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    return CodedImageItem::load(aReader, aId);
}
HEIF::ErrorCode HEVCCodedImageItem::save(HEIF::Writer* aWriter)
{
    return CodedImageItem::save(aWriter);
}

void HEVCCodedImageItem::getBitstream(uint8_t*& aData, uint64_t& aSize)
{
    // convert nal bytestream to nal unit stream (ie. change start code prefixes to lengths)
    NAL_State d;
    d.init_parse(mBuffer, mBufferSize);
    if (aData == nullptr)
    {
        aSize = mBufferSize;
        aData = new uint8_t[aSize];
    }
    uint64_t curSize = 0;
    for (;;)
    {
        const uint8_t* nal_data;
        uint64_t nal_len;
        if (!d.parse_byte_stream(nal_data, nal_len))
        {
            if (!d.end_of_stream())
            {
                // We have corrupted data
                curSize = 0;
                aSize   = 0;
                delete[] aData;
                aData = nullptr;
            }
            break;
        }
        // HEIF::DecoderSpecInfoType type = (HEIF::DecoderSpecInfoType) ((nal_data[0] >> 1) & 0x3f);
        uint64_t required = curSize + 4 + nal_len;
        if (required > aSize)
        {
            uint8_t* tmp = new uint8_t[required];
            aSize        = required;
            memcpy(tmp, aData, curSize);
            delete[] aData;
            aData = tmp;
        }
        // Write length.
        aData[curSize + 0] = (nal_len >> 24) & 0xFF;
        aData[curSize + 1] = (nal_len >> 16) & 0xFF;
        aData[curSize + 2] = (nal_len >> 8) & 0xFF;
        aData[curSize + 3] = (nal_len >> 0) & 0xFF;
        // Write payload.
        memcpy(aData + curSize + 4, nal_data, nal_len);
        curSize += nal_len + 4;
    }
    aSize = curSize;
}
