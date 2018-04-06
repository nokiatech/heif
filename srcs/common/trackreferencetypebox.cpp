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

#include "trackreferencetypebox.hpp"
#include "log.hpp"

using namespace std;

TrackReferenceTypeBox::TrackReferenceTypeBox(FourCCInt trefType)
    : Box(trefType)
    , mTrackId()
{
}

void TrackReferenceTypeBox::setTrackIds(const Vector<uint32_t>& trackId)
{
    mTrackId = trackId;
}

const Vector<uint32_t>& TrackReferenceTypeBox::getTrackIds() const
{
    return mTrackId;
}

void TrackReferenceTypeBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeBoxHeader(bitstr);

    for (auto trackId : mTrackId)
    {
        bitstr.write32Bits(trackId);
    }

    updateSize(bitstr);
}

void TrackReferenceTypeBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    // if there a data available in the file
    while (bitstr.numBytesLeft() > 0)
    {
        mTrackId.push_back(bitstr.read32Bits());
    }
}
