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

#ifndef IMAGESPATIALEXTENTSPROPERTY_HPP
#define IMAGESPATIALEXTENTSPROPERTY_HPP

#include "fullbox.hpp"
#include <cstdint>

class BitStream;

/** The ImageSpatialExtentsProperty 'ispe' declares the width and height of the associated image. */
class ImageSpatialExtentsProperty : public FullBox
{
public:
    ImageSpatialExtentsProperty();
    virtual ~ImageSpatialExtentsProperty() = default;

    /** Set image width.
     *  @param [in] width Image width in pixels. */
    void setDisplayWidth(std::uint32_t width);

    /** Set image height.
     *  @param [in] height Image height in pixels. */
    void setDisplayHeight(std::uint32_t height);

    /** Get image width.
     *  @return Image width in pixels. */
    std::uint32_t getDisplayWidth() const;

    /** Get image height.
     *  @return Image height in pixels. */
    std::uint32_t getDisplayHeight() const;

    /** Write box/property to BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(BitStream& output);

    /** Parse box/property from BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(BitStream& input);

private:
    std::uint32_t mImageWidth;  ///< Width of the reconstructed image in pixels.
    std::uint32_t mImageHeight; ///< Height of the reconstructed image in pixels.
};

#endif

