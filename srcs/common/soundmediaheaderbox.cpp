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

#include "soundmediaheaderbox.hpp"

SoundMediaHeaderBox::SoundMediaHeaderBox()
    : FullBox("smhd", 0, 0)
    , mBalance(0)
{
}

void SoundMediaHeaderBox::setBalance(const std::uint16_t balance)
{
    mBalance = balance;
}

std::uint16_t SoundMediaHeaderBox::getBalance() const
{
    return mBalance;
}

void SoundMediaHeaderBox::writeBox(BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.write16Bits(mBalance);  // Balance
    bitstr.write16Bits(0);         // Reserved

    updateSize(bitstr);
}

void SoundMediaHeaderBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    mBalance = bitstr.read16Bits();  // Balance
    bitstr.read16Bits();             // Reserved
}
