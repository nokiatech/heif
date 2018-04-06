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

#include "samplegroupdescriptionbox.hpp"
#include "directreferencesampleslist.hpp"
#include "sampletometadataitementry.hpp"
#include "visualequivalenceentry.hpp"

#include "bitstream.hpp"
#include "log.hpp"

#include <stdexcept>

SampleGroupDescriptionBox::SampleGroupDescriptionBox()
    : FullBox("sgpd", 0, 0)
    , mGroupingType()
    , mDefaultLength(0)
    , mSampleGroupEntry()
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

void SampleGroupDescriptionBox::addEntry(UniquePtr<SampleGroupDescriptionEntry> sampleGroupEntry)
{
    mSampleGroupEntry.push_back(std::move(sampleGroupEntry));
}

const SampleGroupDescriptionEntry* SampleGroupDescriptionBox::getEntry(std::uint32_t index) const
{
    return mSampleGroupEntry.at(index - 1).get();
}

void SampleGroupDescriptionBox::setGroupingType(FourCCInt groupingType)
{
    mGroupingType = groupingType;
}

FourCCInt SampleGroupDescriptionBox::getGroupingType() const
{
    return mGroupingType;
}

std::uint32_t SampleGroupDescriptionBox::getEntryIndexOfSampleId(const std::uint32_t sampleId) const
{
    uint32_t index = 1;
    for (const auto& entry : mSampleGroupEntry)
    {
        if (entry->getGroupingType() == FourCCInt("refs"))
        {
            const DirectReferenceSamplesList* drsle = static_cast<const DirectReferenceSamplesList*>(entry.get());
            if (drsle->getSampleId() == sampleId)
            {
                return index;
            }
        }
        ++index;
    }

    throw RuntimeError("SampleGroupDescriptionBox::getEntryIndexOfSampleId: no entry for sampleId found.");
}

uint32_t SampleGroupDescriptionBox::getEntryCount() const
{
    return static_cast<uint32_t>(mSampleGroupEntry.size());
}

void SampleGroupDescriptionBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    if (mSampleGroupEntry.size() == 0)
    {
        throw RuntimeError("SampleGroupDescriptionBox::writeBox: not writing an invalid box without entries");
    }

    // Write box headers
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mGroupingType.getUInt32());

    if (getVersion() == 1)
    {
        bitstr.write32Bits(mDefaultLength);
    }

    bitstr.write32Bits(static_cast<unsigned int>(mSampleGroupEntry.size()));

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

void SampleGroupDescriptionBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    //  First parse the box header
    parseFullBoxHeader(bitstr);

    mGroupingType = bitstr.read32Bits();

    if (getVersion() == 1)
    {
        mDefaultLength = bitstr.read32Bits();
    }

    if (getVersion() >= 2)
    {
        bitstr.read32Bits();  // default_sample_description_index;
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
        bitstr.extract(bitstr.getPos(), bitstr.getPos() + descriptionLength,
                       subBitstr);  // extract "sub-bitstream" for entry
        bitstr.skipBytes(descriptionLength);

        if (mGroupingType == "refs")
        {
            UniquePtr<SampleGroupDescriptionEntry> directReferenceSampleListEntry(
                CUSTOM_NEW(DirectReferenceSamplesList, ()));
            directReferenceSampleListEntry->parseEntry(subBitstr);
            mSampleGroupEntry.push_back(std::move(directReferenceSampleListEntry));
        }
        else if (mGroupingType == "eqiv")
        {
            UniquePtr<SampleGroupDescriptionEntry> visualEquivalenceEntry(CUSTOM_NEW(VisualEquivalenceEntry, ()));
            visualEquivalenceEntry->parseEntry(subBitstr);
            mSampleGroupEntry.push_back(std::move(visualEquivalenceEntry));
        }
        else if (mGroupingType == "stmi")
        {
            UniquePtr<SampleGroupDescriptionEntry> sampleToMetadataItemEntry(CUSTOM_NEW(SampleToMetadataItemEntry, ()));
            sampleToMetadataItemEntry->parseEntry(subBitstr);
            mSampleGroupEntry.push_back(std::move(sampleToMetadataItemEntry));
        }
        else
        {
            /** @todo Add support for other entry types here. */
            logWarning() << "Skipping an entry of SampleGroupDescriptionBox of an unknown grouping type '"
                         << mGroupingType.getString() << "'.";
        }
    }
}
