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

#include "mp4audiosampleentrybox.hpp"
#include <string>

MP4AudioSampleEntryBox::MP4AudioSampleEntryBox()
    : AudioSampleEntryBox("mp4a")
    , mESDBox()
    , mRecord(mESDBox)
{
}

ElementaryStreamDescriptorBox& MP4AudioSampleEntryBox::getESDBox()
{
    return mESDBox;
}

const ElementaryStreamDescriptorBox& MP4AudioSampleEntryBox::getESDBox() const
{
    return mESDBox;
}

void MP4AudioSampleEntryBox::writeBox(BitStream& bitstr) const
{
    AudioSampleEntryBox::writeBox(bitstr);
    mESDBox.writeBox(bitstr);

    updateSize(bitstr);
}

void MP4AudioSampleEntryBox::parseBox(BitStream& bitstr)
{
    AudioSampleEntryBox::parseBox(bitstr);

    while (bitstr.numBytesLeft() > 0)
    {
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        if (boxType == "esds")
        {
            mESDBox.parseBox(subBitstr);
        }
    }
}

MP4AudioSampleEntryBox* MP4AudioSampleEntryBox::clone() const
{
    return CUSTOM_NEW(MP4AudioSampleEntryBox, (*this));
}

const Box* MP4AudioSampleEntryBox::getConfigurationBox() const
{
    return &mESDBox;
}

const DecoderConfigurationRecord* MP4AudioSampleEntryBox::getConfigurationRecord() const
{
    return &mRecord;
}
