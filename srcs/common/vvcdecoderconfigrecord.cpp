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

#include "vvcdecoderconfigrecord.hpp"

#include <stdexcept>

#include "bitstream.hpp"
#include "nalutil.hpp"
#include "log.hpp"
#include "vvcparser.hpp"
#include <cstddef>
#include <utility>

VvcDecoderConfigurationRecord::VvcDecoderConfigurationRecord()
    : mNalArray()
{
}

bool VvcDecoderConfigurationRecord::makeConfigFromSPS(const Vector<uint8_t>& srcSps)
{
    const Vector<uint8_t> data(srcSps.begin(), srcSps.end());

    VvcParser::SpsInfo spsInfo = VvcParser::parseSps(data);

    mConfigurationVersion = 1;

    ///this is supposed to be read from config
    //mLengthSizeMinus1     = 3;

    mPtlPresentFlag = spsInfo.mPtlPresentFlag;
    if (mPtlPresentFlag)
    {
        mAvgFrameRate      = 0;  // Unspecified average frame rate.
        mConstantFrameRate = 0;
        mNumSublayers      = spsInfo.mNumSublayers;
        mOlsIdx            = spsInfo.mOlsIdx;
        mChromaFormatIdc   = spsInfo.mChromaFormatIdc;
        mBitDepthMinus8    = spsInfo.mBitDepthMinus8;
        mMaxPictureWidth   = spsInfo.mMaxPictureWidth;
        mMaxPictureHeight  = spsInfo.mMaxPictureHeight;

        mNativePtl.mGeneralProfileIdc          = spsInfo.mNativePtl.mGeneralProfileIdc;
        mNativePtl.mGeneralTierFlag            = spsInfo.mNativePtl.mGeneralTierFlag;
        mNativePtl.mGeneralLevelIdc            = spsInfo.mNativePtl.mGeneralLevelIdc;
        mNativePtl.mPtlFrameOnlyConstraintFlag = spsInfo.mNativePtl.mPtlFrameOnlyConstraintFlag;
        mNativePtl.mPtlMultilayerEnabledFlag   = spsInfo.mNativePtl.mPtlMultilayerEnabledFlag;

        mNativePtl.mGciPresentFlag                     = spsInfo.mNativePtl.mGciPresentFlag;
        mNativePtl.mIntraOnlyConstraintFlag            = spsInfo.mNativePtl.mIntraOnlyConstraintFlag;
        mNativePtl.mAllLayersIndependentConstraintFlag = spsInfo.mNativePtl.mAllLayersIndependentConstraintFlag;
        mNativePtl.mOneAuOnlyConstraintFlag            = spsInfo.mNativePtl.mOneAuOnlyConstraintFlag;

        mNativePtl.mSixteenMinusMaxBitDepthConstraintIdc   = spsInfo.mNativePtl.mSixteenMinusMaxBitDepthConstraintIdc;
        mNativePtl.mThreeMinusMaxChromaFormatConstraintIdc = spsInfo.mNativePtl.mThreeMinusMaxChromaFormatConstraintIdc;
        mNativePtl.mNoMixedNaluTypesInPicConstraintFlag    = spsInfo.mNativePtl.mNoMixedNaluTypesInPicConstraintFlag;
        mNativePtl.mNoTrailConstraintFlag                  = spsInfo.mNativePtl.mNoTrailConstraintFlag;
        mNativePtl.mNoStsaConstraintFlag                   = spsInfo.mNativePtl.mNoStsaConstraintFlag;
        mNativePtl.mNoRaslConstraintFlag                   = spsInfo.mNativePtl.mNoRaslConstraintFlag;
        mNativePtl.mNoRadlConstraintFlag                   = spsInfo.mNativePtl.mNoRadlConstraintFlag;
        mNativePtl.mNoIdrConstraintFlag                    = spsInfo.mNativePtl.mNoIdrConstraintFlag;
        mNativePtl.mNoCraConstraintFlag                    = spsInfo.mNativePtl.mNoCraConstraintFlag;
        mNativePtl.mNoGdrConstraintFlag                    = spsInfo.mNativePtl.mNoGdrConstraintFlag;
        mNativePtl.mNoApsConstraintFlag                    = spsInfo.mNativePtl.mNoApsConstraintFlag;
        mNativePtl.mNoIdrRplConstraintFlag                 = spsInfo.mNativePtl.mNoIdrRplConstraintFlag;
        mNativePtl.mOneTilePerPicConstraintFlag            = spsInfo.mNativePtl.mOneTilePerPicConstraintFlag;
        mNativePtl.mPicHeaderInSliceHeaderConstraintFlag   = spsInfo.mNativePtl.mPicHeaderInSliceHeaderConstraintFlag;
        mNativePtl.mOneSlicePerPicConstraintFlag           = spsInfo.mNativePtl.mOneSlicePerPicConstraintFlag;
        mNativePtl.mNoRectSliceConstraintFlag              = spsInfo.mNativePtl.mNoRectSliceConstraintFlag;
        mNativePtl.mOneSlicePerSubpicConstraintFlag        = spsInfo.mNativePtl.mOneSlicePerSubpicConstraintFlag;
        mNativePtl.mNoSubpicInfoConstraintFlag             = spsInfo.mNativePtl.mNoSubpicInfoConstraintFlag;
        mNativePtl.mThreeMinusMaxLog2CtuSizeConstraintIdc  = spsInfo.mNativePtl.mThreeMinusMaxLog2CtuSizeConstraintIdc;
        mNativePtl.mNoPartitionConstraintsOverrideConstraintFlag =
            spsInfo.mNativePtl.mNoPartitionConstraintsOverrideConstraintFlag;
        mNativePtl.mNoMttConstraintFlag                 = spsInfo.mNativePtl.mNoMttConstraintFlag;
        mNativePtl.mNoQtbttDualTreeIntraConstraintFlag  = spsInfo.mNativePtl.mNoQtbttDualTreeIntraConstraintFlag;
        mNativePtl.mNoPaletteConstraintFlag             = spsInfo.mNativePtl.mNoPaletteConstraintFlag;
        mNativePtl.mNoIbcConstraintFlag                 = spsInfo.mNativePtl.mNoIbcConstraintFlag;
        mNativePtl.mNoIspConstraintFlag                 = spsInfo.mNativePtl.mNoIspConstraintFlag;
        mNativePtl.mNoMrlConstraintFlag                 = spsInfo.mNativePtl.mNoMrlConstraintFlag;
        mNativePtl.mNoMipConstraintFlag                 = spsInfo.mNativePtl.mNoMipConstraintFlag;
        mNativePtl.mNoCclmConstraintFlag                = spsInfo.mNativePtl.mNoCclmConstraintFlag;
        mNativePtl.mNoRefPicResamplingConstraintFlag    = spsInfo.mNativePtl.mNoRefPicResamplingConstraintFlag;
        mNativePtl.mNoResChangeInClvsConstraintFlag     = spsInfo.mNativePtl.mNoResChangeInClvsConstraintFlag;
        mNativePtl.mNoWeightedPredictionConstraintFlag  = spsInfo.mNativePtl.mNoWeightedPredictionConstraintFlag;
        mNativePtl.mNoRefWraparoundConstraintFlag       = spsInfo.mNativePtl.mNoRefWraparoundConstraintFlag;
        mNativePtl.mNoTemporalMvpConstraintFlag         = spsInfo.mNativePtl.mNoTemporalMvpConstraintFlag;
        mNativePtl.mNoSbtmvpConstraintFlag              = spsInfo.mNativePtl.mNoSbtmvpConstraintFlag;
        mNativePtl.mNoAmvrConstraintFlag                = spsInfo.mNativePtl.mNoAmvrConstraintFlag;
        mNativePtl.mNoBdofConstraintFlag                = spsInfo.mNativePtl.mNoBdofConstraintFlag;
        mNativePtl.mNoSmvdConstraintFlag                = spsInfo.mNativePtl.mNoSmvdConstraintFlag;
        mNativePtl.mNoDmvrConstraintFlag                = spsInfo.mNativePtl.mNoDmvrConstraintFlag;
        mNativePtl.mNoMmvdConstraintFlag                = spsInfo.mNativePtl.mNoMmvdConstraintFlag;
        mNativePtl.mNoAffineMotionConstraintFlag        = spsInfo.mNativePtl.mNoAffineMotionConstraintFlag;
        mNativePtl.mNoProfConstraintFlag                = spsInfo.mNativePtl.mNoProfConstraintFlag;
        mNativePtl.mNoBcwConstraintFlag                 = spsInfo.mNativePtl.mNoBcwConstraintFlag;
        mNativePtl.mNoCiipConstraintFlag                = spsInfo.mNativePtl.mNoCiipConstraintFlag;
        mNativePtl.mNoGpmConstraintFlag                 = spsInfo.mNativePtl.mNoGpmConstraintFlag;
        mNativePtl.mNoLumaTransformSize64ConstraintFlag = spsInfo.mNativePtl.mNoLumaTransformSize64ConstraintFlag;
        mNativePtl.mNoTransformSkipConstraintFlag       = spsInfo.mNativePtl.mNoTransformSkipConstraintFlag;
        mNativePtl.mNoBDPCMConstraintFlag               = spsInfo.mNativePtl.mNoBDPCMConstraintFlag;
        mNativePtl.mNoMtsConstraintFlag                 = spsInfo.mNativePtl.mNoMtsConstraintFlag;
        mNativePtl.mNoLfnstConstraintFlag               = spsInfo.mNativePtl.mNoLfnstConstraintFlag;
        mNativePtl.mNoJointCbCrConstraintFlag           = spsInfo.mNativePtl.mNoJointCbCrConstraintFlag;
        mNativePtl.mNoSbtConstraintFlag                 = spsInfo.mNativePtl.mNoSbtConstraintFlag;
        mNativePtl.mNoActConstraintFlag                 = spsInfo.mNativePtl.mNoActConstraintFlag;
        mNativePtl.mNoExplicitScaleListConstraintFlag   = spsInfo.mNativePtl.mNoExplicitScaleListConstraintFlag;
        mNativePtl.mNoDepQuantConstraintFlag            = spsInfo.mNativePtl.mNoDepQuantConstraintFlag;
        mNativePtl.mNoSignDataHidingConstraintFlag      = spsInfo.mNativePtl.mNoSignDataHidingConstraintFlag;
        mNativePtl.mNoCuQpDeltaConstraintFlag           = spsInfo.mNativePtl.mNoCuQpDeltaConstraintFlag;
        mNativePtl.mNoChromaQpOffsetConstraintFlag      = spsInfo.mNativePtl.mNoChromaQpOffsetConstraintFlag;
        mNativePtl.mNoSaoConstraintFlag                 = spsInfo.mNativePtl.mNoSaoConstraintFlag;
        mNativePtl.mNoAlfConstraintFlag                 = spsInfo.mNativePtl.mNoAlfConstraintFlag;
        mNativePtl.mNoCCAlfConstraintFlag               = spsInfo.mNativePtl.mNoCCAlfConstraintFlag;
        mNativePtl.mNoLmcsConstraintFlag                = spsInfo.mNativePtl.mNoLmcsConstraintFlag;
        mNativePtl.mNoLadfConstraintFlag                = spsInfo.mNativePtl.mNoLadfConstraintFlag;
        mNativePtl.mNoVirtualBoundariesConstraintFlag   = spsInfo.mNativePtl.mNoVirtualBoundariesConstraintFlag;
        mNativePtl.mPtlSublayerLevelPresentFlag         = spsInfo.mNativePtl.mPtlSublayerLevelPresentFlag;
        mNativePtl.mSublayerLevelIdc                    = spsInfo.mNativePtl.mSublayerLevelIdc;
        mNativePtl.mNumSubProfiles                      = spsInfo.mNativePtl.mNumSubProfiles;
        mNativePtl.mGeneralSubProfileIdc                = spsInfo.mNativePtl.mGeneralSubProfileIdc;
    }

    return true;
}


