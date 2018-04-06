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

#include "pixelaspectratiobox.hpp"
#include "bitstream.hpp"

PixelAspectRatioBox::PixelAspectRatioBox()
    : Box("pasp")
    , mHSpacing(1)
    , mVSpacing(1)
{
}

void PixelAspectRatioBox::setRelativeWidth(const uint32_t width)
{
    mHSpacing = width;
}

void PixelAspectRatioBox::setRelativeHeight(const uint32_t height)
{
    mVSpacing = height;
}

uint32_t PixelAspectRatioBox::getRelativeWidth() const
{
    return mHSpacing;
}

uint32_t PixelAspectRatioBox::getRelativeHeight() const
{
    return mVSpacing;
}


void PixelAspectRatioBox::writeBox(BitStream& output) const
{
    writeBoxHeader(output);

    output.write32Bits(mHSpacing);
    output.write32Bits(mVSpacing);

    updateSize(output);
}

void PixelAspectRatioBox::parseBox(BitStream& input)
{
    parseBoxHeader(input);
    mHSpacing = input.read32Bits();
    mVSpacing = input.read32Bits();
}
