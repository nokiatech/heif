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

#include "trackheaderbox.hpp"
#include "log.hpp"

TrackHeaderBox::TrackHeaderBox()
    : FullBox("tkhd", 0, 0)
    , mCreationTime(0)
    , mModificationTime(0)
    , mTrackID(0)
    , mDuration(0)
    , mWidth(0)
    , mHeight(0)
    , mAlternateGroup(0)
    , mVolume(0)
    , mMatrix({0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000})
{
}

void TrackHeaderBox::setCreationTime(const std::uint64_t creation_time)
{
    mCreationTime = creation_time;
}

std::uint64_t TrackHeaderBox::getCreationTime() const
{
    return mCreationTime;
}

void TrackHeaderBox::setModificationTime(const std::uint64_t modification_time)
{
    mModificationTime = modification_time;
}

std::uint64_t TrackHeaderBox::getModificationTime() const
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

void TrackHeaderBox::setDuration(const std::uint64_t duration)
{
    mDuration = duration;
}

std::uint64_t TrackHeaderBox::getDuration() const
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

void TrackHeaderBox::setVolume(const std::uint16_t volume)
{
    mVolume = volume;
}

std::uint16_t TrackHeaderBox::getVolume() const
{
    return mVolume;
}

void TrackHeaderBox::setMatrix(const Vector<int32_t>& matrix)
{
    mMatrix = matrix;
}

Vector<int32_t> TrackHeaderBox::getMatrix() const
{
    return mMatrix;
}

void TrackHeaderBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Write box headers
    writeFullBoxHeader(bitstr);

    if (getVersion() == 0)
    {
        bitstr.write32Bits(static_cast<uint32_t>(mCreationTime));
        bitstr.write32Bits(static_cast<uint32_t>(mModificationTime));
        bitstr.write32Bits(mTrackID);
        bitstr.write32Bits(0);
        bitstr.write32Bits(static_cast<uint32_t>(mDuration));
    }
    else if (getVersion() == 1)
    {
        bitstr.write64Bits(mCreationTime);
        bitstr.write64Bits(mModificationTime);
        bitstr.write32Bits(mTrackID);
        bitstr.write32Bits(0);
        bitstr.write64Bits(mDuration);
    }
    else
    {
        throw RuntimeError("TrackHeaderBox::writeBox() supports only 'tkhd' version 0 and version 1");
    }

    bitstr.write32Bits(0);  // Reserved
    bitstr.write32Bits(0);

    bitstr.write16Bits(0);                // Layer
    bitstr.write16Bits(mAlternateGroup);  // Alternate Group
    bitstr.write16Bits(mVolume);          // Volume
    bitstr.write16Bits(0);                // Reserved

    for (auto value : mMatrix)
    {
        bitstr.write32Bits(static_cast<uint32_t>(value));
    }

    bitstr.write32Bits(mWidth);
    bitstr.write32Bits(mHeight);

    updateSize(bitstr);
}

void TrackHeaderBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    //  First parse the box header
    parseFullBoxHeader(bitstr);
    if ((getVersion() != 0) && (getVersion() != 1))
    {
        throw RuntimeError("TrackHeaderBox::parseBox() supports only 'tkhd' version 0 and version 1");
    }

    if (getVersion() == 0)
    {
        mCreationTime     = bitstr.read32Bits();
        mModificationTime = bitstr.read32Bits();
    }
    else
    {
        mCreationTime     = bitstr.read64Bits();
        mModificationTime = bitstr.read64Bits();
    }
    mTrackID = bitstr.read32Bits();
    bitstr.read32Bits();
    if (getVersion() == 0)
    {
        mDuration = bitstr.read32Bits();
    }
    else
    {
        mDuration = bitstr.read64Bits();
    }

    bitstr.read32Bits();  // Reserved
    bitstr.read32Bits();

    bitstr.read16Bits();                    // Layer
    mAlternateGroup = bitstr.read16Bits();  // Alternate Group
    mVolume         = bitstr.read16Bits();  // Volume
    bitstr.read16Bits();                    // Reserved

    mMatrix.clear();
    for (int n = 9; n > 0; n--)  // Matrix[9]
    {
        mMatrix.push_back(static_cast<int32_t>(bitstr.read32Bits()));
    }

    mWidth  = bitstr.read32Bits();
    mHeight = bitstr.read32Bits();
}