void VvcDecoderConfigurationRecord::makeConfigFromVPS(const VvcParser::VpsInfo& vpsInfo){

    mConfigurationVersion = 1;

    ///This should be read from config
    //mLengthSizeMinus1     = 3;

    mPtlPresentFlag = vpsInfo.mPtPresentFlag[0]; // Just take the PTL of OLS 0 for now

    if (mPtlPresentFlag){
        mAvgFrameRate      = 0;  // Unspecified average frame rate.
        mConstantFrameRate = 0;
        mNumSublayers      = static_cast<uint8_t>(vpsInfo.mPtlMaxTemporalId[0] + 1);
        mOlsIdx            = 0; // Just take OLS 0 for now

        /*
        mNumSublayers      = vpsInfo.mNumSublayers;
        mOlsIdx            = vpsInfo.mOlsIdx;
        mChromaFormatIdc   = vpsInfo.mChromaFormatIdc;
        mBitDepthMinus8    = vpsInfo.mBitDepthMinus8;
        mMaxPictureWidth   = vpsInfo.mMaxPictureWidth;
        mMaxPictureHeight  = vpsInfo.mMaxPictureHeight;
        */

        mNativePtl.mGeneralProfileIdc          = vpsInfo.mNativePtl.mGeneralProfileIdc;
        mNativePtl.mGeneralTierFlag            = vpsInfo.mNativePtl.mGeneralTierFlag;
        mNativePtl.mGeneralLevelIdc            = vpsInfo.mNativePtl.mGeneralLevelIdc;
        mNativePtl.mPtlFrameOnlyConstraintFlag = vpsInfo.mNativePtl.mPtlFrameOnlyConstraintFlag;
        mNativePtl.mPtlMultilayerEnabledFlag   = vpsInfo.mNativePtl.mPtlMultilayerEnabledFlag;

        mNativePtl.mGciPresentFlag                     = vpsInfo.mNativePtl.mGciPresentFlag;
        mNativePtl.mIntraOnlyConstraintFlag            = vpsInfo.mNativePtl.mIntraOnlyConstraintFlag;
        mNativePtl.mAllLayersIndependentConstraintFlag = vpsInfo.mNativePtl.mAllLayersIndependentConstraintFlag;
        mNativePtl.mOneAuOnlyConstraintFlag            = vpsInfo.mNativePtl.mOneAuOnlyConstraintFlag;

        mNativePtl.mSixteenMinusMaxBitDepthConstraintIdc   = vpsInfo.mNativePtl.mSixteenMinusMaxBitDepthConstraintIdc;
        mNativePtl.mThreeMinusMaxChromaFormatConstraintIdc = vpsInfo.mNativePtl.mThreeMinusMaxChromaFormatConstraintIdc;
        mNativePtl.mNoMixedNaluTypesInPicConstraintFlag    = vpsInfo.mNativePtl.mNoMixedNaluTypesInPicConstraintFlag;
        mNativePtl.mNoTrailConstraintFlag                  = vpsInfo.mNativePtl.mNoTrailConstraintFlag;
        mNativePtl.mNoStsaConstraintFlag                   = vpsInfo.mNativePtl.mNoStsaConstraintFlag;
        mNativePtl.mNoRaslConstraintFlag                   = vpsInfo.mNativePtl.mNoRaslConstraintFlag;
        mNativePtl.mNoRadlConstraintFlag                   = vpsInfo.mNativePtl.mNoRadlConstraintFlag;
        mNativePtl.mNoIdrConstraintFlag                    = vpsInfo.mNativePtl.mNoIdrConstraintFlag;
        mNativePtl.mNoCraConstraintFlag                    = vpsInfo.mNativePtl.mNoCraConstraintFlag;
        mNativePtl.mNoGdrConstraintFlag                    = vpsInfo.mNativePtl.mNoGdrConstraintFlag;
        mNativePtl.mNoApsConstraintFlag                    = vpsInfo.mNativePtl.mNoApsConstraintFlag;
        mNativePtl.mNoIdrRplConstraintFlag                 = vpsInfo.mNativePtl.mNoIdrRplConstraintFlag;
        mNativePtl.mOneTilePerPicConstraintFlag            = vpsInfo.mNativePtl.mOneTilePerPicConstraintFlag;
        mNativePtl.mPicHeaderInSliceHeaderConstraintFlag   = vpsInfo.mNativePtl.mPicHeaderInSliceHeaderConstraintFlag;
        mNativePtl.mOneSlicePerPicConstraintFlag           = vpsInfo.mNativePtl.mOneSlicePerPicConstraintFlag;
        mNativePtl.mNoRectSliceConstraintFlag              = vpsInfo.mNativePtl.mNoRectSliceConstraintFlag;
        mNativePtl.mOneSlicePerSubpicConstraintFlag        = vpsInfo.mNativePtl.mOneSlicePerSubpicConstraintFlag;
        mNativePtl.mNoSubpicInfoConstraintFlag             = vpsInfo.mNativePtl.mNoSubpicInfoConstraintFlag;
        mNativePtl.mThreeMinusMaxLog2CtuSizeConstraintIdc  = vpsInfo.mNativePtl.mThreeMinusMaxLog2CtuSizeConstraintIdc;
        mNativePtl.mNoPartitionConstraintsOverrideConstraintFlag =
            vpsInfo.mNativePtl.mNoPartitionConstraintsOverrideConstraintFlag;
        mNativePtl.mNoMttConstraintFlag                 = vpsInfo.mNativePtl.mNoMttConstraintFlag;
        mNativePtl.mNoQtbttDualTreeIntraConstraintFlag  = vpsInfo.mNativePtl.mNoQtbttDualTreeIntraConstraintFlag;
        mNativePtl.mNoPaletteConstraintFlag             = vpsInfo.mNativePtl.mNoPaletteConstraintFlag;
        mNativePtl.mNoIbcConstraintFlag                 = vpsInfo.mNativePtl.mNoIbcConstraintFlag;
        mNativePtl.mNoIspConstraintFlag                 = vpsInfo.mNativePtl.mNoIspConstraintFlag;
        mNativePtl.mNoMrlConstraintFlag                 = vpsInfo.mNativePtl.mNoMrlConstraintFlag;
        mNativePtl.mNoMipConstraintFlag                 = vpsInfo.mNativePtl.mNoMipConstraintFlag;
        mNativePtl.mNoCclmConstraintFlag                = vpsInfo.mNativePtl.mNoCclmConstraintFlag;
        mNativePtl.mNoRefPicResamplingConstraintFlag    = vpsInfo.mNativePtl.mNoRefPicResamplingConstraintFlag;
        mNativePtl.mNoResChangeInClvsConstraintFlag     = vpsInfo.mNativePtl.mNoResChangeInClvsConstraintFlag;
        mNativePtl.mNoWeightedPredictionConstraintFlag  = vpsInfo.mNativePtl.mNoWeightedPredictionConstraintFlag;
        mNativePtl.mNoRefWraparoundConstraintFlag       = vpsInfo.mNativePtl.mNoRefWraparoundConstraintFlag;
        mNativePtl.mNoTemporalMvpConstraintFlag         = vpsInfo.mNativePtl.mNoTemporalMvpConstraintFlag;
        mNativePtl.mNoSbtmvpConstraintFlag              = vpsInfo.mNativePtl.mNoSbtmvpConstraintFlag;
        mNativePtl.mNoAmvrConstraintFlag                = vpsInfo.mNativePtl.mNoAmvrConstraintFlag;
        mNativePtl.mNoBdofConstraintFlag                = vpsInfo.mNativePtl.mNoBdofConstraintFlag;
        mNativePtl.mNoSmvdConstraintFlag                = vpsInfo.mNativePtl.mNoSmvdConstraintFlag;
        mNativePtl.mNoDmvrConstraintFlag                = vpsInfo.mNativePtl.mNoDmvrConstraintFlag;
        mNativePtl.mNoMmvdConstraintFlag                = vpsInfo.mNativePtl.mNoMmvdConstraintFlag;
        mNativePtl.mNoAffineMotionConstraintFlag        = vpsInfo.mNativePtl.mNoAffineMotionConstraintFlag;
        mNativePtl.mNoProfConstraintFlag                = vpsInfo.mNativePtl.mNoProfConstraintFlag;
        mNativePtl.mNoBcwConstraintFlag                 = vpsInfo.mNativePtl.mNoBcwConstraintFlag;
        mNativePtl.mNoCiipConstraintFlag                = vpsInfo.mNativePtl.mNoCiipConstraintFlag;
        mNativePtl.mNoGpmConstraintFlag                 = vpsInfo.mNativePtl.mNoGpmConstraintFlag;
        mNativePtl.mNoLumaTransformSize64ConstraintFlag = vpsInfo.mNativePtl.mNoLumaTransformSize64ConstraintFlag;
        mNativePtl.mNoTransformSkipConstraintFlag       = vpsInfo.mNativePtl.mNoTransformSkipConstraintFlag;
        mNativePtl.mNoBDPCMConstraintFlag               = vpsInfo.mNativePtl.mNoBDPCMConstraintFlag;
        mNativePtl.mNoMtsConstraintFlag                 = vpsInfo.mNativePtl.mNoMtsConstraintFlag;
        mNativePtl.mNoLfnstConstraintFlag               = vpsInfo.mNativePtl.mNoLfnstConstraintFlag;
        mNativePtl.mNoJointCbCrConstraintFlag           = vpsInfo.mNativePtl.mNoJointCbCrConstraintFlag;
        mNativePtl.mNoSbtConstraintFlag                 = vpsInfo.mNativePtl.mNoSbtConstraintFlag;
        mNativePtl.mNoActConstraintFlag                 = vpsInfo.mNativePtl.mNoActConstraintFlag;
        mNativePtl.mNoExplicitScaleListConstraintFlag   = vpsInfo.mNativePtl.mNoExplicitScaleListConstraintFlag;
        mNativePtl.mNoDepQuantConstraintFlag            = vpsInfo.mNativePtl.mNoDepQuantConstraintFlag;
        mNativePtl.mNoSignDataHidingConstraintFlag      = vpsInfo.mNativePtl.mNoSignDataHidingConstraintFlag;
        mNativePtl.mNoCuQpDeltaConstraintFlag           = vpsInfo.mNativePtl.mNoCuQpDeltaConstraintFlag;
        mNativePtl.mNoChromaQpOffsetConstraintFlag      = vpsInfo.mNativePtl.mNoChromaQpOffsetConstraintFlag;
        mNativePtl.mNoSaoConstraintFlag                 = vpsInfo.mNativePtl.mNoSaoConstraintFlag;
        mNativePtl.mNoAlfConstraintFlag                 = vpsInfo.mNativePtl.mNoAlfConstraintFlag;
        mNativePtl.mNoCCAlfConstraintFlag               = vpsInfo.mNativePtl.mNoCCAlfConstraintFlag;
        mNativePtl.mNoLmcsConstraintFlag                = vpsInfo.mNativePtl.mNoLmcsConstraintFlag;
        mNativePtl.mNoLadfConstraintFlag                = vpsInfo.mNativePtl.mNoLadfConstraintFlag;
        mNativePtl.mNoVirtualBoundariesConstraintFlag   = vpsInfo.mNativePtl.mNoVirtualBoundariesConstraintFlag;
        mNativePtl.mPtlSublayerLevelPresentFlag         = vpsInfo.mNativePtl.mPtlSublayerLevelPresentFlag;
        mNativePtl.mSublayerLevelIdc                    = vpsInfo.mNativePtl.mSublayerLevelIdc;
        mNativePtl.mNumSubProfiles                      = vpsInfo.mNativePtl.mNumSubProfiles;
        mNativePtl.mGeneralSubProfileIdc                = vpsInfo.mNativePtl.mGeneralSubProfileIdc;
    }
}

