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

#include "vvcsubpicorderentry.hpp"

VvcSubpicOrderEntry::VvcSubpicOrderEntry()
    : SampleGroupDescriptionEntry("spor")
    , mSubpicIdInfoFlag(false)
    , mSubpicIdLenMinus1(0)
    , mSubpicIdBitPos(0)
    , mStartCodeEmulFlag(false)
    , mPpsSpsSubpicIdFlag(false)
    , mPpsId(0)
    , mSpsId(0)
{
}

std::vector<std::uint16_t> VvcSubpicOrderEntry::getSubpicRefIdx() const
{
    return mSubpicRefIdx;
}

void VvcSubpicOrderEntry::setSubpicRefIdx(const std::vector<std::uint16_t>& subpicRefIdx)
{
    mSubpicRefIdx = subpicRefIdx;
}

std::uint16_t VvcSubpicOrderEntry::getSubpicIdLenMinus1() const
{
    return mSubpicIdLenMinus1;
}

void VvcSubpicOrderEntry::setSubpicIdLenMinus1(const std::uint16_t subpicIdLenMinus1)
{
    mSubpicIdLenMinus1 = subpicIdLenMinus1;
}

std::uint16_t VvcSubpicOrderEntry::getSubpicIdBitPos() const
{
    return mSubpicIdBitPos;
}

void VvcSubpicOrderEntry::setSubpicIdBitPos(const std::uint16_t subpicIdBitPos)
{
    mSubpicIdBitPos = subpicIdBitPos;
}

bool VvcSubpicOrderEntry::getStartCodeEmulFlag() const
{
    return mStartCodeEmulFlag;
}

void VvcSubpicOrderEntry::setStartCodeEmulFlag(const bool startCodeEmulFlag)
{
    mStartCodeEmulFlag = startCodeEmulFlag;
}

bool VvcSubpicOrderEntry::getPpsSpsSubpicIdFlag() const
{
    return mPpsSpsSubpicIdFlag;
}

void VvcSubpicOrderEntry::setPpsSpsSubpicIdFlag(const bool ppsSpsSubpicIdFlag)
{
    mPpsSpsSubpicIdFlag = ppsSpsSubpicIdFlag;
}

std::uint8_t VvcSubpicOrderEntry::getPpsId() const
{
    return mPpsId;
}

void VvcSubpicOrderEntry::setPpsId(const std::uint8_t ppsId)
{
    mPpsId = ppsId;
}

std::uint8_t VvcSubpicOrderEntry::getSpsId() const
{
    return mSpsId;
}

void VvcSubpicOrderEntry::setSpsId(const std::uint8_t spsId)
{
    mSpsId = spsId;
}

bool VvcSubpicOrderEntry::getSubpicIdInfoFlag() const
{
    return mSubpicIdInfoFlag;
}

void VvcSubpicOrderEntry::setSubpicIdInfoFlag(const bool subpicIdInfoFlag)
{
    mSubpicIdInfoFlag = subpicIdInfoFlag;
}

std::uint32_t VvcSubpicOrderEntry::VvcSubpicOrderEntry::getSize() const
{
    BitStream tmp;
    writeEntry(tmp);
    return tmp.getSize();
}

void VvcSubpicOrderEntry::writeEntry(ISOBMFF::BitStream& bitstr) const
{
    bitstr.writeBits(mSubpicIdInfoFlag, 1);
    bitstr.writeBits(mSubpicRefIdx.size(), 15);

    for (auto i = 0u; i < mSubpicRefIdx.size(); ++i)
    {
        bitstr.writeBits(mSubpicRefIdx.at(i), 16);
    }

    if (mSubpicIdInfoFlag)
    {
        bitstr.writeBits(mSubpicIdLenMinus1, 4);
        bitstr.writeBits(mSubpicIdBitPos, 12);
        bitstr.writeBits(mStartCodeEmulFlag, 1);
        bitstr.writeBits(mPpsSpsSubpicIdFlag, 1);
        if (mPpsSpsSubpicIdFlag)
        {
            bitstr.writeBits(mPpsId, 6);
        }
        else
        {
            bitstr.writeBits(mSpsId, 4);
            bitstr.writeBits(0, 2);  // reserved
        }
    }
}

void VvcSubpicOrderEntry::parseEntry(ISOBMFF::BitStream& bitstr)
{
    mSubpicIdInfoFlag              = bitstr.readBits(1);
    const uint16_t numSubpicRefIdx = bitstr.readBits(15);
    for (auto i = 0u; i < numSubpicRefIdx; ++i)
    {
        mSubpicRefIdx.push_back(bitstr.read16Bits());
    }

    if (mSubpicIdInfoFlag)
    {
        mSubpicIdLenMinus1  = bitstr.readBits(4);
        mSubpicIdBitPos     = bitstr.readBits(12);
        mStartCodeEmulFlag  = bitstr.readBits(1);
        mPpsSpsSubpicIdFlag = bitstr.readBits(1);
        if (mPpsSpsSubpicIdFlag)
        {
            mPpsId = bitstr.readBits(6);
        }
        else
        {
            mSpsId = bitstr.readBits(4);
            bitstr.readBits(2);  // reserved
        }
    }
}
