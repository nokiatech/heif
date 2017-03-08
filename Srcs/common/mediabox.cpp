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

#include "mediabox.hpp"
#include "log.hpp"

using namespace std;

MediaBox::MediaBox() :
    Box("mdia"),
    mMediaHeaderBox(),
    mHandlerBox(),
    mMediaInformationBox()
{
}

MediaHeaderBox& MediaBox::getMediaHeaderBox()
{
    return mMediaHeaderBox;
}

HandlerBox& MediaBox::getHandlerBox()
{
    return mHandlerBox;
}

MediaInformationBox& MediaBox::getMediaInformationBox()
{
    return mMediaInformationBox;
}

void MediaBox::writeBox(BitStream& bitstr)
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

void MediaBox::parseBox(BitStream& bitstr)
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
    }
}
