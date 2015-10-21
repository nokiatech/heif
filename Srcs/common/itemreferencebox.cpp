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

SingleItemTypeReferenceBox::SingleItemTypeReferenceBox() :
    SingleItemTypeReferenceBox("")
{
}

SingleItemTypeReferenceBox::SingleItemTypeReferenceBox(const std::string& referenceType) :
    Box(referenceType.c_str()),
    mFromItemId(0),
    mToItemIds()
{
}

void SingleItemTypeReferenceBox::setFromItemID(const uint16_t itemID)
{
    mFromItemId = itemID;
}

uint16_t SingleItemTypeReferenceBox::getFromItemID() const
{
    return mFromItemId;
}

void SingleItemTypeReferenceBox::addToItemID(const uint16_t itemID)
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

    bitstr.write16Bits(mFromItemId);
    bitstr.write16Bits(mToItemIds.size());
    for (const auto i : mToItemIds)
    {
        bitstr.write16Bits(i);
    }

    updateSize(bitstr);
}

std::vector<uint16_t> SingleItemTypeReferenceBox::getToItemIds() const
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

    while (bitstr.numBytesLeft() > 0)
    {
        SingleItemTypeReferenceBox singleRef;
        singleRef.parseBox(bitstr);
        addItemRef(singleRef);
    }
}

void SingleItemTypeReferenceBox::parseBox(BitStream& bitstr)
{
    parseBoxHeader(bitstr);  // parent box

    mFromItemId = bitstr.read16Bits();
    const uint16_t referenceCount = bitstr.read16Bits();
    for (unsigned int i = 0; i < referenceCount; ++i)
    {
        mToItemIds.push_back(bitstr.read16Bits());
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
    return std::move(references);
}

void ItemReferenceBox::add(const std::string& type, const std::uint16_t fromId, const std::uint16_t toId)
{
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
        SingleItemTypeReferenceBox ref(type);
        ref.setFromItemID(fromId);
        ref.addToItemID(toId);
        mReferenceList.push_back(ref);
    }
}
