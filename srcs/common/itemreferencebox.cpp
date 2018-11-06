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

#include "itemreferencebox.hpp"
#include "bitstream.hpp"

#include <algorithm>
#include <limits>
#include <list>
#include <stdexcept>

SingleItemTypeReferenceBox::SingleItemTypeReferenceBox(bool isLarge)
    : Box(FourCCInt())
    , mFromItemId(0)
    , mIsLarge(isLarge)
{
}

void SingleItemTypeReferenceBox::setReferenceType(FourCCInt referenceType)
{
    Box::setType(referenceType);
}

void SingleItemTypeReferenceBox::setFromItemID(const uint32_t itemID)
{
    mFromItemId = itemID;
}

uint32_t SingleItemTypeReferenceBox::getFromItemID() const
{
    return mFromItemId;
}

void SingleItemTypeReferenceBox::addToItemID(const uint32_t itemID)
{
    mToItemIds.push_back(itemID);
}

void SingleItemTypeReferenceBox::clearToItemIDs()
{
    mToItemIds.clear();
}

void SingleItemTypeReferenceBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeBoxHeader(bitstr);  // parent box

    if (mIsLarge)
    {
        bitstr.write32Bits(mFromItemId);
    }
    else
    {
        bitstr.write16Bits(static_cast<std::uint16_t>(mFromItemId));
    }

    bitstr.write16Bits(static_cast<std::uint16_t>(mToItemIds.size()));
    for (const auto i : mToItemIds)
    {
        if (mIsLarge)
        {
            bitstr.write32Bits(i);
        }
        else
        {
            bitstr.write16Bits(static_cast<std::uint16_t>(i));
        }
    }

    updateSize(bitstr);
}

const Vector<uint32_t>& SingleItemTypeReferenceBox::getToItemIds() const
{
    return mToItemIds;
}

ItemReferenceBox::ItemReferenceBox()
    : FullBox("iref", 0, 0)
    , mReferenceList()
{
}

void ItemReferenceBox::addItemRef(const SingleItemTypeReferenceBox& ref)
{
    mReferenceList.push_back(ref);
}

void ItemReferenceBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);  // parent box

    for (auto& i : mReferenceList)
    {
        i.writeBox(bitstr);
    }

    updateSize(bitstr);
}

void ItemReferenceBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    const bool largeIds = getVersion() ? true : false;

    while (bitstr.numBytesLeft() > 0)
    {
        FourCCInt boxType;
        BitStream subBitStream = bitstr.readSubBoxBitStream(boxType);

        SingleItemTypeReferenceBox singleRef(largeIds);
        singleRef.parseBox(subBitStream);
        addItemRef(singleRef);
    }
}

void SingleItemTypeReferenceBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseBoxHeader(bitstr);  // parent box

    if (mIsLarge)
    {
        mFromItemId = bitstr.read32Bits();
    }
    else
    {
        mFromItemId = bitstr.read16Bits();
    }
    const uint16_t referenceCount = bitstr.read16Bits();
    for (unsigned int i = 0; i < referenceCount; ++i)
    {
        if (mIsLarge)
        {
            mToItemIds.push_back(bitstr.read32Bits());
        }
        else
        {
            mToItemIds.push_back(bitstr.read16Bits());
        }
    }
}

Vector<SingleItemTypeReferenceBox> ItemReferenceBox::getReferencesOfType(FourCCInt type) const
{
    Vector<SingleItemTypeReferenceBox> references;
    for (const auto& reference : mReferenceList)
    {
        if (reference.getType() == type)
        {
            references.push_back(reference);
        }
    }
    return references;
}

void ItemReferenceBox::add(FourCCInt type, const std::uint32_t fromId, const std::uint32_t toId)
{
    const bool largeIds = getVersion() ? true : false;
    if (((fromId > std::numeric_limits<std::uint16_t>::max()) || (toId > std::numeric_limits<std::uint16_t>::max())) &&
        !largeIds)
    {
        throw RuntimeError("ItemReferenceBox::add can not add large item IDs to box version 0");
    }

    // Add to an existing entry if one exists for this type & fromId pair
    auto reference =
        std::find_if(mReferenceList.begin(), mReferenceList.end(), [&](const SingleItemTypeReferenceBox& entry) {
            return (entry.getType() == type) && (entry.getFromItemID() == fromId);
        });
    if (reference != mReferenceList.end())
    {
        reference->addToItemID(toId);
    }
    else
    {
        // Add a new entry
        SingleItemTypeReferenceBox ref(largeIds);
        ref.setType(type);
        ref.setFromItemID(fromId);
        ref.addToItemID(toId);
        mReferenceList.push_back(ref);
    }
}
