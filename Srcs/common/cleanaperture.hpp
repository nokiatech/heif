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

#ifndef CLEANAPERTURE_HPP
#define CLEANAPERTURE_HPP

#include "bbox.hpp"
#include <cstdint>

/** @brief Implementation of CleanApertureBox as defined in ISO/IEC 14496-12.
 *  @details The class can be used as an item property, and can be included in VisualSampleEntry as on optional box. */
class CleanAperture : public Box
{
public:
    CleanAperture();
    virtual ~CleanAperture() = default;

    /// A helper for getting and setting class data fractional numbers
    struct Fraction
    {
        std::uint32_t numerator = 0;
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
    virtual void writeBox(BitStream& output);

    /// @see Box::parseBox()
    virtual void parseBox(BitStream& input);

private:
    Fraction mWidth;       ///< Clean aperture width, @see setWidth()
    Fraction mHeight;      ///< Clean aperture height, @see setHeight()
    Fraction mHorizOffset; ///< Clean aperture horizontal offset, @see setHorizOffset()
    Fraction mVertOffset;  ///< Clean aperture vertical offset, @see setVertOffset()
};

#endif
