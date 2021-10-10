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

#include "vvcparser.hpp"

#include <vector>

#include "NALread.h"
#include "VLCReader.h"


VvcParser::PpsInfo VvcParser::parsePps(const std::vector<uint8_t>& ppsData)
{
    PpsInfo ppsInfo = {};

    HLSyntaxReader hlsr;
    InputNALUnit nalu;
    const std::uint16_t NALU_HEADER_LENGTH = 16;
    auto& naluFifo                         = nalu.getBitstream().getFifo();
    naluFifo.resize(ppsData.size());
    std::copy(ppsData.begin(), ppsData.end(), naluFifo.begin());

    read(nalu);  // Convert nalu payload to RBSP and parse nalu header
    hlsr.setBitstream(&nalu.getBitstream());

    PPS pps;
    hlsr.parsePPS(&pps);

    bool subpicMappingInPps = pps.getSubPicIdMappingInPpsFlag();
    if (subpicMappingInPps)
    {
        ppsInfo.isSubpicMappingPresent = true;
        ppsInfo.ppsId                  = pps.getPPSId();
        ppsInfo.subpicIdLen            = pps.getSubPicIdLen();
        // Bit position in RBSP does not include two NAL unit header bytes
        ppsInfo.subpicIdBitPos = pps.getSubpicIdBitPos() - NALU_HEADER_LENGTH;

        const auto emulationPreventionByteLocations = nalu.getBitstream().getEmulationPreventionByteLocation();
        for (const auto threeByteLocation : emulationPreventionByteLocations)
        {
            if (threeByteLocation * 8 < ppsInfo.subpicIdBitPos)
            {
                ppsInfo.startCodeEmulationFlag = true;
            }
        }
    }

    return ppsInfo;
}