// C6262: MSVC /analyze reports ~60 KB stack.  The bulk is from the
// VvcParser::parseVps() call path which constructs VpsInfo (containing
// fixed-size OLS/layer tables) on the stack.  This is a single-shot
// parse that runs once per VPS NAL — well within the 1 MB default stack.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 6262)
#endif
void VvcDecoderConfigurationRecord::addNalUnit(const Vector<uint8_t>& nalUnit,
                                               const VvcNalUnitType nalUnitType,
                                               const bool arrayCompleteness)
{
    NALArray* nalArray = nullptr;
    Vector<uint8_t> tmpNalUnit;
    unsigned int startCodeLen;

    // find array for the given NAL unit type
    for (auto& i : mNalArray)
    {
        if (static_cast<uint8_t>(nalUnitType) == static_cast<uint8_t>(i.nalUnitType))
        {
            nalArray = &i;
            break;
        }
    }

    // if an array is not present for the NAL unit type, create one
    if (nalArray == nullptr)
    {
        NALArray nalArrayTmp;
        nalArrayTmp.arrayCompleteness = arrayCompleteness;
        nalArrayTmp.nalUnitType       = nalUnitType;
        mNalArray.push_back(nalArrayTmp);
        nalArray = &mNalArray.back();
    }

    startCodeLen = findStartCodeLen(nalUnit);
    tmpNalUnit.insert(tmpNalUnit.begin(), nalUnit.cbegin() + static_cast<int>(startCodeLen),
                      nalUnit.cend());  // copy NAL data excluding potential start code

    // Capture nuh_layer_id from the first byte of the VVC NAL header
    // (byte 0: forbidden_zero(1) + nuh_reserved_zero(1) + nuh_layer_id(6)).
    // Storing this per-NAL allows SPS/PPS for different layers to be looked up
    // by their actual nuh_layer_id rather than by insertion order.
    uint16_t nuhLayerId = 0;
    if (!tmpNalUnit.empty())
    {
        nuhLayerId = static_cast<uint16_t>(tmpNalUnit[0] & 0x3F);
    }

    // add NAL unit to the NAL unit array
    nalArray->nalList.push_back(tmpNalUnit);
    nalArray->nuhLayerIds.push_back(nuhLayerId);

    // If this is a VPS NAL, parse it once and cache the derived OLS / layer
    // table so downstream code (parseCompactConfig, multi-layer reconstruction,
    // future OLS-aware accessors) does not need to invoke VvcParser::parseVps
    // again over the same bytes. Failures are silently tolerated; the cache
    // simply stays unset and callers can fall back to ad-hoc parsing.
    if (nalUnitType == VvcNalUnitType::VPS_NUT && !mHasVpsInfo && !tmpNalUnit.empty())
    {
        try
        {
            mVpsInfo    = VvcParser::parseVps(tmpNalUnit);
            mHasVpsInfo = true;
        }
        catch (...)
        {
            // Defensive: never let a parser exception escape addNalUnit().
            mHasVpsInfo = false;
        }
    }
    else if (nalUnitType == VvcNalUnitType::OPI_NUT && !mHasOpiInfo && !tmpNalUnit.empty())
    {
        try
        {
            mOpiInfo    = VvcParser::parseOpi(tmpNalUnit);
            mHasOpiInfo = true;
        }
        catch (...)
        {
            mHasOpiInfo = false;
        }
    }
}
#if defined(_MSC_VER)
#pragma warning(pop)  // C6262
#endif

