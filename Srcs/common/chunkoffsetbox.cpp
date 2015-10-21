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

ChunkOffsetBox::ChunkOffsetBox() :
    FullBox("stco", 0, 0),
    mEntryCount(0),
    mChunkOffsets()
{
}

void ChunkOffsetBox::setEntryCount(uint32_t entry_count)
{
    mEntryCount = entry_count;
}

uint32_t ChunkOffsetBox::getEntryCount()
{
    return mEntryCount;
}

void ChunkOffsetBox::setChunkOffsets(const std::vector<uint32_t>& chunk_offsets)
{
    mChunkOffsets = chunk_offsets;
}

std::vector<uint32_t> ChunkOffsetBox::getChunkOffsets()
{
    return mChunkOffsets;
}

void ChunkOffsetBox::writeBox(BitStream& bitstr)
{
    // Write box headers
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mEntryCount);
    for (uint32_t i = 0; i < mEntryCount; i++)
    {
        bitstr.write32Bits(mChunkOffsets.at(i));
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void ChunkOffsetBox::parseBox(BitStream& bitstr)
{
    BitStream subBitstr;

    //  First parse the box header
    parseFullBoxHeader(bitstr);

    mEntryCount = bitstr.read32Bits();
    for (uint32_t i = 0; i < mEntryCount; i++)
    {
        mChunkOffsets.push_back(bitstr.read32Bits());
    }
}
