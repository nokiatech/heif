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

#include "mediabox.hpp"
#include "log.hpp"

using namespace std;

MediaBox::MediaBox()
    : Box("mdia")
    , mMediaHeaderBox()
    , mHandlerBox()
    , mMediaInformationBox()
{
}

const MediaHeaderBox& MediaBox::getMediaHeaderBox() const
{
    return mMediaHeaderBox;
}

MediaHeaderBox& MediaBox::getMediaHeaderBox()
{
    return mMediaHeaderBox;
}

const HandlerBox& MediaBox::getHandlerBox() const
{
    return mHandlerBox;
}

HandlerBox& MediaBox::getHandlerBox()
{
    return mHandlerBox;
}

const MediaInformationBox& MediaBox::getMediaInformationBox() const
{
    return mMediaInformationBox;
}

MediaInformationBox& MediaBox::getMediaInformationBox()
{
    return mMediaInformationBox;
}

void MediaBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Write box headers
    writeBoxHeader(bitstr);

    // Write other boxes contained in the movie box
    mMediaHeaderBox.writeBox(bitstr);
    mHandlerBox.writeBox(bitstr);
    mMediaInformationBox.writeBox(bitstr);

    // Update the size of the movie box
    updateSize(bitstr);
}

void MediaBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    //  First parse the box header
    parseBoxHeader(bitstr);

    // if there a data available in the file
    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        if (boxType == "mdhd")
        {
            mMediaHeaderBox.parseBox(subBitstr);
        }
        else if (boxType == "hdlr")
        {
            mHandlerBox.parseBox(subBitstr);
        }
        else if (boxType == "minf")
        {
            mMediaInformationBox.parseBox(subBitstr);
        }
        else
        {
            logWarning() << "Skipping an unsupported box '" << boxType.getString() << "' inside MediaBox." << std::endl;
        }
    }
}
