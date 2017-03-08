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

#include "avcparser.hpp"
#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>

using namespace std;

AvcParser::AvcParser() :
    mCurrentPic(nullptr),
    mPicIndex(0),
    mPrevPicOrderCntLsb(0),
    mPrevPicOrderCntMsb(0),
    mPicOrderCntLsb(0),
    mPicOrderCntMsb(0),
    mPreviousFrameNumOffset(0),
    mFrameNumOffset(0),
    mPreviousFrameNum(0),
    mPrevPicHasMMCO5(false),
    mPreviousRefPicPoc(0),
    mPreviousRefFrameNum(0),
    mIsCurrPicFinished(false),
    mFullness(0),
    mMaxNumRefFrames(0),
    mNumShortTermPics(0),
    mNumLongTermPics(0),
    mMaxLongTermFrameIdx(0),
    mOutputQueuePos(0),
    mNumQueuedOutputPics(0),
    mIsDpbStorePending(false)
{
    mDPB = {};
}

AvcParser::~AvcParser()
{
    mInFile.close();

    for (auto& i : mSpsList)
    {
        delete i;
    }

    for (auto& i : mPpsList)
    {
        delete i;
    }
}

bool AvcParser::openFile(const char* fileName)
{
    if (!initDisplayOrderArray(fileName))
    {
        return false;
    }

    return doOpenFile(fileName);
}

bool AvcParser::openFile(const std::string& fileName)
{
    return openFile(fileName.c_str());
}

bool AvcParser::doOpenFile(const char* fileName)
{
    mInFile.open(fileName, std::ios::in | std::ios::binary);
    return mInFile.good();
}

bool AvcParser::initDisplayOrderArray(const char* fileName)
{
    AvcParser* parser = new AvcParser;
    vector<PicOrder> picOrder;
    int displayNum = 0;

    if (!parser->doOpenFile(fileName))
    {
        return false;
    }

    bool hasMoreFrames = true;

    // Iterate through all access units.
    while (hasMoreFrames)
    {
        AccessUnit* au = new AccessUnit { };
        hasMoreFrames = parser->parseNextAU(*au);

        // Update display order array if last access unit was parsed or IDR picture was found.
        if (!hasMoreFrames || (hasMoreFrames && au->mIsIdr))
        {
            if (picOrder.size() > 0)
            {
                // Sort in display order.
                std::sort(picOrder.begin(), picOrder.end(), compareDisplayOrder);

                // Replace POCs with unique display order numbers.
                for (auto& i : picOrder)
                {
                    i.mDisplayIdx = displayNum;
                    displayNum++;
                }

                // Sort in decoding order.
                std::sort(picOrder.begin(), picOrder.end(), compareDecodeOrder);

                for (auto& i : picOrder)
                {
                    mDisplayNumArray.push_back(i.mDisplayIdx);
                }
                picOrder.clear();
            }
        }

        if (hasMoreFrames)
        {
            // Add current picture to the picture order array.
            PicOrder po;
            po.mDecodeIdx = au->mPicIndex;
            po.mDisplayIdx = au->mPoc;
            picOrder.push_back(po);
        }

        delete au;
    }

    delete parser;
    return true;
}

unsigned int AvcParser::getNumPictures()
{
    return mDisplayNumArray.size();
}

bool AvcParser::parseNextAU(AccessUnit& accessUnit)
{
    bool isFirstNaluInAU;
    bool firstVclNaluFound;
    bool isAUBoundary;
    NalUnitHeader naluHeader;
    Picture pic = {};

    mCurrentPic = nullptr; // Set the current picture to nullptr, it will be assigned later on.

    accessUnit = {};  // Make it empty.

    isFirstNaluInAU = true;
    firstVclNaluFound = false;
    isAUBoundary = false;

    // Parse NAL units until Access Unit boundary is reached.
    while (!isAUBoundary)
    {
        if (mCurrNalUnitData.size() == 0)
        {
            readNextNalUnit(mCurrNalUnitData);
        }

        if (mIsDpbStorePending)
        {
            dpbStorePicture(mDPB, &mCurrentPic);
            mIsDpbStorePending = false;
        }

        isAUBoundary = checkAccessUnitBoundary(mCurrNalUnitData, isFirstNaluInAU, firstVclNaluFound);

        if (!isAUBoundary)
        {
            parseNalUnit(mCurrNalUnitData, naluHeader);

            // Store NAL unit to current AU.
            switch (naluHeader.mAVCNalUnitType)
            {
            case AVCNalUnitType::SPS:
                accessUnit.mSpsNalUnits.push_back(mCurrNalUnitData);
                break;
            case AVCNalUnitType::PPS:
                accessUnit.mPpsNalUnits.push_back(mCurrNalUnitData);
                break;
            default:
                accessUnit.mNalUnits.push_back(mCurrNalUnitData);
                break;
            }

            // Initialize picture if the NAL unit is the first slice of the picture.
            if (isFirstVclNaluInPic(mCurrNalUnitData))
            {
                SliceHeader& sliceHeader = *mSliceList[0];
                firstVclNaluFound = true;

                mRefPicList0.clear();
                mRefPicList1.clear();

                // Set the current picture.
                mCurrentPic = &pic;

                // Init current picure with correct width and height by using SPS data.
                initializeCurrentPicture(
                    (sliceHeader.mSps->mPicWidthInMbs_minus1 + 1) * 16 - sliceHeader.mSps->mFrameCropLeftOffset * 2
                        - sliceHeader.mSps->mFrameCropRightOffset * 2,
                    (2 - sliceHeader.mSps->mFrameMbsOnlyFlag) * (sliceHeader.mSps->mPicHeightInMapUnitsMinus1 + 1) * 16
                        - sliceHeader.mSps->mFrameCropTopOffset * 2 - sliceHeader.mSps->mFrameCropBottomOffset * 2);

                if (sliceHeader.mIdrPicFlag)
                {
                    // Set Decoded Picture Buffer size.
                    dpbSetSize(mDPB, sliceHeader.mSps->mVuiParameters.mMaxDecFrameBuffering);
                    pic.mPicID = 0; /// @todo Check if mPicID is still needed.
                    mDPB.maxNumRefFrames = sliceHeader.mSps->mNumRefFrames;
                }
                /// @todo Check if the following code can be moved to a more suitable initialization location.
                pic.mAVCNalUnitType = naluHeader.mAVCNalUnitType;
                pic.mIsReferencePic = true;
                pic.mIsLongTermRefPic = sliceHeader.mLongTermReferenceFlag;
                pic.mIndex = mPicIndex;
                pic.mPicOutputFlag = (sliceHeader.mPicOutputFlag != 0) ? true : false;

                // Update picture numbers.
                if (!sliceHeader.mIdrPicFlag)
                    dpbUpdatePicNums(mDPB, sliceHeader.mFrameNum, sliceHeader.mMaxFrameNum);

                int numRefFrames = 0;
                int numRefFramesL1 = 0;
                int refFramesExist = 0;
                int numExistingRefFrames;
                int retCode = refFramesExist; /// @todo retCode may not need this kind of initialization.
                if (!((sliceHeader.mSliceType == SliceType::I) || (sliceHeader.mSliceType == SliceType::SI)))
                {
                    numRefFrames = sliceHeader.mNumRefIdxL0ActiveMinus1 + 1;

                    if (sliceHeader.mSliceType == SliceType::B)
                    {
                        numRefFramesL1 = sliceHeader.mNumRefIdxL1ActiveMinus1 + 1;

                        numExistingRefFrames = sliceInitRefPicListBslice(sliceHeader.mSps->mPicOrderCntType,
                            mCurrentPic->mPoc);

                        if (numExistingRefFrames < numRefFrames)
                        {
                            refFramesExist = sliceFixRefPicList(mRefPicList0, numExistingRefFrames, numRefFrames,
                                mCurrentPic->mWidth, mCurrentPic->mHeight);
                            sliceFixRefPicList(mRefPicList1, numExistingRefFrames, numRefFramesL1,
                                mCurrentPic->mWidth, mCurrentPic->mHeight);
                        }
                        else
                            refFramesExist = 1;

                        if (sliceHeader.mRefPicListModificationFlagL0 && numExistingRefFrames > 0)
                        {
                            retCode = refPicListReordering(sliceHeader, mRefPicList0, numRefFrames,
                                sliceHeader.mRefPicListsModificationL0);
                            if (retCode < 0)
                                return retCode;
                        }
                        if (sliceHeader.mRefPicListModificationFlagL1 && numExistingRefFrames > 0)
                        {
                            retCode = refPicListReordering(sliceHeader, mRefPicList1, numRefFramesL1,
                                sliceHeader.mRefPicListsModificationL1);
                            if (retCode < 0)
                                return retCode;
                        }
                    }
                    else
                    {
                        numExistingRefFrames = sliceInitRefPicList(mRefPicList0);

                        if (numExistingRefFrames < numRefFrames)
                        {
                            refFramesExist = sliceFixRefPicList(mRefPicList0, numExistingRefFrames, numRefFrames,
                                mCurrentPic->mWidth, mCurrentPic->mHeight);
                        }
                        else
                            refFramesExist = 1;

                        if (sliceHeader.mRefPicListModificationFlagL0 && numExistingRefFrames > 0)
                        {
                            retCode = refPicListReordering(sliceHeader, mRefPicList0, numRefFrames,
                                sliceHeader.mRefPicListsModificationL0);
                            if (retCode < 0)
                                return retCode;
                        }
                    }
                }

                // Set Access Unit params.
                accessUnit.mPicIndex = pic.mIndex;
                accessUnit.mDisplayOrder =
                    (pic.mIndex < int(mDisplayNumArray.size())) ? mDisplayNumArray.at(pic.mIndex) : 0;
                accessUnit.mPoc = pic.mPoc;
                accessUnit.mPicWidth = pic.mWidth;
                accessUnit.mPicHeight = pic.mHeight;
                accessUnit.mIsIdr = pic.mIsIDR;
                accessUnit.mPicOutputFlag = pic.mPicOutputFlag;

                getRefPicIndices(accessUnit.mRefPicIndices, mRefPicList0, mRefPicList1);

                /// @todo This code assumes 1 slice per picture. Update the code for multi-slice case when necessary.
                if (!mIsCurrPicFinished)
                    finishCurrentPic();
            }

            mCurrNalUnitData.clear();
        }

        isFirstNaluInAU = false;
    }

    accessUnit.mIsIntra = true;
    for (const auto i : mSliceList)
    {
        if (i->mSliceType != SliceType::I)
        {
            accessUnit.mIsIntra = false;
            break;
        }
    }

    // Delete slices.
    for (auto i : mSliceList)
    {
        delete i;
    }
    mSliceList.clear();

    if (accessUnit.mNalUnits.size() > 0)
    {
        mPicIndex++;
        return true;
    }
    else
    {
        return false;
    }
}

void AvcParser::getRefPicIndices(vector<unsigned int>& refPicIndices, const vector<Picture*>& mRefPicList0,
                                 const vector<Picture*>& mRefPicList1)
{
    refPicIndices.clear();

    for (const auto i : mRefPicList0)
    {
        if ((i) != nullptr)
        {
            if (isUniquePicIndex(refPicIndices, i->mIndex))
            {
                refPicIndices.push_back(i->mIndex);
            }
        }
    }

    for (const auto i : mRefPicList1)
    {
        if ((i) != nullptr)
        {
            if (isUniquePicIndex(refPicIndices, i->mIndex))
            {
                refPicIndices.push_back(i->mIndex);
            }
        }
    }
}

bool AvcParser::isUniquePicIndex(const vector<unsigned int>& refPicIndices, const unsigned int picIndex)
{
    bool isUniqueIndex = true;

    for (const auto i : refPicIndices)
    {
        if (i == picIndex)
        {
            isUniqueIndex = false;
            break;
        }
    }

    return isUniqueIndex;
}

