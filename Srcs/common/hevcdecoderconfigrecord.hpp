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

#ifndef HEVCDECODERCONFIGRECORD_HPP
#define HEVCDECODERCONFIGRECORD_HPP

#include "commondefs.hpp"
#include <vector>

class BitStream;

/**
 * @brief HEVC decoder configuration record
 */
class HevcDecoderConfigurationRecord
{
public:
    HevcDecoderConfigurationRecord();
    ~HevcDecoderConfigurationRecord() = default;

    /**
     * Read configuration parameters from a SPS NAL unit.
     * @param sps       Sequence Parameter Set data
     * @param frameRate Frame rate of video
     */
    void makeConfigFromSPS(const std::vector<std::uint8_t>& sps, float frameRate);

    /**
     * Add NAL unit to the NAL unit array
     * @param sps               Sequence Parameter Set data
     * @param nalUnitType       Type of the NAL unit
     * @param arrayCompleteness TBD
     */
    void addNalUnit(const std::vector<std::uint8_t>& sps, HevcNalUnitType nalUnitType, std::uint8_t arrayCompleteness);

    /**
     * Serialize decoder configuration to BitStream.
     * @param [in,out] bitstr Decoder configuration appended is appended to bitstr.
     */
    void writeDecConfigRecord(BitStream& bitstr) const;

    /**
     * Deserialize HevcDecoderConfigurationRecord from BitStream
     * @param [in,out] bitstr BitStream including decoder configuration data. BitStream position is modified accordingly.
     */
    void parseConfig(BitStream& bitstr);

    /**
     * @brief Append one parameter set of type nalUnitType.
     * @details If NAL unit of type nalUnitType is not found, nothing is appended.
     * @param [in,out] byteStream  Vector where the parameter set is appended
     * @param          nalUnitType NAL unit type to append
     */
    void getOneParameterSet(std::vector<std::uint8_t>& byteStream, HevcNalUnitType nalUnitType) const;

    /**
     * @pre makeConfigFromSPS has been called successfully.
     * @return Picture width in pixels.
     */
    std::uint16_t getPicWidth() const;

    /**
     * @pre makeConfigFromSPS has been called successfully.
     * @return Picture height in pixels.
     */
    std::uint16_t getPicHeight() const;

    /**
     * @pre makeConfigFromSPS or parseConfig has been called successfully.
     * @return Average frame rate
     */
    std::uint16_t getAvgFrameRate() const;

private:
    // Member variables can be found from the High Efficiency Video Coding (HEVC) specification

    struct NALArray
    {
        std::uint8_t arrayCompleteness = 0;
        HevcNalUnitType nalUnitType = HevcNalUnitType::INVALID;
        std::vector<std::vector<std::uint8_t>> nalList;
    };

    std::uint8_t mConfigurationVersion;
    std::uint8_t mGeneralProfileSpace;
    std::uint8_t mGeneralTierFlag;
    std::uint8_t mGeneralProfileIdc;
    std::uint32_t mGeneralProfileCompatibilityFlags;
    std::vector<std::uint8_t> mGeneralConstraintIndicatorFlags;
    std::uint8_t mGeneralLevelIdc;
    std::uint16_t mMinSpatialSegmentationIdc;
    std::uint8_t mParallelismType;
    std::uint8_t mChromaFormat;
    std::uint16_t mPicWidthInLumaSamples;   // this is not written to an mp4 file
    std::uint16_t mPicHeightInLumaSamples;  // this is not written to an mp4 file
    std::uint16_t mConfWinLeftOffset;       // this is not written to an mp4 file
    std::uint16_t mConfWinRightOffset;      // this is not written to an mp4 file
    std::uint16_t mConfWinTopOffset;        // this is not written to an mp4 file
    std::uint16_t mConfWinBottomOffset;     // this is not written to an mp4 file
    std::uint8_t mBitDepthLumaMinus8;
    std::uint8_t mBitDepthChromaMinus8;
    std::uint16_t mAvgFrameRate;
    std::uint8_t mConstantFrameRate;
    std::uint8_t mNumTemporalLayers;
    std::uint8_t mTemporalIdNested;
    std::uint8_t mLengthSizeMinus1;
    std::uint8_t mNumOfArrays;
    std::vector<NALArray> mNalArray;

    /**
     * @brief Returns the number of bytes in start code
     * @details Start code consists of any number of zero bytes (0x00) followed by a one (0x01) byte.
     * @param srcData NAL data to search from
     * @return Number of bytes in start code
     */
    unsigned int findStartCodeLen(const std::vector<std::uint8_t>& srcData) const;
};

#endif /* end of include guard: HEVCDECODERCONFIGRECORD_HPP */
