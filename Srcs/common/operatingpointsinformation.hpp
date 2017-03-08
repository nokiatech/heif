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

#ifndef OPERATINGPOINTSINFORMATION_HPP
#define OPERATINGPOINTSINFORMATION_HPP

#include "fullbox.hpp"
#include <cstdint>
#include <utility>
#include <vector>

class BitStream;

/**
 * The class OperatingPointsInformation corresponds to the operating points information sample group
 * specified in ISO/IEC 14496-15. */
class OperatingPointsInformation : public FullBox
{
public:
    OperatingPointsInformation();
    virtual ~OperatingPointsInformation() = default;

    /** Fill member variables from VPS and SPS.
     * @param [in] vps The VPS to extract oinf data from. */
    void makeOinf(const std::vector<std::uint8_t>& vps);

    /**
     * @brief Get layer IDs required by an operating point.
     * @param outputLayerSetIndex The index of the output layer set that defines this operating point.
     * @return Requested layer IDs.
     * @throws Run-time Exception if the requested output layer set index was not found.
     */
    std::vector<uint8_t> getLayerIds(std::uint16_t outputLayerSetIndex) const;

    /**
     * Get maximum extents of an output layer set.
     * @param outputLayerSetIndex The index of the output layer set that defines the operating point.
     * @return Maximum width and height (maxPicWidth, maxPicHeight) of the requested output layer set.
     * @throws Run-time Exception if the requested output layer set index was not found.
     */
    std::pair<uint32_t, uint32_t> getMaxExtents(std::uint16_t outputLayerSetIndex) const;

    struct ProfileTierLevel
    {
        std::uint8_t mGeneralProfileSpace;              ///< 2 bits
        std::uint8_t mGeneralTierFlag;
        std::uint8_t mGeneralProfileIdc;                ///< 5 bits
        std::uint32_t mGeneralProfileCompatibilityFlags;
        std::vector<std::uint8_t> mGeneralConstraintIndicatorFlags;
        std::uint8_t mGeneralLevelIdc;
    };

    struct OperatingPointLayer
    {
        std::uint8_t mPtlIdx;
        std::uint8_t mLayerId;        ///< 6 bits
        bool mIsOutputlayer;          ///< 1 bit
        bool mIsAlternateOutputlayer; ///< 1 bit
    };

    struct OperatingPoint
    {
        std::uint16_t mOutputLayerSetIdx;
        std::vector<OperatingPointLayer> mOperatingPointLayers;
        std::uint16_t mMinPicWidth;
        std::uint16_t mMinPicHeight;
        std::uint16_t mMaxPicWidth;
        std::uint16_t mMaxPicHeight;
        std::uint8_t mMaxChromaFormat;   ///< 2 bits
        std::uint8_t mMaxBitDepthMinus8; ///< 3 bits
    };

    struct Layer
    {
        std::uint8_t mDependentLayerId;
        std::vector<std::uint8_t> mDependentOnLayerIds;
        std::vector<std::uint8_t> mDimensionIdentifiers;
    };

    void setScalabilityMask(const std::vector<unsigned int> scalabilityMask);
    void addProfileTierLevel(const ProfileTierLevel& profileTierLevel);
    void addOperatingPoint(const OperatingPoint& operatingPoint);

    /** Write property to BitStream
     *  @see Box::writeBox() */
    virtual void writeBox(BitStream& output);

    /** Parse property from BitStream
     *  @see Box::parseBox() */
    virtual void parseBox(BitStream& input);

private:
    std::uint16_t mScalabilityMask;
    std::vector<ProfileTierLevel> mProfileTierLevels;
    std::vector<OperatingPoint> mOperatingPoints;
    std::vector<Layer> mLayers;

    struct SubHrd
    {
        std::vector<unsigned int> mBitRateValueMinus1;
        std::vector<unsigned int> mCpbSizeValueMinus1;
        std::vector<unsigned int> mCpbSizeDuValueMinus1;
        std::vector<unsigned int> mBitRateDuValueMinus1;
        std::vector<unsigned int> mCbrFlag;
    };

