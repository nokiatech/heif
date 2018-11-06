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

#include "mediainformationbox.hpp"
#include "log.hpp"

using namespace std;

MediaInformationBox::MediaInformationBox()
    : Box("minf")
    , mMediaType(MediaType::Null)
    , mVideoMediaHeaderBox()
    , mSoundMediaHeaderBox()
    , mNullMediaHeaderBox()
    , mDataInformationBox()
    , mSampleTableBox()
{
}

void MediaInformationBox::setMediaType(MediaType type)
{
    mMediaType = type;
}

MediaInformationBox::MediaType MediaInformationBox::getMediaType() const
{
    return mMediaType;
}

const VideoMediaHeaderBox& MediaInformationBox::getVideoMediaHeaderBox() const
{
    return mVideoMediaHeaderBox;
}

VideoMediaHeaderBox& MediaInformationBox::getVideoMediaHeaderBox()
{
    return mVideoMediaHeaderBox;
}

const DataInformationBox& MediaInformationBox::getDataInformationBox() const
{
    return mDataInformationBox;
}

DataInformationBox& MediaInformationBox::getDataInformationBox()
{
    return mDataInformationBox;
}

const SampleTableBox& MediaInformationBox::getSampleTableBox() const
{
    return mSampleTableBox;
}

SampleTableBox& MediaInformationBox::getSampleTableBox()
{
    return mSampleTableBox;
}

const NullMediaHeaderBox& MediaInformationBox::getNullMediaHeaderBox() const
{
    return mNullMediaHeaderBox;
}

NullMediaHeaderBox& MediaInformationBox::getNullMediaHeaderBox()
{
    return mNullMediaHeaderBox;
}

const SoundMediaHeaderBox& MediaInformationBox::getSoundMediaHeaderBox() const
{
    return mSoundMediaHeaderBox;
}

SoundMediaHeaderBox& MediaInformationBox::getSoundMediaHeaderBox()
{
    return mSoundMediaHeaderBox;
}

void MediaInformationBox::writeBox(BitStream& bitstr) const
{
    // Write box headers
    writeBoxHeader(bitstr);

    // Write other boxes contained in the movie box
    // @todo should also support hmhd, sthd
    switch (mMediaType)
    {
    case MediaType::Null:
    {
        mNullMediaHeaderBox.writeBox(bitstr);
        break;
    }
    case MediaType::Video:
    {
        mVideoMediaHeaderBox.writeBox(bitstr);
        break;
    }
    case MediaType::Sound:
    {
        mSoundMediaHeaderBox.writeBox(bitstr);
        break;
    }
    }
    mDataInformationBox.writeBox(bitstr);
    mSampleTableBox.writeBox(bitstr);

    // Update the size of the movie box
    updateSize(bitstr);
}

void MediaInformationBox::parseBox(ISOBMFF::BitStream& bitstr)
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
            setMediaType(MediaType::Video);
        }
        else if (boxType == "smhd")
        {
            mSoundMediaHeaderBox.parseBox(subBitstr);
            setMediaType(MediaType::Sound);
        }
        else if (boxType == "nmhd")
        {
            mNullMediaHeaderBox.parseBox(subBitstr);
            setMediaType(MediaType::Null);
        }
        else if (boxType == "dinf")
        {
            mDataInformationBox.parseBox(subBitstr);
        }
        else if (boxType == "stbl")
        {
            mSampleTableBox.parseBox(subBitstr);
        }
        else
        {
            logWarning() << "Skipping an unsupported box '" << boxType.getString() << "' inside MediaInformationBox." << std::endl;
        }
    }
}
