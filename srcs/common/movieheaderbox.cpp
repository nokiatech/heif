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

#include "movieheaderbox.hpp"
#include <stdexcept>

using namespace std;

static const int MATRIX_LENGTH = 9;

MovieHeaderBox::MovieHeaderBox()
    : FullBox("mvhd", 0, 0)
    , mCreationTime(0)
    , mModificationTime(0)
    , mTimeScale(0)
    , mDuration(0)
    , mMatrix({0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000})
    , mNextTrackID(0)
{
}

void MovieHeaderBox::setCreationTime(const uint64_t creationTime)
{
    mCreationTime = creationTime;
}

uint64_t MovieHeaderBox::getCreationTime() const
{
    return mCreationTime;
}

void MovieHeaderBox::setModificationTime(uint64_t modificationTime)
{
    mModificationTime = modificationTime;
}

uint64_t MovieHeaderBox::getModificationTime() const
{
    return mModificationTime;
}

void MovieHeaderBox::setTimeScale(uint32_t timeScale)
{
    mTimeScale = timeScale;
}

uint32_t MovieHeaderBox::getTimeScale() const
{
    return mTimeScale;
}

void MovieHeaderBox::setDuration(uint64_t duration)
{
    mDuration = duration;
}

uint64_t MovieHeaderBox::getDuration() const
{
    return mDuration;
}

void MovieHeaderBox::setNextTrackID(uint32_t nextTrackId)
{
    mNextTrackID = nextTrackId;
}

uint32_t MovieHeaderBox::getNextTrackID() const
{
    return mNextTrackID;
}

void MovieHeaderBox::setMatrix(const Vector<int32_t>& matrix)
{
    mMatrix = matrix;
}

const Vector<int32_t>& MovieHeaderBox::getMatrix() const
{
    return mMatrix;
}

void MovieHeaderBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    if (getVersion() == 0)
    {
        bitstr.write32Bits(static_cast<uint32_t>(mCreationTime));
        bitstr.write32Bits(static_cast<uint32_t>(mModificationTime));
        bitstr.write32Bits(mTimeScale);
        bitstr.write32Bits(static_cast<uint32_t>(mDuration));
    }
    else if (getVersion() == 1)
    {
        bitstr.write64Bits(mCreationTime);
        bitstr.write64Bits(mModificationTime);
        bitstr.write32Bits(mTimeScale);
        bitstr.write64Bits(mDuration);
    }
    else
    {
        throw RuntimeError("MovieHeaderBox::writeBox() supports only 'mvhd' version 0 and version 1");
    }
    bitstr.write32Bits(0x00010000);  // Rate
    bitstr.write16Bits(0x0100);      // Volume
    bitstr.write16Bits(0);           // Reserved

    bitstr.write32Bits(0);  // Reserved
    bitstr.write32Bits(0);

    for (unsigned int i = 0; i < MATRIX_LENGTH; ++i)
    {
        bitstr.write32Bits(static_cast<std::uint32_t>(mMatrix.at(i)));  // Matrix[9]
    }

    bitstr.write32Bits(0);  // Predefined[6]
    bitstr.write32Bits(0);
    bitstr.write32Bits(0);
    bitstr.write32Bits(0);
    bitstr.write32Bits(0);
    bitstr.write32Bits(0);

    bitstr.write32Bits(mNextTrackID);

    updateSize(bitstr);
}

void MovieHeaderBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    if ((getVersion() != 0) && (getVersion() != 1))
    {
        throw RuntimeError("MovieHeaderBox::parseBox() supports only 'mvhd' version 0 and version 1");
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
    mTimeScale = bitstr.read32Bits();
    if (getVersion() == 0)
    {
        mDuration = bitstr.read32Bits();
    }
    else
    {
        mDuration = bitstr.read64Bits();
    }
    bitstr.read32Bits();  // Rate
    bitstr.read16Bits();  // Volume
    bitstr.read16Bits();  // Reserved

    bitstr.read32Bits();  // Reserved
    bitstr.read32Bits();

    mMatrix.clear();
    for (int i = 0; i < MATRIX_LENGTH; ++i)
    {
        mMatrix.push_back(static_cast<std::int32_t>(bitstr.read32Bits()));  // Matrix[9]
    }

    bitstr.read32Bits();  // Predefined[6]
    bitstr.read32Bits();
    bitstr.read32Bits();
    bitstr.read32Bits();
    bitstr.read32Bits();
    bitstr.read32Bits();

    mNextTrackID = bitstr.read32Bits();
}
