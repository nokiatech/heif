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

#include "moviefragmentbox.hpp"

#include "log.hpp"

MovieFragmentBox::MovieFragmentBox(Vector<MOVIEFRAGMENTS::SampleDefaults>& sampleDefaults)
    : Box("moof")
    , mMovieFragmentHeaderBox()
    , mTrackFragmentBoxes()
    , mSampleDefaults(sampleDefaults)
    , mFirstByteOffset(0)
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
            UniquePtr<TrackFragmentBox> trackFragmentBox(CUSTOM_NEW(TrackFragmentBox, (mSampleDefaults)));
            trackFragmentBox->parseBox(subBitstr);
            mTrackFragmentBoxes.push_back(std::move(trackFragmentBox));
        }
        else
        {
            logWarning() << "Skipping an unsupported box '" << boxType.getString() << "' inside MovieFragmentBox."
                         << std::endl;
        }
    }
}
