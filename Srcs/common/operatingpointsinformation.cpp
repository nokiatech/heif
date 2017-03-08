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

#include "operatingpointsinformation.hpp"
#include "bitstream.hpp"
#include "h265parser.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

OperatingPointsInformation::OperatingPointsInformation() :
    FullBox("oinf", 0, 0),
    mScalabilityMask(0)
{
}

std::pair<uint32_t, uint32_t> OperatingPointsInformation::getMaxExtents(const std::uint16_t outputLayerSetIndex) const
{
    for (const auto& operatingPoint : mOperatingPoints)
    {
        if (operatingPoint.mOutputLayerSetIdx == outputLayerSetIndex)
        {
            return std::pair<uint32_t, uint32_t>(operatingPoint.mMaxPicWidth, operatingPoint.mMaxPicHeight);
        }
    }

    throw std::runtime_error("OperatingPointsInformation::getExtents() requested output layer set index not found.");
}

unsigned int OperatingPointsInformation::ceilLog2(const unsigned int x)
{
    unsigned int i = 1;

    while (x > (1u << i))
    {
        ++i;
    }

    return i;
}

void OperatingPointsInformation::parseSubHrd(BitStream& bitstr, SubHrd& hrd, const int cpbCnt,
                                             const unsigned int subPicHrdParamsPresentFlag)
{
    hrd.mBitRateValueMinus1.clear();
    hrd.mCpbSizeValueMinus1.clear();
    hrd.mCpbSizeDuValueMinus1.clear();
    hrd.mBitRateDuValueMinus1.clear();
    hrd.mCbrFlag.clear();

    for (int i = 0; i <= cpbCnt; ++i)
    {
        hrd.mBitRateValueMinus1.push_back(bitstr.readExpGolombCode());
        hrd.mCpbSizeValueMinus1.push_back(bitstr.readExpGolombCode());
        if (subPicHrdParamsPresentFlag)
        {
            hrd.mCpbSizeDuValueMinus1.push_back(bitstr.readExpGolombCode());
            hrd.mBitRateDuValueMinus1.push_back(bitstr.readExpGolombCode());
        }
        hrd.mCbrFlag.push_back(bitstr.readExpGolombCode());
    }
}

void OperatingPointsInformation::parseHrd(BitStream& bitstr, Hrd& hrd, const unsigned int commonInfPresentFlag,
                                          const unsigned int maxNumSubLayersMinus1)
{
    hrd.mSubPicHrdParamsPresentFlag = 0;
    if (commonInfPresentFlag)
    {
        hrd.mNalHrdParameterPresentFlag = bitstr.readBits(1);
        hrd.mVclHrdParametersPresentFlag = bitstr.readBits(1);
        if (hrd.mNalHrdParameterPresentFlag || hrd.mVclHrdParametersPresentFlag)
        {
            hrd.mSubPicHrdParamsPresentFlag = bitstr.readBits(1);
            if (hrd.mSubPicHrdParamsPresentFlag)
            {
                hrd.mTickDivisorMinus2 = bitstr.readBits(8);
                hrd.mDuCpbRemovalDelayIncrementLengthMinus1 = bitstr.readBits(5);
                hrd.mSubPicCpbParamsInPicTimingSeiFlag = bitstr.readBits(1);
                hrd.mDpbOutputDelayDuLengthMinus1 = bitstr.readBits(5);
            }
            hrd.mBitRateScale = bitstr.readBits(4);
            hrd.mCpbSizeScale = bitstr.readBits(4);
            if (hrd.mSubPicHrdParamsPresentFlag)
            {
                hrd.mCpbSizeDuScale = bitstr.readBits(4);
            }
            hrd.mInitialCpbRemovalDelayLengthMinus1 = bitstr.readBits(5);
            hrd.mAuCpbRemovalDelayLengthMinus1 = bitstr.readBits(5);
            hrd.mDpbOutputDelayLengthMinus1 = bitstr.readBits(5);
        }
    }
    hrd.mFixedPicRateGeneralFlag.resize(maxNumSubLayersMinus1 + 1);
    hrd.mFixedPicRateWithinCvsFlag.resize(maxNumSubLayersMinus1 + 1);
    hrd.mElementalDurationInTcMinus1.resize(maxNumSubLayersMinus1 + 1);
    hrd.mLowDelayHrdFlag.resize(maxNumSubLayersMinus1 + 1);
    hrd.mCpbCntMinus1.resize(maxNumSubLayersMinus1 + 1);
    if (hrd.mNalHrdParameterPresentFlag)
    {
        hrd.mSubLayerNalHrdParams.resize(maxNumSubLayersMinus1 + 1);
    }
    if (hrd.mVclHrdParametersPresentFlag)
    {
        hrd.mSubLayerVclHrdParams.resize(maxNumSubLayersMinus1 + 1);
    }
    for (unsigned int i = 0; i <= maxNumSubLayersMinus1; ++i)
    {
        hrd.mFixedPicRateGeneralFlag[i] = bitstr.readBits(1);
        if (!hrd.mFixedPicRateGeneralFlag[i])
        {
            hrd.mFixedPicRateWithinCvsFlag[i] = bitstr.readBits(1);
        }
        if (hrd.mFixedPicRateWithinCvsFlag[i])
        {
            hrd.mElementalDurationInTcMinus1[i] = bitstr.readExpGolombCode();
        }
        else
        {
            hrd.mLowDelayHrdFlag[i] = bitstr.readBits(1);
        }
        hrd.mCpbCntMinus1[i] = 0;
        if (!hrd.mLowDelayHrdFlag[i])
        {
            hrd.mCpbCntMinus1[i] = bitstr.readExpGolombCode();
        }
        if (hrd.mNalHrdParameterPresentFlag)
        {
            parseSubHrd(bitstr, hrd.mSubLayerNalHrdParams[i], hrd.mCpbCntMinus1[i], hrd.mSubPicHrdParamsPresentFlag);
        }
        if (hrd.mVclHrdParametersPresentFlag)
        {
            parseSubHrd(bitstr, hrd.mSubLayerVclHrdParams[i], hrd.mCpbCntMinus1[i], hrd.mSubPicHrdParamsPresentFlag);
        }
    }
}

