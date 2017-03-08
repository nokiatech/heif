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

#ifndef LHEVCDECODERCONFIGRECORD_HPP
#define LHEVCDECODERCONFIGRECORD_HPP

#include "hevccommondefs.hpp"
#include <vector>

class BitStream;

/**
 * @brief Layered HEVC decoder configuration record
 */
class LHevcDecoderConfigurationRecord
{
public:
    LHevcDecoderConfigurationRecord();
    ~LHevcDecoderConfigurationRecord() = default;

    /**
     * Add NAL unit to the NAL unit array
     * @param sps               Sequence Parameter Set data
     * @param nalUnitType       Type of the NAL unit
     * @param arrayCompleteness TBD
     */
    void addNalUnit(const std::vector<std::uint8_t>& sps, HevcNalUnitType nalUnitType, std::uint8_t arrayCompleteness);

    /**
     * Read configuration parameters from a SPS NAL unit.
     * @param sps       Sequence Parameter Set data
     */
    // void makeConfigFromSPS(const std::vector<std::uint8_t>& sps);
    void makeConfigFromSPS(const std::vector<std::uint8_t>& sps, const std::vector<std::uint8_t>& vps);

    /**
     * Serialize decoder configuration to BitStream.
     * @param [in,out] bitstr Decoder configuration appended is appended to bitstr.
     */
    void writeDecConfigRecord(BitStream& bitstr) const;

    /**
     * Deserialize LHevcDecoderConfigurationRecord from BitStream
     * @param [in,out] bitstr BitStream including decoder configuration data. BitStream position is modified accordingly.
     */
    void parseConfig(BitStream& bitstr);

    /**
     * @brief Append one parameter set of type nalUnitType.
     * @details If NAL unit of type nalUnitType is not found, nothing is appended.
     * @param [in,out] byteStream  Vector where the parameter set is appended
     * @param          nalUnitType NAL unit type to append
     */
    void getParameterSet(std::vector<std::uint8_t>& byteStream, HevcNalUnitType nalUnitType) const;

private:
    // Member variables can be found from the High Efficiency Video Coding (HEVC) specification

    struct NALArray
    {
        std::uint8_t arrayCompleteness = 0;
        HevcNalUnitType nalUnitType = HevcNalUnitType::INVALID;
        std::vector<std::vector<std::uint8_t>> nalUnits;
    };

    std::uint8_t mConfigurationVersion;
    std::uint16_t mMinSpatialSegmentationIdc;
    std::uint8_t mParallelismType;
    std::uint8_t mNumTemporalLayers;
    std::uint8_t mTemporalIdNested;
    std::uint8_t mLengthSizeMinus1;
    std::vector<NALArray> mNalArray;

    /**
     * @brief Returns the number of bytes in start code
     * @details Start code consists of any number of zero bytes (0x00) followed by a one (0x01) byte.
     * @param srcData NAL data to search from
     * @return Number of bytes in start code
     */
    unsigned int findStartCodeLen(const std::vector<std::uint8_t>& srcData) const;
};

#endif /* end of include guard: LHEVCDECODERCONFIGRECORD_HPP */
