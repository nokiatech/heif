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

#ifndef FOURCCINT_HPP
#define FOURCCINT_HPP

#include <cstdint>
#include <iostream>
#include <string>

/** FourCCInt is a class for holding the four-character-codes in a
 * memory- and performance-efficient manner. */
class FourCCInt
{
public:
    FourCCInt() : mValue(0)
    {
        // nothing
    }

    FourCCInt(std::uint32_t value) : mValue(value)
    {
        // nothing
    }

    /** Accept 4-character string literals and check their length at
     * compile time */
    FourCCInt(const char(&str)[5]) :
        mValue(0
               | (std::uint32_t(str[0]) << 24)
               | (std::uint32_t(str[1]) << 16)
               | (std::uint32_t(str[2]) <<  8)
               | (std::uint32_t(str[3]) <<  0))
    {
        // nothing
    }

    /** Checks the argument length at runtime */
    explicit FourCCInt(const std::string& str);

    std::uint32_t getUInt32() const
    {
        return mValue;
    }

    std::string getString() const;

    bool operator==(FourCCInt other) const {
        return mValue == other.mValue;
    }
    bool operator!=(FourCCInt other) const {
        return mValue != other.mValue;
    }
    bool operator>=(FourCCInt other) const {
        return mValue >= other.mValue;
    }
    bool operator<=(FourCCInt other) const {
        return mValue <= other.mValue;
    }
    bool operator>(FourCCInt other) const {
        return mValue > other.mValue;
    }
    bool operator<(FourCCInt other) const {
        return mValue < other.mValue;
    }
private:
    std::uint32_t mValue;
};

std::ostream& operator<<(std::ostream& stream, FourCCInt fourcc);

#endif // FOURCCINT_HPP
