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

#include "nalutil.hpp"

unsigned int findStartCodeLen(const Vector<uint8_t>& data)
{
    unsigned int i      = 0;
    const auto dataSize = data.size();

    while ((i + 1) < dataSize && data[i] == 0)
    {
        ++i;
    }

    if (i > 1 && data[i] == 1)
    {
        return i + 1;
    }
    else
    {
        return 0;
    }
}

Vector<uint8_t> convertByteStreamToRBSP(const Vector<uint8_t>& byteStr)
{
    Vector<uint8_t> dest;
    const size_t numBytesInNalUnit = byteStr.size();

    // this is a reasonable guess, as the result Vector can not be larger than the original
    dest.reserve(numBytesInNalUnit);

    // find start code end
    uint32_t i = findStartCodeLen(byteStr);

    // copy NALU header
    static const size_t NALU_HEADER_LENGTH = 2;
    dest.insert(dest.end(), byteStr.cbegin() + static_cast<int32_t>(i),
                byteStr.cbegin() + static_cast<int32_t>(i) + NALU_HEADER_LENGTH);
    i += NALU_HEADER_LENGTH;

    // copy rest of the data while removing start code emulation prevention bytes
    enum class State
    {
        COPY_DATA,
        ONE_ZERO_FOUND,
        TWO_ZEROS_FOUND
    };
    State state         = State::COPY_DATA;
    int copyStartOffset = static_cast<int>(i);
    for (; i < numBytesInNalUnit; ++i)
    {
        const unsigned int byte = byteStr[i];
        switch (state)
        {
        case State::COPY_DATA:
            if (byte != 0)
                state = State::COPY_DATA;
            else
                state = State::ONE_ZERO_FOUND;
            break;

        case State::ONE_ZERO_FOUND:
            if (byte != 0)
                state = State::COPY_DATA;
            else
                state = State::TWO_ZEROS_FOUND;
            break;

        case State::TWO_ZEROS_FOUND:
            // sequence of 0x000003 means that 0x03 is the emulation prevention byte
            if (byte == 0x03)
            {
                // skip copying 0x03
                dest.insert(dest.end(), byteStr.cbegin() + copyStartOffset, byteStr.cbegin() + static_cast<int32_t>(i));
                copyStartOffset = static_cast<int32_t>(i) + 1;
                // continue byte stream copying
                state = State::COPY_DATA;
            }
            else if (byte == 0)
                state = State::TWO_ZEROS_FOUND;
            else
                state = State::COPY_DATA;
            break;
        }
    }
    dest.insert(dest.end(), byteStr.cbegin() + copyStartOffset, byteStr.cend());
    return dest;
}
