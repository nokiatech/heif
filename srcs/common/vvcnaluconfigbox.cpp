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

#include "vvcnaluconfigbox.hpp"

VvcNaluConfigBox::VvcNaluConfigBox()
    : FullBox("vvnC", 0, 0)
    , mLengthSizeMinusOne(0)
{
}

void VvcNaluConfigBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.writeBits(0, 6);  // reserved bits
    bitstr.writeBits(mLengthSizeMinusOne, 2);

    updateSize(bitstr);
}

void VvcNaluConfigBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    bitstr.readBits(6);  // reserved bits
    mLengthSizeMinusOne = bitstr.readBits(2);
}

std::uint8_t VvcNaluConfigBox::getLengthSizeMinusOne() const
{
    return mLengthSizeMinusOne;
}

void VvcNaluConfigBox::setLengthSizeMinusOne(const std::uint8_t lengthSizeMinusOne)
{
    mLengthSizeMinusOne = lengthSizeMinusOne;
}