    struct Hrd
    {
        unsigned int mNalHrdParameterPresentFlag;
        unsigned int mVclHrdParametersPresentFlag;
        unsigned int mSubPicHrdParamsPresentFlag;
        unsigned int mTickDivisorMinus2;
        unsigned int mDuCpbRemovalDelayIncrementLengthMinus1;
        unsigned int mSubPicCpbParamsInPicTimingSeiFlag;
        unsigned int mDpbOutputDelayDuLengthMinus1;
        unsigned int mBitRateScale;
        unsigned int mCpbSizeScale;
        unsigned int mCpbSizeDuScale;
        unsigned int mInitialCpbRemovalDelayLengthMinus1;
        unsigned int mAuCpbRemovalDelayLengthMinus1;
        unsigned int mDpbOutputDelayLengthMinus1;
        std::vector<unsigned int> mFixedPicRateGeneralFlag;
        std::vector<unsigned int> mFixedPicRateWithinCvsFlag;
        std::vector<unsigned int> mElementalDurationInTcMinus1;
        std::vector<unsigned int> mLowDelayHrdFlag;
        std::vector<unsigned int> mCpbCntMinus1;
        std::vector<SubHrd> mSubLayerNalHrdParams;
        std::vector<SubHrd> mSubLayerVclHrdParams;
    };

    struct Ptl
    {
        unsigned int mGeneralProfileSpace;
        unsigned int mGeneralTierFlag;
        unsigned int mGeneralProfileIdc;
        std::vector<unsigned int> mGeneralProfileCompatibilityFlag;
        std::vector<std::uint8_t> mGeneralConstraintIndicatorFlags;
        unsigned int mGeneralLevelIdc;
        std::vector<unsigned int> mSubLayerProfilePresentFlag;
        std::vector<unsigned int> mSubLayerLevelPresentFlag;
    };

    struct Rep
    {
        unsigned int mPicWidthVpsInLumaSamples;
        unsigned int mPicHeightVpsInLumaSamples;
        unsigned int mChromaAndBitDepthVpsPresentFlag;
        unsigned int mChromaFormatVpsIdc;
        unsigned int mSeparateColourPlaneVpsFlag;
        unsigned int mBitDepthVpsLumaMinus8;
        unsigned int mBitDepthVpsChromaMinus8;
        unsigned int mConformanceWindowVpsFlag;
        unsigned int mConfWinVpsLeftOffset;
        unsigned int mConfWinVpsRightOffset;
        unsigned int mConfWinVpsTopOffset;
        unsigned int mConfWinVpsBottomOffset;
    };

    struct VpsExt
    {
        Ptl mProfileTierLevel;
        unsigned int mSplittingFlag;
        unsigned int mNumScalabilityTypes;
        std::vector<unsigned int> mScalabilityMaskFlag;
        std::vector<unsigned int> mDimensionIdLenMinus1;
        unsigned int mVpsNuhLayerIdPresentFlag;
        std::vector<unsigned int> mLayerIdInNuh;
        std::vector<std::vector<unsigned int>> mDimensionId;
        unsigned int mViewIdLen;
        std::vector<unsigned int> mViewIdVal;
        std::vector<std::vector<unsigned int>> mDirectDependencyFlag;
        unsigned int mNumAddLayerSets;
        std::vector<std::vector<unsigned int>> mHighestLayerIdxPlus1;
        unsigned int mVpsSubLayersMaxMinus1PresentFlag;
        std::vector<unsigned int> mSubLayersVpsMaxMinus1;
        unsigned int mMaxTidRefPresentFlag;
        std::vector<std::vector<unsigned int>> mMaxTidIlRefPicsPlus1;
        unsigned int mDefaultRefLayersActiveFlag;
        unsigned int mVpsNumProfileTierLevelMinus1;
        std::vector<unsigned int> mVpsProfilePresentFlag;
        std::vector<Ptl> mProfileTierLevelArray;
        unsigned int mNumAddOlss;
        unsigned int mDefaultOutputLayerIdc;
        std::vector<unsigned int> mLayerSetIdxForOlsMinus1;
        std::vector<std::vector<unsigned int>> mOutputLayerFlag;
        std::vector<std::vector<unsigned int>> mProfileTierLevelIdx;
        std::vector<unsigned int> mAltOutputLayerFlag;
        unsigned int mVpsNumRepFormatsMinus1;
        std::vector<Rep> repFormat;
        unsigned int mRepFormatIdxPresentFlag;
        std::vector<unsigned int> mVpsRepFormatIdx;
        unsigned int mMaxOneActiveRefLayerFlag;
        unsigned int mVpsPocLsbAlignedFlag;
        std::vector<unsigned int> mPocLsbNotPresentFlag;
        unsigned int mDirectDepTypeLenMinus2;
        unsigned int mDirectDependencyAllLayersFlag;
        unsigned int mDirectDependencyAllLayersType;
        std::vector<std::vector<unsigned int>> mDirectDependencyType;
        unsigned int mVpsNonVuiExtensionLength;
        unsigned int mVpsNonVuiExtensionDataByte;
        unsigned int mVpsVuiPresentFlag;
        unsigned int mVpsVuiAlignmentBitEqualToOne;
        std::vector<unsigned int> mLayerIdxInVps;
        std::vector<std::vector<unsigned int>> mIdDirectRefLayer;
        std::vector<std::vector<unsigned int>> mIdRefLayer;
        std::vector<std::vector<unsigned int>> mIdPredictedLayer;
        std::vector<unsigned int> mNumDirectRefLayers;
        std::vector<unsigned int> mNumRefLayer;
        std::vector<unsigned int> mNumPredictedLayer;
        std::vector<unsigned int> mNumLayersInIdList;
        std::vector<std::vector<unsigned int>> mLayerSetLayerIdList;
        std::vector<unsigned int> mOlsIdxToLsIdx;