VvcParser::SpsInfo::VvcPTLRecord makeVvcPTLRecord(const ProfileTierLevel& ptl, const uint8_t numSublayers)
{
    VvcParser::SpsInfo::VvcPTLRecord vpr = {};

    vpr.mGeneralProfileIdc          = ptl.getProfileIdc();
    vpr.mGeneralTierFlag            = ptl.getTierFlag();
    vpr.mGeneralLevelIdc            = ptl.getLevelIdc();
    vpr.mPtlFrameOnlyConstraintFlag = ptl.getFrameOnlyConstraintFlag();
    vpr.mPtlMultilayerEnabledFlag   = ptl.getMultiLayerEnabledFlag();

    const auto ci       = ptl.getConstraintInfo();
    vpr.mGciPresentFlag = ci->getGciPresentFlag();
    if (vpr.mGciPresentFlag)
    {
        vpr.mIntraOnlyConstraintFlag            = ci->getIntraOnlyConstraintFlag();
        vpr.mAllLayersIndependentConstraintFlag = ci->getAllLayersIndependentConstraintFlag();
        vpr.mOneAuOnlyConstraintFlag            = ci->getPicHeaderInSliceHeaderConstraintFlag();

        // picture format
        vpr.mSixteenMinusMaxBitDepthConstraintIdc   = 16 - ci->getMaxBitDepthConstraintIdc();
        vpr.mThreeMinusMaxChromaFormatConstraintIdc = 3 - ci->getMaxChromaFormatConstraintIdc();

        // NAL unit type related
        vpr.mNoMixedNaluTypesInPicConstraintFlag = ci->getNoMixedNaluTypesInPicConstraintFlag();
        vpr.mNoTrailConstraintFlag               = ci->getNoTrailConstraintFlag();
        vpr.mNoStsaConstraintFlag                = ci->getNoStsaConstraintFlag();
        vpr.mNoRaslConstraintFlag                = ci->getNoRaslConstraintFlag();
        vpr.mNoRadlConstraintFlag                = ci->getNoRadlConstraintFlag();
        vpr.mNoIdrConstraintFlag                 = ci->getNoIdrConstraintFlag();
        vpr.mNoCraConstraintFlag                 = ci->getNoCraConstraintFlag();
        vpr.mNoGdrConstraintFlag                 = ci->getNoGdrConstraintFlag();
        vpr.mNoApsConstraintFlag                 = ci->getNoApsConstraintFlag();
        vpr.mNoIdrRplConstraintFlag              = ci->getNoIdrRplConstraintFlag();

        // tile, slice, subpicture partitioning
        vpr.mOneTilePerPicConstraintFlag          = ci->getOneTilePerPicConstraintFlag();
        vpr.mPicHeaderInSliceHeaderConstraintFlag = ci->getPicHeaderInSliceHeaderConstraintFlag();
        vpr.mOneSlicePerPicConstraintFlag         = ci->getOneSlicePerPicConstraintFlag();
        vpr.mNoRectSliceConstraintFlag            = ci->getNoRectSliceConstraintFlag();
        vpr.mOneSlicePerSubpicConstraintFlag      = ci->getOneSlicePerSubpicConstraintFlag();
        vpr.mNoSubpicInfoConstraintFlag           = ci->getNoSubpicInfoConstraintFlag();

        // CTU and block partitioning
        vpr.mThreeMinusMaxLog2CtuSizeConstraintIdc        = ci->getMaxLog2CtuSizeConstraintIdc() - 3;
        vpr.mNoPartitionConstraintsOverrideConstraintFlag = ci->getNoPartitionConstraintsOverrideConstraintFlag();
        vpr.mNoMttConstraintFlag                          = ci->getNoMttConstraintFlag();
        vpr.mNoQtbttDualTreeIntraConstraintFlag           = ci->getNoQtbttDualTreeIntraConstraintFlag();

        // intra
        vpr.mNoPaletteConstraintFlag = ci->getNoPaletteConstraintFlag();
        vpr.mNoIbcConstraintFlag     = ci->getNoIbcConstraintFlag();
        vpr.mNoIspConstraintFlag     = ci->getNoIspConstraintFlag();
        vpr.mNoMrlConstraintFlag     = ci->getNoMrlConstraintFlag();
        vpr.mNoMipConstraintFlag     = ci->getNoMipConstraintFlag();
        vpr.mNoCclmConstraintFlag    = ci->getNoCclmConstraintFlag();

        // inter
        vpr.mNoRefPicResamplingConstraintFlag   = ci->getNoRprConstraintFlag();
        vpr.mNoResChangeInClvsConstraintFlag    = ci->getNoResChangeInClvsConstraintFlag();
        vpr.mNoWeightedPredictionConstraintFlag = ci->getNoWeightedPredictionConstraintFlag();
        vpr.mNoRefWraparoundConstraintFlag      = ci->getNoRefWraparoundConstraintFlag();
        vpr.mNoTemporalMvpConstraintFlag        = ci->getNoTemporalMvpConstraintFlag();
        vpr.mNoSbtmvpConstraintFlag             = ci->getNoSbtmvpConstraintFlag();
        vpr.mNoAmvrConstraintFlag               = ci->getNoAmvrConstraintFlag();
        vpr.mNoBdofConstraintFlag               = ci->getNoBdofConstraintFlag();
        vpr.mNoSmvdConstraintFlag               = ci->getNoSmvdConstraintFlag();
        vpr.mNoDmvrConstraintFlag               = ci->getNoDmvrConstraintFlag();
        vpr.mNoMmvdConstraintFlag               = ci->getNoMmvdConstraintFlag();
        vpr.mNoAffineMotionConstraintFlag       = ci->getNoAffineMotionConstraintFlag();
        vpr.mNoProfConstraintFlag               = ci->getNoProfConstraintFlag();
        vpr.mNoBcwConstraintFlag                = ci->getNoBcwConstraintFlag();
        vpr.mNoCiipConstraintFlag               = ci->getNoCiipConstraintFlag();
        vpr.mNoGpmConstraintFlag                = ci->getNoGeoConstraintFlag();

        // transform, quantization, residual
        vpr.mNoLumaTransformSize64ConstraintFlag = ci->getNoLumaTransformSize64ConstraintFlag();
        vpr.mNoTransformSkipConstraintFlag       = ci->getNoTransformSkipConstraintFlag();
        vpr.mNoBDPCMConstraintFlag               = ci->getNoBDPCMConstraintFlag();
        vpr.mNoMtsConstraintFlag                 = ci->getNoMtsConstraintFlag();
        vpr.mNoLfnstConstraintFlag               = ci->getNoLfnstConstraintFlag();
        vpr.mNoJointCbCrConstraintFlag           = ci->getNoJointCbCrConstraintFlag();
        vpr.mNoSbtConstraintFlag                 = ci->getNoSbtConstraintFlag();
        vpr.mNoActConstraintFlag                 = ci->getNoActConstraintFlag();
        vpr.mNoExplicitScaleListConstraintFlag   = ci->getNoExplicitScaleListConstraintFlag();
        vpr.mNoDepQuantConstraintFlag            = ci->getNoDepQuantConstraintFlag();
        vpr.mNoSignDataHidingConstraintFlag      = ci->getNoSignDataHidingConstraintFlag();
        vpr.mNoCuQpDeltaConstraintFlag           = ci->getNoCuQpDeltaConstraintFlag();
        vpr.mNoChromaQpOffsetConstraintFlag      = ci->getNoChromaQpOffsetConstraintFlag();

        // loop filter
        vpr.mNoSaoConstraintFlag               = ci->getNoSaoConstraintFlag();
        vpr.mNoAlfConstraintFlag               = ci->getNoAlfConstraintFlag();
        vpr.mNoCCAlfConstraintFlag             = ci->getNoCCAlfConstraintFlag();
        vpr.mNoLmcsConstraintFlag              = ci->getNoLmcsConstraintFlag();
        vpr.mNoLadfConstraintFlag              = ci->getNoLadfConstraintFlag();
        vpr.mNoVirtualBoundariesConstraintFlag = ci->getNoVirtualBoundaryConstraintFlag();
    }

    if (numSublayers > 0)
    {
        vpr.mPtlSublayerLevelPresentFlag.resize(numSublayers - 1);
        for (int i = numSublayers - 2; i >= 0; --i)
        {
            vpr.mPtlSublayerLevelPresentFlag.at(i) = ptl.getSubLayerLevelPresentFlag(i);
        }

        vpr.mSublayerLevelIdc.resize(numSublayers);
        vpr.mSublayerLevelIdc.at(numSublayers - 1) = vpr.mGeneralLevelIdc;
        for (int i = numSublayers - 2; i >= 0; i--)
        {
            if (vpr.mPtlSublayerLevelPresentFlag.at(i))
            {
                vpr.mSublayerLevelIdc.at(i) = ptl.getSubLayerLevelIdc(i);
            }
        }
        for (int i = numSublayers - 2; i >= 0; i--)
        {
            if (!vpr.mPtlSublayerLevelPresentFlag.at(i))
            {
                vpr.mSublayerLevelIdc.at(i) = ptl.getSubLayerLevelIdc(i + 1);
            }
        }
    }

    vpr.mNumSubProfiles = ptl.getNumSubProfile();
    vpr.mGeneralSubProfileIdc.resize(vpr.mNumSubProfiles);
    for (auto j = 0; j < vpr.mNumSubProfiles; ++j)
    {
        vpr.mGeneralSubProfileIdc.at(j) = ptl.getSubProfileIdc(j);
    }

    return vpr;
}