void VvcDecoderConfigurationRecord::writeDecConfigRecord(ISOBMFF::BitStream& bitstr) const
{
    bitstr.writeBits(0xff, 5);  // reserved '11111'b
    bitstr.writeBits(mLengthSizeMinus1, 2);
    bitstr.writeBits(mPtlPresentFlag, 1);
    if (mPtlPresentFlag)
    {
        bitstr.writeBits(mOlsIdx, 9);
        bitstr.writeBits(mNumSublayers, 3);
        bitstr.writeBits(mConstantFrameRate, 2);
        bitstr.writeBits(mChromaFormatIdc, 2);
        bitstr.writeBits(mBitDepthMinus8, 3);
        bitstr.writeBits(0xff, 5);  // reserved '11111'b
        writeVvcPTLRecord(bitstr, mNativePtl, mNumSublayers);
        bitstr.writeBits(mMaxPictureWidth, 16);
        bitstr.writeBits(mMaxPictureHeight, 16);
        bitstr.writeBits(mAvgFrameRate, 16);
    }

    bitstr.writeBits(mNalArray.size(), 8);
    for (const auto& i : mNalArray)
    {
        bitstr.writeBits(i.arrayCompleteness, 1);
        bitstr.writeBits(0, 2);  // reserved = 0
        bitstr.writeBits(static_cast<uint8_t>(i.nalUnitType), 5);

        if (i.nalUnitType != VvcNalUnitType::DCI_NUT && i.nalUnitType != VvcNalUnitType::OPI_NUT)
        {
            bitstr.writeBits(static_cast<unsigned int>(i.nalList.size()), 16);
        }
        for (const auto& j : i.nalList)
        {
            bitstr.writeBits(static_cast<unsigned int>(j.size()), 16);
            bitstr.write8BitsArray(j, j.size());  // write parameter set NAL unit
        }
    }
}

void VvcDecoderConfigurationRecord::writeCompactDecConfigRecord(ISOBMFF::BitStream& bitstr) const
{
    bitstr.writeBits(mMultiLayerFlag,1);    //bit(1) multi_layer_flag
    bitstr.writeBits(mLengthSizeMinus1,2);  //bit(2) length_size_minus1
    size_t totalNalUnits = 0;
    for(const auto& nalutype : mNalArray){
        totalNalUnits += nalutype.nalList.size();
    }
    bitstr.writeBits(totalNalUnits - 2, 5);  //bit(5) num_nal_units_minus2
    for(const auto& nalutype : mNalArray){
        for(const auto& nalu : nalutype.nalList){
            bitstr.write8Bits(static_cast<uint8_t>(nalu.size())); //bit(8) nal_unit_length
            bitstr.write8BitsArray(nalu,nalu.size());   //nal_unit
        }
    }
}

void VvcDecoderConfigurationRecord::parseConfig(ISOBMFF::BitStream& bitstr)
{
    unsigned int numOfArrays;

    bitstr.readBits(5);  // reserved = '11111'b
    mLengthSizeMinus1 = static_cast<uint8_t>(bitstr.readBits(2));
    mPtlPresentFlag   = static_cast<uint8_t>(bitstr.readBits(1));
    if (mPtlPresentFlag)
    {
        mOlsIdx            = static_cast<uint16_t>(bitstr.readBits(9));
        mNumSublayers      = static_cast<uint8_t>(bitstr.readBits(3));
        mConstantFrameRate = static_cast<uint8_t>(bitstr.readBits(2));
        mChromaFormatIdc   = static_cast<uint8_t>(bitstr.readBits(2));
        mBitDepthMinus8    = static_cast<uint8_t>(bitstr.readBits(3));
        bitstr.readBits(5);  // reserved '11111'b
        mNativePtl        = parseVvcPTLRecord(bitstr, mNumSublayers);
        mMaxPictureWidth  = static_cast<uint16_t>(bitstr.readBits(16));
        mMaxPictureHeight = static_cast<uint16_t>(bitstr.readBits(16));
        mAvgFrameRate     = static_cast<uint16_t>(bitstr.readBits(16));
    }

    numOfArrays = bitstr.readBits(8);
    for (unsigned int i = 0; i < numOfArrays; i++)
    {
        bool arrayCompleteness = bitstr.readBits(1);
        VvcNalUnitType nalUnitType;
        unsigned int numNalus = 1;

        bitstr.readBits(2);  // reserved = 0
        nalUnitType = static_cast<VvcNalUnitType>(bitstr.readBits(5));

        if (nalUnitType != VvcNalUnitType::DCI_NUT && nalUnitType != VvcNalUnitType::OPI_NUT)
        {
            numNalus = bitstr.readBits(16);
        }
        for (unsigned int j = 0; j < numNalus; j++)
        {
            unsigned int nalSize;
            nalSize = bitstr.readBits(16);
            Vector<uint8_t> nalData(nalSize);
            nalData.clear();
            bitstr.read8BitsArray(nalData, nalSize);  // read parameter set NAL unit
            addNalUnit(nalData, nalUnitType, arrayCompleteness);
        }
    }
}