        std::vector<std::vector<unsigned int>> mOutputLayerFlagDerived;
    };

    struct Vps
    {
        unsigned int mVpsId;
        unsigned int mBaseLayerInternalFlag;
        unsigned int mBaseLayerAvailableFlag;
        unsigned int mMaxLayersMinus1;
        unsigned int mMaxSubLayersMinus1;
        unsigned int mTemporalIdNestingFlag;
        unsigned int mReserved0xffff16Bits;
        Ptl mProfileTierLevel;
        unsigned int mSubLayerOrderingInfoPresentFlag;
        std::vector<unsigned int> mMaxDecPicBufferingMinus1;
        std::vector<unsigned int> mMaxNumReorderPics;
        std::vector<unsigned int> mMaxLatencyIncreasePlus1;
        unsigned int mMaxLayerId;
        unsigned int mNumLayerSetsMinus1;
        std::vector<std::vector<unsigned int>> mLayerIdIncludedFlag;
        unsigned int mTimingInfoPresentFlag;
        unsigned int mNumUnitsInTick;
        unsigned int mTimeScale;
        unsigned int mPocProportionalToTimingFlag;
        unsigned int mNumTicksPocDiffOneMinus1;
        unsigned int mNumHrdParameter;
        std::vector<unsigned int> mHrdLayerSetIdx;
        std::vector<unsigned int> mCprmsPresentFlag;
        std::vector<Hrd> mHrdParameters;
        unsigned int mExtensionFlag;
        std::vector<unsigned int> mExtensionAlignmentBitEqualToOne;
        VpsExt mExtension;
        unsigned int mExtension2Flag;
        std::vector<unsigned int> mExtensionDataFlag;
    };

    unsigned int ceilLog2(unsigned int x);

    void parseDpb();
    void parseVui();
    void parseRep(BitStream& bitstr, Rep& rep);
    void parseHrd(BitStream& bitstr, Hrd& hrd, unsigned int commonInfPresentFlag, unsigned int maxNumSubLayersMinus1);
    void parseSubHrd(BitStream& bitstr, SubHrd& hrd, int cpbCnt, unsigned int subPicHrdParamsPresentFlag);
    void parsePtl(BitStream& bitstr, Ptl& ptl, unsigned int maxNumSubLayersMinus1, unsigned int profilePresentFlag = 1);
    void parseVps(BitStream& bitstr, Vps& vps);
    void parseVpsExt(BitStream& bitstr, Vps& vps, VpsExt& vpsExt);
};

#endif
