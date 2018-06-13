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

#include "avcdecoderconfigrecord.hpp"

#include "avccommondefs.hpp"
#include "avcparser.hpp"
#include "bitstream.hpp"
#include "nalutil.hpp"

#include <cassert>

AvcDecoderConfigurationRecord::AvcDecoderConfigurationRecord()
    : mConfigurationVersion(1)
    , mAvcProfileIndication(66)
    , mProfileCompatibility(128)
    , mAvcLevelIndication(30)
    , mLengthSizeMinus1(3)
    , mChromaFormat(0)
    , mBitDepthLumaMinus8(0)
    , mBitDepthChromaMinus8(0)
    , mPicWidth(0)
    , mPicHeight(0)
    , mNalArray()
{
}

bool AvcDecoderConfigurationRecord::makeConfigFromSPS(const Vector<uint8_t>& sps)
{
    const Vector<uint8_t> rbspData = convertByteStreamToRBSP(sps);
    BitStream bitstr(rbspData);
    SPSConfigValues spsConfig;
    // ignore the first byte indicating type
    bitstr.read8Bits();
    if (!parseSPS(bitstr, spsConfig))
    {
        return false;
    }

    mAvcProfileIndication = spsConfig.profile_idc;
    mProfileCompatibility = spsConfig.profile_compatibility;
    mAvcLevelIndication   = spsConfig.level_idc;
    mChromaFormat         = static_cast<uint8_t>(spsConfig.chroma_format_idc);
    mBitDepthLumaMinus8   = static_cast<uint8_t>(spsConfig.bit_depth_luma_minus8);
    mBitDepthChromaMinus8 = static_cast<uint8_t>(spsConfig.bit_depth_chroma_minus8);

    mPicWidth  = static_cast<uint16_t>((spsConfig.pic_width_in_mbs_minus1 + 1) * 16 -
                                      (spsConfig.frame_crop_left_offset + spsConfig.frame_crop_right_offset) * 2);
    mPicHeight = static_cast<uint16_t>((2 - static_cast<uint32_t>(spsConfig.frame_mbs_only_flag)) *
                                           (spsConfig.pic_height_in_map_units_minus1 + 1) * 16 -
                                       (spsConfig.frame_crop_top_offset + spsConfig.frame_crop_bottom_offset) * 2);

    return true;
}