void VvcDecoderConfigurationRecord::parseCompactConfig(ISOBMFF::BitStream& bitstr, const ConfigValues& vals)
{
    mMultiLayerFlag = bitstr.readBits(1);
    mLengthSizeMinus1 = static_cast<uint8_t>(bitstr.readBits(2));
    uint8_t num_nal_units_minus2 = static_cast<uint8_t>(bitstr.readBits(5));
    for (int i = 0; i < (num_nal_units_minus2 + 2); i++) {
        uint8_t nal_unit_length = bitstr.read8Bits();
        Vector<uint8_t> nal_unit;
        bitstr.read8BitsArray(nal_unit, nal_unit_length);

        // nuh_layer_id is captured inside addNalUnit() (low 6 bits of byte 0).
        // Per-layer SPS/PPS retrieval is handled by getNALU() / getAllParameterSets()
        // which look up by stored nuh_layer_id rather than by position.

        //bit(5) nal_unit_type from second byte of NAL unit header
        auto nalType = static_cast<VvcNalUnitType>((nal_unit.at(1) & 0xF8) >> 3);
        addNalUnit(nal_unit, nalType, true);
    }

    mPtlPresentFlag = true;
    mNumSublayers = 1;
    mConstantFrameRate = 1;

    if (vals.item_id == 1) {  // main item
        mChromaFormatIdc = vals.chroma_subsampling;
    } else if (vals.item_id == 3) {  // gainmap
        mChromaFormatIdc = vals.gainmap_chroma_subsampling;
    } else {  // alpha item
        mChromaFormatIdc = 0;
    }
    if (vals.item_id == 1 || vals.item_id == 2) {  // main | alpha
        if (!vals.high_bit_depth_flag) {
            mBitDepthMinus8 = 0;
        } else {
            mBitDepthMinus8 = vals.bit_depth_minus9 + 1;
        }
        mMaxPictureWidth  = static_cast<uint16_t>(vals.width_minus1 + 1);
        mMaxPictureHeight = static_cast<uint16_t>(vals.height_minus1 + 1);
    } else if (vals.item_id == 3) {  // gainmap
        if (!vals.gainmap_high_bit_depth_flag) {
            mBitDepthMinus8 = 0;
        } else {
            mBitDepthMinus8 = vals.gainmap_bit_depth_minus9 + 1;
        }
        mMaxPictureWidth  = static_cast<uint16_t>(vals.gainmap_width_minus1 + 1);
        mMaxPictureHeight = static_cast<uint16_t>(vals.gainmap_height_minus1 + 1);
    }

    mAvgFrameRate = 0;

    //Native PTL
    if (mMultiLayerFlag) {

        // All NALs were already passed through addNalUnit() above, so VPS
        // (and, when present, OPI) have already been parsed and cached. We
        // just consume the cached structs here -- no raw NAL re-read, no
        // second VvcParser invocation.
        if (!mHasVpsInfo)
        {
            // A multi-layer compact config without a parseable VPS is
            // malformed; abort cleanly rather than silently produce garbage.
            throw std::runtime_error(
                "VvcDecoderConfigurationRecord::parseCompactConfig: "
                "multi_layer_flag=1 but no VPS NAL was cached");
        }
        const VvcParser::VpsInfo& vpsInfo = mVpsInfo;

        // "All other parameters are set based on the information present in
        //  the VPS NAL unit" -- this fully populates mNativePtl, so any
        // explicit per-field copy below would be redundant.
        makeConfigFromVPS(vpsInfo);

        // ols_idx selection per ISO/IEC 14496-15:
        //   - if OPI present: ols_idx = opi_ols_idx
        //   - else:           lowest OLS idx with largest #output layers
        if (mHasOpiInfo)
        {
            mOlsIdx = static_cast<uint16_t>(mOpiInfo.mOpiolsidx);
        }
        else
        {
            mOlsIdx = VvcParser::getLowestOlsIdxWithLargestNumOutputLayers(vpsInfo);
        }

        // Refine native_ptl to use the PTL record that conforms to mOlsIdx.
        // mOlsPtlIdx[i] maps OLS index i to the corresponding PTL record index.
        if(!vpsInfo.mPtlRecords.empty()){
            const uint32_t ptlIdx = vpsInfo.mOlsPtlIdx[mOlsIdx];
            if(ptlIdx < vpsInfo.mPtlRecords.size()){
                const auto& src = vpsInfo.mPtlRecords[ptlIdx];
                mNativePtl.mGeneralProfileIdc          = src.mGeneralProfileIdc;
                mNativePtl.mGeneralTierFlag            = src.mGeneralTierFlag;
                mNativePtl.mGeneralLevelIdc            = src.mGeneralLevelIdc;
                mNativePtl.mPtlFrameOnlyConstraintFlag = src.mPtlFrameOnlyConstraintFlag;
                mNativePtl.mPtlMultilayerEnabledFlag   = src.mPtlMultilayerEnabledFlag;
                mNativePtl.mGciPresentFlag             = src.mGciPresentFlag;
                mNativePtl.mIntraOnlyConstraintFlag    = src.mIntraOnlyConstraintFlag;
                mNativePtl.mAllLayersIndependentConstraintFlag = src.mAllLayersIndependentConstraintFlag;
                mNativePtl.mOneAuOnlyConstraintFlag    = src.mOneAuOnlyConstraintFlag;
                mNativePtl.mSixteenMinusMaxBitDepthConstraintIdc   = src.mSixteenMinusMaxBitDepthConstraintIdc;
                mNativePtl.mThreeMinusMaxChromaFormatConstraintIdc = src.mThreeMinusMaxChromaFormatConstraintIdc;
                mNativePtl.mNoMixedNaluTypesInPicConstraintFlag = src.mNoMixedNaluTypesInPicConstraintFlag;
                mNativePtl.mNoTrailConstraintFlag      = src.mNoTrailConstraintFlag;
                mNativePtl.mNoStsaConstraintFlag       = src.mNoStsaConstraintFlag;
                mNativePtl.mNoRaslConstraintFlag       = src.mNoRaslConstraintFlag;
                mNativePtl.mNoRadlConstraintFlag       = src.mNoRadlConstraintFlag;
                mNativePtl.mNoIdrConstraintFlag        = src.mNoIdrConstraintFlag;
                mNativePtl.mNoCraConstraintFlag        = src.mNoCraConstraintFlag;
                mNativePtl.mNoGdrConstraintFlag        = src.mNoGdrConstraintFlag;
                mNativePtl.mNoApsConstraintFlag        = src.mNoApsConstraintFlag;
                mNativePtl.mNoIdrRplConstraintFlag     = src.mNoIdrRplConstraintFlag;
                mNativePtl.mOneTilePerPicConstraintFlag          = src.mOneTilePerPicConstraintFlag;
                mNativePtl.mPicHeaderInSliceHeaderConstraintFlag = src.mPicHeaderInSliceHeaderConstraintFlag;
                mNativePtl.mOneSlicePerPicConstraintFlag         = src.mOneSlicePerPicConstraintFlag;
                mNativePtl.mNoRectSliceConstraintFlag            = src.mNoRectSliceConstraintFlag;
                mNativePtl.mOneSlicePerSubpicConstraintFlag      = src.mOneSlicePerSubpicConstraintFlag;
                mNativePtl.mNoSubpicInfoConstraintFlag           = src.mNoSubpicInfoConstraintFlag;
                mNativePtl.mThreeMinusMaxLog2CtuSizeConstraintIdc = src.mThreeMinusMaxLog2CtuSizeConstraintIdc;
                mNativePtl.mNoPartitionConstraintsOverrideConstraintFlag = src.mNoPartitionConstraintsOverrideConstraintFlag;
                mNativePtl.mNoMttConstraintFlag                 = src.mNoMttConstraintFlag;
                mNativePtl.mNoQtbttDualTreeIntraConstraintFlag  = src.mNoQtbttDualTreeIntraConstraintFlag;
                mNativePtl.mNoPaletteConstraintFlag             = src.mNoPaletteConstraintFlag;
                mNativePtl.mNoIbcConstraintFlag                 = src.mNoIbcConstraintFlag;
                mNativePtl.mNoIspConstraintFlag                 = src.mNoIspConstraintFlag;
                mNativePtl.mNoMrlConstraintFlag                 = src.mNoMrlConstraintFlag;
                mNativePtl.mNoMipConstraintFlag                 = src.mNoMipConstraintFlag;
                mNativePtl.mNoCclmConstraintFlag                = src.mNoCclmConstraintFlag;
                mNativePtl.mNoRefPicResamplingConstraintFlag    = src.mNoRefPicResamplingConstraintFlag;
                mNativePtl.mNoResChangeInClvsConstraintFlag     = src.mNoResChangeInClvsConstraintFlag;
                mNativePtl.mNoWeightedPredictionConstraintFlag  = src.mNoWeightedPredictionConstraintFlag;
                mNativePtl.mNoRefWraparoundConstraintFlag       = src.mNoRefWraparoundConstraintFlag;
                mNativePtl.mNoTemporalMvpConstraintFlag         = src.mNoTemporalMvpConstraintFlag;
                mNativePtl.mNoSbtmvpConstraintFlag              = src.mNoSbtmvpConstraintFlag;
                mNativePtl.mNoAmvrConstraintFlag                = src.mNoAmvrConstraintFlag;
                mNativePtl.mNoBdofConstraintFlag                = src.mNoBdofConstraintFlag;
                mNativePtl.mNoSmvdConstraintFlag                = src.mNoSmvdConstraintFlag;
                mNativePtl.mNoDmvrConstraintFlag                = src.mNoDmvrConstraintFlag;
                mNativePtl.mNoMmvdConstraintFlag                = src.mNoMmvdConstraintFlag;
                mNativePtl.mNoAffineMotionConstraintFlag        = src.mNoAffineMotionConstraintFlag;
                mNativePtl.mNoProfConstraintFlag                = src.mNoProfConstraintFlag;
                mNativePtl.mNoBcwConstraintFlag                 = src.mNoBcwConstraintFlag;
                mNativePtl.mNoCiipConstraintFlag                = src.mNoCiipConstraintFlag;
                mNativePtl.mNoGpmConstraintFlag                 = src.mNoGpmConstraintFlag;
                mNativePtl.mNoLumaTransformSize64ConstraintFlag = src.mNoLumaTransformSize64ConstraintFlag;
                mNativePtl.mNoTransformSkipConstraintFlag       = src.mNoTransformSkipConstraintFlag;
                mNativePtl.mNoBDPCMConstraintFlag               = src.mNoBDPCMConstraintFlag;
                mNativePtl.mNoMtsConstraintFlag                 = src.mNoMtsConstraintFlag;
                mNativePtl.mNoLfnstConstraintFlag               = src.mNoLfnstConstraintFlag;
                mNativePtl.mNoJointCbCrConstraintFlag           = src.mNoJointCbCrConstraintFlag;
                mNativePtl.mNoSbtConstraintFlag                 = src.mNoSbtConstraintFlag;
                mNativePtl.mNoActConstraintFlag                 = src.mNoActConstraintFlag;
                mNativePtl.mNoExplicitScaleListConstraintFlag   = src.mNoExplicitScaleListConstraintFlag;
                mNativePtl.mNoDepQuantConstraintFlag            = src.mNoDepQuantConstraintFlag;
                mNativePtl.mNoSignDataHidingConstraintFlag      = src.mNoSignDataHidingConstraintFlag;
                mNativePtl.mNoCuQpDeltaConstraintFlag           = src.mNoCuQpDeltaConstraintFlag;
                mNativePtl.mNoChromaQpOffsetConstraintFlag      = src.mNoChromaQpOffsetConstraintFlag;
                mNativePtl.mNoSaoConstraintFlag                 = src.mNoSaoConstraintFlag;
                mNativePtl.mNoAlfConstraintFlag                 = src.mNoAlfConstraintFlag;
                mNativePtl.mNoCCAlfConstraintFlag               = src.mNoCCAlfConstraintFlag;
                mNativePtl.mNoLmcsConstraintFlag                = src.mNoLmcsConstraintFlag;
                mNativePtl.mNoLadfConstraintFlag                = src.mNoLadfConstraintFlag;
                mNativePtl.mNoVirtualBoundariesConstraintFlag   = src.mNoVirtualBoundariesConstraintFlag;
                mNativePtl.mPtlSublayerLevelPresentFlag         = src.mPtlSublayerLevelPresentFlag;
                mNativePtl.mSublayerLevelIdc                    = src.mSublayerLevelIdc;
                mNativePtl.mNumSubProfiles                      = src.mNumSubProfiles;
                mNativePtl.mGeneralSubProfileIdc                = src.mGeneralSubProfileIdc;
            }
        }
    } else {
        // native_ptl parameters inferred from profile_tier_level(1,
        // sps_max_sublayers_minus1) in seq_parameter_set_rbsp per ISO/IEC 23090-3
        makeConfigFromSPS(getNALU(VvcNalUnitType::SPS_NUT));
    }
}


