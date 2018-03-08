/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

#ifndef IMAGESPATIALEXTENTSPROPERTY_HPP
#define IMAGESPATIALEXTENTSPROPERTY_HPP

#include <cstdint>
#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** The ImageSpatialExtentsProperty 'ispe' declares the width and height of the associated image. */
class ImageSpatialExtentsProperty : public FullBox
{
public:
    ImageSpatialExtentsProperty();
    ImageSpatialExtentsProperty(std::uint32_t width, std::uint32_t height);
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

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(ISOBMFF::BitStream& output) const;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(ISOBMFF::BitStream& input);

private:
    std::uint32_t mImageWidth;   ///< Width of the reconstructed image in pixels.
    std::uint32_t mImageHeight;  ///< Height of the reconstructed image in pixels.
};

#endif
