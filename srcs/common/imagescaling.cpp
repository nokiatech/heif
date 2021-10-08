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

#include "imagescaling.hpp"

#include "bitstream.hpp"

ImageScaling::ImageScaling()
    : FullBox("iscl", 0, 0)
    , mTargetWidth()
    , mTargetHeight()
{
}

void ImageScaling::setWidth(Fraction value)
{
    mTargetWidth = value;
}

void ImageScaling::setHeight(Fraction value)
{
    mTargetHeight = value;
}

ImageScaling::Fraction ImageScaling::getWidth() const
{
    return mTargetWidth;
}

ImageScaling::Fraction ImageScaling::getHeight() const
{
    return mTargetHeight;
}

void ImageScaling::writeBox(BitStream& output) const
{
    writeFullBoxHeader(output);
    output.write16Bits(mTargetWidth.numerator);
    output.write16Bits(mTargetWidth.denominator);
    output.write16Bits(mTargetHeight.numerator);
    output.write16Bits(mTargetHeight.denominator);
    updateSize(output);
}

void ImageScaling::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);
    mTargetWidth.numerator    = input.read16Bits();
    mTargetWidth.denominator  = input.read16Bits();
    mTargetHeight.numerator   = input.read16Bits();
    mTargetHeight.denominator = input.read16Bits();
}