int AvcParser::parseNalUnit(const vector<uint8_t>& nalUnit, NalUnitHeader& naluHeader)
{
    vector<uint8_t> nalUnitRBSP;

    convertByteStreamToRBSP(nalUnit, nalUnitRBSP);
    BitStream bitstr(nalUnitRBSP);
    parseNalUnitHeader(bitstr, naluHeader);

    // Only SPS, PPS and slice header are parsed
    if (naluHeader.mAVCNalUnitType == AVCNalUnitType::SPS)
    {
        SequenceParameterSet* sps = new SequenceParameterSet { };
        parseSPS(bitstr, *sps);
        removeSps(sps->mSpsId);
        mSpsList.push_back(sps);
    }
    else if (naluHeader.mAVCNalUnitType == AVCNalUnitType::PPS)
    {
        PictureParameterSet* pps = new PictureParameterSet { };
        parsePPS(bitstr, *pps);
        removePps(pps->mPpsId);
        mPpsList.push_back(pps);
    }
    else if (isVclNaluType(naluHeader.mAVCNalUnitType))
    {
        // Parse the Slice Header here.
        SliceHeader* sliceHeader = new SliceHeader { };
        parseSliceHeader(bitstr, *sliceHeader, naluHeader.mAVCNalUnitType, naluHeader.mNuhRefIdc);
        mSliceList.push_back(sliceHeader);
    }
    /// @todo In the future, we may need to check the boundary condition here, if necessary for multi-slice parsing.
    return 0;
}

AvcParser::SequenceParameterSet* AvcParser::findSps(const unsigned int spsId)
{
    SequenceParameterSet* sps = nullptr;

    for (auto& i : mSpsList)
    {
        if (i->mSpsId == spsId)
        {
            sps = i;
            break;
        }
    }

    return sps;
}

AvcParser::PictureParameterSet* AvcParser::findPps(const unsigned int ppsId)
{
    PictureParameterSet* pps = nullptr;

    for (auto& i : mPpsList)
    {
        if (i->mPpsId == ppsId)
        {
            pps = i;
            break;
        }
    }

    return pps;
}

void AvcParser::removeSps(const unsigned int spsId)
{
    auto i = mSpsList.begin();

    for (; i != mSpsList.end(); ++i)
    {
        if ((*i)->mSpsId == spsId)
        {
            delete (*i);
            mSpsList.erase(i);
            break;
        }
    }
}

void AvcParser::removePps(const unsigned int ppsId)
{
    auto i = mPpsList.begin();

    for (; i != mPpsList.end(); ++i)
    {
        if ((*i)->mPpsId == ppsId)
        {
            delete (*i);
            mPpsList.erase(i);
            break;
        }
    }
}

bool AvcParser::isVclNaluType(const AVCNalUnitType naluType)
{
    return (naluType >= AVCNalUnitType::CODED_SLICE_NON_IDR && naluType <= AVCNalUnitType::CODED_SLICE_IDR);
}

int AvcParser::decodePoc(const SliceHeader& slice)
{
    int i;
    int maxPocLsb;
    int expectedPicOrderCnt, picOrderCntCycleCnt = 0;
    int expectedDeltaPerPicOrderCntCycle, frameNumInPicOrderCntCycle = 0, absFrameNum;
    int tempPicOrderCnt;
    int poc = 0;

    // POC
    if (slice.mSps->mPicOrderCntType == 0)
    {
        // Reset prevPocMsb, prevPocLsb if needed.
        if (slice.mIdrPicFlag)
        {
            mPrevPicOrderCntMsb = mPrevPicOrderCntLsb = 0;
        }
        else if (mPrevPicHasMMCO5)
        {
            mPrevPicOrderCntMsb = 0;
            mPrevPicOrderCntLsb = mPreviousRefPicPoc;
        }

        // PicOrderCntMsb is derived:
        maxPocLsb = (unsigned int)1 << (slice.mSps->mLog2MaxPicOrderCntLsbMinus4 + 4);
        if ((int)slice.mPicOrderCntLsb < (int)mPrevPicOrderCntLsb
            && ((int)mPrevPicOrderCntLsb - (int)slice.mPicOrderCntLsb) >= (maxPocLsb / 2))
            mPicOrderCntMsb = mPrevPicOrderCntMsb + maxPocLsb;
        else if ((int)slice.mPicOrderCntLsb > (int)mPrevPicOrderCntLsb
            && ((int)slice.mPicOrderCntLsb - (int)mPrevPicOrderCntLsb) > (maxPocLsb / 2))
            mPicOrderCntMsb = mPrevPicOrderCntMsb - maxPocLsb;
        else
            mPicOrderCntMsb = mPrevPicOrderCntMsb;
        // poc
        poc = mPicOrderCntMsb + slice.mPicOrderCntLsb;
    }
    else if (slice.mSps->mPicOrderCntType == 1)
    {
        // Reset prevFrameNumOffset if needed.
        if (!slice.mIdrPicFlag && mPrevPicHasMMCO5)
            mPreviousFrameNumOffset = 0;

        // frameNumOffset is derived as follows:
        if (slice.mIdrPicFlag)
            mFrameNumOffset = 0;
        else if (mPreviousFrameNum > (int)slice.mFrameNum)
            mFrameNumOffset = mPreviousFrameNumOffset + slice.mMaxFrameNum;
        else
            mFrameNumOffset = mPreviousFrameNumOffset;

        // absFrameNum is derived as follows:
        if (slice.mSps->mNumRefFramesInPicOrderCntCycle != 0)
            absFrameNum = mFrameNumOffset + slice.mFrameNum;
        else
            absFrameNum = 0;
        if (slice.mNuhRefIdc == 0 && absFrameNum > 0)
            absFrameNum = absFrameNum - 1;

        // When absFrameNum > 0, picOrderCntCycleCnt and frameNumInPicOrderCntCycle are derived as follows:
        if (absFrameNum > 0)
        {
            picOrderCntCycleCnt = (absFrameNum - 1) / slice.mSps->mNumRefFramesInPicOrderCntCycle;
            frameNumInPicOrderCntCycle = (absFrameNum - 1) % slice.mSps->mNumRefFramesInPicOrderCntCycle;
        }

        // expectedDeltaPerPicOrderCntCycle
        expectedDeltaPerPicOrderCntCycle = 0;
        for (i = 0; i < (int)slice.mSps->mNumRefFramesInPicOrderCntCycle; i++)
            expectedDeltaPerPicOrderCntCycle += slice.mSps->mOffsetForRefFrame[i];

        // expectedPicOrderCnt
        if (absFrameNum > 0)
        {
            expectedPicOrderCnt = picOrderCntCycleCnt * expectedDeltaPerPicOrderCntCycle;
            for (i = 0; i <= frameNumInPicOrderCntCycle; i++)
                expectedPicOrderCnt = expectedPicOrderCnt + slice.mSps->mOffsetForRefFrame[i];
        }
        else
            expectedPicOrderCnt = 0;
        if (slice.mNuhRefIdc == 0)
            expectedPicOrderCnt = expectedPicOrderCnt + slice.mSps->mOffsetForNonRefPic;

        // poc
        poc = expectedPicOrderCnt + slice.mDeltaPicOrderCnt0;
    }
    else if (slice.mSps->mPicOrderCntType == 2)
    {
        // prevFrameNumOffset is derived as follows
        if (!slice.mIdrPicFlag && mPrevPicHasMMCO5)
            mPreviousFrameNumOffset = 0;

        // FrameNumOffset is derived as follows.
        if (slice.mIdrPicFlag)
            mFrameNumOffset = 0;
        else if (mPreviousFrameNum > (int)slice.mFrameNum)
            mFrameNumOffset = mPreviousFrameNumOffset + slice.mMaxFrameNum;
        else
            mFrameNumOffset = mPreviousFrameNumOffset;

        // tempPicOrderCnt is derived as follows
        if (slice.mIdrPicFlag)
            tempPicOrderCnt = 0;
        else if (slice.mNuhRefIdc == 0)
            tempPicOrderCnt = 2 * (mFrameNumOffset + slice.mFrameNum) - 1;
        else
            tempPicOrderCnt = 2 * (mFrameNumOffset + slice.mFrameNum);

        // poc
        poc = tempPicOrderCnt;
    }

    return poc;
}

bool AvcParser::checkAccessUnitBoundary(const vector<uint8_t>& nalUnit, const bool isFirstNaluInAU,
                                        const bool firstVclNaluFound)
{
    if (nalUnit.size() == 0)
    {
        return true;
    }

    if (isFirstVclNaluInPic(nalUnit))
    {
        if (firstVclNaluFound)
        {
            // NALU is not the first vcl NALU of the current picture so it must be first NALU of the next picture.
            return true;
        }
        else
        {
            return false;
        }
    }

    if (isFirstNaluInAU)
    {
        return false;
    }

    /// @todo The following code may require an update when there are multiple slices in an AU. Handle this case when necessary.
    if (mSliceList.size() > 2)
    {
        // If there are at least 2 slices present in the list.
        SliceHeader* nextSlice = mSliceList.at(mSliceList.size() - 1);
        SliceHeader* currSlice = mSliceList.at(mSliceList.size() - 2);

        // frame_num differs in value.
        if (currSlice->mFrameNum != nextSlice->mFrameNum)
            return true;

        // nal_ref_idc differs in value with one of the nal_ref_idc values being equal to 0.
        if ((currSlice->mIdrPicId != nextSlice->mIdrPicId) &&
            (currSlice->mNuhRefIdc == 0 || nextSlice->mNuhRefIdc == 0))
            return true;

        // nal_unit_type is equal to 5 for one coded slice NAL unit and
        // is not equal to 5 in the other coded slice NAL unit
        if ((currSlice->mNaluType == AVCNalUnitType::CODED_SLICE_IDR
            || nextSlice->mNaluType == AVCNalUnitType::CODED_SLICE_IDR) &&
            (currSlice->mNaluType != nextSlice->mNaluType))
            return true;

        // nal_unit_type is equal to 5 for both and idr_pic_id differs in value.
        if (currSlice->mNaluType == AVCNalUnitType::CODED_SLICE_IDR &&
            nextSlice->mNaluType == AVCNalUnitType::CODED_SLICE_IDR &&
            (currSlice->mIdrPicId != nextSlice->mIdrPicId))
            return true;

        SequenceParameterSet* prevSps = currSlice->mSps;
        SequenceParameterSet* currSps = nextSlice->mSps;

        // pic_order_cnt_type is equal to 0 for both and
        // either pic_order_cnt_lsb differs in value, or delta_pic_order_cnt_bottom differs in value.
        if ((prevSps->mPicOrderCntType == 0 && currSps->mPicOrderCntType == 0) &&
            ((currSlice->mPicOrderCntLsb != nextSlice->mPicOrderCntLsb) ||
                (currSlice->mDeltaPicOrderCntBottom != nextSlice->mDeltaPicOrderCntBottom)))
            return true;

        // pic_order_cnt_type is equal to 1 for both and
        // either delta_pic_order_cnt[ 0 ] differs in value, or delta_pic_order_cnt[ 1 ] differs in value.
        if ((prevSps->mPicOrderCntType == 1 && currSps->mPicOrderCntType == 1) &&
            ((currSlice->mDeltaPicOrderCnt0 != nextSlice->mDeltaPicOrderCnt0) ||
                (currSlice->mDeltaPicOrderCnt1 != nextSlice->mDeltaPicOrderCnt1)))
            return true;
    }

    return false;
}

unsigned int AvcParser::ceilLog2(const unsigned int x)
{
    unsigned int i = 1;

    while (x > (1u << i))
    {
        ++i;
    }

    return i;
}

int AvcParser::parseNalUnitHeader(BitStream& bitstr, NalUnitHeader& naluHeader)
{
    bitstr.readBits(1);  // forbidden_zero_bit
    naluHeader.mNuhRefIdc = bitstr.readBits(2); //nal_ref_idc
    naluHeader.mAVCNalUnitType = AVCNalUnitType(bitstr.readBits(5)); // nal_unit_type
    return 0;
}

