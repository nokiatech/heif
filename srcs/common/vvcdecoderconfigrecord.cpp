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

#include "vvcdecoderconfigrecord.hpp"

#include <algorithm>
#include <cassert>

#include "bitstream.hpp"
#include "nalutil.hpp"
#include "vvcparser.hpp"

VvcDecoderConfigurationRecord::VvcDecoderConfigurationRecord()
    : mNalArray()
{
}


bool VvcDecoderConfigurationRecord::makeConfigFromSPS(const Vector<uint8_t>& srcSps)
{
    const std::vector<uint8_t> data(srcSps.begin(), srcSps.end());
    const VvcParser::SpsInfo spsInfo = VvcParser::parseSps(data);

    mConfigurationVersion = 1;
    mLengthSizeMinus1     = 3;  // NAL length fields are 4 bytes long (3+1)

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

    // add NAL unit to the NAL unit array
    nalArray->nalList.push_back(tmpNalUnit);
}

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
        bool arrayCompleteness;
        VvcNalUnitType nalUnitType;
        unsigned int numNalus = 1;

        arrayCompleteness = (bitstr.readBits(1) != 0);
        bitstr.readBits(2);  // reserved = 0
        nalUnitType = static_cast<VvcNalUnitType>(bitstr.readBits(5));

        if (nalUnitType != VvcNalUnitType::DCI_NUT && nalUnitType != VvcNalUnitType::OPI_NUT)
        {
            numNalus = bitstr.readBits(16);
        }
        for (unsigned int j = 0; j < numNalus; j++)
        {
            Vector<uint8_t> nalData;
            unsigned int nalSize;

            nalSize = bitstr.readBits(16);
            nalData.clear();
            bitstr.read8BitsArray(nalData, nalSize);  // read parameter set NAL unit
            addNalUnit(nalData, nalUnitType, arrayCompleteness);
        }
    }
}

void VvcDecoderConfigurationRecord::getOneParameterSet(Vector<uint8_t>& byteStream,
                                                       const VvcNalUnitType nalUnitType) const
{
    for (const auto& array : mNalArray)
    {
        if (array.nalUnitType == nalUnitType && array.nalList.size() > 0)
        {
            // add start code (0x00000001) before the NAL unit
            byteStream.push_back(0);
            byteStream.push_back(0);
            byteStream.push_back(0);
            byteStream.push_back(1);
            byteStream.insert(byteStream.end(), array.nalList.at(0).cbegin(), array.nalList.at(0).cend());
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

void VvcDecoderConfigurationRecord::getConfigurationMap(ConfigurationMap& aMap) const
{
    Vector<std::uint8_t> aps;
    Vector<std::uint8_t> pps;
    Vector<std::uint8_t> sps;
    Vector<std::uint8_t> vps;
    getOneParameterSet(aps, VvcNalUnitType::PREFIX_APS_NUT);
    getOneParameterSet(pps, VvcNalUnitType::PPS_NUT);
    getOneParameterSet(sps, VvcNalUnitType::SPS_NUT);
    getOneParameterSet(vps, VvcNalUnitType::VPS_NUT);

    aMap.insert({DecoderParameterType::VVC_APS, move(aps)});
    aMap.insert({DecoderParameterType::VVC_SPS, move(sps)});
    aMap.insert({DecoderParameterType::VVC_PPS, move(pps)});
    aMap.insert({DecoderParameterType::VVC_VPS, move(vps)});
}

void VvcDecoderConfigurationRecord::writeVvcPTLRecord(ISOBMFF::BitStream& bitstr,
                                                      const VvcPTLRecord& vpr,
                                                      const std::uint8_t numSublayers) const
{
    assert(bitstr.isByteAligned());

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

    while (!bitstr.isByteAligned())
    {
        bitstr.writeBits(0, 1);  // gci_alignment_zero_bit
    }

    // general_constraint_info end

    for (auto i = numSublayers - 2; i >= 0; --i)
    {
        bitstr.writeBits(vpr.mPtlSublayerLevelPresentFlag.at(i), 1);
    }
    for (auto j = numSublayers; j <= 8 && numSublayers > 1; j++)
    {
        bitstr.writeBits(0, 1);
    }
    for (auto i = numSublayers - 2; i >= 0; --i)
    {
        if (vpr.mPtlSublayerLevelPresentFlag.at(i))
        {
            bitstr.writeBits(vpr.mSublayerLevelIdc.at(i), 8);
        }
    }
    bitstr.writeBits(vpr.mNumSubProfiles, 8);
    assert(vpr.mGeneralSubProfileIdc.size() == vpr.mNumSubProfiles);
    for (auto j = 0; j < vpr.mNumSubProfiles; ++j)
    {
        bitstr.writeBits(vpr.mGeneralSubProfileIdc.at(j), 32);
    }
}

VvcDecoderConfigurationRecord::VvcPTLRecord
VvcDecoderConfigurationRecord::parseVvcPTLRecord(ISOBMFF::BitStream& bitstr, const std::uint8_t numSublayers)
{
    VvcPTLRecord vpr;

    const auto reserved = bitstr.readBits(2);  // reserved '00'b
    assert(reserved == 0);
    const auto numBytesConstraintInfo = bitstr.readBits(6);
    vpr.mGeneralProfileIdc            = bitstr.readBits(7);
    vpr.mGeneralTierFlag              = bitstr.readBits(1);
    vpr.mGeneralLevelIdc              = bitstr.readBits(8);
    vpr.mPtlFrameOnlyConstraintFlag   = bitstr.readBits(1);
    vpr.mPtlMultilayerEnabledFlag     = bitstr.readBits(1);

    // general_constraint_info begins here
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

        const auto numReservedBits = bitstr.read8Bits();
        bitstr.readBits(numReservedBits);
    }

    while (!bitstr.isByteAligned())
    {
        const auto zeroBit = bitstr.readBits(1);
        assert(zeroBit == 0);
    }
    // general_constraint_info ends here

    if (numSublayers > 0)
    {
        vpr.mPtlSublayerLevelPresentFlag.resize(numSublayers - 1);
        for (int i = numSublayers - 2; i >= 0; --i)
        {
            vpr.mPtlSublayerLevelPresentFlag.at(i) = bitstr.readBits(1);
        }

        for (auto j = numSublayers; j <= 8 && numSublayers > 1; j++)
        {
            bitstr.readBits(1);
        }

        vpr.mSublayerLevelIdc.resize(numSublayers);
        vpr.mSublayerLevelIdc.at(numSublayers - 1) = vpr.mGeneralLevelIdc;
        for (int i = numSublayers - 2; i >= 0; i--)
        {
            if (vpr.mPtlSublayerLevelPresentFlag.at(i))
            {
                vpr.mSublayerLevelIdc.at(i) = bitstr.readBits(1);
            }
        }
        for (int i = numSublayers - 2; i >= 0; i--)
        {
            if (!vpr.mPtlSublayerLevelPresentFlag.at(i))
            {
                vpr.mSublayerLevelIdc.at(i) = vpr.mSublayerLevelIdc.at(i + 1);
            }
        }
    }

    vpr.mNumSubProfiles = bitstr.read8Bits();
    vpr.mGeneralSubProfileIdc.resize(vpr.mNumSubProfiles);
    for (auto j = 0; j < vpr.mNumSubProfiles; ++j)
    {
        vpr.mGeneralSubProfileIdc.at(j) = bitstr.read32Bits();
    }

    return vpr;
}
