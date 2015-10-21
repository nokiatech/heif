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

#include "samplesizebox.hpp"
#include "log.hpp"

using namespace std;

SampleSizeBox::SampleSizeBox() :
    FullBox("stsz", 0, 0),
    mSampleSize(0),
    mSampleCount(0),
    mEntrySize()
{
}

void SampleSizeBox::setSampleSize(uint32_t sample_size)
{
    mSampleSize = sample_size;
}

uint32_t SampleSizeBox::getSampleSize()
{
    return mSampleSize;
}

void SampleSizeBox::setSampleCount(uint32_t sample_count)
{
    mSampleCount = sample_count;
}

uint32_t SampleSizeBox::getSampleCount()
{
    return mSampleCount;
}

void SampleSizeBox::setEntrySize(vector<uint32_t> sample_sizes)
{
    mEntrySize = sample_sizes;
}

vector<uint32_t> SampleSizeBox::getEntrySize()
{
    return mEntrySize;
}

void SampleSizeBox::writeBox(BitStream& bitstr)
{
    // Write box headers
    writeFullBoxHeader(bitstr);
    bitstr.write32Bits(mSampleSize);
    bitstr.write32Bits(mSampleCount); // number of samples in the track
    for (uint32_t i = 0; i < mSampleCount; i++)
    {
        bitstr.write32Bits(mEntrySize.at(i));
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void SampleSizeBox::parseBox(BitStream& bitstr)
{
    BitStream subBitstr;

    //  First parse the box header
    parseFullBoxHeader(bitstr);

    mSampleSize = bitstr.read32Bits();
    mSampleCount = bitstr.read32Bits();
    for (uint32_t i = 0; i < mSampleCount; i++)
    {
        mEntrySize.push_back(bitstr.read32Bits());
    }
}