int AvcParser::parseRefPicListsModification(BitStream& bitstr, SliceHeader& slice)
{
    if ((slice.mSliceType != SliceType::I) && (slice.mSliceType != SliceType::SI))
    {
        slice.mRefPicListsModificationL0.clear();
        slice.mRefPicListModificationFlagL0 = bitstr.readBits(1);
        if (slice.mRefPicListModificationFlagL0)
        {
            unsigned int modification_of_pic_nums_idc;
            do
            {
                modification_of_pic_nums_idc = bitstr.readExpGolombCode();
                RefPicListsModification rpm { };
                rpm.mModificationOfPicNumsIdc = modification_of_pic_nums_idc;
                if ((rpm.mModificationOfPicNumsIdc == 0) || (rpm.mModificationOfPicNumsIdc == 1))
                    rpm.mAbsDiffPicNumMinus1 = bitstr.readExpGolombCode();
                else if (rpm.mModificationOfPicNumsIdc == 2)
                    rpm.mLongTermPicNum = bitstr.readExpGolombCode();
                slice.mRefPicListsModificationL0.push_back(rpm);
            } while (modification_of_pic_nums_idc != 3);
        }
    }
    if (slice.mSliceType == SliceType::B)
    {
        slice.mRefPicListsModificationL1.clear();
        slice.mRefPicListModificationFlagL1 = bitstr.readBits(1);
        if (slice.mRefPicListModificationFlagL1)
        {
            unsigned int modification_of_pic_nums_idc;
            do
            {
                modification_of_pic_nums_idc = bitstr.readExpGolombCode();
                RefPicListsModification rpm { };
                rpm.mModificationOfPicNumsIdc = modification_of_pic_nums_idc;
                if ((rpm.mModificationOfPicNumsIdc == 0) || (rpm.mModificationOfPicNumsIdc == 1))
                    rpm.mAbsDiffPicNumMinus1 = bitstr.readExpGolombCode();
                else if (rpm.mModificationOfPicNumsIdc == 2)
                    rpm.mLongTermPicNum = bitstr.readExpGolombCode();
                slice.mRefPicListsModificationL1.push_back(rpm);
            } while (modification_of_pic_nums_idc != 3);
        }
    }
    return 0;
}

int AvcParser::parseScalingListData(BitStream& bitstr, int* scalingList, unsigned int& useDefaultScalingMatrixFlag,
                                    const unsigned int sizeOfScalingList)
{
    int lastScale = 8;
    int nextScale = 8;

    for (unsigned int j = 0; j < sizeOfScalingList; j++)
    {
        if (nextScale != 0)
        {
            const int delta_scale = bitstr.readSignedExpGolombCode();
            nextScale = (lastScale + delta_scale + 256) % 256;
            useDefaultScalingMatrixFlag = (j == 0 && nextScale == 0);
        }
        scalingList[j] = ((nextScale == 0) ? lastScale : nextScale);
        lastScale = scalingList[j];
    }
    return 0;
}

void AvcParser::setVuiDefaults(VuiParameters& vui)
{
    vui = {};

    // The following syntax elements have explicit default values in the spec
    vui.mVideoFormat = 5;
    vui.mColourPrimaries = 2;
    vui.mTransferCharacteristics = 2;
    vui.mMatrixCoeffs = 2;
    vui.mMotionVectorsOverPicBoundariesFlag = 1;
    vui.mMaxBytesPerPicDenom = 2;
    vui.mMaxBitsPerMbDenom = 1;
    vui.mLog2MaxMvLengthHorizontal = 16;
    vui.mLog2MaxMvLengthVertical = 16;

    vui.mMaxDecFrameBuffering = DPB_MAX_SIZE;
    vui.mMaxNumReorderFrames = vui.mMaxDecFrameBuffering;
}

int AvcParser::parseVuiParameters(BitStream& bitstr, VuiParameters& vui)
{
    vui.mAspectRatioInfoPresentFlag = bitstr.readBits(1);
    if (vui.mAspectRatioInfoPresentFlag)
    {
        vui.mAspectRatioIdc = bitstr.readBits(8);
        if (vui.mAspectRatioIdc == EXTENDED_SAR)
        {
            vui.mSarWidth = bitstr.readBits(16);
            vui.mSarHeight = bitstr.readBits(16);
        }
    }
    vui.mOverscanInfoPresentFlag = bitstr.readBits(1);
    if (vui.mOverscanInfoPresentFlag)
    {
        vui.mOverscanAppropriateFlag = bitstr.readBits(1);
    }
    vui.mVideoSignalTypePresentFlag = bitstr.readBits(1);
    if (vui.mVideoSignalTypePresentFlag)
    {
        vui.mVideoFormat = bitstr.readBits(3);
        vui.mVideoFullRangeFlag = bitstr.readBits(1);
        vui.mColourDescriptionPresentFlag = bitstr.readBits(1);
        if (vui.mColourDescriptionPresentFlag)
        {
            vui.mColourPrimaries = bitstr.readBits(8);
            vui.mTransferCharacteristics = bitstr.readBits(8);
            vui.mMatrixCoeffs = bitstr.readBits(8);
        }
    }
    vui.mChromaLocInfoPresentFlag = bitstr.readBits(1);
    if (vui.mChromaLocInfoPresentFlag)
    {
        vui.mChromaSampleLocTypeTopField = bitstr.readExpGolombCode();
        vui.mChromaSampleLocTypeBottomField = bitstr.readExpGolombCode();
    }
    vui.mVuiTimingInfoPresentFlag = bitstr.readBits(1);
    if (vui.mVuiTimingInfoPresentFlag)
    {
        vui.mVuiNumUnitsInTick = bitstr.readBits(32);
        vui.mVuiTimeScale = bitstr.readBits(32);
        vui.mVuiFixedFrameRateFlag = bitstr.readBits(1);
    }
    vui.mHrdParametersPresentFlag = bitstr.readBits(1);
    if (vui.mHrdParametersPresentFlag)
    {
        parseHrdParameters(bitstr, vui.mHrdParameters);
    }
    vui.mVclParametersPresentFlag = bitstr.readBits(1);
    if (vui.mVclParametersPresentFlag)
    {
        parseHrdParameters(bitstr, vui.mVclParameters);
    }
    if (vui.mHrdParametersPresentFlag || vui.mVclParametersPresentFlag)
    {
        vui.mLowDelayHrdFlag = bitstr.readBits(1);
    }
    vui.mPicStructPresentFlag = bitstr.readBits(1);
    vui.mBitstreamRestrictionFlag = bitstr.readBits(1);
    if (vui.mBitstreamRestrictionFlag)
    {
        vui.mMotionVectorsOverPicBoundariesFlag = bitstr.readBits(1);
        vui.mMaxBytesPerPicDenom = bitstr.readExpGolombCode();
        vui.mMaxBitsPerMbDenom = bitstr.readExpGolombCode();
        vui.mLog2MaxMvLengthHorizontal = bitstr.readExpGolombCode();
        vui.mLog2MaxMvLengthVertical = bitstr.readExpGolombCode();
        vui.mMaxNumReorderFrames = bitstr.readExpGolombCode();
        vui.mMaxDecFrameBuffering = bitstr.readExpGolombCode();
    }

    return 0;
}

int AvcParser::parseHrdParameters(BitStream& bitstr, HrdParameters& hrd)
{
    hrd.mCpbCntMinus1 = bitstr.readExpGolombCode();
    hrd.mBitRateScale = bitstr.readBits(4);
    hrd.mCpbSizeScale = bitstr.readBits(4);

    hrd.mBitRateValueMinus1.clear();
    hrd.mCpbSizeValueMinus1.clear();
    hrd.mCbrFlag.clear();
    for (unsigned int i = 0; i <= hrd.mCpbCntMinus1; i++)
    {
        hrd.mBitRateValueMinus1.push_back(bitstr.readExpGolombCode());
        hrd.mCpbSizeValueMinus1.push_back(bitstr.readExpGolombCode());
        hrd.mCbrFlag.push_back(bitstr.readBits(1));
    }
    hrd.mInitialCpbRemovalDelayLengthMinus1 = bitstr.readBits(5);
    hrd.mCpbRemovalDelayLengthMinus1 = bitstr.readBits(5);
    hrd.mDpbOutputDelayLengthMinus1 = bitstr.readBits(5);
    hrd.mTimeOffsetLength = bitstr.readBits(5);

    return 0;
}

int AvcParser::parsePredWeightTable(BitStream& bitstr, SliceHeader& slice)
{
    slice.mPredWeightTable.mWeightedPredFlag = slice.mPps->mWeightedPredFlag;
    slice.mPredWeightTable.mWeightedBipredIdc = slice.mPps->mWeightedBipredIdc;

    slice.mPredWeightTable.mLumaLog2WeightDenom = bitstr.readBits(1);
    unsigned int chromaArrayType = (slice.mSps->mSeparateColourPlaneFlag == false) ? slice.mSps->mChromaFormatIdc : 0;
    if (chromaArrayType != 0)
        slice.mPredWeightTable.mChromaLog2WeightDenom = bitstr.readExpGolombCode();

    slice.mPredWeightTable.mLumaWeightL0Flag.resize(slice.mNumRefIdxL0ActiveMinus1 + 1, 0);
    slice.mPredWeightTable.mLumaWeightL0.resize(slice.mNumRefIdxL0ActiveMinus1 + 1);
    slice.mPredWeightTable.mLumaOffsetL0.resize(slice.mNumRefIdxL0ActiveMinus1 + 1);

    if (chromaArrayType != 0)
    {
        slice.mPredWeightTable.mChromaWeightL0Flag.resize(slice.mNumRefIdxL0ActiveMinus1 + 1, 0);
        slice.mPredWeightTable.mChromaWeightL0.resize(slice.mNumRefIdxL0ActiveMinus1 + 1);
        slice.mPredWeightTable.mChromaOffsetL0.resize(slice.mNumRefIdxL0ActiveMinus1 + 1);
    }

    for (unsigned int i = 0; i <= slice.mNumRefIdxL0ActiveMinus1; i++)
    {
        slice.mPredWeightTable.mLumaWeightL0Flag[i] = bitstr.readBits(1);
        if (slice.mPredWeightTable.mLumaWeightL0Flag[i])
        {
            slice.mPredWeightTable.mLumaWeightL0[i] = bitstr.readSignedExpGolombCode();
            slice.mPredWeightTable.mLumaOffsetL0[i] = bitstr.readSignedExpGolombCode();
        }
        if (chromaArrayType != 0)
        {
            slice.mPredWeightTable.mChromaWeightL0Flag[i] = bitstr.readBits(1);
            if (slice.mPredWeightTable.mChromaWeightL0Flag[i])
            {
                for (unsigned int j = 0; j < 2; j++)
                {
                    slice.mPredWeightTable.mChromaWeightL0[i][j] = bitstr.readSignedExpGolombCode();
                    slice.mPredWeightTable.mChromaOffsetL0[i][j] = bitstr.readSignedExpGolombCode();
                }
            }
        }
    }
    if (slice.mSliceType == SliceType::B)
    {
        slice.mPredWeightTable.mLumaWeightL1Flag.resize(slice.mNumRefIdxL1ActiveMinus1 + 1, 0);
        if (chromaArrayType != 0)
        {
            slice.mPredWeightTable.mChromaWeightL1Flag.resize(slice.mNumRefIdxL1ActiveMinus1 + 1, 0);
            slice.mPredWeightTable.mChromaWeightL1.resize(slice.mNumRefIdxL1ActiveMinus1 + 1);
            slice.mPredWeightTable.mChromaOffsetL1.resize(slice.mNumRefIdxL1ActiveMinus1 + 1);
        }

        for (unsigned int i = 0; i <= slice.mNumRefIdxL1ActiveMinus1; i++)
        {
            slice.mPredWeightTable.mLumaWeightL1Flag[i] = bitstr.readBits(1);
            if (slice.mPredWeightTable.mLumaWeightL1Flag[i])
            {
                slice.mPredWeightTable.mLumaWeightL1[i] = bitstr.readSignedExpGolombCode();
                slice.mPredWeightTable.mLumaOffsetL1[i] = bitstr.readSignedExpGolombCode();
            }
            if (chromaArrayType != 0)
            {
                slice.mPredWeightTable.mChromaWeightL1Flag[i] = bitstr.readBits(1);
                if (slice.mPredWeightTable.mChromaWeightL1Flag[i])
                {
                    for (unsigned int j = 0; j < 2; j++)
                    {
                        slice.mPredWeightTable.mChromaWeightL1[i][j] = bitstr.readSignedExpGolombCode();
                        slice.mPredWeightTable.mChromaOffsetL1[i][j] = bitstr.readSignedExpGolombCode();
                    }
                }
            }
        }
    }
    return 0;
}

