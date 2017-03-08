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

#include "avcdecoderconfigrecord.hpp"

#include "avccommondefs.hpp"
#include "avcparser.hpp"
#include "bitstream.hpp"

#include <cassert>

AvcDecoderConfigurationRecord::AvcDecoderConfigurationRecord() :
    mConfigurationVersion(1),
    mAvcProfileIndication(66),
    mProfileCompatibility(128),
    mAvcLevelIndication(30),
    mLengthSizeMinus1(3),

    mChromaFormat(0),
    mBitDepthLumaMinus8(0),
    mBitDepthChromaMinus8(0),

    mPicWidth(0),
    mPicHeight(0),

    mNumOfArrays(0),
    mNalArray()
{
}

void AvcDecoderConfigurationRecord::makeConfigFromSPS(const std::vector<uint8_t>& srcSps)
{
    mLengthSizeMinus1 = 3;  // NAL length fields are 4 bytes long (3+1)
    mNumOfArrays = 0;
    mNalArray.clear();

    // Parse picture width and height from SPS data.
    AvcParser::SPSConfigValues spsConfigValues;
    AvcParser::parseSPSConfigValues(srcSps, spsConfigValues);

    mChromaFormat         = spsConfigValues.mChromaFormat;
    mBitDepthLumaMinus8   = spsConfigValues.mBitDepthLumaMinus8;
    mBitDepthChromaMinus8 = spsConfigValues.mBitDepthChromaMinus8;
    mPicWidth             = spsConfigValues.mPicWidthPx;
    mPicHeight            = spsConfigValues.mPicHeightPx;
}

void AvcDecoderConfigurationRecord::addNalUnit(const std::vector<uint8_t>& nalUnit, const AvcNalUnitType nalUnitType,
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
        mNumOfArrays++;
    }

    startCodeLen = findStartCodeLen(nalUnit);
    tmpNalUnit.insert(tmpNalUnit.begin(), nalUnit.cbegin() + startCodeLen, nalUnit.cend()); // Copy NAL data excluding potential start code.

    // Add NAL unit to the NAL unit array.
    nalArray->nalList.push_back(tmpNalUnit);
}

void AvcDecoderConfigurationRecord::writeDecConfigRecord(BitStream& bitstr) const
{
    bitstr.writeBits(mConfigurationVersion, 8);
    bitstr.writeBits(mAvcProfileIndication, 8);
    bitstr.writeBits(mProfileCompatibility, 8);
    bitstr.writeBits(mAvcLevelIndication, 8);

    bitstr.writeBits(0xff, 6);  // reserved = '111111'b
    bitstr.writeBits(mLengthSizeMinus1, 2);

    // SPS NALS
    bitstr.writeBits(0xff, 3);  // reserved = '111'b
    const NALArray* nalArray = getNALArray(AvcNalUnitType::SPS);
    int count = (nalArray ? nalArray->nalList.size() : 0);

    assert(count < (1<<6)); // Must fit into 5 bits
    bitstr.writeBits(count, 5);

    if (count)
    {
        for (const auto& nal : nalArray->nalList)
        {
            bitstr.writeBits(nal.size(), 16);
            bitstr.write8BitsArray(nal, nal.size());
        }
    }

    // PPS NALS
    nalArray = getNALArray(AvcNalUnitType::PPS);
    count = (nalArray ? nalArray->nalList.size() : 0);

    assert(count < (1<<9)); // Must fit into 8 bits
    bitstr.writeBits(count, 8);

    if (count)
    {
        for (const auto& nal : nalArray->nalList)
        {
            bitstr.writeBits(nal.size(), 16);
            bitstr.write8BitsArray(nal, nal.size());
        }
    }

    if (mAvcProfileIndication == 100 || mAvcProfileIndication == 110 ||
        mAvcProfileIndication == 122 || mAvcProfileIndication == 144)
    {
        bitstr.writeBits(0xff, 6);  // reserved = '111111'b
        bitstr.writeBits(mChromaFormat, 2);
        bitstr.writeBits(0xff, 5);  // reserved = '11111'b
        bitstr.writeBits(mBitDepthLumaMinus8, 3);
        bitstr.writeBits(0xff, 5);  // reserved = '11111'b
        bitstr.writeBits(mBitDepthChromaMinus8, 3);

        // SPS EXT NALS
        nalArray = getNALArray(AvcNalUnitType::SPS_EXT);
        count = (nalArray ? nalArray->nalList.size() : 0);

        assert(count < (1<<9)); // Must fit into 8 bits
        bitstr.writeBits(count, 8);

        if (count)
        {
            for (const auto& nal : nalArray->nalList)
            {
                bitstr.writeBits(nal.size(), 16);
                bitstr.write8BitsArray(nal, nal.size());
            }
        }
    }
}

