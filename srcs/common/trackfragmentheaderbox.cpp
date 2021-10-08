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

#include "trackfragmentheaderbox.hpp"

#include <stdexcept>

TrackFragmentHeaderBox::TrackFragmentHeaderBox(std::uint32_t tr_flags)
    : FullBox("tfhd", 0, tr_flags)
    , mTrackId(0)
    , mBaseDataOffset(0)
    , mSampleDescriptionIndex(0)
    , mDefaultSampleDuration(0)
    , mDefaultSampleSize(0)
{
    mDefaultSampleFlags.flagsAsUInt = 0;
}

void TrackFragmentHeaderBox::setTrackId(const uint32_t trackId)
{
    mTrackId = trackId;
}

uint32_t TrackFragmentHeaderBox::getTrackId() const
{
    return mTrackId;
}

void TrackFragmentHeaderBox::setBaseDataOffset(const uint64_t baseDataOffset)
{
    mBaseDataOffset = baseDataOffset;
    setFlags(getFlags() | TrackFragmentHeaderBox::BaseDataOffsetPresent);
}

uint64_t TrackFragmentHeaderBox::getBaseDataOffset() const
{
    if ((getFlags() & TrackFragmentHeaderBox::BaseDataOffsetPresent) != 0)
    {
        return mBaseDataOffset;
    }
    else
    {
        throw RuntimeError(
            "TrackFragmentHeaderBox::getBaseDataOffset() according to flags BaseDataOffsetPresent not present.");
    }
}

void TrackFragmentHeaderBox::setSampleDescriptionIndex(const uint32_t sampleDescriptionIndex)
{
    mSampleDescriptionIndex = sampleDescriptionIndex;
}

uint32_t TrackFragmentHeaderBox::getSampleDescriptionIndex() const
{
    return mSampleDescriptionIndex;
}

void TrackFragmentHeaderBox::setDefaultSampleDuration(const uint32_t defaultSampleDuration)
{
    mDefaultSampleDuration = defaultSampleDuration;
    setFlags(getFlags() | TrackFragmentHeaderBox::DefaultSampleDurationPresent);
}

uint32_t TrackFragmentHeaderBox::getDefaultSampleDuration() const
{
    if ((getFlags() & TrackFragmentHeaderBox::DefaultSampleDurationPresent) != 0)
    {
        return mDefaultSampleDuration;
    }
    else
    {
        throw RuntimeError(
            "TrackFragmentHeaderBox::getDefaultSampleDuration() according to flags DefaultSampleDurationPresent not "
            "present.");
    }
}

void TrackFragmentHeaderBox::setDefaultSampleSize(const uint32_t defaultSampleSize)
{
    mDefaultSampleSize = defaultSampleSize;
    setFlags(getFlags() | TrackFragmentHeaderBox::DefaultSampleSizePresent);
}

uint32_t TrackFragmentHeaderBox::getDefaultSampleSize() const
{
    if ((getFlags() & TrackFragmentHeaderBox::DefaultSampleSizePresent) != 0)
    {
        return mDefaultSampleSize;
    }
    else
    {
        throw RuntimeError(
            "TrackFragmentHeaderBox::getDefaultSampleSize() according to flags DefaultSampleSizePresent not present.");
    }
}

void TrackFragmentHeaderBox::setDefaultSampleFlags(const MOVIEFRAGMENTS::SampleFlags defaultSampleFlags)
{
    mDefaultSampleFlags = defaultSampleFlags;
    setFlags(getFlags() | TrackFragmentHeaderBox::DefaultSampleFlagsPresent);
}

MOVIEFRAGMENTS::SampleFlags TrackFragmentHeaderBox::getDefaultSampleFlags() const
{
    if ((getFlags() & TrackFragmentHeaderBox::DefaultSampleFlagsPresent) != 0)
    {
        return mDefaultSampleFlags;
    }
    else
    {
        throw RuntimeError(
            "TrackFragmentHeaderBox::setDefaultSampleFlags() according to flags DefaultSampleFlagsPresent not "
            "present.");
    }
}

void TrackFragmentHeaderBox::writeBox(BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mTrackId);
    if ((getFlags() & TrackFragmentHeaderBox::BaseDataOffsetPresent) != 0)
    {
        bitstr.write64Bits(mBaseDataOffset);
    }
    if ((getFlags() & TrackFragmentHeaderBox::SampleDescriptionIndexPresent) != 0)
    {
        bitstr.write32Bits(mSampleDescriptionIndex);
    }
    if ((getFlags() & TrackFragmentHeaderBox::DefaultSampleDurationPresent) != 0)
    {
        bitstr.write32Bits(mDefaultSampleDuration);
    }
    if ((getFlags() & TrackFragmentHeaderBox::DefaultSampleSizePresent) != 0)
    {
        bitstr.write32Bits(mDefaultSampleSize);
    }
    if ((getFlags() & TrackFragmentHeaderBox::DefaultSampleFlagsPresent) != 0)
    {
        MOVIEFRAGMENTS::SampleFlags::write(bitstr, mDefaultSampleFlags);
    }

    updateSize(bitstr);
}

void TrackFragmentHeaderBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    mTrackId = bitstr.read32Bits();
    if ((getFlags() & TrackFragmentHeaderBox::BaseDataOffsetPresent) != 0)
    {
        mBaseDataOffset = bitstr.read64Bits();
    }
    if ((getFlags() & TrackFragmentHeaderBox::SampleDescriptionIndexPresent) != 0)
    {
        mSampleDescriptionIndex = bitstr.read32Bits();
    }
    if ((getFlags() & TrackFragmentHeaderBox::DefaultSampleDurationPresent) != 0)
    {
        mDefaultSampleDuration = bitstr.read32Bits();
    }
    if ((getFlags() & TrackFragmentHeaderBox::DefaultSampleSizePresent) != 0)
    {
        mDefaultSampleSize = bitstr.read32Bits();
    }
    if ((getFlags() & TrackFragmentHeaderBox::DefaultSampleFlagsPresent) != 0)
    {
        mDefaultSampleFlags = MOVIEFRAGMENTS::SampleFlags::read(bitstr);
    }
}
