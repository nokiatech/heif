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

#include "freespacebox.hpp"

FreeSpaceBox::FreeSpaceBox()
    : Box("free")
{
}

bool FreeSpaceBox::setSize(const std::uint32_t size)
{
    const auto BOX_HEADER_SIZE = 8u;
    if (size < BOX_HEADER_SIZE)
    {
        return false;
    }
    Box::setSize(size);

    return true;
}

void FreeSpaceBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    const auto boxStart = bitstr.getSize();
    writeBoxHeader(bitstr);
    const auto fillSize = boxStart + Box::getSize() - bitstr.getSize();
    for (unsigned int i = 0; i < fillSize; ++i)
    {
        bitstr.write8Bits(0);
    }
    updateSize(bitstr);
}

void FreeSpaceBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseBoxHeader(bitstr);
    while (bitstr.numBytesLeft() > 0)
    {
        bitstr.read8Bits();
    }
}
