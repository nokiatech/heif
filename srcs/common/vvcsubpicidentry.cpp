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

#include "vvcsubpicidentry.hpp"

VvcSubpicIdEntry::VvcSubpicIdEntry()
    : SampleGroupDescriptionEntry("spid")
    , mRectRegionFlag(false)
    , mSubPicIdContinuousFlag(true)
{
}


std::uint32_t VvcSubpicIdEntry::getSize() const
{
    BitStream tmp;
    writeEntry(tmp);
    return tmp.getSize();
}


std::vector<std::uint16_t> VvcSubpicIdEntry::getSubPicIds() const
{
    return mSubPicIds;
}

void VvcSubpicIdEntry::setSubPicIds(const std::vector<std::uint16_t>& subPicIds)
{
    mSubPicIds = subPicIds;
}

bool VvcSubpicIdEntry::getSubPicIdContinuousFlag() const
{
    return mSubPicIdContinuousFlag;
}

void VvcSubpicIdEntry::setSubPicIdContinuousFlag(const bool subPicIdContinuousFlag)
{
    mSubPicIdContinuousFlag = subPicIdContinuousFlag;
}

bool VvcSubpicIdEntry::getRectRegionFlag() const
{
    return mRectRegionFlag;
}

void VvcSubpicIdEntry::setRectRegionFlag(const bool rectRegioFlag)
{
    mRectRegionFlag = rectRegioFlag;
}

std::vector<std::uint16_t> VvcSubpicIdEntry::getGroupIds() const
{
    return mGroupdIds;
}

void VvcSubpicIdEntry::setGroupIds(const std::vector<std::uint16_t>& groupIds)
{
    mGroupdIds = groupIds;
}

void VvcSubpicIdEntry::writeEntry(ISOBMFF::BitStream& bitstr) const
{
    bitstr.writeBits(mRectRegionFlag, 1);
    bitstr.writeBits(0, 2);  // reserved
    bitstr.writeBits(mSubPicIdContinuousFlag, 1);
    bitstr.writeBits(mSubPicIds.size() - 1, 12);

    if (mSubPicIdContinuousFlag && mSubPicIds.size() != 1)
    {
        throw RuntimeError("VvcSubpicIdEntry: subpicture_id_continuous_flag set but subpic_id[] size is not 1");
    }

    for (auto i = 0u; i < mSubPicIds.size(); ++i)
    {
        if ((mSubPicIdContinuousFlag && i == 0) || !mSubPicIdContinuousFlag)
        {
            bitstr.writeBits(mSubPicIds.at(i), 16);
        }
        if (mRectRegionFlag)
        {
            bitstr.writeBits(mGroupdIds.at(i), 16);
        }
    }
}

void VvcSubpicIdEntry::parseEntry(ISOBMFF::BitStream& bitstr)
{
    mRectRegionFlag = bitstr.readBits(1);
    bitstr.readBits(2);  // reserved
    mSubPicIdContinuousFlag = bitstr.readBits(1);
    const auto numSubPics   = bitstr.readBits(12) + 1;
    mSubPicIds.reserve(numSubPics);
    for (auto i = 0u; i < numSubPics; ++i)
    {
        if ((mSubPicIdContinuousFlag && i == 0) || !mSubPicIdContinuousFlag)
        {
            mSubPicIds.push_back(bitstr.read16Bits());
        }
        if (mRectRegionFlag)
        {
            mGroupdIds.push_back(bitstr.read16Bits());
        }
    }
}
