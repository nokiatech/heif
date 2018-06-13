/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#pragma once

#include <cstdint>

namespace HEIFPP
{
    class NAL_State
    {
    public:
        NAL_State()  = default;
        ~NAL_State() = default;
        bool init_parse(const std::uint8_t* aData, std::uint64_t aLength);
        bool parse_byte_stream(const std::uint8_t*& aNal_unit, std::uint64_t& aNal_unit_length);
        bool end_of_stream();

        // in-place modification of nal_lengths to bytestream headers (size change not neede due to using 4 byte
        // headers.)
        static bool convertToByteStream(std::uint8_t* aData, std::uint64_t aLength);
        // generates new buffer with bytestream headers converted to nal_lengths. (no inplace conversion, since size
        // could change)
        static bool
        convertFromByteStream(uint8_t* aBuffer, std::uint64_t aBufferSize, uint8_t*& aData, std::uint64_t& aSize);

    protected:
        const std::uint8_t* mData;
        std::uint64_t mLength;

    private:
        NAL_State& operator=(const NAL_State&) = delete;
        NAL_State(const NAL_State&)            = delete;
        NAL_State(NAL_State&&)                 = delete;
    };
}  // namespace HEIFPP