void OperatingPointsInformation::parsePtl(BitStream& bitstr, Ptl& ptl, const unsigned int maxNumSubLayersMinus1,
                                         const unsigned int profilePresentFlag)
{
    if (profilePresentFlag)
    {
        ptl.mGeneralProfileSpace = bitstr.readBits(2);
        ptl.mGeneralTierFlag = bitstr.readBits(1);
        ptl.mGeneralProfileIdc = bitstr.readBits(5);
        for (int i = 0; i < 32; ++i)
        {
            ptl.mGeneralProfileCompatibilityFlag.push_back(bitstr.readBits(1));
        }
        // general_constraint_indicator_flags (48 flags)
        for (int i = 0; i < 6; i++)
        {
            ptl.mGeneralConstraintIndicatorFlags.push_back(static_cast<uint8_t>(bitstr.readBits(8)));
        }
    }
    ptl.mGeneralLevelIdc = bitstr.readBits(8);
    for (unsigned int i = 0; i < maxNumSubLayersMinus1; ++i)
    {
        ptl.mSubLayerProfilePresentFlag.push_back(bitstr.readBits(1));
        ptl.mSubLayerLevelPresentFlag.push_back(bitstr.readBits(1));
    }
    if (maxNumSubLayersMinus1 > 0)
    {
        for (int i = maxNumSubLayersMinus1; i < 8; ++i)
        {
            bitstr.readBits(2);  // reserved_zeor_2bits
        }
    }
    for (unsigned int i = 0; i < maxNumSubLayersMinus1; ++i)
    {
        if (ptl.mSubLayerProfilePresentFlag[i])
        {
            bitstr.readBits(2);
            bitstr.readBits(1);
            bitstr.readBits(5);
            for (int j = 0; j < 32; ++j)
            {
                bitstr.readBits(1);
            }
            bitstr.readBits(1);
            bitstr.readBits(1);
            bitstr.readBits(1);
            bitstr.readBits(1);
            for (int i = 0; i < 2; ++i)  // sub_layer_reserved_zero_44bits
            {
                bitstr.readBits(22);
            }
        }
        if (ptl.mSubLayerLevelPresentFlag[i])
        {
            bitstr.readBits(8);
        }
    }
}

void OperatingPointsInformation::parseVps(BitStream& bitstr, Vps& vps)
{
    bitstr.readBits(1);         // forbidden_zero_bit
    bitstr.readBits(6);         // nal_unit_type
    bitstr.readBits(6);         // nuh_layer_id
    bitstr.readBits(3);         // nuh_temporal_id_plus1

    vps.mVpsId = bitstr.readBits(4);
    vps.mBaseLayerInternalFlag = bitstr.readBits(1);
    vps.mBaseLayerAvailableFlag = bitstr.readBits(1);
    vps.mMaxLayersMinus1 = bitstr.readBits(6);
    vps.mMaxSubLayersMinus1 = bitstr.readBits(3);
    vps.mTemporalIdNestingFlag = bitstr.readBits(1);
    vps.mReserved0xffff16Bits = bitstr.readBits(16);
    parsePtl(bitstr, vps.mProfileTierLevel, vps.mMaxSubLayersMinus1);
    vps.mSubLayerOrderingInfoPresentFlag = bitstr.readBits(1);
    for (unsigned int i = (vps.mSubLayerOrderingInfoPresentFlag ? 0 : vps.mMaxSubLayersMinus1);
        i <= vps.mMaxSubLayersMinus1; i++)
    {
        vps.mMaxDecPicBufferingMinus1.push_back(bitstr.readExpGolombCode());
        vps.mMaxNumReorderPics.push_back(bitstr.readExpGolombCode());
        vps.mMaxLatencyIncreasePlus1.push_back(bitstr.readExpGolombCode());
    }
    vps.mMaxLayerId = bitstr.readBits(6);
    vps.mNumLayerSetsMinus1 = bitstr.readExpGolombCode();
    vps.mLayerIdIncludedFlag.resize(vps.mNumLayerSetsMinus1 + 1);
    for (unsigned int i = 1; i <= vps.mNumLayerSetsMinus1; i++)
    {
        std::vector<unsigned int> layerIdIncludedFlag(vps.mMaxLayerId + 1);
        for (unsigned int j = 0; j <= vps.mMaxLayerId; j++)
        {
            layerIdIncludedFlag.at(j) = bitstr.readBits(1);
        }
        vps.mLayerIdIncludedFlag.at(i) = layerIdIncludedFlag;
    }
    vps.mTimingInfoPresentFlag = bitstr.readBits(1);
    if (vps.mTimingInfoPresentFlag)
    {
        vps.mNumUnitsInTick = bitstr.readBits(32);
        vps.mTimeScale = bitstr.readBits(32);
        vps.mPocProportionalToTimingFlag = bitstr.readBits(1);
        if (vps.mPocProportionalToTimingFlag)
        {
            vps.mNumTicksPocDiffOneMinus1 = bitstr.readExpGolombCode();
        }
        vps.mNumHrdParameter = bitstr.readExpGolombCode();
        for (unsigned int i = 0; i < vps.mNumHrdParameter; i++)
        {
            vps.mHrdLayerSetIdx.push_back(bitstr.readExpGolombCode());
            if (i > 0)
            {
                vps.mCprmsPresentFlag.push_back(bitstr.readBits(1));
            }
            parseHrd(bitstr, vps.mHrdParameters.back(), vps.mCprmsPresentFlag.at(i), vps.mMaxSubLayersMinus1);
        }
    }
    vps.mExtensionFlag = bitstr.readBits(1);
    if (vps.mExtensionFlag)
    {
        while (!bitstr.isByteAligned())
        {
            vps.mExtensionAlignmentBitEqualToOne.push_back(bitstr.readBits(1));
        }

        parseVpsExt(bitstr, vps, vps.mExtension);
    }
}

void OperatingPointsInformation::parseRep(BitStream& bitstr, Rep& rep)
{
    rep.mPicWidthVpsInLumaSamples = bitstr.readBits(16);
    rep.mPicHeightVpsInLumaSamples = bitstr.readBits(16);
    rep.mChromaAndBitDepthVpsPresentFlag = bitstr.readBits(1);
    if (rep.mChromaAndBitDepthVpsPresentFlag)
    {
        rep.mChromaFormatVpsIdc = bitstr.readBits(2);
        if (rep.mChromaFormatVpsIdc == 3)
        {
            rep.mSeparateColourPlaneVpsFlag = bitstr.readBits(1);
        }
        rep.mBitDepthVpsLumaMinus8 = bitstr.readBits(4);
        rep.mBitDepthVpsChromaMinus8 = bitstr.readBits(4);
    }

    rep.mConformanceWindowVpsFlag = bitstr.readBits(1);
    if (rep.mConformanceWindowVpsFlag)
    {
        rep.mConfWinVpsLeftOffset = bitstr.readExpGolombCode();
        rep.mConfWinVpsRightOffset = bitstr.readExpGolombCode();
        rep.mConfWinVpsTopOffset = bitstr.readExpGolombCode();
        rep.mConfWinVpsBottomOffset = bitstr.readExpGolombCode();
    }
}

void OperatingPointsInformation::parseDpb()
{
    /// @todo Remove this?
    return;
}

void OperatingPointsInformation::parseVui()
{
    /// @todo Remove this?
    return;
}

