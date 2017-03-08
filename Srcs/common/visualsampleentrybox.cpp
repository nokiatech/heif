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

#include "visualsampleentrybox.hpp"
#include "cleanaperture.hpp"
#include "log.hpp"

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <string>

/// Length of compressorname string in VisualSampleEntry class
static const unsigned int COMPRESSORNAME_STRING_LENGTH = 31;

VisualSampleEntryBox::VisualSampleEntryBox(const FourCCInt codingName, const std::string& compressorName) :
    SampleEntryBox(codingName),
    mWidth(0),
    mHeight(0),
    mCompressorName(compressorName),
    mClap(nullptr)
{
}

void VisualSampleEntryBox::setWidth(uint32_t width)
{
    mWidth = width;
}

uint32_t VisualSampleEntryBox::getWidth() const
{
    return mWidth;
}

void VisualSampleEntryBox::setHeight(uint32_t height)
{
    mHeight = height;
}
uint32_t VisualSampleEntryBox::getHeight() const
{
    return mHeight;
}

const CleanAperture* VisualSampleEntryBox::getClap() const
{
    return mClap.get();
}

void VisualSampleEntryBox::writeBox(BitStream& bitstr)
{
    SampleEntryBox::writeBox(bitstr);

    bitstr.write16Bits(0); // pre_defined = 0
    bitstr.write16Bits(0); // reserved = 0
    bitstr.write32Bits(0); // reserved = 0
    bitstr.write32Bits(0); // reserved = 0
    bitstr.write32Bits(0); // reserved = 0
    bitstr.write16Bits(mWidth); // width
    bitstr.write16Bits(mHeight); // height
    bitstr.write32Bits(0x00480000); // horizresolution 72 dpi
    bitstr.write32Bits(0x00480000); // vertresolution 72 dpi
    bitstr.write32Bits(0); // reserved = 0
    bitstr.write16Bits(1); // frame_count = 1

    assert(mCompressorName.length() <= COMPRESSORNAME_STRING_LENGTH);

    // Length-byte prefixed compressor name padded to 32 bytes total.
    bitstr.write8Bits(mCompressorName.length());
    mCompressorName.resize(COMPRESSORNAME_STRING_LENGTH, '\0'); // Make fixed length
    bitstr.writeString(mCompressorName); // Write entire string buffer, including padding zeros

    bitstr.write16Bits(0x0018); // depth
    bitstr.write16Bits(-1);     // pre_defined

    // Update the size of the movie box
    updateSize(bitstr);
}

void VisualSampleEntryBox::parseBox(BitStream& bitstr)
{
    SampleEntryBox::parseBox(bitstr);

    bitstr.read16Bits(); // pre_defined
    bitstr.read16Bits(); // reserved
    bitstr.read32Bits(); // predefined
    bitstr.read32Bits(); // predefined
    bitstr.read32Bits(); // predefined
    mWidth = bitstr.read16Bits(); // width
    mHeight = bitstr.read16Bits(); // height
    bitstr.read32Bits(); // horizontal resolution
    bitstr.read32Bits(); // vertical resolution
    bitstr.read32Bits(); // reserved
    bitstr.read16Bits(); // frame_count

    const uint8_t compressorNameLength = bitstr.read8Bits();
    if (compressorNameLength > COMPRESSORNAME_STRING_LENGTH)
    {
        throw std::runtime_error("Too long compressorname string length read from VisualSampleEntry (>31 bytes).");
    }
    std::string codecName;
    bitstr.readStringWithLen(codecName, compressorNameLength); // compressor name
    for (unsigned int i = compressorNameLength; i < COMPRESSORNAME_STRING_LENGTH; ++i)
    {
        bitstr.read8Bits(); // discard padding
    }

    bitstr.read16Bits(); // depth
    bitstr.read16Bits(); // pre_defined

    // Read the optional clap box, if present
    if (bitstr.numBytesLeft() > 0)
    {
        const unsigned int startOffset = bitstr.getPos();
        FourCCInt boxType;
        BitStream subBitStream = bitstr.readSubBoxBitStream(boxType);
        if (boxType == "clap")
        {
            const auto clap = std::make_shared<CleanAperture>();
            clap->parseBox(subBitStream);
            mClap = clap;
        }
        else
        {
            // It was not 'clap', so the contained box probably belongs to the box box extending VisualSampleEntryBox.
            // Reset bitstream position so it will be possible to read the whole extending box.
            bitstr.setPosition(startOffset);
        }
    }
}

