/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef COMMONTYPES_HPP
#define COMMONTYPES_HPP

#include <cstdint>
#include "bitstream.hpp"

enum class ViewIdcType : std::uint8_t
{
    MONOSCOPIC     = 0,
    LEFT           = 1,
    RIGHT          = 2,
    LEFT_AND_RIGHT = 3,
    INVALID        = 0xff
};


struct SphereRegion
{
    std::int32_t centreAzimuth;
    std::int32_t centreElevation;
    std::int32_t centreTilt;
    std::uint32_t azimuthRange   = 0;  // not always used
    std::uint32_t elevationRange = 0;  // not always used
    bool interpolate;

    void write(BitStream& bitstr, bool rangeIncluded = true) const
    {
        bitstr.write32Bits(centreAzimuth);
        bitstr.write32Bits(centreElevation);
        bitstr.write32Bits(centreTilt);
        if (rangeIncluded)
        {
            bitstr.write32Bits(azimuthRange);
            bitstr.write32Bits(elevationRange);
        }
        bitstr.write8Bits(interpolate ? 0b10000000 : 0x0);
    }

    void read(BitStream& bitstr, bool rangeIncluded = true)
    {
        centreAzimuth   = bitstr.read32Bits();
        centreElevation = bitstr.read32Bits();
        centreTilt      = bitstr.read32Bits();
        if (rangeIncluded)
        {
            azimuthRange   = bitstr.read32Bits();
            elevationRange = bitstr.read32Bits();
        }
        interpolate = (bitstr.read8Bits() & 0b10000000) != 0;
    }
};

#endif
