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

#include "sampledescriptionbox.hpp"

#include "avcsampleentry.hpp"
#include "hevcsampleentry.hpp"
#include "log.hpp"
#include "mp4audiosampleentrybox.hpp"

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

        /** @todo Add new sample entry types based on handler if necessary */
        if (boxType == "hvc1" || boxType == "hev1")
        {
            UniquePtr<HevcSampleEntry, SampleEntryBox> hevcSampleEntry(CUSTOM_NEW(HevcSampleEntry, ()));
            hevcSampleEntry->parseBox(entryBitStream);

            mIndex.push_back(std::move(hevcSampleEntry));
        }
        else if (boxType == "avc1" || boxType == "avc3")
        {
            UniquePtr<AvcSampleEntry, SampleEntryBox> avcSampleEntry(CUSTOM_NEW(AvcSampleEntry, ()));
            avcSampleEntry->parseBox(entryBitStream);
            mIndex.push_back(std::move(avcSampleEntry));
        }
        else if (boxType == "mp4a")
        {
            UniquePtr<MP4AudioSampleEntryBox, SampleEntryBox> mp4AudioSampleEntry(
                CUSTOM_NEW(MP4AudioSampleEntryBox, ()));
            mp4AudioSampleEntry->parseBox(entryBitStream);
            mIndex.push_back(std::move(mp4AudioSampleEntry));
        }
        else
        {
            logWarning() << "Skipping unknown SampleDescriptionBox entry of type '" << boxType << "'" << std::endl;
            // Push nullptr to keep indexing correct, in case it will still be possible to operate with the file.
            mIndex.push_back(nullptr);
        }
    }
}

/** @brief Get the list of sample entries.
 *  @returns Vector of sample entries */
const Vector<UniquePtr<SampleEntryBox>>& SampleDescriptionBox::getSampleEntries() const
{
    return mIndex;
}

/** @brief Get the sample entry at a particular index from the list.
 *  @param [in] index 1-based index of the sample entry
 *  @returns Sample Entry of defined type */
const SampleEntryBox* SampleDescriptionBox::getSampleEntry(unsigned int index) const
{
    if (mIndex.size() < index || index == 0)
    {
        throw RuntimeError("SampleDescriptionBox::getSampleEntry invalid sample entry index.");
    }

    return mIndex[index - 1].get();
}
