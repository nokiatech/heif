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

#include "directreferencesampleslist.hpp"

#include <stdexcept>

DirectReferenceSamplesList::DirectReferenceSamplesList()
    : SampleGroupDescriptionEntry("refs")
    , mSampleId(0)
{
}

void DirectReferenceSamplesList::setSampleId(const std::uint32_t sampleId)
{
    mSampleId = sampleId;
}

std::uint32_t DirectReferenceSamplesList::getSampleId() const
{
    return mSampleId;
}

void DirectReferenceSamplesList::setDirectReferenceSampleIds(const Vector<std::uint32_t>& referenceSampleIds)
{
    if (referenceSampleIds.size() > 255)
    {
        throw RuntimeError("Too many entries in referenceSampleIds");
    }

    mDirectReferenceSampleIds = referenceSampleIds;
}

Vector<std::uint32_t> DirectReferenceSamplesList::getDirectReferenceSampleIds() const
{
    return mDirectReferenceSampleIds;
}

std::uint32_t DirectReferenceSamplesList::getSize() const
{
    // Sizes: sample_id (4 bytes), num_referenced_samples (1 byte), reference_sample_id (4 bytes) * number of references
    const uint32_t size = static_cast<uint32_t>(sizeof(mSampleId) + sizeof(uint8_t) +
                                                (sizeof(uint32_t) * mDirectReferenceSampleIds.size()));
    return size;
}

void DirectReferenceSamplesList::writeEntry(ISOBMFF::BitStream& bitstr)
{
    bitstr.write32Bits(mSampleId);

    bitstr.write8Bits(static_cast<std::uint8_t>(mDirectReferenceSampleIds.size()));
    for (auto id : mDirectReferenceSampleIds)
    {
        bitstr.write32Bits(id);
    }
}

void DirectReferenceSamplesList::parseEntry(ISOBMFF::BitStream& bitstr)
{
    mSampleId                               = bitstr.read32Bits();
    const uint8_t numberOfReferencedSamples = bitstr.read8Bits();
    for (unsigned int i = 0; i < numberOfReferencedSamples; ++i)
    {
        mDirectReferenceSampleIds.push_back(bitstr.read32Bits());
    }
}
