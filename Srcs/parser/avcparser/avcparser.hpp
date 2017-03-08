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

#ifndef AVC_PARSER_HPP
#define AVC_PARSER_HPP

#include "bitstream.hpp"
#include "parserinterface.hpp"

#include <array>
#include <cstdint>
#include <fstream>
#include <list>
#include <string>
#include <vector>

#define EXTENDED_SAR 255

#define PS_MAX_NUM_REF_FRAMES_IN_PIC_ORDER_CNT_CYCLE    256
#define PS_MAX_NUM_SLICE_GROUPS              8
#define MAX_NUM_OF_MMCO_OPS      35
#define MAX_NUM_OF_SCALING_LISTS 12
#define DPB_MAX_SIZE 16
#define MAX_NUM_OF_REORDER_CMDS  17

// AVC Profiles and Levels
#define PS_BASELINE_PROFILE_IDC                     66
#define PS_MAIN_PROFILE_IDC                         77
#define PS_EXTENDED_PROFILE_IDC                     88
#define PS_HIGH_PROFILE_IDC                         100
#define PS_HIGH_10_PROFILE_IDC                      110
#define PS_HIGH_422_PROFILE_IDC                     122
#define PS_HIGH_444_PREDICTIVE_PROFILE_IDC          244
#define PS_CAVLC_444_PROFILE_IDC                    44
#define PS_SCALABLE_BASELINE_PROFILE_IDC            83
#define PS_SCALABLE_HIGH_PROFILE_IDC                86
#define PS_STEREO_HIGH_PROFILE_IDC                  128
#define PS_MULTIVIEW_HIGH_PROFILE_IDC               118
#define PS_MULTIVIEW_DEPTH_HIGH_PROFILE_IDC         138
/// @todo Define ids 139 and 134.

#define FRM_NON_REF_PIC     0
#define FRM_SHORT_TERM_PIC  1
#define FRM_LONG_TERM_PIC   2

/** @brief AVC bitstream parser.
 *  @details Note: This is not a full-featured AVC bitstream parser.
 *  It contains sufficient features in order to parse H.264 bitstreams compliant with HEIF standard.
 */
class AvcParser : public ParserInterface
{
public:
    AvcParser();
    virtual ~AvcParser();
    virtual bool openFile(const char* fileName);
    virtual bool openFile(const std::string& fileName);
    virtual bool parseNextAU(AccessUnit& accessUnit);
    virtual unsigned int getNumPictures();

    static void convertByteStreamToRBSP(const std::vector<uint8_t>& byteStr, std::vector<uint8_t>& dest);

    /// AVC SPS config values used in HEIF.
    struct SPSConfigValues
    {
        std::uint8_t mChromaFormat;
        std::uint8_t mBitDepthLumaMinus8;
        std::uint8_t mBitDepthChromaMinus8;

        std::uint16_t mPicWidthPx;  ///< Calculated picture width in pixels
        std::uint16_t mPicHeightPx; ///< Calculated picture height in pixels
    };

    /** @brief Parse SPS data into config values structure
     *  @param configValues SPS config values
     *  @return true when successful */
    static bool parseSPSConfigValues(const std::vector<uint8_t>& byteStr,
                                     SPSConfigValues& configValues);

private:
    enum class AVCNalUnitType : std::uint8_t
    {
        UNSPECIFIED_0 = 0,
        CODED_SLICE_NON_IDR,        //1
        CODED_SLICE_DPAR_A,
        CODED_SLICE_DPAR_B,
        CODED_SLICE_DPAR_C,
        CODED_SLICE_IDR,            //5
        SEI,                        //6
        SPS,                        //7
        PPS,                        //8
        ACCESS_UNIT_DELIMITER,
        EOS,                        //10
        EOB,                        //11
        FILLER_DATA,
        SPS_EXT,
        PREFIX_NALU,
        SUB_SPS,
        DPS,
        RESERVED_17,
        RESERVED_18,
        SLICE_AUX_NOPAR,
        SLICE_EXT,
        SLICE_EXT_3D,
        RESERVED_22,
        RESERVED_23,
        UNSPECIFIED_24,
        UNSPECIFIED_25,
        UNSPECIFIED_26,
        UNSPECIFIED_27,
        UNSPECIFIED_28,
        UNSPECIFIED_29,
        UNSPECIFIED_30,
        UNSPECIFIED_31,
        INVALID
    };

