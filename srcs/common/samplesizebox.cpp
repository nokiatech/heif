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

#include "samplesizebox.hpp"
#include "log.hpp"

using namespace std;

SampleSizeBox::SampleSizeBox()
    : FullBox("stsz", 0, 0)
    , mSampleSize(0)
    , mSampleCount(0)
    , mEntrySize()
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

uint32_t SampleSizeBox::getSampleCount() const
{
    return mSampleCount;
}

void SampleSizeBox::setEntrySize(Vector<uint32_t>& sample_sizes)
{
    mEntrySize = sample_sizes;
}

const Vector<uint32_t>& SampleSizeBox::getEntrySize() const
{
    // Fill the entry size in a lazy fashion to avoid doing too much work
    // if the box ends up being discarded due to invalid data determined
    // from othere sources
    if (mEntrySize.size() == 0 && mSampleSize != 0)
    {
        for (uint32_t i = 0; i < mSampleCount; i++)
        {
            mEntrySize.push_back(mSampleSize);
        }
    }
    return mEntrySize;
}

void SampleSizeBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Write box headers
    writeFullBoxHeader(bitstr);
    bitstr.write32Bits(mSampleSize);
    bitstr.write32Bits(mSampleCount);  // number of samples in the track
    for (uint32_t i = 0; i < mSampleCount; i++)
    {
        bitstr.write32Bits(mEntrySize.at(i));
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void SampleSizeBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    //  First parse the box header
    parseFullBoxHeader(bitstr);

    mSampleSize  = bitstr.read32Bits();
    mSampleCount = bitstr.read32Bits();

    if (mSampleSize == 0)
    {
        for (uint32_t i = 0; i < mSampleCount; i++)
        {
            mEntrySize.push_back(bitstr.read32Bits());
        }
    }
}