Vector<uint8_t> VvcDecoderConfigurationRecord::getNALU(const VvcNalUnitType nalutype, const uint16_t& nuh_layer_id) const{
    for(const auto& array : mNalArray){
        if(array.nalUnitType == nalutype){
            if(array.nalList.empty()){
                throw std::runtime_error("Error trying to access Nal Unit. It has empty payload");
            }

            // ALL_LAYERS / wildcard: just return the first NAL of this type.
            // (Callers that actually need every layer's NAL should use
            //  getAllParameterSets with ALL_LAYERS instead.)
            if(nuh_layer_id == ALL_LAYERS){
                return array.nalList.at(0);
            }

            // Look up by stored nuh_layer_id rather than by index. nuhLayerIds
            // is parallel to nalList and was populated in addNalUnit() from the
            // real NAL header byte. This lets multi-layer files where SPS/PPS
            // exist per nuh_layer_id resolve correctly to the right NAL.
            if(array.nuhLayerIds.size() == array.nalList.size()){
                for(size_t i = 0; i < array.nuhLayerIds.size(); ++i){
                    if(array.nuhLayerIds[i] == nuh_layer_id){
                        return array.nalList.at(i);
                    }
                }
            }

            // No exact layer match. Preserve historical behavior:
            // fall back to the first NAL of this type, with a warning when the
            // caller asked for a non-zero layer (likely indicates a real
            // mismatch versus the layer 0 default-fetch path).
            if(nuh_layer_id != 0){
                logWarning() << "Trying to access NALU with nuh_layer_id "
                             << nuh_layer_id << " for type "
                             << static_cast<int>(nalutype)
                             << " but no NAL with that layer id is present. "
                             << "Returning first available (layer "
                             << (array.nuhLayerIds.empty() ? 0 : array.nuhLayerIds.at(0))
                             << ")." << '\n';
            }
            return array.nalList.at(0);
        }
    }
    return Vector<uint8_t>();
}

void VvcDecoderConfigurationRecord::getOneParameterSet(Vector<uint8_t>& byteStream,
                                                       const VvcNalUnitType nalUnitType,
                                                        const uint16_t& nuh_layer_id) const
{
    auto array = getNALU(nalUnitType, nuh_layer_id);
    //Skip writing if parameter set is empty
    if(array.size() != 0){
        // add start code (0x00000001) before the NAL unit
        byteStream.push_back(0);
        byteStream.push_back(0);
        byteStream.push_back(0);
        byteStream.push_back(1);
        byteStream.insert(byteStream.end(), array.cbegin(), array.cend());
    }
}

void VvcDecoderConfigurationRecord::getAllParameterSets(Vector<Vector<std::uint8_t>>& byteStream,
                                                       const VvcNalUnitType nalUnitType,
                                                        const uint16_t& nuh_layer_id) const
{
    for(const auto& array : mNalArray){
        if(array.nalUnitType == nalUnitType){
            const bool layerAware = (array.nuhLayerIds.size() == array.nalList.size());
            for(size_t i = 0; i < array.nalList.size(); ++i){
                // Layer filter: ALL_LAYERS = include everything (legacy behavior);
                // any specific id = include only NALs whose nuh_layer_id matches.
                // If we have no per-NAL layer info (defensive), include everything.
                if(nuh_layer_id != ALL_LAYERS && layerAware
                   && array.nuhLayerIds[i] != nuh_layer_id){
                    continue;
                }
                const auto& nalu = array.nalList[i];
                Vector<uint8_t> NAL;
                // add start code (0x00000001) before the NAL unit
                NAL.push_back(0);
                NAL.push_back(0);
                NAL.push_back(0);
                NAL.push_back(1);
                NAL.insert(NAL.end(), nalu.cbegin(), nalu.cend());
                byteStream.push_back(NAL);
            }
        }
    }
}

uint16_t VvcDecoderConfigurationRecord::getMaxPictureWidth() const
{
    return mMaxPictureWidth;
}

uint16_t VvcDecoderConfigurationRecord::getMaxPictureHeight() const
{
    return mMaxPictureHeight;
}


uint16_t VvcDecoderConfigurationRecord::getAvgFrameRate() const
{
    return mAvgFrameRate;
}

std::uint8_t VvcDecoderConfigurationRecord::getChromaFormat() const
{
    return mChromaFormatIdc;
}

