/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
    , mIsOzoPreviewFile(false)
{
}

void MovieBox::clear()
{
    mMovieHeaderBox = {};
    mTracks.clear();
    mIsOzoPreviewFile = false;
}

MovieHeaderBox& MovieBox::getMovieHeaderBox()
{
    return mMovieHeaderBox;
}

const MovieHeaderBox& MovieBox::getMovieHeaderBox() const
{
    return mMovieHeaderBox;
}

Vector<TrackBox*> MovieBox::getTrackBoxes()
{
    Vector<TrackBox*> trackBoxes;
    for (auto& track : mTracks)
    {
        trackBoxes.push_back(track.get());
    }
    return trackBoxes;
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

void MovieBox::addTrackBox(UniquePtr<TrackBox> trackBox)
{
    mTracks.push_back(std::move(trackBox));
}

bool MovieBox::isOzoPreviewFile() const
{
    return mIsOzoPreviewFile;
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
                handlerType == "auxv" ||  // Auxiliary Image Sequence rack
                handlerType == "soun" ||  // Audio track
                handlerType == "vide")    // Video track
            {
                mTracks.push_back(move(trackBox));
            }
        }
        else if (boxType == "udta")
        {
            unsigned int udtaSize = subBitstr.read32Bits();
            if (udtaSize < 200)
            {
                String udtaData;
                subBitstr.readStringWithLen(udtaData, static_cast<unsigned int>(subBitstr.numBytesLeft()));
                std::size_t found = udtaData.find("NokiaPC");
                if (found != String::npos)
                {
                    // this is Ozo Preview file with "NokiaPC" in udta
                    mIsOzoPreviewFile = true;
                }
            }
        }
        else
        {
            logWarning() << "Skipping an unsupported box '" << boxType << "' inside movie box." << std::endl;
        }
    }
}
