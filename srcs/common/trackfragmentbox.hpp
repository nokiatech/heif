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

#ifndef TRACKFRAGMENTBOX_HPP
#define TRACKFRAGMENTBOX_HPP

#include <memory>

#include "bbox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"
#include "trackextendsbox.hpp"
#include "trackfragmentbasemediadecodetimebox.hpp"
#include "trackfragmentheaderbox.hpp"
#include "trackrunbox.hpp"

/**
 * @brief  Track Fragment Box class
 * @details 'traf' box implementation as specified in the ISOBMFF specification.
 */
class TrackFragmentBox : public Box
{
public:
    TrackFragmentBox(Vector<MOVIEFRAGMENTS::SampleDefaults>& sampleDefaults);
    ~TrackFragmentBox() override = default;

    /** @return Reference to the contained TrackFragmentHeaderBox. */
    TrackFragmentHeaderBox& getTrackFragmentHeaderBox();

    /**
     * Add a TrackRunBox to TrackFragmentBox
     * @param trackRunBox TrackRunBox to add. */
    void addTrackRunBox(UniquePtr<TrackRunBox> trackRunBox);

    /** @return Pointers to all contained TrackRunBoxes. */
    Vector<TrackRunBox*> getTrackRunBoxes();

    /**
     * Add a TrackFragmentBaseMediaDecodeTimeBox to TrackFragmentBox
     * @param trackRunBox TrackRunBox to add. */
    void setTrackFragmentDecodeTimeBox(UniquePtr<TrackFragmentBaseMediaDecodeTimeBox> trackFragmentDecodeTimeBox);

    /** @return Pointers to all contained TrackFragmentBaseMediaDecodeTimeBoxes. */
    TrackFragmentBaseMediaDecodeTimeBox* getTrackFragmentBaseMediaDecodeTimeBox();

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
    TrackFragmentHeaderBox mTrackFragmentHeaderBox;
    Vector<MOVIEFRAGMENTS::SampleDefaults>& mSampleDefaults;
    // Optional boxes:
    Vector<UniquePtr<TrackRunBox>> mTrackRunBoxes;  ///< Contains TrackRunBoxes
    UniquePtr<TrackFragmentBaseMediaDecodeTimeBox> mTrackFragmentDecodeTimeBox;
};

#endif /* end of include guard: TRACKFRAGMENTBOX_HPP */