void OperatingPointsInformation::parseVpsExt(BitStream& bitstr, Vps& vps, VpsExt& vpsExt)
{
    if (vps.mMaxLayersMinus1 > 0 && vps.mBaseLayerInternalFlag)
    {
        parsePtl(bitstr, vpsExt.mProfileTierLevel, vps.mMaxSubLayersMinus1, 0);
    }
    vpsExt.mSplittingFlag = bitstr.readBits(1);

    vpsExt.mNumScalabilityTypes = 0;
    for (unsigned int i = 0; i < 16; i++)
    {
        vpsExt.mScalabilityMaskFlag.push_back(bitstr.readBits(1));
        vpsExt.mNumScalabilityTypes += vpsExt.mScalabilityMaskFlag.back();
    }

    vpsExt.mDimensionIdLenMinus1.resize(vpsExt.mNumScalabilityTypes - vpsExt.mSplittingFlag);
    for (unsigned int j = 0; j < (vpsExt.mNumScalabilityTypes - vpsExt.mSplittingFlag); j++)
    {
        vpsExt.mDimensionIdLenMinus1.at(j) = bitstr.readBits(3);
    }
    vpsExt.mVpsNuhLayerIdPresentFlag = bitstr.readBits(1);

    unsigned int maxLayersMinus1 = std::min((unsigned int)62, vps.mMaxLayersMinus1);
    vpsExt.mLayerIdInNuh.resize(maxLayersMinus1 + 1);
    vpsExt.mDimensionId.resize(maxLayersMinus1 + 1);
    for (unsigned int i = 1; i <= maxLayersMinus1; i++)
    {
        if (vpsExt.mVpsNuhLayerIdPresentFlag)
        {
            vpsExt.mLayerIdInNuh.at(i) = bitstr.readBits(6);
        }
        else
        {
            vpsExt.mLayerIdInNuh.at(i) = i;
        }
        if (!vpsExt.mSplittingFlag)
        {
            std::vector<unsigned int> dimensionId(vpsExt.mNumScalabilityTypes);
            for (unsigned int j = 0; j < vpsExt.mNumScalabilityTypes; j++)
            {
                unsigned int numBits = vpsExt.mDimensionIdLenMinus1.at(j) + 1;
                dimensionId.at(j) = bitstr.readBits(numBits);
            }
            vpsExt.mDimensionId.at(i) = dimensionId;
        }
    }

    vpsExt.mViewIdLen = bitstr.readBits(4);

    vpsExt.mLayerIdxInVps.resize(maxLayersMinus1 + 1);
    for (unsigned int i = 0; i <= maxLayersMinus1; i++)
    {
        vpsExt.mLayerIdxInVps.at(vpsExt.mLayerIdInNuh.at(i)) = i;
    }

    if (vpsExt.mViewIdLen > 0)
    {
        // NOTE numViews has to be calculated based on algo present in pg 387 of the
        // standard. For now set it to one.
        unsigned int numViews = 1;
        for (unsigned int i = 1; i < numViews; i++)
        {
            vpsExt.mViewIdVal.push_back(bitstr.readExpGolombCode());
        }
    }

    // NOTE: This part is nasty. Refactor
    vpsExt.mDirectDependencyFlag.resize(maxLayersMinus1 + 1);
    for (unsigned int i = 0; i <= maxLayersMinus1; i++)
    {
        vpsExt.mDirectDependencyFlag.at(i).resize(maxLayersMinus1 + 1);
    }

    for (unsigned int i = 1; i <= maxLayersMinus1; i++)
    {
        for (unsigned int j = 0; j < i; j++)
        {
            vpsExt.mDirectDependencyFlag.at(i).at(j) = bitstr.readBits(1);
        }
    }

    // Derivation for DependencyFlag, numDirectRefLayers, idDirectRefLayer,
    // numRefLayers, idRefLayer, numPredictedLayers, and idPredictedLayer are
    // done now
    std::vector<std::vector<unsigned int>> dependencyFlag(maxLayersMinus1 + 1,
        std::vector<unsigned int>(maxLayersMinus1 + 1));
    for (unsigned int i = 0; i <= maxLayersMinus1; i++)
    {
        for (unsigned int j = 0; j <= maxLayersMinus1; j++)
        {
            dependencyFlag.at(i).at(j) = vpsExt.mDirectDependencyFlag.at(i).at(j);
            for (unsigned int k = 0; k < i; k++)
            {
                if (vpsExt.mDirectDependencyFlag.at(i).at(k) && dependencyFlag.at(k).at(j))
                {
                    dependencyFlag.at(i).at(j) = 1;
                }
            }
        }
    }

    std::vector<std::vector<unsigned int>> idDirectRefLayer(maxLayersMinus1 + 1);
    std::vector<std::vector<unsigned int>> idRefLayer(maxLayersMinus1 + 1);
    std::vector<std::vector<unsigned int>> idPredictedLayer(maxLayersMinus1 + 1);

    std::vector<unsigned int> numDirectRefLayers(maxLayersMinus1 + 1);
    std::vector<unsigned int> numRefLayer(maxLayersMinus1 + 1);
    std::vector<unsigned int> numPredictedLayer(maxLayersMinus1 + 1);
    for (unsigned int i = 0; i <= maxLayersMinus1; i++)
    {
        unsigned int iNuhLId = vpsExt.mLayerIdInNuh.at(i);
        unsigned int d = 0;
        unsigned int r = 0;
        unsigned int p = 0;
        for (unsigned j = 0; j <= maxLayersMinus1; j++)
        {
           unsigned int jNuhLid = vpsExt.mLayerIdInNuh.at(j);
           if(vpsExt.mDirectDependencyFlag.at(i).at(j))
           {
               idDirectRefLayer.at(iNuhLId).push_back(jNuhLid);
               d++;
           }
           if(dependencyFlag.at(i).at(j))
           {
               idRefLayer.at(iNuhLId).push_back(jNuhLid);
               r++;
           }
           if(dependencyFlag.at(j).at(i))
           {
               idPredictedLayer.at(iNuhLId).push_back(jNuhLid);
               p++;
           }
        }
        numDirectRefLayers.at(iNuhLId) = d;
        numRefLayer.at(iNuhLId) = r;
        numPredictedLayer.at(iNuhLId) = p;
    }

    // Note: this copying is horrible; refactor later.
    vpsExt.mIdDirectRefLayer = idDirectRefLayer;
    vpsExt.mIdRefLayer = idRefLayer;
    vpsExt.mIdPredictedLayer = idPredictedLayer;
    vpsExt.mNumDirectRefLayers = numDirectRefLayers;
    vpsExt.mNumRefLayer = numRefLayer;
    vpsExt.mNumPredictedLayer = numPredictedLayer;

    std::vector<std::vector<unsigned int>> treePartitionLayerIdList(maxLayersMinus1 + 1,
        std::vector<unsigned int>(maxLayersMinus1 + 1));
    std::vector<unsigned int> numLayersInTreePartition(maxLayersMinus1 + 1);
    std::vector<unsigned int> layerIdInListFlag(63);
    unsigned int k = 0;
    for (unsigned int i = 0; i <= maxLayersMinus1; i++)
    {
        unsigned int iNuhLId = vpsExt.mLayerIdInNuh.at(i);
        if (numDirectRefLayers.at(iNuhLId))
        {
            treePartitionLayerIdList.at(k).at(0) = iNuhLId;
            unsigned int h = 1;
            for (unsigned int j = 0; j < numPredictedLayer.at(iNuhLId); j++)
            {
                unsigned int predLId = idPredictedLayer.at(iNuhLId).at(j);
                if (!layerIdInListFlag.at(predLId))
                {
                    treePartitionLayerIdList.at(k).at(h++) = predLId;
                    layerIdInListFlag.at(predLId) = 1;
                }
            }
            numLayersInTreePartition.at(k++) = h;
        }
    }
    unsigned int numIndependentLayers = k;

    vpsExt.mNumAddLayerSets = 0;
    if (numIndependentLayers > 1)
    {
        vpsExt.mNumAddLayerSets = bitstr.readExpGolombCode();
    }

    // derivation of numLayerSet
    unsigned int numLayerSets = vps.mNumLayerSetsMinus1 + 1 + vpsExt.mNumAddLayerSets;

    // Should vpsExt.mHighestLayerIdxPlus1 be an array?
    for (unsigned int i = 0; i < vpsExt.mNumAddLayerSets; i++)
    {
        std::vector<unsigned int> highestLayerIdxPlus1;
        for (unsigned int j = 0; j < numIndependentLayers; j++)
        {
            unsigned int numBits = ceilLog2(numLayersInTreePartition.at(j) + 1);
            highestLayerIdxPlus1.push_back(numBits);
        }
        vpsExt.mHighestLayerIdxPlus1.push_back(highestLayerIdxPlus1);
    }

    // derivation for numLayersInIdList
    std::vector<std::vector<unsigned int>> layerSetLayerIdList(2048, std::vector<unsigned int>(8));
    std::vector<unsigned int> numLayersInIdList(2048);
    numLayersInIdList.at(0) = 1;
    for (unsigned int i = 1; i <= vps.mNumLayerSetsMinus1; i++)
    {
        unsigned int n = 0;
        for (unsigned int m = 0; m <= vps.mMaxLayerId; m++)
        {
            if (vps.mLayerIdIncludedFlag.at(i).at(m))
            {
                layerSetLayerIdList.at(i).at(n++) = m;
            }
        }
        numLayersInIdList.at(i) = n;
    }

    vpsExt.mNumLayersInIdList = numLayersInIdList;
    vpsExt.mLayerSetLayerIdList = layerSetLayerIdList;

    unsigned int layerNum = 0;
    unsigned int isIdx = 0;
    for (unsigned int i = 0; i < vpsExt.mNumAddLayerSets; i++)
    {
        isIdx = vps.mNumLayerSetsMinus1 + 1 + i;
        for (unsigned int treeIdx = 1; treeIdx < numIndependentLayers; treeIdx++)
        {
            std::vector<unsigned int> LayerSetLayerIdList;
            for (unsigned int layerCnt = 0; layerCnt < vpsExt.mHighestLayerIdxPlus1.at(i).at(treeIdx); layerCnt++)
            {
                layerSetLayerIdList.at(isIdx).at(layerNum++) = treePartitionLayerIdList.at(treeIdx).at(layerCnt);
            }
        }
        numLayersInIdList.at(isIdx) = layerNum;
    }

    vpsExt.mVpsSubLayersMaxMinus1PresentFlag = bitstr.readBits(1);
    if (vpsExt.mVpsSubLayersMaxMinus1PresentFlag)
    {
        for (unsigned int i = 0; i <= maxLayersMinus1; i++)
        {
            vpsExt.mSubLayersVpsMaxMinus1.push_back(bitstr.readBits(3));
        }
    }

    // Derivation for maxSubLayersInLayerSetMinus1
    std::vector<unsigned int> maxSubLayersInLayerSetMinus1(numLayerSets);
    for (unsigned int i = 0; i < numLayerSets; i++)
    {
        unsigned int maxSlMinus1 = 0;
        for (unsigned int k = 0; k < numLayersInIdList.at(i); k++)
        {
            unsigned int lid = layerSetLayerIdList.at(i).at(k);
            maxSlMinus1 = std::max(maxSlMinus1, vpsExt.mSubLayersVpsMaxMinus1.at(vpsExt.mLayerIdxInVps.at(lid)));
        }
        maxSubLayersInLayerSetMinus1.at(i) = maxSlMinus1;
    }

    vpsExt.mMaxTidRefPresentFlag = bitstr.readBits(1);

    if (vpsExt.mMaxTidRefPresentFlag)
    {
        for (unsigned int i = 0; i < maxLayersMinus1; i++)
        {
            for (unsigned int j = (i + 1); j <= maxLayersMinus1; j++)
            {
                std::vector<unsigned int> maxTidIlRefPicsPlus1;
                if (vpsExt.mDirectDependencyFlag.at(j).at(i))
                {
                    maxTidIlRefPicsPlus1.push_back(bitstr.readBits(3));
                }
                vpsExt.mMaxTidIlRefPicsPlus1.push_back(maxTidIlRefPicsPlus1);
            }
        }
    }

    vpsExt.mDefaultRefLayersActiveFlag = bitstr.readBits(1);

    vpsExt.mVpsNumProfileTierLevelMinus1 = bitstr.readExpGolombCode();

    for (unsigned int i = (vps.mBaseLayerInternalFlag ? 2 : 1); i <= vpsExt.mVpsNumProfileTierLevelMinus1; i++)
    {
        vpsExt.mVpsProfilePresentFlag.push_back(bitstr.readBits(1));
        vpsExt.mProfileTierLevelArray.resize(vpsExt.mProfileTierLevelArray.size() + 1);
        parsePtl(bitstr, vpsExt.mProfileTierLevelArray.back(), vps.mMaxSubLayersMinus1,
            vpsExt.mVpsProfilePresentFlag.back());
    }

    // Note: Derive numLayerSets
    if (numLayerSets > 1)
    {
        vpsExt.mNumAddOlss = bitstr.readExpGolombCode();
        vpsExt.mDefaultOutputLayerIdc = bitstr.readBits(2);
    }

    unsigned int numOutputLayerSets = vpsExt.mNumAddOlss + numLayerSets;
    vpsExt.mLayerSetIdxForOlsMinus1.resize(numOutputLayerSets);
    for (unsigned int i = 1; i < numOutputLayerSets; i++)
    {
        unsigned int numBits = ceilLog2(numLayerSets - 1);
        if (numLayerSets > 2 && i >= numLayerSets)
        {
            vpsExt.mLayerSetIdxForOlsMinus1.at(i) = bitstr.readBits(numBits);
        }
    }

    // Derivation olsIdxToLsIdx
    std::vector<unsigned int> olsIdxToLsIdx(numOutputLayerSets);
    for (unsigned int i = 0; i < numOutputLayerSets; i++)
    {
        olsIdxToLsIdx.at(i) = (i < numLayerSets) ? i : (vpsExt.mLayerSetIdxForOlsMinus1.at(i) + 1);
    }

    vpsExt.mOlsIdxToLsIdx = olsIdxToLsIdx;

    vpsExt.mOutputLayerFlag.resize(numOutputLayerSets);
    for (unsigned int i = 1; i < numOutputLayerSets; i++)
    {
        // What is the size of outputLayerFlag??
        // std::vector<unsigned int> outputLayerFlag(8);
        // Should this be this?
        std::vector<unsigned int> outputLayerFlag;
        if (i == 1)
        {
            outputLayerFlag.resize(numLayersInIdList.at(olsIdxToLsIdx.at(0)));
            outputLayerFlag.at(0) = 1;
            vpsExt.mOutputLayerFlag.at(0) = outputLayerFlag;
        }
        if ((i > vps.mNumLayerSetsMinus1) || (vpsExt.mDefaultOutputLayerIdc == 2))
        {
            outputLayerFlag.resize(numLayersInIdList.at(olsIdxToLsIdx.at(i)));
            for (unsigned int j = 0; j < numLayersInIdList[olsIdxToLsIdx[i]]; j++)
            {
                outputLayerFlag.at(j) = bitstr.readBits(1);
            }
            vpsExt.mOutputLayerFlag.at(i) = outputLayerFlag;
        }
    }

    // Note: The following part of deriving  outputLayerFlag requires some
    // refactoring.
    vpsExt.mOutputLayerFlagDerived.resize(numOutputLayerSets);
    if ((vpsExt.mDefaultOutputLayerIdc == 0) || (vpsExt.mDefaultOutputLayerIdc == 1))
    {
        for (unsigned int i = 0; i <= vps.mNumLayerSetsMinus1; i++)
        {
            std::vector<unsigned int> outputLayerFlag(numLayersInIdList.at(olsIdxToLsIdx.at(i)));
            unsigned int maxValInlayerSetLayerIdList = 0;
            for (unsigned int j = 0; j < numLayersInIdList.at(olsIdxToLsIdx.at(i)); j++)
            {
                if (layerSetLayerIdList.at(olsIdxToLsIdx.at(i)).at(j) > maxValInlayerSetLayerIdList)
                {
                    maxValInlayerSetLayerIdList = layerSetLayerIdList.at(olsIdxToLsIdx.at(i)).at(j);
                }
            }
            for (unsigned int j = 0; j <= numLayersInIdList.at(olsIdxToLsIdx.at(i)) - 1; j++)
            {
                if (layerSetLayerIdList.at(olsIdxToLsIdx.at(i)).at(j) == maxValInlayerSetLayerIdList)
                {
                    outputLayerFlag.at(j) = 1;
                }
            }
            vpsExt.mOutputLayerFlagDerived.at(i) = outputLayerFlag;
        }
    }
    for (unsigned int i = (vpsExt.mDefaultOutputLayerIdc == 2) ? 0 : (vps.mNumLayerSetsMinus1) + 1;
        i < numOutputLayerSets - 1; i++)
    {
        std::vector<unsigned int> outputLayerFlag(numLayersInIdList.at(olsIdxToLsIdx.at(i)));
        for (unsigned int j = 0; j <= (numLayersInIdList.at(olsIdxToLsIdx.at(i)) - 1); j++)
        {
            outputLayerFlag.at(j) = vpsExt.mOutputLayerFlag.at(i).at(j);
        }
        vpsExt.mOutputLayerFlagDerived.at(i) = outputLayerFlag;
    }

    // Derivation of numOutputLayersInOutputLayerSet
    std::vector<unsigned int> numOutputLayersInOutputLayerSet(16);
    std::vector<unsigned int> olsHighestOutputLayerId(2048);
    for (unsigned int i = 1; i < numOutputLayerSets; i++)
    {
        for (unsigned int j = 0; j < numLayersInIdList.at(olsIdxToLsIdx.at(i)); j++)
        {
            numOutputLayersInOutputLayerSet.at(i) += vpsExt.mOutputLayerFlagDerived.at(i).at(j);
            if (vpsExt.mOutputLayerFlagDerived.at(i).at(j))
            {
                olsHighestOutputLayerId.at(i) = layerSetLayerIdList.at(olsIdxToLsIdx.at(i)).at(j);
            }
        }
    }

    // Derivation of numNecessaryLayers and necessaryLayerFlag
    // Note: Find exactly how much memory needs to be allocated.
    std::vector<std::vector<unsigned int>> necessaryLayerFlag(2048, std::vector<unsigned int>(2048));
    std::vector<unsigned int> numNecessaryLayers(1024);
    for (unsigned int olsIdx = 0; olsIdx < numOutputLayerSets; olsIdx++)
    {
        unsigned int lsIdx = olsIdxToLsIdx.at(olsIdx);

        for (unsigned int lsLayerIdx = 0; lsLayerIdx < numLayersInIdList.at(lsIdx); lsLayerIdx++)
        {
            necessaryLayerFlag.at(olsIdx).at(lsLayerIdx) = 0;
        }
        for (unsigned int lsLayerIdx = 0; lsLayerIdx < numLayersInIdList.at(lsIdx); lsLayerIdx++)
        {
            if (vpsExt.mOutputLayerFlagDerived.at(olsIdx).at(lsLayerIdx))
            {
                necessaryLayerFlag.at(olsIdx).at(lsLayerIdx) = 1;
                unsigned int currLayerId = layerSetLayerIdList.at(lsIdx).at(lsLayerIdx);

                for (unsigned int rLsLayerIdx = 0; rLsLayerIdx < lsLayerIdx; rLsLayerIdx++)
                {
                    unsigned int refLayerId = layerSetLayerIdList.at(lsIdx).at(rLsLayerIdx);
                    if (dependencyFlag.at(vpsExt.mLayerIdxInVps.at(currLayerId)).at(
                        vpsExt.mLayerIdxInVps.at(refLayerId)))
                    {
                        necessaryLayerFlag.at(olsIdx).at(rLsLayerIdx) = 1;
                    }
                }
            }

            numNecessaryLayers.at(olsIdx) = 0;
            for (unsigned int lsLayerIdx = 0; lsLayerIdx < numLayersInIdList.at(lsIdx); lsLayerIdx++)
            {
                numNecessaryLayers.at(olsIdx) += necessaryLayerFlag.at(olsIdx).at(lsLayerIdx);
            }
        }
    }

    vpsExt.mProfileTierLevelIdx.resize(numOutputLayerSets);
    vpsExt.mAltOutputLayerFlag.resize(numOutputLayerSets);
    for (unsigned int i = 1; i < numOutputLayerSets; i++)
    {
        vpsExt.mProfileTierLevelIdx.at(i).resize(8);
        for (unsigned int j = 0; j < numLayersInIdList[olsIdxToLsIdx[i]]; j++)
        {
            if (necessaryLayerFlag.at(i).at(j) && vpsExt.mVpsNumProfileTierLevelMinus1 > 0)
            {
                unsigned int numBits = ceilLog2(vpsExt.mVpsNumProfileTierLevelMinus1 + 1);
                vpsExt.mProfileTierLevelIdx.at(i).at(j) = bitstr.readBits(numBits);
            }
        }
        if (numOutputLayersInOutputLayerSet[i] == 1 && numDirectRefLayers[olsHighestOutputLayerId[i]] > 0)
        {
            vpsExt.mAltOutputLayerFlag.at(i) = bitstr.readBits(1);
        }
    }
    if (vps.mMaxLayersMinus1 > 0)
    {
        vpsExt.mProfileTierLevelIdx.at(0).push_back(1);
    }
    else
    {
        vpsExt.mProfileTierLevelIdx.at(0).push_back(0);
    }

    vpsExt.mVpsNumRepFormatsMinus1 = bitstr.readExpGolombCode();

    for (unsigned int i = 0; i <= vpsExt.mVpsNumRepFormatsMinus1; i++)
    {
        vpsExt.repFormat.resize(vpsExt.repFormat.size() + 1);
        // Get representation formats
        parseRep(bitstr, vpsExt.repFormat.back());
    }
    if (vpsExt.mVpsNumRepFormatsMinus1 > 0)
    {
        vpsExt.mRepFormatIdxPresentFlag = bitstr.readBits(1);
    }
    else
    {
        vpsExt.mRepFormatIdxPresentFlag = 0;
    }

    vpsExt.mVpsRepFormatIdx.resize(maxLayersMinus1 + 1);
    if (vpsExt.mRepFormatIdxPresentFlag)
    {
        for (unsigned int i = (vps.mBaseLayerInternalFlag ? 1 : 0); i <= maxLayersMinus1; i++)
        {
            vpsExt.mVpsRepFormatIdx.at(i) = bitstr.readBits(ceilLog2(vpsExt.mVpsNumRepFormatsMinus1 + 1));
        }
    }
    else
    {
        for(unsigned int i = (vps.mBaseLayerInternalFlag ? 1 : 0); i <= maxLayersMinus1; i++)
        {
            vpsExt.mVpsRepFormatIdx.at(i) = std::min(i, vpsExt.mVpsNumRepFormatsMinus1);
        }
    }

    vpsExt.mMaxOneActiveRefLayerFlag = bitstr.readBits(1);

    vpsExt.mVpsPocLsbAlignedFlag = bitstr.readBits(1);

    vpsExt.mPocLsbNotPresentFlag.resize(maxLayersMinus1 + 1);
    for (unsigned int i = 1; i <= maxLayersMinus1; i++)
    {
        if (numDirectRefLayers.at(vpsExt.mLayerIdInNuh.at(i)) == 0)
        {
            vpsExt.mPocLsbNotPresentFlag.at(i) = bitstr.readBits(1);
        }
    }

    // Note: Note not verified the following code.
    // Implement the following function
    parseDpb();
    vpsExt.mDirectDepTypeLenMinus2 = bitstr.readExpGolombCode();
    vpsExt.mDirectDependencyAllLayersFlag = bitstr.readBits(1);
    if (vpsExt.mDirectDependencyAllLayersFlag)
    {
        vpsExt.mDirectDependencyAllLayersType = bitstr.readExpGolombCode();
    }
    else
    {
        for (unsigned int i = (vps.mBaseLayerInternalFlag ? 1 : 2); i <= maxLayersMinus1; i++)
        {
            for (unsigned int j = (vps.mBaseLayerInternalFlag ? 0 : 1); j < i; j++)
            {
                std::vector<unsigned int> directDependencyType;
                // Note: derive directDependencyFlag
                // Note: a matrix of the final size may be required to be initialized.
                if (vpsExt.mDirectDependencyFlag.at(i).at(j))
                {
                    directDependencyType.push_back(bitstr.readExpGolombCode());
                }
                vpsExt.mDirectDependencyType.push_back(directDependencyType);
            }
        }
    }
    vpsExt.mVpsNonVuiExtensionLength = bitstr.readExpGolombCode();
    for (unsigned int i = 1; i <= vpsExt.mVpsNonVuiExtensionLength; i++)
    {
        vpsExt.mVpsNonVuiExtensionDataByte = bitstr.readBits(8);
    }
    vpsExt.mVpsVuiPresentFlag = bitstr.readBits(1);
    if (vpsExt.mVpsVuiPresentFlag)
    {
        while (!bitstr.isByteAligned())
        {
            vpsExt.mVpsVuiAlignmentBitEqualToOne = bitstr.readBits(1);
        }

        // Note: Implement the following
        parseVui();
    }
}

