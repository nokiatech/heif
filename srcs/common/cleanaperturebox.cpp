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

#include "cleanaperturebox.hpp"
#include "bitstream.hpp"

CleanApertureBox::CleanApertureBox()
    : Box("clap")
    , mWidth()
    , mHeight()
    , mHorizOffset()
    , mVertOffset()
{
}

void CleanApertureBox::setWidth(Fraction value)
{
    mWidth = value;
}

void CleanApertureBox::setHeight(Fraction value)
{
    mHeight = value;
}

void CleanApertureBox::setHorizOffset(Fraction value)
{
    mHorizOffset = value;
}

void CleanApertureBox::setVertOffset(Fraction value)
{
    mVertOffset = value;
}

CleanApertureBox::Fraction CleanApertureBox::getWidth() const
{
    return mWidth;
}

CleanApertureBox::Fraction CleanApertureBox::getHeight() const
{
    return mHeight;
}

CleanApertureBox::Fraction CleanApertureBox::getHorizOffset() const
{
    return mHorizOffset;
}

CleanApertureBox::Fraction CleanApertureBox::getVertOffset() const
{
    return mVertOffset;
}

void CleanApertureBox::writeBox(BitStream& output) const
{
    writeBoxHeader(output);
    output.write32Bits(mWidth.numerator);
    output.write32Bits(mWidth.denominator);
    output.write32Bits(mHeight.numerator);
    output.write32Bits(mHeight.denominator);
    output.write32Bits(mHorizOffset.numerator);
    output.write32Bits(mHorizOffset.denominator);
    output.write32Bits(mVertOffset.numerator);
    output.write32Bits(mVertOffset.denominator);
    updateSize(output);
}

void CleanApertureBox::parseBox(BitStream& input)
{
    parseBoxHeader(input);
    mWidth.numerator         = input.read32Bits();
    mWidth.denominator       = input.read32Bits();
    mHeight.numerator        = input.read32Bits();
    mHeight.denominator      = input.read32Bits();
    mHorizOffset.numerator   = input.read32Bits();
    mHorizOffset.denominator = input.read32Bits();
    mVertOffset.numerator    = input.read32Bits();
    mVertOffset.denominator  = input.read32Bits();
}
