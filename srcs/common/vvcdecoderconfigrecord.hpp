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

#ifndef VVCDECODERCONFIGRECORD_HPP
#define VVCDECODERCONFIGRECORD_HPP

#include "decoderconfigrecord.hpp"
#include "vvccommondefs.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/**
 * @brief VVC decoder configuration record
 */
class VvcDecoderConfigurationRecord : public DecoderConfigurationRecord
{
public:
    VvcDecoderConfigurationRecord();
    ~VvcDecoderConfigurationRecord() override = default;

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
    void addNalUnit(const Vector<std::uint8_t>& sps, VvcNalUnitType nalUnitType, bool arrayCompleteness);

    /**
     * Serialize decoder configuration to ISOBMFF::BitStream.
     * @param [in,out] bitstr Decoder configuration appended is appended to
     * bitstr.
     */
    void writeDecConfigRecord(ISOBMFF::BitStream& bitstr) const;

    /**
     * Deserialize VvcDecoderConfigurationRecord from ISOBMFF::BitStream
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
    void getOneParameterSet(Vector<std::uint8_t>& byteStream, VvcNalUnitType nalUnitType) const;

    /**
     * @pre makeConfigFromSPS has been called successfully.
     * @return Picture maximum width in pixels.
     */
    std::uint16_t getMaxPictureWidth() const;

    /**
     * @pre makeConfigFromSPS has been called successfully.
     * @return Picture maximum height in pixels.
     */
    std::uint16_t getMaxPictureHeight() const;

    /**
     * @pre makeConfigFromSPS or parseConfig has been called successfully.
     * @return Average frame rate
     */
    std::uint16_t getAvgFrameRate() const;

    /** @brief Returns configuration parameter map for this record */
    void getConfigurationMap(ConfigurationMap& aMap) const override;

    /**
     * @return Returns chroma_format_idc value.
     */
    std::uint8_t getChromaFormat() const;

