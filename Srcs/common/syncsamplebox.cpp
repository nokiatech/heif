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

#include "syncsamplebox.hpp"

SyncSampleBox::SyncSampleBox() :
    FullBox("stss", 0, 0)
{
}


void SyncSampleBox::addSample(std::uint32_t sampleNumber)
{
    mSampleNumber.push_back(sampleNumber);
}


void SyncSampleBox::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mSampleNumber.size());

    for (auto sampleNumber : mSampleNumber)
    {
        bitstr.write32Bits(sampleNumber);
    }

    // Update the size
    updateSize(bitstr);
}


void SyncSampleBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    std::uint32_t entryCount = bitstr.read32Bits();

    for (std::uint32_t i = 0; i < entryCount; ++i)
    {
        mSampleNumber.push_back(bitstr.read32Bits());
    }
}
