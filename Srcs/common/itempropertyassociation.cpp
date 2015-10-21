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

#include "itempropertyassociation.hpp"

#include <algorithm>
#include <limits>

static const int PROPERTY_INDEX_WIDTH_LARGE = 15;
static const int PROPERTY_INDEX_WIDTH_SMALL = 7;

ItemPropertyAssociation::ItemPropertyAssociation() :
    FullBox("ipma", 0, 0),
    mAssociations()
{
}


void ItemPropertyAssociation::addEntry(const std::uint32_t itemId, const std::uint32_t index, const bool essential)
{
    Entry entry;
    entry.essential = essential;
    entry.index = index;

    mAssociations[itemId].push_back(entry);
}


ItemPropertyAssociation::AssociationEntries ItemPropertyAssociation::getAssociationEntries(const std::uint32_t itemId) const
{
    std::vector<Entry> indexes;

    if (mAssociations.count(itemId) != 0)
    {
        for (const auto& entry : mAssociations.at(itemId))
        {
            indexes.push_back(entry);
        }
    }

    return indexes;
}


void ItemPropertyAssociation::writeBox(BitStream& bitstream)
{
    bool useLargeItemIds = false;

    if (std::any_of(mAssociations.cbegin(), mAssociations.cend(), [](const std::pair<ItemId, AssociationEntries>& entry)
    {
        return entry.first > std::numeric_limits<std::uint16_t>::max();
    }))
    {
        useLargeItemIds = true;
    }
    setVersion(useLargeItemIds);

    bool useLargeIndexes = false;
    for (const auto& entry : mAssociations)
    {
        for (const auto association : entry.second)
        {
            if (association.index > std::numeric_limits<std::int8_t>::max())
            {
                useLargeIndexes = true;
                break;
            }
        }
    }
    setFlags(useLargeIndexes);

    writeFullBoxHeader(bitstream);

    bitstream.write32Bits(mAssociations.size());

    for (const auto& entry : mAssociations)
    {
        if (getVersion() < 1)
        {
            bitstream.write16Bits(entry.first);
        }
        else
        {
            bitstream.write32Bits(entry.first);
        }

        bitstream.write8Bits(entry.second.size()); // write association_count

        for (const auto& property : entry.second)
        {
            bitstream.writeBits(property.essential, 1);
            if (getFlags() & 1)
            {
                bitstream.writeBits(property.index, PROPERTY_INDEX_WIDTH_LARGE);
            }
            else
            {
                bitstream.writeBits(property.index, PROPERTY_INDEX_WIDTH_SMALL);
            }
        }
    }

    updateSize(bitstream);
}


void ItemPropertyAssociation::parseBox(BitStream& bitstream)
{
    parseFullBoxHeader(bitstream);

    const std::uint32_t entryCount = bitstream.read32Bits();
    for (unsigned int k = 0; k < entryCount; ++k)
    {
        uint32_t itemId = 0;
        if (getVersion() < 1)
        {
            itemId = bitstream.read16Bits();
        }
        else
        {
            itemId = bitstream.read32Bits();
        }

        AssociationEntries propertyIndexVector;
        const unsigned int associationCount = bitstream.read8Bits();
        for (unsigned int i = 0; i < associationCount; ++i)
        {
            Entry propertyIndexEntry;
            propertyIndexEntry.essential = bitstream.readBits(1);
            if (getFlags() & 1)
            {
                propertyIndexEntry.index = bitstream.readBits(PROPERTY_INDEX_WIDTH_LARGE);
            }
            else
            {
                propertyIndexEntry.index = bitstream.readBits(PROPERTY_INDEX_WIDTH_SMALL);
            }
            propertyIndexVector.push_back(propertyIndexEntry);
        }
        mAssociations.insert( { itemId, propertyIndexVector} );
    }
}
