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

#include "fullbox.hpp"

using namespace std;

FullBox::FullBox(FourCCInt boxType, uint8_t version, uint32_t flags)
    : Box(boxType)
    , mVersion(version)
    , mFlags(flags)
{
}

void FullBox::setVersion(uint8_t version)
{
    mVersion = version;
}

uint8_t FullBox::getVersion() const
{
    return mVersion;
}

void FullBox::setFlags(uint32_t flags)
{
    mFlags = flags;
}

uint32_t FullBox::getFlags() const
{
    return mFlags;
}

void FullBox::writeFullBoxHeader(ISOBMFF::BitStream& bitstr) const
{
    writeBoxHeader(bitstr);

    bitstr.write8Bits(mVersion);
    bitstr.write24Bits(mFlags);
}

void FullBox::parseFullBoxHeader(ISOBMFF::BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    mVersion = bitstr.read8Bits();
    mFlags   = bitstr.read24Bits();
}
