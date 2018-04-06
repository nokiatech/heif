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

#include "itemlocationbox.hpp"

#include <algorithm>
#include <stdexcept>

ItemLocation::ItemLocation()
    : mItemID(0)
    , mConstructionMethod(ConstructionMethod::FILE_OFFSET)
    , mDataReferenceIndex(0)
    , mBaseOffset(0)
    , mExtentList()
{
}

void ItemLocation::setItemID(const std::uint32_t itemID)
{
    mItemID = itemID;
}

uint32_t ItemLocation::getItemID() const
{
    return mItemID;
}

void ItemLocation::setConstructionMethod(const ItemLocation::ConstructionMethod constructionMethod)
{
    mConstructionMethod = constructionMethod;
}

ItemLocation::ConstructionMethod ItemLocation::getConstructionMethod() const
{
    return mConstructionMethod;
}

void ItemLocation::setDataReferenceIndex(const std::uint16_t dataReferenceIndex)
{
    mDataReferenceIndex = dataReferenceIndex;
}

std::uint16_t ItemLocation::getDataReferenceIndex() const
{
    return mDataReferenceIndex;
}

void ItemLocation::setBaseOffset(const std::uint64_t baseOffset)
{
    mBaseOffset = baseOffset;
}

std::uint64_t ItemLocation::getBaseOffset() const
{
    return mBaseOffset;
}

std::uint16_t ItemLocation::getExtentCount() const
{
    return static_cast<std::uint16_t>(mExtentList.size());
}

void ItemLocation::addExtent(const ItemLocationExtent& extent)
{
    mExtentList.push_back(extent);
}

const ExtentList& ItemLocation::getExtentList() const
{
    return mExtentList;
}

ItemLocationBox::ItemLocationBox()
    : FullBox("iloc", 0, 0)
    , mOffsetSize(4)
    , mLengthSize(4)
    , mBaseOffsetSize(4)
    , mIndexSize(0)
    , mItemLocations()
{
}

void ItemLocationBox::setOffsetSize(const std::uint8_t offsetSize)
{
    mOffsetSize = offsetSize;
}

std::uint8_t ItemLocationBox::getOffsetSize() const
{
    return mOffsetSize;
}

void ItemLocationBox::setLengthSize(const std::uint8_t lengthSize)
{
    mLengthSize = lengthSize;
}

std::uint8_t ItemLocationBox::getLengthSize() const
{
    return mLengthSize;
}

void ItemLocationBox::setBaseOffsetSize(const std::uint8_t baseOffsetSize)
{
    mBaseOffsetSize = baseOffsetSize;
}

std::uint8_t ItemLocationBox::getBaseOffsetSize() const
{
    return mBaseOffsetSize;
}

void ItemLocationBox::setIndexSize(const std::uint8_t indexSize)
{
    mIndexSize = indexSize;
}

std::uint8_t ItemLocationBox::getIndexSize() const
{
    return mIndexSize;
}

std::uint32_t ItemLocationBox::getItemCount() const
{
    return static_cast<std::uint32_t>(mItemLocations.size());
}

void ItemLocationBox::addLocation(const ItemLocation& itemLoc)
{
    // Use version to 1 if needed
    if (itemLoc.getConstructionMethod() != ItemLocation::ConstructionMethod::FILE_OFFSET)
    {
        setVersion(1);
    }
    mItemLocations.push_back(itemLoc);
}

void ItemLocationBox::addExtent(const std::uint32_t itemId, const ItemLocationExtent& extent)
{
    const auto iter = findItem(itemId);
    if (iter == mItemLocations.end())
    {
        throw RuntimeError("ItemLocationBox::addExtent() invalid item id");
    }

    iter->addExtent(extent);
}

ItemLocationVector& ItemLocationBox::getItemLocations()
{
    return mItemLocations;
}

bool ItemLocationBox::hasItemIdEntry(std::uint32_t itemId) const
{
    if (findItem(itemId) != mItemLocations.cend())
    {
        return true;
    }
    return false;
}

bool ItemLocationBox::setItemDataReferenceIndex(const std::uint32_t itemId, const std::uint16_t dataReferenceIndex)
{
    const auto iter = findItem(itemId);
    if (iter != mItemLocations.end())
    {
        iter->setDataReferenceIndex(dataReferenceIndex);
        return true;
    }

    return false;
}

void ItemLocationBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.writeBits(mOffsetSize, 4);
    bitstr.writeBits(mLengthSize, 4);
    bitstr.writeBits(mBaseOffsetSize, 4);
    if ((getVersion() == 1) || (getVersion() == 2))
    {
        bitstr.writeBits(mIndexSize, 4);
    }
    else
    {
        bitstr.writeBits(0, 4);  // reserved = 0
    }
    if (getVersion() < 2)
    {
        bitstr.write16Bits(static_cast<std::uint16_t>(mItemLocations.size()));
    }
    else if (getVersion() == 2)
    {
        bitstr.write32Bits(static_cast<unsigned int>(mItemLocations.size()));
    }

    for (const auto& itemLoc : mItemLocations)
    {
        if (getVersion() < 2)
        {
            bitstr.write16Bits(static_cast<std::uint16_t>(itemLoc.getItemID()));
        }
        else if (getVersion() == 2)
        {
            bitstr.write32Bits(itemLoc.getItemID());
        }

        if ((getVersion() == 1) || (getVersion() == 2))
        {
            bitstr.writeBits(0, 12);  // reserved = 0
            bitstr.writeBits(static_cast<unsigned int>(itemLoc.getConstructionMethod()), 4);
        }
        bitstr.write16Bits(itemLoc.getDataReferenceIndex());
        bitstr.writeBits(itemLoc.getBaseOffset(), static_cast<unsigned int>(mBaseOffsetSize * 8));
        bitstr.write16Bits(itemLoc.getExtentCount());

        const ExtentList& extents = itemLoc.getExtentList();
        for (const auto& locExt : extents)
        {
            if (((getVersion() == 1) || (getVersion() == 2)) && (mIndexSize > 0))
            {
                bitstr.writeBits(locExt.mExtentIndex, static_cast<unsigned int>(mIndexSize * 8));
            }
            bitstr.writeBits(locExt.mExtentOffset, static_cast<unsigned int>(mOffsetSize * 8));
            bitstr.writeBits(locExt.mExtentLength, static_cast<unsigned int>(mLengthSize * 8));
        }
    }

    updateSize(bitstr);
}

void ItemLocationBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    unsigned int itemCount = 0;

    parseFullBoxHeader(bitstr);

    mOffsetSize     = static_cast<uint8_t>(bitstr.readBits(4));
    mLengthSize     = static_cast<uint8_t>(bitstr.readBits(4));
    mBaseOffsetSize = static_cast<uint8_t>(bitstr.readBits(4));
    if ((getVersion() == 1) || (getVersion() == 2))
    {
        mIndexSize = static_cast<uint8_t>(bitstr.readBits(4));
    }
    else
    {
        bitstr.readBits(4);  // reserved = 0
    }

    if (getVersion() < 2)
    {
        itemCount = bitstr.read16Bits();
    }
    else if (getVersion() == 2)
    {
        itemCount = bitstr.read32Bits();
    }

    for (unsigned int i = 0; i < itemCount; i++)
    {
        ItemLocation itemLoc;
        if (getVersion() < 2)
        {
            itemLoc.setItemID(bitstr.read16Bits());
        }
        else if (getVersion() == 2)
        {
            itemLoc.setItemID(bitstr.read32Bits());
        }

        if ((getVersion() == 1) || (getVersion() == 2))
        {
            bitstr.readBits(12);  // reserved = 0
            itemLoc.setConstructionMethod(static_cast<ItemLocation::ConstructionMethod>(bitstr.readBits(4)));
        }
        itemLoc.setDataReferenceIndex(bitstr.read16Bits());
        itemLoc.setBaseOffset(bitstr.readBits(static_cast<std::uint8_t>(mBaseOffsetSize * 8)));
        const unsigned int extentCount = bitstr.read16Bits();
        for (unsigned int j = 0; j < extentCount; j++)
        {
            ItemLocationExtent locExt;
            if (((getVersion() == 1) || (getVersion() == 2)) && (mIndexSize > 0))
            {
                locExt.mExtentIndex = bitstr.readBits(static_cast<std::uint8_t>(mIndexSize * 8));
            }
            locExt.mExtentOffset = bitstr.readBits(static_cast<std::uint8_t>(mOffsetSize * 8));
            locExt.mExtentLength = bitstr.readBits(static_cast<std::uint8_t>(mLengthSize * 8));
            itemLoc.addExtent(locExt);
        }
        addLocation(itemLoc);
    }
}

const ItemLocation& ItemLocationBox::getItemLocationForID(const unsigned int itemID) const
{
    const auto iter = findItem(itemID);
    if (iter != mItemLocations.cend())
    {
        return *iter;
    }

    throw RuntimeError(
        "ItemLocationBox::getItemLocationForID: invalid item ID, should be wrapped internally with hasItemIdEntry()");
}

const ItemLocationExtent& ItemLocation::getExtent(const unsigned int i) const
{
    if (i >= mExtentList.size())
    {
        throw RuntimeError("ItemLocationBox::getExtent: invalid extent ID");
    }
    else
    {
        return mExtentList.at(i);
    }
}

ItemLocationVector::const_iterator ItemLocationBox::findItem(const std::uint32_t itemId) const
{
    ItemLocationVector::const_iterator iter =
        std::find_if(mItemLocations.cbegin(), mItemLocations.cend(),
                     [itemId](const ItemLocation& itemLocation) { return itemLocation.getItemID() == itemId; });
    return iter;
}

ItemLocationVector::iterator ItemLocationBox::findItem(const std::uint32_t itemId)
{
    ItemLocationVector::iterator iter =
        std::find_if(mItemLocations.begin(), mItemLocations.end(),
                     [itemId](const ItemLocation& itemLocation) { return itemLocation.getItemID() == itemId; });
    return iter;
}
