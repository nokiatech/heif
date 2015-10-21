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

#include "compositionoffsetbox.hpp"
#include "bitstream.hpp"

#include <stdexcept>

CompositionOffsetBox::CompositionOffsetBox() :
    FullBox("ctts", 0, 0)
{
}

void CompositionOffsetBox::addCompositionOffsetEntryVersion0(const EntryVersion0& entry)
{
    if (mEntryVersion1.size() != 0 || getVersion() != 0)
    {
        throw std::runtime_error("Invalid attempt to add version0 CompositionOffsetBox entries.");
    }
    mEntryVersion0.push_back(entry);
}

void CompositionOffsetBox::addCompositionOffsetEntryVersion1(const EntryVersion1& entry)
{
    if (mEntryVersion0.size() != 0 || getVersion() != 1)
    {
        throw std::runtime_error("Invalid attempt to add version1 CompositionOffsetBox entries.");
    }
    mEntryVersion1.push_back(entry);
}

std::vector<int> CompositionOffsetBox::getSampleCompositionOffsets() const
{
    std::vector<int> offsets;
    if (getVersion() == 0)
    {
        for (const auto& entry : mEntryVersion0)
        {
            for (unsigned int i = 0; i < entry.mSampleCount; ++i)
            {
                offsets.push_back(entry.mSampleOffset);
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

void CompositionOffsetBox::writeBox(BitStream& bitstr)
{
    // Write box headers
    writeFullBoxHeader(bitstr);

    if (mEntryVersion0.empty() == false)
    {
        bitstr.write32Bits(mEntryVersion0.size());
        for (const auto& entry : mEntryVersion0)
        {
            bitstr.write32Bits(entry.mSampleCount);
            bitstr.write32Bits(entry.mSampleOffset);
        }
    }
    else if (mEntryVersion1.empty() == false)
    {
        bitstr.write32Bits(mEntryVersion1.size());
        for (const auto& entry : mEntryVersion1)
        {
            bitstr.write32Bits(entry.mSampleCount);
            bitstr.write32Bits(static_cast<std::uint32_t>(entry.mSampleOffset));
        }
    }
    else
    {
        throw std::runtime_error("Can not write an empty CompositionOffsetBox.");
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void CompositionOffsetBox::parseBox(BitStream& bitstr)
{
    //  First parse the box header
    parseFullBoxHeader(bitstr);

    const std::uint32_t entryCount = bitstr.read32Bits();

    if (getVersion() == 0)
    {
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            EntryVersion0 entryVersion0;
            entryVersion0.mSampleCount = bitstr.read32Bits();
            entryVersion0.mSampleOffset = bitstr.read32Bits();
            mEntryVersion0.push_back(entryVersion0);
        }
    }
    else if (getVersion() == 1)
    {
        for (uint32_t i = 0; i < entryCount; ++i)
        {
            EntryVersion1 entryVersion1;
            entryVersion1.mSampleCount = bitstr.read32Bits();
            entryVersion1.mSampleOffset = static_cast<std::int32_t>(bitstr.read32Bits());
            mEntryVersion1.push_back(entryVersion1);
        }
    }
}
