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

#include "vvcsubpicturelayoutmapentry.hpp"

#include "bitstream.hpp"

VvcSubpictureLayoutMapEntry::VvcSubpictureLayoutMapEntry()
    : SampleGroupDescriptionEntry("sulm")
    , mGroupIdInfo(0)
{
}

std::uint32_t VvcSubpictureLayoutMapEntry::getSize() const
{
    return static_cast<uint32_t>(sizeof(std::uint32_t) + sizeof(std::uint16_t) + mGroupdIds.size() * sizeof(std::uint16_t));
}

FourCCInt VvcSubpictureLayoutMapEntry::getGroupIdInfo4CC() const
{
    return mGroupIdInfo;
}

void VvcSubpictureLayoutMapEntry::setGroupIdInfo4CC(const FourCCInt groupID_info_4cc)
{
    mGroupIdInfo = groupID_info_4cc;
}

std::vector<std::uint16_t> VvcSubpictureLayoutMapEntry::getGroupIds() const
{
    return mGroupdIds;
}

void VvcSubpictureLayoutMapEntry::setGroupIds(const std::vector<std::uint16_t>& groupIds)
{
    mGroupdIds = groupIds;
}

void VvcSubpictureLayoutMapEntry::writeEntry(ISOBMFF::BitStream& bitstr) const
{
    if(mGroupdIds.empty()){
        return;
    }
    bitstr.writeBits(mGroupIdInfo.getUInt32(), 32);  //bit(32) groupID_info_4cc
    bitstr.writeBits(mGroupdIds.size() - 1, 16);     //bit(16) entry_count_minus1

    for (unsigned short mGroupdId : mGroupdIds)
    {
        bitstr.writeBits(mGroupdId, 16);             //bit(16) group_id
    }
}

void VvcSubpictureLayoutMapEntry::parseEntry(ISOBMFF::BitStream& bitstr)
{
    mGroupIdInfo          = bitstr.readBits(32);
    const auto numEntries = bitstr.readBits(16) + 1;
    mGroupdIds.reserve(numEntries);
    for (auto i = 0u; i < numEntries; ++i)
    {
        mGroupdIds.push_back(bitstr.read16Bits());
    }
}
