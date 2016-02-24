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

#include "chunkoffsetbox.hpp"
#include "bitstream.hpp"
#include <algorithm>
#include <limits>

ChunkOffsetBox::ChunkOffsetBox() :
    FullBox("stco", 0, 0),
    mChunkOffsets()
{
}

void ChunkOffsetBox::setChunkOffsets(const std::vector<uint64_t>& chunkOffsets)
{
    mChunkOffsets = chunkOffsets;
    if (*std::max_element(mChunkOffsets.cbegin(), mChunkOffsets.cend()) > std::numeric_limits<std::uint32_t>::max())
    {
        setType("co64");
    }
    else
    {
        setType("stco");
    }
}

std::vector<uint64_t> ChunkOffsetBox::getChunkOffsets()
{
    return mChunkOffsets;
}

void ChunkOffsetBox::writeBox(BitStream& bitstr)
{
    // Write box headers
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mChunkOffsets.size());
    if (getType() == "stco")
    {
        for (uint32_t i = 0; i < mChunkOffsets.size(); ++i)
        {
            bitstr.write32Bits(mChunkOffsets.at(i));
        }
    }
    else
    {
        // This is a ChunkLargeOffsetBox 'co64' with unsigned int (64) chunk_offsets.
        for (uint32_t i = 0; i < mChunkOffsets.size(); ++i)
        {
            bitstr.write64Bits(mChunkOffsets.at(i));
        }
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void ChunkOffsetBox::parseBox(BitStream& bitstr)
{
    BitStream subBitstr;

    //  First parse the box header
    parseFullBoxHeader(bitstr);

    const std::uint32_t entryCount = bitstr.read32Bits();
    if (getType() == "stco")
    {
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            mChunkOffsets.push_back(bitstr.read32Bits());
        }
    }
    else // This is a ChunkLargeOffsetBox 'co64' with unsigned int (64) chunk_offsets.
    {
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            mChunkOffsets.push_back(bitstr.read64Bits());
        }
    }
}
