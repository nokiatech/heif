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

#include "log.hpp"
#include <iostream>
#include <ostream>

Log::LogLevel Log::mLogLevel = Log::LogLevel::NONE;

Log::Log(LogLevel level)
    : mLevel(level)
    , mOut((level == LogLevel::ERROR) ? std::cerr : std::cout)
{
}

Log const& Log::operator<<(std::ostream& (*os)(std::ostream&) ) const
{
    if (mLevel >= mLogLevel)
    {
        os(mOut);
    }

    return *this;
}

void Log::setLevel(LogLevel level)
{
    mLogLevel = level;
}

Log& logError()
{
    return Log::getErrorInstance();
}

Log& logWarning()
{
    return Log::getWarningInstance();
}

Log& logInfo()
{
    return Log::getInfoInstance();
}

Log& logPanic()
{
    return Log::getPanicInstance();
}

Log& logNone()
{
    return Log::getNoneInstance();
}
