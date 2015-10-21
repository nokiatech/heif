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

#include "trackheaderbox.hpp"
#include "log.hpp"

TrackHeaderBox::TrackHeaderBox() :
    FullBox("tkhd", 0, 0),
    mCreationTime(0),
    mModificationTime(0),
    mTrackID(0),
    mDuration(0),
    mWidth(0),
    mHeight(0),
    mAlternateGroup(0)
{
}

void TrackHeaderBox::setCreationTime(const std::uint32_t creation_time)
{
    mCreationTime = creation_time;
}

std::uint32_t TrackHeaderBox::getCreationTime() const
{
    return mCreationTime;
}

void TrackHeaderBox::setModificationTime(const std::uint32_t modification_time)
{
    mModificationTime = modification_time;
}

std::uint32_t TrackHeaderBox::getModificationTime() const
{
    return mModificationTime;
}

void TrackHeaderBox::setTrackID(const std::uint32_t track_ID)
{
    mTrackID = track_ID;
}

std::uint32_t TrackHeaderBox::getTrackID() const
{
    return mTrackID;
}

void TrackHeaderBox::setDuration(const std::uint32_t duration)
{
    mDuration = duration;
}

std::uint32_t TrackHeaderBox::getDuration() const
{
    return mDuration;
}

void TrackHeaderBox::setWidth(const std::uint32_t width)
{
    mWidth = width;
}

std::uint32_t TrackHeaderBox::getWidth() const
{
    return mWidth;
}

void TrackHeaderBox::setHeight(const std::uint32_t height)
{
    mHeight = height;
}

std::uint32_t TrackHeaderBox::getHeight() const
{
    return mHeight;
}

void TrackHeaderBox::setAlternateGroup(const std::uint16_t groupID)
{
    mAlternateGroup = groupID;
}

std::uint16_t TrackHeaderBox::getAlternateGroup() const
{
    return mAlternateGroup;
}


void TrackHeaderBox::writeBox(BitStream& bitstr)
{
    // Write box headers
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mCreationTime);
    bitstr.write32Bits(mModificationTime);
    bitstr.write32Bits(mTrackID);
    bitstr.write32Bits(0);
    bitstr.write32Bits(mDuration);

    bitstr.write32Bits(0);                  //Reserved
    bitstr.write32Bits(0);

    bitstr.write16Bits(0);                  //Layer
    bitstr.write16Bits(mAlternateGroup);    //Alternate Group
    bitstr.write16Bits(0);                  //Volume
    bitstr.write16Bits(0);                  //Reserved

    bitstr.write32Bits(0x00010000);         //Matrix[9]
    bitstr.write32Bits(0);
    bitstr.write32Bits(0);
    bitstr.write32Bits(0);
    bitstr.write32Bits(0x00010000);
    bitstr.write32Bits(0);
    bitstr.write32Bits(0);
    bitstr.write32Bits(0);
    bitstr.write32Bits(0x40000000);

    bitstr.write32Bits(mWidth);
    bitstr.write32Bits(mHeight);

    updateSize(bitstr);
}

void TrackHeaderBox::parseBox(BitStream& bitstr)
{
    //  First parse the box header
    parseFullBoxHeader(bitstr);

    mCreationTime = bitstr.read32Bits();
    mModificationTime = bitstr.read32Bits();
    mTrackID = bitstr.read32Bits();
    bitstr.read32Bits();
    mDuration = bitstr.read32Bits();

    bitstr.read32Bits();          //Reserved
    bitstr.read32Bits();

    bitstr.read16Bits();          //Layer
    mAlternateGroup = bitstr.read16Bits(); //Alternate Group
    bitstr.read16Bits();          //Volume
    bitstr.read16Bits();          //Reserved

    bitstr.read32Bits(); //Matrix[9]
    bitstr.read32Bits();
    bitstr.read32Bits();
    bitstr.read32Bits();
    bitstr.read32Bits();
    bitstr.read32Bits();
    bitstr.read32Bits();
    bitstr.read32Bits();
    bitstr.read32Bits();

    mWidth = bitstr.read32Bits();
    mHeight = bitstr.read32Bits();
}
