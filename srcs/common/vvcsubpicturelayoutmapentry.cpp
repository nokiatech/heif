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

#include "vvcsubpicturelayoutmapentry.hpp"

VvcSubpictureLayoutMapEntry::VvcSubpictureLayoutMapEntry()
    : SampleGroupDescriptionEntry("sulm")
    , mGroupIdInfo(0)
{
}

std::uint32_t VvcSubpictureLayoutMapEntry::getSize() const
{
    return sizeof(std::uint32_t) + sizeof(std::uint16_t) + mGroupdIds.size() * sizeof(std::uint16_t);
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
    bitstr.writeBits(mGroupIdInfo.getUInt32(), 32);
    bitstr.writeBits(mGroupdIds.size() - 1, 16);

    for (auto i = 0u; i < mGroupdIds.size(); ++i)
    {
        bitstr.writeBits(mGroupdIds.at(i), 16);
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
