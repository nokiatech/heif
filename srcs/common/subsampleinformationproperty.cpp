/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "subsampleinformationproperty.hpp"

#include "bitstream.hpp"
#include "fourccint.hpp"

#include <vector>

SubSampleInformationProperty::SubSampleInformationProperty()
    : FullBox("subs", 0, 0)
{
}

void SubSampleInformationProperty::setSubSamples(const Vector<SubSampleEntry>& entries)
{
    mSubSamples = entries;
}

const Vector<SubSampleInformationProperty::SubSampleEntry>& SubSampleInformationProperty::getSubSamples() const
{
    return mSubSamples;
}

std::size_t SubSampleInformationProperty::getSubSampleCount() const
{
    return mSubSamples.size();
}

void SubSampleInformationProperty::writeBox(BitStream& output) const
{
    writeFullBoxHeader(output);

    //bit(32) entry_count -- always 1 for an item property
    output.write32Bits(1);

    //bit(32) sample_delta
    output.write32Bits(1);

    //bit(16) subsample_count
    const auto count = static_cast<uint16_t>(mSubSamples.size());
    output.write16Bits(count);

    for (const auto& ss : mSubSamples)
    {
        if (getVersion() == 1)
        {
            //bit(32) subsample_size
            output.write32Bits(ss.subsampleSize);
        }
        else
        {
            //bit(16) subsample_size
            output.write16Bits(static_cast<uint16_t>(ss.subsampleSize));
        }
        //bit(8) subsample_priority
        output.write8Bits(ss.subsamplePriority);
        //bit(8) discardable
        output.write8Bits(ss.discardable);
        //bit(32) codec_specific_parameters
        output.write32Bits(ss.codecSpecificParameters);
    }

    updateSize(output);
}

void SubSampleInformationProperty::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);

    //bit(32) entry_count
    const uint32_t entryCount = input.read32Bits();

    mSubSamples.clear();

    for (uint32_t i = 0; i < entryCount; ++i)
    {
        //bit(32) sample_delta
        input.read32Bits();  // ignored for item properties (always 1)

        //bit(16) subsample_count
        const uint16_t subsampleCount = input.read16Bits();

        for (uint16_t j = 0; j < subsampleCount; ++j)
        {
            SubSampleEntry entry{};
            if (getVersion() == 1)
            {
                //bit(32) subsample_size
                entry.subsampleSize = input.read32Bits();
            }
            else
            {
                //bit(16) subsample_size
                entry.subsampleSize = input.read16Bits();
            }
            //bit(8) subsample_priority
            entry.subsamplePriority = input.read8Bits();
            //bit(8) discardable
            entry.discardable = input.read8Bits();
            //bit(32) codec_specific_parameters
            entry.codecSpecificParameters = input.read32Bits();
            mSubSamples.push_back(entry);
        }
    }
}
