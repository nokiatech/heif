/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
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

    // NOLINTNEXTLINE(google-explicit-constructor): implicit so call sites can pass uint32_t literals directly.
    FourCCInt(std::uint32_t value) noexcept
        : mValue(value)
    {
        // nothing
    }

    /** Accept 4-character string literals and check their length at
     * compile time */
    // NOLINTNEXTLINE(google-explicit-constructor): implicit so Box("foo") and FourCCInt("foo") == x compile.
    FourCCInt(const char (&str)[5])
        : mValue(0 | (static_cast<std::uint32_t>(str[0]) << 24) | (static_cast<std::uint32_t>(str[1]) << 16) | (static_cast<std::uint32_t>(str[2]) << 8) |
                 (static_cast<std::uint32_t>(str[3]) << 0))
    {
        // nothing
    }

    /** Checks the argument length at runtime */
    // NOLINTNEXTLINE(google-explicit-constructor): implicit for symmetry with the const char[5] ctor.
    FourCCInt(const String& str);

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