void AvcDecoderConfigurationRecord::parseConfig(BitStream& bitstr)
{
    mConfigurationVersion = static_cast<uint8_t>(bitstr.readBits(8));
    mAvcProfileIndication = static_cast<uint8_t>(bitstr.readBits(8));
    mProfileCompatibility = static_cast<uint8_t>(bitstr.readBits(8));
    mAvcLevelIndication   = static_cast<uint8_t>(bitstr.readBits(8));

    bitstr.readBits(6);  // reserved = '111111'b
    mLengthSizeMinus1     = static_cast<uint8_t>(bitstr.readBits(2));

    // SPS NALS
    bitstr.readBits(3);  // reserved = '111'b
    int count = static_cast<uint8_t>(bitstr.readBits(5));

    for (int nal = 0; nal < count; ++nal)
    {
        int nalSize = bitstr.readBits(16);

        std::vector<uint8_t> nalData;
        nalData.clear();
        bitstr.read8BitsArray(nalData, nalSize);  // read parameter set NAL unit
        addNalUnit(nalData, AvcNalUnitType::SPS);
    }

    // PPS NALS
    count = static_cast<uint8_t>(bitstr.readBits(8));

    for (int nal = 0; nal < count; ++nal)
    {
        int nalSize = bitstr.readBits(16);

        std::vector<uint8_t> nalData;
        nalData.clear();
        bitstr.read8BitsArray(nalData, nalSize);  // read parameter set NAL unit
        addNalUnit(nalData, AvcNalUnitType::PPS);
    }

    // Stop reading if there is no more data (ignore possibly wrong mAvcProfileIndication).
    if (bitstr.getSize() == bitstr.getPos())
    {
        return;
    }

    if (mAvcProfileIndication == 100 || mAvcProfileIndication == 110 ||
        mAvcProfileIndication == 122 || mAvcProfileIndication == 144)
    {
        bitstr.readBits(6);  // reserved = '111111'b
        mChromaFormat         = static_cast<uint8_t>(bitstr.readBits(2));
        bitstr.readBits(5);  // reserved = '11111'b
        mBitDepthLumaMinus8   = static_cast<uint8_t>(bitstr.readBits(3));
        bitstr.readBits(5);  // reserved = '11111'b
        mBitDepthChromaMinus8 = static_cast<uint8_t>(bitstr.readBits(3));

        // SPS EXT NALS
        count = static_cast<uint8_t>(bitstr.readBits(8));

        for (int nal = 0; nal < count; ++nal)
        {
            int nalSize = bitstr.readBits(16);

            std::vector<uint8_t> nalData;
            nalData.clear();
            bitstr.read8BitsArray(nalData, nalSize);  // read parameter set NAL unit
            addNalUnit(nalData, AvcNalUnitType::SPS_EXT);
        }
    }
}

unsigned int AvcDecoderConfigurationRecord::findStartCodeLen(const std::vector<uint8_t>& srcData) const
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

const AvcDecoderConfigurationRecord::NALArray* AvcDecoderConfigurationRecord::getNALArray(AvcNalUnitType nalUnitType) const
{
    for (const auto& array : mNalArray)
    {
        if (array.nalUnitType == nalUnitType)
        {
            return &array; // Found
        }
    }

    return nullptr; // Not found
}

void AvcDecoderConfigurationRecord::getOneParameterSet(std::vector<uint8_t>& byteStream, const AvcNalUnitType nalUnitType) const
{
    const NALArray* nalArray = getNALArray(nalUnitType);

    if (nalArray && nalArray->nalList.size() > 0)
    {
        // Add start code (0x00000001) before the NAL unit.
        byteStream.push_back(0);
        byteStream.push_back(0);
        byteStream.push_back(0);
        byteStream.push_back(1);
        byteStream.insert(byteStream.end(), nalArray->nalList.at(0).cbegin(), nalArray->nalList.at(0).cend());
    }
}

uint16_t AvcDecoderConfigurationRecord::getPicWidth() const
{
    return mPicWidth;
}

uint16_t AvcDecoderConfigurationRecord::getPicHeight() const
{
    return mPicHeight;
}
