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

#include "imagemirror.hpp"
#include "bitstream.hpp"

ImageMirror::ImageMirror()
    : Box("imir")
    , mHorizontalAxis(true)
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

void ImageMirror::writeBox(BitStream& output) const
{
    writeBoxHeader(output);
    output.writeBits(0, 7);  // reserved = 0
    output.writeBits(mHorizontalAxis, 1);
    updateSize(output);
}

void ImageMirror::parseBox(BitStream& input)
{
    parseBoxHeader(input);
    input.readBits(7);  // reserved = 0
    mHorizontalAxis = (input.readBits(1) != 0);
}
