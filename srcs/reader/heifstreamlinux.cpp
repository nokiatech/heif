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

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include "customallocator.hpp"

#include "heifstreamlinux.hpp"


namespace HEIF
{
    LinuxStream::LinuxStream()
        : m_handle(0)
        , m_size(0)
        , m_bufFileOffset(0)
        , m_bufRead(0)
        , m_bufEnd(0)
        , m_buffer()
    {
        // nothing
    }

    LinuxStream::LinuxStream(const char* filename)
        : m_size(0)
        , m_bufFileOffset(0)
        , m_bufRead(0)
        , m_bufEnd(0)
        , m_buffer()
    {
        m_handle = open(filename, O_RDONLY);
        if (m_handle >= 0)
        {
            auto offset = lseek64(m_handle, 0, SEEK_END);
            if (offset >= 0)
            {
                m_buffer.resize(8192);
                m_size = offset;
                lseek64(m_handle, 0, SEEK_SET);
            }
            else
            {
                close(m_handle);
                m_handle = -1;
            }
        }
    }

    LinuxStream& LinuxStream::operator=(LinuxStream&& other)
    {
        if (m_handle >= 0)
        {
            close(m_handle);
        }

        m_handle       = other.m_handle;
        other.m_handle = -1;

        m_size       = other.m_size;
        other.m_size = 0;

        m_bufFileOffset       = other.m_bufFileOffset;
        other.m_bufFileOffset = 0;

        m_bufRead       = other.m_bufRead;
        other.m_bufRead = 0;

        m_bufEnd       = other.m_bufEnd;
        other.m_bufEnd = 0;

        m_buffer = std::move(other.m_buffer);

        return *this;
    }

    LinuxStream::~LinuxStream()
    {
        if (m_handle >= 0)
        {
            close(m_handle);
        }
    }

    void LinuxStream::fillBuffer()
    {
        m_bufRead       = 0;
        m_bufEnd        = 0;
        bool keepTrying = true;
        while (keepTrying)
        {
            keepTrying      = false;
            m_bufFileOffset = lseek64(m_handle, 0, SEEK_CUR);
            int n           = ::read(m_handle, &m_buffer[0], m_buffer.size());
            if (n > 0)
            {
                m_bufEnd = n;
            }
            else if (n < 0 && errno == EAGAIN)
            {
                keepTrying = true;
            }
            else
            {
                // Error or end of file
            }
        }
    }

    LinuxStream::offset_t LinuxStream::read(char* buffer, offset_t size_)
    {
        if (m_handle >= 0)
        {
            offset_t bytesRead = 0;
            bool eof           = false;
            while (!eof && bytesRead < size_)
            {
                if (m_bufRead == m_bufEnd)
                {
                    fillBuffer();
                }
                if (m_bufRead == m_bufEnd)
                {
                    eof = true;
                }
                else
                {
                    offset_t bytesLeft  = size_ - bytesRead;
                    offset_t copyNBytes = std::min(bytesLeft, offset_t(m_bufEnd - m_bufRead));
                    std::copy(m_buffer.begin() + m_bufRead, m_buffer.begin() + m_bufRead + copyNBytes,
                              buffer + bytesRead);
                    bytesRead += copyNBytes;
                    m_bufRead += copyNBytes;
                }
            }
            return bytesRead;
        }
        else
        {
            return 0;
        }
    }

    bool LinuxStream::absoluteSeek(offset_t offset)
    {
        if (m_handle >= 0)
        {
            bool withinBuffer = offset >= m_bufFileOffset && offset < m_bufFileOffset + m_bufEnd;
            if (withinBuffer)
            {
                m_bufRead = offset - m_bufFileOffset;
                return true;
            }
            else
            {
                m_bufRead       = 0;
                m_bufEnd        = 0;
                m_bufFileOffset = offset;
                return lseek64(m_handle, offset, SEEK_SET) >= 0;
            }
        }
        else
        {
            return false;
        }
    }

    LinuxStream::offset_t LinuxStream::tell()
    {
        return m_bufFileOffset + m_bufRead;
    }

    LinuxStream::offset_t LinuxStream::size()
    {
        return m_size;
    }
}  // namespace HEIF
