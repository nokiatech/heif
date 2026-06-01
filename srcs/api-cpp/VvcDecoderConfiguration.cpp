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

#include "VvcDecoderConfiguration.h"


#include <cstring>

#include "H26xTools.h"
#include "DecoderConfiguration.h"
#include "heifreaderdatatypes.h"

using namespace HEIFPP;

VVCDecoderConfiguration::VVCDecoderConfiguration(Heif* aHeif)
    : DecoderConfig(aHeif, HEIF::FourCC("vvc1"))
{
}
VVCDecoderConfiguration::VVCDecoderConfiguration(Heif* aHeif, const HEIF::FourCC& aType)
    : DecoderConfig(aHeif, aType)
{
}

HEIF::ErrorCode VVCDecoderConfiguration::convertToRawData(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig,
                                                           std::uint8_t*& aData,
                                                           std::uint32_t& aSize) const{
    aSize = 0;
    for (std::size_t i = 0; i < aConfig.size; i++){
        if ((aConfig[i].decSpecInfoType != HEIF::DecoderSpecInfoType::VVC_VPS) &&
            (aConfig[i].decSpecInfoType != HEIF::DecoderSpecInfoType::VVC_SPS) &&
            (aConfig[i].decSpecInfoType != HEIF::DecoderSpecInfoType::VVC_PPS) &&
            (aConfig[i].decSpecInfoType != HEIF::DecoderSpecInfoType::VVC_APS_PREFIX) &&
            (aConfig[i].decSpecInfoType != HEIF::DecoderSpecInfoType::VVC_APS_SUFFIX)){
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
        aSize += static_cast<uint32_t>(aConfig[i].decSpecInfoData.size);
    }

    std::uint8_t* d = aData = new std::uint8_t[aSize];
    for (std::size_t i = 0; i < aConfig.size; i++){
        std::memcpy(d, aConfig[i].decSpecInfoData.begin(), aConfig[i].decSpecInfoData.size);
        d += aConfig[i].decSpecInfoData.size;
    }

    return HEIF::ErrorCode::OK;
}

HEIF::ErrorCode VVCDecoderConfiguration::convertFromRawData(const std::uint8_t* aData, std::uint32_t aSize){

    // First pass: count NAL units to allocate proper array size
    // VVC can have multiple APS NAL units, so we need dynamic sizing
    NAL_State d;
    d.init_parse(aData, aSize);
    
    std::uint32_t nalCount = 0;
    std::uint32_t flags = 0;
    
    for (;;){
        const std::uint8_t* nal_data = nullptr;
        std::uint64_t nal_len = 0;
        if (!d.parse_byte_stream(nal_data, nal_len)){
            break;
        }
        
        // VVC NAL unit header is 2 bytes:
        // Byte 0: forbidden_zero_bit(1) | nuh_reserved_zero_bit(1) | nuh_layer_id(6)
        // Byte 1: nal_unit_type(5) | nuh_temporal_id_plus1(3)
        // Extract NAL type from bits 3-7 of second byte
        HEIF::DecoderSpecInfoType type = static_cast<HEIF::DecoderSpecInfoType>((nal_data[1] >> 3) & 0x1F);
        
        if (type == HEIF::DecoderSpecInfoType::VVC_VPS ||
            type == HEIF::DecoderSpecInfoType::VVC_SPS ||
            type == HEIF::DecoderSpecInfoType::VVC_PPS ||
            type == HEIF::DecoderSpecInfoType::VVC_APS_PREFIX ||
            type == HEIF::DecoderSpecInfoType::VVC_APS_SUFFIX){
            
            // Track which required types we have (VPS=bit0, SPS=bit1, PPS=bit2)
            if (type == HEIF::DecoderSpecInfoType::VVC_VPS) flags |= 1;
            else if (type == HEIF::DecoderSpecInfoType::VVC_SPS) flags |= 2;
            else if (type == HEIF::DecoderSpecInfoType::VVC_PPS) flags |= 4;
            
            nalCount++;
        } else {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
    }
    
    // VVC requires at least SPS (bit 1) and PPS (bit 2)
    // VPS is optional in VVC (unlike HEVC)
    if ((flags & 0x6) != 0x6){
        return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
    }
    
    // Allocate array for all NAL units
    mConfig.decoderSpecificInfo = HEIF::Array<HEIF::DecoderSpecificInfo>(nalCount);
    
    // Second pass: populate the array
    d.init_parse(aData, aSize);
    std::uint32_t idx = 0;
    
    for (;;){
        const std::uint8_t* nal_data = nullptr;
        std::uint64_t nal_len = 0;
        if (!d.parse_byte_stream(nal_data, nal_len)){
            break;
        }
        
        HEIF::DecoderSpecInfoType type = static_cast<HEIF::DecoderSpecInfoType>((nal_data[1] >> 3) & 0x1F);
        
        mConfig.decoderSpecificInfo[idx].decSpecInfoType = type;
        mConfig.decoderSpecificInfo[idx].decSpecInfoData = HEIF::Array<uint8_t>(nal_len + 4);
        // Add start code prefix
        mConfig.decoderSpecificInfo[idx].decSpecInfoData[0] = 0;
        mConfig.decoderSpecificInfo[idx].decSpecInfoData[1] = 0;
        mConfig.decoderSpecificInfo[idx].decSpecInfoData[2] = 0;
        mConfig.decoderSpecificInfo[idx].decSpecInfoData[3] = 1;
        std::memcpy(mConfig.decoderSpecificInfo[idx].decSpecInfoData.elements + 4, nal_data, nal_len);
        idx++;
    }
    
    return HEIF::ErrorCode::OK;
}

HEIF::ErrorCode VVCDecoderConfiguration::setConfig(const std::uint8_t* aData, std::uint32_t aSize){
    delete[] mBuffer;
    mBuffer     = nullptr;
    mBufferSize = 0;
    mBuffer     = new std::uint8_t[aSize];
    std::memcpy(mBuffer, aData, aSize);
    mBufferSize = aSize;
    return convertFromRawData(mBuffer, mBufferSize);
}

void VVCDecoderConfiguration::getConfig(uint8_t*& aData, std::uint32_t& aSize) const{
    aData = mBuffer;
    aSize = mBufferSize;
}
