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

#include "timetosamplebox.hpp"
#include "log.hpp"

#include <limits>
#include <stdexcept>

using namespace std;

TimeToSampleBox::TimeToSampleBox()
    : FullBox("stts", 0, 0)
{
}

Vector<std::uint32_t> TimeToSampleBox::getSampleTimes() const
{
    Vector<std::uint32_t> sampleTimes;
    uint32_t time = 0;
    for (const auto& entry : mEntryVersion0)
    {
        for (unsigned int i = 0; i < entry.mSampleCount; ++i)
        {
            sampleTimes.push_back(time);
            time += entry.mSampleDelta;
        }
    }

    return sampleTimes;
}

Vector<std::uint32_t> TimeToSampleBox::getSampleDeltas() const
{
    Vector<std::uint32_t> sampleDeltas;

    if (mEntryVersion0.size())
    {
        sampleDeltas.reserve(mEntryVersion0.at(0).mSampleCount);
        for (const auto& entry : mEntryVersion0)
        {
            for (unsigned int i = 0; i < entry.mSampleCount; ++i)
            {
                sampleDeltas.push_back(entry.mSampleDelta);
            }
        }
    }

    return sampleDeltas;
}

std::uint32_t TimeToSampleBox::getSampleCount() const
{
    std::uint64_t sampleCount = 0;

    if (mEntryVersion0.size())
    {
        for (const auto& entry : mEntryVersion0)
        {
            sampleCount += entry.mSampleCount;
            if (sampleCount > std::numeric_limits<std::uint32_t>::max())
            {
                throw RuntimeError("TimeToSampleBox::sampleCount >= 2^32");
            }
        }
    }

    return std::uint32_t(sampleCount);
}

TimeToSampleBox::EntryVersion0& TimeToSampleBox::getDecodeDeltaEntry()
{
    mEntryVersion0.resize(mEntryVersion0.size() + 1);
    return mEntryVersion0.back();
}

void TimeToSampleBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Write box headers
    writeFullBoxHeader(bitstr);
    bitstr.write32Bits(static_cast<unsigned int>(mEntryVersion0.size()));
    for (auto entry : mEntryVersion0)
    {
        bitstr.write32Bits(entry.mSampleCount);
        bitstr.write32Bits(entry.mSampleDelta);
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void TimeToSampleBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    //  First parse the box header
    parseFullBoxHeader(bitstr);

    std::uint32_t entryCount = bitstr.read32Bits();
    for (uint32_t i = 0; i < entryCount; ++i)
    {
        EntryVersion0 entryVersion0;
        entryVersion0.mSampleCount = bitstr.read32Bits();
        entryVersion0.mSampleDelta = bitstr.read32Bits();
        mEntryVersion0.push_back(entryVersion0);
    }
}

void TimeToSampleBox::addSampleDelta(std::uint32_t mSampleDelta)
{
    if (!mEntryVersion0.size() || mSampleDelta != mEntryVersion0.back().mSampleDelta)
    {
        mEntryVersion0.push_back({1, mSampleDelta});
    }
    else
    {
        ++mEntryVersion0.back().mSampleCount;
    }
}
