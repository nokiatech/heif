/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#ifndef H265_PARSER_HPP
#define H265_PARSER_HPP

#include "bitstream.hpp"
#include "parserinterface.hpp"

#include <array>
#include <cstdint>
#include <fstream>
#include <list>
#include <string>
#include <vector>

#define EXTENDED_SAR 255

/** @brief H265 bitstream parser.
 *  @details Note: This is not a full-featured H.265 bitstream parser.
 *  @details It contains sufficient features in order to parse H.265 bitstreams compliant with HEIF standard.
 */
class H265Parser : public ParserInterface
{
public:
    H265Parser();
    virtual ~H265Parser();
    virtual bool openFile(const char* fileName);
    virtual bool openFile(const std::string& fileName);
    virtual bool parseNextAU(AccessUnit& accessUnit);
    virtual unsigned int getNumPictures();

    static void convertByteStreamToRBSP(const std::vector<uint8_t>& byteStr, std::vector<uint8_t>& dest);

private:
    enum class H265NalUnitType : std::uint8_t
    {
        CODED_SLICE_TRAIL_N = 0, // 0
        CODED_SLICE_TRAIL_R,     // 1

        CODED_SLICE_TSA_N,       // 2
        CODED_SLICE_TSA_R,       // 3

        CODED_SLICE_STSA_N,      // 4
        CODED_SLICE_STSA_R,      // 5

        CODED_SLICE_RADL_N,      // 6
        CODED_SLICE_RADL_R,      // 7

        CODED_SLICE_RASL_N,      // 8
        CODED_SLICE_RASL_R,      // 9

        RESERVED_VCL_N10,
        RESERVED_VCL_R11,
        RESERVED_VCL_N12,
        RESERVED_VCL_R13,
        RESERVED_VCL_N14,
        RESERVED_VCL_R15,

        CODED_SLICE_BLA_W_LP,    // 16
        CODED_SLICE_BLA_W_RADL,  // 17
        CODED_SLICE_BLA_N_LP,    // 18
        CODED_SLICE_IDR_W_RADL,  // 19
        CODED_SLICE_IDR_N_LP,    // 20
        CODED_SLICE_CRA,         // 21
        RESERVED_IRAP_VCL22,
        RESERVED_IRAP_VCL23,

        RESERVED_VCL24,
        RESERVED_VCL25,
        RESERVED_VCL26,
        RESERVED_VCL27,
        RESERVED_VCL28,
        RESERVED_VCL29,
        RESERVED_VCL30,
        RESERVED_VCL31,
        VPS,                     // 32
        SPS,                     // 33
        PPS,                     // 34
        ACCESS_UNIT_DELIMITER,   // 35
        EOS,                     // 36
        EOB,                     // 37
        FILLER_DATA,             // 38
        PREFIX_SEI,              // 39
        SUFFIX_SEI,              // 40
        RESERVED_NVCL41,
        RESERVED_NVCL42,
        RESERVED_NVCL43,
        RESERVED_NVCL44,
        RESERVED_NVCL45,
        RESERVED_NVCL46,
        RESERVED_NVCL47,
        UNSPECIFIED_48,
        UNSPECIFIED_49,
        UNSPECIFIED_50,
        UNSPECIFIED_51,
        UNSPECIFIED_52,
        UNSPECIFIED_53,
        UNSPECIFIED_54,
        UNSPECIFIED_55,
        UNSPECIFIED_56,
        UNSPECIFIED_57,
        UNSPECIFIED_58,
        UNSPECIFIED_59,
        UNSPECIFIED_60,
        UNSPECIFIED_61,
        UNSPECIFIED_62,
        UNSPECIFIED_63,
        INVALID
    };

    enum class SliceType
    {
        B = 0,
        P,
        I
    };

    struct NalUnitHeader
    {
        H265NalUnitType mH265NalUnitType;
        unsigned int mNuhLayerId;
        unsigned int mNuhTemporalIdPlus1;
    };

    struct Picture
    {
        H265NalUnitType mH265NalUnitType;
        int mIndex;               // decode order
        int mPoc;                 // display order
        unsigned int mWidth;
        unsigned int mHeight;
        unsigned int mSlicePicOrderCntLsb;
        bool mIsReferecePic;
        bool mIsLongTermRefPic;
        bool mPicOutputFlag;
    };

