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

#include "imagemirror.hpp"
#include "bitstream.hpp"

ImageMirror::ImageMirror() :
    Box("imir"),
    mHorizontalAxis(true)
{
}

void ImageMirror::setHorizontalAxis(const bool horizontalAxis)
{
    mHorizontalAxis = horizontalAxis;
}

bool ImageMirror::getHorizontalAxis() const
{
    return mHorizontalAxis;
}

void ImageMirror::writeBox(BitStream& output)
{
    writeBoxHeader(output);
    output.writeBits(0, 7); // reserved = 0
    output.writeBits(mHorizontalAxis, 1);
    updateSize(output);
}

void ImageMirror::parseBox(BitStream& input)
{
    parseBoxHeader(input);
    input.readBits(7);  // reserved = 0
    mHorizontalAxis = input.readBits(1);
}
