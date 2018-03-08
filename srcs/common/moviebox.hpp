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

#ifndef MOVIEBOX_HPP
#define MOVIEBOX_HPP

#include "bbox.hpp"
#include "customallocator.hpp"
#include "movieheaderbox.hpp"
#include "trackbox.hpp"


/**
 * Movie Box class
 * @details 'moov' box implementation as specified in the ISOBMFF specification.
 */
class MovieBox : public Box
{
public:
    MovieBox();
    virtual ~MovieBox() = default;

    /**
     * @brief Clear track boxes, initialize mvhd and other data.
     */
    void clear();

    /** @return Reference to the contained MovieHeaderBox. */
    MovieHeaderBox& getMovieHeaderBox();
    const MovieHeaderBox& getMovieHeaderBox() const;

    /** @return Pointers to all contained TrackBoxes. */
    Vector<TrackBox*> getTrackBoxes();

    /** @return Pointer to contained TrackBox with given track id. */
    TrackBox* getTrackBox(uint32_t trackId);

    bool isOzoPreviewFile() const;

    bool isMovieExtendsBoxPresent() const;

    /**
     * Add a TrackBox to MovieBox
     * @param trackBox TrackBox to add. */
    void addTrackBox(UniquePtr<TrackBox> trackBox);

    /**
     * @brief Serialize box data to the ISOBMFF::BitStream.
     * @see Box::writeBox()
     */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /**
     * @brief Deserialize box data from the ISOBMFF::BitStream.
     * @see Box::parseBox()
     */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    MovieHeaderBox mMovieHeaderBox;       ///< The mandatory MovieHeaderBox
    Vector<UniquePtr<TrackBox>> mTracks;  ///< Contained TrackBoxes
    bool mIsOzoPreviewFile;               ///< Whether file is Ozo Preview file
};

#endif /* end of include guard: MOVIEBOX_HPP */
