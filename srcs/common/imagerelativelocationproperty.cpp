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

#include "imagerelativelocationproperty.hpp"
#include "bitstream.hpp"

ImageRelativeLocationProperty::ImageRelativeLocationProperty()
    : FullBox("rloc", 0, 0)
    , mHorizontalOffset(0)
    , mVerticalOffset(0)
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

void ImageRelativeLocationProperty::writeBox(BitStream& output) const
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
    mVerticalOffset   = input.read32Bits();
}
