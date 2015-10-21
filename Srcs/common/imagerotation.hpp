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

#ifndef IMAGEROTATION_HPP
#define IMAGEROTATION_HPP

#include "bbox.hpp"
#include <cstdint>

class BitStream;

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

    /** Write box/property to BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(BitStream& output);

    /** Parse box/property from BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(BitStream& input);

private:
    std::uint32_t mAngle; ///< Rotation angle in anti-clockwise direction. Valid values are 0, 90, 180 and 270 degrees.
};

#endif
