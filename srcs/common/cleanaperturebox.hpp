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

#ifndef CLEANAPERTUREBOX_HPP
#define CLEANAPERTUREBOX_HPP

#include <cstdint>
#include "bbox.hpp"
#include "customallocator.hpp"

/** @brief Implementation of CleanApertureBox as defined in ISO/IEC 14496-12.
 *  @details The class can be used as an item property, and can be included in VisualSampleEntry as on optional box. */
class CleanApertureBox : public Box
{
public:
    CleanApertureBox();
    virtual ~CleanApertureBox() = default;

    /// A helper for getting and setting class data fractional numbers
    struct Fraction
    {
        std::uint32_t numerator   = 0;
        std::uint32_t denominator = 1;
    };

    /** @param [in] value Fractional number defining the clean aperture width, counted in pixels. */
    void setWidth(Fraction value);

    /** @param [in] value Fractional number defining the clean aperture height, counted in pixels. */
    void setHeight(Fraction value);

    /** @param [in] value Fractional number defining the horizontal offset of clean aperture
                          center minus (width-1)/2. Typically 0. */
    void setHorizOffset(Fraction value);

    /** @param [in] value Fractional number defining the vertical offset of clean aperture
                          center minus (width-1)/2. Typically 0. */
    void setVertOffset(Fraction value);

    /** Get width, @see setWidth() */
    Fraction getWidth() const;

    /** Get width, @see setHeight() */
    Fraction getHeight() const;

    /** Get width, @see setHorizOffset() */
    Fraction getHorizOffset() const;

    /** Get width, @see setVertOffset() */
    Fraction getVertOffset() const;

    /// @see Box::writeBox()
    virtual void writeBox(ISOBMFF::BitStream& output) const;

    /// @see Box::parseBox()
    virtual void parseBox(ISOBMFF::BitStream& input);

private:
    Fraction mWidth;        ///< Clean aperture width, @see setWidth()
    Fraction mHeight;       ///< Clean aperture height, @see setHeight()
    Fraction mHorizOffset;  ///< Clean aperture horizontal offset, @see setHorizOffset()
    Fraction mVertOffset;   ///< Clean aperture vertical offset, @see setVertOffset()
};

#endif
