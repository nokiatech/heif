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

#include "compositionoffsetbox.hpp"
#include <limits>
#include "bitstream.hpp"

#include <stdexcept>

CompositionOffsetBox::CompositionOffsetBox()
    : FullBox("ctts", 0, 0)
{
}

void CompositionOffsetBox::addCompositionOffsetEntryVersion0(const EntryVersion0& entry)
{
    if (mEntryVersion1.size() != 0 || getVersion() != 0)
    {
        throw RuntimeError("Invalid attempt to add version0 CompositionOffsetBox entries.");
    }
    mEntryVersion0.push_back(entry);
}

void CompositionOffsetBox::addCompositionOffsetEntryVersion1(const EntryVersion1& entry)
{
    if (mEntryVersion0.size() != 0 || getVersion() != 1)
    {
        throw RuntimeError("Invalid attempt to add version1 CompositionOffsetBox entries.");
    }
    mEntryVersion1.push_back(entry);
}

uint32_t CompositionOffsetBox::getSampleCount()
{
    uint64_t sampleCount = 0;
    if (getVersion() == 0)
    {
        for (const auto& entry : mEntryVersion0)
        {
            sampleCount += static_cast<uint64_t>(entry.mSampleCount);
            if (sampleCount > std::numeric_limits<std::uint32_t>::max())
            {
                throw RuntimeError("CompositionOffsetBox::getSampleCount >= 2^32");
            }
        }
    }
    else if (getVersion() == 1)
    {
        for (const auto& entry : mEntryVersion1)
        {
            sampleCount += static_cast<uint64_t>(entry.mSampleCount);
            if (sampleCount > std::numeric_limits<std::uint32_t>::max())
            {
                throw RuntimeError("CompositionOffsetBox::getSampleCount >= 2^32");
            }
        }
    }
    return static_cast<uint32_t>(sampleCount);
}

Vector<int> CompositionOffsetBox::getSampleCompositionOffsets() const
{
    Vector<int> offsets;
    if (getVersion() == 0)
    {
        for (const auto& entry : mEntryVersion0)
        {
            for (unsigned int i = 0; i < entry.mSampleCount; ++i)
            {
                offsets.push_back(static_cast<int>(entry.mSampleOffset));
            }
        }
    }
    else if (getVersion() == 1)
    {
        for (const auto& entry : mEntryVersion1)
        {
            for (unsigned int i = 0; i < entry.mSampleCount; ++i)
            {
                offsets.push_back(entry.mSampleOffset);
            }
        }
    }

    return offsets;
}

void CompositionOffsetBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Write box headers
    writeFullBoxHeader(bitstr);

    if (mEntryVersion0.empty() == false)
    {
        bitstr.write32Bits(static_cast<std::uint32_t>(mEntryVersion0.size()));
        for (const auto& entry : mEntryVersion0)
        {
            bitstr.write32Bits(entry.mSampleCount);
            bitstr.write32Bits(entry.mSampleOffset);
        }
    }
    else if (mEntryVersion1.empty() == false)
    {
        bitstr.write32Bits(static_cast<std::uint32_t>(mEntryVersion1.size()));
        for (const auto& entry : mEntryVersion1)
        {
            bitstr.write32Bits(entry.mSampleCount);
            bitstr.write32Bits(static_cast<std::uint32_t>(entry.mSampleOffset));
        }
    }
    else
    {
        throw RuntimeError("Can not write an empty CompositionOffsetBox.");
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void CompositionOffsetBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    //  First parse the box header
    parseFullBoxHeader(bitstr);

    const std::uint32_t entryCount = bitstr.read32Bits();

    if (getVersion() == 0)
    {
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            EntryVersion0 entryVersion0;
            entryVersion0.mSampleCount  = bitstr.read32Bits();
            entryVersion0.mSampleOffset = bitstr.read32Bits();
            mEntryVersion0.push_back(entryVersion0);
        }
    }
    else if (getVersion() == 1)
    {
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            EntryVersion1 entryVersion1;
            entryVersion1.mSampleCount  = bitstr.read32Bits();
            entryVersion1.mSampleOffset = static_cast<std::int32_t>(bitstr.read32Bits());
            mEntryVersion1.push_back(entryVersion1);
        }
    }
}
