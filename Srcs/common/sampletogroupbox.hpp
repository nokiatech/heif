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

#ifndef SAMPLETOGROUPBOX_HPP
#define SAMPLETOGROUPBOX_HPP

#include "bitstream.hpp"
#include "fullbox.hpp"

#include <string>
#include <vector>

/** @brief SampleToGroupBox class. Extends from FullBox.
 *  @details 'sbgp' box contains sample to group mappings as defined in the ISOBMFF and HEIF standards. **/
class SampleToGroupBox : public FullBox
{
public:
    SampleToGroupBox();
    virtual ~SampleToGroupBox() = default;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data.
     *  @throws Run-time Error of the box has no entries. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a SampleToGroupBox bitstream and fills in the necessary member variables
     *  @param [in] bitstr Bitstream that contains the box data
     *  @throws Run-time Error of the box has no entries. */
    virtual void parseBox(BitStream& bitstr);

    /** @brief Sets the grouping type
     *  @param [in] groupingType Grouping Type as a string
     *  @throws Run-time Error if the grouping type is less than 4 characters. */
    void setGroupingType(const std::string& groupingType);

    /** @brief Gets the grouping type
     *  @returns Grouping Type as a string */
    const std::string& getGroupingType() const;

    /** @brief Sets the number of entries in the box
     *  @param [in]  entryCount number of entries to be listed in the box. */
    void setEntryCount(std::uint32_t entryCount);

    /** @brief Gets the number of entries in the box
     *  @returns The number of entries to be listed in the box. */
    std::uint32_t getEntryCount() const;

    /** @brief Adds a run-length coded value for a particular group description index
     *  @param [in] sampleCount Number of samples with the same group description index consecutively.
     *  @param [in] groupDescriptionIndex 1-based group description index */
    void addSampleRun(std::uint32_t sampleCount, std::uint32_t groupDescriptionIndex);

    /** @brief Gets the number of entries in the box
     *  @param [in] sampleIndex 1-based sample index to retrieve the group description index.
     *  @returns 1-based group description index. */
    std::uint32_t getSampleGroupDescriptionIndex(std::uint32_t sampleIndex) const;

    /** @brief Gets the first sample Id that has the requedted group description index.
     *  @param [in] groupDescriptionIndex 1-based group description index.
     *  @returns 1-based sample index
     *  @throws Run-time Error if there are no entries for the sample Id */
    std::uint32_t getSampleId(std::uint32_t groupDescriptionIndex) const;

    /** @brief Gets the number of samples listed.
     *  @returns Number of samples*/
    unsigned int getNumberOfSamples() const;

private:
    std::string   mGroupingType; /// Grouping Type
    std::uint32_t mEntryCount; /// Entry count value
    std::uint32_t mGroupingTypeParameter; /// Grouping Type parameter as defined in the ISOBMFF specification

    struct SampleRun  /// data structure of a run-length coded sample run
    {
        std::uint32_t sampleCount;
        std::uint32_t groupDescriptionIndex;
    };
    std::vector<SampleRun> mRunOfSamples;/// vector of sample IDs in the sample run

    /// Internal structure, flattened mRunOfSamples
    std::vector<std::uint32_t> mSampleToGroupIndex;

    /// Update mSampleToGroupIndex after mRunOfSamples has been edited
    void updateInternalIndex();
};

#endif /* end of include guard: SAMPLETOGROUPBOX_HPP */

