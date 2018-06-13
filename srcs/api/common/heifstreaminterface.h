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

#ifndef HEIFSTREAMINTERFACE_HPP_
#define HEIFSTREAMINTERFACE_HPP_

#include <cstdint>
#include "heifexport.h"

namespace HEIF
{
    class HEIF_DLL_PUBLIC StreamInterface
    {
    public:
        typedef int64_t offset_t;

        /** Size of an indeterminately sized source, ie. a network stream */
        static const offset_t IndeterminateSize = 0x7fffffffffffffffll;

        /** Construct a stream object. After this the data is accessible. */
        StreamInterface();

        /** Destruct a stream object. */
        virtual ~StreamInterface();

        /** Returns the number of bytes read. The value of 0 indicates end
            of file.
            @param [buffer] The buffer to write the data into
            @param [size]   The number of bytes to read from the stream
            @returns The number of bytes read, or 0 on EOF.
        */
        virtual offset_t read(char* buffer, offset_t size) = 0;

        /** Seeks to the given offset. Should the offset be erronous we'll
            find it out also by the next read that will signal EOF.

            Seeking to the point after the last input byte is permissable;
            so seeking to offset 0 of an empty file should be OK as well
            as seeking to offset 1 of a 1-byte file. The next read would
            indicate EOF, though.

            @param [offset] Offset to seek into
            @returns true if the seek was successful
         */
        virtual bool absoluteSeek(offset_t offset) = 0;

        /** Retrieve the current offset of the file.
            @returns The current offset of the file.
         */
        virtual offset_t tell() = 0;

        /** Retrieve the size of the current file.

            @returns The current size of the file. Return
            StreamInterface::IndeterminateSize if the file size cannot be determined.
         */
        virtual offset_t size() = 0;
    };
}  // namespace HEIF

#endif  // HEIFSTREAMINTERFACE_HPP_
