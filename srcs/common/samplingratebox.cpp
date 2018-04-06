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

#include "samplingratebox.hpp"

SamplingRateBox::SamplingRateBox()
    : FullBox("srat", 0, 0)
    , mSamplingRate(0)
{
}

SamplingRateBox::SamplingRateBox(const SamplingRateBox& box)
    : FullBox(box.getType(), 0, 0)
    , mSamplingRate(box.mSamplingRate)
{
}

std::uint32_t SamplingRateBox::getSamplingRate() const
{
    return mSamplingRate;
}

void SamplingRateBox::setSamplingRate(std::uint32_t samplingRate)
{
    mSamplingRate = samplingRate;
}

void SamplingRateBox::writeBox(BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mSamplingRate);

    updateSize(bitstr);
}

void SamplingRateBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    mSamplingRate = bitstr.read32Bits();
}