    enum class SliceType
    {
        P = 0, // 0, 5
        B,     // 1,6
        I,     // 2,7
        SP,    // 3,8
        SI     // 4,9
    };

    struct NalUnitHeader
    {
        unsigned int mNuhRefIdc;
        AVCNalUnitType mAVCNalUnitType;
    };

    struct Picture
    {
        AVCNalUnitType mAVCNalUnitType;
        int mIndex;               ///< Decode order
        int mPoc;                 ///< Display order
        unsigned int mWidth;
        unsigned int mHeight;
        bool mIsReferencePic;
        bool mIsLongTermRefPic;
        bool mPicOutputFlag;
        bool mHasMMCO5;  ///< Copy from the slice header

        int mPicID;
        bool mIsIDR;

        int mFrameNum;
        int mMaxFrameNum;
        int mPicNum;
        int mLongTermFrmIdx;
        int mLongTermPicNum;
        int mRefType;        ///< non-ref, short term or long term.
        int mForOutput;      ///< If this frame is waiting for output.
        int mNonExisting;
        int mPocType;
        int mIdrPicID;
        SliceType mPicType;
        unsigned int mConstraintSet0Flag; ///< From SPS
        unsigned int mConstraintSet1Flag; ///< From SPS
        unsigned int mConstraintSet2Flag; ///< From SPS
        unsigned int mProfileIdc;         ///< From SPS
        unsigned int mLevelIdc;           ///< From SPS
    };

    struct RefPicListsModification
    {
        unsigned int mModificationOfPicNumsIdc;
        unsigned int mAbsDiffPicNumMinus1;
        unsigned int mLongTermPicNum;
    };

    struct ScalingListData
    {
        unsigned int   mUseDefaultScalingMatrix4x4Flag[6];
        int mScalingList4x4[6][16]; // at most 6
        unsigned int mUseDefaultScalingMatrix8x8Flag[6];
        int  mScalingList8x8[6][64]; // at most 6
    };

    struct SliceMMCO
    {
      unsigned int mMemoryManagementControlOperation;
      unsigned int mDifferenceOfPicNumsMinus1;
      unsigned int mLongTermPicNum;
      unsigned int mLongTermFrameIdx;
      unsigned int mMaxLongTermFrameIdxPlus1;
    };

    struct PredWeightTable
    {
        unsigned int mWeightedPredFlag;  ///< Copied from PPS
        unsigned int mWeightedBipredIdc; ///< Copied from PPS
        unsigned int mLumaLog2WeightDenom;
        unsigned int mChromaLog2WeightDenom;
        std::vector<unsigned int> mLumaWeightL0Flag;
        std::vector<unsigned int> mChromaWeightL0Flag;
        std::vector<int> mLumaWeightL0;
        std::vector<int> mLumaOffsetL0;
        std::vector<std::array<int, 2>> mChromaWeightL0;
        std::vector<std::array<int, 2>> mChromaOffsetL0;
        std::vector<unsigned int> mLumaWeightL1Flag;
        std::vector<unsigned int> mChromaWeightL1Flag;
        std::vector<int> mLumaWeightL1;
        std::vector<int> mLumaOffsetL1;
        std::vector<std::array<int, 2>> mChromaWeightL1;
        std::vector<std::array<int, 2>> mChromaOffsetL1;
    };

    struct HrdParameters
    {
        unsigned int mCpbCntMinus1;
        unsigned int mBitRateScale;
        unsigned int mCpbSizeScale;
        std::vector<unsigned int> mBitRateValueMinus1;
        std::vector<unsigned int> mCpbSizeValueMinus1;
        std::vector<unsigned int> mCbrFlag;
        unsigned int mInitialCpbRemovalDelayLengthMinus1;
        unsigned int mCpbRemovalDelayLengthMinus1;
        unsigned int mDpbOutputDelayLengthMinus1;
        unsigned int mTimeOffsetLength;
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
        unsigned int mColourPrimaries;
        unsigned int mTransferCharacteristics;
        unsigned int mMatrixCoeffs;
        unsigned int mChromaLocInfoPresentFlag;
        unsigned int mChromaSampleLocTypeTopField;
        unsigned int mChromaSampleLocTypeBottomField;
        unsigned int mVuiTimingInfoPresentFlag;
        unsigned int mVuiNumUnitsInTick;
        unsigned int mVuiTimeScale;
        unsigned int mVuiFixedFrameRateFlag;
        unsigned int mHrdParametersPresentFlag;
        HrdParameters mHrdParameters;
        unsigned int mVclParametersPresentFlag;
        HrdParameters mVclParameters;
        unsigned int mLowDelayHrdFlag;
        unsigned int mPicStructPresentFlag;
        unsigned int mBitstreamRestrictionFlag;
        unsigned int mMotionVectorsOverPicBoundariesFlag;
        unsigned int mMaxBytesPerPicDenom;
        unsigned int mMaxBitsPerMbDenom;
        unsigned int mLog2MaxMvLengthHorizontal;
        unsigned int mLog2MaxMvLengthVertical;
        unsigned int mMaxNumReorderFrames;
        unsigned int mMaxDecFrameBuffering;
    };

