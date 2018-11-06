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

#ifndef OUTPUTSTREAMINTERFACE_HPP
#define OUTPUTSTREAMINTERFACE_HPP

#include <cstdint>
#include "heifexport.h"

namespace HEIF
{
    class HEIF_DLL_PUBLIC OutputStreamInterface
    {
    public:
        virtual ~OutputStreamInterface() = default;

        /** Sets current write position
        @param [aPosition] Position in stream where next write will occur. */
        virtual void seekp(std::uint64_t aPosition) = 0;

        /** Returns current position in stream.
        @return Current position in stream. */
        virtual std::uint64_t tellp() = 0;

        /** Writes data to stream
        @param [buffer] The buffer of date to write to the stream
        @param [size]   The number of bytes to write to the stream
        */
        virtual void write(const void* aBuffer, std::uint64_t aSize) = 0;

        /** Request to remove the file.
            Called on error cases to cleanup partial files.
        */
        virtual void remove() = 0;
    protected:
        OutputStreamInterface() = default;  //ctor    
        OutputStreamInterface & operator=(const OutputStreamInterface&) = delete;   //copy assignment
        OutputStreamInterface & operator=(OutputStreamInterface&&) = delete;        //move assignment
        OutputStreamInterface(OutputStreamInterface&) = delete;                     //copy ctor
        OutputStreamInterface(OutputStreamInterface&&) = delete;                    //move ctor

    };
}
#endif