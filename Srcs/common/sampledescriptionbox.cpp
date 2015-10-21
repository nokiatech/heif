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

#include "sampledescriptionbox.hpp"
#include "hevcsampleentry.hpp"
#include "log.hpp"

SampleDescriptionBox::SampleDescriptionBox() :
    FullBox("stsd", 0, 0)
{
}


void SampleDescriptionBox::addSampleEntry(std::unique_ptr<SampleEntryBox> sampleEntry)
{
    mIndex.push_back(std::move(sampleEntry));
}

void SampleDescriptionBox::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);
    bitstr.write32Bits(mIndex.size());
    for (auto& entry : mIndex)
    {
        if (not entry)
        {
            throw std::runtime_error("SampleDescriptionBox::writeBox can not write file because an unknown sample entry type was present when the file was read.");
        }
        entry->writeBox(bitstr);
    }
    updateSize(bitstr);
}

void SampleDescriptionBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    const unsigned int entryCount = bitstr.read32Bits();
    for (unsigned int i = 0; i < entryCount; ++i)
    {
        // Extract contained box bitstream and type
        std::string boxType;
        BitStream entryBitStream = bitstr.readSubBoxBitStream(boxType);

        /** @todo Add new sample entry types based on handler if necessary **/
        if (boxType == "hvc1" || boxType == "hev1")
        {
            std::unique_ptr<HevcSampleEntry> hevcSampleEntry(new HevcSampleEntry);
            hevcSampleEntry->parseBox(entryBitStream);
            mIndex.push_back(std::move(hevcSampleEntry));
        }
        else
        {
            logWarning() << "Skipping unknown SampleDescriptionBox entry of type '" << boxType << "'" << std::endl;
            // Push nullptr to keep indexing correct, in case it will still be possible to operate with the file.
            mIndex.push_back(nullptr);
        }
    }
}