    struct SequenceParameterSet
    {
        unsigned int mProfileIdc;
        unsigned int mConstraintSet0Flag;
        unsigned int mConstraintSet1Flag;
        unsigned int mConstraintSet2Flag;
        unsigned int mConstraintSet3Flag;
        unsigned int mConstraintSet4Flag;
        unsigned int mConstraintSet5Flag;
        unsigned int mReservedZero2Bits;
        unsigned int mLevelIdc;
        unsigned int mSpsId;
        unsigned int mChromaFormatIdc;
        unsigned int mSeparateColourPlaneFlag;
        unsigned int mBitDepthLumaMinus8;
        unsigned int mBitDepthChromaMinus8;
        unsigned int mQpPrimeYZeroTransformBypassFlag;
        unsigned int mScalingMatrixPresentFlag;
        unsigned int mSpsScalingListPresentFlag[MAX_NUM_OF_SCALING_LISTS];
        ScalingListData mScalingListData;
        unsigned int mLog2MaxFrameNumMinus4;
        unsigned int mPicOrderCntType;
        unsigned int mLog2MaxPicOrderCntLsbMinus4;
        unsigned int mDeltaPicOrderAlwaysZeroFlag;
        int mOffsetForNonRefPic;
        int mOffsetForTopToBottomField;
        unsigned int mNumRefFramesInPicOrderCntCycle;
        int mOffsetForRefFrame[PS_MAX_NUM_REF_FRAMES_IN_PIC_ORDER_CNT_CYCLE];
        unsigned int mNumRefFrames;
        unsigned int mGapsInFrameNumValueAllowedFlag;
        unsigned int mPicWidthInMbs_minus1;
        unsigned int mPicHeightInMapUnitsMinus1;
        unsigned int mFrameMbsOnlyFlag;
        unsigned int mMbAdaptiveFrameFieldFlag;
        unsigned int mDirect8x8InferenceFlag;
        unsigned int mFrameCroppingFlag;
        unsigned int mFrameCropLeftOffset;
        unsigned int mFrameCropRightOffset;
        unsigned int mFrameCropTopOffset;
        unsigned int mFrameCropBottomOffset;
        unsigned int mVuiParametersPresentFlag;
        VuiParameters mVuiParameters;
    };

    struct PictureParameterSet
    {
        unsigned int mPpsId;
        unsigned int mSpsId;
        unsigned int mEntropyCodingModeFlag;
        unsigned int mPicOrderPresentFlag;
        unsigned int mNumSliceGroupsMinus1;
        unsigned int mSliceGroupMapType;
        unsigned int mRunLengthMinus1[PS_MAX_NUM_SLICE_GROUPS];
        unsigned int mTopLeft[PS_MAX_NUM_SLICE_GROUPS];
        unsigned int mBottomRight[PS_MAX_NUM_SLICE_GROUPS];
        unsigned int mSliceGroupChangeDirectionFlag;
        unsigned int mSliceGroupChangeRateMinus1;
        unsigned int mPicSizeInMapUnitsMinus1;
        std::vector<unsigned int> mSliceGroupId;
        unsigned int mNumRefIdxL0ActiveMinus1;
        unsigned int mNumRefIdxL1ActiveMinus1;
        unsigned int mWeightedPredFlag;
        unsigned int mWeightedBipredIdc;
        int mPicInitQpMinus26;
        int mPicInitQsMinus26;
        int mChromaQpIndexOffset;
        unsigned int mDeblockingFilterParametersPresentFlag;
        unsigned int mConstrainedIntraPredFlag;
        unsigned int mRedundantPicCntPresentFlag;
        unsigned int mTransform8x8ModeFlag;
        unsigned int mPicScalingMatrixPresentFlag;
        unsigned int mPicScalingListPresentFlag[8];
        unsigned int mUseDefaultScalingMatrix4x4Flag[6];
        int mScalingList4x4[6][16];
        unsigned int   mUseDefaultScalingMatrix8x8Flag[2];
        int mScalingList8x8[2][64];
        int mSecondChromaQpIndexOffset;
    };

