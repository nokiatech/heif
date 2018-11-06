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

#include "sampletochunkbox.hpp"
#include "log.hpp"

#include <limits>
#include <stdexcept>

using namespace std;

SampleToChunkBox::SampleToChunkBox()
    : FullBox("stsc", 0, 0)
    , mRunOfChunks()
    , mMaxSampleCount(-1)
{
}

bool SampleToChunkBox::getSampleDescriptionIndex(std::uint32_t sampleIndex, std::uint32_t& sampleDescriptionIdx) const
{
    if (sampleIndex >= mDecodedEntries.size())
    {
        return false;
    }

    sampleDescriptionIdx = mDecodedEntries.at(sampleIndex).sampleDescriptionIndex;
    return true;
}

bool SampleToChunkBox::getSampleChunkIndex(std::uint32_t sampleIndex, std::uint32_t& chunkIdx) const
{
    if (sampleIndex >= mDecodedEntries.size())
    {
        return false;
    }

    chunkIdx = mDecodedEntries.at(sampleIndex).chunkIndex;
    return true;
}

void SampleToChunkBox::setSampleCountMaxSafety(int64_t maxSampleCount)
{
    mMaxSampleCount = maxSampleCount;
}

void SampleToChunkBox::addChunkEntry(const ChunkEntry& chunkEntry)
{
    mRunOfChunks.push_back(chunkEntry);
}

void SampleToChunkBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(static_cast<std::uint32_t>(mRunOfChunks.size()));
    for (const auto& run : mRunOfChunks)
    {
        bitstr.write32Bits(run.firstChunk);
        bitstr.write32Bits(run.samplesPerChunk);
        bitstr.write32Bits(run.sampleDescriptionIndex);
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void SampleToChunkBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    const uint32_t entryCount = bitstr.read32Bits();
    for (uint32_t i = 0; i < entryCount; ++i)
    {
        ChunkEntry chunkEntry;
        chunkEntry.firstChunk      = bitstr.read32Bits();
        chunkEntry.samplesPerChunk = bitstr.read32Bits();

        if ((mMaxSampleCount != -1 && (chunkEntry.samplesPerChunk > mMaxSampleCount)) ||
            (chunkEntry.samplesPerChunk == 0))
        {
            throw RuntimeError("SampleToChunkBox::parseBox samplesPerChunk sanity check fails");
        }

        chunkEntry.sampleDescriptionIndex = bitstr.read32Bits();
        mRunOfChunks.push_back(chunkEntry);
    }
}

uint32_t SampleToChunkBox::getSampleCountLowerBound(uint32_t chunkEntryCount) const
{
    if (chunkEntryCount == 0)
    {
        // nothing to do.
        return 0;
    }

    uint64_t sampleCount = 0;

    if (mRunOfChunks.at(0).firstChunk != 1)
    {
        throw RuntimeError("SampleToChunkBox first entry first_chunk != 1");
    }

    for (unsigned int chunkEntryIndex = 0; chunkEntryIndex < mRunOfChunks.size(); ++chunkEntryIndex)
    {
        const uint32_t firstChunk      = mRunOfChunks.at(chunkEntryIndex).firstChunk;
        const uint32_t samplesPerChunk = mRunOfChunks.at(chunkEntryIndex).samplesPerChunk;

        uint32_t chunkRepetitions = 1;
        if ((chunkEntryIndex + 1) < mRunOfChunks.size())
        {
            if (mRunOfChunks.at(chunkEntryIndex + 1).firstChunk <= firstChunk)
            {
                throw RuntimeError(
                    "Invalid first_chunk value in SampleToChunkBox entry. Must be greater than previous");
            }

            chunkRepetitions = mRunOfChunks.at(chunkEntryIndex + 1).firstChunk - firstChunk;
        }
        else if (chunkEntryIndex == mRunOfChunks.size() - 1)
        {
            // handle last entry.
            chunkRepetitions = chunkEntryCount - mRunOfChunks.at(chunkEntryIndex).firstChunk + 1;
        }

        sampleCount += uint64_t(chunkRepetitions) * samplesPerChunk;
    }

    if (sampleCount <= std::numeric_limits<uint32_t>::max())
    {
        return static_cast<uint32_t>(sampleCount);
    }
    else
    {
        throw RuntimeError("SampleToChunkBox has >= 2^32 samples");
    }
}

void SampleToChunkBox::decodeEntries(std::uint32_t chunkEntryCount)
{
    mDecodedEntries.clear();

    if (mRunOfChunks.size() == 0 || chunkEntryCount == 0)
    {
        // nothing to do.
        return;
    }

    if (mRunOfChunks.at(0).firstChunk != 1)
    {
        throw RuntimeError("SampleToChunkBox first entry first_chunk != 1");
    }

    for (unsigned int chunkEntryIndex = 0; chunkEntryIndex < mRunOfChunks.size(); ++chunkEntryIndex)
    {
        const uint32_t firstChunk             = mRunOfChunks.at(chunkEntryIndex).firstChunk;
        const uint32_t samplesPerChunk        = mRunOfChunks.at(chunkEntryIndex).samplesPerChunk;
        const uint32_t sampleDescriptionIndex = mRunOfChunks.at(chunkEntryIndex).sampleDescriptionIndex;

        uint32_t chunkRepetitions = 1;
        if ((chunkEntryIndex + 1) < mRunOfChunks.size())
        {
            if (mRunOfChunks.at(chunkEntryIndex + 1).firstChunk <= firstChunk)
            {
                throw RuntimeError(
                    "Invalid first_chunk value in SampleToChunkBox entry. Must be greater than previous");
            }

            chunkRepetitions = mRunOfChunks.at(chunkEntryIndex + 1).firstChunk - firstChunk;
        }
        else if (chunkEntryIndex == mRunOfChunks.size() - 1)
        {
            // handle last entry.
            chunkRepetitions = chunkEntryCount - mRunOfChunks.at(chunkEntryIndex).firstChunk + 1;
        }

        if (mMaxSampleCount != -1 && std::uint64_t(samplesPerChunk) * chunkRepetitions > std::uint64_t(mMaxSampleCount))
        {
            throw RuntimeError("SampleToChunkBox::parseBox samplesPerChunk is larger than total number of samples");
        }

        DecodedEntry entry;
        entry.samplesPerChunk        = samplesPerChunk;
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
