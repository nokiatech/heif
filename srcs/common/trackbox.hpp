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

#ifndef TRACKBOX_HPP
#define TRACKBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"
#include "editbox.hpp"
#include "mediabox.hpp"
#include "trackgroupbox.hpp"
#include "trackheaderbox.hpp"
#include "trackreferencebox.hpp"
#include "tracktypebox.hpp"

/** @brief TrackBox class. Extends from Box.
 *  @details 'trak' box contains all relevant meta information of a media track as defined in ISOBMFF specification. */
class TrackBox : public Box
{
public:
    TrackBox();
    ~TrackBox() override = default;

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

    /** @brief Gets the reference of the Track Group Box.
     *  @return Reference to the  Track Group Box. **/
    TrackGroupBox& getTrackGroupBox();

    /** @brief Gets the reference of the Track Group Box.
     *  @return Reference to the  Track Group Box. **/
    const TrackGroupBox& getTrackGroupBox() const;

    /** @brief Sets information about whether the track has track group box.
     *  @param [in] value TRUE if track has track group box, FALSE otherwise **/
    void setHasTrackGroup(bool value);

    /** @brief Gets information about whether the track has other track group boxes associated with it.
     *  @return TRUE if track has track group, FALSE otherwise **/
    bool getHasTrackGroup() const;

    /** @brief Gets the reference of the contained box
     *  @return Reference to the TrackTypeBox. */
    TrackTypeBox& getTrackTypeBox();

    /** @brief Gets the reference of the contained box
     *  @return Reference to the TrackTypeBox. */
    const TrackTypeBox& getTrackTypeBox() const;

    /** @brief Sets whether the track contains TrackTypeBox.
     *  @param [in] value TRUE if track has the box, FALSE otherwise */
    void setHasTrackTypeBox(bool value);

    /** @brief Gets whether the track contains TrackTypeBox.
     *  @return TRUE if track has the box, FALSE otherwise */
    bool getHasTrackTypeBox() const;

    /** @brief Set EditBox */
    void setEditBox(const EditBox& EditBox);

    /** @return shared_ptr to EditBox if set, or nullptr */
    std::shared_ptr<const EditBox> getEditBox() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /** @brief Parses a TrackBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    TrackHeaderBox mTrackHeaderBox;  ///< Track Header Box
    MediaBox mMediaBox;              ///< Media Box related to this track

    TrackReferenceBox mTrackReferenceBox;  ///< Track Reference Box
    bool mHasTrackReferences;              ///< Flag that shows whether the track has references from other tracks

    TrackGroupBox mTrackGroupBox;  ///< Track Group Box
    bool mHasTrackGroupBox;        ///< Flag that shows whether the track has a track group box

    TrackTypeBox mTrackTypeBox;  ///< Track Type Box
    bool mHasTrackTypeBox;       ///< Flag that shows whether the track has a track type box

    std::shared_ptr<EditBox> mEditBox;  ///< Edit box (optional)
};

#endif /* end of include guard: TRACKBOX_HPP */
