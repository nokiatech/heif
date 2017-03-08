/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#include "itemprotectionbox.hpp"

ItemProtectionBox::ItemProtectionBox() :
    FullBox("ipro", 0, 0)
{
}

std::uint16_t ItemProtectionBox::getSize() const
{
    return mProtectionInformation.size();
}

const ProtectionSchemeInfoBox& ItemProtectionBox::getEntry(const std::uint16_t index) const
{
    return mProtectionInformation.at(index);
}

std::uint16_t ItemProtectionBox::addEntry(const ProtectionSchemeInfoBox& sinf)
{
    mProtectionInformation.push_back(sinf);
    return mProtectionInformation.size() - 1 ;
}

void ItemProtectionBox::writeBox(BitStream& bitstream)
{
    if (mProtectionInformation.size() == 0)
    {
        return;
    }

    writeFullBoxHeader(bitstream);
    bitstream.write16Bits(mProtectionInformation.size());
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
