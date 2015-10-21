/* Copyright (c) 2015, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#include "imagegrid.hpp"
#include "bitstream.hpp"

#include <limits>

void writeImageGrid(const ImageGrid& grid, BitStream& output)
{
    bool write32BitFields = false;
    output.write8Bits(0); // version = 0
    if ((grid.outputWidth > std::numeric_limits<std::uint32_t>::max()) &&
        (grid.outputHeight > std::numeric_limits<std::uint32_t>::max()))
    {
        write32BitFields = true;
    }

    output.write8Bits(write32BitFields); // flags

    output.write8Bits(grid.rowsMinusOne);
    output.write8Bits(grid.columnsMinusOne);

    if (write32BitFields)
    {
        output.write32Bits(grid.outputWidth);
        output.write32Bits(grid.outputHeight);
    }
    else
    {
        output.write16Bits(grid.outputWidth);
        output.write16Bits(grid.outputHeight);
    }
}


ImageGrid parseImageGrid(BitStream& input)
{
    ImageGrid grid;

    input.read8Bits(); // discard version
    bool read32BitFields = input.read8Bits() & 1; // flags

    grid.rowsMinusOne = input.read8Bits();
    grid.columnsMinusOne = input.read8Bits();

    if (read32BitFields)
    {
        grid.outputWidth = input.read32Bits();
        grid.outputHeight = input.read32Bits();
    }
    else
    {
        grid.outputWidth = input.read16Bits();
        grid.outputHeight = input.read16Bits();
    }

    return grid;
}
