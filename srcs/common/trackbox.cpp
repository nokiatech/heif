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

#include "trackbox.hpp"

#include "log.hpp"

using namespace std;

TrackBox::TrackBox()
    : Box("trak")
    , mTrackHeaderBox()
    , mMediaBox()
    , mTrackReferenceBox()
    , mHasTrackReferences(false)
    , mTrackGroupBox()
    , mHasTrackGroupBox(false)
    , mTrackTypeBox()
    , mHasTrackTypeBox(false)
    , mEditBox(nullptr)
{
}

void TrackBox::setHasTrackReferences(bool value)
{
    mHasTrackReferences = value;
}

bool TrackBox::getHasTrackReferences() const
{
    return mHasTrackReferences;
}

const TrackHeaderBox& TrackBox::getTrackHeaderBox() const
{
    return mTrackHeaderBox;
}

TrackHeaderBox& TrackBox::getTrackHeaderBox()
{
    return mTrackHeaderBox;
}

const MediaBox& TrackBox::getMediaBox() const
{
    return mMediaBox;
}

MediaBox& TrackBox::getMediaBox()
{
    return mMediaBox;
}

const TrackReferenceBox& TrackBox::getTrackReferenceBox() const
{
    return mTrackReferenceBox;
}

const TrackGroupBox& TrackBox::getTrackGroupBox() const
{
    return mTrackGroupBox;
}

TrackGroupBox& TrackBox::getTrackGroupBox()
{
    return mTrackGroupBox;
}

void TrackBox::setHasTrackGroup(bool value)
{
    mHasTrackGroupBox = value;
}

bool TrackBox::getHasTrackGroup() const
{
    return mHasTrackGroupBox;
}


std::shared_ptr<const EditBox> TrackBox::getEditBox() const
{
    return mEditBox;
}

const TrackTypeBox& TrackBox::getTrackTypeBox() const
{
    return mTrackTypeBox;
}

TrackTypeBox& TrackBox::getTrackTypeBox()
{
    return mTrackTypeBox;
}

void TrackBox::setHasTrackTypeBox(bool value)
{
    mHasTrackTypeBox = value;
}

bool TrackBox::getHasTrackTypeBox() const
{
    return mHasTrackTypeBox;
}

void TrackBox::setEditBox(const EditBox& editBox)
{
    if (mEditBox == nullptr)
    {
        mEditBox = makeCustomShared<EditBox>(editBox);
    }
    else
    {
        *mEditBox = editBox;
    }
}

TrackReferenceBox& TrackBox::getTrackReferenceBox()
{
    return mTrackReferenceBox;
}

void TrackBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Write box headers
    writeBoxHeader(bitstr);

    // Write other boxes contained in the movie box
    // The TrackHeaderBox
    mTrackHeaderBox.writeBox(bitstr);

    if (mHasTrackReferences == true)
    {
        mTrackReferenceBox.writeBox(bitstr);
    }

    if (mEditBox != nullptr)
    {
        mEditBox->writeBox(bitstr);
    }

    // The MediaBox
    mMediaBox.writeBox(bitstr);

    if (mHasTrackGroupBox)
    {
        mTrackGroupBox.writeBox(bitstr);
    }

    if (mHasTrackTypeBox)
    {
        mTrackTypeBox.writeBox(bitstr);
    }

    updateSize(bitstr);
}

void TrackBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    //  First parse the box header
    parseBoxHeader(bitstr);

    // if there a data available in the file
    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        if (boxType == "tkhd")
        {
            mTrackHeaderBox.parseBox(subBitstr);
        }
        else if (boxType == "mdia")
        {
            mMediaBox.parseBox(subBitstr);
        }
        else if (boxType == "meta")
        {
            /** @todo Implement this when reading meta box in tracks is supported. */
        }
        else if (boxType == "tref")
        {
            mTrackReferenceBox.parseBox(subBitstr);
            mHasTrackReferences = true;
        }
        else if (boxType == "trgr")
        {
            mHasTrackGroupBox = true;
            mTrackGroupBox.parseBox(subBitstr);
        }
        else if (boxType == "ttyp")
        {
            mHasTrackTypeBox = true;
            mTrackTypeBox.parseBox(subBitstr);
        }
        else if (boxType == "edts")
        {
            mEditBox = makeCustomShared<EditBox>();
            mEditBox->parseBox(subBitstr);
        }
        else
        {
            logWarning() << "Skipping an unsupported box '" << boxType.getString() << "' inside TrackBox." << std::endl;
        }
    }
}
