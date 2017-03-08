/* Copyright (c) 2017, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#include "lhevcdecoderconfigrecord.hpp"
#include "bitstream.hpp"
#include "h265parser.hpp"

LHevcDecoderConfigurationRecord::LHevcDecoderConfigurationRecord() :
    mConfigurationVersion(1),
    mMinSpatialSegmentationIdc(0),
    mParallelismType(0),
    mNumTemporalLayers(0),
    mTemporalIdNested(0),
    mLengthSizeMinus1(0),
    mNalArray()
{
}


void LHevcDecoderConfigurationRecord::addNalUnit(const std::vector<uint8_t>& nalUnit, const HevcNalUnitType nalUnitType,
    const uint8_t arrayCompleteness)
{
    NALArray* nalArray = nullptr;
    std::vector<uint8_t> tmpNalUnit;
    unsigned int startCodeLen;

    // Find array for the given NAL unit type.
    for (auto& i : mNalArray)
    {
        if (static_cast<uint8_t>(nalUnitType) == static_cast<uint8_t>(i.nalUnitType))
        {
            nalArray = &i;
            break;
        }
    }

    // If an array is not present for the NAL unit type, create one.
    if (nalArray == nullptr)
    {
        NALArray nalArrayTmp;
        nalArrayTmp.arrayCompleteness = arrayCompleteness;
        nalArrayTmp.nalUnitType = nalUnitType;
        mNalArray.push_back(nalArrayTmp);
        nalArray = &mNalArray.back();
    }

    startCodeLen = findStartCodeLen(nalUnit);
    tmpNalUnit.insert(tmpNalUnit.begin(), nalUnit.cbegin() + startCodeLen, nalUnit.cend()); // copy NAL data excluding potential start code

    // Add NAL unit to the NAL unit array.
    nalArray->nalUnits.push_back(tmpNalUnit);
}

void LHevcDecoderConfigurationRecord::makeConfigFromSPS(const std::vector<uint8_t>& srcSps, const std::vector<uint8_t>& srcVps)
{
    std::vector<int> subLayerProfilePresentFlag(8, 0);
    std::vector<int> subLayerLevelPresentFlag(8, 0);
    std::vector<uint8_t> sps;
    std::vector<uint8_t> vps;

    mLengthSizeMinus1 = 3;  // NAL length fields are 4 bytes long (3+1)
    mNalArray.clear();
    H265Parser::convertByteStreamToRBSP(srcSps, sps);
    H265Parser::convertByteStreamToRBSP(srcVps, vps);

    BitStream vpsBitstr(vps);
    vpsBitstr.readBits(1); // forbidden_zero_bit
    vpsBitstr.readBits(6); // nal_unit_type
    vpsBitstr.readBits(6); // nuh_layer_id
    vpsBitstr.readBits(3); // nuh_temporal_id_plus1

    vpsBitstr.readBits(4);
    vpsBitstr.readBits(1);
    vpsBitstr.readBits(1);
    vpsBitstr.readBits(6);
    unsigned int vpsMaxSubLayersMinus1 = vpsBitstr.readBits(3);

    BitStream spsBitstr(sps);

    // NALU header
    spsBitstr.readBits(1);                              // forbidden_zero_bit
    spsBitstr.readBits(6);                              // nal_unit_type
    unsigned int spsNuhLayerId = spsBitstr.readBits(6); // nuh_layer_id
    spsBitstr.readBits(3);                              // nuh_temporal_id_plus1

    spsBitstr.readBits(4);                              // sps_video_parametr_set_id  -> not needed

    unsigned int spsMaxSubLayersMinus1 = 0;
    unsigned int spsExtOrMaxSubLayersMinus1 = 0;
    if (spsNuhLayerId == 0)
    {
        // Note: Should not come here for lhv1 bitstreams.
        spsMaxSubLayersMinus1 = spsBitstr.readBits(3);
    }
    else
    {
        spsExtOrMaxSubLayersMinus1 = spsBitstr.readBits(3);
        spsMaxSubLayersMinus1 =  (spsExtOrMaxSubLayersMinus1 == 7) ? vpsMaxSubLayersMinus1 : spsExtOrMaxSubLayersMinus1;
    }
    mNumTemporalLayers = static_cast<uint8_t>(spsMaxSubLayersMinus1 + 1);
    mMinSpatialSegmentationIdc = 0;
    mParallelismType = 0;
}


void LHevcDecoderConfigurationRecord::writeDecConfigRecord(BitStream& bitstr) const
{
    bitstr.writeBits(mConfigurationVersion, 8);
    bitstr.writeBits(0xf, 4);  // reserved = '1111'b
    bitstr.writeBits(mMinSpatialSegmentationIdc, 12);
    bitstr.writeBits(0x3f, 6);  // reserved = '111111'b
    bitstr.writeBits(mParallelismType, 2);
    bitstr.writeBits(0x3, 2);  // reserved = '11'b
    bitstr.writeBits(mNumTemporalLayers, 3);
    bitstr.writeBits(mTemporalIdNested, 1);
    bitstr.writeBits(mLengthSizeMinus1, 2);

    bitstr.writeBits(mNalArray.size(), 8);
    for (const auto& i : mNalArray)
    {
        bitstr.writeBits(i.arrayCompleteness, 1);
        bitstr.writeBits(0, 1);  // reserved = 0
        bitstr.writeBits(static_cast<uint8_t>(i.nalUnitType), 6);
        bitstr.writeBits(i.nalUnits.size(), 16);
        for (const auto& j : i.nalUnits)
        {
            bitstr.writeBits(j.size(), 16);
            bitstr.write8BitsArray(j, j.size());  // write parameter set NAL unit
        }
    }
}

void LHevcDecoderConfigurationRecord::parseConfig(BitStream& bitstr)
{
    mConfigurationVersion = static_cast<uint8_t>(bitstr.readBits(8));
    bitstr.readBits(4);  // reserved = '1111'b
    mMinSpatialSegmentationIdc = static_cast<uint16_t>(bitstr.readBits(12));
    bitstr.readBits(6);  // reserved = '111111'b
    mParallelismType = static_cast<uint8_t>(bitstr.readBits(2));
    bitstr.readBits(2);  // reserved = '11'b
    mNumTemporalLayers = static_cast<uint8_t>(bitstr.readBits(3));
    mTemporalIdNested = static_cast<uint8_t>(bitstr.readBits(1));
    mLengthSizeMinus1 = static_cast<uint8_t>(bitstr.readBits(2));

    const unsigned int numOfArrays = bitstr.readBits(8);
    for (unsigned int i = 0; i < numOfArrays; i++)
    {
        uint8_t arrayCompleteness;
        HevcNalUnitType nalUnitType;
        unsigned int numNalus;

        arrayCompleteness = static_cast<uint8_t>(bitstr.readBits(1));
        bitstr.readBits(1);  // reserved = 0
        nalUnitType = (HevcNalUnitType) bitstr.readBits(6);
        numNalus = bitstr.readBits(16);
        for (unsigned int j = 0; j < numNalus; j++)
        {
            const unsigned int nalSize = bitstr.readBits(16);
            std::vector<uint8_t> nalData;
            bitstr.read8BitsArray(nalData, nalSize);  // read parameter set NAL unit
            addNalUnit(nalData, nalUnitType, arrayCompleteness);
        }
    }
}

unsigned int LHevcDecoderConfigurationRecord::findStartCodeLen(const std::vector<uint8_t>& srcData) const
{
    unsigned int i = 0;

    while ((i + 1) < srcData.size() && srcData.at(i) == 0)
    {
        i++;
    }

    if (i > 1 && srcData.at(i) == 1)
    {
        return i + 1;
    }
    else
    {
        return 0;
    }
}

void LHevcDecoderConfigurationRecord::getParameterSet(std::vector<uint8_t>& byteStream, const HevcNalUnitType nalUnitType) const
{
    for (const auto& array : mNalArray)
    {
        if (array.nalUnitType == nalUnitType)
        {
            for (const auto& data : array.nalUnits)
            {
                // Add start code (0x00000001) before the NAL unit.
                byteStream.push_back(0);
                byteStream.push_back(0);
                byteStream.push_back(0);
                byteStream.push_back(1);
                byteStream.insert(byteStream.end(), data.cbegin(), data.cend());
            }
        }
    }
}
