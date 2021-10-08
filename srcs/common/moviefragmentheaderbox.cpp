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

#include "moviefragmentheaderbox.hpp"

MovieFragmentHeaderBox::MovieFragmentHeaderBox()
    : FullBox("mfhd", 0, 0)
    , mSequenceNumber(0)

{
}

void MovieFragmentHeaderBox::setSequenceNumber(const uint32_t sequenceNumber)
{
    mSequenceNumber = sequenceNumber;
}

uint32_t MovieFragmentHeaderBox::getSequenceNumber() const
{
    return mSequenceNumber;
}

void MovieFragmentHeaderBox::writeBox(BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);
    bitstr.write32Bits(mSequenceNumber);
    updateSize(bitstr);
}

void MovieFragmentHeaderBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    mSequenceNumber = bitstr.read32Bits();
}
