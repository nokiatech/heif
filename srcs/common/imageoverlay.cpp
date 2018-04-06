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

#include "imageoverlay.hpp"
#include "bitstream.hpp"

#include <limits>
#include <stdexcept>

void writeImageOverlay(const ImageOverlay& iovl, BitStream& output)
{
    bool write32BitFields = false;
    if ((iovl.outputWidth > std::numeric_limits<std::uint16_t>::max()) &&
        (iovl.outputHeight > std::numeric_limits<std::uint16_t>::max()))
    {
        write32BitFields = true;
    }
    for (const auto& entry : iovl.offsets)
    {
        if ((entry.horizontalOffset > std::numeric_limits<std::int16_t>::max()) ||
            (entry.verticalOffset > std::numeric_limits<std::int16_t>::max()) ||
            (entry.horizontalOffset < std::numeric_limits<std::int16_t>::min()) ||
            (entry.verticalOffset < std::numeric_limits<std::int16_t>::min()))
        {
            write32BitFields = true;
            break;
        }
    }

    output.write8Bits(0);                 // version
    output.write8Bits(write32BitFields);  // flags
    output.write16Bits(iovl.canvasFillValueR);
    output.write16Bits(iovl.canvasFillValueG);
    output.write16Bits(iovl.canvasFillValueB);
    output.write16Bits(iovl.canvasFillValueA);

    if (write32BitFields)
    {
        output.write32Bits(iovl.outputWidth);
        output.write32Bits(iovl.outputHeight);
    }
    else
    {
        output.write16Bits(static_cast<std::uint16_t>(iovl.outputWidth));
        output.write16Bits(static_cast<std::uint16_t>(iovl.outputHeight));
    }

    for (const auto& entry : iovl.offsets)
    {
        if (write32BitFields)
        {
            output.write32Bits(static_cast<std::uint32_t>(entry.horizontalOffset));
            output.write32Bits(static_cast<std::uint32_t>(entry.verticalOffset));
        }
        else
        {
            output.write16Bits(static_cast<std::uint16_t>(entry.horizontalOffset));
            output.write16Bits(static_cast<std::uint16_t>(entry.verticalOffset));
        }
    }
}

ImageOverlay parseImageOverlay(BitStream& input)
{
    ImageOverlay iovl;
    input.read8Bits();  // discard version
    bool read32BitFields = input.read8Bits() & 1;

    iovl.canvasFillValueR = input.read16Bits();
    iovl.canvasFillValueG = input.read16Bits();
    iovl.canvasFillValueB = input.read16Bits();
    iovl.canvasFillValueA = input.read16Bits();

    if (read32BitFields)
    {
        iovl.outputWidth  = input.read32Bits();
        iovl.outputHeight = input.read32Bits();
    }
    else
    {
        iovl.outputWidth  = input.read16Bits();
        iovl.outputHeight = input.read16Bits();
    }

    // Read as many offsets as there is. This should match to number of relevant 'dimg' references, but it is
    // not feasible to verify it during reading.
    while (input.getPos() < input.getSize())
    {
        ImageOverlay::Offset offsets;
        if (read32BitFields)
        {
            offsets.horizontalOffset = static_cast<std::int32_t>(input.read32Bits());
            offsets.verticalOffset   = static_cast<std::int32_t>(input.read32Bits());
        }
        else
        {
            offsets.horizontalOffset = static_cast<std::int16_t>(input.read16Bits());
            offsets.verticalOffset   = static_cast<std::int16_t>(input.read16Bits());
        }
        iovl.offsets.push_back(offsets);
    }

    return iovl;
}
