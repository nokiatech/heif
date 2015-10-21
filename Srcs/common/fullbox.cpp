/* Copyright (c) 2015, Nokia Technologies Ltd.
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

#include "fullbox.hpp"

using namespace std;

FullBox::FullBox(const char* boxType, uint8_t version, uint32_t flags) :
    Box(boxType),
    mVersion(version),
    mFlags(flags)
{
    increaseSize(4);
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

void FullBox::writeFullBoxHeader(BitStream& bitstr)
{
    writeBoxHeader(bitstr);

    bitstr.write8Bits(mVersion);
    bitstr.write24Bits(mFlags);
}

void FullBox::parseFullBoxHeader(BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    mVersion = bitstr.read8Bits();
    mFlags = bitstr.read24Bits();
}