    struct SubLayerProfileTierLevel
    {
        unsigned int mSubLayerProfileSpace;
        unsigned int mSubLayerTierFlag;
        unsigned int mSubLayerProfileIdc;
        unsigned int mSubLayerProfileCompatibilityFlag[32];
        unsigned int mSubLayerProgressiveSourceFlag;
        unsigned int mSubLayerInterlacedSourceFlag;
        unsigned int mSubLayerNonPackedConstraintFlag;
        unsigned int mSubLayerFrameOnlyConstraintFlag;
        unsigned int mSubLayerLevelIdc;
    };

    struct ProfileTierLevel
    {
        unsigned int mGeneralProfileSpace;
        unsigned int mGeneralTierFlag;
        unsigned int mGeneralProfileIdc;
        unsigned int mGeneralProfileCompatibilityFlag[32];
        unsigned int mGeneralProgressiveSourceFlag;
        unsigned int mGeneralInterlacedSourceFlag;
        unsigned int mGeneralNonPackedConstraintFlag;
        unsigned int mGeneralFrameOnlyConstraintFlag;
        unsigned int mGeneralLevelIdc;
        std::vector<unsigned int> mSubLayerProfilePresentFlag;
        std::vector<unsigned int> mSubLayerLevelPresentFlag;
        std::vector<SubLayerProfileTierLevel> mSubLayerProfileTierLevels;
    };

    struct ShortTermRefPicSet
    {
        unsigned int mInterRefPicSetPredictionFlag;
        unsigned int mDeltaIdxMinus1;
        unsigned int mDeltaRpsSign;
        unsigned int mAbsDeltaRpsMinus1;
        std::vector<unsigned int> mUsedByCurrPicFlag;
        std::vector<unsigned int> mUseDeltaFlag;
        unsigned int mNumNegativePics;
        unsigned int mNumPositivePics;
        std::vector<unsigned int> mDeltaPocS0Minus1;
        std::vector<unsigned int> mUsedByCurrPicS0Flag;
        std::vector<unsigned int> mDeltaPocS1Minus1;
        std::vector<unsigned int> mUsedByCurrPicS1Flag;
    };

    struct ShortTermRefPicSetDerived
    {
        unsigned int mNumNegativePics;
        unsigned int mNumPositivePics;
        std::vector<unsigned int> mUsedByCurrPicS0;
        std::vector<unsigned int> mUsedByCurrPicS1;
        std::vector<int> mDeltaPocS0;
        std::vector<int> mDeltaPocS1;
        unsigned int mNumDeltaPocs;
    };

    struct RefPicSet
    {
        std::vector<int> mPocStCurrBefore;
        std::vector<int> mPocStCurrAfter;
        std::vector<int> mPocStFoll;
        std::vector<int> mPocLtCurr;
        std::vector<int> mPocLtFoll;
        std::vector<unsigned int> mCurrDeltaPocMsbPresentFlag;
        std::vector<unsigned int> mFollDeltaPocMsbPresentFlag;
        unsigned int mNumPocStCurrBefore;
        unsigned int mNumPocStCurrAfter;
        unsigned int mNumPocStFoll;
        unsigned int mNumPocLtCurr;
        unsigned int mNumPocLtFoll;
        std::vector<Picture*> mRefPicSetStCurrBefore;
        std::vector<Picture*> mRefPicSetStCurrAfter;
        std::vector<Picture*> mRefPicSetStFoll;
        std::vector<Picture*> mRefPicSetLtCurr;
        std::vector<Picture*> mRefPicSetLtFoll;
    };

    struct RefPicListsModification
    {
        unsigned int mRefPicListModificationFlagL0;
        std::vector<unsigned int> mListEntryL0;
        unsigned int mRefPicListModificationFlagL1;
        std::vector<unsigned int> mListEntryL1;
    };

    struct ScalingListData
    {
        std::vector<unsigned int> mScalingListPredModeFlag[4];
        std::vector<unsigned int> mScalingListPredMatrixIdDelta[4];
        std::vector<int> mScalingListDcCoefMinus8[2];
        std::vector<std::vector<unsigned int>> mScalingList[4];
    };