    struct SliceHeader
    {
        AVCNalUnitType mNaluType;
        unsigned int mNuhRefIdc; ///< From NAL Unit Header.
        unsigned int mFirstMbInSlice;
        SliceType mSliceType;
        unsigned int mPpsId;
        unsigned int mColorPlaneId;
        unsigned int mFrameNum;
        unsigned int mFieldPicFlag;
        unsigned int mBottomFieldFlag;
        unsigned int mIdrPicId;
        unsigned int mPicOrderCntLsb;
        unsigned int mDeltaPicOrderCntLsb;
        int mDeltaPicOrderCntBottom;
        int mDeltaPicOrderCnt0;
        int mDeltaPicOrderCnt1;
        unsigned int mRedundantPicCnt;
        unsigned int mDirectSpatialMvPredFlag;
        unsigned int mNumRefIdxActiveOverrideFlag;
        unsigned int mNumRefIdxL0ActiveMinus1;
        unsigned int mNumRefIdxL1ActiveMinus1;
        // if( nal_unit_type = = 20 | | nal_unit_type = = 21 )
        // ref_pic_list_mvc_modification( ) specified in Annex H
        unsigned int mRefPicListModificationFlagL0;
        unsigned int mRefPicListModificationFlagL1;
        std::vector<RefPicListsModification> mRefPicListsModificationL0;
        std::vector<RefPicListsModification> mRefPicListsModificationL1;
        PredWeightTable mPredWeightTable;
        unsigned int mNoOutputOfPriorPicsFlag;
        unsigned int mLongTermReferenceFlag;
        unsigned int mAdaptiveRefPicMarkingModeFlag;
        std::vector<SliceMMCO> mMMCOCmdList;
        bool mHasMMCO5;
        unsigned int mCabacInitIdc;
        int mSliceQpDelta;
        unsigned int mSpForSwitchFlag;
        int mSliceQsDelta;
        unsigned int mDisableDeblockingFilterIdc;
        int mSliceAlphaCoOffsetDiv2;
        int mSliceBetaOffsetDiv2;
        unsigned int mSliceGroupChangeCycle;

        SequenceParameterSet* mSps;
        PictureParameterSet*  mPps;

        unsigned int mMaxFrameNum;
        bool mIdrPicFlag;
        unsigned int mSliceId;

        unsigned int mPicOutputFlag;
    };

    /// Decoded Picture Buffer structure
    struct Dpb
    {
      int size;
      std::list<Picture> mDecodedPicBuffer;

      int fullness;
      int maxNumRefFrames;
      int numShortTermPics;
      int numLongTermPics;

      int maxLongTermFrameIdx;
    };

    Dpb mDPB; // Decoded Picture buffer

    Picture* mCurrentPic;
    std::ifstream mInFile;
    std::vector<uint8_t> mCurrNalUnitData;
    std::vector<Picture*> mRefPicList0;
    std::vector<Picture*> mRefPicList1;
    std::list<PictureParameterSet*> mPpsList;
    std::list<SequenceParameterSet*> mSpsList;
    int mPicIndex;
    unsigned int mPrevPicOrderCntLsb;
    int mPrevPicOrderCntMsb;
    std::vector<SliceHeader*> mSliceList;
    std::vector<unsigned int> mDisplayNumArray;

    // Needed for poc calculations.
    int mPicOrderCntLsb;
    int mPicOrderCntMsb;
    int mPreviousFrameNumOffset;
    int mFrameNumOffset;
    int mPreviousFrameNum;
    bool mPrevPicHasMMCO5;
    int mPreviousRefPicPoc;
    int mPreviousRefFrameNum;

    bool mIsCurrPicFinished;

    // Decoded Picture Buffer related parameters.
    int mFullness;
    int mMaxNumRefFrames;
    int mNumShortTermPics;
    int mNumLongTermPics;
    int mMaxLongTermFrameIdx;
    std::list<Picture> mOutputPictureList;
    int mOutputQueuePos;
    int mNumQueuedOutputPics;
    bool mIsDpbStorePending;

