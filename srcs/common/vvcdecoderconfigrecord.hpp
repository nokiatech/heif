/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

#include "customallocator.hpp"
#include "decoderconfigrecord.hpp"
#include "vvccommondefs.hpp"

#include <cstdint>
#include <vector>

#include "../vvc-parser/vvcparser.hpp"

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
    /**
     * @brief Sentinel "nuh_layer_id" value meaning "all layers".
     * @details Pass this to getNALU / getOneParameterSet / getAllParameterSets /
     *          getConfigurationMap to indicate that the caller wants every
     *          parameter set across every layer (useful for assembling a
     *          full multi-layer VVC bitstream that contains parameter sets
     *          for both base and dependent layers).
     *          For getNALU which returns a single NAL, ALL_LAYERS still
     *          returns the first matching NAL.
     */
    static constexpr std::uint16_t ALL_LAYERS = 0xFFFF;

    VvcDecoderConfigurationRecord();
    ~VvcDecoderConfigurationRecord() override = default;

    /// Parameters conveyed by the MinimizedImageBox compact header.
    struct ConfigValues {
        uint32_t item_id;
        uint8_t chroma_subsampling;
        bool high_bit_depth_flag;
        uint8_t bit_depth_minus9;
        uint32_t width_minus1;
        uint32_t height_minus1;
        uint8_t gainmap_chroma_subsampling;
        uint8_t gainmap_high_bit_depth_flag;
        uint8_t gainmap_bit_depth_minus9;
        uint32_t gainmap_width_minus1;
        uint32_t gainmap_height_minus1;
    };

    /**
     * Read configuration parameters from a SPS NAL unit.
     * @param sps       Sequence Parameter Set data
     * @return True if success, false if there was no input data or processing SPS failed.
     */
    bool makeConfigFromSPS(const Vector<std::uint8_t>& sps);

    /**
     * Read config parameters from a VPS NAL unit.
     * @param vpsInfo VVC Parser VPS info structure
     */
    void makeConfigFromVPS(const VvcParser::VpsInfo& vpsInfo);

    /**
     * Add NAL unit to the NAL unit array
     * @param nalu              Nal Unit data
     * @param nalUnitType       Type of the NAL unit
     * @param arrayCompleteness TBD
     */
    void addNalUnit(const Vector<std::uint8_t>& nalu, VvcNalUnitType nalUnitType, bool arrayCompleteness);

    /**
     * Serialize decoder configuration to ISOBMFF::BitStream.
     * @param [in,out] bitstr Decoder configuration is appended to
     */
    void writeDecConfigRecord(ISOBMFF::BitStream& bitstr) const override;

    /**
     * Serialize compact decoder configuration to ISOBMFF::BitStream.
     * @param [in,out] bitstr Compact decoder configuration is appended to
     */
    void writeCompactDecConfigRecord(ISOBMFF::BitStream& bitstr) const final;

    /**
     * Deserialize VvcDecoderConfigurationRecord from ISOBMFF::BitStream
     * @param [in,out] bitstr ISOBMFF::BitStream including decoder configuration
     * data. ISOBMFF::BitStream position is modified accordingly.
     */
    void parseConfig(ISOBMFF::BitStream& bitstr);

        /**
     * @brief Parse mini related low overhead Compact decoder configuration
     *
     * @param [in, out] bitstr
     */
    void parseCompactConfig(ISOBMFF::BitStream& bitstr, const ConfigValues& vals);

    /**
     * @brief Append one parameter set of type nalUnitType.
     * @details If NAL unit of type nalUnitType is not found, nothing is appended.
     * @param [in,out] byteStream  Vector where the parameter set is appended
     * @param          nalUnitType NAL unit type to append
     */
    void getOneParameterSet(Vector<std::uint8_t>& byteStream, VvcNalUnitType nalUnitType, const uint16_t& nuh_layer_id = 0) const;

    /**
     * @brief Get all occurences of parametertype from this config..
     * @param [in,out] byteStream  Vector where the parameter sets are appended
     * @param          nalUnitType NAL unit type to append
     */
    void getAllParameterSets(Vector<Vector<std::uint8_t>>& byteStream, VvcNalUnitType nalUnitType, const uint16_t& nuh_layer_id = 0) const;

    /**
     * @brief Returns the first NALU of the type
     * @param [in] nalutype Nal unit type to get
     * @param [in] nuh_layer_id Optional layer id for mapping of parameter set <-> layer
     * @note the layer id is currently set by the processing order during parsing
     * @return Vector with nalu payload. Empty if NALU not found. [ mNalArray.NalArray.NalList.at(0) ]
     */
    Vector<uint8_t> getNALU(const VvcNalUnitType nalutype, const uint16_t& nuh_layer_id = 0) const;

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

    /** @brief Returns configuration parameter map for this record 
     *  @bug Doesn't retrieve all parameter sets, in case of multiple of same type (other than APS)
     *  @bug Might have issues with complex layers / configs / files / items.
    */
    void getConfigurationMap(ConfigurationMap& aMap, const uint16_t& nuh_layer_id = 0) const override;

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

    bool getMultiLayerFlag() const {return mMultiLayerFlag;}

    /** @brief Get the LengthSize value
     *  @return mLengthSizeMinus1 PLUS ONE.
     *  @note THIS IS THE ACTUAL LENGTHSIZE NOT MINUS1
     */
    uint8_t getLengthSize() const {return mLengthSizeMinus1 + 1;}

    /**
     * @brief Returns true if a parsed VPS NAL has been observed and its
     *        derived OLS / layer information has been cached on the record.
     * @details Populated either by parseCompactConfig() (mini path) or by
     *          addNalUnit(VPS_NUT, ...) when the bytes look parseable. Once
     *          set, downstream code can consult getCachedVpsInfo() instead of
     *          re-running VvcParser::parseVps over the stored NAL bytes.
     */
    bool hasCachedVpsInfo() const { return mHasVpsInfo; }

    /**
     * @brief Return the cached VpsInfo. Caller must check hasCachedVpsInfo()
     *        first; the returned reference is only meaningful in that case.
     */
    const VvcParser::VpsInfo& getCachedVpsInfo() const { return mVpsInfo; }

    /**
     * @brief Cache a VpsInfo struct on the record. Idempotent: subsequent
     *        calls overwrite. Used by code paths that have already parsed a
     *        VPS NAL and want to make the result available without forcing a
     *        second parse later.
     */
    void setCachedVpsInfo(const VvcParser::VpsInfo& vpsInfo)
    {
        mVpsInfo    = vpsInfo;
        mHasVpsInfo = true;
    }

    /// @todo why are some flags uint8_t?
    struct VvcPTLRecord
    {
        ///@todo why is this not parsed?
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
        /// Parallel to nalList: nuh_layer_id (lower 6 bits of NAL header byte 0)
        /// of every NAL stored in nalList. Captured at addNalUnit() time from the
        /// real NAL header so SPS/PPS belonging to different layers can be
        /// disambiguated, instead of relying on positional ordering.
        Vector<std::uint16_t> nuhLayerIds;
    };

    std::uint8_t mConfigurationVersion = 0;
    std::uint8_t mLengthSizeMinus1      = 0;
    std::uint8_t mPtlPresentFlag        = 0;

    std::uint16_t mOlsIdx            = 0;
    std::uint16_t mAvgFrameRate      = 0;
    std::uint8_t  mConstantFrameRate = 0;
    std::uint8_t  mNumSublayers      = 0;

    std::uint8_t  mChromaFormatIdc   = 0;
    std::uint8_t  mBitDepthMinus8    = 0;
    std::uint16_t mMaxPictureWidth   = 0;
    std::uint16_t mMaxPictureHeight  = 0;

    VvcPTLRecord mNativePtl;

    Vector<NALArray> mNalArray;

    bool mMultiLayerFlag = false;

    /// Cached VPS info, populated when a VPS NAL is observed (see
    /// addNalUnit / parseCompactConfig). Lets downstream code reuse the rich
    /// OLS / layer table without re-parsing the raw VPS bytes.
    VvcParser::VpsInfo mVpsInfo{};
    bool mHasVpsInfo = false;

    /// Cached OPI info, populated when an OPI NAL is observed in addNalUnit().
    /// OPI is optional in VVC; absence is signalled by mHasOpiInfo == false.
    VvcParser::OpiInfo mOpiInfo{};
    bool mHasOpiInfo = false;

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

public:
    /**
     * @brief Inject a PTL record and set ptl_present_flag = 1.
     *        Primarily used by tests that need to exercise write/parse round-trips
     *        without going through makeConfigFromSPS.
     * @param ptl            The PTL record to store.
     * @param numSublayers   Value stored in mNumSublayers (must match ptl.mSublayerLevelIdc size).
     */
    void setNativePtl(const VvcPTLRecord& ptl, std::uint8_t numSublayers)
    {
        mNativePtl      = ptl;
        mNumSublayers   = numSublayers;
        mPtlPresentFlag = 1;
    }

    /**
     * @brief Return the parsed/stored PTL record.
     */
    const VvcPTLRecord& getNativePtl() const { return mNativePtl; }

    /** @brief Returns true when ptl_present_flag is set in the decoder config record. */
    bool getPtlPresentFlag() const { return mPtlPresentFlag != 0; }
};

#endif /* end of include guard: VVCDECODERCONFIGRECORD_HPP */
