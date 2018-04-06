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

#include "rawpropertybox.hpp"
#include "bitstream.hpp"

RawPropertyBox::RawPropertyBox()
    : Box(0)  // Actual box type will be provided along raw data.
{
}

Vector<std::uint8_t> RawPropertyBox::getData() const
{
    return mData;
}

void RawPropertyBox::setData(const Vector<std::uint8_t>& data)
{
    mData = data;
}

void RawPropertyBox::writeBox(BitStream& bitstream) const
{
    bitstream.write8BitsArray(mData, mData.size());
}

void RawPropertyBox::parseBox(BitStream& bitstream)
{
    const auto startpos = bitstream.getPos();
    Box::parseBoxHeader(bitstream);
    bitstream.setPosition(startpos);
    const auto boxSize = Box::getSize();
    bitstream.read8BitsArray(mData, boxSize);
}
