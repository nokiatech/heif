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

#include "sampledescriptionbox.hpp"

#include "bitstream.hpp"

#include "avcsampleentry.hpp"
#include "hevcsampleentry.hpp"
#include "log.hpp"
#include "mp4audiosampleentrybox.hpp"
#include "vvcsampleentry.hpp"
#include "vvcsubpicsampleentry.hpp"
#include "fourccint.hpp"
#include "sampleentrybox.hpp"
#include <memory>
#include <utility>
#include <vector>


SampleDescriptionBox::SampleDescriptionBox()
    : FullBox("stsd", 0, 0)
{
}

void SampleDescriptionBox::addSampleEntry(UniquePtr<SampleEntryBox> sampleEntry)
{
    mIndex.push_back(std::move(sampleEntry));
}

void SampleDescriptionBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);
    bitstr.write32Bits(static_cast<unsigned int>(mIndex.size()));
    for (auto& entry : mIndex)
    {
        if (!entry)
        {
            throw RuntimeError(
                "SampleDescriptionBox::writeBox can not write file because an unknown sample entry type was present "
                "when the file was read.");
        }
        entry->writeBox(bitstr);
    }
    updateSize(bitstr);
}

void SampleDescriptionBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    const unsigned int entryCount = bitstr.read32Bits();
    for (unsigned int i = 0; i < entryCount; ++i)
    {
        // Extract contained box bitstream and type
        FourCCInt boxType;
        BitStream entryBitStream = bitstr.readSubBoxBitStream(boxType);

        ///@todo Add new sample entry types based on handler if necessary
        if (boxType == "hvc1" || boxType == "hev1")
        {
            auto hevcSampleEntry = makeCustomUnique<HevcSampleEntry, SampleEntryBox>();
            hevcSampleEntry->parseBox(entryBitStream);

            mIndex.push_back(std::move(hevcSampleEntry));
        }
        else if (boxType == "avc1" || boxType == "avc3")
        {
            auto avcSampleEntry = makeCustomUnique<AvcSampleEntry, SampleEntryBox>();
            avcSampleEntry->parseBox(entryBitStream);
            mIndex.push_back(std::move(avcSampleEntry));
        }
        else if (boxType == "vvc1")
        {
            auto vvcSampleEntry = makeCustomUnique<VvcSampleEntry, SampleEntryBox>();
            vvcSampleEntry->parseBox(entryBitStream);
            mIndex.push_back(std::move(vvcSampleEntry));
        }
        else if (boxType == "vvs1")
        {
            auto vvcSubpicSampleEntry = makeCustomUnique<VvcSubpicSampleEntry, SampleEntryBox>();
            vvcSubpicSampleEntry->parseBox(entryBitStream);
            mIndex.push_back(std::move(vvcSubpicSampleEntry));
        }
        else if (boxType == "mp4a")
        {
            auto mp4AudioSampleEntry = makeCustomUnique<MP4AudioSampleEntryBox, SampleEntryBox>();
            mp4AudioSampleEntry->parseBox(entryBitStream);
            mIndex.push_back(std::move(mp4AudioSampleEntry));
        }
        else
        {
            logWarning() << "Skipping unknown SampleDescriptionBox entry of type '" << boxType.getString() << "'"
                         << '\n';
            // Push nullptr to keep indexing correct, in case it will still be possible to operate with the file.
            mIndex.push_back(nullptr);
        }
    }
}

// Get the list of sample entries.
const Vector<UniquePtr<SampleEntryBox>>& SampleDescriptionBox::getSampleEntries() const
{
    return mIndex;
}

// Get the sample entry at a particular index from the list.
const SampleEntryBox* SampleDescriptionBox::getSampleEntry(unsigned int index) const
{
    if (mIndex.size() < index || index == 0)
    {
        throw RuntimeError("SampleDescriptionBox::getSampleEntry invalid sample entry index.");
    }

    return mIndex[index - 1].get();
}
