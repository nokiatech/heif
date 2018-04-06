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

#include "sampletogroupbox.hpp"
#include <limits>
#include <stdexcept>

SampleToGroupBox::SampleToGroupBox()
    : FullBox("sbgp", 0, 0)
    , mEntryCount(0)
    , mGroupingTypeParameter(0)
    , mRunOfSamples()
{
}

void SampleToGroupBox::setGroupingType(FourCCInt groupingType)
{
    mGroupingType = groupingType;
}

FourCCInt SampleToGroupBox::getGroupingType() const
{
    return mGroupingType;
}

void SampleToGroupBox::setGroupingTypeParameter(std::uint32_t groupingTypeParameter)
{
    mGroupingTypeParameter = groupingTypeParameter;
}

std::uint32_t SampleToGroupBox::getGroupingTypeParameter() const
{
    return mGroupingTypeParameter;
}

void SampleToGroupBox::setEntryCount(std::uint32_t entryCount)
{
    mEntryCount = entryCount;
}

void SampleToGroupBox::addSampleRun(std::uint32_t sampleCount, std::uint32_t groupDescriptionIndex)
{
    SampleRun sampleRun;
    sampleRun.sampleCount           = sampleCount;
    sampleRun.groupDescriptionIndex = groupDescriptionIndex;

    mRunOfSamples.push_back(sampleRun);

    setEntryCount(static_cast<unsigned int>(mRunOfSamples.size()));
    updateInternalIndex();
}

std::uint32_t SampleToGroupBox::getSampleGroupDescriptionIndex(const std::uint32_t sampleIndex) const
{
    if (sampleIndex >= mSampleToGroupIndex.size())
    {
        // No group
        return 0;
    }

    return mSampleToGroupIndex.at(sampleIndex);
}

std::uint32_t SampleToGroupBox::getSampleId(std::uint32_t groupDescriptionIndex) const
{
    for (unsigned int i = 0; i < mSampleToGroupIndex.size(); ++i)
    {
        if (groupDescriptionIndex == mSampleToGroupIndex.at(i))
        {
            return i;
        }
    }

    throw RuntimeError("SampleToGroupBox::getSampleId: no entry for requested sample id");
}

unsigned int SampleToGroupBox::getNumberOfSamples() const
{
    return static_cast<unsigned int>(mSampleToGroupIndex.size());
}

void SampleToGroupBox::updateInternalIndex()
{
    mSampleToGroupIndex.clear();
    for (const auto& sampleRun : mRunOfSamples)
    {
        mSampleToGroupIndex.insert(mSampleToGroupIndex.end(), sampleRun.sampleCount, sampleRun.groupDescriptionIndex);
    }
}

void SampleToGroupBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    if (mRunOfSamples.size() == 0)
    {
        throw RuntimeError("SampleToGroupBox::writeBox: not writing an invalid box without entries");
    }

    // Write box headers
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mGroupingType.getUInt32());

    if (getVersion() == 1)
    {
        bitstr.write32Bits(mGroupingTypeParameter);
    }

    bitstr.write32Bits(mEntryCount);

    for (auto entry : mRunOfSamples)
    {
        bitstr.write32Bits(entry.sampleCount);
        bitstr.write32Bits(entry.groupDescriptionIndex);
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void SampleToGroupBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    mGroupingType = bitstr.read32Bits();

    if (getVersion() == 1)
    {
        mGroupingTypeParameter = bitstr.read32Bits();
    }

    mEntryCount = bitstr.read32Bits();
    if (mEntryCount == 0)
    {
        throw RuntimeError("Read an empty SampleToGroupBox without entries.");
    }

    uint64_t sampleCount = 0;
    for (unsigned int i = 0; i < mEntryCount; ++i)
    {
        SampleRun sampleRun;
        sampleRun.sampleCount = bitstr.read32Bits();
        sampleCount += sampleRun.sampleCount;
        if (sampleCount > std::numeric_limits<std::uint32_t>::max())
        {
            throw RuntimeError("SampleToGroupBox  sampleCount >= 2^32");
        }
        sampleRun.groupDescriptionIndex = bitstr.read32Bits();
        mRunOfSamples.push_back(sampleRun);
    }

    updateInternalIndex();
}
