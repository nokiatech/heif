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

#include "chunkoffsetbox.hpp"
#include <algorithm>
#include <limits>
#include "bitstream.hpp"

ChunkOffsetBox::ChunkOffsetBox()
    : FullBox("stco", 0, 0)
    , mChunkOffsets()
{
}

void ChunkOffsetBox::setChunkOffsets(const Vector<uint64_t>& chunkOffsets)
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

Vector<uint64_t>& ChunkOffsetBox::getChunkOffsets()
{
    return mChunkOffsets;
}

const Vector<uint64_t> ChunkOffsetBox::getChunkOffsets() const
{
    return mChunkOffsets;
}

void ChunkOffsetBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Write box headers
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(static_cast<uint32_t>(mChunkOffsets.size()));
    if (getType() == "stco")
    {
        for (uint32_t i = 0; i < mChunkOffsets.size(); ++i)
        {
            bitstr.write32Bits(static_cast<uint32_t>(mChunkOffsets.at(i)));
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

void ChunkOffsetBox::parseBox(ISOBMFF::BitStream& bitstr)
{
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
    else  // This is a ChunkLargeOffsetBox 'co64' with unsigned int (64) chunk_offsets.
    {
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            mChunkOffsets.push_back(bitstr.read64Bits());
        }
    }
}
