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

#include "lhevcdecoderconfigrecord.hpp"

#include <utility>
#include <vector>

#include "bitstream.hpp"
#include "nalutil.hpp"

LhevcDecoderConfigurationRecord::LhevcDecoderConfigurationRecord()
    : mNalArray()
{
}

void LhevcDecoderConfigurationRecord::addNalUnit(const Vector<std::uint8_t>& nalUnit,
                                                 const HevcNalUnitType nalUnitType,
                                                 const bool arrayCompleteness)
{
    NALArray* nalArray = nullptr;
    Vector<std::uint8_t> tmpNalUnit;

    for (auto& i : mNalArray)
    {
        if (static_cast<std::uint8_t>(nalUnitType) == static_cast<std::uint8_t>(i.nalUnitType))
        {
            nalArray = &i;
            break;
        }
    }

    if (nalArray == nullptr)
    {
        NALArray nalArrayTmp;
        nalArrayTmp.arrayCompleteness = arrayCompleteness;
        nalArrayTmp.nalUnitType       = nalUnitType;
        mNalArray.push_back(nalArrayTmp);
        nalArray = &mNalArray.back();
    }

    const unsigned int startCodeLen = findStartCodeLen(nalUnit);
    tmpNalUnit.insert(tmpNalUnit.begin(),
                      nalUnit.cbegin() + static_cast<int>(startCodeLen),
                      nalUnit.cend());

    nalArray->nalList.push_back(tmpNalUnit);
}

void LhevcDecoderConfigurationRecord::writeDecConfigRecord(ISOBMFF::BitStream& bitstr) const
{
    //bit(8)  configurationVersion
    bitstr.writeBits(mConfigurationVersion, 8);
    //bit(4)  reserved = '1111'b
    bitstr.writeBits(0xf, 4);
    //bit(12) min_spatial_segmentation_idc
    bitstr.writeBits(mMinSpatialSegmentationIdc, 12);
    //bit(6)  reserved = '111111'b
    bitstr.writeBits(0x3f, 6);
    //bit(2)  parallelismType
    bitstr.writeBits(mParallelismType, 2);
    //bit(2)  reserved = '11'b
    bitstr.writeBits(0x3, 2);
    //bit(3)  numTemporalLayers
    bitstr.writeBits(mNumTemporalLayers, 3);
    //bit(1)  temporalIdNested
    bitstr.writeBits(mTemporalIdNested, 1);
    //bit(2)  lengthSizeMinusOne
    bitstr.writeBits(mLengthSizeMinus1, 2);

    //bit(8)  numOfArrays
    bitstr.writeBits(static_cast<unsigned int>(mNalArray.size()), 8);
    for (const auto& i : mNalArray)
    {
        //bit(1)  array_completeness
        bitstr.writeBits(i.arrayCompleteness, 1);
        //bit(1)  reserved = 0
        bitstr.writeBits(0, 1);
        //bit(6)  NAL_unit_type
        bitstr.writeBits(static_cast<std::uint8_t>(i.nalUnitType), 6);
        //bit(16) numNalus
        bitstr.writeBits(static_cast<unsigned int>(i.nalList.size()), 16);
        for (const auto& j : i.nalList)
        {
            //bit(16)              nalUnitLength
            bitstr.writeBits(static_cast<unsigned int>(j.size()), 16);
            //bit(8*nalUnitLength) nalUnit
            bitstr.write8BitsArray(j, j.size());
        }
    }
}

