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

#include "heifstreaminternal.hpp"
#include <iostream>
#include "customallocator.hpp"
#include "log.hpp"

namespace HEIF
{
#define TRACE(x) \
    do           \
    {            \
    } while (0)
    //#define TRACE(x) x

    InternalStream::InternalStream(StreamInterface* stream)
        : m_stream(stream)
        , m_error(false)
        , m_eof(false)
    {
        m_error = !stream || !stream->absoluteSeek(0);
    }

    InternalStream::~InternalStream()
    {
        // nothing
    }

    void InternalStream::read(char* buffer, StreamInterface::offset_t size_)
    {
        TRACE(logInfo() << "Reading " << size_ << " at " << m_stream->tell() << " ");
        StreamInterface::offset_t got = m_stream->read(buffer, size_);
        if (got < size_)
        {
            TRACE(logInfo() << "FAIL!" << std::endl);
            m_eof   = true;
            m_error = true;
        }
        else
        {
            TRACE(logInfo() << "OK!" << std::endl);
        }
    }

    int InternalStream::get()
    {
        char ch;
        TRACE(logInfo() << "Getting at " << m_stream->tell() << " ");
        StreamInterface::offset_t got = m_stream->read(&ch, sizeof(ch));
        if (got)
        {
            TRACE(logInfo() << "OK!" << std::endl);
            return static_cast<unsigned char>(ch);
        }
        else
        {
            TRACE(logInfo() << "FAIL!" << std::endl);
            m_eof = true;
            return 0;
        }
    }

    bool InternalStream::peekEof()
    {
        char buffer;
        TRACE(logInfo() << "Peek EOF at " << m_stream->tell() << " ");
        auto was = m_stream->tell();
        if (m_stream->read(&buffer, sizeof(buffer)) == 0)
        {
            TRACE(logInfo() << "EOF!" << std::endl);
            return true;
        }
        else
        {
            TRACE(logInfo() << "No EOF!" << std::endl);
            m_stream->absoluteSeek(was);
            return false;
        }
    }

    void InternalStream::seek(StreamInterface::offset_t offset)
    {
        TRACE(logInfo() << "Seeking to " << offset << " at " << m_stream->tell() << " ");
        if (!m_stream->absoluteSeek(offset))
        {
            TRACE(logInfo() << "FAIL!" << std::endl);
            m_eof   = true;
            m_error = true;
        }
        else
        {
            TRACE(logInfo() << "OK!" << std::endl);
        }
    }

    StreamInterface::offset_t InternalStream::tell()
    {
        return m_stream->tell();
    }

    StreamInterface::offset_t InternalStream::size()
    {
        return m_stream->size();
    }

    void InternalStream::clear()
    {
        m_eof   = false;
        m_error = false;
    }

    bool InternalStream::good() const
    {
        return !m_error;
    }

    bool InternalStream::eof() const
    {
        return m_eof;
    }
}  // namespace HEIF
