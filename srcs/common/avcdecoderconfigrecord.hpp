/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

#ifndef AVCDECODERCONFIGRECORD_HPP
#define AVCDECODERCONFIGRECORD_HPP

#include "avccommondefs.hpp"
#include "decoderconfigrecord.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/**
 * @brief AVC decoder configuration record
 * @details Member variable descriptions can be found from the Advanced Video Coding (AVC) specification.
 */
class AvcDecoderConfigurationRecord : public DecoderConfigurationRecord
{
public:
    AvcDecoderConfigurationRecord();
    ~AvcDecoderConfigurationRecord() = default;

    /**
     * Read configuration parameters from a SPS NAL unit. You still
     * need to explicitly add the NAL unit(s) with addNalUnit.
     * @param sps       Sequence Parameter Set data
     */
    bool makeConfigFromSPS(const Vector<std::uint8_t>& sps);

    /**
     * Add NAL unit to the NAL unit array
     * @param sps               Sequence Parameter Set data
     * @param nalUnitType       Type of the NAL unit
     */
    void addNalUnit(const Vector<std::uint8_t>& sps, AvcNalUnitType nalUnitType);

    /**
     * Serialize decoder configuration to ISOBMFF::BitStream.
     * @param [in,out] bitstr Decoder configuration appended is appended to bitstr.
     */
    void writeDecConfigRecord(ISOBMFF::BitStream& bitstr) const;

    /**
     * Deserialize AvcDecoderConfigurationRecord from ISOBMFF::BitStream
     * @param [in,out] bitstr ISOBMFF::BitStream including decoder configuration data. ISOBMFF::BitStream position is modified accordingly.
     */
    void parseConfig(ISOBMFF::BitStream& bitstr);

    /**
     * @brief Append one parameter set of type nalUnitType.
     * @details If NAL unit of type nalUnitType is not found, nothing is appended.
     * @param [in,out] byteStream  Vector where the parameter set is appended
     * @param          nalUnitType NAL unit type to append
     */
    void getOneParameterSet(Vector<std::uint8_t>& byteStream, AvcNalUnitType nalUnitType) const;

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
     * @brief Set picture width in pixels.
     */
    void setPicWidth(std::uint16_t picWidth);

    /**
     * @brief Set picture heigth in pixels.
     */
    void setPicHeight(std::uint16_t picHeight);

    std::uint8_t getConfigurationVersion() const;
    void setConfigurationVersion(std::uint8_t configurationVersion);

    std::uint8_t getAvcProfileIndication() const;
    void setAvcProfileIndication(std::uint8_t avcProfileIndication);

    std::uint8_t getProfileCompatibility() const;
    void setProfileCompatibility(std::uint8_t profileCompatibility);

    std::uint8_t getAvcLevelIndication() const;
    void setAvcLevelIndication(std::uint8_t avcLevelIndication);

    std::uint8_t getLengthSizeMinus1() const;
    void setLengthSizeMinus1(std::uint8_t lengthSizeMinus1);

    std::uint8_t getChromaFormat() const;
    void setChromaFormat(std::uint8_t chromaFormat);

    std::uint8_t getBitDepthLumaMinus8() const;
    void setBitDepthLumaMinus8(std::uint8_t bitDepthLumaMinus8);

    std::uint8_t getBitDepthChromaMinus8() const;
    void setBitDepthChromaMinus8(std::uint8_t bitDepthChromaMinus8);

    /* @brief Returns configuration parameter map for this record */
    virtual void getConfigurationMap(ConfigurationMap& aMap) const override;

private:
    // Member variable descriptions can be found from the Advanced Video Coding (AVC) specification.

    struct NALArray
    {
        AvcNalUnitType nalUnitType = AvcNalUnitType::INVALID;
        Vector<Vector<std::uint8_t>> nalList;
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

    Vector<NALArray> mNalArray;

    /**
     * @brief Get NALArray for this NAL unit type
     * @param nalUnitType NAL unit type to append
     * @return Pointer to NALArray or nullptr if not found
     */
    const NALArray* getNALArray(AvcNalUnitType nalUnitType) const;
};

#endif /* end of include guard: AVCDECODERCONFIGRECORD_HPP */
