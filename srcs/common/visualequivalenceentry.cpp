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

#include "visualequivalenceentry.hpp"

VisualEquivalenceEntry::VisualEquivalenceEntry()
    : SampleGroupDescriptionEntry("eqiv")
    , mTimeOffset(0)
    , mTimescaleMultiplier(0)
{
}

void VisualEquivalenceEntry::setTimeOffset(std::int16_t timeOffset)
{
    mTimeOffset = timeOffset;
}

std::int16_t VisualEquivalenceEntry::getTimeOffset() const
{
    return mTimeOffset;
}

void VisualEquivalenceEntry::setTimescaleMultiplier(uint16_t multiplier)
{
    mTimescaleMultiplier = multiplier;
}

std::uint16_t VisualEquivalenceEntry::getTimescaleMultiplier() const
{
    return mTimescaleMultiplier;
}

std::uint32_t VisualEquivalenceEntry::getSize() const
{
    const uint32_t size = static_cast<uint32_t>(sizeof(mTimeOffset) + sizeof(mTimescaleMultiplier));
    return size;
}

void VisualEquivalenceEntry::writeEntry(ISOBMFF::BitStream& bitstr)
{
    bitstr.write16Bits(static_cast<uint16_t>(mTimeOffset));
    bitstr.write16Bits(mTimescaleMultiplier);
}

void VisualEquivalenceEntry::parseEntry(ISOBMFF::BitStream& bitstr)
{
    mTimeOffset          = static_cast<int16_t>(bitstr.read16Bits());
    mTimescaleMultiplier = bitstr.read16Bits();
}
