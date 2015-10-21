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

#ifndef TRACKBOX_HPP
#define TRACKBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "mediabox.hpp"
#include "trackheaderbox.hpp"
#include "trackreferencebox.hpp"


/** @brief TrackBox class. Extends from Box.
 *  @details 'trak' box contains all relevant meta information of a media track as defined in ISOBMFF specification. **/
class TrackBox: public Box
{
public:
    TrackBox();
    virtual ~TrackBox() = default;


    /** @brief Sets information about whether the track has other reference tracks associated with it.
     *  @param [in] value TRUE if track has reference to other tracks, FALSE otherwise **/
    void setHasTrackReferences(bool value = true);

    /** @brief Gets information about whether the track has other reference tracks associated with it.
     *  @return TRUE if track has reference to other tracks, FALSE otherwise **/
    bool getHasTrackReferences() const;

    /** @brief Gets the reference of the Track Header Box.
     *  @return Reference to the  Track Header Box. **/
    TrackHeaderBox& getTrackHeaderBox();

    /** @brief Gets the reference of the Media Header Box.
     *  @return Reference to the  Media Header Box. **/
    MediaBox& getMediaBox();

    /** @brief Gets the reference of the Track Reference Box.
     *  @return Reference to the  Track Reference Box. **/
    TrackReferenceBox& getTrackReferenceBox();

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(BitStream& bitstr);

    /** @brief Parses a TrackBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(BitStream& bitstr);

private:
    TrackHeaderBox mTrackHeaderBox; /// Track Header Box
    MediaBox mMediaBox; /// Media Box related to this track
    TrackReferenceBox mTrackReferenceBox; /// Track Reference Box
    bool mHasTrackReferences; /// Flag that shows whether the track has references from other tracks
};

#endif /* end of include guard: TRACKBOX_HPP */
