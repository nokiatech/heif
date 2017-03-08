/* Copyright (c) 2017, Nokia Technologies Ltd.
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

#ifndef IMAGEMIRROR_HPP
#define IMAGEMIRROR_HPP

#include "bbox.hpp"

class BitStream;

/** @brief The ImageMirror documents the rotation angle property 'imir' of the associated image(s). */
class ImageMirror : public Box
{
public:
    ImageMirror();
    virtual ~ImageMirror() = default;

    /** @brief Set mirror axis
     *  @param [in] horizontalAxis Specifies if horizontal axis is used for the mirroring operation (otherwise (\c false) vertical axis is used) */
    void setHorizontalAxis(const bool horizontalAxis);

    /** @brief Get mirror axis
     *  @return \c true if horizontal axis is used for the mirroring operation, otherwise (\c false) vertical axis is used */
    bool getHorizontalAxis() const;

    /** @brief Write box/property to BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(BitStream& output);

    /** @brief Parse box/property from BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(BitStream& input);

private:
    bool mHorizontalAxis; ///< Mirror axis (\c true = horizontal, \c false = vertical)
};

#endif
