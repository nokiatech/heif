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

#include "log.hpp"
#include <iostream>
#include <ostream>

Log::LogLevel Log::mLogLevel = Log::LogLevel::WARNING;

Log::Log(LogLevel level) :
    mLevel(level),
    mOut((level == LogLevel::ERROR) ? std::cerr : std::cout)
{
}

Log const& Log::operator<<(std::ostream& (_MSVC_CDECL *os)(std::ostream&)) const
{
    if (mLevel <= mLogLevel)
    {
        os(mOut);
    }

    return *this;
}

void Log::setLevel(LogLevel level)
{
    mLogLevel = level;
}

Log& _MSVC_CDECL logError()
{
    return Log::getErrorInstance();
}

Log& _MSVC_CDECL logWarning()
{
    return Log::getWarningInstance();
}

Log& _MSVC_CDECL logInfo()
{
    return Log::getInfoInstance();
}