void OperatingPointsInformation::makeOinf(const std::vector<std::uint8_t>& srcVps)
{
    std::vector<uint8_t> vps;

    H265Parser::convertByteStreamToRBSP(srcVps, vps);
    BitStream bitstr(vps);

    Vps vpsInfo;
    parseVps(bitstr, vpsInfo);

    // scalability_mask
    setScalabilityMask(vpsInfo.mExtension.mScalabilityMaskFlag);

    // First write the plt in the VPS
    ProfileTierLevel ptl;
    ptl.mGeneralProfileSpace = vpsInfo.mProfileTierLevel.mGeneralProfileSpace;
    ptl.mGeneralTierFlag = vpsInfo.mProfileTierLevel.mGeneralTierFlag;
    ptl.mGeneralProfileIdc = vpsInfo.mProfileTierLevel.mGeneralProfileIdc;
    ptl.mGeneralProfileCompatibilityFlags = 0;
    for (unsigned int i = 0; i < 32; i++)
    {
        ptl.mGeneralProfileCompatibilityFlags = (ptl.mGeneralProfileCompatibilityFlags << 1) |
            ((vpsInfo.mProfileTierLevel.mGeneralProfileCompatibilityFlag[i] == 1) ? 1 : 0);
    }
    ptl.mGeneralConstraintIndicatorFlags = vpsInfo.mProfileTierLevel.mGeneralConstraintIndicatorFlags;
    ptl.mGeneralLevelIdc = vpsInfo.mProfileTierLevel.mGeneralProfileIdc;

    addProfileTierLevel(ptl);

    // Next write the plt in the VPS extension
    for (unsigned int i = 1; i < vpsInfo.mExtension.mVpsNumProfileTierLevelMinus1; i++)
    {
        ProfileTierLevel ptl;
        ptl.mGeneralProfileSpace = vpsInfo.mExtension.mProfileTierLevelArray.at(i - 1).mGeneralProfileSpace;
        ptl.mGeneralTierFlag = vpsInfo.mExtension.mProfileTierLevelArray.at(i - 1).mGeneralTierFlag;
        ptl.mGeneralProfileIdc = vpsInfo.mExtension.mProfileTierLevelArray.at(i - 1).mGeneralProfileIdc;
        ptl.mGeneralProfileCompatibilityFlags = 0;
        for (unsigned int j = 0; j < 32; j++)
        {
            ptl.mGeneralProfileCompatibilityFlags = (ptl.mGeneralProfileCompatibilityFlags << 1) |
                ((vpsInfo.mExtension.mProfileTierLevelArray.at(i - 1).mGeneralProfileCompatibilityFlag[j] == 1) ? 1 : 0);
        }
        ptl.mGeneralConstraintIndicatorFlags =
            vpsInfo.mExtension.mProfileTierLevelArray.at(i - 1).mGeneralConstraintIndicatorFlags;
        ptl.mGeneralLevelIdc = vpsInfo.mExtension.mProfileTierLevelArray.at(i - 1).mGeneralProfileIdc;
        addProfileTierLevel(ptl);
    }

    unsigned int numLayerSets = 0;
    unsigned int numOutputLayerSets = 0;
    std::vector<unsigned int> outputLayerSetIndex;
    numLayerSets = vpsInfo.mNumLayerSetsMinus1 + 1 + vpsInfo.mExtension.mNumAddLayerSets;
    numOutputLayerSets = numLayerSets + vpsInfo.mExtension.mNumAddLayerSets;
    for (unsigned int i = 0; i < numOutputLayerSets; i++)
    {
        outputLayerSetIndex.push_back((i < numLayerSets) ? i : (vpsInfo.mExtension.mLayerSetIdxForOlsMinus1.at(i) + 1));
    }

    // A vector to collect all uniqueLayers of any operating point
    std::vector<unsigned int> uniqueLayerId;

    // An implicit assumption is made here that each output layer set is a valid output operation point.
    for (unsigned int i = 0; i < outputLayerSetIndex.size(); i++)
    {
        OperatingPoint operatingPoint;
        operatingPoint.mOutputLayerSetIdx = outputLayerSetIndex.at(i);

        for (unsigned int j = 0; j < vpsInfo.mExtension.mNumLayersInIdList.at(vpsInfo.mExtension.mOlsIdxToLsIdx.at(i));
            j++)
        {
            OperatingPointLayer operatingPointLayer;
            operatingPointLayer.mPtlIdx = vpsInfo.mExtension.mProfileTierLevelIdx.at(i).at(j);
            operatingPointLayer.mLayerId = vpsInfo.mExtension.mLayerSetLayerIdList.at(i).at(j);
            operatingPointLayer.mIsOutputlayer = vpsInfo.mExtension.mOutputLayerFlagDerived.at(i).at(j) ? true : false;
            // Note: The derivation of mIsAlternateOutputlayer is currently a very bad hack.
            operatingPointLayer.mIsAlternateOutputlayer = vpsInfo.mExtension.mAltOutputLayerFlag.at(i) ? true : false;
            operatingPoint.mOperatingPointLayers.push_back(operatingPointLayer);
        }

        unsigned int minPicHeight = std::numeric_limits<unsigned int>::max();
        unsigned int maxPicHeight = std::numeric_limits<unsigned int>::min();
        unsigned int minPicWidth = std::numeric_limits<unsigned int>::max();
        unsigned int maxPicWidth = std::numeric_limits<unsigned int>::min();
        unsigned int maxChromaFormat = std::numeric_limits<unsigned int>::min();
        unsigned int maxBitDepthMinus8 = std::numeric_limits<unsigned int>::min();
        for (unsigned int j = 0; j < vpsInfo.mExtension.mNumLayersInIdList.at(vpsInfo.mExtension.mOlsIdxToLsIdx.at(i)); j++)
        {
            unsigned int layerId = vpsInfo.mExtension.mLayerSetLayerIdList.at(i).at(j);

            // If this layerId is not in the vector of uniqueLayerId then add it.
            if (!(std::find(uniqueLayerId.begin(), uniqueLayerId.end(), layerId) != uniqueLayerId.end()))
            {
                uniqueLayerId.push_back(layerId);
            }

            const unsigned int index = vpsInfo.mExtension.mVpsRepFormatIdx.at(layerId);
            const Rep& rep = vpsInfo.mExtension.repFormat.at(index);
            minPicWidth = std::min(minPicWidth, rep.mPicWidthVpsInLumaSamples);
            maxPicWidth = std::max(maxPicWidth, rep.mPicWidthVpsInLumaSamples);
            minPicHeight = std::min(minPicHeight, rep.mPicHeightVpsInLumaSamples);
            maxPicHeight = std::max(maxPicHeight, rep.mPicHeightVpsInLumaSamples);
            maxChromaFormat = std::max(maxChromaFormat, rep.mChromaFormatVpsIdc);
            maxBitDepthMinus8 = std::max(maxBitDepthMinus8, rep.mBitDepthVpsLumaMinus8);
            maxBitDepthMinus8 = std::max(maxBitDepthMinus8, rep.mBitDepthVpsChromaMinus8);
        }

        operatingPoint.mMinPicWidth = minPicWidth;
        operatingPoint.mMaxPicWidth = maxPicWidth;
        operatingPoint.mMinPicHeight = minPicHeight;
        operatingPoint.mMaxPicHeight = maxPicHeight;

        operatingPoint.mMaxChromaFormat = maxChromaFormat;
        operatingPoint.mMaxBitDepthMinus8 = maxBitDepthMinus8;

        // Append the information collected for this operating point
        addOperatingPoint(operatingPoint);
    }

    for (unsigned int i = 0; i < uniqueLayerId.size(); i++)
    {
        if (uniqueLayerId.at(i) != 0)
        {
            Layer layer;
            layer.mDependentLayerId = uniqueLayerId.at(i);
            for (unsigned int j = 0; j < vpsInfo.mExtension.mNumDirectRefLayers.at(i); j++)
            {
                layer.mDependentOnLayerIds.push_back(vpsInfo.mExtension.mIdDirectRefLayer.at(i).at(j));
            }

            for (unsigned int j = 0; j < vpsInfo.mExtension.mDimensionId.at(i).size(); j++)
            {
                layer.mDimensionIdentifiers.push_back(vpsInfo.mExtension.mDimensionId.at(i).at(j));
            }
            mLayers.push_back(layer);
        }
    }
}

