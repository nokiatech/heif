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

#ifndef MOVIEBOX_HPP
#define MOVIEBOX_HPP

#include "bbox.hpp"
#include "movieheaderbox.hpp"
#include "trackbox.hpp"

#include <memory>
#include <vector>

/**
 * Movie Box class
 * @details 'moov' box implementation as specified in the ISOBMFF specification.
 */
class MovieBox : public Box
{
public:
    MovieBox();
    virtual ~MovieBox() = default;

    /** @return Reference to the contained MovieHeaderBox. */
    MovieHeaderBox& getMovieHeaderBox();

    /** @return Pointers to all contained TrackBoxes. */
    std::vector<TrackBox*> getTrackBoxes();

    /** @return True if the MovieBox contains a MetaBox */
    bool isMetaBoxPresent() const;

    /**
     * Add a TrackBox to MovieBox
     * @param trackBox TrackBox to add. */
    void addTrackBox(std::unique_ptr<TrackBox> trackBox);

    /**
     * @brief Serialize box data to the BitStream.
     * @see Box::writeBox()
     */
    virtual void writeBox(BitStream& bitstr);

    /**
     * @brief Deserialize box data from the BitStream.
     * @see Box::parseBox()
     */
    virtual void parseBox(BitStream& bitstr);

private:
    MovieHeaderBox mMovieHeaderBox;                 ///< The mandatory MovieHeaderBox
    std::vector<std::unique_ptr<TrackBox>> mTracks; ///< Contained TrackBoxes
};

#endif /* end of include guard: MOVIEBOX_HPP */
