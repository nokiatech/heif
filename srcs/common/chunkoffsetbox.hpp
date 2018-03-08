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

#ifndef CHUNKOFFSETBOX_HPP
#define CHUNKOFFSETBOX_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

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
    void setChunkOffsets(const Vector<std::uint64_t>& chunkOffsets);

    /// @return Chunk offset values as a vector.
    Vector<std::uint64_t>& getChunkOffsets();

    /// @return Chunk offset values as a vector.
    const Vector<std::uint64_t> getChunkOffsets() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a Chunk Offset Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    /// @brief Chunk offset values. 'stco' uses just first 32 bits, 'co64' all 64 bits.
    Vector<std::uint64_t> mChunkOffsets;
};

#endif /* end of include guard: CHUNKOFFSETBOX_HPP */
