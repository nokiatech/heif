/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "movieextendsheaderbox.hpp"

#include <stdexcept>

MovieExtendsHeaderBox::MovieExtendsHeaderBox(uint8_t version)
    : FullBox("mehd", version, 0)
    , mFragmentDuration(0)
{
}

void MovieExtendsHeaderBox::setFragmentDuration(const uint64_t fragmentDuration)
{
    mFragmentDuration = fragmentDuration;
}

uint64_t MovieExtendsHeaderBox::getFragmentDuration() const
{
    return mFragmentDuration;
}

void MovieExtendsHeaderBox::writeBox(BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);
    if (getVersion() == 0)
    {
        bitstr.write32Bits(static_cast<uint32_t>(mFragmentDuration));
    }
    else if (getVersion() == 1)
    {
        bitstr.write64Bits(mFragmentDuration);
    }
    else
    {
        throw RuntimeError("MovieExtendsHeaderBox::writeBox() supports only 'mehd' version 0 or 1");
    }
    updateSize(bitstr);
}

void MovieExtendsHeaderBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    if (getVersion() == 0)
    {
        mFragmentDuration = bitstr.read32Bits();
    }
    else if (getVersion() == 1)
    {
        mFragmentDuration = bitstr.read64Bits();
    }
    else
    {
        throw RuntimeError("MovieExtendsHeaderBox::parseBox() supports only 'mehd' version 0 or 1");
    }
}
