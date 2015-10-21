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

#ifndef IMAGEGRID_HPP
#define IMAGEGRID_HPP

#include <cstdint>

class BitStream;

/** Image Grid item struct for derived images of 'grid' type. */
struct ImageGrid
{
    std::uint8_t rowsMinusOne = 0;    ///< Number of rows in grid minus one (0 = one row, 1 = two rows, etc.)
    std::uint8_t columnsMinusOne = 0; ///< Number of columns in grid minus one (0 = one column, 1 = two columns, etc.)
    std::uint32_t outputWidth = 0;    ///< Height of the reconstructed image canvas.
    std::uint32_t outputHeight = 0;   ///< Width of the reconstructed image canvas.
};

/** Write ImageGrid to BitStream.
 *  @param [in]  grid ImageGrid to serialize.
 *  @param [out] output BitStream where to serialize ImageGrid data. */
void writeImageGrid(const ImageGrid& grid, BitStream& output);

/** Parse ImageGrid from BitStream.
 *  @param [in] input BitStream where to read ImageGrid data from.
 *  @return Read and filled ImageGrid. */
ImageGrid parseImageGrid(BitStream& input);

#endif /* end of include guard: IMAGEGRID_HPP */
