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
#include <string>

class BitStream;

/** @brief Chunk Offset Box class
 *  @details 'stco' box implementation. Extends from FullBox.
 */

class ChunkOffsetBox : public FullBox
{
public:
    ChunkOffsetBox();
    virtual ~ChunkOffsetBox() = default;

    /** @brief Sets the number of entries in the chunk offset box
     *  @param [in] entry_count number of entries */
    void setEntryCount(std::uint32_t entry_count);

    ///@return Number of entries in the Chunk Offset Box
    uint32_t getEntryCount();

    /** @brief Sets the chunk offset values
     *  @param [in] chunk_offsets Chunk offset values */
    void setChunkOffsets(const std::vector<std::uint32_t>& chunk_offsets);

    ///@return Chunk offset values as a vector of unsigned 32 bit integers
    std::vector<std::uint32_t> getChunkOffsets();

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a Chunk Offset Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    ///@brief number of entries in the chunk offset box
    std::uint32_t mEntryCount;

    ///@brief vector of chunk offset values as unsigned 32 bit integers
    std::vector<std::uint32_t> mChunkOffsets;
};

#endif /* end of include guard: CHUNKOFFSETBOX_HPP */

