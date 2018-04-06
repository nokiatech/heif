/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "sampletometadataitementry.hpp"

SampleToMetadataItemEntry::SampleToMetadataItemEntry()
    : SampleGroupDescriptionEntry("stmi")
    , mMetaBoxHandlerType(0)
    , mItemIds()
{
}

void SampleToMetadataItemEntry::setMetaBoxHandlerType(FourCCInt type)
{
    mMetaBoxHandlerType = type.getUInt32();
}

FourCCInt SampleToMetadataItemEntry::getMetaBoxHandlerType() const
{
    return mMetaBoxHandlerType;
}

void SampleToMetadataItemEntry::setItemIds(const Vector<std::uint32_t>& itemIds)
{
    mItemIds = itemIds;
}

Vector<uint32_t>& SampleToMetadataItemEntry::getItemIds()
{
    return mItemIds;
}

const Vector<uint32_t>& SampleToMetadataItemEntry::getItemIds() const
{
    return mItemIds;
}

std::uint32_t SampleToMetadataItemEntry::getSize() const
{
    const uint32_t size =
        static_cast<uint32_t>(sizeof(mMetaBoxHandlerType) + sizeof(uint32_t) + (mItemIds.size() * sizeof(uint32_t)));
    return size;
}

void SampleToMetadataItemEntry::writeEntry(ISOBMFF::BitStream& bitstr)
{
    bitstr.write32Bits(mMetaBoxHandlerType);
    bitstr.write32Bits(static_cast<uint32_t>(mItemIds.size()));
    for (auto itemId : mItemIds)
    {
        bitstr.write32Bits(itemId);
    }
}

void SampleToMetadataItemEntry::parseEntry(ISOBMFF::BitStream& bitstr)
{
    mMetaBoxHandlerType  = bitstr.read32Bits();
    const uint32_t count = bitstr.read32Bits();
    for (uint32_t i = 0; i < count; ++i)
    {
        mItemIds.push_back(bitstr.read32Bits());
    }
}
