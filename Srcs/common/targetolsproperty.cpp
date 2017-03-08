/* Copyright (c) 2017, Nokia Technologies Ltd.
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

#include "targetolsproperty.hpp"
#include "bitstream.hpp"

TargetOlsProperty::TargetOlsProperty() :
    FullBox("tols", 0, 0),
    mTargetOlsIndex(0)
{
}

TargetOlsProperty::TargetOlsProperty(const std::uint16_t index) :
    FullBox("tols", 0, 0),
    mTargetOlsIndex(index)
{
}

std::uint16_t TargetOlsProperty::getTargetOlsIndex() const
{
    return mTargetOlsIndex;
}

void TargetOlsProperty::setTargetOlsIndex(const std::uint16_t index)
{
    mTargetOlsIndex = index;
}

void TargetOlsProperty::writeBox(BitStream& output)
{
    writeFullBoxHeader(output);
    output.write16Bits(mTargetOlsIndex);
    updateSize(output);
}

void TargetOlsProperty::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);
    mTargetOlsIndex = input.read16Bits();
}