int AvcParser::parseSPS(BitStream& bitstr, SequenceParameterSet& sps)
{
    sps.mProfileIdc = bitstr.read8Bits();
    sps.mConstraintSet0Flag = bitstr.readBits(1);
    sps.mConstraintSet1Flag = bitstr.readBits(1);
    sps.mConstraintSet2Flag = bitstr.readBits(1);
    sps.mConstraintSet3Flag = bitstr.readBits(1);
    sps.mConstraintSet4Flag = bitstr.readBits(1);
    sps.mConstraintSet5Flag = bitstr.readBits(1);

    bitstr.readBits(2); // reserved_zero_2bits
    sps.mLevelIdc = bitstr.read8Bits();
    sps.mSpsId = bitstr.readExpGolombCode();
    unsigned int pid = sps.mProfileIdc;
    if ((pid == PS_HIGH_PROFILE_IDC) || (pid == PS_HIGH_10_PROFILE_IDC) || (pid == PS_HIGH_422_PROFILE_IDC) ||
        (pid == PS_HIGH_444_PREDICTIVE_PROFILE_IDC) || (pid == PS_CAVLC_444_PROFILE_IDC) ||
        (pid == PS_SCALABLE_BASELINE_PROFILE_IDC) || (pid == PS_SCALABLE_HIGH_PROFILE_IDC) ||
        (pid == PS_MULTIVIEW_HIGH_PROFILE_IDC) || (pid == PS_STEREO_HIGH_PROFILE_IDC) ||
        (pid == PS_MULTIVIEW_DEPTH_HIGH_PROFILE_IDC) || (pid == 139) || (pid == 134))
    {
        sps.mChromaFormatIdc = bitstr.readExpGolombCode();
        if (sps.mChromaFormatIdc == 3)
        {
            sps.mSeparateColourPlaneFlag = bitstr.readBits(1);
        }
        sps.mBitDepthLumaMinus8 = bitstr.readExpGolombCode();
        sps.mBitDepthChromaMinus8 = bitstr.readExpGolombCode();
        sps.mQpPrimeYZeroTransformBypassFlag = bitstr.readBits(1);
        sps.mScalingMatrixPresentFlag = bitstr.readBits(1);
        if (sps.mScalingMatrixPresentFlag)
        {
            for (unsigned int i = 0; i < ((sps.mChromaFormatIdc != 3) ? 8 : 12); i++)
            {
                sps.mSpsScalingListPresentFlag[i] = bitstr.readBits(1);
                if (sps.mSpsScalingListPresentFlag[i])
                {
                    if (i < 6)
                    {
                        parseScalingListData(bitstr, &(sps.mScalingListData.mScalingList4x4[i][0]),
                            sps.mScalingListData.mUseDefaultScalingMatrix4x4Flag[i], 16);
                    }
                    else
                    {
                        parseScalingListData(bitstr, &(sps.mScalingListData.mScalingList8x8[i - 6][0]),
                            sps.mScalingListData.mUseDefaultScalingMatrix8x8Flag[i - 6], 64);
                    }
                }
            }
        }
    }
    sps.mLog2MaxFrameNumMinus4 = bitstr.readExpGolombCode();
    sps.mPicOrderCntType = bitstr.readExpGolombCode();
    if (sps.mPicOrderCntType == 0)
    {
        sps.mLog2MaxPicOrderCntLsbMinus4 = bitstr.readExpGolombCode();
    }
    else if (sps.mPicOrderCntType == 1)
    {
        sps.mDeltaPicOrderAlwaysZeroFlag = bitstr.readBits(1);
        sps.mOffsetForNonRefPic = bitstr.readSignedExpGolombCode();
        sps.mOffsetForTopToBottomField = bitstr.readSignedExpGolombCode();
        sps.mNumRefFramesInPicOrderCntCycle = bitstr.readExpGolombCode();
        for (unsigned int i = 0; i < sps.mNumRefFramesInPicOrderCntCycle; i++)
        {
            sps.mOffsetForRefFrame[i] = bitstr.readSignedExpGolombCode();
        }
    }
    sps.mNumRefFrames = bitstr.readExpGolombCode();
    sps.mGapsInFrameNumValueAllowedFlag = bitstr.readBits(1);
    sps.mPicWidthInMbs_minus1 = bitstr.readExpGolombCode();
    sps.mPicHeightInMapUnitsMinus1 = bitstr.readExpGolombCode();
    sps.mFrameMbsOnlyFlag = bitstr.readBits(1);
    if (!sps.mFrameMbsOnlyFlag)
    {
        sps.mMbAdaptiveFrameFieldFlag = bitstr.readBits(1);
    }
    sps.mDirect8x8InferenceFlag = bitstr.readBits(1);
    sps.mFrameCroppingFlag = bitstr.readBits(1);
    if (sps.mFrameCroppingFlag)
    {
        sps.mFrameCropLeftOffset = bitstr.readExpGolombCode();
        sps.mFrameCropRightOffset = bitstr.readExpGolombCode();
        sps.mFrameCropTopOffset = bitstr.readExpGolombCode();
        sps.mFrameCropBottomOffset = bitstr.readExpGolombCode();
    }
    sps.mVuiParametersPresentFlag = bitstr.readBits(1);

    setVuiDefaults(sps.mVuiParameters);
    if (sps.mVuiParametersPresentFlag)
    {
        parseVuiParameters(bitstr, sps.mVuiParameters);
    }

    return 0;
}

/// @todo Add scaling matrix parsing in the future, if needed.
int AvcParser::parsePPS(BitStream& bitstr, PictureParameterSet& pps)
{
    pps.mPpsId = bitstr.readExpGolombCode();
    pps.mSpsId = bitstr.readExpGolombCode();
    pps.mEntropyCodingModeFlag = bitstr.readBits(1);
    pps.mPicOrderPresentFlag = bitstr.readBits(1);
    pps.mNumSliceGroupsMinus1 = bitstr.readExpGolombCode();
    if (pps.mNumSliceGroupsMinus1 > 0)
    {
        pps.mSliceGroupMapType = bitstr.readExpGolombCode();
        if (pps.mSliceGroupMapType == 0)
        {
            for (unsigned int iGroup = 0; iGroup <= pps.mNumSliceGroupsMinus1; iGroup++)
                pps.mRunLengthMinus1[iGroup] = bitstr.readExpGolombCode();
        }
        else if (pps.mSliceGroupMapType == 2)
        {
            for (unsigned int iGroup = 0; iGroup <= pps.mNumSliceGroupsMinus1; iGroup++)
            {
                pps.mTopLeft[iGroup] = bitstr.readExpGolombCode();
                pps.mBottomRight[iGroup] = bitstr.readExpGolombCode();
            }
        }
        else if ((pps.mSliceGroupMapType == 3) || (pps.mSliceGroupMapType == 4) || (pps.mSliceGroupMapType == 5))
        {
            pps.mSliceGroupChangeDirectionFlag = bitstr.readBits(1);
            pps.mSliceGroupChangeRateMinus1 = bitstr.readExpGolombCode();
        }
        else if (pps.mSliceGroupMapType == 6)
        {
            pps.mPicSizeInMapUnitsMinus1 = bitstr.readExpGolombCode();

            pps.mSliceGroupId.clear();
            unsigned int tmp, len = 0;

            // Calculate len = ceil( Log2( num_slice_groups_minus1 + 1 ) )
            tmp = pps.mNumSliceGroupsMinus1 + 1;
            tmp = tmp >> 1;
            for (len = 0; len < 16 && tmp != 0; len++)
                tmp >>= 1;
            if ((((unsigned int)1) << len) < (pps.mNumSliceGroupsMinus1 + 1))
                len++;

            for (unsigned int i = 0; i >= pps.mPicSizeInMapUnitsMinus1; i++)
                pps.mSliceGroupId.push_back(len);
        }
    }
    pps.mNumRefIdxL0ActiveMinus1 = bitstr.readExpGolombCode();
    pps.mNumRefIdxL1ActiveMinus1 = bitstr.readExpGolombCode();
    pps.mWeightedPredFlag = bitstr.readBits(1);
    pps.mWeightedBipredIdc = bitstr.readBits(2);
    pps.mPicInitQpMinus26 = bitstr.readSignedExpGolombCode();
    pps.mPicInitQsMinus26 = bitstr.readSignedExpGolombCode();
    pps.mChromaQpIndexOffset = bitstr.readSignedExpGolombCode();
    pps.mDeblockingFilterParametersPresentFlag = bitstr.readBits(1);
    pps.mConstrainedIntraPredFlag = bitstr.readBits(1);
    pps.mRedundantPicCntPresentFlag = bitstr.readBits(1);
    if (moreRBSPData(bitstr))
    {
        pps.mTransform8x8ModeFlag = bitstr.readBits(1);
        pps.mPicScalingMatrixPresentFlag = bitstr.readBits(1);
        if (pps.mPicScalingMatrixPresentFlag)
        {
            /// @todo Add scaling matrix parsing in the future, if needed.
        }
        pps.mSecondChromaQpIndexOffset = bitstr.readSignedExpGolombCode();
    }

    return 0;
}