void OperatingPointsInformation::addProfileTierLevel(const ProfileTierLevel &plt)
{
    mProfileTierLevels.push_back(plt);
}

void OperatingPointsInformation::setScalabilityMask(const std::vector<unsigned int> scalabilityMask)
{
    mScalabilityMask = 0;
    for (unsigned int i = 0; i < 16; i++)
    {
        mScalabilityMask = (mScalabilityMask << 1) | (scalabilityMask.at(i) ? 1 : 0);
    }
}

void OperatingPointsInformation::addOperatingPoint(const OperatingPoint& operatingPoint)
{
    mOperatingPoints.push_back(operatingPoint);
}

void OperatingPointsInformation::writeBox(BitStream& output)
{
    writeFullBoxHeader(output);
    output.write16Bits(mScalabilityMask);
    output.writeBits(0, 2); // reserved 2 bits

    output.writeBits(mProfileTierLevels.size(), 6);
    for (const auto& profileTierLevel : mProfileTierLevels)
    {
        output.writeBits(profileTierLevel.mGeneralProfileSpace, 2);
        output.writeBits(profileTierLevel.mGeneralTierFlag, 1);
        output.writeBits(profileTierLevel.mGeneralProfileIdc, 5);
        output.write32Bits(profileTierLevel.mGeneralProfileCompatibilityFlags);
        for (int i = 0; i < 6; i++)
        {
            output.writeBits(profileTierLevel.mGeneralConstraintIndicatorFlags.at(i), 8);
        }
        output.write8Bits(profileTierLevel.mGeneralLevelIdc);
    }

    output.write16Bits(mOperatingPoints.size());
    for (const auto& operatingPoint : mOperatingPoints)
    {
        output.write16Bits(operatingPoint.mOutputLayerSetIdx);
        output.write8Bits(0); // reserved 8 bits
        output.write8Bits(operatingPoint.mOperatingPointLayers.size());
        for (const auto& operatingPointLayer : operatingPoint.mOperatingPointLayers)
        {
            output.write8Bits(operatingPointLayer.mPtlIdx);
            output.writeBits(operatingPointLayer.mLayerId, 6);
            output.writeBits(operatingPointLayer.mIsOutputlayer, 1);
            output.writeBits(operatingPointLayer.mIsAlternateOutputlayer, 1);
        }
        output.write16Bits(operatingPoint.mMinPicWidth);
        output.write16Bits(operatingPoint.mMinPicHeight);
        output.write16Bits(operatingPoint.mMaxPicWidth);
        output.write16Bits(operatingPoint.mMaxPicHeight);
        output.writeBits(operatingPoint.mMaxChromaFormat, 2);
        output.writeBits(operatingPoint.mMaxBitDepthMinus8, 3);
        output.writeBits(0, 3); // reserved 3 bits
    }

    output.write8Bits(mLayers.size());
    for (const auto& layer : mLayers)
    {
        output.write8Bits(layer.mDependentLayerId);
        output.write8Bits(layer.mDependentOnLayerIds.size());
        for (const auto dependentOnLayerId : layer.mDependentOnLayerIds)
        {
            output.write8Bits(dependentOnLayerId);
        }

        for (const auto dimensionIdentifiers : layer.mDimensionIdentifiers)
        {
            output.write8Bits(dimensionIdentifiers);
        }
    }

    updateSize(output);
}

