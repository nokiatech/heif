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

#ifndef SAMPLEGROUPDESCRIPTIONBOX_HPP
#define SAMPLEGROUPDESCRIPTIONBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"
#include "samplegroupdescriptionentry.hpp"


/** @brief Sample Group Description Box class. Extends from FullBox.
 *  @details Contains Sample Group Description Box data structure as defined in the ISOBMFF standard. */
class SampleGroupDescriptionBox : public FullBox
{
public:
    SampleGroupDescriptionBox();
    virtual ~SampleGroupDescriptionBox() = default;

    /** @brief Set the version of the Box.
     *  @param [in] version Box version */
    void setVersion(std::uint8_t version);

    /** @brief Set the grouping type to use.
     *  @param [in] groupingType Grouping Type as defined in ISOBMFF standard.
     *  @throws Run time error if the groupingtype is bigger than 4 characters. */
    void setGroupingType(FourCCInt groupingType);

    /** @brief Get the grouping type used.
     *  @return Grouping Type as defined in ISOBMFF standard. */
    FourCCInt getGroupingType() const;

    /** @brief Get index of entry having a desired sampleId.
     * @details This is used for mapping ids in ReferencedSampleListEntries to
     * sample ids (=0-based index of samples in track, decoding order).
     * @param [in] sampleId Sample ID
     * @throws Run-time error if sampleId is invalid. */
    std::uint32_t getEntryIndexOfSampleId(std::uint32_t sampleId) const;

    /** @brief Set default byte length of the sample group description.
     * @details This parameter indicates the default length of the box.
     * @param [in] defaultLength default byte length */
    void setDefaultLength(std::uint32_t defaultLength);

    /** @brief Get default byte length of the sample group description.
     * @details This parameter indicates the default length of the box.
     * @returns default byte length */
    std::uint32_t getDefaultLength() const;

    /** @brief Add a new entry to the sample group description box.
     * @param [in] sampleGroupEntry sample group entry data structure */
    void addEntry(UniquePtr<SampleGroupDescriptionEntry> sampleGroupEntry);

    /** @brief Get a Sample Group Entry
     * @param  [in] index Entry index (1-based)
     * @returns Reference to the requested SampleGroupEntry */
    const SampleGroupDescriptionEntry* getEntry(std::uint32_t index) const;

    /** returns Number of contained sample group entries. */
    uint32_t getEntryCount() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data.
     *  @throws Run-time Error if there are no sample description entries or groiping type is not properly set. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a SampleGroupDescriptionBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    FourCCInt mGroupingType;       ///< Grouping type
    std::uint32_t mDefaultLength;  ///< Default byte size of the description

    Vector<UniquePtr<SampleGroupDescriptionEntry>> mSampleGroupEntry;  ///< Vector of sample group entries
};

#endif /* end of include guard: SAMPLEGROUPDESCRIPTIONBOX_HPP */
