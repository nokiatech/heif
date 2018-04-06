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

#include "imagerotation.hpp"
#include "bitstream.hpp"

ImageRotation::ImageRotation()
    : Box("irot")
    , mAngle(0)
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

void ImageRotation::writeBox(BitStream& output) const
{
    writeBoxHeader(output);
    output.writeBits(0, 6);  // reserved = 0
    output.writeBits(mAngle / 90, 2);
    updateSize(output);
}

void ImageRotation::parseBox(BitStream& input)
{
    parseBoxHeader(input);
    input.readBits(6);  // reserved = 0
    mAngle = (input.readBits(2)) * 90;
}
