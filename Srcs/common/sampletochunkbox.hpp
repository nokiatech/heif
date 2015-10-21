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

#ifndef SAMPLETOCHUNKBOX_HPP
#define SAMPLETOCHUNKBOX_HPP

#include "bitstream.hpp"
#include "fullbox.hpp"

#include <vector>

/** @brief SampleToChunkBox class. Extends from FullBox.
 *  @details 'stsc' box contains chunk offsets and sample mapping information as defined in the ISOBMFF standard. **/
class SampleToChunkBox : public FullBox
{
public:
    SampleToChunkBox();
    ~SampleToChunkBox() = default;

    /** @brief Gets the sample description index
     *  @param [in] sampleIndex sample index value
     *  @returns sample description index */
    std::uint32_t getSampleDescriptionIndex(std::uint32_t sampleIndex) const;

    /// chunk entry data structure
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
    void writeBox(BitStream& bitstr);

    /** @brief Parses a SampleToChunkBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data. */
    void parseBox(BitStream& bitstr);

private:
    std::vector<ChunkEntry> mRunOfChunks; /// vector that contains the chunk entries

    /// run-length decoded chunk entries
    struct DecodedEntry
    {
        std::uint32_t chunkIndex;
        std::uint32_t samplesPerChunk;
        std::uint32_t sampleDescriptionIndex;
    };

    /// A decoded representation of ChunkEntries. Each sample has an own entry here.
    std::vector<DecodedEntry> mDecodedEntries;

    /** @brief Decodes the Chunk entries
     *  @throws Run-time error if there are no chunk entries in the box.
     *  @throws Run-time error if the first chunk entry run does not have a length of 1.
     *  @throws Run-time error if the first chunk value of the next chunk entry is not bigger than the previous one.*/
    void decodeEntries();
};

#endif /* end of include guard: SAMPLETOCHUNKBOX_HPP */