    struct PredWeightTable
    {
        unsigned int mLumaLog2WeightDenom;
        int mDeltaChromaLog2WeightDenom;
        std::vector<unsigned int> mLumaWeightL0Flag;
        std::vector<unsigned int> mChromaWeightL0Flag;
        std::vector<int> mDeltaLumaWeightL0;
        std::vector<int> mLumaOffsetL0;
        std::vector<std::array<int, 2>> mDeltaChromaWeightL0;
        std::vector<std::array<int, 2>> mDeltaChromaOffsetL0;
        std::vector<unsigned int> mLumaWeightL1Flag;
        std::vector<unsigned int> mChromaWeightL1Flag;
        std::vector<int> mDeltaLumaWeightL1;
        std::vector<int> mLumaOffsetL1;
        std::vector<std::array<int, 2>> mDeltaChromaWeightL1;
        std::vector<std::array<int, 2>> mDeltaChromaOffsetL1;
    };

    struct SubLayerHrdParameters
    {
        std::vector<unsigned int> mBitRateValueMinus1;
        std::vector<unsigned int> mCpbSizeValueMinus1;
        std::vector<unsigned int> mCpbSizeDuValueMinus1;
        std::vector<unsigned int> mBitRateDuValueMinus1;
        std::vector<unsigned int> mCbrFlag;
    };

