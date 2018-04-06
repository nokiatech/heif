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

#include "imagespatialextentsproperty.hpp"
#include "bitstream.hpp"

ImageSpatialExtentsProperty::ImageSpatialExtentsProperty()
    : FullBox("ispe", 0, 0)
    , mImageWidth(0)
    , mImageHeight(0)
{
}

ImageSpatialExtentsProperty::ImageSpatialExtentsProperty(const std::uint32_t width, const std::uint32_t height)
    : FullBox("ispe", 0, 0)
    , mImageWidth(width)
    , mImageHeight(height)
{
}

void ImageSpatialExtentsProperty::setDisplayWidth(const uint32_t width)
{
    mImageWidth = width;
}

void ImageSpatialExtentsProperty::setDisplayHeight(const uint32_t height)
{
    mImageHeight = height;
}

uint32_t ImageSpatialExtentsProperty::getDisplayWidth() const
{
    return mImageWidth;
}

uint32_t ImageSpatialExtentsProperty::getDisplayHeight() const
{
    return mImageHeight;
}

void ImageSpatialExtentsProperty::writeBox(BitStream& output) const
{
    writeFullBoxHeader(output);
    output.write32Bits(mImageWidth);
    output.write32Bits(mImageHeight);
    updateSize(output);
}

void ImageSpatialExtentsProperty::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);
    mImageWidth  = input.read32Bits();
    mImageHeight = input.read32Bits();
}