int AvcParser::parseSliceHeader(BitStream& bitstr, SliceHeader& slice, AVCNalUnitType naluType,
                                const unsigned int naluhRefIdc)
{
    /// @todo Some value initializations to be aligned with the spec.
    slice.mPicOutputFlag = 1; /// @todo check of this flag is still needed when we have the mForOutput flag...
    slice.mNaluType = naluType;
    slice.mNuhRefIdc = naluhRefIdc;
    slice.mIdrPicFlag = ((naluType == AVCNalUnitType::CODED_SLICE_IDR) ? 1 : 0); // spec. 7.4.1 pg. 86

    // Parse slices based on the NAL Unit Type.
    if ((naluType == AVCNalUnitType::CODED_SLICE_NON_IDR) || (naluType == AVCNalUnitType::CODED_SLICE_IDR)
        || (naluType == AVCNalUnitType::CODED_SLICE_DPAR_A))
    {
        slice.mFirstMbInSlice = bitstr.readExpGolombCode();
        unsigned int slice_type = bitstr.readExpGolombCode();
        // Map to the compact Slice Type enumeration based on the sice_type enumeration.
        switch (slice_type % 5)
        {
        case 0:
            slice.mSliceType = SliceType::P;  // 0, 5
            break;
        case 1:
            slice.mSliceType = SliceType::B;  // 1, 6
            break;
        case 2:
            slice.mSliceType = SliceType::I;  // 2, 7
            break;
        case 3:
            slice.mSliceType = SliceType::SP; // 3, 8
            break;
        case 4:
            slice.mSliceType = SliceType::SI; // 4, 9
            break;
        }

        slice.mPpsId = bitstr.readExpGolombCode();
        // Find the related PPS.
        PictureParameterSet* pps = findPps(slice.mPpsId);
        slice.mPps = pps;
        // Find the related SPS.
        SequenceParameterSet* sps = findSps(slice.mPps->mSpsId);
        slice.mSps = sps;

        if (slice.mSps->mSeparateColourPlaneFlag)
            slice.mColorPlaneId = bitstr.readBits(2);

        slice.mMaxFrameNum = (unsigned int)(1 << (slice.mSps->mLog2MaxFrameNumMinus4 + 4));
        slice.mFrameNum = bitstr.readBits((int)(slice.mSps->mLog2MaxFrameNumMinus4) + 4);

        if (!slice.mSps->mFrameMbsOnlyFlag)
        {
            slice.mFieldPicFlag = bitstr.readBits(1);
            if (slice.mFieldPicFlag)
                slice.mBottomFieldFlag = bitstr.readBits(1);
        }
        if (slice.mIdrPicFlag)
            slice.mIdrPicId = bitstr.readExpGolombCode();
        if (slice.mSps->mPicOrderCntType == 0)
        {
            slice.mPicOrderCntLsb = bitstr.readBits((int)(slice.mSps->mLog2MaxPicOrderCntLsbMinus4) + 4);
            if (slice.mPps->mPicOrderPresentFlag && !slice.mFieldPicFlag)
                slice.mDeltaPicOrderCntBottom = bitstr.readSignedExpGolombCode();
        }
        if ((slice.mSps->mPicOrderCntType == 1) && !(slice.mSps->mDeltaPicOrderAlwaysZeroFlag))
        {
            slice.mDeltaPicOrderCnt0 = bitstr.readSignedExpGolombCode();
            if (slice.mPps->mPicOrderPresentFlag && !slice.mFieldPicFlag)
                slice.mDeltaPicOrderCnt1 = bitstr.readSignedExpGolombCode();
        }
        if (slice.mPps->mRedundantPicCntPresentFlag)
            slice.mRedundantPicCnt = bitstr.readExpGolombCode();
        if (slice.mSliceType == SliceType::B)
            slice.mDirectSpatialMvPredFlag = bitstr.readBits(1);
        if ((slice.mSliceType == SliceType::P) || (slice.mSliceType == SliceType::SP)
            || (slice.mSliceType == SliceType::B))
        {
            slice.mNumRefIdxActiveOverrideFlag = bitstr.readBits(1);
            if (slice.mNumRefIdxActiveOverrideFlag)
            {
                slice.mNumRefIdxL0ActiveMinus1 = bitstr.readExpGolombCode();
                if (slice.mSliceType == SliceType::B)
                    slice.mNumRefIdxL1ActiveMinus1 = bitstr.readExpGolombCode();
            }
        }

        if ((slice.mNaluType == AVCNalUnitType::SLICE_EXT) || (slice.mNaluType == AVCNalUnitType::SLICE_EXT_3D))
        {
            /// @todo SLICE_EXT and SLICE_EXT_3D parsing not implemented. To be implemented if needed (specified in Annex H).
        }
        else
            parseRefPicListsModification(bitstr, slice);

        if ((slice.mPps->mWeightedPredFlag && (slice.mSliceType == SliceType::P || slice.mSliceType == SliceType::SP))
            ||
            (slice.mPps->mWeightedBipredIdc == 1 && slice.mSliceType == SliceType::B))
            parsePredWeightTable(bitstr, slice);

        if (slice.mNuhRefIdc != 0)
            parseDecRefPicMarking(bitstr, slice);
        else
            slice.mAdaptiveRefPicMarkingModeFlag = false;

        if (slice.mPps->mEntropyCodingModeFlag && slice.mSliceType != SliceType::I && slice.mSliceType != SliceType::SI)
            slice.mCabacInitIdc = bitstr.readExpGolombCode();

        slice.mSliceQpDelta = bitstr.readSignedExpGolombCode();

        if (slice.mSliceType == SliceType::SP || slice.mSliceType == SliceType::SI)
        {
            if (slice.mSliceType == SliceType::SP)
                slice.mSpForSwitchFlag = bitstr.readBits(1);
            slice.mSliceQsDelta = bitstr.readSignedExpGolombCode();
        }
        if (slice.mPps->mDeblockingFilterParametersPresentFlag)
        {
            slice.mDisableDeblockingFilterIdc = bitstr.readExpGolombCode();
            if (slice.mDisableDeblockingFilterIdc != 1)
            {
                slice.mSliceAlphaCoOffsetDiv2 = bitstr.readSignedExpGolombCode();
                slice.mSliceBetaOffsetDiv2 = bitstr.readSignedExpGolombCode();
            }
        }

        if (slice.mPps->mNumSliceGroupsMinus1 > 0 && slice.mPps->mSliceGroupMapType >= 3
            && slice.mPps->mSliceGroupMapType <= 5)
        {
            unsigned int picSizeInMapUnits = (slice.mSps->mPicWidthInMbs_minus1 + 1)
                * (slice.mSps->mPicHeightInMapUnitsMinus1 + 1);
            unsigned int temp = picSizeInMapUnits / (slice.mPps->mSliceGroupChangeRateMinus1 + 1);
            unsigned int len1;

            len1 = ceilLog2(temp);
            slice.mSliceGroupChangeCycle = bitstr.readBits(len1);
        }
    }
    if (naluType == AVCNalUnitType::CODED_SLICE_DPAR_A)
    {
        slice.mSliceId = bitstr.readExpGolombCode();
    }
    if ((naluType == AVCNalUnitType::CODED_SLICE_DPAR_B) || (naluType == AVCNalUnitType::CODED_SLICE_DPAR_C))
    {
        slice.mSliceId = bitstr.readExpGolombCode();
        if (slice.mSps->mSeparateColourPlaneFlag)
            slice.mColorPlaneId = bitstr.readBits(2);
        if (slice.mPps->mRedundantPicCntPresentFlag)
            slice.mRedundantPicCnt = bitstr.readExpGolombCode();
    }

    return 0;
}

// Read next NAL unit, startcode assumed in the beginning.
AvcParser::AVCNalUnitType AvcParser::readNextNalUnit(vector<uint8_t>& nalUnit)
{
    unsigned int numZeros = 0;
    bool startCodeFound = false;
    unsigned int startCodeLen = 0;
    unsigned int nextChar;

    if (mInFile.peek() == EOF)
    {
        return AVCNalUnitType::INVALID;
    }

    nextChar = mInFile.get();
    startCodeLen++;
    while (!mInFile.eof() && nextChar == 0)
    {
        nalUnit.push_back(0);
        nextChar = mInFile.get();
        startCodeLen++;
    }
    nalUnit.push_back((uint8_t)nextChar);

    const size_t startPos = mInFile.tellg();

    // Copy nal data to output vector while checking for the next start code.
    unsigned int readBytes = 0;
    while (!startCodeFound)
    {
        nextChar = mInFile.get();
        if (mInFile.eof())
        {
            break;
        }
        if (nextChar == 0)
        {
            numZeros++;
        }
        else if (numZeros >= 2 && nextChar == 1)
        {
            startCodeFound = true;
            break;
        }
        else
        {
            numZeros = 0;
        }
        ++readBytes;
    }

    if (readBytes > 0)
    {
        if (startCodeFound)
        {
            readBytes -= numZeros;
        }
        if (mInFile.eof())
        {
            mInFile.clear();
        }
        mInFile.seekg(startPos);
        const size_t nalUnitOldSize = nalUnit.size();
        nalUnit.resize(nalUnitOldSize + readBytes);
        mInFile.read(reinterpret_cast<char*>(nalUnit.data() + nalUnitOldSize), readBytes);
    }
    else
    {
        throw std::runtime_error("No data in NAL unit after start code?");
    }

    return AVCNalUnitType((nalUnit[startCodeLen] >> 1) & 0x1f);  // NAL unit type
}

AvcParser::AVCNalUnitType AvcParser::getAVCNalUnitType(const vector<uint8_t>& nalUnit)
{
    // Skip start code
    int i = 0;
    while (nalUnit[i] == 0)
    {
        ++i;
    }
    assert(nalUnit[i] == 0x01);
    ++i;

    return AVCNalUnitType((nalUnit[i] >> 1) & 0x1f);
}