void AvcDecoderConfigurationRecord::addNalUnit(const Vector<uint8_t>& nalUnit, const AvcNalUnitType nalUnitType)
{
    NALArray* nalArray = nullptr;
    Vector<uint8_t> tmpNalUnit;
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
        nalArrayTmp.nalUnitType = nalUnitType;
        mNalArray.push_back(nalArrayTmp);
        nalArray = &mNalArray.back();
    }

    startCodeLen = findStartCodeLen(nalUnit);
    tmpNalUnit.insert(tmpNalUnit.begin(), nalUnit.cbegin() + static_cast<int>(startCodeLen),
                      nalUnit.cend());  // copy NAL data excluding potential start code

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
    unsigned int count       = static_cast<unsigned int>(nalArray ? nalArray->nalList.size() : 0);

    assert(count < (1 << 6));  // Must fit into 5 bits
    bitstr.writeBits(count, 5);

    if (count)
    {
        for (const auto& nal : nalArray->nalList)
        {
            bitstr.writeBits(static_cast<unsigned int>(nal.size()), 16);
            bitstr.write8BitsArray(nal, static_cast<unsigned int>(nal.size()));
        }
    }

    // PPS NALS
    nalArray = getNALArray(AvcNalUnitType::PPS);
    count    = static_cast<unsigned int>(nalArray ? nalArray->nalList.size() : 0);

    assert(count < (1 << 9));  // Must fit into 8 bits
    bitstr.writeBits(count, 8);

    if (count)
    {
        for (const auto& nal : nalArray->nalList)
        {
            bitstr.writeBits(static_cast<unsigned int>(nal.size()), 16);
            bitstr.write8BitsArray(nal, static_cast<unsigned int>(nal.size()));
        }
    }

    if (mAvcProfileIndication == 100 || mAvcProfileIndication == 110 || mAvcProfileIndication == 122 ||
        mAvcProfileIndication == 144)
    {
        bitstr.writeBits(0xff, 6);  // reserved = '111111'b
        bitstr.writeBits(mChromaFormat, 2);
        bitstr.writeBits(0xff, 5);  // reserved = '11111'b
        bitstr.writeBits(mBitDepthLumaMinus8, 3);
        bitstr.writeBits(0xff, 5);  // reserved = '11111'b
        bitstr.writeBits(mBitDepthChromaMinus8, 3);

        // SPS EXT NALS
        nalArray = getNALArray(AvcNalUnitType::SPS_EXT);
        count    = static_cast<unsigned int>(nalArray ? nalArray->nalList.size() : 0);

        assert(count < (1 << 9));  // Must fit into 8 bits
        bitstr.writeBits(count, 8);

        if (count)
        {
            for (const auto& nal : nalArray->nalList)
            {
                bitstr.writeBits(static_cast<unsigned int>(nal.size()), 16);
                bitstr.write8BitsArray(nal, static_cast<unsigned int>(nal.size()));
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
    mLengthSizeMinus1 = static_cast<uint8_t>(bitstr.readBits(2));

    // SPS NALS
    bitstr.readBits(3);  // reserved = '111'b
    unsigned int count = static_cast<uint8_t>(bitstr.readBits(5));

    for (unsigned int nal = 0; nal < count; ++nal)
    {
        unsigned int nalSize = bitstr.readBits(16);

        Vector<uint8_t> nalData;
        nalData.clear();
        bitstr.read8BitsArray(nalData, nalSize);  // read parameter set NAL unit
        addNalUnit(nalData, AvcNalUnitType::SPS);
    }

    // PPS NALS
    count = static_cast<uint8_t>(bitstr.readBits(8));

    for (unsigned int nal = 0; nal < count; ++nal)
    {
        unsigned int nalSize = bitstr.readBits(16);

        Vector<uint8_t> nalData;
        nalData.clear();
        bitstr.read8BitsArray(nalData, nalSize);  // read parameter set NAL unit
        addNalUnit(nalData, AvcNalUnitType::PPS);
    }

    // Stop reading if there is no more data (ignore possibly wrong mAvcProfileIndication).
    if (bitstr.getSize() == bitstr.getPos())
    {
        return;
    }

    if (mAvcProfileIndication == 100 || mAvcProfileIndication == 110 || mAvcProfileIndication == 122 ||
        mAvcProfileIndication == 144)
    {
        bitstr.readBits(6);  // reserved = '111111'b
        mChromaFormat = static_cast<uint8_t>(bitstr.readBits(2));
        bitstr.readBits(5);  // reserved = '11111'b
        mBitDepthLumaMinus8 = static_cast<uint8_t>(bitstr.readBits(3));
        bitstr.readBits(5);  // reserved = '11111'b
        mBitDepthChromaMinus8 = static_cast<uint8_t>(bitstr.readBits(3));

        // SPS EXT NALS
        count = static_cast<uint8_t>(bitstr.readBits(8));

        for (unsigned int nal = 0; nal < count; ++nal)
        {
            unsigned int nalSize = bitstr.readBits(16);

            Vector<uint8_t> nalData;
            nalData.clear();
            bitstr.read8BitsArray(nalData, nalSize);  // Read parameter set NAL unit.
            addNalUnit(nalData, AvcNalUnitType::SPS_EXT);
        }
    }
}

const AvcDecoderConfigurationRecord::NALArray*
AvcDecoderConfigurationRecord::getNALArray(AvcNalUnitType nalUnitType) const
{
    for (const auto& array : mNalArray)
    {
        if (array.nalUnitType == nalUnitType)
        {
            return &array;  // Found
        }
    }

    return nullptr;  // Not found
}

void AvcDecoderConfigurationRecord::getOneParameterSet(Vector<uint8_t>& byteStream,
                                                       const AvcNalUnitType nalUnitType) const
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

std::uint8_t AvcDecoderConfigurationRecord::getConfigurationVersion() const
{
    return mConfigurationVersion;
}

void AvcDecoderConfigurationRecord::setPicWidth(std::uint16_t picWidth)
{
    mPicWidth = picWidth;
}

void AvcDecoderConfigurationRecord::setPicHeight(std::uint16_t picHeight)
{
    mPicHeight = picHeight;
}

void AvcDecoderConfigurationRecord::setConfigurationVersion(std::uint8_t configurationVersion)
{
    mConfigurationVersion = configurationVersion;
}

std::uint8_t AvcDecoderConfigurationRecord::getAvcProfileIndication() const
{
    return mAvcProfileIndication;
}

void AvcDecoderConfigurationRecord::setAvcProfileIndication(std::uint8_t avcProfileIndication)
{
    mAvcProfileIndication = avcProfileIndication;
}

std::uint8_t AvcDecoderConfigurationRecord::getProfileCompatibility() const
{
    return mProfileCompatibility;
}

void AvcDecoderConfigurationRecord::setProfileCompatibility(std::uint8_t profileCompatibility)
{
    mProfileCompatibility = profileCompatibility;
}

std::uint8_t AvcDecoderConfigurationRecord::getAvcLevelIndication() const
{
    return mAvcLevelIndication;
}

void AvcDecoderConfigurationRecord::setAvcLevelIndication(std::uint8_t avcLevelIndication)
{
    mAvcLevelIndication = avcLevelIndication;
}

std::uint8_t AvcDecoderConfigurationRecord::getLengthSizeMinus1() const
{
    return mLengthSizeMinus1;
}

void AvcDecoderConfigurationRecord::setLengthSizeMinus1(std::uint8_t lengthSizeMinus1)
{
    mLengthSizeMinus1 = lengthSizeMinus1;
}

std::uint8_t AvcDecoderConfigurationRecord::getChromaFormat() const
{
    return mChromaFormat;
}

void AvcDecoderConfigurationRecord::setChromaFormat(std::uint8_t chromaFormat)
{
    mChromaFormat = chromaFormat;
}

std::uint8_t AvcDecoderConfigurationRecord::getBitDepthLumaMinus8() const
{
    return mBitDepthLumaMinus8;
}

void AvcDecoderConfigurationRecord::setBitDepthLumaMinus8(std::uint8_t bitDepthLumaMinus8)
{
    mBitDepthLumaMinus8 = bitDepthLumaMinus8;
}

std::uint8_t AvcDecoderConfigurationRecord::getBitDepthChromaMinus8() const
{
    return mBitDepthChromaMinus8;
}

void AvcDecoderConfigurationRecord::setBitDepthChromaMinus8(std::uint8_t bitDepthChromaMinus8)
{
    mBitDepthChromaMinus8 = bitDepthChromaMinus8;
}

void AvcDecoderConfigurationRecord::getConfigurationMap(ConfigurationMap& aMap) const
{
    Vector<std::uint8_t> sps;
    Vector<std::uint8_t> pps;
    getOneParameterSet(sps, AvcNalUnitType::SPS);
    getOneParameterSet(pps, AvcNalUnitType::PPS);

    aMap.clear();
    aMap.insert({DecoderParameterType::AVC_SPS, move(sps)});
    aMap.insert({DecoderParameterType::AVC_PPS, move(pps)});
}