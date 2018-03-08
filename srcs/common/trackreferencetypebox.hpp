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

#ifndef TRACKREFERENCETYPEBOX_HPP
#define TRACKREFERENCETYPEBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"


/** @brief TrackReferenceTypeBox class. Extends from Box.
 *  @details This box contains a particular type of reference. Reference types are defined in the ISOBMFF standard. */
class TrackReferenceTypeBox : public Box
{
public:
    TrackReferenceTypeBox(FourCCInt trefType);
    virtual ~TrackReferenceTypeBox() = default;

    /** @brief Sets track IDs that reference this track
     *  @param [in] trackId vector of track Ids */
    void setTrackIds(const Vector<std::uint32_t>& trackId);

    /** @brief Gets track IDs that reference this track
     *  @returns Vector of track Ids */
    const Vector<std::uint32_t>& getTrackIds() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a TrackReferenceTypeBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    Vector<std::uint32_t> mTrackId;  ///< Vector of referenced track Ids
};

#endif /* end of include guard: TRACKREFERENCETYPEBOX_HPP */
