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

#ifndef VTMWRAPPER_HPP
#define VTMWRAPPER_HPP

#include <cstdint>
#include <vector>

class VvcParser
{
public:
    /**
     * @brief The PpsInfo struct
     */
    struct PpsInfo
    {
        int ppsId;
        bool isSubpicMappingPresent;
        std::uint32_t subpicIdLen;
        int subpicIdBitPos;
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
        int subpicIdBitPos;
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

        VvcPTLRecord mNativePtl;
    };

    static PpsInfo parsePps(const std::vector<uint8_t>& ppsData);

    static SpsInfo parseSps(const std::vector<uint8_t>& spsData);
};

#endif
