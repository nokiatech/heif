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

#include "cleanaperture.hpp"
#include "bitstream.hpp"

CleanAperture::CleanAperture() :
    Box("clap"),
    mWidth(),
    mHeight(),
    mHorizOffset(),
    mVertOffset()
{
}

void CleanAperture::setWidth(Fraction value)
{
    mWidth = value;
}

void CleanAperture::setHeight(Fraction value)
{
    mHeight = value;
}

void CleanAperture::setHorizOffset(Fraction value)
{
    mHorizOffset = value;
}

void CleanAperture::setVertOffset(Fraction value)
{
    mVertOffset = value;
}

CleanAperture::Fraction CleanAperture::getWidth() const
{
    return mWidth;
}

CleanAperture::Fraction CleanAperture::getHeight() const
{
    return mHeight;
}

CleanAperture::Fraction CleanAperture::getHorizOffset() const
{
    return mHorizOffset;
}

CleanAperture::Fraction CleanAperture::getVertOffset() const
{
    return mVertOffset;
}

void CleanAperture::writeBox(BitStream& output)
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

void CleanAperture::parseBox(BitStream& input)
{
    parseBoxHeader(input);
    mWidth.numerator = input.read32Bits();
    mWidth.denominator = input.read32Bits();
    mHeight.numerator = input.read32Bits();
    mHeight.denominator = input.read32Bits();
    mHorizOffset.numerator = input.read32Bits();
    mHorizOffset.denominator = input.read32Bits();
    mVertOffset.numerator = input.read32Bits();
    mVertOffset.denominator = input.read32Bits();
}

