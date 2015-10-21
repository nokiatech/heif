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

#include "imagerelativelocationproperty.hpp"
#include "bitstream.hpp"

ImageRelativeLocationProperty::ImageRelativeLocationProperty() :
    FullBox("rloc", 0, 0),
    mHorizontalOffset(0),
    mVerticalOffset(0)
{
}

void ImageRelativeLocationProperty::setHorizontalOffset(const uint32_t offset)
{
    mHorizontalOffset = offset;
}

uint32_t ImageRelativeLocationProperty::getHorizontalOffset() const
{
    return mHorizontalOffset;
}

void ImageRelativeLocationProperty::setVerticalOffset(const uint32_t offset)
{
    mVerticalOffset = offset;
}

uint32_t ImageRelativeLocationProperty::getVerticalOffset() const
{
    return mVerticalOffset;
}

void ImageRelativeLocationProperty::writeBox(BitStream& output)
{
    writeFullBoxHeader(output);
    output.write32Bits(mHorizontalOffset);
    output.write32Bits(mVerticalOffset);
    updateSize(output);
}

void ImageRelativeLocationProperty::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);
    mHorizontalOffset = input.read32Bits();
    mVerticalOffset = input.read32Bits();
}
