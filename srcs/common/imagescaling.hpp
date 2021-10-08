/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef IMAGESCALING_HPP
#define IMAGESCALING_HPP

#include <cstdint>

#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** @brief The ImageScaling documents the Image scaling property 'iscl' of the associated image(s). */
class ImageScaling : public FullBox
{
public:
    ImageScaling();
    ~ImageScaling() override = default;

    /// A helper for getting and setting class data fractional numbers
    struct Fraction
    {
        std::uint16_t numerator   = 1;
        std::uint16_t denominator = 1;
    };

    /** @param [in] value Fractional number defining the scaling ratio in the horizontal dimension. Value 0 is not
     * allowed as numerator or denominator. */
    void setWidth(Fraction value);

    /** @param [in] value Fractional number defining the scaling ratio in the vertical dimension. Value 0 is not
     * allowed as numerator or denominator. */
    void setHeight(Fraction value);

    /** Get width, @see setWidth() */
    Fraction getWidth() const;

    /** Get width, @see setHeight() */
    Fraction getHeight() const;

    /** @brief Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    void writeBox(ISOBMFF::BitStream& output) const override;

    /** @brief Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    void parseBox(ISOBMFF::BitStream& input) override;

private:
    Fraction mTargetWidth;   ///< The scaling ratio in the horizontal dimension. Value 0 is not allowed.
    Fraction mTargetHeight;  ///< The scaling ratio in the vertical dimension. Value 0 is not allowed.
};

#endif