bool AvcParser::isFirstVclNaluInPic(const vector<uint8_t>& nalUnit)
{
    AVCNalUnitType naluType;
    NalUnitHeader naluHeader;
    vector<uint8_t> nalUnitRBSP;

    convertByteStreamToRBSP(nalUnit, nalUnitRBSP);
    BitStream bitstr(nalUnitRBSP);

    parseNalUnitHeader(bitstr, naluHeader);

    naluType = naluHeader.mAVCNalUnitType;

    if (isVclNaluType(naluType))
    {
        SliceHeader sliceHeader = {};
        parseSliceHeader(bitstr, sliceHeader, naluHeader.mAVCNalUnitType, naluHeader.mNuhRefIdc);

        /// @todo This code currently assumes 1 slice per AU and arbitrary slice order is not allowed.
        if (sliceHeader.mFirstMbInSlice == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

void AvcParser::convertByteStreamToRBSP(const vector<uint8_t>& byteStr, vector<uint8_t>& dest)
{
    const unsigned int numBytesInNalUnit = byteStr.size();

    // This is a reasonable guess, as the result vector can not be larger than the original.
    dest.reserve(numBytesInNalUnit);

    // Find start code.
    unsigned int i = 0;
    while ((i < (numBytesInNalUnit - 3)) && !(byteStr[i] == 0 && byteStr[i + 1] == 0 && byteStr[i + 2] == 1))
    {
        ++i;
    }

    i += 3;  // Skip start code.

    // Copy NALU header.
    static const size_t NALU_HEADER_LENGTH = 2;
    dest.insert(dest.end(), byteStr.cbegin() + i, byteStr.cbegin() + i + NALU_HEADER_LENGTH);
    i += NALU_HEADER_LENGTH;

    // Copy rest of the data while removing start code emulation prevention bytes.
    enum class State
    {
        COPY_DATA,
        ONE_ZERO_FOUND,
        TWO_ZEROS_FOUND
    };
    State state = State::COPY_DATA;
    size_t copyStartOffset = i;
    for (; i < numBytesInNalUnit; ++i)
    {
        const unsigned int byte = byteStr[i];
        switch (state)
        {
        case State::COPY_DATA:
            if (byte != 0)
                state = State::COPY_DATA;
            else
                state = State::ONE_ZERO_FOUND;
            break;

        case State::ONE_ZERO_FOUND:
            if (byte != 0)
                state = State::COPY_DATA;
            else
                state = State::TWO_ZEROS_FOUND;
            break;

        case State::TWO_ZEROS_FOUND:
            // Sequence of 0x000003 means that 0x03 is the emulation prevention byte.
            if (byte == 0x03)
            {
                // Skip copying 0x03.
                dest.insert(dest.end(), byteStr.cbegin() + copyStartOffset, byteStr.cbegin() + i);
                copyStartOffset = i + 1;
                // Continue byte stream copying.
                state = State::COPY_DATA;
            }
            else if (byte == 0)
                state = State::TWO_ZEROS_FOUND;
            else
                state = State::COPY_DATA;
            break;
        }
    }
    dest.insert(dest.end(), byteStr.cbegin() + copyStartOffset, byteStr.cend());
}

bool AvcParser::parseSPSConfigValues(const std::vector<uint8_t>& byteStr,
                                     SPSConfigValues& configValues)
{
    // Convert data for parsing.
    std::vector<uint8_t> sps;
    convertByteStreamToRBSP(byteStr, sps);
    BitStream bitstr(sps);

    // Parse NALU header.
    NalUnitHeader naluHeader;
    parseNalUnitHeader(bitstr, naluHeader);
    assert(naluHeader.mAVCNalUnitType == AVCNalUnitType::SPS); // Check NAL type.

    // Parse SPS data.
    SequenceParameterSet spsData =
    { };
    int result = AvcParser::parseSPS(bitstr, spsData);
    assert(result == 0); // Check that parsing was successful.

    // Store data.
    configValues.mChromaFormat = spsData.mChromaFormatIdc;
    configValues.mBitDepthLumaMinus8 = spsData.mBitDepthLumaMinus8;
    configValues.mBitDepthChromaMinus8 = spsData.mBitDepthChromaMinus8;

    // Calculate and store picture width and height.
    configValues.mPicWidthPx = static_cast<uint16_t>((spsData.mPicWidthInMbs_minus1 + 1) * 16
        - spsData.mFrameCropLeftOffset * 2
        - spsData.mFrameCropRightOffset * 2);

    configValues.mPicHeightPx = static_cast<uint16_t>((2 - spsData.mFrameMbsOnlyFlag)
        * (spsData.mPicHeightInMapUnitsMinus1 + 1) * 16
        - spsData.mFrameCropTopOffset * 2 - spsData.mFrameCropBottomOffset * 2);

    return true;
}

bool AvcParser::moreRBSPData(BitStream& bitstr)
{
    if (bitstr.numBytesLeft() >= 1) // There are at least 9 bits left.
        return true;
    else
        return false;
}

int AvcParser::parseDecRefPicMarking(BitStream& bitstr, SliceHeader& slice)
{
    if (slice.mIdrPicFlag)
    {
        slice.mNoOutputOfPriorPicsFlag = bitstr.readBits(1);
        slice.mLongTermReferenceFlag = bitstr.readBits(1);
    }
    else
    {
        slice.mAdaptiveRefPicMarkingModeFlag = bitstr.readBits(1);
        if (slice.mAdaptiveRefPicMarkingModeFlag)
        {
            unsigned int memoryManagementControlOperation = 0;
            slice.mMMCOCmdList.clear();
            do
            {
                SliceMMCO sMMCO {};
                memoryManagementControlOperation = bitstr.readExpGolombCode();
                sMMCO.mMemoryManagementControlOperation = memoryManagementControlOperation;
                if ((sMMCO.mMemoryManagementControlOperation == 1) || (sMMCO.mMemoryManagementControlOperation == 3))
                    sMMCO.mDifferenceOfPicNumsMinus1 = bitstr.readExpGolombCode();
                if (sMMCO.mMemoryManagementControlOperation == 2)
                    sMMCO.mLongTermPicNum = bitstr.readExpGolombCode();
                if ((sMMCO.mMemoryManagementControlOperation == 3) || (sMMCO.mMemoryManagementControlOperation == 6))
                    sMMCO.mLongTermFrameIdx = bitstr.readExpGolombCode();
                if (sMMCO.mMemoryManagementControlOperation == 4)
                    sMMCO.mMaxLongTermFrameIdxPlus1 = bitstr.readExpGolombCode();

                if (sMMCO.mMemoryManagementControlOperation == 5)
                    slice.mHasMMCO5 = true;
                else
                    slice.mHasMMCO5 = false;

                slice.mMMCOCmdList.push_back(sMMCO);
            } while (memoryManagementControlOperation != 0);
        }
    }

    return 0;
}

void AvcParser::dpbSetSize(Dpb& dpb, const int dpbSize)
{
    // If new DPB size is smaller than old, close any unneeded frame buffers.
    if (dpbSize < dpb.size)
    {
        std::list<Picture>::iterator range_begin = dpb.mDecodedPicBuffer.begin();
        std::list<Picture>::iterator range_end = dpb.mDecodedPicBuffer.begin();
        std::advance(range_begin, dpbSize);
        std::advance(range_end, dpb.size - 1);
        dpb.mDecodedPicBuffer.erase(range_begin, range_end);
    }
    dpb.size = dpbSize;

    int i = 0;
    for (auto& iter : dpb.mDecodedPicBuffer)
    {
        iter.mPicID = i + 1;
        i++;
    }
}

AvcParser::Picture* AvcParser::dpbGetNextOutputPic(Dpb& dpb, int& dpbHasIDRorMMCO5)
{
    Picture * tmpPic = nullptr;
    int i;

    // Find first output pic in decoding order.
    std::list<Picture>::iterator iter = dpb.mDecodedPicBuffer.begin();
    std::advance(iter, dpb.fullness - 1);

    for (i = dpb.fullness - 1; i >= 0; i--)
    {
        if (iter->mForOutput)
        {
            tmpPic = &(*iter);
            break;
        }
        iter--;
    }

    dpbHasIDRorMMCO5 = 0;

    if (i >= 0 && (iter->mIsIDR || iter->mHasMMCO5))
    {
        i--;
        iter--;
    }

    // Find picture with lowest poc. Stop search if IDR or MMCO5 is found.
    for (; i >= 0; i--)
    {
        if (iter->mIsIDR || iter->mHasMMCO5)
        {
            dpbHasIDRorMMCO5 = 1;
            break;
        }
        if (iter->mForOutput && iter->mPoc < tmpPic->mPoc)
            tmpPic = &(*iter);
        iter--;
    }

    return tmpPic;
}

int AvcParser::dpbStorePicture(Dpb& dpb, Picture** currPicPtr)
{
    Picture *tmpFrm;
    Picture *currPic;
    int numFrm;
    int i;
    int freeBufferFound;
    int numOutput;
    int dpbHasIDRorMMCO5;

    currPic = *currPicPtr;

    std::list<Picture>::iterator iter = dpb.mDecodedPicBuffer.begin();
    std::advance(iter, dpb.fullness - 1);

    // If the current picture is a reference picture and DPB is already full of
    // reference pictures, we cannot insert current picture to DPB. Therefore,
    // we force one reference picture out of DPB to make space for current
    // picture. This situation can only happen with corrupted bitstreams.
    if (currPic->mRefType != FRM_NON_REF_PIC &&
        (dpb.numShortTermPics + dpb.numLongTermPics) == dpb.size)
    {
        if (dpb.numLongTermPics == dpb.size)
            iter->mRefType = FRM_NON_REF_PIC;
        else
            dpbMarkLowestShortTermPicAsNonRef(dpb);
    }

    // Remove unused frames from dpb.
    std::list<Picture>::iterator iter2 = dpb.mDecodedPicBuffer.begin();

    numFrm = 0;
    iter = dpb.mDecodedPicBuffer.begin();

    for (i = 0; i < dpb.fullness; i++)
    {
        if (iter->mRefType != FRM_NON_REF_PIC || iter->mForOutput)
        {
            iter++;
            numFrm++;
        }
        else
        {
            // Put unsused pics to the end of the list.
            iter2 = iter;
            iter++;
            dpb.mDecodedPicBuffer.splice(dpb.mDecodedPicBuffer.end(), dpb.mDecodedPicBuffer, iter2);
        }
    }

    dpb.fullness = numFrm;

    // Try to store current pic to dpb. If there is no room in dpb, we have to
    // output some pictures to make room.

    // Case 1: if current pic is unused, it won't be stored in dpb.
    if (currPic->mRefType == FRM_NON_REF_PIC && !currPic->mForOutput)
    {
        return 0;
    }

    // Case 2: if there is space left in dpb, store current pic.
    if (dpb.fullness < dpb.size)
    {

        tmpFrm = currPic; // Unused frame.

        tmpFrm->mWidth = (*currPicPtr)->mWidth;
        tmpFrm->mHeight = (*currPicPtr)->mHeight;

        int picID = tmpFrm ? tmpFrm->mPicID : dpb.fullness + 1;
        tmpFrm->mPicID = picID;

        // Always insert new frame to the beginning of the list.
        dpb.mDecodedPicBuffer.push_front(*tmpFrm);
        dpb.fullness++;
        if (dpb.mDecodedPicBuffer.size() > (unsigned int)dpb.fullness)
            do
            {
                dpb.mDecodedPicBuffer.pop_back();
            } while (dpb.mDecodedPicBuffer.size() > (unsigned int)dpb.fullness);

        if (tmpFrm->mRefType == FRM_SHORT_TERM_PIC)
            dpb.numShortTermPics++;
        else if (tmpFrm->mRefType == FRM_LONG_TERM_PIC)
            dpb.numLongTermPics++;

        // No pictures in ouput queue.
        return 0;
    }

    // Case 3: output pictures with bumping process until there is space in dpb or
    // current pic is non-reference and has lowest poc.
    freeBufferFound = 0;
    numOutput = 0;
    while (!freeBufferFound)
    {

        // Next picture to output is a picture having smallest POC in DPB.
        tmpFrm = dpbGetNextOutputPic(dpb, dpbHasIDRorMMCO5);

        // Current picture is output if it is non-reference picture having
        // smaller POC than any of the pictures in DPB.
        if (currPic->mRefType == FRM_NON_REF_PIC &&
            (tmpFrm == 0 || (!dpbHasIDRorMMCO5 && currPic->mPoc < tmpFrm->mPoc)))
        {
            // Output current picture.
            currPic->mForOutput = 0;
            mOutputPictureList.push_back(*currPic);
            numOutput++;

            freeBufferFound = 1;
        }
        else
        {
            // Output picture that we got from DPB.
            if (tmpFrm != 0)
            {
                tmpFrm->mForOutput = 0;
                mOutputPictureList.push_back(*tmpFrm);
                numOutput++;

                if (tmpFrm->mRefType == FRM_NON_REF_PIC)
                    freeBufferFound = 1;
            }
            else
                // Can only happen with erroneus bitstreams.
                freeBufferFound = 1;
        }
    }

    return numOutput;
}

void AvcParser::dpbUpdatePicNums(Dpb& dpb, const int frameNum, const int maxFrameNum)
{
    std::list<Picture>::iterator refPicListIter = dpb.mDecodedPicBuffer.begin();

    for (int i = 0; i < dpb.fullness; i++)
    {
        if (refPicListIter->mRefType == FRM_SHORT_TERM_PIC)
        {
            // Short term pictures
            if (refPicListIter->mFrameNum > frameNum)
                refPicListIter->mPicNum = refPicListIter->mFrameNum - maxFrameNum;
            else
                refPicListIter->mPicNum = refPicListIter->mFrameNum;
        }
        else if (refPicListIter->mRefType == FRM_LONG_TERM_PIC)
            // Long term pictures
            refPicListIter->mLongTermPicNum = refPicListIter->mLongTermFrmIdx;

        refPicListIter++;
    }
}

void AvcParser::dpbMarkAllPicsAsNonRef(Dpb& dpb)
{
    int i;
    std::list<Picture>::iterator refPicListIter = dpb.mDecodedPicBuffer.begin();

    // Mark all pictures as not used for reference.
    for (i = 0; i < dpb.fullness; i++)
    {
        refPicListIter->mRefType = FRM_NON_REF_PIC;
        refPicListIter++;
    }

    dpb.numShortTermPics = 0;
    dpb.numLongTermPics = 0;
}

void AvcParser::dpbMarkLowestShortTermPicAsNonRef(Dpb& dpb)
{
    int picIdx;
    int i;

    std::list<Picture>::iterator refPicListIter = dpb.mDecodedPicBuffer.end();
    refPicListIter--;
    std::list<Picture>::iterator refPicListIter2 = dpb.mDecodedPicBuffer.begin();

    /* Find short term pic with lowest picNum */
    picIdx = -1;
    std::advance(refPicListIter2, picIdx);

    for (i = dpb.fullness - 1; i >= 0; i--)
    {
        if (refPicListIter->mRefType == FRM_SHORT_TERM_PIC)
        {
            if ((picIdx < 0) || (refPicListIter->mPicNum < refPicListIter2->mPicNum))
            {
                picIdx = i;
                refPicListIter2 = dpb.mDecodedPicBuffer.begin();
                std::advance(refPicListIter2, picIdx);
            }

        }
        refPicListIter--;
    }

    // Mark short term pic with lowest picNum as not reference picture.
    if (picIdx >= 0)
    {
        refPicListIter2->mRefType = FRM_NON_REF_PIC;
        dpb.numShortTermPics--;
    }
}

int AvcParser::dpbMarkShortTermPicAsNonRef(Dpb& dpb, const int picNum)
{
    std::list<Picture>::iterator refPicListIter = dpb.mDecodedPicBuffer.begin();

    for (int i = 0; i < dpb.fullness; i++)
    {
        if (refPicListIter->mRefType == FRM_SHORT_TERM_PIC &&
            refPicListIter->mPicNum == picNum)
        {
            refPicListIter->mRefType = FRM_NON_REF_PIC;
            dpb.numShortTermPics--;
            return 0;
        }
        refPicListIter++;
    }

    return -1;
}

int AvcParser::dpbMarkLongTermPicAsNonRef(Dpb& dpb, const int longTermPicNum)
{
    std::list<Picture>::iterator refPicListIter = dpb.mDecodedPicBuffer.begin();

    for (int i = 0; i < dpb.fullness; i++)
    {
        if (refPicListIter->mRefType == FRM_LONG_TERM_PIC &&
            refPicListIter->mLongTermPicNum == longTermPicNum)
        {
            refPicListIter->mRefType = FRM_NON_REF_PIC;
            dpb.numLongTermPics--;
            return 0;
        }
        refPicListIter++;
    }

    return -1;
}

void AvcParser::dpbVerifyLongTermFrmIdx(Dpb& dpb, const int longTermFrmIdx)
{
    std::list<Picture>::iterator refPicListIter = dpb.mDecodedPicBuffer.begin();

    // Check if longTermFrmIdx is already in use.
    for (int i = 0; i < dpb.fullness; i++)
    {
        if (refPicListIter->mRefType == FRM_LONG_TERM_PIC &&
            refPicListIter->mLongTermFrmIdx == longTermFrmIdx)
        {
            refPicListIter->mRefType = FRM_NON_REF_PIC;
            dpb.numLongTermPics--;
            break;
        }
        refPicListIter++;
    }
}

int AvcParser::dpbMarkShortTermPicAsLongTerm(Dpb& dpb, const int picNum, const int longTermFrmIdx)
{
    // To avoid duplicate of longTermFrmIdx
    dpbVerifyLongTermFrmIdx(dpb, longTermFrmIdx);

    std::list<Picture>::iterator refPicListIter = dpb.mDecodedPicBuffer.begin();

    // Mark pic with picNum as long term and assign longTermFrmIdx to it.
    for (int i = 0; i < dpb.fullness; i++)
    {
        if (refPicListIter->mRefType == FRM_SHORT_TERM_PIC &&
            refPicListIter->mPicNum == picNum)
        {
            refPicListIter->mRefType = FRM_LONG_TERM_PIC;
            refPicListIter->mLongTermFrmIdx = longTermFrmIdx;
            dpb.numShortTermPics--;
            dpb.numLongTermPics++;
            return 0;
        }
        refPicListIter++;
    }

    return -1;
}

void AvcParser::dpbSetMaxLongTermFrameIdx(Dpb& dpb, const int maxLongTermFrmIdxPlus1)
{
    std::list<Picture>::iterator refPicListIter = dpb.mDecodedPicBuffer.begin();

    for (int i = 0; i < dpb.fullness; i++)
    {
        if (refPicListIter->mRefType == FRM_LONG_TERM_PIC &&
            refPicListIter->mLongTermFrmIdx > maxLongTermFrmIdxPlus1 - 1)
        {
            refPicListIter->mRefType = FRM_NON_REF_PIC;
            dpb.numLongTermPics--;
        }
        refPicListIter++;
    }

    dpb.maxLongTermFrameIdx = maxLongTermFrmIdxPlus1 - 1;
}

int AvcParser::finishCurrentPic()
{
    SliceHeader *slice;
    Picture *currPic;
    int numOutput;

    slice = mSliceList.at(mSliceList.size() - 1); // current slice...
    currPic = mCurrentPic;
    decRefPicMarking();

    // After the decoding of the current picture and the processing of the
    // memory management control operations a picture including
    // a memory_management_control_operation equal to 5 shall be inferred
    // to have had frame_num equal to 0 for all subsequent use in the decoding
    // process.
    if (slice->mHasMMCO5)
        currPic->mFrameNum = slice->mFrameNum = 0;

    if (slice->mNuhRefIdc != 0)
    {
        mPreviousRefPicPoc = currPic->mPoc;
    }

    // This action is not defined in the H.264/AVC standard, but reference SW does it.
    if (currPic->mHasMMCO5)
        currPic->mPoc = 0;

    // Try to store current picture to dpb.
    numOutput = dpbStorePicture(mDPB, &mCurrentPic);
    // If numOutput != 0, picture was not stored.

    // recoBuf might have changed in dpbStorePicture()
    currPic = mCurrentPic;

    if (numOutput != 0)
    {
        // numOutput != 0 implies that pictures were output from dpb.
        mOutputQueuePos = 0;
        mNumQueuedOutputPics = numOutput;

        // Picture was not stored so we have to store it later.
        mIsDpbStorePending = true;
    }
    else
        mIsDpbStorePending = false;

    mPreviousFrameNum = slice->mFrameNum;
    mPreviousFrameNumOffset = mFrameNumOffset;

    mPrevPicHasMMCO5 = slice->mHasMMCO5;

    // prevRefFrameNum, prevPocLsb and prevPocMsb for latest reference picture
    if (slice->mNuhRefIdc != 0)
    {
        mPreviousRefFrameNum = slice->mFrameNum;
        mPrevPicOrderCntLsb = slice->mPicOrderCntLsb;
        mPrevPicOrderCntMsb = mPicOrderCntMsb;
    }

    mIsCurrPicFinished = true;

    return numOutput;
}

int AvcParser::decRefPicMarking()
{
    SliceHeader *slice;
    Picture *recoBuf;

    slice = mSliceList.at(mSliceList.size() - 1);
    recoBuf = mCurrentPic;

    recoBuf->mRefType = FRM_SHORT_TERM_PIC;
    recoBuf->mFrameNum = slice->mFrameNum;
    recoBuf->mHasMMCO5 = slice->mHasMMCO5;
    recoBuf->mIsIDR = slice->mIdrPicFlag;

    if (slice->mIdrPicFlag)
    {
        recoBuf->mIdrPicID = slice->mIdrPicId;

        // All reference frames are marked as non-reference frames.
        dpbMarkAllPicsAsNonRef(mDPB);

        // Set reference type for current picture.
        if (!slice->mLongTermReferenceFlag)
        {
            mDPB.maxLongTermFrameIdx = -1;
        }
        else
        {
            recoBuf->mRefType = FRM_LONG_TERM_PIC;
            recoBuf->mLongTermFrmIdx = 0;
            mDPB.maxLongTermFrameIdx = 0;
        }
    }
    else if (slice->mNuhRefIdc != 0)
    {
        if (!slice->mAdaptiveRefPicMarkingModeFlag)
            return slidingWindowDecRefPicMarking();
        else
            return adaptiveDecRefPicMarking();
    }
    else
        recoBuf->mRefType = FRM_NON_REF_PIC;

    return 0;
}

int AvcParser::slidingWindowDecRefPicMarking()
{
    int numRefPics = mDPB.numShortTermPics + mDPB.numLongTermPics;

    // If dpb contains maximum number of reference pitures allowed, short
    // term reference picture with lowest picture number is removed.
    if (numRefPics == mDPB.maxNumRefFrames)
    {
        if (mDPB.numShortTermPics == 0)
        {
            return -1;
        }

        dpbMarkLowestShortTermPicAsNonRef(mDPB);
    }

    return 0;
}

int AvcParser::adaptiveDecRefPicMarking()
{
    std::vector<SliceMMCO> mmcoCmdList;
    int currPicNum, picNumX;
    int i = 0;

    SliceHeader* slice = mSliceList.at(mSliceList.size() - 1);
    currPicNum = slice->mFrameNum;
    mmcoCmdList = slice->mMMCOCmdList;
    SliceMMCO* sliceMMCO;

    do
    {
        sliceMMCO = &(mmcoCmdList.at(i));
        switch (sliceMMCO->mMemoryManagementControlOperation)
        {
        case 1:
            picNumX = currPicNum - (sliceMMCO->mDifferenceOfPicNumsMinus1 + 1);
            if (dpbMarkShortTermPicAsNonRef(mDPB, (int)picNumX) < 0)
                return -1;
            break;
        case 2:
            if (dpbMarkLongTermPicAsNonRef(mDPB, (int)(sliceMMCO->mLongTermPicNum)) < 0)
                return -1;
            break;
        case 3:
            picNumX = currPicNum - (sliceMMCO->mDifferenceOfPicNumsMinus1 + 1);
            if (dpbMarkShortTermPicAsLongTerm(mDPB, picNumX, sliceMMCO->mLongTermFrameIdx) < 0)
                return -1;
            break;
        case 4:
            dpbSetMaxLongTermFrameIdx(mDPB, sliceMMCO->mMaxLongTermFrameIdxPlus1);
            break;
        case 5:
            dpbMarkAllPicsAsNonRef(mDPB);
            mDPB.maxLongTermFrameIdx = -1;
            break;
        case 6:
            /* To avoid duplicate of longTermFrmIdx */
            dpbVerifyLongTermFrmIdx(mDPB, sliceMMCO->mLongTermFrameIdx);

            mCurrentPic->mRefType = FRM_LONG_TERM_PIC;
            mCurrentPic->mLongTermFrmIdx = sliceMMCO->mLongTermFrameIdx;
            break;
        }
        i++;
    } while (sliceMMCO->mMemoryManagementControlOperation != 0 && i < MAX_NUM_OF_MMCO_OPS);

    return 0;
}

int AvcParser::sliceInitRefPicListBslice(const int pocType, const int slicePoc)
{
    int numRef, numShort;
    Picture *refTmp;
    int i, j;

    std::list<Picture>::iterator iter = mDPB.mDecodedPicBuffer.begin();

    // Select the reference pictures from the DPB.
    j = 0;
    // Put short term pictures first in the list./
    for (i = 0; i < mDPB.fullness; i++)
    {
        if (iter->mRefType == FRM_SHORT_TERM_PIC && !(pocType == 0 && iter->mNonExisting))
        {
            mRefPicList1.push_back(&(*iter));
            mRefPicList0.push_back(&(*iter));
            j++;
        }
        iter++;
    }
    numShort = j;

    iter = mDPB.mDecodedPicBuffer.begin();
    // Put long term pictures after the short term pictures.
    for (i = 0; i < mDPB.fullness; i++)
    {
        if (iter->mRefType == FRM_LONG_TERM_PIC)
        {
            mRefPicList0.push_back(&(*iter));
            j++;
        }
        iter++;
    }
    numRef = j;

    // Initialisation process for reference picture lists.

    // Order both lists according to POC.

    // list0,      ref frames with POC < slicePoc in descending order
    // followed by ref frames with POC > slicePoc in ascending order
    for (i = 0; i < numShort - 1; i++)
    {
        for (j = i + 1; j < numShort; j++)
        {
            // Switch for 3 cases:
            //   POC < slicePoc, in descending order
            //   POC > slicePoc, in ascending order
            //   POC < slicePoc, ahead of POC > slicePoc
            if ((mRefPicList0[j]->mPoc < slicePoc &&
                mRefPicList0[i]->mPoc < mRefPicList0[j]->mPoc) ||
                (mRefPicList0[j]->mPoc > slicePoc &&
                    mRefPicList0[i]->mPoc > mRefPicList0[j]->mPoc) ||
                (mRefPicList0[i]->mPoc > slicePoc &&
                    mRefPicList0[j]->mPoc < slicePoc))
            {
                // Exchange refList0[i] and refList0[j]
                refTmp = mRefPicList0[i];
                mRefPicList0[i] = mRefPicList0[j];
                mRefPicList0[j] = refTmp;
            }
        }
    }

    // list1,      ref frames with POC > slicePoc in ascending order
    // followed by ref frames with POC < slicePoc in descending order
    for (i = 0; i < numShort - 1; i++)
    {
        for (j = i + 1; j < numShort; j++)
        {
            // Switch for 3 cases:
            //   POC < slicePoc, in descending order
            //   POC > slicePoc, in ascending order
            //   POC > slicePoc, ahead of POC < slicePoc
            if ((mRefPicList1[j]->mPoc < slicePoc &&
                mRefPicList1[i]->mPoc < mRefPicList1[j]->mPoc) ||
                (mRefPicList1[j]->mPoc > slicePoc &&
                    mRefPicList1[i]->mPoc > mRefPicList1[j]->mPoc) ||
                (mRefPicList1[i]->mPoc < slicePoc &&
                    mRefPicList1[j]->mPoc > slicePoc))
            {
                // Exchange refPicList1[i] and refPicList1[j].
                refTmp = mRefPicList1[i];
                mRefPicList1[i] = mRefPicList1[j];
                mRefPicList1[j] = refTmp;
            }
        }
    }

    // Sort long term pictures in the order of ascending longTermPicNum.
    for (i = numShort; i < numRef; i++)
    {
        for (j = i + 1; j < numRef; j++)
        {
            if (mRefPicList0[i]->mLongTermPicNum > mRefPicList0[j]->mLongTermPicNum)
            {
                // Exchange refPicList[i] and refPicList[j].
                refTmp = mRefPicList0[i];
                mRefPicList0[i] = mRefPicList0[j];
                mRefPicList0[j] = refTmp;
            }
        }
    }

    // Copy the long-term ref frame pointers from list0 to list1.
    for (i = numShort; i < numRef; i++)
        mRefPicList1[i] = mRefPicList0[i];

    // If refPicList and refPicList1 are identical,
    // exchange refPicList1[0] and refPicList1[0].
    if (numRef > 1)
    {
        for (i = 0; i < numRef; i++)
        {
            if (mRefPicList0[i] != mRefPicList1[i])
                break;
        }

        if (i == numRef)
        {
            refTmp = mRefPicList1[0];
            mRefPicList1[0] = mRefPicList1[1];
            mRefPicList1[1] = refTmp;
        }
    }

    return numRef;
}

int AvcParser::sliceFixRefPicList(std::vector<Picture*>& RefPicList,
                                  int numExistingRefFrames,
                                  const int numRefFrames,
                                  const unsigned int width,
                                  const unsigned int height)
{
    int i;

    std::list<Picture>::iterator iter = mDPB.mDecodedPicBuffer.begin();

    if (numExistingRefFrames == 0)
    {
        /* Try to find any picture in DPB, even non-reference frame */
        for (i = 0; i < mDPB.size; i++)
        {
            if (iter != mDPB.mDecodedPicBuffer.end() && iter->mWidth == width && iter->mHeight == height)
                break;
            iter++;
        }

        if (i < mDPB.size)
        {
            RefPicList.push_back(&(*iter));
            numExistingRefFrames = 1;
        }
        else
            return 0;
    }

    // Duplicate last entry of the reference frame list so that list becomes full.
    for (i = numExistingRefFrames; i < (0 * numRefFrames + DPB_MAX_SIZE); i++)
        RefPicList.push_back(RefPicList[numExistingRefFrames - 1]);

    return 1;
}

int AvcParser::refPicListReordering(SliceHeader& slice, std::vector<Picture*>& RefPicList, const int numRefPicActive,
                                    std::vector<RefPicListsModification>& reorderCmdList)
{
    int i;
    int reordering_of_pic_nums_idc, longTermPicNum;
    int absDiffPicNum;
    int refIdx;
    int currPicNum, picNumPred, picNumNoWrap;
    int maxPicNum, picNum;
    int cmdNum;
    int cIdx, nIdx;

    // 3. Reordering process for reference picture list.

    maxPicNum = slice.mMaxFrameNum; // for frame coding only
    currPicNum = slice.mFrameNum;
    picNumPred = currPicNum;
    refIdx = 0;
    cmdNum = 0;

    std::list<Picture>::iterator iter = mDPB.mDecodedPicBuffer.begin();

    do
    {
        reordering_of_pic_nums_idc = reorderCmdList[cmdNum].mModificationOfPicNumsIdc;

        if (reordering_of_pic_nums_idc == 0 || reordering_of_pic_nums_idc == 1)
        {

            // Reorder short-term ref pic  -subclause 8.2.4.3.1

            absDiffPicNum = reorderCmdList[cmdNum].mAbsDiffPicNumMinus1 + 1;

            // Derive picNumNoWrap
            if (reordering_of_pic_nums_idc == 0)
            {
                if (picNumPred - absDiffPicNum < 0)
                    picNumNoWrap = picNumPred - absDiffPicNum + maxPicNum;
                else
                    picNumNoWrap = picNumPred - absDiffPicNum;
            }
            else
            {
                // Reordering_of_pic_nums_idc == 1
                if (picNumPred + absDiffPicNum >= maxPicNum)
                    picNumNoWrap = picNumPred + absDiffPicNum - maxPicNum;
                else
                    picNumNoWrap = picNumPred + absDiffPicNum;
            }

            // Derive picNum.
            if (picNumNoWrap > currPicNum)
                picNum = picNumNoWrap - maxPicNum;
            else
                picNum = picNumNoWrap;

            iter = mDPB.mDecodedPicBuffer.begin();
            // Find short term picture with picture number picNum.
            for (i = 0; i < mDPB.fullness; i++)
            {
                if (!iter->mNonExisting &&
                    iter->mRefType == FRM_SHORT_TERM_PIC &&
                    iter->mPicNum == picNum)
                    break;
                iter++;
            }

            /* If picNum was not found */
            if (i == mDPB.fullness)
            {
                return -1;
            }

            // Shift remaining pictures later in the list.
            for (cIdx = numRefPicActive; cIdx > refIdx; cIdx--)
                RefPicList[cIdx] = RefPicList[cIdx - 1];

            // Place picture with number picNum into the index position refIdx.
            RefPicList[refIdx++] = &(*iter);

            // Remove duplicate of the inserted picture.
            nIdx = refIdx;
            for (cIdx = refIdx; cIdx <= numRefPicActive; cIdx++)
                if (RefPicList[cIdx]->mRefType == FRM_LONG_TERM_PIC || RefPicList[cIdx]->mPicNum != picNum)
                    RefPicList[nIdx++] = RefPicList[cIdx];

            picNumPred = picNumNoWrap;
        }

        else if (reordering_of_pic_nums_idc == 2)
        {
            // Reorder long-term ref pic  -subclause 8.2.4.3.2

            // Get long-term picture number.
            longTermPicNum = reorderCmdList[cmdNum].mLongTermPicNum;

            iter = mDPB.mDecodedPicBuffer.begin();
            // Find long-term picture with picture number longTermPicNum.
            for (i = 0; i < mDPB.fullness; i++)
            {
                if (iter->mRefType == FRM_LONG_TERM_PIC &&
                    iter->mLongTermPicNum == longTermPicNum)
                    break;
                iter++;
            }

            if (i == mDPB.fullness)
            {
                // Something is wrong.
                return -1;
            }

            // Shift remaining pictures later in the list.
            for (cIdx = numRefPicActive; cIdx > refIdx; cIdx--)
                RefPicList[cIdx] = RefPicList[cIdx - 1];

            // Place picture with number longTermPicNum into the index position refIdx.
            RefPicList[refIdx++] = &(*iter);

            // Remove duplicate of the inserted picture.
            nIdx = refIdx;
            for (cIdx = refIdx; cIdx <= numRefPicActive; cIdx++)
            {
                if (RefPicList[cIdx]->mRefType == FRM_SHORT_TERM_PIC ||
                    RefPicList[cIdx]->mLongTermPicNum != longTermPicNum)
                {
                    RefPicList[nIdx++] = RefPicList[cIdx];
                }
            }
        }

        cmdNum++;

    } while (reordering_of_pic_nums_idc != 3 && cmdNum < MAX_NUM_OF_REORDER_CMDS);

    RefPicList[numRefPicActive] = 0;

    return 0;
}

int AvcParser::sliceInitRefPicList(std::vector<Picture*>& RefPicList)
{
    int numRef, numShort;
    Picture *refTmp;
    int i, j;

    std::list<Picture>::iterator iter = mDPB.mDecodedPicBuffer.begin();
    // Select the reference pictures from the DPB.
    j = 0;
    // Put short term pictures first in the list.
    for (i = 0; i < mDPB.fullness; i++)
    {
        if (iter->mRefType == FRM_SHORT_TERM_PIC)
        {
            RefPicList.push_back(&(*iter));
            j++;
        }
        iter++;
    }
    numShort = j;

    iter = mDPB.mDecodedPicBuffer.begin();
    // Put long term pictures after the short term pictures.
    for (i = 0; i < mDPB.fullness; i++)
    {
        if (iter->mRefType == FRM_LONG_TERM_PIC)
        {
            RefPicList.push_back(&(*iter));
            j++;
        }
        iter++;
    }
    numRef = j;

    // Initialisation process for reference picture lists

    // Sort short term pictures in the order of descending picNum.
    for (i = 0; i < numShort; i++)
    {
        for (j = i + 1; j < numShort; j++)
        {
            if (RefPicList[i]->mPicNum < RefPicList[j]->mPicNum)
            {
                // Exchange refPicList[i] and refPicList[j]
                refTmp = RefPicList[i];
                RefPicList[i] = RefPicList[j];
                RefPicList[j] = refTmp;
            }
        }
    }
    // Sort long term pictures in the order of ascending longTermPicNum.
    for (i = numShort; i < numRef; i++)
    {
        for (j = i + 1; j < numRef; j++)
        {
            if (RefPicList[i]->mLongTermPicNum > RefPicList[j]->mLongTermPicNum)
            {
                // Exchange refPicList[i] and refPicList[j]
                refTmp = RefPicList[i];
                RefPicList[i] = RefPicList[j];
                RefPicList[j] = refTmp;
            }
        }
    }

    return numRef;
}

int AvcParser::initializeCurrentPicture(const unsigned int width, const unsigned int height)

{
    Picture* currPic;
    SliceHeader* slice;

    slice = mSliceList.at(mSliceList.size() - 1); // Get the current slice which was lastly added to the list.

    // (Re)initialize frame buffer for current picture if picture size has changed.

    currPic = mCurrentPic;
    if (currPic == nullptr)
        return -1; // Something is wrong.

    if (width != currPic->mWidth || height != currPic->mHeight)
    {
        currPic->mWidth = width;
        currPic->mHeight = height;
    }

    // Parameter from SPS
    currPic->mConstraintSet0Flag = slice->mSps->mConstraintSet0Flag;
    currPic->mConstraintSet1Flag = slice->mSps->mConstraintSet1Flag;
    currPic->mConstraintSet2Flag = slice->mSps->mConstraintSet2Flag;
    currPic->mProfileIdc = slice->mSps->mProfileIdc;
    currPic->mLevelIdc = slice->mSps->mLevelIdc;
    currPic->mMaxFrameNum = slice->mMaxFrameNum;
    currPic->mPocType = slice->mSps->mPicOrderCntType;

    // By default picture will be output.
    currPic->mForOutput = 1;
    currPic->mNonExisting = 0;
    currPic->mPicType = slice->mSliceType;
    currPic->mIsIDR = slice->mIdrPicFlag;
    currPic->mRefType = slice->mNuhRefIdc == 0 ? FRM_NON_REF_PIC : FRM_SHORT_TERM_PIC;

    // Get poc for current picture.
    currPic->mPoc = decodePoc(*slice);

    mIsCurrPicFinished = false;

    return 0;
}

