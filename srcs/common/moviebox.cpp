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

#include "moviebox.hpp"

#include "bitstream.hpp"
#include "log.hpp"

MovieBox::MovieBox()
    : Box("moov")
    , mMovieHeaderBox()
    , mTracks()
{
}

void MovieBox::clear()
{
    mMovieHeaderBox = {};
    mTracks.clear();
}

MovieHeaderBox& MovieBox::getMovieHeaderBox()
{
    return mMovieHeaderBox;
}

const MovieHeaderBox& MovieBox::getMovieHeaderBox() const
{
    return mMovieHeaderBox;
}

const Vector<UniquePtr<TrackBox>>& MovieBox::getTrackBoxes() const
{
    return mTracks;
}

TrackBox* MovieBox::getTrackBox(uint32_t trackId)
{
    for (auto& track : mTracks)
    {
        if (track.get()->getTrackHeaderBox().getTrackID() == trackId)
        {
            return track.get();
        }
    }
    return nullptr;
}

bool MovieBox::isMovieExtendsBoxPresent() const
{
    return mMovieExtendsBox != nullptr;
}

const MovieExtendsBox* MovieBox::getMovieExtendsBox() const
{
    return mMovieExtendsBox.get();
}

void MovieBox::addMovieExtendsBox(UniquePtr<MovieExtendsBox> movieExtendsBox)
{
    mMovieExtendsBox = std::move(movieExtendsBox);
}

void MovieBox::addTrackBox(UniquePtr<TrackBox> trackBox)
{
    mTracks.push_back(std::move(trackBox));
}

void MovieBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeBoxHeader(bitstr);

    mMovieHeaderBox.writeBox(bitstr);

    for (auto& track : mTracks)
    {
        track->writeBox(bitstr);
    }

    updateSize(bitstr);
}

void MovieBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    while (bitstr.numBytesLeft() > 0)
    {
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        if (boxType == "mvhd")
        {
            mMovieHeaderBox.parseBox(subBitstr);
        }
        else if (boxType == "trak")
        {
            UniquePtr<TrackBox> trackBox(CUSTOM_NEW(TrackBox, ()));
            trackBox->parseBox(subBitstr);
            // Ignore box if the handler type is not pict
            FourCCInt handlerType = trackBox->getMediaBox().getHandlerBox().getHandlerType();
            if (handlerType == "pict" ||  // Image Sequence track
                handlerType == "auxv" ||  // Auxiliary Image Sequence track
                handlerType == "soun" ||  // Audio track
                handlerType == "vide")    // Video track
            {
                mTracks.push_back(move(trackBox));
            }
        }
        else if (boxType == "mvex")
        {
            mMovieExtendsBox = makeCustomUnique<MovieExtendsBox, MovieExtendsBox>();
            mMovieExtendsBox->parseBox(subBitstr);
        }
        else
        {
            logWarning() << "Skipping an unsupported box '" << boxType.getString() << "' inside movie box."
                         << std::endl;
        }
    }
}
