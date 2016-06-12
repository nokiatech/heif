/* Copyright (c) 2015, Nokia Technologies Ltd.
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

#include "hevcdecoderconfigrecord.hpp"
#include "bitstream.hpp"
#include "h265parser.hpp"

HevcDecoderConfigurationRecord::HevcDecoderConfigurationRecord() :
    mConfigurationVersion(1),
    mGeneralProfileSpace(0),
    mGeneralTierFlag(0),
    mGeneralProfileIdc(0),
    mGeneralProfileCompatibilityFlags(0),
    mGeneralConstraintIndicatorFlags(6, 0),
    mGeneralLevelIdc(0),
    mMinSpatialSegmentationIdc(0),
    mParallelismType(0),
    mChromaFormat(0),
    mPicWidthInLumaSamples(0),
    mPicHeightInLumaSamples(0),
    mConfWinLeftOffset(0),
    mConfWinRightOffset(0),
    mConfWinTopOffset(0),
    mConfWinBottomOffset(0),
    mBitDepthLumaMinus8(0),
    mBitDepthChromaMinus8(0),
    mAvgFrameRate(0),
    mConstantFrameRate(0),
    mNumTemporalLayers(0),
    mTemporalIdNested(0),
    mLengthSizeMinus1(0),
    mNumOfArrays(0),
    mNalArray()
{
}

void HevcDecoderConfigurationRecord::makeConfigFromSPS(const std::vector<uint8_t>& srcSps, float frameRate)
{
    unsigned int maxNumSubLayersMinus1;
    std::vector<int> subLayerProfilePresentFlag(8, 0);
    std::vector<int> subLayerLevelPresentFlag(8, 0);
    std::vector<uint8_t> sps;

    /// @todo Verify this does what is intended. Casts look a bit unusual?
    if (frameRate > ((float) 0xffff / 256))
    {
        frameRate = (float) 0xffff / 256;
    }
    mAvgFrameRate = static_cast<uint16_t>(frameRate * 256 + 0.5);
    mConstantFrameRate = 0;

    mLengthSizeMinus1 = 3;  // NAL length fields are 4 bytes long (3+1)
    mNumOfArrays = 0;
    mNalArray.clear();
    H265Parser::convertByteStreamToRBSP(srcSps, sps);

    BitStream bitstr(sps);

    // NALU header
    bitstr.readBits(1);                              // forbidden_zero_bit
    bitstr.readBits(6);                              // nal_unit_type
    bitstr.readBits(6);                              // nuh_layer_id
    bitstr.readBits(3);                              // nuh_temporal_id_plus1

    bitstr.readBits(4);                              // sps_video_parametr_set_id  -> not needed
    maxNumSubLayersMinus1 = bitstr.readBits(3);      // sps_max_sub_layers_minus1
    mNumTemporalLayers = static_cast<uint8_t>(maxNumSubLayersMinus1 + 1);
    mTemporalIdNested = static_cast<uint8_t>(bitstr.readBits(1)); // sps_temporal_id_nesting_flag

    // start profile_tier_level parsing

    mGeneralProfileSpace = static_cast<uint8_t>(bitstr.readBits(2)); // general_profile_space
    mGeneralTierFlag = static_cast<uint8_t>(bitstr.readBits(1));     // general_tier_flag
    mGeneralProfileIdc = static_cast<uint8_t>(bitstr.readBits(5));   // general_profile_idc
    mGeneralProfileCompatibilityFlags = 0;
    // general_profile_compatibility_flags (32 flags)
    for (int i = 0; i < 32; i++)
    {
        mGeneralProfileCompatibilityFlags = (mGeneralProfileCompatibilityFlags << 1) | bitstr.readBits(1);
    }
    // constrain_flags (48 flags)
    for (int i = 0; i < 6; i++)
    {
        mGeneralConstraintIndicatorFlags.at(i) = static_cast<uint8_t>(bitstr.readBits(8));
    }
    mGeneralLevelIdc = static_cast<uint8_t>(bitstr.readBits(8));  // general_level_idc
    for (unsigned int i = 0; i < maxNumSubLayersMinus1; i++)
    {
        subLayerProfilePresentFlag.at(i) = bitstr.readBits(1);  // sub_layer_profile_present_flag
        subLayerLevelPresentFlag.at(i) = bitstr.readBits(1);    // sub_layer_level_present_flag
    }
    if (maxNumSubLayersMinus1 > 0)
    {
        for (int i = maxNumSubLayersMinus1; i < 8; i++)
        {
            bitstr.readBits(2); // reserved_zero_2bits
        }
    }
    // The following sub-layer syntax element are not needed in the decoder configuration record
    for (unsigned int i = 0; i < maxNumSubLayersMinus1; i++)
    {
        if (subLayerProfilePresentFlag.at(i))
        {
            bitstr.readBits(2);  // sub_layer_profile_space[i]
            bitstr.readBits(1);  // sub_layer_tier_flag[i]
            bitstr.readBits(5);  // sub_layer_profile_idc[i]
            for (int j = 0; j < 32; j++)
            {
                bitstr.readBits(1);  // sub_layer_profile_compatibility_flag[i][j]
            }
            for (int j = 0; j < 6; j++)
            {
                bitstr.readBits(8);  // Constraint flags
            }
        }
        if (subLayerLevelPresentFlag.at(i))
        {
            bitstr.readBits(8);  // sub_level_idc[i]
        }
    }

    // end profile_tier_level parsing

    bitstr.readExpGolombCode();  // sps_seq_parameter_set_id
    mChromaFormat = static_cast<uint8_t>(bitstr.readExpGolombCode());  // chroma_format_idc
    if (mChromaFormat == 3)
    {
        bitstr.readBits(1);  // separate_colour_plane_flag
    }
    mPicWidthInLumaSamples = static_cast<uint16_t>(bitstr.readExpGolombCode());  // pic_width_in_luma_samples
    mPicHeightInLumaSamples = static_cast<uint16_t>(bitstr.readExpGolombCode());  // pic_height_in_luma_samples

    if (bitstr.readBits(1))  // conformance_window_flag
    {
        mConfWinLeftOffset = static_cast<uint16_t>(bitstr.readExpGolombCode());  // conf_win_left_offset
        mConfWinRightOffset = static_cast<uint16_t>(bitstr.readExpGolombCode());  // conf_win_right_offset
        mConfWinTopOffset = static_cast<uint16_t>(bitstr.readExpGolombCode());  // conf_win_top_offset
        mConfWinBottomOffset = static_cast<uint16_t>(bitstr.readExpGolombCode());  // conf_win_bottom_offset
    }
    else
    {
        mConfWinLeftOffset = 0;
        mConfWinRightOffset = 0;
        mConfWinTopOffset = 0;
        mConfWinBottomOffset = 0;
    }

    mBitDepthLumaMinus8 = static_cast<uint8_t>(bitstr.readExpGolombCode());    // bit_depth_luma_minus8
    mBitDepthChromaMinus8 = static_cast<uint8_t>(bitstr.readExpGolombCode());  // bit_depth_chroma_minus8
    bitstr.readExpGolombCode();  // log2_max_pic_order_cnt_lsb_minus4

    mMinSpatialSegmentationIdc = 0;
    mParallelismType = 0;
}


void HevcDecoderConfigurationRecord::addNalUnit(const std::vector<uint8_t>& nalUnit, const HevcNalUnitType nalUnitType,
    const uint8_t arrayCompleteness)
{
    NALArray* nalArray = nullptr;
    std::vector<uint8_t> tmpNalUnit;
    unsigned int startCodeLen;

    // find array for the given NAL unit type
    for (auto& i : mNalArray)
    {
        if (static_cast<uint8_t>(nalUnitType) == static_cast<uint8_t>(i.nalUnitType))
        {
            nalArray = &i;
            break;
        }
    }

    // if an array is not present for the NAL unit type, create one
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
    tmpNalUnit.insert(tmpNalUnit.begin(), nalUnit.cbegin() + startCodeLen, nalUnit.cend()); // copy NAL data excluding potential start code

    // add NAL unit to the NAL unit array
    nalArray->nalList.push_back(tmpNalUnit);
}

void HevcDecoderConfigurationRecord::writeDecConfigRecord(BitStream& bitstr) const
{
    bitstr.writeBits(mConfigurationVersion, 8);
    bitstr.writeBits(mGeneralProfileSpace, 2);
    bitstr.writeBits(mGeneralTierFlag, 1);
    bitstr.writeBits(mGeneralProfileIdc, 5);
    bitstr.writeBits(mGeneralProfileCompatibilityFlags, 32);
    for (int i = 0; i < 6; i++)
    {
        bitstr.writeBits(mGeneralConstraintIndicatorFlags.at(i), 8);
    }
    bitstr.writeBits(mGeneralLevelIdc, 8);
    bitstr.writeBits(0xf, 4);  // reserved = '1111'b
    bitstr.writeBits(mMinSpatialSegmentationIdc, 12);
    bitstr.writeBits(0x3f, 6);  // reserved = '111111'b
    bitstr.writeBits(mParallelismType, 2);
    bitstr.writeBits(0x3f, 6);  // reserved = '111111'b
    bitstr.writeBits(mChromaFormat, 2);
    bitstr.writeBits(0x1f, 5);  // reserved = '11111'b
    bitstr.writeBits(mBitDepthLumaMinus8, 3);
    bitstr.writeBits(0x1f, 5);  // reserved = '11111'b
    bitstr.writeBits(mBitDepthChromaMinus8, 3);
    bitstr.writeBits(mAvgFrameRate, 16);
    bitstr.writeBits(mConstantFrameRate, 2);
    bitstr.writeBits(mNumTemporalLayers, 3);
    bitstr.writeBits(mTemporalIdNested, 1);
    bitstr.writeBits(mLengthSizeMinus1, 2);

    bitstr.writeBits(mNumOfArrays, 8);
    for (const auto& i : mNalArray)
    {
        bitstr.writeBits(i.arrayCompleteness, 1);
        bitstr.writeBits(0, 1);  // reserved = 0
        bitstr.writeBits(static_cast<uint8_t>(i.nalUnitType), 6);
        bitstr.writeBits(i.nalList.size(), 16);
        for (const auto& j : i.nalList)
        {
            bitstr.writeBits(j.size(), 16);
            bitstr.write8BitsArray(j, j.size());  // write parameter set NAL unit
        }
    }

}

void HevcDecoderConfigurationRecord::parseConfig(BitStream& bitstr)
{
    unsigned int numOfArrays;

    mConfigurationVersion = static_cast<uint8_t>(bitstr.readBits(8));
    mGeneralProfileSpace = static_cast<uint8_t>(bitstr.readBits(2));
    mGeneralTierFlag = static_cast<uint8_t>(bitstr.readBits(1));
    mGeneralProfileIdc = static_cast<uint8_t>(bitstr.readBits(5));
    mGeneralProfileCompatibilityFlags = bitstr.readBits(32);
    for (int i = 0; i < 6; i++)
    {
        mGeneralConstraintIndicatorFlags.at(i) = static_cast<uint8_t>(bitstr.readBits(8));
    }
    mGeneralLevelIdc = static_cast<uint8_t>(bitstr.readBits(8));
    bitstr.readBits(4);  // reserved = '1111'b
    mMinSpatialSegmentationIdc = static_cast<uint16_t>(bitstr.readBits(12));
    bitstr.readBits(6);  // reserved = '111111'b
    mParallelismType = static_cast<uint8_t>(bitstr.readBits(2));
    bitstr.readBits(6);  // reserved = '111111'b
    mChromaFormat = static_cast<uint8_t>(bitstr.readBits(2));
    bitstr.readBits(5);  // reserved = '11111'b
    mBitDepthLumaMinus8 = static_cast<uint8_t>(bitstr.readBits(3));
    bitstr.readBits(5);  // reserved = '11111'b
    mBitDepthChromaMinus8 = static_cast<uint8_t>(bitstr.readBits(3));
    mAvgFrameRate = static_cast<uint16_t>(bitstr.readBits(16));
    mConstantFrameRate = static_cast<uint8_t>(bitstr.readBits(2));
    mNumTemporalLayers = static_cast<uint8_t>(bitstr.readBits(3));
    mTemporalIdNested = static_cast<uint8_t>(bitstr.readBits(1));
    mLengthSizeMinus1 = static_cast<uint8_t>(bitstr.readBits(2));

    numOfArrays = bitstr.readBits(8);
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
            std::vector<uint8_t> nalData;
            unsigned int nalSize;

            nalSize = bitstr.readBits(16);
            nalData.clear();
            bitstr.read8BitsArray(nalData, nalSize);  // read parameter set NAL unit
            addNalUnit(nalData, nalUnitType, arrayCompleteness);
        }
    }
}

unsigned int HevcDecoderConfigurationRecord::findStartCodeLen(const std::vector<uint8_t>& srcData) const
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

void HevcDecoderConfigurationRecord::getOneParameterSet(std::vector<uint8_t>& byteStream, const HevcNalUnitType nalUnitType) const
{
    for (const auto& array : mNalArray)
    {
        if (array.nalUnitType == nalUnitType && array.nalList.size() > 0)
        {
            // add start code (0x00000001) before the NAL unit
            byteStream.push_back(0);
            byteStream.push_back(0);
            byteStream.push_back(0);
            byteStream.push_back(1);
            byteStream.insert(byteStream.end(), array.nalList.at(0).cbegin(), array.nalList.at(0).cend());
        }
    }

}

uint16_t HevcDecoderConfigurationRecord::getPicWidth() const
{
    static const std::vector<int> subWidthC = { 1, 2, 2, 1 };
    unsigned int width;

    width = mPicWidthInLumaSamples - subWidthC.at(mChromaFormat) * (mConfWinLeftOffset + mConfWinRightOffset);

    return static_cast<uint16_t>(width);
}

uint16_t HevcDecoderConfigurationRecord::getPicHeight() const
{
    static const std::vector<int> subHeightC = { 1, 2, 1, 1 };
    unsigned int height;

    height = mPicHeightInLumaSamples - subHeightC.at(mChromaFormat) * (mConfWinTopOffset + mConfWinBottomOffset);

    return static_cast<uint16_t>(height);
}

uint16_t HevcDecoderConfigurationRecord::getAvgFrameRate() const
{
    return mAvgFrameRate;
}
