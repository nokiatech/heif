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

#ifndef SAMPLEDESCRIPTIONBOX_HPP
#define SAMPLEDESCRIPTIONBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"
#include "sampleentrybox.hpp"


/** @brief SampleDescriptionBox class. Extends from FullBox.
 *  @details This class makes use of templates in order to support different sample entry types. */
class SampleDescriptionBox : public FullBox
{
public:
    SampleDescriptionBox();
    ~SampleDescriptionBox() = default;

    /** @brief Add a new sample entry.
     *  @param [in] sampleEntry Sample Entry of type SampleEntryBox */
    void addSampleEntry(UniquePtr<SampleEntryBox> sampleEntry);

    /** @brief Get the list of sample entries.
    *  @returns Vector of sample entries */
    const Vector<UniquePtr<SampleEntryBox>>& getSampleEntries() const;

    /** @brief Get the sample entry at a particular index from the list.
    *  @param [in] index 1-based index of the sample entry
    *  @returns Sample Entry of defined type */
    const SampleEntryBox* getSampleEntry(unsigned int index) const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a SampleDescriptionBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr);

private:
    Vector<UniquePtr<SampleEntryBox>> mIndex;  ///< Vector of sample entries
};
#endif /* end of include guard: SAMPLEDESCRIPTIONBOX_HPP */
