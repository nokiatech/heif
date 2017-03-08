/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#ifndef COMPOSITIONOFFSETBOX_HPP
#define COMPOSITIONOFFSETBOX_HPP

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

    /// @return vector of sample composition offsets as signed integers
    std::vector<int> getSampleCompositionOffsets() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data
     *  @throws Runtime Error if the write operation is unsuccessful */
    void writeBox(BitStream& bitstr);

    /** @brief Parses a Composition Offset Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(BitStream& bitstr);

private:
    /// @brief vector of sample offset entries of type version 0
    std::vector<EntryVersion0> mEntryVersion0;

    /// @brief vector of sample offset entries of type version 1
    std::vector<EntryVersion1> mEntryVersion1;
};

#endif /* end of include guard: COMPOSITIONOFFSETBOX_HPP */