    bool doOpenFile(const char* fileName);
    bool initDisplayOrderArray(const char* fileName);
    static int parseNalUnitHeader(BitStream& bitstr, NalUnitHeader& naluHeader);
    int parseNalUnit(const std::vector<uint8_t>& nalUnit, NalUnitHeader& naluHeader);
    int decodePoc(const SliceHeader& slice);

    SequenceParameterSet* findSps(unsigned int spsId);
    PictureParameterSet* findPps(unsigned int ppsId);
    void removeSps(unsigned int spsId);
    void removePps(unsigned int ppsId);
    bool isVclNaluType(AVCNalUnitType naluType);
    void getRefPicIndices(std::vector<unsigned int>& refPicIndices, const std::vector<Picture*>& mRefPicList0, const std::vector<Picture*>& mRefPicList1);
    bool isUniquePicIndex(const std::vector<unsigned int>& refPicIndices, unsigned int picIndex);

    unsigned int ceilLog2(unsigned int x);
    int parseSliceHeader(BitStream& bitstr, SliceHeader& slice, AVCNalUnitType naluType, unsigned int naluhRefIdc);

    static int parseScalingListData(BitStream& bitstr, int* scalingList, unsigned int& useDefaultScalingMatrixFlag, unsigned int sizeOfScalingList );
    static int parseRefPicListsModification(BitStream& bitstr, SliceHeader& slice);
    static int parsePredWeightTable(BitStream& bitstr, SliceHeader& slice);
    static void setVuiDefaults(VuiParameters& vui);
    static int parseVuiParameters(BitStream& bitstr, VuiParameters& vui);
    static int parseHrdParameters(BitStream& bitstr, HrdParameters& hrd);
    static int parseSPS(BitStream& bitstr, SequenceParameterSet& sps);
    static int parsePPS(BitStream& bitstr, PictureParameterSet& pps);
    static int parseDecRefPicMarking(BitStream& bitstr, SliceHeader& slice);

    AVCNalUnitType readNextNalUnit(std::vector<uint8_t>& nalUnit);
    bool checkAccessUnitBoundary(const std::vector<uint8_t>& nalUnit, bool isFirstNaluInAU, bool firstVclNaluFound);
    AVCNalUnitType getAVCNalUnitType(const std::vector<uint8_t>& nalUnit);
    bool isFirstVclNaluInPic(const std::vector<uint8_t>& nalUnit);
    static bool moreRBSPData(BitStream& bitstr);

    // Dpb related operations
    void dpbSetSize(Dpb& dpb, int dpbSize);
    Picture* dpbGetNextOutputPic(Dpb& dpb, int& dpbHasIDRorMMCO5);
    int dpbStorePicture(Dpb& dpb, Picture **currPicPtr);
    void dpbUpdatePicNums(Dpb& dpb, int frameNum, int maxFrameNum);
    void dpbMarkAllPicsAsNonRef(Dpb& dpb);
    void dpbMarkLowestShortTermPicAsNonRef(Dpb& dpb);
    int dpbMarkShortTermPicAsNonRef(Dpb& dpb, int picNum);
    int dpbMarkLongTermPicAsNonRef(Dpb& dpb, int longTermPicNum);
    void dpbVerifyLongTermFrmIdx(Dpb& dpb, int longTermFrmIdx);
    int dpbMarkShortTermPicAsLongTerm(Dpb& dpb, int picNum, int longTermFrmIdx);
    void dpbSetMaxLongTermFrameIdx(Dpb& dpb, int maxLongTermFrmIdxPlus1);

    // Sequence related
    int generateNonExistingFrames();
    int finishCurrentPic();
    int decRefPicMarking();

    int slidingWindowDecRefPicMarking();
    int adaptiveDecRefPicMarking();

    // refPic related
    int sliceInitRefPicListBslice(int pocType, int slicePoc);
    int sliceFixRefPicList(std::vector<Picture*> &RefPicList, int numExistingRefFrames, int numRefFrames, unsigned int width, unsigned int height);
    int refPicListReordering(SliceHeader& slice, std::vector<Picture*> &RefPicList, int numRefPicActive, std::vector<RefPicListsModification> &reorderCmdList);
    int sliceInitRefPicList(std::vector<Picture*> &RefPicList);
    int initializeCurrentPicture(unsigned int width, unsigned int height);
};

#endif
