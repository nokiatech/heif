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

#include "syncsamplebox.hpp"

SyncSampleBox::SyncSampleBox()
    : FullBox("stss", 0, 0)
    , mSampleNumber()
    , mSampleCountMax(-1)
{
}

void SyncSampleBox::addSample(std::uint32_t sampleNumber)
{
    mSampleNumber.push_back(sampleNumber);
}

const Vector<std::uint32_t>& SyncSampleBox::getSyncSampleIds() const
{
    return mSampleNumber;
}

void SyncSampleBox::setSampleCountMaxSafety(int64_t sampleCountMax)
{
    mSampleCountMax = sampleCountMax;
}

void SyncSampleBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(static_cast<unsigned int>(mSampleNumber.size()));

    for (auto sampleNumber : mSampleNumber)
    {
        bitstr.write32Bits(sampleNumber);
    }

    // Update the size
    updateSize(bitstr);
}

void SyncSampleBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    std::uint32_t entryCount = bitstr.read32Bits();

    if (mSampleCountMax != -1 && (entryCount > mSampleCountMax))
    {
        throw RuntimeError("SyncSampleBox::parseBox entryCount is larger than total number of samples");
    }

    for (std::uint32_t i = 0; i < entryCount; ++i)
    {
        mSampleNumber.push_back(bitstr.read32Bits());
    }
}
