/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#include "moviebox.hpp"
#include "bitstream.hpp"
#include "log.hpp"

MovieBox::MovieBox() :
    Box("moov"),
    mMovieHeaderBox(),
    mTracks()
{
}

MovieHeaderBox& MovieBox::getMovieHeaderBox()
{
    return mMovieHeaderBox;
}

std::vector<TrackBox*> MovieBox::getTrackBoxes()
{
    std::vector<TrackBox*> trackBoxes;
    for (auto& track : mTracks)
    {
        trackBoxes.push_back(track.get());
    }
    return trackBoxes;
}

void MovieBox::addTrackBox(std::unique_ptr<TrackBox> trackBox)
{
    mTracks.push_back(std::move(trackBox));
}


/** @todo Implement support for MovieBox-level MetaBox */
bool MovieBox::isMetaBoxPresent() const
{
    return false;
}

void MovieBox::writeBox(BitStream& bitstr)
{
    writeBoxHeader(bitstr);

    mMovieHeaderBox.writeBox(bitstr);
    for (auto& track : mTracks)
    {
        track->writeBox(bitstr);
    }

    updateSize(bitstr);
}

void MovieBox::parseBox(BitStream& bitstr)
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
            std::unique_ptr<TrackBox> trackBox(new TrackBox);
            trackBox->parseBox(subBitstr);
            // Ignore box if the handler type is not 'pict'
            if (trackBox->getMediaBox().getHandlerBox().getHandlerType() == "pict")
            {
                mTracks.push_back(move(trackBox));
            }
        }
        else
        {
            logWarning() << "Skipping an unsupported box '" << boxType << "' inside movie box." << std::endl;
        }
    }
}
