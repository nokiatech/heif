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

#ifndef CHUNKOFFSETBOX_HPP
#define CHUNKOFFSETBOX_HPP

#include "fullbox.hpp"

class BitStream;

/** @brief Chunk Offset Box and Large Chunk Offset Box class
 *  @details 'stco' and 'co64' box implementation. Extends from FullBox.
 */

class ChunkOffsetBox : public FullBox
{
public:
    ChunkOffsetBox();
    virtual ~ChunkOffsetBox() = default;

    /** @brief Sets the chunk offset values. If some of values does not fit 32-bit field, the box type is set to 'co64'.
     *  @param [in] chunk_offsets Chunk offset values */
    void setChunkOffsets(const std::vector<std::uint64_t>& chunkOffsets);

    /// @return Chunk offset values as a vector.
    std::vector<std::uint64_t> getChunkOffsets();

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a Chunk Offset Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    /// @brief Chunk offset values. 'stco' uses just first 32 bits, 'co64' all 64 bits.
    std::vector<std::uint64_t> mChunkOffsets;
};

#endif /* end of include guard: CHUNKOFFSETBOX_HPP */

