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

#ifndef IMAGEROTATION_HPP
#define IMAGEROTATION_HPP

#include <cstdint>
#include "bbox.hpp"
#include "customallocator.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** The ImageRotation documents the rotation angle property 'irot' of the associated image(s). */
class ImageRotation : public Box
{
public:
    ImageRotation();
    virtual ~ImageRotation() = default;

    /** Set rotation angle in anti-clockwise direction.
     *  @param [in] angle Rotation angle in degrees. Valid values are 0, 90, 180 and 270. */
    void setAngle(std::uint32_t angle);

    /** @return Rotation angle in anti-clockwise direction (0, 90, 180 and 270 degrees). */
    std::uint32_t getAngle() const;

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(ISOBMFF::BitStream& output) const;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(ISOBMFF::BitStream& input);

private:
    std::uint32_t mAngle;  ///< Rotation angle in anti-clockwise direction. Valid values are 0, 90, 180 and 270 degrees.
};

#endif
