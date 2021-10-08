/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "trackgrouptypebox.hpp"

TrackGroupTypeBox::TrackGroupTypeBox(FourCCInt boxType, std::uint32_t trackGroupId)
    : FullBox(boxType, 0, 0)
    , mTrackGroupId(trackGroupId)
{
}

std::uint32_t TrackGroupTypeBox::getTrackGroupId() const
{
    return mTrackGroupId;
}

void TrackGroupTypeBox::setTrackGroupId(std::uint32_t trackGroupId)
{
    mTrackGroupId = trackGroupId;
}

void TrackGroupTypeBox::writeBox(BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);
    bitstr.write32Bits(mTrackGroupId);
    updateSize(bitstr);
}

void TrackGroupTypeBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    mTrackGroupId = bitstr.read32Bits();
}

TrackGroupTypeBox* TrackGroupTypeBox::clone() const
{
    return new TrackGroupTypeBox(*this);
}
