/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef TRACKGROUPTYPEBOX_HPP
#define TRACKGROUPTYPEBOX_HPP

#include <cstdint>

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"

/** @brief Track Group Type Box class. Extends from FullBox.
 *  @details inside 'trgr' box as specified in the ISOBMFF specification
 */
class TrackGroupTypeBox : public FullBox
{
public:
    TrackGroupTypeBox(FourCCInt boxType, std::uint32_t trackGroupId = 0);
    ~TrackGroupTypeBox() override = default;

    /** @brief Gets track group id.
     *  @returns std::uint32_t containing track group id. */
    std::uint32_t getTrackGroupId() const;

    /** @brief Sets track group id. Applicable to track group type boxes of type obsp.
     * The pair of track_group_id and track_group_type identifies a track group
     *  @param [in]  bitstr Bitstream that contains the box data */
    void setTrackGroupId(std::uint32_t trackGroupId);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parses an Track Group Type Box bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

    virtual TrackGroupTypeBox* clone() const;

private:
    std::uint32_t mTrackGroupId;  ///< indicates the grouping type
};

#endif /* TRACKGROUPTYPEBOX_HPP */
