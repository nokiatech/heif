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

#include "itemprotectionbox.hpp"

ItemProtectionBox::ItemProtectionBox()
    : FullBox("ipro", 0, 0)
{
}

std::uint16_t ItemProtectionBox::getProtectionCount() const
{
    return static_cast<std::uint16_t>(mProtectionInformation.size());
}

const ProtectionSchemeInfoBox& ItemProtectionBox::getEntry(const std::uint16_t index) const
{
    return mProtectionInformation.at(index);
}

std::uint16_t ItemProtectionBox::addEntry(const ProtectionSchemeInfoBox& sinf)
{
    mProtectionInformation.push_back(sinf);
    return static_cast<std::uint16_t>(mProtectionInformation.size() - 1);
}

void ItemProtectionBox::writeBox(BitStream& bitstream) const
{
    if (mProtectionInformation.size() == 0)
    {
        return;
    }

    writeFullBoxHeader(bitstream);
    bitstream.write16Bits(static_cast<std::uint16_t>(mProtectionInformation.size()));
    for (auto& box : mProtectionInformation)
    {
        box.writeBox(bitstream);
    }
    updateSize(bitstream);
}

void ItemProtectionBox::parseBox(BitStream& bitstream)
{
    parseFullBoxHeader(bitstream);
    const unsigned int boxes = bitstream.read16Bits();
    for (unsigned int i = 0; i < boxes; ++i)
    {
        FourCCInt boxType;
        BitStream subBitStream = bitstream.readSubBoxBitStream(boxType);
        ProtectionSchemeInfoBox sinf;
        sinf.parseBox(subBitStream);
        mProtectionInformation.push_back(sinf);
    }
}
