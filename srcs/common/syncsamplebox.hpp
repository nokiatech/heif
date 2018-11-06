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

#ifndef SYNCSAMPLEBOX_HPP
#define SYNCSAMPLEBOX_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"

/** @brief SyncSampleBox class. Extends from FullBox.
 *  @details 'stss' box contains Sync sample entries as defined in the ISOBMFF standard. */
class SyncSampleBox : public FullBox
{
public:
    SyncSampleBox();
    virtual ~SyncSampleBox() = default;

    /** @brief Add a sample as a sync sample.
     *  @param [in] sampleNumber 1-based sample number in the track to be added as a sync sample. */
    void addSample(std::uint32_t sampleNumber);

    const Vector<std::uint32_t>& getSyncSampleIds() const;

    void setSampleCountMaxSafety(int64_t sampleCountMax);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a SyncSampleBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    Vector<std::uint32_t> mSampleNumber;  ///< Vector of sync sample Ids

    int64_t mSampleCountMax;
};

#endif /* end of include guard: SYNCSAMPLEBOX_HPP */
