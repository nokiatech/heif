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

#include "log.hpp"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

#include "heiferrorinfo.h"

Log::LogLevel Log::mLogLevel = Log::LogLevel::NONE;

namespace
{
    // Trace sink (HEIF::setTraceSink). Two atomics rather than one packed
    // pair so we stay portable on 32-bit targets; same publish ordering as
    // the error sink.
    std::atomic<HEIF::TraceSink> gTraceSink{nullptr};
    std::atomic<void*>           gTraceSinkUser{nullptr};

    // Per-thread accumulator: deliver one complete line per callback
    // regardless of how operator<< fragments writes at the streambuf level.
    std::string& tlsTraceBuf()
    {
        thread_local std::string buf;
        return buf;
    }

    void dispatchLine(HEIF::TraceSink sink, const char* line)
    {
        void* user = gTraceSinkUser.load(std::memory_order_acquire);
        sink(line, user);
    }

    // Either forwards every byte to the fallback ostream when no sink is
    // installed, or accumulates into the per-thread buffer and dispatches
    // on '\n' when one is.
    class TraceStreambuf : public std::streambuf
    {
    public:
        explicit TraceStreambuf(std::ostream& fallback)
            : mFallback(fallback)
        {
        }

    protected:
        int_type overflow(int_type ch) override
        {
            if (auto sink = gTraceSink.load(std::memory_order_acquire))
            {
                if (traits_type::eq_int_type(ch, traits_type::eof()))
                {
                    return traits_type::not_eof(ch);
                }
                const char c = static_cast<char>(ch);
                std::string& buf = tlsTraceBuf();
                if (c == '\n')
                {
                    dispatchLine(sink, buf.c_str());
                    buf.clear();
                }
                else
                {
                    buf.push_back(c);
                }
                return ch;
            }
            return mFallback.rdbuf()->sputc(static_cast<char>(ch));
        }

        std::streamsize xsputn(const char* s, std::streamsize n) override
        {
            if (auto sink = gTraceSink.load(std::memory_order_acquire))
            {
                std::string&    buf       = tlsTraceBuf();
                std::streamsize remaining = n;
                const char*     cursor    = s;
                while (remaining > 0)
                {
                    const char* nl = static_cast<const char*>(
                        std::memchr(cursor, '\n', static_cast<std::size_t>(remaining)));
                    if (nl == nullptr)
                    {
                        buf.append(cursor, static_cast<std::size_t>(remaining));
                        break;
                    }
                    buf.append(cursor, static_cast<std::size_t>(nl - cursor));
                    dispatchLine(sink, buf.c_str());
                    buf.clear();
                    const std::streamsize consumed = (nl - cursor) + 1;
                    cursor += consumed;
                    remaining -= consumed;
                }
                return n;
            }
            return mFallback.rdbuf()->sputn(s, n);
        }

    private:
        std::ostream& mFallback;
    };

    // Magic-static singletons: thread-safe init, ordered after std::cout /
    // std::cerr (kept alive by ios_base::Init for program lifetime).
    std::ostream& cerrRoute()
    {
        static TraceStreambuf buf(std::cerr);
        static std::ostream   stream(&buf);
        return stream;
    }

    std::ostream& coutRoute()
    {
        static TraceStreambuf buf(std::cout);
        static std::ostream   stream(&buf);
        return stream;
    }
}  // namespace

Log::Log(LogLevel level)
    : mLevel(level)
    // Route through TraceStreambuf so an installed HEIF::setTraceSink
    // intercepts on the '\n' boundary; with no sink installed bytes pass
    // through to std::cerr / std::cout unchanged.
    , mOut((level == LogLevel::ERROR) ? cerrRoute() : coutRoute())
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

// These three functions are declared HEIF_DLL_PUBLIC in heiferrorinfo.h
// (dllexport when HEIF_BUILDING_DLL is defined) but compiled here inside
// the static `common` library where HEIF_BUILDING_DLL is not set.  When
// the shared library links common.lib the declaration/definition linkage
// specs mismatch, producing MSVC C4273.  The mismatch is benign -- the
// linker resolves correctly -- so we suppress the warning locally.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4273)
#endif

namespace HEIF
{
    void setTraceSink(TraceSink sink, void* user)
    {
        // Publish user before sink so a concurrent dispatch never sees a
        // new sink paired with a stale user.
        gTraceSinkUser.store(user, std::memory_order_release);
        gTraceSink.store(sink, std::memory_order_release);
    }

    bool hasTraceSink()
    {
        return gTraceSink.load(std::memory_order_acquire) != nullptr;
    }

    void stderrTraceSink(const char* line, void* user)
    {
        std::FILE* fp = (user != nullptr) ? static_cast<std::FILE*>(user) : stderr;
        std::fputs("[heif:trace] ", fp);
        std::fputs((line != nullptr) ? line : "", fp);
        std::fputc('\n', fp);
    }
}  // namespace HEIF

#ifdef _MSC_VER
#pragma warning(pop)
#endif
