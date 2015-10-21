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

/// @brief Fix for Android: std::to_string is missing even if c++11 enabled

#if defined(ANDROID) || defined(__ANDROID__)

#ifdef ANDROID_TO_STRING_HACK
#include <sstream>
namespace std {
    static string to_string(int num)
    {
        ostringstream convert;
        convert << num;
        return convert.str();
    }
}
#endif // ANDROID_TO_STRING_HACK

#ifdef ANDROID_STOI_HACK
#include <cstdlib>
namespace std {
    static int stoi(const std::string & str)
    {
        return std::atoi(str.c_str());
    }
}
#endif // ANDROID_STOI_HACK

#endif