void LhevcDecoderConfigurationRecord::parseConfig(ISOBMFF::BitStream& bitstr)
{
    mConfigurationVersion      = static_cast<std::uint8_t>(bitstr.readBits(8));
    bitstr.readBits(4);   // reserved = '1111'b
    mMinSpatialSegmentationIdc = static_cast<std::uint16_t>(bitstr.readBits(12));
    bitstr.readBits(6);   // reserved = '111111'b
    mParallelismType           = static_cast<std::uint8_t>(bitstr.readBits(2));
    bitstr.readBits(2);   // reserved = '11'b
    mNumTemporalLayers         = static_cast<std::uint8_t>(bitstr.readBits(3));
    mTemporalIdNested          = static_cast<std::uint8_t>(bitstr.readBits(1));
    mLengthSizeMinus1          = static_cast<std::uint8_t>(bitstr.readBits(2));

    const unsigned int numOfArrays = bitstr.readBits(8);
    for (unsigned int i = 0; i < numOfArrays; i++)
    {
        const bool arrayCompleteness = (bitstr.readBits(1) != 0);
        bitstr.readBits(1);  // reserved = 0
        const HevcNalUnitType nalUnitType = static_cast<HevcNalUnitType>(bitstr.readBits(6));
        const unsigned int numNalus       = bitstr.readBits(16);
        for (unsigned int j = 0; j < numNalus; j++)
        {
            const unsigned int nalSize = bitstr.readBits(16);
            Vector<std::uint8_t> nalData;
            bitstr.read8BitsArray(nalData, nalSize);
            addNalUnit(nalData, nalUnitType, arrayCompleteness);
        }
    }
}

void LhevcDecoderConfigurationRecord::getOneParameterSet(Vector<std::uint8_t>& byteStream,
                                                         const HevcNalUnitType nalUnitType) const
{
    for (const auto& array : mNalArray)
    {
        if (array.nalUnitType == nalUnitType && array.nalList.size() > 0)
        {
            byteStream.push_back(0);
            byteStream.push_back(0);
            byteStream.push_back(0);
            byteStream.push_back(1);
            byteStream.insert(byteStream.end(), array.nalList.at(0).cbegin(), array.nalList.at(0).cend());
        }
    }
}

void LhevcDecoderConfigurationRecord::getConfigurationMap(ConfigurationMap& aMap,
                                                          const std::uint16_t& /*nuh_layer_id*/) const
{
    Vector<std::uint8_t> sps;
    Vector<std::uint8_t> pps;
    Vector<std::uint8_t> vps;
    getOneParameterSet(sps, HevcNalUnitType::SPS);
    getOneParameterSet(pps, HevcNalUnitType::PPS);
    getOneParameterSet(vps, HevcNalUnitType::VPS);

    aMap.insert({DecoderParameterType::HEVC_SPS, std::move(sps)});
    aMap.insert({DecoderParameterType::HEVC_PPS, std::move(pps)});
    aMap.insert({DecoderParameterType::HEVC_VPS, std::move(vps)});
}

std::uint8_t  LhevcDecoderConfigurationRecord::getConfigurationVersion() const { return mConfigurationVersion; }
void          LhevcDecoderConfigurationRecord::setConfigurationVersion(std::uint8_t version) { mConfigurationVersion = version; }
std::uint16_t LhevcDecoderConfigurationRecord::getMinSpatialSegmentationIdc() const { return mMinSpatialSegmentationIdc; }
void          LhevcDecoderConfigurationRecord::setMinSpatialSegmentationIdc(std::uint16_t idc) { mMinSpatialSegmentationIdc = idc; }
std::uint8_t  LhevcDecoderConfigurationRecord::getParallelismType() const { return mParallelismType; }
void          LhevcDecoderConfigurationRecord::setParallelismType(std::uint8_t type) { mParallelismType = type; }
std::uint8_t  LhevcDecoderConfigurationRecord::getNumTemporalLayers() const { return mNumTemporalLayers; }
void          LhevcDecoderConfigurationRecord::setNumTemporalLayers(std::uint8_t numLayers) { mNumTemporalLayers = numLayers; }
std::uint8_t  LhevcDecoderConfigurationRecord::getTemporalIdNested() const { return mTemporalIdNested; }
void          LhevcDecoderConfigurationRecord::setTemporalIdNested(std::uint8_t nested) { mTemporalIdNested = nested; }
std::uint8_t  LhevcDecoderConfigurationRecord::getLengthSizeMinus1() const { return mLengthSizeMinus1; }
void          LhevcDecoderConfigurationRecord::setLengthSizeMinus1(std::uint8_t lengthSizeMinus1) { mLengthSizeMinus1 = lengthSizeMinus1; }
