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

#include "samplegroupdescriptionbox.hpp"

#include "bitstream.hpp"
#include "log.hpp"

#include <memory>
#include <stdexcept>

SampleGroupDescriptionBox::SampleGroupDescriptionBox() :
    FullBox("sgpd", 0, 0),
    mGroupingType(),
    mDefaultLength(0),
    mSampleGroupEntry()
{
}

void SampleGroupDescriptionBox::setDefaultLength(std::uint32_t defaultLength)
{
    mDefaultLength = defaultLength;
}

std::uint32_t SampleGroupDescriptionBox::getDefaultLength() const
{
    return mDefaultLength;
}

void SampleGroupDescriptionBox::addEntry(std::unique_ptr<SampleGroupEntry> sampleGroupEntry)
{
    mSampleGroupEntry.push_back(std::move(sampleGroupEntry));
}

const SampleGroupEntry* SampleGroupDescriptionBox::getEntry(std::uint32_t index) const
{
    return mSampleGroupEntry.at(index - 1).get();
}

void SampleGroupDescriptionBox::setGroupingType(const std::string& groupingType)
{
    if (groupingType.length() != 4)
    {
        throw std::runtime_error("SampleGroupDescriptionBox::setGroupingType: invalid grouping type length, must be 4 characters");
    }
    mGroupingType = groupingType;
}

const std::string& SampleGroupDescriptionBox::getGroupingType() const
{
    return mGroupingType;
}

std::uint32_t SampleGroupDescriptionBox::getEntryIndexOfSampleId(const std::uint32_t sampleId) const
{
    uint32_t index = 1;
    for (const auto& entry : mSampleGroupEntry)
    {
        DirectReferenceSampleListEntry* drsle = dynamic_cast<DirectReferenceSampleListEntry*>(entry.get());
        if ((drsle != nullptr) && (drsle->getSampleId() == sampleId))
        {
            return index;
        }
        ++index;
    }

    throw std::runtime_error("SampleGroupDescriptionBox::getEntryIndexOfSampleId: no entry for sampleId found.");
}

void SampleGroupDescriptionBox::writeBox(BitStream& bitstr)
{
    if (mSampleGroupEntry.size() == 0)
    {
        throw std::runtime_error("SampleGroupDescriptionBox::writeBox: not writing an invalid box without entries");
    }

    // Write box headers
    writeFullBoxHeader(bitstr);

    if (mGroupingType.length() != 4)
    {
        throw std::runtime_error("SampleGroupDescriptionBox::writeBox: Invalid grouping_type length");
    }
    bitstr.writeString(mGroupingType);

    if (getVersion() == 1)
    {
        bitstr.write32Bits(mDefaultLength);
    }

    bitstr.write32Bits(mSampleGroupEntry.size());

    for (auto& entry : mSampleGroupEntry)
    {
        if (getVersion() == 1 && mDefaultLength == 0)
        {
            bitstr.write32Bits(entry->getSize());
        }
        entry->writeEntry(bitstr);
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void SampleGroupDescriptionBox::parseBox(BitStream& bitstr)
{
    //  First parse the box header
    parseFullBoxHeader(bitstr);

    bitstr.readStringWithLen(mGroupingType, 4);

    if (getVersion() == 1)
    {
        mDefaultLength = bitstr.read32Bits();
    }

    const uint32_t entryCount = bitstr.read32Bits();

    for (unsigned int i = 0; i < entryCount; ++i)
    {
        uint32_t descriptionLength = mDefaultLength;
        if (getVersion() == 1 && mDefaultLength == 0)
        {
            descriptionLength = bitstr.read32Bits();
        }

        BitStream subBitstr;
        bitstr.extract(bitstr.getPos(), bitstr.getPos() + descriptionLength, subBitstr); // extract "sub-bitstream" for entry
        bitstr.skipBytes(descriptionLength);

        if (mGroupingType == "refs")
        {
            std::unique_ptr<SampleGroupEntry> directReferenceSampleListEntry(new DirectReferenceSampleListEntry);
            directReferenceSampleListEntry->parseEntry(subBitstr);
            mSampleGroupEntry.push_back(std::move(directReferenceSampleListEntry));
        }
        else
        {
            /** @todo Add support for other entry types here. **/
            logWarning() << "Skipping an entry of SampleGroupDescriptionBox of an unknown grouping type '" << mGroupingType << "'.";
        }
    }
}