    struct HrdParameters
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
        std::vector<SubLayerHrdParameters> mSubLayerNalHrdParams;
        std::vector<SubLayerHrdParameters> mSubLayerVclHrdParams;
    };

    struct VuiParameters
    {
        unsigned int mAspectRatioInfoPresentFlag;
        unsigned int mAspectRatioIdc;
        unsigned int mSarWidth;
        unsigned int mSarHeight;
        unsigned int mOverscanInfoPresentFlag;
        unsigned int mOverscanAppropriateFlag;
        unsigned int mVideoSignalTypePresentFlag;
        unsigned int mVideoFormat;
        unsigned int mVideoFullRangeFlag;
        unsigned int mColourDescriptionPresentFlag;
        unsigned int mCcolourPrimaries;
        unsigned int mTransferCharacteristics;
        unsigned int mMatrixCoeffs;
        unsigned int mChromaLocInfoPresentFlag;
        unsigned int mChromaSampleLocTypeTopField;
        unsigned int mChromaSampleLocTypeBottomField;
        unsigned int mNeutralChromaIndicationFlag;
        unsigned int mFieldSeqFlag;
        // Check if the ptl initialization bug is fixed
        unsigned int mFrameFieldInfoPresentFlag;
        unsigned int mDefaultDisplayWindowFlag;
        unsigned int mDefDispWinLeftOffset;
        unsigned int mDefDispWinRightOffset;
        unsigned int mDefDispWinTopOffset;
        unsigned int mDefDispWinBottomOffset;
        unsigned int mVuiTimingInfoPresentFlag;
        unsigned int mVuiNumUnitsInTick;
        unsigned int mVuiTimeScale;
        unsigned int mVuiPocProportionalToTimingFlag;
        unsigned int mVuiNumTicksPocDiffOneMinus1;
        unsigned int mVuiHrdParametersPresentFlag;
        HrdParameters mHrdParameters;
        unsigned int mBitstreamRestrictionFlag;
        unsigned int mTilesFixedStructureFlag;
        unsigned int mMotionVectorsOverPicBoundariesFlag;
        unsigned int mRestrictedRefPicListsFlag;
        unsigned int mMinSpatialSegmentationIdc;
        unsigned int mMaxBytesPerPicDenom;
        unsigned int mMaxBitsPerMinCuDenom;
        unsigned int mLog2MaxMvLengthHorizontal;
        unsigned int mLog2MaxMvLengthVertical;
    };

    struct RepFormat
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

    struct VpsExtension
    {
        ProfileTierLevel mProfileTierLevel;
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
        unsigned int  mNumAddLayerSets;

        std::vector<std::vector<unsigned int>> mHighestLayerIdxPlus1;
        unsigned int mVpsSubLayersMaxMinus1PresentFlag;

        std::vector<unsigned int> mSubLayersVpsMaxMinus1;
        unsigned int mMaxTidRefPresentFlag;

        std::vector<std::vector<unsigned int>> mMaxTidIlRefPicsPlus1;

        unsigned int mDefaultRefLayersActiveFlag;
        unsigned int mVpsNumProfileTierLevelMinus1;
        std::vector<unsigned int> mVpsProfilePresentFlag;
        std::vector<ProfileTierLevel> mProfileTierLevelArray;
        unsigned int mNumAddOlss;
        unsigned int mDefaultOutputLayerIdc;

        std::vector<unsigned int> mLayerSetIdxForOlsMinus1;
        std::vector<std::vector<unsigned int>> mOutputLayerFlag;

        std::vector<std::vector<unsigned int>> mProfileTierLevelIdx;
        std::vector<unsigned int> mAltOutputLayerFlag;

        unsigned int mVpsNumRepFormatsMinus1;
        std::vector<RepFormat> repFormat;
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
    };

    struct VideoParameterSet
    {
        unsigned int mVpsId;
        unsigned int mBaseLayerInternalFlag;
        unsigned int mBaseLayerAvailableFlag;
        unsigned int mMaxLayersMinus1;
        unsigned int mMaxSubLayersMinus1;
        unsigned int mTemporalIdNestingFlag;
        unsigned int mReserved0xffff16Bits;

        ProfileTierLevel mProfileTierLevel;

        unsigned int mSubLayerOrderingInfoPresentFlag;

        std::vector<unsigned int> mMaxDecPicBufferingMinus1;
        std::vector<unsigned int> mMaxNumReorderPics;
        std::vector<unsigned int> mMaxLatencyIncreasePlus1;

        unsigned int mMaxLayerId;
        unsigned int mNumLayerSetsMinus1;
        std::vector<std::vector<unsigned int>> mLayerIdIncludedFlag;

        unsigned int mTimingInfoPresentFlag;

        // If mVpsTimingInfoPresentFlag
        unsigned int mNumUnitsInTick;
        unsigned int mTimeScale;
        unsigned int mPocProportionalToTimingFlag;

        // If mVpsPocPropotionalToTimingFlag
        unsigned int mNumTicksPocDiffOneMinus1;

        unsigned int mNumHrdParameter;
        std::vector<unsigned int> mHrdLayerSetIdx;
        std::vector<unsigned int> mCprmsPresentFlag;
        std::vector<HrdParameters> mHrdParameters;

        unsigned int mExtensionFlag;
        std::vector<unsigned int> mExtensionAlignmentBitEqualToOne;
        VpsExtension mExtension;

        unsigned int mExtension2Flag;
        std::vector<unsigned int> mExtensionDataFlag;
    };

    struct SequenceParameterSet
    {
        unsigned int mVpsId;
        unsigned int mSpsMaxSubLayersMinus1;
        unsigned int mSpsExtOrMaxSubLayersMinus1;
        unsigned int mSpsTemporalIdNestingFlag;
        ProfileTierLevel mProfileTierLevel;
        unsigned int mSpsId;
        unsigned int mUpdateRepFormatFlag;
        unsigned int mSpsRepFormatIdx;
        unsigned int mChromaFormatIdc;
        unsigned int mSeparateColourPlaneFlag;
        unsigned int mPicWidthInLumaSamples;
        unsigned int mPicHeightInLumaSamples;
        unsigned int mConformanceWindowFlag;
        unsigned int mConfWinLeftOffset;
        unsigned int mConfWinRightOffset;
        unsigned int mConfWinTopOffset;
        unsigned int mConfWinBottomOffset;
        unsigned int mBitDepthLumaMinus8;
        unsigned int mBitDepthChromaMinus8;
        unsigned int mLog2MaxPicOrderCntLsbMinus4;
        unsigned int mSpsSubLayerOrderingInfoPresentFlag;
        std::vector<unsigned int> mSpsMaxDecPicBufferingMinus1;
        std::vector<unsigned int> mSpsMaxNumReorderPics;
        std::vector<unsigned int> mSpsMaxLatencyIncreasePlus1;
        unsigned int mLog2MinLumaCodingBlockSizeMinus3;
        unsigned int mLog2DiffMaxMinLumaCodingBlockSize;
        unsigned int mLog2MinTransformBlockSizeMinus2;
        unsigned int mLog2DiffMaxMinTransformBlockSize;
        unsigned int mMaxTransformHierarchyDepthInter;
        unsigned int mMaxTransformHierarchyDepthIntra;
        unsigned int mScalingListEnabledFlag;
        unsigned int mSpsInferScalingListFlag;
        unsigned int mSpsScalingListRefLayerId;
        unsigned int mSpsScalingListDataPresentFlag;
        ScalingListData mScalingListData;
        unsigned int mAmpEnabledFlag;
        unsigned int mSampleAdaptiveOffsetEnabledFlag;
        unsigned int mPcmEnabledFlag;
        unsigned int mPcmSampleBitDepthLumaMinus1;
        unsigned int mPcmSampleBitDepthChromaMinus1;
        unsigned int mLog2MinPcmLumaCodingBlockSizeMinus3;
        unsigned int mLog2DiffMaxMinPcmLumaCodingBlockSize;
        unsigned int mPcmLoopFilterDisabledFlag;
        unsigned int mNumShortTermRefPicSets;
        std::vector<ShortTermRefPicSet> mShortTermRefPicSets;
        std::vector<ShortTermRefPicSetDerived> mShortTermRefPicSetsDerived;
        unsigned int mLongTermRefPicsPresentFlag;
        unsigned int mNumLongTermRefPicsSps;
        std::vector<unsigned int> mLtRefPicPocLsbSps;
        std::vector<unsigned int> mUsedByCurrPicLtSpsFlag;
        unsigned int mSpsTemporalMvpEnabledFlag;
        unsigned int mStrongIntraSmoothingEnabledFlag;
        unsigned int mVuiParametersPresentFlag;
        VuiParameters mVuiParameters;
        unsigned int mSpsExtensionFlag;
    };

    struct PictureParameterSet
    {
        unsigned int mPpsId;
        unsigned int mSpsId;
        unsigned int mDependentSliceSegmentsEnabledFlag;
        unsigned int mOutputFlagPresentFlag;
        unsigned int mNumExtraSliceHeaderBits;
        unsigned int mSignDataHidingEnabledFlag;
        unsigned int mCabacInitPresentFlag;
        unsigned int mNumRefIdxL0DefaultActiveMinus1;
        unsigned int mNumRefIdxL1DefaultActiveMinus1;
        int mInitQpMinus26;
        unsigned int mConstrainedIntraPredFlag;
        unsigned int mTransformSkipEnabledFlag;
        unsigned int mCuQpDeltaEnabledFlag;
        unsigned int mDiffCuQpDeltaDepth;
        int mPpsCbQpOffset;
        int mPpsCrQpOffset;
        unsigned int mPpsSliceChromaQpOffsetsPresentFlag;
        unsigned int mWeightedPredFlag;
        unsigned int mWeightedBipredFlag;
        unsigned int mTransquantBypassEnabledFlag;
        unsigned int mTilesEnabledFlag;
        unsigned int mEntropyCodingSyncEnabledFlag;

        unsigned int mNumTileColumnsMinus1;
        unsigned int mNumTileRowsMinus1;
        unsigned int mUniformSpacingFlag;
        std::vector<unsigned int> mColumnWidthMinus1;
        std::vector<unsigned int> mRowHeightMinus1;
        unsigned int mLoopFilterAcrossTilesEnabledFlag;

        unsigned int mPpsLoopFilterAcrossSicesEnabledFlag;
        unsigned int mDeblockingFilterControlPresentFlag;
        unsigned int mDeblockingFilterOverrideEnabledFlag;
        unsigned int mPpsDeblockingFilterDisabledFlag;
        int mPpsBetaOffsetDiv2;
        int mPpsTcOffsetDiv2;

        unsigned int mPpsScalingListDataPresentFlag;
        ScalingListData mScalingListData;

        unsigned int mListsModificationPresentFlag;
        unsigned int mLog2ParallelMergeLevelMinus2;
        unsigned int mSliceSegmentHeaderExtensionPresentFlag;
        unsigned int mPpsExtensionFlag;
        unsigned int mPpsExtensionDataFlag;
    };

    struct SliceHeader
    {
        H265NalUnitType mNaluType;
        unsigned int mFirstSliceSegmentInPicFlag;
        unsigned int mNoOutputOfPriorPicsFlag;
        unsigned int mPpsId;
        unsigned int mDependentSliceSegmentFlag;
        unsigned int mSliceSegmentAddress;
        unsigned int mDiscardableFlag;
        unsigned int mCrossLayerBlaFlag;  // For LHEVC
        SliceType mSliceType;
        unsigned int mPicOutputFlag;
        unsigned int mColourPlaneId;
        unsigned int mSlicePicOrderCntLsb;
        unsigned int mShortTermRefPicSetSpsFlag;
        ShortTermRefPicSet mShortTermRefPicSet;
        ShortTermRefPicSetDerived mShortTermRefPicSetDerived;
        unsigned int mShortTermRefPicSetIdx;
        ShortTermRefPicSetDerived* mCurrStRps;
        unsigned int mNumPocTotalCurr;

        unsigned int mNumLongTermSps;
        unsigned int mNumLongTermPics;

        std::vector<unsigned int> mLtIdxSps;
        std::vector<unsigned int> mPocLsbLtSyntax;
        std::vector<unsigned int> mUsedByCurrPicLtFlag;
        std::vector<unsigned int> mDeltaPocMsbPresentFlag;
        std::vector<unsigned int> mDeltaPocMsbCycleLt;

        unsigned int mNumActiveRefLayerPics;
        unsigned int mSliceTemporalMvpEnabledFlag;
        unsigned int mInterLayerPredEnabledFlag;
        unsigned int mNumInterLayerRefPicsMinus1;
        std::vector<unsigned int> mInterLayerPredLayerIdc;

        unsigned int mSliceSaoLumaFlag;
        unsigned int mSliceSaoChromaFlag;
        unsigned int mNumRefIdxActiveOverrideFlag;
        unsigned int mNumRefIdxL0ActiveMinus1;
        unsigned int mNumRefIdxL1ActiveMinus1;
        RefPicListsModification mRefPicListsModification;
        unsigned int mMvdL1ZeroFlag;
        unsigned int mCabacInitFlag;
        unsigned int mCollocatedFromL0Flag;
        unsigned int mCollocatedRefIdx;
        PredWeightTable mPredWeightTable;
        unsigned int mFiveMinusMaxNumMergeCand;
        int mSliceQpDelta;
        int mSliceCbQpOffset;
        int mSliceCrQpOffset;
        unsigned int mDeblockingFilterOverrideFlag;
        unsigned int mSliceDeblockingFilterDisabledFlag;
        int mSliceBetaOffsetDiv2;
        int mSliceTcOffsetDiv2;
        unsigned int mSliceLoopFilterAcrossSlicesEnabledFlag;

        unsigned int mNumEntryPointOffsets;
        unsigned int mOffsetLenMinus1;
        std::vector<unsigned int> mEntryPointOffsetMinus1;

        std::vector<unsigned int> mPocLsbLt;
        std::vector<unsigned int> mUsedByCurrPicLt;

        SequenceParameterSet* mSps;
        PictureParameterSet*  mPps;
        VideoParameterSet* mVps;
    };

    std::ifstream mInFile;
    std::vector<uint8_t> mCurrNalUnitData;
    std::vector<Picture*> mRefPicList0;
    std::vector<Picture*> mRefPicList1;
    std::list<PictureParameterSet*> mPpsList;
    std::list<SequenceParameterSet*> mSpsList;
    std::list<VideoParameterSet*> mVpsList;
    std::list<Picture> mDecodedPicBuffer;
    int mPicIndex;
    unsigned int mPrevPicOrderCntLsb;
    int mPrevPicOrderCntMsb;
    std::vector<SliceHeader*> mSliceList;
    std::vector<unsigned int> mDisplayNumArray;

    bool hasVpsExtension();
    bool doOpenFile(const char* fileName);
    bool initDisplayOrderArray(const char* fileName);
    int parseNalUnitHeader(BitStream& bitstr, NalUnitHeader& naluHeader);
    int parseNalUnit(const std::vector<uint8_t>& nalUnit, NalUnitHeader& naluHeader);
    void printPicStats(const Picture& pic);
    int decodePoc(const SliceHeader& slice, NalUnitHeader& naluHeader);
    int decodeRefPicSet(SliceHeader& slice, RefPicSet& rps, int poc);
    int deltaPocMsbCycleLt(SliceHeader& slice, int i);
    Picture* findPicInDpbPocLsb(unsigned int pocLsb);
    Picture* findPicInDpbPoc(int poc);
    int generateRefPicLists(SliceHeader& slice, RefPicSet& rps);
    int generateRefPicListsOld(SliceHeader& slice, RefPicSet& rps);
    SequenceParameterSet* findSps(unsigned int spsId);
    PictureParameterSet* findPps(unsigned int ppsId);
    VideoParameterSet* findVps(unsigned int vpsId);
    void removeVps(unsigned int vpsId);
    void removeSps(unsigned int spsId);
    void removePps(unsigned int ppsId);
    bool isVclNaluType(H265NalUnitType naluType);
    void getRefPicIndices(std::vector<unsigned int>& refPicIndices, const std::vector<Picture*>& mRefPicList0, const std::vector<Picture*>& mRefPicList1);
    bool isUniquePicIndex(const std::vector<unsigned int>& refPicIndices, unsigned int picIndex);

    unsigned int ceilLog2(unsigned int x);
    int parseProfileTierLevel(BitStream& bitstr, ProfileTierLevel& ptl, const unsigned int maxNumSubLayersMinus1, const unsigned int profilePresentFlag = 1);
    int parseScalingListData(BitStream& bitstr, ScalingListData& scalingList);
    int parseShortTermRefPicSet(BitStream& bitstr, const std::vector<ShortTermRefPicSetDerived>& rpsList, ShortTermRefPicSet& rps, unsigned int stRpsIdx, unsigned int numShortTermRefPicSets);
    int deriveRefPicSetParams(const std::vector<ShortTermRefPicSetDerived>& rpsList, const ShortTermRefPicSet& rps, ShortTermRefPicSetDerived& rpsDerived, unsigned int stRpsIdx);
    int parseRefPicListsModification(BitStream& bitstr, const SliceHeader& slice, RefPicListsModification& refPicListsMod);
    int parsePredWeightTable(BitStream& bitstr, SequenceParameterSet& sps, SliceHeader& slice, PredWeightTable& pwTable);
    void setVuiDefaults(ProfileTierLevel& ptl, VuiParameters& vui);
    int parseVuiParameters(BitStream& bitstr, VuiParameters& vui);
    int parseHrdParameters(BitStream& bitstr, HrdParameters& hrd, unsigned int commonInfPresentFlag, unsigned int maxNumSubLayersMinus1);
    int parseSubLayerHrd(BitStream& bitstr, SubLayerHrdParameters& hrd, int cpbCnt, unsigned int subPicHrdParamsPresentFlag);
    int parseVPS(BitStream& bitstr, VideoParameterSet& vps);
    // int parseSPS(BitStream& bitstr, SequenceParameterSet& sps);
    int parseSPS(BitStream& bitstr, SequenceParameterSet& sps, const NalUnitHeader& naluHeader);
    int parsePPS(BitStream& bitstr, PictureParameterSet& pps);
    RepFormat parseRepFormat(BitStream& bitstr) const;
    int dpbSize();
    int vpsVui();
    int parseVpsExtension(BitStream& bitstr, VideoParameterSet& vps, VpsExtension& vpsExt);
    // int parseSliceHeader(BitStream& bitstr, SliceHeader& slice, H265NalUnitType naluType);
    // int parseSliceHeader(BitStream& bitstr, SliceHeader& slice, H265NalUnitType naluType, const unsigned int nuh_layer_id);
    int parseSliceHeader(BitStream& bitstr, SliceHeader& slice, NalUnitHeader nalUnitHeader);
    int parseLhvcSliceHeader(BitStream& bitstr, SliceHeader& slice, NalUnitHeader nalUnitHeader);

    H265NalUnitType readNextNalUnit(std::vector<uint8_t>& nalUnit);
    bool checkAccessUnitBoundary(const std::vector<uint8_t>& nalUnit, bool isFirstNaluInAU, bool firstVclNaluFound);
    H265NalUnitType getH265NalUnitType(const std::vector<uint8_t>& nalUnit);
    bool isFirstVclNaluInPic(const std::vector<uint8_t>& nalUnit);
    unsigned int getLayerId(const std::vector<uint8_t>& nalUnit);
};

#endif
