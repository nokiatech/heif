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

#include "heifstreaminternal.hpp"



namespace HEIF
{
#define TRACE(x) \
    do           \
    {            \
    } while (0)
    //#define TRACE(x) x

    InternalStream::InternalStream(StreamInterface* stream)
        : m_stream(stream)
         
    {
        m_error = !stream || !stream->absoluteSeek(0);
    }

    void InternalStream::read(char* buffer, StreamInterface::offset_t size_)
    {
        TRACE(logInfo() << "Reading " << size_ << " at " << m_stream->tell() << " ");
        if (size_ <= 0)
        {
            return;
        }

        // Loop on short reads. The StreamInterface contract permits a
        // partial read for any reason - non-EOF reasons include POSIX
        // signal interruption, network packet boundaries, Python
        // file.read1() returning what is currently buffered, JNI
        // InputStream.read() returning whatever a single OS read
        // delivered, etc. The reader's box parser hands us a fixed-size
        // request and expects the buffer to be filled or a hard error.
        // Translation: keep calling the underlying stream until either
        // (a) the buffer is full, (b) the stream returns 0 (true EOF)
        // or (c) the stream returns a negative value (I/O error).
        StreamInterface::offset_t total = 0;
        while (total < size_)
        {
            StreamInterface::offset_t got = m_stream->read(buffer + total, size_ - total);
            if (got <= 0)
            {
                // got == 0  -> end of stream before request satisfied (short -> EOF + error,
                //              because the caller asked for size_ bytes and got fewer).
                // got <  0  -> stream-reported I/O failure (e.g. POSIX read(2) returning -1
                //              surfaced through a binding). Treat as hard error and DO NOT
                //              advance into the buffer; the bytes from total..size_-1 are
                //              left untouched (the caller will see m_error=true via good()).
                TRACE(logInfo() << "FAIL got=" << got << "!" << std::endl);
                m_eof   = true;
                m_error = true;
                return;
            }
            total += got;
        }
        TRACE(logInfo() << "OK!" << std::endl);
    }

    int InternalStream::get()
    {
        char ch;
        TRACE(logInfo() << "Getting at " << m_stream->tell() << " ");
        StreamInterface::offset_t got = m_stream->read(&ch, sizeof(ch));
        if (got > 0)
        {
            TRACE(logInfo() << "OK!" << std::endl);
            return static_cast<unsigned char>(ch);
        }
        // got == 0 -> EOF, got < 0 -> hard I/O error. Treat both as
        // "no byte produced" for the caller; mark error on negative.
        TRACE(logInfo() << "FAIL got=" << got << "!" << std::endl);
        m_eof = true;
        if (got < 0)
        {
            m_error = true;
        }
        return 0;
    }

    bool InternalStream::peekEof()
    {
        char buffer;
        TRACE(logInfo() << "Peek EOF at " << m_stream->tell() << " ");
        auto was                      = m_stream->tell();
        StreamInterface::offset_t got = m_stream->read(&buffer, sizeof(buffer));
        if (got <= 0)
        {
            // Either true EOF (0) or an I/O error (<0). Both look like
            // "no byte available right now" to peekEof's contract; we
            // additionally raise m_error on negative returns so good()
            // becomes false and the caller can stop the parse.
            TRACE(logInfo() << "EOF (got=" << got << ")!" << std::endl);
            if (got < 0)
            {
                m_error = true;
            }
            return true;
        }
        TRACE(logInfo() << "No EOF!" << std::endl);
        m_stream->absoluteSeek(was);
        return false;
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
            // Successful seek to a known-good offset: previous transient
            // EOF/error is irrelevant for whatever read sequence the
            // caller is about to start. Without this, a momentary stream
            // hiccup (a single short/zero/negative read or a Python
            // EAGAIN) would leave the reader permanently broken even
            // after the stream recovers -- every getItemData / getItem /
            // getProperty would still see good() == false. Real bindings
            // retry on transient I/O failures; the retry must be allowed
            // to succeed.
            m_eof   = false;
            m_error = false;
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
