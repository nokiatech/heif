/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "moviefragmentbox.hpp"

#include "bitstream.hpp"

#include "log.hpp"
#include "fourccint.hpp"
#include <memory>
#include <utility>
#include <vector>

MovieFragmentBox::MovieFragmentBox(Vector<MOVIEFRAGMENTS::SampleDefaults>& sampleDefaults)
    : Box("moof")
    , mMovieFragmentHeaderBox()
    , mTrackFragmentBoxes()
    , mSampleDefaults(sampleDefaults)
     
{
}

MovieFragmentHeaderBox& MovieFragmentBox::getMovieFragmentHeaderBox()
{
    return mMovieFragmentHeaderBox;
}

void MovieFragmentBox::addTrackFragmentBox(UniquePtr<TrackFragmentBox> trackFragmentBox)
{
    mTrackFragmentBoxes.push_back(std::move(trackFragmentBox));
}

Vector<TrackFragmentBox*> MovieFragmentBox::getTrackFragmentBoxes()
{
    Vector<TrackFragmentBox*> trackFragmentBoxes;
    for (auto& trackFragmentBox : mTrackFragmentBoxes)
    {
        trackFragmentBoxes.push_back(trackFragmentBox.get());
    }
    return trackFragmentBoxes;
}

void MovieFragmentBox::setMoofFirstByteOffset(const std::uint64_t moofFirstByteOffset)
{
    mFirstByteOffset = moofFirstByteOffset;
}

std::uint64_t MovieFragmentBox::getMoofFirstByteOffset()
{
    return mFirstByteOffset;
}

void MovieFragmentBox::writeBox(BitStream& bitstr) const
{
    writeBoxHeader(bitstr);
    mMovieFragmentHeaderBox.writeBox(bitstr);
    for (auto& trackFragmentBox : mTrackFragmentBoxes)
    {
        trackFragmentBox->writeBox(bitstr);
    }
    updateSize(bitstr);
}

void MovieFragmentBox::parseBox(BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    while (bitstr.numBytesLeft() > 0)
    {
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        if (boxType == "mfhd")
        {
            mMovieFragmentHeaderBox.parseBox(subBitstr);
        }
        else if (boxType == "traf")
        {
            auto trackFragmentBox = makeCustomUnique<TrackFragmentBox, TrackFragmentBox>(mSampleDefaults);
            trackFragmentBox->parseBox(subBitstr);
            mTrackFragmentBoxes.push_back(std::move(trackFragmentBox));
        }
        else
        {
            logWarning() << "Skipping an unsupported box '" << boxType.getString() << "' inside MovieFragmentBox."
                         << '\n';
        }
    }
}
