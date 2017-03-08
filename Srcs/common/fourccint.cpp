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

#include "fourccint.hpp"
#include <stdexcept>

FourCCInt::FourCCInt(const std::string& str)
{
    if (str.size() != 4)
    {
        throw std::runtime_error("FourCCInt given an String argument not exactly 4 characters long");
    }
    mValue = 0
        | (std::uint32_t(str[0]) << 24)
        | (std::uint32_t(str[1]) << 16)
        | (std::uint32_t(str[2]) <<  8)
        | (std::uint32_t(str[3]) <<  0);
}

std::string FourCCInt::getString() const
{
    std::string str(4, ' ');
    str[0] = char((mValue >> 24) & 0xff);
    str[1] = char((mValue >> 16) & 0xff);
    str[2] = char((mValue >>  8) & 0xff);
    str[3] = char((mValue >>  0) & 0xff);
    return str;
}

std::ostream& operator<<(std::ostream& stream, FourCCInt fourcc)
{
    return stream << fourcc.getString();
}
