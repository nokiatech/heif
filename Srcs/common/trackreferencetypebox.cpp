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

#include "trackreferencetypebox.hpp"
#include "log.hpp"

using namespace std;

TrackReferenceTypeBox::TrackReferenceTypeBox(const char* trefType) :
    Box(trefType),
    mTrackId()
{
}

void TrackReferenceTypeBox::setTrackIds(const vector<uint32_t>& trackId)
{
    mTrackId = trackId;
}

const vector<uint32_t>& TrackReferenceTypeBox::getTrackIds() const
{
    return mTrackId;
}

void TrackReferenceTypeBox::writeBox(BitStream& bitstr)
{
    writeBoxHeader(bitstr);

    for (auto trackId :  mTrackId)
    {
        bitstr.write32Bits(trackId);
    }

    updateSize(bitstr);
}

void TrackReferenceTypeBox::parseBox(BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    // if there a data available in the file
    while (bitstr.numBytesLeft() > 0)
    {
        mTrackId.push_back(bitstr.read32Bits());
    }
}
