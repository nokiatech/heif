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

#include "movieextendsbox.hpp"

#include "log.hpp"

MovieExtendsBox::MovieExtendsBox()
    : Box("mvex")
    , mMovieExtendsHeaderBoxPresent(false)
    , mMovieExtendsHeaderBox()
    , mTrackExtends()
{
}

void MovieExtendsBox::addMovieExtendsHeaderBox(const MovieExtendsHeaderBox& movieExtendsHeaderBox)
{
    mMovieExtendsHeaderBoxPresent = true;
    mMovieExtendsHeaderBox        = movieExtendsHeaderBox;
}

bool MovieExtendsBox::isMovieExtendsHeaderBoxPresent() const
{
    return mMovieExtendsHeaderBoxPresent;
}

const MovieExtendsHeaderBox& MovieExtendsBox::getMovieExtendsHeaderBox() const
{
    return mMovieExtendsHeaderBox;
}

void MovieExtendsBox::addTrackExtendsBox(UniquePtr<TrackExtendsBox> trackExtendsBox)
{
    mTrackExtends.push_back(std::move(trackExtendsBox));
}

const Vector<TrackExtendsBox*> MovieExtendsBox::getTrackExtendsBoxes() const
{
    Vector<TrackExtendsBox*> trackExtendsBoxes;
    for (auto& trackExtends : mTrackExtends)
    {
        trackExtendsBoxes.push_back(trackExtends.get());
    }
    return trackExtendsBoxes;
}

void MovieExtendsBox::writeBox(BitStream& bitstr) const
{
    writeBoxHeader(bitstr);
    if (isMovieExtendsHeaderBoxPresent() == true)
    {
        mMovieExtendsHeaderBox.writeBox(bitstr);
    }
    for (auto& trackExtends : mTrackExtends)
    {
        trackExtends->writeBox(bitstr);
    }
    updateSize(bitstr);
}

void MovieExtendsBox::parseBox(BitStream& bitstr)
{
    parseBoxHeader(bitstr);
    bool foundTrex = false;
    while (bitstr.numBytesLeft() > 0)
    {
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        if (boxType == "mehd")
        {
            mMovieExtendsHeaderBoxPresent = true;
            mMovieExtendsHeaderBox.parseBox(subBitstr);
        }
        else if (boxType == "trex")
        {
            UniquePtr<TrackExtendsBox> trackExtendsBox(CUSTOM_NEW(TrackExtendsBox, ()));
            trackExtendsBox->parseBox(subBitstr);
            mTrackExtends.push_back(std::move(trackExtendsBox));
            foundTrex = true;
        }
        else
        {
            logWarning() << "Skipping an unsupported box '" << boxType.getString() << "' inside MovieExtendsBox."
                         << std::endl;
        }
    }

    if (!foundTrex)
    {
        throw RuntimeError("MovieExtendsBox::parseBox cannot find mandatory TrackExtendsBox box");
    }
}
