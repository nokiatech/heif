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

#include "sampletogroupbox.hpp"
#include <stdexcept>

SampleToGroupBox::SampleToGroupBox() :
    FullBox("sbgp", 0, 0),
    mEntryCount(0),
    mRunOfSamples()
{
}

void SampleToGroupBox::setGroupingType(const std::string& groupingType)
{
    if (groupingType.length() != 4)
    {
        throw std::runtime_error("SampleToGroupBox::setGroupingType: invalid grouping type length, must be 4 characters");
    }
    mGroupingType = groupingType;
}

const std::string& SampleToGroupBox::getGroupingType() const
{
    return mGroupingType;
}

void SampleToGroupBox::setEntryCount(std::uint32_t entryCount)
{
    mEntryCount = entryCount;
}

void SampleToGroupBox::addSampleRun(std::uint32_t sampleCount, std::uint32_t groupDescriptionIndex)
{
    SampleRun sampleRun;
    sampleRun.sampleCount = sampleCount;
    sampleRun.groupDescriptionIndex = groupDescriptionIndex;

    mRunOfSamples.push_back(sampleRun);

    setEntryCount(mRunOfSamples.size());
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

    throw std::runtime_error("SampleToGroupBox::getSampleId: no entry for requested sample id");
}

unsigned int SampleToGroupBox::getNumberOfSamples() const
{
    return mSampleToGroupIndex.size();
}

void SampleToGroupBox::updateInternalIndex()
{
    mSampleToGroupIndex.clear();
    for (const auto& sampleRun : mRunOfSamples)
    {
        mSampleToGroupIndex.insert(mSampleToGroupIndex.end(), sampleRun.sampleCount, sampleRun.groupDescriptionIndex);
    }
}

void SampleToGroupBox::writeBox(BitStream& bitstr)
{
    if (mRunOfSamples.size() == 0)
    {
        throw std::runtime_error("SampleToGroupBox::writeBox: not writing an invalid box without entries");
    }

    // Write box headers
    writeFullBoxHeader(bitstr);

    if (mGroupingType.length() != 4)
    {
        throw std::runtime_error("SampleToGroupBox::writeBox: Invalid grouping_type length");
    }
    bitstr.writeString(mGroupingType);

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

void SampleToGroupBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    bitstr.readStringWithLen(mGroupingType, 4);

    if (getVersion() == 1)
    {
        mGroupingTypeParameter = bitstr.read32Bits();
    }

    mEntryCount = bitstr.read32Bits();
    if (mEntryCount == 0)
    {
        throw std::runtime_error("Read an empty SampleToGroupBox without entries.");
    }

    for (unsigned int i = 0; i < mEntryCount; ++i)
    {
        SampleRun sampleRun;
        sampleRun.sampleCount = bitstr.read32Bits();
        sampleRun.groupDescriptionIndex = bitstr.read32Bits();
        mRunOfSamples.push_back(sampleRun);
    }

    updateInternalIndex();
}