VvcParser::SpsInfo VvcParser::parseSps(const std::vector<uint8_t>& spsData)
{
    SpsInfo spsInfo = {};

    HLSyntaxReader hlsr;
    InputNALUnit nalu;
    const std::uint16_t NALU_HEADER_LENGTH = 16;
    auto& naluFifo                         = nalu.getBitstream().getFifo();
    naluFifo.resize(spsData.size());
    std::copy(spsData.begin(), spsData.end(), naluFifo.begin());

    read(nalu);  // Convert nalu payload to RBSP and parse nalu header
    hlsr.setBitstream(&nalu.getBitstream());

    SPS sps;
    hlsr.parseSPS(&sps);

    spsInfo.isSubpicMappingPresent = true;
    spsInfo.spsId                  = sps.getSPSId();
    spsInfo.subpicIdLen            = sps.getSubPicIdLen();
    // Bit position in RBSP does not include two NAL unit header bytes
    spsInfo.subpicIdBitPos = sps.getSubpicIdBitPos() - NALU_HEADER_LENGTH;

    const auto emulationPreventionByteLocations = nalu.getBitstream().getEmulationPreventionByteLocation();
    for (const auto threeByteLocation : emulationPreventionByteLocations)
    {
        if (threeByteLocation * 8 < spsInfo.subpicIdBitPos)
        {
            spsInfo.startCodeEmulationFlag = true;
        }
    }

    const auto profileTierLevel = sps.getProfileTierLevel();
    if (profileTierLevel == nullptr)
    {
        spsInfo.mPtlPresentFlag = false;
    }
    else
    {
        spsInfo.mPtlPresentFlag    = true;
        spsInfo.mAvgFrameRate      = 0;  // Unspecified average frame rate.
        spsInfo.mConstantFrameRate = 0;
        spsInfo.mNumSublayers      = sps.getMaxTLayers();
        spsInfo.mOlsIdx            = sps.getLayerId();
        spsInfo.mChromaFormatIdc   = sps.getChromaFormatIdc();
        spsInfo.mBitDepthMinus8    = sps.getBitDepth(CHANNEL_TYPE_LUMA) - 8;
        spsInfo.mMaxPictureWidth   = sps.getMaxPicWidthInLumaSamples();
        spsInfo.mMaxPictureHeight  = sps.getMaxPicHeightInLumaSamples();
        spsInfo.mNativePtl         = makeVvcPTLRecord(*profileTierLevel, spsInfo.mNumSublayers);
    }

    return spsInfo;
}
