/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include <stdexcept>

#include "fourccint.hpp"

FourCCInt::FourCCInt(const String& str)
{
    if (str.size() != 4)
    {
        throw RuntimeError("FourCCInt given an String argument not exactly 4 characters long");
    }
    mValue = 0 | (std::uint32_t(str[0]) << 24) | (std::uint32_t(str[1]) << 16) | (std::uint32_t(str[2]) << 8) |
             (std::uint32_t(str[3]) << 0);
}

String FourCCInt::getString() const
{
    String str(4, ' ');
    str[0] = char((mValue >> 24) & 0xff);
    str[1] = char((mValue >> 16) & 0xff);
    str[2] = char((mValue >> 8) & 0xff);
    str[3] = char((mValue >> 0) & 0xff);
    return str;
}