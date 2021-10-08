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

#include "trackextendsbox.hpp"

TrackExtendsBox::TrackExtendsBox()
    : FullBox("trex", 0, 0)
    , mSampleDefaults()
{
}

void TrackExtendsBox::setFragmentSampleDefaults(const MOVIEFRAGMENTS::SampleDefaults& fragmentSampleDefaults)
{
    mSampleDefaults = fragmentSampleDefaults;
}

const MOVIEFRAGMENTS::SampleDefaults& TrackExtendsBox::getFragmentSampleDefaults() const
{
    return mSampleDefaults;
}

void TrackExtendsBox::writeBox(BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);
    bitstr.write32Bits(mSampleDefaults.trackId);
    bitstr.write32Bits(mSampleDefaults.defaultSampleDescriptionIndex);
    bitstr.write32Bits(mSampleDefaults.defaultSampleDuration);
    bitstr.write32Bits(mSampleDefaults.defaultSampleSize);
    MOVIEFRAGMENTS::SampleFlags::write(bitstr, mSampleDefaults.defaultSampleFlags);
    updateSize(bitstr);
}

void TrackExtendsBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    mSampleDefaults.trackId                       = bitstr.read32Bits();
    mSampleDefaults.defaultSampleDescriptionIndex = bitstr.read32Bits();
    mSampleDefaults.defaultSampleDuration         = bitstr.read32Bits();
    mSampleDefaults.defaultSampleSize             = bitstr.read32Bits();
    mSampleDefaults.defaultSampleFlags            = MOVIEFRAGMENTS::SampleFlags::read(bitstr);
}
