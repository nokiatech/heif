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

#include "samplegroupentry.hpp"

#include <stdexcept>

DirectReferenceSampleListEntry::DirectReferenceSampleListEntry() :
    mSampleId(0)
{
}

void DirectReferenceSampleListEntry::setSampleId(const std::uint32_t sampleId)
{
    mSampleId = sampleId;
}

std::uint32_t DirectReferenceSampleListEntry::getSampleId() const
{
    return mSampleId;
}

void DirectReferenceSampleListEntry::setDirectReferenceSampleIds(const std::vector<std::uint32_t>& referenceSampleIds)
{
    if (referenceSampleIds.size() > 255)
    {
        throw std::runtime_error("Too many entries in referenceSampleIds");
    }

    mDirectReferenceSampleIds = referenceSampleIds;
}

std::vector<std::uint32_t> DirectReferenceSampleListEntry::getDirectReferenceSampleIds() const
{
    return mDirectReferenceSampleIds;
}

std::uint32_t DirectReferenceSampleListEntry::getSize() const
{
    // Sizes: sample_id (4 bytes), num_referenced_samples (1 byte), reference_sample_id (4 bytes) * number of references
    const uint32_t size = sizeof(mSampleId) + sizeof(uint8_t) + (sizeof(uint32_t) * mDirectReferenceSampleIds.size());
    return size;
}

void DirectReferenceSampleListEntry::writeEntry(BitStream& bitstr)
{
    bitstr.write32Bits(mSampleId);

    bitstr.write8Bits(mDirectReferenceSampleIds.size());
    for (auto id : mDirectReferenceSampleIds)
    {
        bitstr.write32Bits(id);
    }
}

void DirectReferenceSampleListEntry::parseEntry(BitStream& bitstr)
{
    mSampleId = bitstr.read32Bits();
    const uint8_t numberOfReferencedSamples = bitstr.read8Bits();
    for (unsigned int i = 0; i < numberOfReferencedSamples; ++i)
    {
        mDirectReferenceSampleIds.push_back(bitstr.read32Bits());
    }
}
