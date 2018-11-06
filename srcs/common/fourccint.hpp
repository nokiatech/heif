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

#ifndef FOURCCINT_HPP
#define FOURCCINT_HPP

#include <cstdint>
#include "customallocator.hpp"

/** FourCCInt is a class for holding the four-character-codes in a
 * memory- and performance-efficient manner. */
class FourCCInt
{
public:
    FourCCInt()
        : mValue(0)
    {
        // nothing
    }

    FourCCInt(std::uint32_t value)
        : mValue(value)
    {
        // nothing
    }

    /** Accept 4-character string literals and check their length at
     * compile time */
    FourCCInt(const char (&str)[5])
        : mValue(0 | (std::uint32_t(str[0]) << 24) | (std::uint32_t(str[1]) << 16) | (std::uint32_t(str[2]) << 8) | (std::uint32_t(str[3]) << 0))
    {
        // nothing
    }

    /** Checks the argument length at runtime */
    explicit FourCCInt(const String& str);

    std::uint32_t getUInt32() const
    {
        return mValue;
    }

    String getString() const;

    bool operator==(FourCCInt other) const
    {
        return mValue == other.mValue;
    }
    bool operator!=(FourCCInt other) const
    {
        return mValue != other.mValue;
    }
    bool operator>=(FourCCInt other) const
    {
        return mValue >= other.mValue;
    }
    bool operator<=(FourCCInt other) const
    {
        return mValue <= other.mValue;
    }
    bool operator>(FourCCInt other) const
    {
        return mValue > other.mValue;
    }
    bool operator<(FourCCInt other) const
    {
        return mValue < other.mValue;
    }

private:
    std::uint32_t mValue;
};

#endif  // FOURCCINT_HPP
