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

#ifndef TRACKREFERENCETYPEBOX_HPP
#define TRACKREFERENCETYPEBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"

#include <string>

/** @brief TrackReferenceTypeBox class. Extends from Box.
 *  @details This box contains a particular type of reference. Reference types are defined in the ISOBMFF and HEIF standards.**/
class TrackReferenceTypeBox : public Box
{
public:
    TrackReferenceTypeBox(FourCCInt trefType);
    virtual ~TrackReferenceTypeBox() = default;

    /** @brief Sets track IDs that reference this track
     *  @param [in] trackId vector of track Ids **/
    void setTrackIds(const std::vector<std::uint32_t>& trackId);

    /** @brief Gets track IDs that reference this track
     *  @returns Vector of track Ids **/
    const std::vector<std::uint32_t>& getTrackIds() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a TrackReferenceTypeBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    std::vector<std::uint32_t> mTrackId; ///< Vector of referenced track Ids
};

#endif /* end of include guard: TRACKREFERENCETYPEBOX_HPP */
