/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef HEVCDECODERCONFIGRECORD_HPP
#define HEVCDECODERCONFIGRECORD_HPP

#include "decoderconfigrecord.hpp"
#include "hevccommondefs.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/**
 * @brief HEVC decoder configuration record
 */
class HevcDecoderConfigurationRecord : public DecoderConfigurationRecord
{
public:
    HevcDecoderConfigurationRecord();
    ~HevcDecoderConfigurationRecord() override = default;

    /**
     * Read configuration parameters from a SPS NAL unit.
     * @param sps       Sequence Parameter Set data
     * @return True if success, false if there was no input data or processing SPS failed.
     */
    bool makeConfigFromSPS(const Vector<std::uint8_t>& sps);

    /**
     * Add NAL unit to the NAL unit array
     * @param sps               Sequence Parameter Set data
     * @param nalUnitType       Type of the NAL unit
     * @param arrayCompleteness TBD
     */
    void addNalUnit(const Vector<std::uint8_t>& sps, HevcNalUnitType nalUnitType, bool arrayCompleteness);

    /**
     * Serialize decoder configuration to ISOBMFF::BitStream.
     * @param [in,out] bitstr Decoder configuration appended is appended to
     * bitstr.
     */
    void writeDecConfigRecord(ISOBMFF::BitStream& bitstr) const;

    /**
     * Deserialize HevcDecoderConfigurationRecord from ISOBMFF::BitStream
     * @param [in,out] bitstr ISOBMFF::BitStream including decoder configuration
     * data. ISOBMFF::BitStream position is modified accordingly.
     */
    void parseConfig(ISOBMFF::BitStream& bitstr);

    /**
     * @brief Append one parameter set of type nalUnitType.
     * @details If NAL unit of type nalUnitType is not found, nothing is appended.
     * @param [in,out] byteStream  Vector where the parameter set is appended
     * @param          nalUnitType NAL unit type to append
     */
    void getOneParameterSet(Vector<std::uint8_t>& byteStream, HevcNalUnitType nalUnitType) const;

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

    /* @brief Returns configuration parameter map for this record */
    void getConfigurationMap(ConfigurationMap& aMap) const override;

    /**
     * @return Returns chroma_format_idc value.
     */
    std::uint8_t getChromaFormat() const;

    /**
     * @return Return general constraint flags.
     */
    Vector<std::uint8_t> getGeneralConstraintFlags() const;

    /**
     * @return Return general_profile_idc value.
     */
    std::uint8_t getGeneralProfileIdc() const;

    /**
     * @return Return general_profile_compatibility_flag value.
     */
    std::uint32_t getGeneralProfileCompatibilityFlags() const;

    /**
     * @return Return general_level_idc value.
     */
    std::uint8_t getGeneralLevelIdc() const;

private:
    // Member variables can be found from the High Efficiency Video Coding (HEVC)
    // specification

    struct NALArray
    {
        bool arrayCompleteness      = false;
        HevcNalUnitType nalUnitType = HevcNalUnitType::INVALID;
        Vector<Vector<std::uint8_t>> nalList;
    };

    std::uint8_t mConfigurationVersion;
    std::uint8_t mGeneralProfileSpace;
    std::uint8_t mGeneralTierFlag;
    std::uint8_t mGeneralProfileIdc;
    std::uint32_t mGeneralProfileCompatibilityFlags;
    Vector<std::uint8_t> mGeneralConstraintIndicatorFlags;
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
    Vector<NALArray> mNalArray;
};

#endif /* end of include guard: HEVCDECODERCONFIGRECORD_HPP */
