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

#ifndef LOG_HPP
#define LOG_HPP
#include <ostream>
#include "customallocator.hpp"

/** @brief Helper class for Logging information during execution.
 *  @details Log levels can be Error, Warning and Info */
class Log
{
public:
    enum class LogLevel
    {
        INFO,
        WARNING,
        ERROR,
        PANIC,
        NONE
    };

    /// Get error logger singleton reference
    static Log& getErrorInstance()
    {
        static Log errorLogger(LogLevel::ERROR);
        return errorLogger;
    }

    /// Get warning logger singleton reference
    static Log& getWarningInstance()
    {
        static Log warningLogger(LogLevel::WARNING);
        return warningLogger;
    }

    /// Get info logger singleton reference
    static Log& getInfoInstance()
    {
        static Log infoLogger(LogLevel::INFO);
        return infoLogger;
    }

    /// Get info logger singleton reference
    static Log& getPanicInstance()
    {
        static Log infoLogger(LogLevel::PANIC);
        return infoLogger;
    }

    // Get info logger singleton reference
    static Log& getNoneInstance()
    {
        static Log infoLogger(LogLevel::NONE);
        return infoLogger;
    }

    /// Set log level of output
    static void setLevel(LogLevel level);

    /// Handle logging to target ostream
    template <typename T>
    const Log& operator<<(const T& logMessage) const
    {
        if (mLevel >= mLogLevel)
        {
            mOut << logMessage;
        }
        return *this;
    }

    /// Overload for manipulators
    Log const& operator<<(std::ostream& (*os)(std::ostream&) ) const;

private:
    Log();
    Log(LogLevel level);

    /// Log level of output
    static LogLevel mLogLevel;

    /// Log level
    LogLevel mLevel;

    /// Output stream
    std::ostream& mOut;
};

/// Convenience function to get error logger singleton reference
Log& logError();

/// Convenience function to get warning logger singleton reference
Log& logWarning();

/// Convenience function to get info logger singleton reference
Log& logInfo();

/// Convenience function to get info logger singleton reference
Log& logPanic();

/// Convenience function to get info logger singleton reference
Log& logNone();

#endif  // LOG_HPP
