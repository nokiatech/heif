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

#include "sampletochunkbox.hpp"
#include "log.hpp"

#include <stdexcept>

using namespace std;

SampleToChunkBox::SampleToChunkBox() :
    FullBox("stsc", 0, 0),
    mRunOfChunks()
{
}

std::uint32_t SampleToChunkBox::getSampleDescriptionIndex(std::uint32_t sampleIndex) const
{
    return mDecodedEntries.at(sampleIndex).sampleDescriptionIndex;
}

void SampleToChunkBox::addChunkEntry(const ChunkEntry& chunkEntry)
{
    mRunOfChunks.push_back(chunkEntry);
    decodeEntries();
}

void SampleToChunkBox::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mRunOfChunks.size());
    for (const auto& run : mRunOfChunks)
    {
        bitstr.write32Bits(run.firstChunk);
        bitstr.write32Bits(run.samplesPerChunk);
        bitstr.write32Bits(run.sampleDescriptionIndex);
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void SampleToChunkBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    const uint32_t entryCount = bitstr.read32Bits();
    for (uint32_t i = 0; i < entryCount; ++i)
    {
        ChunkEntry chunkEntry;
        chunkEntry.firstChunk = bitstr.read32Bits();
        chunkEntry.samplesPerChunk = bitstr.read32Bits();
        chunkEntry.sampleDescriptionIndex = bitstr.read32Bits();
        mRunOfChunks.push_back(chunkEntry);
    }

    decodeEntries();
}

void SampleToChunkBox::decodeEntries()
{
    mDecodedEntries.clear();

    if (mRunOfChunks.size() == 0)
    {
        throw runtime_error("A SampleToChunkBox without entries");
    }

    if (mRunOfChunks.at(0).firstChunk != 1)
    {
        throw runtime_error("SampleToChunkBox first entry first_chunk != 1");
    }

    for (unsigned int chunkEntryIndex = 0; chunkEntryIndex < mRunOfChunks.size(); ++chunkEntryIndex)
    {
        const uint32_t firstChunk = mRunOfChunks.at(chunkEntryIndex).firstChunk;
        const uint32_t samplesPerChunk = mRunOfChunks.at(chunkEntryIndex).samplesPerChunk;
        const uint32_t sampleDescriptionIndex = mRunOfChunks.at(chunkEntryIndex).sampleDescriptionIndex;

        uint32_t chunkRepetitions = 1;
        if ((chunkEntryIndex + 1) < mRunOfChunks.size())
        {
            if (mRunOfChunks.at(chunkEntryIndex + 1).firstChunk <= firstChunk)
            {
                throw std::runtime_error("Invalid first_chunk value in SampleToChunkBox entry. Must be greater than previous");
            }

            chunkRepetitions = mRunOfChunks.at(chunkEntryIndex + 1).firstChunk - firstChunk;
        }

        DecodedEntry entry;
        entry.samplesPerChunk = samplesPerChunk;
        entry.sampleDescriptionIndex = sampleDescriptionIndex;
        for (unsigned int i = 0; i < chunkRepetitions; ++i)
        {
            entry.chunkIndex = firstChunk + i;
            for (unsigned int k = 0; k < samplesPerChunk; ++k)
            {
                mDecodedEntries.push_back(entry);
            }
        }
    }
}
