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

#include "primaryitembox.hpp"

PrimaryItemBox::PrimaryItemBox()
    : FullBox("pitm", 0, 0)
    , mItemId(0)
{
}

void PrimaryItemBox::setItemId(uint32_t itemId)
{
    mItemId = itemId;
}

uint32_t PrimaryItemBox::getItemId() const
{
    return mItemId;
}

void PrimaryItemBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);
    if (getVersion() == 0)
    {
        bitstr.write16Bits(static_cast<std::uint16_t>(mItemId));
    }
    else
    {
        bitstr.write32Bits(mItemId);
    }
    updateSize(bitstr);
}

void PrimaryItemBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    if (getVersion() == 0)
    {
        mItemId = bitstr.read16Bits();
    }
    else
    {
        mItemId = bitstr.read32Bits();
    }
}
