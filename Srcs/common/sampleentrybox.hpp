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

#ifndef SAMPLEENTRYBOX_HPP
#define SAMPLEENTRYBOX_HPP

#include "bbox.hpp"
#include <cstdint>

/** @brief Sample Entry Box class. Extends from Box.
 *  @details Contains Sample Entry data structure as defined in the ISOBMFF and HEIF standards. **/
class SampleEntryBox : public Box
{
public:
    SampleEntryBox(FourCCInt codingname);
    virtual ~SampleEntryBox() = default;

    /** @brief Get Data Reference Index.
     *  @returns Data reference index value **/
    std::uint16_t getDataReferenceIndex() const;

    /** @brief Set Data Reference Index.
     *  @param [in] dataReferenceIndex Data reference index value **/
    void setDataReferenceIndex(std::uint16_t dataReferenceIndex);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a SampleEntryBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    std::uint16_t mDataReferenceIndex;/// data reference index value
};

#endif
