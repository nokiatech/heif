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

#include "mediaheaderbox.hpp"
#include "log.hpp"

using namespace std;

MediaHeaderBox::MediaHeaderBox()
    : FullBox("mdhd", 0, 0)
    , mCreationTime(0)
    , mModificationTime(0)
    , mTimeScale(0)
    , mDuration(0)
    , mLanguage(0)
{
}

void MediaHeaderBox::setCreationTime(uint64_t creation_time)
{
    mCreationTime = creation_time;
}

uint64_t MediaHeaderBox::getCreationTime()
{
    return mCreationTime;
}

void MediaHeaderBox::setModificationTime(uint64_t modification_time)
{
    mModificationTime = modification_time;
}
uint64_t MediaHeaderBox::getModificationTime()
{
    return mModificationTime;
}

void MediaHeaderBox::setTimeScale(uint32_t time_scale)
{
    mTimeScale = time_scale;
}
uint32_t MediaHeaderBox::getTimeScale() const
{
    return mTimeScale;
}

void MediaHeaderBox::setDuration(uint64_t duration)
{
    mDuration = duration;
}
uint64_t MediaHeaderBox::getDuration() const
{
    return mDuration;
}

void MediaHeaderBox::setLanguage(uint16_t language)
{
    mLanguage = language;
}
uint32_t MediaHeaderBox::getLanguage() const
{
    return mLanguage;
}

void MediaHeaderBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Write box headers
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
        throw RuntimeError("MediaHeaderBox::writeBox() supports only 'mdhd' version 0 and version 1");
    }

    bitstr.write16Bits(0);  // Pad, Langauge
    bitstr.write16Bits(0);  // Predefined

    // Update the size of the movie box
    updateSize(bitstr);
}

void MediaHeaderBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    //  First parse the box header
    parseFullBoxHeader(bitstr);
    if ((getVersion() != 0) && (getVersion() != 1))
    {
        throw RuntimeError("MediaHeaderBox::parseBox() supports only 'mdhd' version 0 and version 1");
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

    bitstr.read16Bits();  // Pad, Langauge
    bitstr.read16Bits();  // Predefined
}
