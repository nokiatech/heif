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

#ifndef TIMETOSAMPLEBOX_HPP
#define TIMETOSAMPLEBOX_HPP

#include "bitstream.hpp"
#include "fullbox.hpp"

/** @brief TimeToSampleBox class. Extends from FullBox.
 *  @details 'stts' box contains display timing information of samples as defined in the ISOBMFF and HEIF standards.
 *  @details Currently version 0 is supported. **/
class TimeToSampleBox : public FullBox
{
public:
    /// entries in this box can be of typeversion 0 or version 1
    struct EntryVersion0
    {
        std::uint32_t mSampleCount;
        std::uint32_t mSampleDelta;
    };

    TimeToSampleBox();
    virtual ~TimeToSampleBox() = default;

    /** @brief Get sample timing information.
     *  @returns vector of sample timing information. **/
    std::vector<std::uint32_t> getSampleTimes() const;

    /** @brief Get sample decoding delta value information.
     *  @returns decode delta entry in version 0 format. **/
    EntryVersion0& getDecodeDeltaEntry();

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a TimeToSampleBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    std::vector<EntryVersion0> mEntryVersion0; /// vector of decode delta entries of type version 0
};


#endif /* end of include guard: TIMETOSAMPLEBOX_HPP */
