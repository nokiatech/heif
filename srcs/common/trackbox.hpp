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

#ifndef TRACKBOX_HPP
#define TRACKBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"
#include "editbox.hpp"
#include "mediabox.hpp"
#include "trackheaderbox.hpp"
#include "trackreferencebox.hpp"

/** @brief TrackBox class. Extends from Box.
 *  @details 'trak' box contains all relevant meta information of a media track as defined in ISOBMFF specification. */
class TrackBox : public Box
{
public:
    TrackBox();
    virtual ~TrackBox() = default;

    /** @brief Sets information about whether the track has other reference tracks associated with it.
     *  @param [in] value TRUE if track has reference to other tracks, FALSE otherwise */
    void setHasTrackReferences(bool value = true);

    /** @brief Gets information about whether the track has other reference tracks associated with it.
     *  @return TRUE if track has reference to other tracks, FALSE otherwise */
    bool getHasTrackReferences() const;

    /** @brief Gets the reference of the Track Header Box.
     *  @return Reference to the  Track Header Box. */
    TrackHeaderBox& getTrackHeaderBox();

    /** @brief Gets the reference of the Track Header Box.
     *  @return Reference to the  Track Header Box. */
    const TrackHeaderBox& getTrackHeaderBox() const;

    /** @brief Gets the reference of the Media Header Box.
     *  @return Reference to the  Media Header Box. */
    MediaBox& getMediaBox();

    /** @brief Gets the reference of the Media Header Box.
     *  @return Reference to the  Media Header Box. */
    const MediaBox& getMediaBox() const;

    /** @brief Gets the reference of the Track Reference Box.
     *  @return Reference to the  Track Reference Box. */
    TrackReferenceBox& getTrackReferenceBox();

    /** @brief Gets the reference of the Track Reference Box.
     *  @return Reference to the  Track Reference Box. */
    const TrackReferenceBox& getTrackReferenceBox() const;

    /** @brief Set EditBox */
    void setEditBox(const EditBox& EditBox);

    /** @return shared_ptr to EditBox if set, or nullptr */
    std::shared_ptr<const EditBox> getEditBox() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a TrackBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr);

private:
    TrackHeaderBox mTrackHeaderBox;        ///< Track Header Box
    MediaBox mMediaBox;                    ///< Media Box related to this track
    TrackReferenceBox mTrackReferenceBox;  ///< Track Reference Box
    bool mHasTrackReferences;              ///< Flag that shows whether the track has references from other tracks

    std::shared_ptr<EditBox> mEditBox;  ///< Edit box (optional)
};

#endif /* end of include guard: TRACKBOX_HPP */
