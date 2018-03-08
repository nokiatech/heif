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

#ifndef PIXELASPECTRATIOBOX_HPP
#define PIXELASPECTRATIOBOX_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** The PixelAspectRatioBox 'pasp' describes aspect ratio of pixels in the associated image. */
class PixelAspectRatioBox : public Box
{
public:
    PixelAspectRatioBox();
    virtual ~PixelAspectRatioBox() = default;

    /**
     * Set relative width of a pixel.
     * @param width Relative width of a pixel. Unit must be same as in height set by setRelativeHeight().
     */
    void setRelativeWidth(uint32_t width);

    /**
     * Set relative height of a pixel.
     * @param height Relative Height of a pixel. Unit must be same as in width set by setRelativeWidth().
     */
    void setRelativeHeight(uint32_t height);

    /**
     * Get relative width of a pixel.
     * @return Relative width of a pixel.
     */
    uint32_t getRelativeWidth() const;

    /**
     * Get relative height of a pixel.
     * @return Relative Height of a pixel.
     */
    uint32_t getRelativeHeight() const;

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(ISOBMFF::BitStream& output) const;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(ISOBMFF::BitStream& input);

private:
    uint32_t mHSpacing;  ///< Relative width of a pixel.
    uint32_t mVSpacing;  ///< Relative height of a pixel.
};

#endif
