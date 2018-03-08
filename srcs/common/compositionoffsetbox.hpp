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

#ifndef COMPOSITIONOFFSETBOX_HPP
#define COMPOSITIONOFFSETBOX_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"

/** @brief Composition Time to Sample Box class
 *  @details 'sttc' box implementation. Extends from FullBox.
 *  @details  Both version 0 and version 1 support exists.
 */
class CompositionOffsetBox : public FullBox
{
public:
    CompositionOffsetBox();
    virtual ~CompositionOffsetBox() = default;

    /// @brief version 1 supported data structure
    struct EntryVersion0
    {
        std::uint32_t mSampleCount;
        std::uint32_t mSampleOffset;
    };

    /// @brief version 0 supported dta structure. Sample offsets can be negative
    struct EntryVersion1
    {
        std::uint32_t mSampleCount;
        std::int32_t mSampleOffset;
    };

    /** @brief Inserts a new composition time to sample entry of verison 0 type
     *  @param [in] entry EntryVersion0 data structure
     *  @throws Runtime Error if the box version conflicts with the Entry Version */
    void addCompositionOffsetEntryVersion0(const EntryVersion0& entry);

    /** @brief Inserts a new composition time to sample entry of verison 0 type
     *  @param [in] entry EntryVersion0 data structure
     *  @throws Runtime Error if the box version conflicts with the Entry Version */
    void addCompositionOffsetEntryVersion1(const EntryVersion1& entry);

    /** @brief Get number of samples (used for box parsing safety checks)
     *  @return Sample count
     */
    uint32_t getSampleCount();

    /// @return vector of sample composition offsets as signed integers
    Vector<int> getSampleCompositionOffsets() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data
     *  @throws Runtime Error if the write operation is unsuccessful */
    void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a Composition Offset Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr);

private:
    /// @brief vector of sample offset entries of type version 0
    Vector<EntryVersion0> mEntryVersion0;

    /// @brief vector of sample offset entries of type version 1
    Vector<EntryVersion1> mEntryVersion1;
};

#endif /* end of include guard: COMPOSITIONOFFSETBOX_HPP */
