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

#ifndef LOG_HPP
#define LOG_HPP
#include <ostream>

/** @brief Helper class for Logging information during execution.
 *  @details Log levels can be Error, Warning and Info **/
class Log
{
public:
    enum class LogLevel
    {
        ERROR,
        WARNING,
        INFO
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

    /// Set log level of output
    static void setLevel(LogLevel level);

    /// Handle logging to target ostream
    template<typename T>
    const Log& operator<<(const T& logMessage) const
    {
        if (mLevel <= mLogLevel)
        {
            mOut << logMessage;
        }
        return *this;
    }

    /// Overload for manipulators
    Log const& operator<<(std::ostream& (*os)(std::ostream&)) const;

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

#endif // LOG_HPP
