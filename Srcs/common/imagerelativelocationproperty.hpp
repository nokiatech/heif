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

#ifndef IMAGERELATIVELOCATIONBOX_HPP
#define IMAGERELATIVELOCATIONBOX_HPP

#include "fullbox.hpp"
#include <cstdint>

class BitStream;

/** The ImageRelativeLocationProperty documents the relative location property 'rloc' of the associated image(s). */
class ImageRelativeLocationProperty : public FullBox
{
public:
    ImageRelativeLocationProperty();
    virtual ~ImageRelativeLocationProperty() = default;

    /** Set new horizontal offset of the image.
     *  @param [in] offset Horizontal offset of the image in the reconstructed image, counted from left-most pixel column. */
    void setHorizontalOffset(std::uint32_t offset);

    /** Get horizontal offset of the image.
     *  @return Horizontal offset of the image in the reconstructed image, counted from left-most pixel column. */
    std::uint32_t getHorizontalOffset() const;

    /** Set new vertical offset of the image.
     *  @param [in] offset Vertical offset of the image in the reconstructed image, counted from top-most pixel column. */
    void setVerticalOffset(std::uint32_t offset);

    /** Get vertical offset of the image.
     *  @return Vertical offset of the image in the reconstructed image, counted from top-most pixel column. */
    std::uint32_t getVerticalOffset() const;

    /** Write box/property to BitStream
     *  @see Box::writeBox() */
    virtual void writeBox(BitStream& output);

    /** Parse box/property from BitStream
     *  @see Box::parseBox() */
    virtual void parseBox(BitStream& input);

private:
    std::uint32_t mHorizontalOffset; ///< 0-based horizontal offset in pixels of the left-most pixel column.
    std::uint32_t mVerticalOffset;   ///< 0-based vertical offset in pixels of the top-most pixel row.
};

#endif
