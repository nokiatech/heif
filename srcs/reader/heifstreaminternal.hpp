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

#ifndef HEIFSTREAMINTERNAL_HPP_
#define HEIFSTREAMINTERNAL_HPP_

#include "customallocator.hpp"
#include "heifstreaminterface.h"

namespace HEIF
{
    class InternalStream
    {
    public:
        InternalStream(StreamInterface* stream = nullptr);
        ~InternalStream();

        /** Returns the number of bytes read. A short read sets the EOF flag.
        @param [buffer] The buffer to write the data into
        @param [size]   The number of bytes to read from the stream */
        void read(char* buffer, StreamInterface::offset_t size);

        /** Reads one character from the stream.
        @return Returns the character read or undefined on error. */
        int get();

        /** Seeks to the given offset. Should the offset be erronous we'll
        find it out by the next read that will signal EOF.
        @param [offset] Offset to seek into */

        /// @see StreamInterface::seek
        void seek(StreamInterface::offset_t offset);

        /// @see StreamInterface::tell
        StreamInterface::offset_t tell();

        /// @see StreamInterface::size
        StreamInterface::offset_t size();

        /** Returns false if we can read at least one byte from the
        current position of the file.  In other words, returns true if
        we have reached the end of the file (but before have read
        it). */
        bool peekEof();

        /** Returns true if there is no error in the file. End-of-file is
        not an error. */
        bool good() const;

        /** Returns true if we have reached the end of the file. */
        bool eof() const;

        /// Clears error and eof status
        void clear();

    private:
        StreamInterface* m_stream;
        bool m_error;
        bool m_eof;
    };
}  // namespace HEIF

#endif  // HEIFSTREAMINTERNAL_HPP_
