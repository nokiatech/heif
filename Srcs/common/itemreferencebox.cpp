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

#include "itemreferencebox.hpp"
#include "bitstream.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

SingleItemTypeReferenceBox::SingleItemTypeReferenceBox(bool isLarge) :
    Box(""),
    mFromItemId(0),
    mIsLarge(isLarge)
{
}

void SingleItemTypeReferenceBox::setReferenceType(const std::string& referenceType)
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

void SingleItemTypeReferenceBox::writeBox(BitStream& bitstr)
{
    writeBoxHeader(bitstr);  // parent box

    if (mIsLarge)
    {
         bitstr.write32Bits(mFromItemId);
    }
    else
    {
        bitstr.write16Bits(mFromItemId);
    }

    bitstr.write16Bits(mToItemIds.size());
    for (const auto i : mToItemIds)
    {
        if (mIsLarge)
        {
             bitstr.write32Bits(i);
        }
        else
        {
            bitstr.write16Bits(i);
        }
    }

    updateSize(bitstr);
}

std::vector<uint32_t> SingleItemTypeReferenceBox::getToItemIds() const
{
    return mToItemIds;
}

ItemReferenceBox::ItemReferenceBox() :
    FullBox("iref", 0, 0),
    mReferenceList()
{
}

void ItemReferenceBox::addItemRef(const SingleItemTypeReferenceBox& ref)
{
    mReferenceList.push_back(ref);
}

void ItemReferenceBox::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);  // parent box

    for (auto& i : mReferenceList)
    {
        i.writeBox(bitstr);
    }

    updateSize(bitstr);
}

void ItemReferenceBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    const bool largeIds = getVersion() ? true : false;

    while (bitstr.numBytesLeft() > 0)
    {
        SingleItemTypeReferenceBox singleRef(largeIds);
        singleRef.parseBox(bitstr);
        addItemRef(singleRef);
    }
}

void SingleItemTypeReferenceBox::parseBox(BitStream& bitstr)
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

std::vector<SingleItemTypeReferenceBox> ItemReferenceBox::getReferencesOfType(const std::string& type) const
{
    std::vector<SingleItemTypeReferenceBox> references;
    for (const auto& reference : mReferenceList)
    {
        if (reference.getType() == type)
        {
            references.push_back(reference);
        }
    }
    return references;
}

void ItemReferenceBox::add(const std::string& type, const std::uint32_t fromId, const std::uint32_t toId)
{
    const bool largeIds = getVersion() ? true : false;
    if (((fromId > std::numeric_limits<std::uint16_t>::max()) || 
        (toId > std::numeric_limits<std::uint16_t>::max())) && not largeIds)
    {
        throw std::runtime_error("ItemReferenceBox::add can not add large item IDs to box version 0");
    }
    
    // Add to an existing entry if one exists for this type & fromId pair
    auto reference = std::find_if(mReferenceList.begin(), mReferenceList.end(),
        [&](const SingleItemTypeReferenceBox& entry)
        {
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
