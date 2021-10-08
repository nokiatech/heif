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

#ifndef MOVIEEXTENDSBOX_HPP
#define MOVIEEXTENDSBOX_HPP

#include "bbox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"
#include "movieextendsheaderbox.hpp"
#include "trackextendsbox.hpp"

/**
 * @brief  Movie Extends Box class
 * @details 'mvex' box implementation as specified in the ISOBMFF specification.
 */
class MovieExtendsBox : public Box
{
public:
    MovieExtendsBox();
    ~MovieExtendsBox() override = default;

    /**
     * Add a MovieExtendsHeaderBox to MovieExtendsBox
     * @param movieExtendsHeaderBox MovieExtendsHeaderBox to add. */
    void addMovieExtendsHeaderBox(const MovieExtendsHeaderBox& movieExtendsHeaderBox);

    /** @return True if the MovieExtendsBox contains a MovieExtendsHeaderBox */
    bool isMovieExtendsHeaderBoxPresent() const;

    /** @return Reference to the contained MovieExtendsHeaderBox. */
    const MovieExtendsHeaderBox& getMovieExtendsHeaderBox() const;

    /**
     * Add a TrackExtendsBox to MovieExtendsBox
     * @param trackExtendsBox TrackExtendsBox to add. */
    void addTrackExtendsBox(UniquePtr<TrackExtendsBox> trackExtendsBox);

    /** @return Pointers to all contained TrackExtendsBoxes. */
    const Vector<TrackExtendsBox*> getTrackExtendsBoxes() const;

    /**
     * @brief Serialize box data to the ISOBMFF::BitStream.
     * @see Box::writeBox()
     */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /**
     * @brief Deserialize box data from the ISOBMFF::BitStream.
     * @see Box::parseBox()
     */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    bool mMovieExtendsHeaderBoxPresent;
    MovieExtendsHeaderBox mMovieExtendsHeaderBox;
    Vector<UniquePtr<TrackExtendsBox>> mTrackExtends;  ///< Contained TrackExtendsBoxes
};

#endif /* end of include guard: MOVIEEXTENDSBOX_HPP */
