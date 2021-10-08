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

#ifndef MOVIEFRAGMENTBOX_HPP
#define MOVIEFRAGMENTBOX_HPP

#include <memory>

#include "bbox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"
#include "moviefragmentheaderbox.hpp"
#include "trackextendsbox.hpp"
#include "trackfragmentbox.hpp"

/**
 * @brief  Movie Fragment Box class
 * @details 'moof' box implementation as specified in the ISOBMFF specification.
 */
class MovieFragmentBox : public Box
{
public:
    MovieFragmentBox(Vector<MOVIEFRAGMENTS::SampleDefaults>& sampleDefaults);
    ~MovieFragmentBox() override = default;

    /** @return Reference to the contained MovieFragmentHeaderBox. */
    MovieFragmentHeaderBox& getMovieFragmentHeaderBox();

    /**
     * Add a TrackFragmentBox to MovieFragmentBox
     * @param trackFragmentBox TrackFragmentBox to add. */
    void addTrackFragmentBox(UniquePtr<TrackFragmentBox> trackFragmentBox);

    /** @return Pointers to all contained TrackFragmentBoxes. */
    Vector<TrackFragmentBox*> getTrackFragmentBoxes();

    /** @brief Sets movie fragment first byte offset inside the segment data */
    void setMoofFirstByteOffset(std::uint64_t moofFirstByteOffset);

    /** @return std::uint64_t Gets movie fragment first byte offset inside the segment data */
    std::uint64_t getMoofFirstByteOffset();

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
    MovieFragmentHeaderBox mMovieFragmentHeaderBox;
    Vector<UniquePtr<TrackFragmentBox>> mTrackFragmentBoxes;  ///< Contained TrackFragmentBoxes
    Vector<MOVIEFRAGMENTS::SampleDefaults>& mSampleDefaults;
    std::uint64_t mFirstByteOffset;  ///< Offset of 1st byte of this moof inside its segment/file
};

#endif /* end of include guard: MOVIEFRAGMENTBOX_HPP */
