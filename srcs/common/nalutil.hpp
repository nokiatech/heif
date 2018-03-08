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

#ifndef NALUTIL_HPP
#define NALUTIL_HPP

#include "customallocator.hpp"

/**
 * @brief Returns the number of bytes in start code
 * @details Start code consists of any number of zero bytes (0x00) followed by a
 * one (0x01) byte.
 * @param srcData NAL data to search from
 * @return Number of bytes in start code
 */
unsigned int findStartCodeLen(const Vector<uint8_t> &data);

/**
 * Convert byte stream to Raw Byte Sequence Payload (RBSP) by removing emulation
 * prevention bytes (0x03).
 * Possible start code will be stripped from beginning.
 * @param byteStr ByteStream to convert.
 * @return Data as RBSP without emulation prevention bytes and start code.
 */
Vector<uint8_t> convertByteStreamToRBSP(const Vector<uint8_t> &byteStr);

#endif  // NALUTIL_HPP