void VvcDecoderConfigurationRecord::getConfigurationMap(ConfigurationMap& aMap, const uint16_t& nuh_layer_id) const
{
    Vector<Vector<std::uint8_t>> aps;
    Vector<std::uint8_t> vps;

    // APS NALs are typically global to a CVS (a slice can reference any APS by
    // id regardless of layer); always emit every APS so a single-layer fetch
    // does not strip APS that the slice may reference. APS emission is
    // unaffected by the layer filter.
    getAllParameterSets(aps, VvcNalUnitType::PREFIX_APS_NUT, ALL_LAYERS);

    // VPS is global to the CVS (one VPS per CVS), so always emit the single VPS
    // regardless of the requested per-item layer.
    getOneParameterSet(vps, VvcNalUnitType::VPS_NUT);

    if(nuh_layer_id == ALL_LAYERS)
    {
        // Multi-layer fetch: include every SPS and every PPS across all layers.
        // This produces a parameter set bundle suitable for feeding a VVC
        // decoder a complete multi-layer access unit (base + dependent layers).
        Vector<Vector<std::uint8_t>> spsAll;
        Vector<Vector<std::uint8_t>> ppsAll;
        getAllParameterSets(spsAll, VvcNalUnitType::SPS_NUT, ALL_LAYERS);
        getAllParameterSets(ppsAll, VvcNalUnitType::PPS_NUT, ALL_LAYERS);

        if(aps.size() != 0){
            for(auto& nalu : aps){
                aMap.insert({DecoderParameterType::VVC_APS, std::move(nalu)});
            }
        }
        for(auto& nalu : spsAll){
            aMap.insert({DecoderParameterType::VVC_SPS, std::move(nalu)});
        }
        for(auto& nalu : ppsAll){
            aMap.insert({DecoderParameterType::VVC_PPS, std::move(nalu)});
        }
        if(vps.size() != 0){
            aMap.insert({DecoderParameterType::VVC_VPS, std::move(vps)});
        }
        return;
    }

    // Single-layer fetch: SPS and PPS specific to nuh_layer_id (matches the
    // layer of the image item the caller is about to decode).
    Vector<std::uint8_t> pps;
    Vector<std::uint8_t> sps;
    getOneParameterSet(pps, VvcNalUnitType::PPS_NUT, nuh_layer_id);
    getOneParameterSet(sps, VvcNalUnitType::SPS_NUT, nuh_layer_id);

    // prevent ghost PSets
    if(aps.size() != 0){
        for(auto& nalu : aps){
            aMap.insert({DecoderParameterType::VVC_APS, std::move(nalu)});
        }
    }
    if(sps.size() != 0){aMap.insert({DecoderParameterType::VVC_SPS, std::move(sps)});}
    if(pps.size() != 0){aMap.insert({DecoderParameterType::VVC_PPS, std::move(pps)});}
    if(vps.size() != 0){aMap.insert({DecoderParameterType::VVC_VPS, std::move(vps)});}
}

void VvcDecoderConfigurationRecord::writeVvcPTLRecord(ISOBMFF::BitStream& bitstr,
                                                      const VvcPTLRecord& vpr,
                                                      const std::uint8_t numSublayers) const
{
    if (!bitstr.isByteAligned())
    {
        throw RuntimeError("writeVvcPTLRecord: bitstream must be byte-aligned at entry");
    }

    bitstr.writeBits(0, 2);

    auto numBytesConstraintInfo = 1u;
    if (vpr.mGciPresentFlag)
    {
        numBytesConstraintInfo += 10;
    }

    bitstr.writeBits(numBytesConstraintInfo, 6);

    bitstr.writeBits(vpr.mGeneralProfileIdc, 7);
    bitstr.writeBits(vpr.mGeneralTierFlag, 1);
    bitstr.writeBits(vpr.mGeneralLevelIdc, 8);
    bitstr.writeBits(vpr.mPtlFrameOnlyConstraintFlag, 1);
    bitstr.writeBits(vpr.mPtlMultilayerEnabledFlag, 1);

    // general_constraint_info start
    bitstr.writeBits(vpr.mGciPresentFlag, 1);
    if (vpr.mGciPresentFlag)
    {
        bitstr.writeBits(vpr.mIntraOnlyConstraintFlag, 1);
        bitstr.writeBits(vpr.mAllLayersIndependentConstraintFlag, 1);
        bitstr.writeBits(vpr.mOneAuOnlyConstraintFlag, 1);

        // picture format
        bitstr.writeBits(vpr.mSixteenMinusMaxBitDepthConstraintIdc, 4);
        bitstr.writeBits(vpr.mThreeMinusMaxChromaFormatConstraintIdc, 2);

        // NAL unit type related
        bitstr.writeBits(vpr.mNoMixedNaluTypesInPicConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoTrailConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoStsaConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoRaslConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoRadlConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoIdrConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoCraConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoGdrConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoApsConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoIdrRplConstraintFlag, 1);

        // tile, slice, subpicture partitioning
        bitstr.writeBits(vpr.mOneTilePerPicConstraintFlag, 1);
        bitstr.writeBits(vpr.mPicHeaderInSliceHeaderConstraintFlag, 1);
        bitstr.writeBits(vpr.mOneSlicePerPicConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoRectSliceConstraintFlag, 1);
        bitstr.writeBits(vpr.mOneSlicePerSubpicConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoSubpicInfoConstraintFlag, 1);

        // CTU and block partitioning
        bitstr.writeBits(vpr.mThreeMinusMaxLog2CtuSizeConstraintIdc, 2);
        bitstr.writeBits(vpr.mNoPartitionConstraintsOverrideConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoMttConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoQtbttDualTreeIntraConstraintFlag, 1);

        // intra
        bitstr.writeBits(vpr.mNoPaletteConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoIbcConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoIspConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoMrlConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoMipConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoCclmConstraintFlag, 1);

        // inter
        bitstr.writeBits(vpr.mNoRefPicResamplingConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoResChangeInClvsConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoWeightedPredictionConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoRefWraparoundConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoTemporalMvpConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoSbtmvpConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoAmvrConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoBdofConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoSmvdConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoDmvrConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoMmvdConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoAffineMotionConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoProfConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoBcwConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoCiipConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoGpmConstraintFlag, 1);

        // transform, quantization, residual
        bitstr.writeBits(vpr.mNoLumaTransformSize64ConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoTransformSkipConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoBDPCMConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoMtsConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoLfnstConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoJointCbCrConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoSbtConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoActConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoExplicitScaleListConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoDepQuantConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoSignDataHidingConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoCuQpDeltaConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoChromaQpOffsetConstraintFlag, 1);

        // loop filter
        bitstr.writeBits(vpr.mNoSaoConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoAlfConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoCCAlfConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoLmcsConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoLadfConstraintFlag, 1);
        bitstr.writeBits(vpr.mNoVirtualBoundariesConstraintFlag, 1);

        bitstr.writeBits(0, 8);  // gci_num_reserved_bits
        // no for gci_reserved_zero_bit
    }

    //Byte alignment
    bitstr.addTrailingBits();

    // general_constraint_info end

    for (int i = numSublayers - 2; i >= 0; --i)
    {
        bitstr.writeBits(vpr.mPtlSublayerLevelPresentFlag.at(static_cast<std::size_t>(i)), 1);
    }
    for (size_t j = numSublayers; j <= 8 && numSublayers > 1; j++)
    {
        bitstr.writeBits(0, 1);
    }
    for (int i = numSublayers - 2; i >= 0; --i)
    {
        if (vpr.mPtlSublayerLevelPresentFlag.at(static_cast<std::size_t>(i)))
        {
            bitstr.writeBits(vpr.mSublayerLevelIdc.at(static_cast<std::size_t>(i)), 8);
        }
    }
    bitstr.writeBits(vpr.mNumSubProfiles, 8);
    if (vpr.mGeneralSubProfileIdc.size() != vpr.mNumSubProfiles)
    {
        throw RuntimeError("writeVvcPTLRecord: mGeneralSubProfileIdc size does not match mNumSubProfiles");
    }
    for (size_t j = 0; j < vpr.mNumSubProfiles; ++j)
    {
        bitstr.writeBits(vpr.mGeneralSubProfileIdc.at(j), 32);
    }
}

