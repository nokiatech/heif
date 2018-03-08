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

#ifndef IMAGEMIRROR_HPP
#define IMAGEMIRROR_HPP

#include <cstdint>
#include "bbox.hpp"
#include "customallocator.hpp"

namespace ISOBMFF
{
    class BitStream;
}

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

    /** @brief Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    virtual void writeBox(ISOBMFF::BitStream& output) const;

    /** @brief Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    virtual void parseBox(ISOBMFF::BitStream& input);

private:
    bool mHorizontalAxis;  ///< Mirror axis (\c true = horizontal, \c false = vertical)
};

#endif
