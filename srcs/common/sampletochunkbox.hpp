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

#ifndef SAMPLETOCHUNKBOX_HPP
#define SAMPLETOCHUNKBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"


/** @brief SampleToChunkBox class. Extends from FullBox.
 *  @details 'stsc' box contains chunk offsets and sample mapping information as defined in the ISOBMFF standard. */
class SampleToChunkBox : public FullBox
{
public:
    SampleToChunkBox();
    ~SampleToChunkBox() = default;

    /** @brief Gets the sample description index
     *  @param [in] sampleIndex sample index value
     *  @param [out] sampleDescriptionIdx sample description index
     *  @returns true if succcess */
    bool getSampleDescriptionIndex(std::uint32_t sampleIndex, std::uint32_t& sampleDescriptionIdx) const;

    /** @brief Get the sample chunk index.
     *  @param [in] sampleIndex Sample index value.
     *  @param [out] chunkIdx Chunk index of the sample. The index is 1-based.
     *  @returns true if success */
    bool getSampleChunkIndex(std::uint32_t sampleIndex, std::uint32_t& chunkIdx) const;

    void setSampleCountMaxSafety(int64_t maxSampleCount);

    /// Chunk entry data structure
    struct ChunkEntry
    {
        std::uint32_t firstChunk;
        std::uint32_t samplesPerChunk;
        std::uint32_t sampleDescriptionIndex;
    };

    /** @brief Adds a chunk entry to the box.
     *  @param [in] chunkEntry a Chunk Entry data structure */
    void addChunkEntry(const ChunkEntry& chunkEntry);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a SampleToChunkBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data. */
    void parseBox(ISOBMFF::BitStream& bitstr);

    /* @brief Calculate a lower bound for the number of samples
       @param [in] chunkEntryCount number of total chunk entries from 'stco'
    */
    uint32_t getSampleCountLowerBound(uint32_t chunkEntryCount) const;

    /** @brief Decodes the representation of ChunkEntries. Each sample wil have an own entry after this.
     *  @param [in] chunkEntryCount number of total chunk entries from 'stco' */
    void decodeEntries(std::uint32_t chunkEntryCount);

private:
    Vector<ChunkEntry> mRunOfChunks;  ///< Vector that contains the chunk entries

    /// Run-length decoded chunk entries
    struct DecodedEntry
    {
        std::uint32_t chunkIndex;
        std::uint32_t samplesPerChunk;
        std::uint32_t sampleDescriptionIndex;
    };

    /// A decoded representation of ChunkEntries. Each sample has an own entry here.
    Vector<DecodedEntry> mDecodedEntries;

    int64_t mMaxSampleCount;
};

#endif /* end of include guard: SAMPLETOCHUNKBOX_HPP */
