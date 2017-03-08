/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#include "imagespatialextentsproperty.hpp"
#include "bitstream.hpp"

ImageSpatialExtentsProperty::ImageSpatialExtentsProperty() :
    FullBox("ispe", 0, 0),
    mImageWidth(0),
    mImageHeight(0)
{
}

ImageSpatialExtentsProperty::ImageSpatialExtentsProperty(const std::uint32_t width, const std::uint32_t height) :
    FullBox("ispe", 0, 0),
    mImageWidth(width),
    mImageHeight(height)
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

void ImageSpatialExtentsProperty::writeBox(BitStream& output)
{
    writeFullBoxHeader(output);
    output.write32Bits(mImageWidth);
    output.write32Bits(mImageHeight);
    updateSize(output);
}

void ImageSpatialExtentsProperty::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);
    mImageWidth = input.read32Bits();
    mImageHeight = input.read32Bits();
}
