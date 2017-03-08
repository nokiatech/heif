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

#include "mediainformationbox.hpp"
#include "log.hpp"

using namespace std;

MediaInformationBox::MediaInformationBox() :
    Box("minf"),
    mVideoMediaHeaderBox(),
    mDataInformationBox(),
    mSampleTableBox()
{
}

VideoMediaHeaderBox& MediaInformationBox::getVideoMediaHeaderBox()
{
    return mVideoMediaHeaderBox;
}

DataInformationBox& MediaInformationBox::getDataInformationBox()
{
    return mDataInformationBox;
}

SampleTableBox& MediaInformationBox::getSampleTableBox()
{
    return mSampleTableBox;
}

void MediaInformationBox::writeBox(BitStream& bitstr)
{
    // Write box headers
    writeBoxHeader(bitstr);

    // Write other boxes contained in the movie box
    mVideoMediaHeaderBox.writeBox(bitstr);
    mDataInformationBox.writeBox(bitstr);
    mSampleTableBox.writeBox(bitstr);

    // Update the size of the movie box
    updateSize(bitstr);
}

void MediaInformationBox::parseBox(BitStream& bitstr)
{
    //  First parse the box header
    parseBoxHeader(bitstr);

    // if there a data available in the file
    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        // Handle this box based on the type
        if (boxType == "vmhd")
        {
            mVideoMediaHeaderBox.parseBox(subBitstr);
        }
        else if (boxType == "dinf")
        {
            mDataInformationBox.parseBox(subBitstr);
        }
        else if (boxType == "stbl")
        {
            mSampleTableBox.parseBox(subBitstr);
        }
    }
}
