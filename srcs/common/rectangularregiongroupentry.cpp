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

#include "rectangularregiongroupentry.hpp"

RectangularRegionGroupEntry::RectangularRegionGroupEntry()
    : SampleGroupDescriptionEntry("trif")
    , mGroupId(0)
    , mRectRegionFlag(false)
    , mIndependentIdc(0)
    , mFullPicture(false)
    , mFilteringDisabled(false)
    , mHasDependencyList(false)
    , mHorizontalOffset(0)
    , mVerticalOffset(0)
    , mRegionWidth(0)
    , mRegionHeight(0)
{
}

std::uint32_t RectangularRegionGroupEntry::RectangularRegionGroupEntry::getSize() const
{
    BitStream tmp;
    writeEntry(tmp);
    return tmp.getSize();
}

std::uint16_t RectangularRegionGroupEntry::getGroupId() const
{
    return mGroupId;
}

void RectangularRegionGroupEntry::setGroupId(std::uint16_t groupId)
{
    mGroupId = groupId;
}

bool RectangularRegionGroupEntry::getRectRegionFlag() const
{
    return mRectRegionFlag;
}

void RectangularRegionGroupEntry::setRectRegionFlag(bool rectRegionFlag)
{
    mRectRegionFlag = rectRegionFlag;
}

std::uint8_t RectangularRegionGroupEntry::getIndependentIdc() const
{
    return mIndependentIdc;
}

void RectangularRegionGroupEntry::setIndependentIdc(std::uint8_t independentIdc)
{
    mIndependentIdc = independentIdc;
}

bool RectangularRegionGroupEntry::getFullPicture() const
{
    return mFullPicture;
}

void RectangularRegionGroupEntry::setFullPicture(bool fullPicture)
{
    mFullPicture = fullPicture;
}

bool RectangularRegionGroupEntry::getFilteringDisabled() const
{
    return mFilteringDisabled;
}

void RectangularRegionGroupEntry::setFilteringDisabled(bool filteringDisabled)
{
    mFilteringDisabled = filteringDisabled;
}

bool RectangularRegionGroupEntry::getHasDependencyList() const
{
    return mHasDependencyList;
}

void RectangularRegionGroupEntry::setHasDependencyList(bool hasDependencyList)
{
    mHasDependencyList = hasDependencyList;
}

std::uint16_t RectangularRegionGroupEntry::getHorizontalOffset() const
{
    return mHorizontalOffset;
}

void RectangularRegionGroupEntry::setHorizontalOffset(std::uint16_t horizontalOffset)
{
    mHorizontalOffset = horizontalOffset;
}

std::uint16_t RectangularRegionGroupEntry::getVerticalOffset() const
{
    return mVerticalOffset;
}

void RectangularRegionGroupEntry::setVerticalOffset(std::uint16_t verticalOffset)
{
    mVerticalOffset = verticalOffset;
}

std::uint16_t RectangularRegionGroupEntry::getRegionWidth() const
{
    return mRegionWidth;
}

void RectangularRegionGroupEntry::setRegionWidth(std::uint16_t regionWidth)
{
    mRegionWidth = regionWidth;
}

std::uint16_t RectangularRegionGroupEntry::getRegionHeight() const
{
    return mRegionHeight;
}

void RectangularRegionGroupEntry::setRegionHeight(std::uint16_t regionHeight)
{
    mRegionHeight = regionHeight;
}

std::vector<std::uint16_t> RectangularRegionGroupEntry::getDependencyRectRegionGroupIds() const
{
    return mDependencyRectRegionGroupIds;
}

void RectangularRegionGroupEntry::setDependencyRectRegionGroupIds(
    const std::vector<std::uint16_t>& dependencyRectRegionGroupIds)
{
    mDependencyRectRegionGroupIds = dependencyRectRegionGroupIds;
}

void RectangularRegionGroupEntry::writeEntry(ISOBMFF::BitStream& bitstr) const
{
    bitstr.writeBits(mGroupId, 16);
    bitstr.writeBits(mRectRegionFlag, 1);
    if (mRectRegionFlag == false)
    {
        bitstr.writeBits(0, 7);  // reserved
    }
    else
    {
        bitstr.writeBits(mIndependentIdc, 2);
        bitstr.writeBits(mFullPicture, 1);
        bitstr.writeBits(mFilteringDisabled, 1);
        bitstr.writeBits(mHasDependencyList, 1);
        bitstr.writeBits(0, 2);  // reserved
        if (mFullPicture == false)
        {
            bitstr.writeBits(mHorizontalOffset, 16);
            bitstr.writeBits(mVerticalOffset, 16);
        }
        bitstr.writeBits(mRegionWidth, 16);
        bitstr.writeBits(mRegionHeight, 16);
        if (mHasDependencyList)
        {
            bitstr.writeBits(mDependencyRectRegionGroupIds.size(), 16);
            for (const auto groupId : mDependencyRectRegionGroupIds)
            {
                bitstr.writeBits(groupId, 16);
            }
        }
    }
}

void RectangularRegionGroupEntry::parseEntry(ISOBMFF::BitStream& bitstr)
{
    mGroupId        = bitstr.readBits(16);
    mRectRegionFlag = bitstr.readBits(1);
    if (mRectRegionFlag == false)
    {
        bitstr.readBits(7);  // reserved
    }
    else
    {
        mIndependentIdc    = bitstr.readBits(2);
        mFullPicture       = bitstr.readBits(1);
        mFilteringDisabled = bitstr.readBits(1);
        mHasDependencyList = bitstr.readBits(1);
        bitstr.readBits(2);  // reserved
        if (mFullPicture == false)
        {
            mHorizontalOffset = bitstr.readBits(16);
            mVerticalOffset   = bitstr.readBits(16);
        }
        mRegionWidth  = bitstr.readBits(16);
        mRegionHeight = bitstr.readBits(16);
        if (mHasDependencyList)
        {
            const auto groupIdSize = bitstr.readBits(16);
            mDependencyRectRegionGroupIds.reserve(groupIdSize);
            for (auto i = 0u; i < groupIdSize; ++i)
            {
                mDependencyRectRegionGroupIds.push_back(bitstr.readBits(16));
            }
        }
    }
}
