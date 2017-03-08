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

#include "sampleentrybox.hpp"
#include "bitstream.hpp"
#include "log.hpp"

using std::endl;

static const int RESERVED_BYTES = 6;

SampleEntryBox::SampleEntryBox(const FourCCInt codingname) :
    Box(codingname),
    mDataReferenceIndex(0)
{
}

std::uint16_t SampleEntryBox::getDataReferenceIndex() const
{
    return mDataReferenceIndex;
}

void SampleEntryBox::setDataReferenceIndex(std::uint16_t dataReferenceIndex)
{
    mDataReferenceIndex = dataReferenceIndex;
}

void SampleEntryBox::writeBox(BitStream& bitstr)
{
    writeBoxHeader(bitstr);

    for (int i = 0; i < RESERVED_BYTES; ++i)
    {
        bitstr.write8Bits(0); // reserved = 0
    }

    bitstr.write16Bits(mDataReferenceIndex);

    // Update the size of the movie box
    updateSize(bitstr);
}


void SampleEntryBox::parseBox(BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    for (int i = 0; i < RESERVED_BYTES; ++i)
    {
        bitstr.read8Bits(); // reserved
    }

    mDataReferenceIndex = bitstr.read16Bits();
}
