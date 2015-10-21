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

#include "imagerotation.hpp"
#include "bitstream.hpp"

ImageRotation::ImageRotation() :
    Box("irot"),
    mAngle(0)
{
}

void ImageRotation::setAngle(const uint32_t angle)
{
    mAngle = angle;
}

uint32_t ImageRotation::getAngle() const
{
    return mAngle;
}

void ImageRotation::writeBox(BitStream& output)
{
    writeBoxHeader(output);
    output.writeBits(0, 6); // reserved = 0
    output.writeBits(mAngle / 90, 2);
    updateSize(output);
}

void ImageRotation::parseBox(BitStream& input)
{
    parseBoxHeader(input);
    input.readBits(6);  // reserved = 0
    mAngle = (input.readBits(2)) * 90;
}
