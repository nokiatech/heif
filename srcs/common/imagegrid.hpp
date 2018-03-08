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

#ifndef IMAGEGRID_HPP
#define IMAGEGRID_HPP

#include <cstdint>
#include "customallocator.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** Image Grid item struct for derived images of 'grid' type. */
struct ImageGrid
{
    std::uint8_t rowsMinusOne    = 0;  ///< Number of rows in grid minus one (0 = one row, 1 = two rows, etc.)
    std::uint8_t columnsMinusOne = 0;  ///< Number of columns in grid minus one (0 = one column, 1 = two columns, etc.)
    std::uint32_t outputWidth    = 0;  ///< Height of the reconstructed image canvas.
    std::uint32_t outputHeight   = 0;  ///< Width of the reconstructed image canvas.
};

/** Write ImageGrid to ISOBMFF::BitStream.
 *  @param [in]  grid ImageGrid to serialize.
 *  @param [out] output ISOBMFF::BitStream where to serialize ImageGrid data. */
void writeImageGrid(const ImageGrid& grid, ISOBMFF::BitStream& output);

/** Parse ImageGrid from ISOBMFF::BitStream.
 *  @param [in] input ISOBMFF::BitStream where to read ImageGrid data from.
 *  @return Read and filled ImageGrid. */
ImageGrid parseImageGrid(ISOBMFF::BitStream& input);

#endif /* end of include guard: IMAGEGRID_HPP */
