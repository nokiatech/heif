/* Copyright (c) 2015, Nokia Technologies Ltd.
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

#include "trackbox.hpp"
#include "log.hpp"

using namespace std;

TrackBox::TrackBox() :
    Box("trak"),
    mTrackHeaderBox(),
    mMediaBox(),
    mTrackReferenceBox(),
    mEditBox(nullptr),
    mHasTrackReferences(false)
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


TrackHeaderBox& TrackBox::getTrackHeaderBox()
{
    return mTrackHeaderBox;
}


MediaBox& TrackBox::getMediaBox()
{
    return mMediaBox;
}

TrackReferenceBox& TrackBox::getTrackReferenceBox()
{
    return mTrackReferenceBox;
}

void TrackBox::setEditBox(const EditBox& editBox)
{
    if (mEditBox == nullptr)
    {
        mEditBox = std::make_shared<EditBox>(editBox);
    }
    else
    {
        *mEditBox = editBox;
    }
}

std::shared_ptr<const EditBox> TrackBox::getEditBox() const
{
    return mEditBox;
}

void TrackBox::writeBox(BitStream& bitstr)
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

    // Update the size of the movie box
    updateSize(bitstr);
}

void TrackBox::parseBox(BitStream& bitstr)
{
    //  First parse the box header
    parseBoxHeader(bitstr);

    // if there a data available in the file
    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        std::string boxType;
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
            /** @todo Implement this when reading meta box in tracks is supported. **/
        }
        else if (boxType == "tref")
        {
            mTrackReferenceBox.parseBox(subBitstr);
            mHasTrackReferences = true;
        }
        else if (boxType == "edts")
        {
            mEditBox = std::make_shared<EditBox>();
            mEditBox->parseBox(subBitstr);
        }
    }
}
