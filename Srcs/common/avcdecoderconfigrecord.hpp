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

#ifndef AVCDECODERCONFIGRECORD_HPP
#define AVCDECODERCONFIGRECORD_HPP

#include "avccommondefs.hpp"
#include <vector>

class BitStream;

/**
 * @brief AVC decoder configuration record
 * @details Member variable descriptions can be found from the Advanced Video Coding (AVC) specification.
 */
class AvcDecoderConfigurationRecord
{
public:
    AvcDecoderConfigurationRecord();
    ~AvcDecoderConfigurationRecord() = default;

    /**
     * Read configuration parameters from a SPS NAL unit.
     * @param sps       Sequence Parameter Set data
     */
    void makeConfigFromSPS(const std::vector<std::uint8_t>& sps);

    /**
     * Add NAL unit to the NAL unit array
     * @param sps               Sequence Parameter Set data
     * @param nalUnitType       Type of the NAL unit
     * @param arrayCompleteness TBD
     */
    void addNalUnit(const std::vector<std::uint8_t>& sps, AvcNalUnitType nalUnitType, std::uint8_t arrayCompleteness = 0);

    /**
     * Serialize decoder configuration to BitStream.
     * @param [in,out] bitstr Decoder configuration appended is appended to bitstr.
     */
    void writeDecConfigRecord(BitStream& bitstr) const;

    /**
     * Deserialize AvcDecoderConfigurationRecord from BitStream
     * @param [in,out] bitstr BitStream including decoder configuration data. BitStream position is modified accordingly.
     */
    void parseConfig(BitStream& bitstr);

    /**
     * @brief Append one parameter set of type nalUnitType.
     * @details If NAL unit of type nalUnitType is not found, nothing is appended.
     * @param [in,out] byteStream  Vector where the parameter set is appended
     * @param          nalUnitType NAL unit type to append
     */
    void getOneParameterSet(std::vector<std::uint8_t>& byteStream, AvcNalUnitType nalUnitType) const;

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

private:
    // Member variable descriptions can be found from the Advanced Video Coding (AVC) specification.
    struct NALArray
    {
        std::uint8_t arrayCompleteness = 0;
        AvcNalUnitType nalUnitType = AvcNalUnitType::INVALID;
        std::vector<std::vector<std::uint8_t>> nalList;
    };

    std::uint8_t mConfigurationVersion;
    std::uint8_t mAvcProfileIndication;
    std::uint8_t mProfileCompatibility;
    std::uint8_t mAvcLevelIndication;
    std::uint8_t mLengthSizeMinus1;

    std::uint8_t mChromaFormat;
    std::uint8_t mBitDepthLumaMinus8;
    std::uint8_t mBitDepthChromaMinus8;

    std::uint16_t mPicWidth;
    std::uint16_t mPicHeight;

    std::uint8_t mNumOfArrays;
    std::vector<NALArray> mNalArray;

    /**
     * @brief Get NALArray for this NAL unit type
     * @param nalUnitType NAL unit type to append
     * @return Pointer to NALArray or nullptr if not found
     */
    const NALArray* getNALArray(AvcNalUnitType nalUnitType) const;

    /**
     * @brief Returns the number of bytes in start code
     * @details Start code consists of any number of zero bytes (0x00) followed by a one (0x01) byte.
     * @param srcData NAL data to search from
     * @return Number of bytes in start code
     */
    unsigned int findStartCodeLen(const std::vector<std::uint8_t>& srcData) const;
};

#endif /* end of include guard: AVCDECODERCONFIGRECORD_HPP */
