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

#ifndef SAMPLETOGROUPBOX_HPP
#define SAMPLETOGROUPBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"


/** @brief SampleToGroupBox class. Extends from FullBox.
 *  @details 'sbgp' box contains sample to group mappings as defined in the ISOBMFF standard. */
class SampleToGroupBox : public FullBox
{
public:
    SampleToGroupBox();
    virtual ~SampleToGroupBox() = default;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data.
     *  @throws Run-time Error of the box has no entries. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a SampleToGroupBox bitstream and fills in the necessary member variables
     *  @param [in] bitstr Bitstream that contains the box data
     *  @throws Run-time Error of the box has no entries. */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

    /** @brief Sets the grouping type
     *  @param [in] groupingType Grouping Type as a string
     *  @throws Run-time Error if the grouping type is less than 4 characters. */
    void setGroupingType(FourCCInt groupingType);

    /** @brief Gets the grouping type
     *  @returns Grouping Type as a string */
    FourCCInt getGroupingType() const;

    /** @brief Sets the grouping type parameter
    *  @param [in] groupingTypeParameter Grouping Type Parameter as a uint32_t. */
    void setGroupingTypeParameter(std::uint32_t groupingTypeParameter);

    /** @brief Gets the grouping type parameter.
    *  @returns Grouping Type as a uint32_t */
    std::uint32_t getGroupingTypeParameter() const;

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

    /** @brief Gets the 1-based group description index.
     *  @param [in] sampleIndex 1-based sample index to retrieve the group description index.
     *  @returns 1-based group description index. If sampleIndex is invalid value 0 is returned. */
    std::uint32_t getSampleGroupDescriptionIndex(std::uint32_t sampleIndex) const;

    /** @brief Gets the first sample Id that has the requested group description index.
     *  @param [in] groupDescriptionIndex 1-based group description index.
     *  @returns 1-based sample index
     *  @throws Run-time Error if there are no entries for the sample Id */
    std::uint32_t getSampleId(std::uint32_t groupDescriptionIndex) const;

    /** @brief Gets the number of samples listed.
     *  @returns Number of samples*/
    unsigned int getNumberOfSamples() const;

private:
    FourCCInt mGroupingType;               ///< Grouping Type
    std::uint32_t mEntryCount;             ///< Entry count value
    std::uint32_t mGroupingTypeParameter;  ///< Grouping Type parameter as defined in the ISOBMFF specification

    /// Data structure of a run-length coded sample run
    struct SampleRun
    {
        std::uint32_t sampleCount;
        std::uint32_t groupDescriptionIndex;
    };
    Vector<SampleRun> mRunOfSamples;  ///< Vector of sample IDs in the sample run

    /// Internal structure, flattened mRunOfSamples
    Vector<std::uint32_t> mSampleToGroupIndex;

    /// Update mSampleToGroupIndex after mRunOfSamples has been edited
    void updateInternalIndex();
};

#endif /* end of include guard: SAMPLETOGROUPBOX_HPP */
