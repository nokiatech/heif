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

#include "mediaheaderbox.hpp"
#include "log.hpp"

using namespace std;

MediaHeaderBox::MediaHeaderBox() :
    FullBox("mdhd", 0, 0),
    mCreationTime(0),
    mModificationTime(0),
    mTimeScale(0),
    mDuration(0),
    mLanguage(0)
{
}

void MediaHeaderBox::setCreationTime(uint32_t creation_time)
{
    mCreationTime = creation_time;
}

uint32_t MediaHeaderBox::getCreationTime()
{
    return mCreationTime;
}

void MediaHeaderBox::setModificationTime(uint32_t modification_time)
{
    mModificationTime = modification_time;
}
uint32_t MediaHeaderBox::getModificationTime()
{
    return mModificationTime;
}

void MediaHeaderBox::setTimeScale(uint32_t time_scale)
{
    mTimeScale = time_scale;
}
uint32_t MediaHeaderBox::getTimeScale()
{
    return mTimeScale;
}

void MediaHeaderBox::setDuration(uint32_t duration)
{
    mDuration = duration;
}
uint32_t MediaHeaderBox::getDuration()
{
    return mDuration;
}

void MediaHeaderBox::setLanguage(uint16_t language)
{
    mLanguage = language;
}
uint32_t MediaHeaderBox::getLanguage()
{
    return mLanguage;
}

void MediaHeaderBox::writeBox(BitStream& bitstr)
{
    // Write box headers
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mCreationTime);
    bitstr.write32Bits(mModificationTime);
    bitstr.write32Bits(mTimeScale);
    bitstr.write32Bits(mDuration);

    bitstr.write16Bits(0);    //Pad, Langauge
    bitstr.write16Bits(0);    //Predefined

    // Update the size of the movie box
    updateSize(bitstr);
}

void MediaHeaderBox::parseBox(BitStream& bitstr)
{
    BitStream subBitstr;

    //  First parse the box header
    parseFullBoxHeader(bitstr);

    mCreationTime = bitstr.read32Bits();
    mModificationTime = bitstr.read32Bits();
    mTimeScale = bitstr.read32Bits();
    mDuration = bitstr.read32Bits();

    bitstr.read16Bits();    //Pad, Langauge
    bitstr.read16Bits();    //Predefined

}
