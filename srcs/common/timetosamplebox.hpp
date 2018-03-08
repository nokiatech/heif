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

#ifndef TIMETOSAMPLEBOX_HPP
#define TIMETOSAMPLEBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"

/** @brief TimeToSampleBox class. Extends from FullBox.
 *  @details 'stts' box contains display timing information of samples as defined in the ISOBMFF standard.
 *  @details Currently version 0 is supported. */
class TimeToSampleBox : public FullBox
{
public:
    /// Entries in this box can be of type version 0 or version 1
    struct EntryVersion0
    {
        std::uint32_t mSampleCount;
        std::uint32_t mSampleDelta;
    };

    TimeToSampleBox();
    virtual ~TimeToSampleBox() = default;

    /** @brief Get sample timing information.
     *  @returns vector of sample timing information. */
    Vector<std::uint32_t> getSampleTimes() const;

    /** @brief Get sample timing information as sample deltas.
     *  @returns vector of sample timing information as sample deltas. */
    Vector<std::uint32_t> getSampleDeltas() const;

    /** @brief Get sample count without consuming potentially a lot of memory.
     *  @returns the number of samples */
    std::uint32_t getSampleCount() const;

    /** @brief Get sample decoding delta value information.
     *  @returns decode delta entry in version 0 format. */
    EntryVersion0& getDecodeDeltaEntry();

    /** @brief Adds a sample delta; or merges it to previous enty
     * if it's the same value. */
    void addSampleDelta(std::uint32_t mSampleDelta);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a TimeToSampleBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    Vector<EntryVersion0> mEntryVersion0;  ///< Vector of decode delta entries of type version 0
};

#endif /* end of include guard: TIMETOSAMPLEBOX_HPP */