void OperatingPointsInformation::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);
    mScalabilityMask = input.read16Bits();
    input.readBits(2); // reserved 2 bits

    const unsigned int profileTierLevelCount = input.readBits(6);
    for (unsigned int i = 0; i < profileTierLevelCount; ++i)
    {
        ProfileTierLevel profileTierLevel;
        profileTierLevel.mGeneralProfileSpace = input.readBits(2);
        profileTierLevel.mGeneralTierFlag = input.readBits(1);
        profileTierLevel.mGeneralProfileIdc = input.readBits(5);
        profileTierLevel.mGeneralProfileCompatibilityFlags = input.read32Bits();
        for (unsigned int j = 0; j < 6; ++j)
        {
            profileTierLevel.mGeneralConstraintIndicatorFlags.push_back(input.readBits(8));
        }

        profileTierLevel.mGeneralLevelIdc = input.read8Bits();
        mProfileTierLevels.push_back(profileTierLevel);
    }

    const unsigned int operatingPointsCount = input.read16Bits();
    for (unsigned int i = 0; i < operatingPointsCount; ++i)
    {
        OperatingPoint operatingPoint;
        operatingPoint.mOutputLayerSetIdx = input.read16Bits();
        input.read8Bits(); // reserved 8 bits
        const unsigned int layerCount = input.read8Bits();
        for (unsigned int j = 0; j < layerCount; ++j)
        {
            OperatingPointLayer operatingPointLayer;
            operatingPointLayer.mPtlIdx = input.read8Bits();
            operatingPointLayer.mLayerId = input.readBits(6);
            operatingPointLayer.mIsOutputlayer = input.readBits(1);
            operatingPointLayer.mIsAlternateOutputlayer = input.readBits(1);
            operatingPoint.mOperatingPointLayers.push_back(operatingPointLayer);
        }

        operatingPoint.mMinPicWidth = input.read16Bits();
        operatingPoint.mMinPicHeight = input.read16Bits();
        operatingPoint.mMaxPicWidth = input.read16Bits();
        operatingPoint.mMaxPicHeight = input.read16Bits();
        operatingPoint.mMaxChromaFormat = input.readBits(2);
        operatingPoint.mMaxBitDepthMinus8 = input.readBits(3);
        input.readBits(3); // reserved 3 bits

        mOperatingPoints.push_back(operatingPoint);
    }

    const unsigned int layerCount = input.read8Bits();
    for (unsigned int i = 0; i < layerCount; ++i)
    {
        Layer layer;
        layer.mDependentLayerId = input.read8Bits();
        const unsigned int dependentLayerIdCount = input.read8Bits();
        for (unsigned int j = 0; j < dependentLayerIdCount; ++j)
        {
            layer.mDependentOnLayerIds.push_back(input.read8Bits());
        }
        for (unsigned int j = 0; j < 16; ++j)
        {
            if (mScalabilityMask & (1 << j))
            {
                layer.mDimensionIdentifiers.push_back(input.read8Bits());
            }
        }
        mLayers.push_back(layer);
    }
}

std::vector<uint8_t> OperatingPointsInformation::getLayerIds(const std::uint16_t outputLayerSetIndex) const
{
    for (size_t i = 0; i < mOperatingPoints.size(); ++i)
    {
        if (mOperatingPoints.at(i).mOutputLayerSetIdx == outputLayerSetIndex)
        {
            std::vector<uint8_t> layerIds;
            for (const auto layer : mOperatingPoints.at(i).mOperatingPointLayers)
            {
                layerIds.push_back(layer.mLayerId);
            }
            return layerIds;
        }
    }

    throw std::runtime_error("OperatingPointsInformation::getLayerIds() requested output layer set index not found.");
}
