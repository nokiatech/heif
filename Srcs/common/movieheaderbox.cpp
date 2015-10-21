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

#include "movieheaderbox.hpp"
#include <stdexcept>

using namespace std;

static const int MATRIX_LENGTH = 9;

MovieHeaderBox::MovieHeaderBox() :
    FullBox("mvhd", 0, 0),
    mCreationTime(0),
    mModificationTime(0),
    mTimeScale(0),
    mDuration(0),
    mMatrix( { 0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000 } ),
    mNextTrackID(0)
{
}

void MovieHeaderBox::setCreationTime(const uint32_t creationTime)
{
    mCreationTime = creationTime;
}

uint32_t MovieHeaderBox::getCreationTime() const
{
    return mCreationTime;
}

void MovieHeaderBox::setModificationTime(uint32_t modificationTime)
{
    mModificationTime = modificationTime;
}

uint32_t MovieHeaderBox::getModificationTime() const
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

void MovieHeaderBox::setDuration(uint32_t duration)
{
    mDuration = duration;
}

uint32_t MovieHeaderBox::getDuration() const
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

std::vector<int32_t> MovieHeaderBox::getMatrix() const
{
    return mMatrix;
}

void MovieHeaderBox::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mCreationTime);
    bitstr.write32Bits(mModificationTime);
    bitstr.write32Bits(mTimeScale);
    bitstr.write32Bits(mDuration);
    bitstr.write32Bits(0x00010000); //Rate
    bitstr.write16Bits(0x0100);     //Volume
    bitstr.write16Bits(0);          //Reserved

    bitstr.write32Bits(0);          //Reserved
    bitstr.write32Bits(0);

    for (int i = 0; i < MATRIX_LENGTH; ++i)
    {
        bitstr.write32Bits(mMatrix.at(i)); //Matrix[9]
    }

    bitstr.write32Bits(0);          //Predefined[6]
    bitstr.write32Bits(0);
    bitstr.write32Bits(0);
    bitstr.write32Bits(0);
    bitstr.write32Bits(0);
    bitstr.write32Bits(0);

    bitstr.write32Bits(mNextTrackID);

    updateSize(bitstr);
}

void MovieHeaderBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    if (getVersion() != 0)
    {
        throw std::runtime_error("MovieHeaderBox::parseBox() supports only 'mvhd' version 0");
    }

    mCreationTime = bitstr.read32Bits();
    mModificationTime = bitstr.read32Bits();
    mTimeScale = bitstr.read32Bits();
    mDuration = bitstr.read32Bits();
    bitstr.read32Bits();  //Rate
    bitstr.read16Bits();  //Volume
    bitstr.read16Bits();  //Reserved

    bitstr.read32Bits();  //Reserved
    bitstr.read32Bits();

    mMatrix.clear();
    for (int i = 0; i < MATRIX_LENGTH; ++i)
    {
        mMatrix.push_back(bitstr.read32Bits()); //Matrix[9]
    }

    bitstr.read32Bits();  //Predefined[6]
    bitstr.read32Bits();
    bitstr.read32Bits();
    bitstr.read32Bits();
    bitstr.read32Bits();
    bitstr.read32Bits();

    mNextTrackID = bitstr.read32Bits();
}

