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

#include "pixelinformationproperty.hpp"
#include "bitstream.hpp"

PixelInformationProperty::PixelInformationProperty()
    : FullBox("pixi", 0, 0)
{
}

const Vector<std::uint8_t>& PixelInformationProperty::getBitsPerChannels() const
{
    return mBitsPerChannel;
}

void PixelInformationProperty::setBitsPerChannels(const Vector<std::uint8_t>& bitsPerChannel)
{
    mBitsPerChannel = bitsPerChannel;
}

void PixelInformationProperty::writeBox(BitStream& output) const
{
    writeFullBoxHeader(output);
    output.write8Bits(static_cast<uint8_t>(mBitsPerChannel.size()));
    for (const auto bitsPerChannel : mBitsPerChannel)
    {
        output.write8Bits(bitsPerChannel);
    }
    updateSize(output);
}

void PixelInformationProperty::parseBox(BitStream& input)
{
    mBitsPerChannel.clear();

    parseFullBoxHeader(input);
    const auto numChannels = input.read8Bits();
    mBitsPerChannel.reserve(numChannels);
    for (unsigned int i = 0; i < numChannels; ++i)
    {
        mBitsPerChannel.push_back(input.read8Bits());
    }
}
