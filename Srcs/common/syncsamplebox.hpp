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

#ifndef SYNCSAMPLEBOX_HPP
#define SYNCSAMPLEBOX_HPP

#include "fullbox.hpp"
#include <vector>

/** @brief SyncSampleBox class. Extends from FullBox.
 *  @details 'stss' box contains Sync sample entries as defined in the ISOBMFF and HEIF standards. **/
class SyncSampleBox : public FullBox
{
public:
    SyncSampleBox();
    virtual ~SyncSampleBox() = default;

    /** @brief Add a sample as a sync sample.
     *  @param [in] sampleNumber 1-based sample number in the track to be added as a sync sample.**/
    void addSample(std::uint32_t sampleNumber);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a SyncSampleBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    std::vector<std::uint32_t> mSampleNumber; ///< Vector of sync sample Ids
};


#endif /* end of include guard: SYNCSAMPLEBOX_HPP */

