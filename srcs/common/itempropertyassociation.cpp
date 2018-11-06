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

#include "itempropertyassociation.hpp"

#include <algorithm>
#include <limits>

static const int PROPERTY_INDEX_WIDTH_LARGE = 15;
static const int PROPERTY_INDEX_WIDTH_SMALL = 7;

ItemPropertyAssociation::ItemPropertyAssociation()
    : FullBox("ipma", 0, 0)
    , mAssociations()
{
}

void ItemPropertyAssociation::addEntry(const std::uint32_t itemId, const std::uint16_t index, const bool essential)
{
    Entry entry;
    entry.essential = essential;
    entry.index     = index;

    mAssociations[itemId].push_back(entry);

    // Use 32-bit item ids if required.
    if ((FullBox::getVersion() == 0) && (itemId > std::numeric_limits<std::uint16_t>::max()))
    {
        setVersion(1);
    }

    // Use large property indexes if required.
    if ((FullBox::getFlags() & 1) == false)
    {
        if (entry.index > std::numeric_limits<std::int8_t>::max())
        {
            FullBox::setFlags(FullBox::getFlags() | 1);
        }
    }
}


const ItemPropertyAssociation::AssociationEntries&
ItemPropertyAssociation::getAssociationEntries(const std::uint32_t itemId) const
{
    const auto it = mAssociations.find(itemId);
    if (it != mAssociations.end())
    {
        return it->second;
    }
    static Vector<Entry> empty;
    return empty;
}

void ItemPropertyAssociation::writeBox(BitStream& bitstream) const
{
    writeFullBoxHeader(bitstream);

    bitstream.write32Bits(static_cast<unsigned int>(mAssociations.size()));

    for (const auto& entry : mAssociations)
    {
        if (getVersion() < 1)
        {
            bitstream.write16Bits(static_cast<std::uint16_t>(entry.first));
        }
        else
        {
            bitstream.write32Bits(entry.first);
        }

        bitstream.write8Bits(static_cast<std::uint8_t>(entry.second.size()));  // write association_count

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

        AssociationEntries& propertyIndexVector = mAssociations[itemId];
        const unsigned int associationCount = bitstream.read8Bits();
        propertyIndexVector.reserve(associationCount);
        for (unsigned int i = 0; i < associationCount; ++i)
        {
            Entry propertyIndexEntry;
            propertyIndexEntry.essential = (bitstream.readBits(1) != 0);
            if (getFlags() & 1)
            {
                propertyIndexEntry.index = static_cast<std::uint16_t>(bitstream.readBits(PROPERTY_INDEX_WIDTH_LARGE));
            }
            else
            {
                propertyIndexEntry.index = static_cast<std::uint16_t>(bitstream.readBits(PROPERTY_INDEX_WIDTH_SMALL));
            }
            propertyIndexVector.push_back(propertyIndexEntry);
        }
    }
}
