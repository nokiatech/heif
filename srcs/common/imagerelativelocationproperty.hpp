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

#ifndef IMAGERELATIVELOCATIONBOX_HPP
#define IMAGERELATIVELOCATIONBOX_HPP

#include <cstdint>
#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

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

    /** Write box/property to ISOBMFF::BitStream
     *  @see Box::writeBox() */
    virtual void writeBox(ISOBMFF::BitStream& output) const;

    /** Parse box/property from ISOBMFF::BitStream
     *  @see Box::parseBox() */
    virtual void parseBox(ISOBMFF::BitStream& input);

private:
    std::uint32_t mHorizontalOffset;  ///< 0-based horizontal offset in pixels of the left-most pixel column.
    std::uint32_t mVerticalOffset;    ///< 0-based vertical offset in pixels of the top-most pixel row.
};

#endif
