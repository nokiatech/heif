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

#include <stdint.h>
namespace HEIFPP
{
    class NAL_State
    {
    public:
        NAL_State()  = default;
        ~NAL_State() = default;
        bool init_parse(const uint8_t* data, uint64_t length);
        bool parse_byte_stream(const uint8_t*& nal_unit, uint64_t& nal_unit_length);
        bool end_of_stream();

    protected:
        const uint8_t* mData;
        uint64_t mLength;

    private:
        NAL_State& operator=(const NAL_State&) = delete;
        NAL_State(const NAL_State&)            = delete;
        NAL_State(NAL_State&&)                 = delete;
    };
}  // namespace HEIFPP
