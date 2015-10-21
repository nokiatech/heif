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

#include "timetosamplebox.hpp"
#include "log.hpp"

#include <stdexcept>

using namespace std;

TimeToSampleBox::TimeToSampleBox() :
    FullBox("stts", 0, 0)
{
}

std::vector<std::uint32_t> TimeToSampleBox::getSampleTimes() const
{
    std::vector<std::uint32_t> sampleTimes;
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

TimeToSampleBox::EntryVersion0& TimeToSampleBox::getDecodeDeltaEntry()
{
    mEntryVersion0.resize(mEntryVersion0.size() + 1);
    return mEntryVersion0.back();
}

void TimeToSampleBox::writeBox(BitStream& bitstr)
{
    // Write box headers
    writeFullBoxHeader(bitstr);
    bitstr.write32Bits(mEntryVersion0.size());
    for(auto entry : mEntryVersion0)
    {
        bitstr.write32Bits(entry.mSampleCount);
        bitstr.write32Bits(entry.mSampleDelta);
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void TimeToSampleBox::parseBox(BitStream& bitstr)
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