    /**
     * @return Return general contraint flags.
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

    struct VvcPTLRecord
    {
        // std::uint8_t mNumBytesConstraintInfo;
        std::uint8_t mGeneralProfileIdc;
        std::uint8_t mGeneralTierFlag;
        std::uint8_t mGeneralLevelIdc;
        std::uint8_t mPtlFrameOnlyConstraintFlag;
        std::uint8_t mPtlMultilayerEnabledFlag;

        // general_constraints_info flags
        bool mGciPresentFlag;

        // general
        bool mIntraOnlyConstraintFlag;
        bool mAllLayersIndependentConstraintFlag;
        bool mOneAuOnlyConstraintFlag;

        // picture format
        uint32_t mSixteenMinusMaxBitDepthConstraintIdc;
        uint32_t mThreeMinusMaxChromaFormatConstraintIdc;

        // NAL unit type related
        bool mNoMixedNaluTypesInPicConstraintFlag;
        bool mNoTrailConstraintFlag;
        bool mNoStsaConstraintFlag;
        bool mNoRaslConstraintFlag;
        bool mNoRadlConstraintFlag;
        bool mNoIdrConstraintFlag;
        bool mNoCraConstraintFlag;
        bool mNoGdrConstraintFlag;
        bool mNoApsConstraintFlag;
        bool mNoIdrRplConstraintFlag;

        // tile, slice, subpicture partitioning
        bool mOneTilePerPicConstraintFlag;
        bool mPicHeaderInSliceHeaderConstraintFlag;
        bool mOneSlicePerPicConstraintFlag;
        bool mNoRectSliceConstraintFlag;
        bool mOneSlicePerSubpicConstraintFlag;
        bool mNoSubpicInfoConstraintFlag;

        // CTU and block partitioning
        uint32_t mThreeMinusMaxLog2CtuSizeConstraintIdc;
        bool mNoPartitionConstraintsOverrideConstraintFlag;
        bool mNoMttConstraintFlag;
        bool mNoQtbttDualTreeIntraConstraintFlag;

        // intra
        bool mNoPaletteConstraintFlag;
        bool mNoIbcConstraintFlag;
        bool mNoIspConstraintFlag;
        bool mNoMrlConstraintFlag;
        bool mNoMipConstraintFlag;
        bool mNoCclmConstraintFlag;

        // inter
        bool mNoRefPicResamplingConstraintFlag;
        bool mNoResChangeInClvsConstraintFlag;
        bool mNoWeightedPredictionConstraintFlag;
        bool mNoRefWraparoundConstraintFlag;
        bool mNoTemporalMvpConstraintFlag;
        bool mNoSbtmvpConstraintFlag;
        bool mNoAmvrConstraintFlag;
        bool mNoBdofConstraintFlag;
        bool mNoSmvdConstraintFlag;
        bool mNoDmvrConstraintFlag;
        bool mNoMmvdConstraintFlag;
        bool mNoAffineMotionConstraintFlag;
        bool mNoProfConstraintFlag;
        bool mNoBcwConstraintFlag;
        bool mNoCiipConstraintFlag;
        bool mNoGpmConstraintFlag;

        // transform, quantization, residual
        bool mNoLumaTransformSize64ConstraintFlag;
        bool mNoTransformSkipConstraintFlag;
        bool mNoBDPCMConstraintFlag;
        bool mNoMtsConstraintFlag;
        bool mNoLfnstConstraintFlag;
        bool mNoJointCbCrConstraintFlag;
        bool mNoSbtConstraintFlag;
        bool mNoActConstraintFlag;
        bool mNoExplicitScaleListConstraintFlag;
        bool mNoDepQuantConstraintFlag;
        bool mNoSignDataHidingConstraintFlag;
        bool mNoCuQpDeltaConstraintFlag;
        bool mNoChromaQpOffsetConstraintFlag;

        // loop filter
        bool mNoSaoConstraintFlag;
        bool mNoAlfConstraintFlag;
        bool mNoCCAlfConstraintFlag;
        bool mNoLmcsConstraintFlag;
        bool mNoLadfConstraintFlag;
        bool mNoVirtualBoundariesConstraintFlag;
        // general_constraints_info flags end

        std::vector<bool> mPtlSublayerLevelPresentFlag;
        std::vector<std::uint8_t> mSublayerLevelIdc;
        std::uint8_t mNumSubProfiles;
        std::vector<std::uint32_t> mGeneralSubProfileIdc;
    };

private:
    // Member variable descriptions can be found from the Versatile Video Coding (VVC) specification

    struct NALArray
    {
        bool arrayCompleteness     = false;
        VvcNalUnitType nalUnitType = VvcNalUnitType::INVALID;
        Vector<Vector<std::uint8_t>> nalList;
    };

    std::uint8_t mConfigurationVersion;
    std::uint8_t mLengthSizeMinus1;
    std::uint8_t mPtlPresentFlag;

    std::uint16_t mOlsIdx;
    std::uint16_t mAvgFrameRate;
    std::uint8_t mConstantFrameRate;
    std::uint8_t mNumSublayers;

    std::uint8_t mChromaFormatIdc;
    std::uint8_t mBitDepthMinus8;
    std::uint16_t mMaxPictureWidth;
    std::uint16_t mMaxPictureHeight;

    VvcPTLRecord mNativePtl;

    Vector<NALArray> mNalArray;

    /**
     * Serialize VvcPTLRecord to ISOBMFF::BitStream.
     * @param [in,out] bitstr BitStream VvcPTLRecord is appended to.
     */
    void writeVvcPTLRecord(ISOBMFF::BitStream& bitstr,
                           const VvcPTLRecord& vvcPTLRecord,
                           std::uint8_t numSublayers) const;

    /**
     * Deserialize VvcPTLRecord from ISOBMFF::BitStream
     * @param [in,out] bitstr BitStream including VvcPTLRecord
     *                        data. ISOBMFF::BitStream position is modified accordingly.
     */
    VvcPTLRecord parseVvcPTLRecord(ISOBMFF::BitStream& bitstr, const std::uint8_t numSublayers);
};

#endif /* end of include guard: VVCDECODERCONFIGRECORD_HPP */
