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

#ifndef HEIFSTREAMFILE_HPP_
#define HEIFSTREAMFILE_HPP_

#include <cstdio>
#include "customallocator.hpp"
#include "heifstreaminterface.h"

namespace HEIF
{
    class FileStream : public StreamInterface
    {
    public:
        FileStream();
        FileStream(const char* filename);

        FileStream(const FileStream& other) = delete;
        FileStream& operator=(const FileStream& other) = delete;
        FileStream& operator                           =(FileStream&& other);

        ~FileStream() override;

        /** Returns the number of bytes read. The value of 0 indicates end
        of file.
        @param [buffer] The buffer to write the data into
        @param [size]   The number of bytes to read from the stream
        @returns The number of bytes read, or 0 on EOF. */
        offset_t read(char* buffer, offset_t size) override;

        /** Seeks to the given offset. Should the offset be erronous we'll
        find it out by the next read that will signal EOF.
        @param [offset] Offset to seek iEOF*/
        bool absoluteSeek(offset_t offset) override;

        /** Retrieve the current offset of the file.
        @returns The current offset of the fiEOF*/
        offset_t tell() override;

        /** Retrieve the size of the current file.

        @returns The current size of the file. Return
        FileStream::StreamSize if the file size cannot be determinEOF*/
        offset_t size() override;

        /** Was the file successfully opened? */
        bool isOpen() const;

    private:
        FILE* m_file;
        offset_t m_curOffset;
        offset_t m_size;
        char m_buffer[8192];
    };
}  // namespace HEIF

#endif  // HEIFSTREAMFILE_HPP_