VvcDecoderConfigurationRecord::VvcPTLRecord
VvcDecoderConfigurationRecord::parseVvcPTLRecord(ISOBMFF::BitStream& bitstr, const std::uint8_t numSublayers)
{
    VvcPTLRecord vpr;

    bitstr.readBits(2);  // reserved '00'b
    const auto numBytesConstraintInfo = bitstr.readBits(6);   //bit(6) numBytesConstraintInfo
    vpr.mGeneralProfileIdc            = static_cast<uint8_t>(bitstr.readBits(7));
    vpr.mGeneralTierFlag              = static_cast<uint8_t>(bitstr.readBits(1));
    vpr.mGeneralLevelIdc              = static_cast<uint8_t>(bitstr.readBits(8));
    vpr.mPtlFrameOnlyConstraintFlag   = static_cast<uint8_t>(bitstr.readBits(1));
    vpr.mPtlMultilayerEnabledFlag     = static_cast<uint8_t>(bitstr.readBits(1));

    // general_constraint_info begins here
    // Per ISO 14496-15: numBytesConstraintInfo == 0 means no constraint info at all
    // (not even gci_present_flag). Only read GCI fields when numBytesConstraintInfo > 0.
    vpr.mGciPresentFlag = false;
    if (numBytesConstraintInfo > 0)
    {
        vpr.mGciPresentFlag = bitstr.readBits(1);
        if (vpr.mGciPresentFlag)
        {
            vpr.mIntraOnlyConstraintFlag            = bitstr.readBits(1);
            vpr.mAllLayersIndependentConstraintFlag = bitstr.readBits(1);
            vpr.mOneAuOnlyConstraintFlag            = bitstr.readBits(1);

            // picture format
            vpr.mSixteenMinusMaxBitDepthConstraintIdc   = bitstr.readBits(4);
            vpr.mThreeMinusMaxChromaFormatConstraintIdc = bitstr.readBits(2);

            // NAL unit type related
            vpr.mNoMixedNaluTypesInPicConstraintFlag = bitstr.readBits(1);
            vpr.mNoTrailConstraintFlag               = bitstr.readBits(1);
            vpr.mNoStsaConstraintFlag                = bitstr.readBits(1);
            vpr.mNoRaslConstraintFlag                = bitstr.readBits(1);
            vpr.mNoRadlConstraintFlag                = bitstr.readBits(1);
            vpr.mNoIdrConstraintFlag                 = bitstr.readBits(1);
            vpr.mNoCraConstraintFlag                 = bitstr.readBits(1);
            vpr.mNoGdrConstraintFlag                 = bitstr.readBits(1);
            vpr.mNoApsConstraintFlag                 = bitstr.readBits(1);
            vpr.mNoIdrRplConstraintFlag              = bitstr.readBits(1);

            // tile, slice, subpicture partitioning
            vpr.mOneTilePerPicConstraintFlag          = bitstr.readBits(1);
            vpr.mPicHeaderInSliceHeaderConstraintFlag = bitstr.readBits(1);
            vpr.mOneSlicePerPicConstraintFlag         = bitstr.readBits(1);
            vpr.mNoRectSliceConstraintFlag            = bitstr.readBits(1);
            vpr.mOneSlicePerSubpicConstraintFlag      = bitstr.readBits(1);
            vpr.mNoSubpicInfoConstraintFlag           = bitstr.readBits(1);

            // CTU and block partitioning
            vpr.mThreeMinusMaxLog2CtuSizeConstraintIdc        = bitstr.readBits(2);
            vpr.mNoPartitionConstraintsOverrideConstraintFlag = bitstr.readBits(1);
            vpr.mNoMttConstraintFlag                          = bitstr.readBits(1);
            vpr.mNoQtbttDualTreeIntraConstraintFlag           = bitstr.readBits(1);

            // intra
            vpr.mNoPaletteConstraintFlag = bitstr.readBits(1);
            vpr.mNoIbcConstraintFlag     = bitstr.readBits(1);
            vpr.mNoIspConstraintFlag     = bitstr.readBits(1);
            vpr.mNoMrlConstraintFlag     = bitstr.readBits(1);
            vpr.mNoMipConstraintFlag     = bitstr.readBits(1);
            vpr.mNoCclmConstraintFlag    = bitstr.readBits(1);

            // inter
            vpr.mNoRefPicResamplingConstraintFlag   = bitstr.readBits(1);
            vpr.mNoResChangeInClvsConstraintFlag    = bitstr.readBits(1);
            vpr.mNoWeightedPredictionConstraintFlag = bitstr.readBits(1);
            vpr.mNoRefWraparoundConstraintFlag      = bitstr.readBits(1);
            vpr.mNoTemporalMvpConstraintFlag        = bitstr.readBits(1);
            vpr.mNoSbtmvpConstraintFlag             = bitstr.readBits(1);
            vpr.mNoAmvrConstraintFlag               = bitstr.readBits(1);
            vpr.mNoBdofConstraintFlag               = bitstr.readBits(1);
            vpr.mNoSmvdConstraintFlag               = bitstr.readBits(1);
            vpr.mNoDmvrConstraintFlag               = bitstr.readBits(1);
            vpr.mNoMmvdConstraintFlag               = bitstr.readBits(1);
            vpr.mNoAffineMotionConstraintFlag       = bitstr.readBits(1);
            vpr.mNoProfConstraintFlag               = bitstr.readBits(1);
            vpr.mNoBcwConstraintFlag                = bitstr.readBits(1);
            vpr.mNoCiipConstraintFlag               = bitstr.readBits(1);
            vpr.mNoGpmConstraintFlag                = bitstr.readBits(1);

            // transform, quantization, residual
            vpr.mNoLumaTransformSize64ConstraintFlag = bitstr.readBits(1);
            vpr.mNoTransformSkipConstraintFlag       = bitstr.readBits(1);
            vpr.mNoBDPCMConstraintFlag               = bitstr.readBits(1);
            vpr.mNoMtsConstraintFlag                 = bitstr.readBits(1);
            vpr.mNoLfnstConstraintFlag               = bitstr.readBits(1);
            vpr.mNoJointCbCrConstraintFlag           = bitstr.readBits(1);
            vpr.mNoSbtConstraintFlag                 = bitstr.readBits(1);
            vpr.mNoActConstraintFlag                 = bitstr.readBits(1);
            vpr.mNoExplicitScaleListConstraintFlag   = bitstr.readBits(1);
            vpr.mNoDepQuantConstraintFlag            = bitstr.readBits(1);
            vpr.mNoSignDataHidingConstraintFlag      = bitstr.readBits(1);
            vpr.mNoCuQpDeltaConstraintFlag           = bitstr.readBits(1);
            vpr.mNoChromaQpOffsetConstraintFlag      = bitstr.readBits(1);

            // loop filter
            vpr.mNoSaoConstraintFlag               = bitstr.readBits(1);
            vpr.mNoAlfConstraintFlag               = bitstr.readBits(1);
            vpr.mNoCCAlfConstraintFlag             = bitstr.readBits(1);
            vpr.mNoLmcsConstraintFlag              = bitstr.readBits(1);
            vpr.mNoLadfConstraintFlag              = bitstr.readBits(1);
            vpr.mNoVirtualBoundariesConstraintFlag = bitstr.readBits(1);

            const auto numReservedBits = bitstr.readBits(8);
            bitstr.readBits(numReservedBits);
        }
    }

    // Align to the next byte boundary to skip over the GCI section (mirroring
    // the writer's addTrailingBits call). This is correct for files written by
    // this library (numBytesConstraintInfo > 0 with gci_present flag written).
    // For files where numBytesConstraintInfo == 0, we skip the GCI entirely and
    // still do byte alignment, which matches a conformant encoder that also
    // byte-aligns after the numBytesConstraintInfo field section.
    bitstr.skipToByteBoundary();
    // general_constraint_info ends here

    if (numSublayers > 0)
    {
        vpr.mPtlSublayerLevelPresentFlag.resize(numSublayers - 1);
        for (int i = numSublayers - 2; i >= 0; --i)
        {
            vpr.mPtlSublayerLevelPresentFlag.at(static_cast<std::size_t>(i)) = bitstr.readBits(1);
        }

        for (size_t j = numSublayers; j <= 8 && numSublayers > 1; j++)
        {
            bitstr.readBits(1);
        }

        vpr.mSublayerLevelIdc.resize(numSublayers);
        vpr.mSublayerLevelIdc.at(numSublayers - 1) = vpr.mGeneralLevelIdc;
        for (int i = numSublayers - 2; i >= 0; i--)
        {
            if (vpr.mPtlSublayerLevelPresentFlag.at(static_cast<std::size_t>(i)))
            {
                vpr.mSublayerLevelIdc.at(static_cast<std::size_t>(i)) = static_cast<uint8_t>(bitstr.readBits(8));
            }
        }
        for (int i = numSublayers - 2; i >= 0; i--)
        {
            if (!vpr.mPtlSublayerLevelPresentFlag.at(static_cast<std::size_t>(i)))
            {
                // NOLINTNEXTLINE(bugprone-misplaced-widening-cast): i+1 stays in int range; numSublayers is < 7.
                vpr.mSublayerLevelIdc.at(static_cast<std::size_t>(i)) = vpr.mSublayerLevelIdc.at(static_cast<std::size_t>(i+1));
            }
        }
    }

    vpr.mNumSubProfiles = bitstr.read8Bits();
    vpr.mGeneralSubProfileIdc.resize(vpr.mNumSubProfiles);
    for (size_t j = 0; j < vpr.mNumSubProfiles; ++j)
    {
        vpr.mGeneralSubProfileIdc.at(j) = bitstr.read32Bits();
    }

    return vpr;
}
