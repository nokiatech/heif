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

#ifndef TRACKGROUPBOX_HPP
#define TRACKGROUPBOX_HPP

#include <functional>
#include <memory>
#include <vector>

#include "bbox.hpp"
#include "customallocator.hpp"
#include "trackgrouptypebox.hpp"

/** @brief Track Group Box class. Extends from Box.
 *  @details Contains zero or more TrackGroupTypeBoxes as defined in the ISOBMFF standard. **/
class TrackGroupBox : public Box
{
public:
    TrackGroupBox();
    ~TrackGroupBox() override = default;

    /** @brief Gets Track Group Type Boxes inside TrackGroupBox
     *  @returns Vector<TrackGroupTypeBox> containing Track Group Type Boxes */
    const Vector<std::reference_wrapper<const TrackGroupTypeBox>> getTrackGroupTypeBoxes() const;

    /** @brief Adds Track Group Type Box into TrackGroupBox
     *  @param [in]  trackGroupTypeBox TrackGroupTypeBox that contains the box data */
    void addTrackGroupTypeBox(const TrackGroupTypeBox& trackGroupTypeBox);

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parses a TrackGroupBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    Vector<std::shared_ptr<TrackGroupTypeBox>> mTrackGroupTypeBoxes;
};

#endif  // TRACKGROUPBOX_HPP
