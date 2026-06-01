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

#ifndef VVCPARSER_HPP
#define VVCPARSER_HPP

#include <cstdint>
#include <vector>
#include "../common/customallocator.hpp"

class VvcParser
{
public:

    
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


    /**
     * @brief The PpsInfo struct
     */
    struct PpsInfo
    {
        int ppsId;
        bool isSubpicMappingPresent;
        std::uint32_t subpicIdLen;
        uint64_t subpicIdBitPos;
        bool startCodeEmulationFlag;
    };

    /**
     * @brief The SpsInfo struct
     */
    struct SpsInfo
    {
        int spsId;
        bool isSubpicMappingPresent;
        std::uint32_t subpicIdLen;
        uint64_t subpicIdBitPos;
        bool startCodeEmulationFlag;

        bool mPtlPresentFlag;

        std::uint16_t mOlsIdx;
        std::uint16_t mAvgFrameRate;
        std::uint8_t mConstantFrameRate;
        std::uint8_t mNumSublayers;

        std::uint8_t mChromaFormatIdc;
        std::uint8_t mBitDepthMinus8;
        std::uint16_t mMaxPictureWidth;
        std::uint16_t mMaxPictureHeight;

        VvcPTLRecord mNativePtl;
    };

    struct OpiInfo{
        bool mOlsinfopresentflag;
        bool mHtidinfopresentflag;
        uint32_t  mOpiolsidx;
        uint32_t  mOpihtidplus1;
    };

    struct VpsInfo{
        uint8_t mVpsId;                                          // u(4), range 1..15
        uint32_t mMaxLayers;
        uint32_t mVpsMaxSubLayers;
        uint32_t mVpsLayerId[64];
        bool mVpsDefaultPtlDpbHrdMaxTidFlag;
        bool mVpsAllIndependentLayersFlag;
        uint32_t mVpsCfgPredDirection[7];
        bool mVpsIndependentLayerFlag[64];
        bool mVpsDirectRefLayerFlag[64][64];
        std::vector<std::vector<uint32_t>> mVpsMaxTidIlRefPicsPlus1;
        bool mVpsEachLayerIsAnOlsFlag;
        uint32_t mVpsOlsModeIdc;
        uint32_t mVpsNumOutputLayerSets;
        bool mVpsOlsOutputLayerFlag[256][64];
        uint32_t mDirectRefLayerIdx[64][64];
        uint32_t mGeneralLayerIdx[64];

        bool mPtPresentFlag[256];
        uint32_t mPtlMaxTemporalId[256];
        uint32_t mOlsPtlIdx[256];

        uint32_t mInterLayerRefIdx[64][64];
        bool mVpsExtensionFlag;
        bool mVpsGeneralHrdParamsPresentFlag;
        bool mVpsSublayerCpbParamsPresentFlag;
        uint32_t mNumOlsTimingHrdParamsMinus1;
        uint32_t mHrdMaxTid[256];
        uint32_t mOlsTimingHrdIdx[256];
        std::vector<uint16_t> mOlsDpbParamsIdx;               // ue(v), index into DPB params, max 255
        std::vector<std::vector<uint8_t>> mOutputLayerIdInOls;   // layer IDs, u(6), max 63
        std::vector<std::vector<uint8_t>> mNumSubLayersInLayerInOLS; // sublayer count, max 8
        std::vector<uint16_t> mMultiLayerOlsIdxToOlsIdx;        // index, max 256

        uint16_t mTotalNumOLSs;                                  // derived, max 257
        uint16_t mNumMultiLayeredOlss;                            // derived, max 256
        uint32_t mMultiLayerOlsIdx[256];
        uint16_t mNumDpbParams;                                // ue(v)+1, max 256
        bool mSublayerDpbParamsPresentFlag;
        std::vector<uint8_t> mDpbMaxTemporalId;                  // u(3), max 7
        std::vector<uint8_t> mTargetOutputLayerIdSet;            // layer IDs, u(6), max 63
        std::vector<uint8_t> mTargetLayerIdSet;                  // layer IDs, u(6), max 63
        uint16_t mTargetOlsIdx;                                  // index, max 256
        std::vector<uint8_t> mNumOutputLayersInOls;              // count, max 64
        std::vector<uint8_t> mNumLayersInOls;                    // count, max 64
        std::vector<std::vector<uint8_t>> mLayerIdInOls;         // layer IDs, u(6), max 63
        std::vector<uint8_t> mOlsDpbBitDepthMinus8;              // ue(v), range 0..2

        std::vector<VvcPTLRecord> mPtlRecords;  // one per vps_num_ptls_minus1+1
        VvcPTLRecord mNativePtl;                // alias for mPtlRecords[0] (backward compat)
    };

    /// @brief Get the lowest OLS index that contains the largest number of output layers among all OLSs
    ///        which contain the highest layer count specified by the VPS.
    /// @param vpsinfo formatted VPS information
    /// @return the OLS index, or 0 if nothing better found
    static uint16_t getLowestOlsIdxWithLargestNumOutputLayers(const VpsInfo& vpsinfo);

    static PpsInfo parsePps(const Vector<uint8_t>& ppsData);

    static SpsInfo parseSps(const Vector<uint8_t>& spsData);

    static OpiInfo parseOpi(const Vector<uint8_t>& opiData);

    static VpsInfo parseVps(const Vector<uint8_t>& vpsData);


};

#endif
