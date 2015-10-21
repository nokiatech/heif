/* Copyright (c) 2015, Nokia Technologies Ltd.
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

#include "itemlocationbox.hpp"

#include <algorithm>
#include <stdexcept>

using namespace std;

ItemLocation::ItemLocation() :
    mItemID(0),
    mConstructionMethod(ConstructionMethod::FILE_OFFSET),
    mDataReferenceIndex(0),
    mBaseOffset(0),
    mExtentList()
{
}

void ItemLocation::setItemID(const uint16_t itemID)
{
    mItemID = itemID;
}

uint16_t ItemLocation::getItemID() const
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

void ItemLocation::setDataReferenceIndex(const uint16_t dataReferenceIndex)
{
    mDataReferenceIndex = dataReferenceIndex;
}

uint16_t ItemLocation::getDataReferenceIndex() const
{
    return mDataReferenceIndex;
}

void ItemLocation::setBaseOffset(const uint64_t baseOffset)
{
    mBaseOffset = baseOffset;
}

uint64_t ItemLocation::getBaseOffset() const
{
    return mBaseOffset;
}

uint16_t ItemLocation::getExtentCount() const
{
    return mExtentList.size();
}

void ItemLocation::addExtent(const ItemLocationExtent& extent)
{
    mExtentList.push_back(extent);
}

const ExtentList& ItemLocation::getExtentList() const
{
    return mExtentList;
}

ItemLocationBox::ItemLocationBox() :
    FullBox("iloc", 0, 0),
    mOffsetSize(4),
    mLengthSize(4),
    mBaseOffsetSize(4),
    mIndexSize(0),
    mItemLocations()
{
}

void ItemLocationBox::setOffsetSize(const uint8_t offsetSize)
{
    mOffsetSize = offsetSize;
}

uint8_t ItemLocationBox::getOffsetSize() const
{
    return mOffsetSize;
}

void ItemLocationBox::setLengthSize(const uint8_t lengthSize)
{
    mLengthSize = lengthSize;
}

uint8_t ItemLocationBox::getLengthSize() const
{
    return mLengthSize;
}

void ItemLocationBox::setBaseOffsetSize(const uint8_t baseOffsetSize)
{
    mBaseOffsetSize = baseOffsetSize;
}

uint8_t ItemLocationBox::getBaseOffsetSize() const
{
    return mBaseOffsetSize;
}

void ItemLocationBox::setIndexSize(const uint8_t indexSize)
{
    mIndexSize = indexSize;
}

uint8_t ItemLocationBox::getIndexSize() const
{
    return mIndexSize;
}

uint16_t ItemLocationBox::getItemCount() const
{
    return mItemLocations.size();
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

void ItemLocationBox::addExtent(const std::uint16_t itemId, const ItemLocationExtent& extent)
{
    const auto iter = findItem(itemId);
    if (iter == mItemLocations.end())
    {
        throw std::runtime_error("ItemLocationBox::addExtent() invalid item id");
    }

    iter->addExtent(extent);
}

ItemLocationVector& ItemLocationBox::getItemLocations()
{
    return mItemLocations;
}

bool ItemLocationBox::hasItemIdEntry(std::uint16_t itemId) const
{
    if (findItem(itemId) != mItemLocations.cend())
    {
        return true;
    }
    return false;
}

bool ItemLocationBox::setItemDataReferenceIndex(const std::uint16_t itemId, const std::uint16_t dataReferenceIndex)
{
    const auto iter = findItem(itemId);
    if (iter != mItemLocations.end())
    {
        iter->setDataReferenceIndex(dataReferenceIndex);
        return true;
    }

    return false;
}

void ItemLocationBox::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);

    bitstr.writeBits(mOffsetSize, 4);
    bitstr.writeBits(mLengthSize, 4);
    bitstr.writeBits(mBaseOffsetSize, 4);
    if (getVersion() == 1)
    {
        bitstr.writeBits(mIndexSize, 4);
    }
    else
    {
        bitstr.writeBits(0, 4); // reserved = 0
    }
    bitstr.write16Bits(mItemLocations.size());

    for (const auto& itemLoc : mItemLocations)
    {
        bitstr.write16Bits(itemLoc.getItemID());
        if (getVersion() == 1)
        {
            bitstr.writeBits(0, 12); // reserved = 0
            bitstr.writeBits(static_cast<unsigned int>(itemLoc.getConstructionMethod()), 4);
        }
        bitstr.write16Bits(itemLoc.getDataReferenceIndex());
        bitstr.writeBits(itemLoc.getBaseOffset(), mBaseOffsetSize * 8);
        bitstr.write16Bits(itemLoc.getExtentCount());

        const ExtentList& extents = itemLoc.getExtentList();
        for (const auto& locExt : extents)
        {
            if ((getVersion() == 1) && (mIndexSize > 0))
            {
                bitstr.writeBits(locExt.mExtentIndex, mIndexSize * 8);
            }
            bitstr.writeBits(locExt.mExtentOffset, mOffsetSize * 8);
            bitstr.writeBits(locExt.mExtentLength, mLengthSize * 8);
        }
    }

    updateSize(bitstr);
}

void ItemLocationBox::parseBox(BitStream& bitstr)
{
    unsigned int itemCount;

    parseFullBoxHeader(bitstr);

    mOffsetSize = static_cast<uint8_t>(bitstr.readBits(4));
    mLengthSize = static_cast<uint8_t>(bitstr.readBits(4));
    mBaseOffsetSize = static_cast<uint8_t>(bitstr.readBits(4));
    if (getVersion() == 1)
    {
        mIndexSize = static_cast<uint8_t>(bitstr.readBits(4));
    }
    else
    {
        bitstr.readBits(4); // reserved = 0
    }
    itemCount = bitstr.read16Bits();
    for (unsigned int i = 0; i < itemCount; i++)
    {
        ItemLocation itemLoc;
        itemLoc.setItemID(bitstr.read16Bits());
        if (getVersion() == 1)
        {
            bitstr.readBits(12); // reserved = 0
            itemLoc.setConstructionMethod(static_cast<ItemLocation::ConstructionMethod>(bitstr.readBits(4)));
        }
        itemLoc.setDataReferenceIndex(bitstr.read16Bits());
        itemLoc.setBaseOffset(bitstr.readBits(mBaseOffsetSize * 8));
        const unsigned int extentCount = bitstr.read16Bits();
        for (unsigned int j = 0; j < extentCount; j++)
        {
            ItemLocationExtent locExt;
            if ((getVersion() == 1) && (mIndexSize > 0))
            {
                locExt.mExtentIndex = bitstr.readBits(mIndexSize * 8);
            }
            locExt.mExtentOffset = bitstr.readBits(mOffsetSize * 8);
            locExt.mExtentLength = bitstr.readBits(mLengthSize * 8);
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

    throw std::runtime_error("ItemLocationBox::getItemLocationForID: invalid item ID");
}

const ItemLocationExtent& ItemLocation::getExtent(const unsigned int i) const
{
    if (i >= mExtentList.size())
    {
        throw std::runtime_error("ItemLocationBox::getExtent: invalid extent ID");
    }
    else
    {
        return mExtentList.at(i);
    }
}

ItemLocationVector::const_iterator ItemLocationBox::findItem(const std::uint16_t itemId) const
{
    ItemLocationVector::const_iterator iter = std::find_if(mItemLocations.cbegin(), mItemLocations.cend(),
        [itemId](const ItemLocation& itemLocation)
        {
            return itemLocation.getItemID() == itemId;
        });
    return iter;
}

ItemLocationVector::iterator ItemLocationBox::findItem(const std::uint16_t itemId)
{
    ItemLocationVector::iterator iter = std::find_if(mItemLocations.begin(), mItemLocations.end(),
        [itemId](const ItemLocation& itemLocation)
        {
            return itemLocation.getItemID() == itemId;
        });
    return iter;
}
