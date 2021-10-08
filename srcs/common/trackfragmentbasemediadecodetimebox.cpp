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

#include "trackfragmentbasemediadecodetimebox.hpp"

#include <stdexcept>

TrackFragmentBaseMediaDecodeTimeBox::TrackFragmentBaseMediaDecodeTimeBox()
    : FullBox("tfdt", 0, 0)
    , mBaseMediaDecodeTime(0)
{
}

void TrackFragmentBaseMediaDecodeTimeBox::setBaseMediaDecodeTime(const uint64_t baseMediaDecodeTime)
{
    mBaseMediaDecodeTime = baseMediaDecodeTime;
}

uint64_t TrackFragmentBaseMediaDecodeTimeBox::getBaseMediaDecodeTime() const
{
    return mBaseMediaDecodeTime;
}

void TrackFragmentBaseMediaDecodeTimeBox::writeBox(BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);
    if (getVersion() == 0)
    {
        bitstr.write32Bits(static_cast<uint32_t>(mBaseMediaDecodeTime));
    }
    else if (getVersion() == 1)
    {
        bitstr.write64Bits(mBaseMediaDecodeTime);
    }
    else
    {
        throw RuntimeError("TrackFragmentBaseMediaDecodeTimeBox::writeBox() supports only 'tfdt' version 0 or 1");
    }
    updateSize(bitstr);
}

void TrackFragmentBaseMediaDecodeTimeBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    if (getVersion() == 0)
    {
        mBaseMediaDecodeTime = bitstr.read32Bits();
    }
    else if (getVersion() == 1)
    {
        mBaseMediaDecodeTime = bitstr.read64Bits();
    }
    else
    {
        throw RuntimeError("TrackFragmentBaseMediaDecodeTimeBox::parseBox() supports only 'tfdt' version 0 or 1");
    }
}
