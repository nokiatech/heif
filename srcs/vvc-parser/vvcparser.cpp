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

#include "vvcparser.hpp"
#include "heiferrorinfo.h"

#include <vector>
#include "NAL/vvcrbspreader.hpp"
#include "NAL/vvcopiparser.hpp"
#include "NAL/vvcppsparser.hpp"
#include "NAL/vvcspsparser.hpp"
#include "NAL/vvcvpsparser.hpp"
#include "../common/customallocator.hpp"
#include "heifcommondatatypes.h"


uint16_t VvcParser::getLowestOlsIdxWithLargestNumOutputLayers(const VpsInfo& vpsinfo){
    uint16_t largestNumLayers = 0;
    std::vector<uint16_t> mostLayersOlsIdx;
    
    // First pass: find all OLS indices with the largest number of layers
    for (uint16_t olsIdx = 0; olsIdx < vpsinfo.mTotalNumOLSs; olsIdx++){
        if(vpsinfo.mNumLayersInOls[olsIdx] > largestNumLayers){
            largestNumLayers = static_cast<uint16_t>(vpsinfo.mNumLayersInOls[olsIdx]);
            mostLayersOlsIdx.clear();
            mostLayersOlsIdx.push_back(olsIdx);
        } else if(vpsinfo.mNumLayersInOls[olsIdx] == largestNumLayers){
            mostLayersOlsIdx.push_back(olsIdx);
        }
    }

    // Second pass: among those, find the lowest index with most output layers
    uint16_t mostOutputLayers = 0;
    uint16_t returnOlsIdx = 0;
    for(const auto& molsIdx : mostLayersOlsIdx){
        if(vpsinfo.mNumOutputLayersInOls[molsIdx] > mostOutputLayers){
            mostOutputLayers = static_cast<uint16_t>(vpsinfo.mNumOutputLayersInOls[molsIdx]);
            returnOlsIdx = molsIdx;
        }
    }
    return returnOlsIdx;
}

VvcParser::PpsInfo VvcParser::parsePps(const Vector<uint8_t>& ppsData){
    VvcRbspReader reader;
    if(reader.prepare(ppsData)){
        return VvcPpsParser::parse(ppsData);
    }else{
        HEIF_RECORD(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                    ::HEIF::ErrorOrigin::Parser_VvcRbsp,
                    "VvcParser::parsePps: VvcRbspReader::prepare rejected PPS NAL");
        return PpsInfo{};
    }
}


VvcParser::SpsInfo VvcParser::parseSps(const Vector<uint8_t>& spsData)
{
    return VvcSpsParser::parse(spsData);
}

VvcParser::OpiInfo VvcParser::parseOpi(const Vector<uint8_t>& opiData)
{
    VvcRbspReader reader;
    if(reader.prepare(opiData)){
        return VvcOpiParser::parse(opiData);
    }else{
        HEIF_RECORD(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                    ::HEIF::ErrorOrigin::Parser_VvcRbsp,
                    "VvcParser::parseOpi: VvcRbspReader::prepare rejected OPI NAL");
        return OpiInfo{};
    }
}

VvcParser::VpsInfo VvcParser::parseVps(const Vector<uint8_t>& vpsData)
{
    return VvcVpsParser::parse(vpsData);
}
