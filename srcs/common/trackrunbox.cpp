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

#include "trackrunbox.hpp"
#include <stdexcept>

TrackRunBox::TrackRunBox(uint8_t version, std::uint32_t tr_flags)
    : FullBox("trun", version, tr_flags)
    , mSampleDefaultsSet(false)
    , mSampleDefaults()
    , mSampleCount(0)
    , mDataOffset(0)
    , mFirstSampleFlags()
    , mSampleDetails()
{
}

void TrackRunBox::setSampleCount(const uint32_t sampleCount)
{
    mSampleCount = sampleCount;
}

uint32_t TrackRunBox::getSampleCount() const
{
    return mSampleCount;
}

void TrackRunBox::setDataOffset(const int32_t dataOffset)
{
    mDataOffset = dataOffset;
    setFlags(getFlags() | TrackRunFlags::DataOffsetPresent);
}

int32_t TrackRunBox::getDataOffset() const
{
    if ((getFlags() & TrackRunFlags::DataOffsetPresent) != 0)
    {
        return mDataOffset;
    }
    else
    {
        throw RuntimeError("TrackRunBox::getDataOffset() according to flags DataOffsetPresent not present.");
    }
}

void TrackRunBox::setFirstSampleFlags(const MOVIEFRAGMENTS::SampleFlags firstSampleFlags)
{
    mFirstSampleFlags = firstSampleFlags;
    setFlags(getFlags() | TrackRunFlags::FirstSampleFlagsPresent);
}

MOVIEFRAGMENTS::SampleFlags TrackRunBox::getFirstSampleFlags() const
{
    if ((getFlags() & TrackRunFlags::FirstSampleFlagsPresent) != 0)
    {
        return mFirstSampleFlags;
    }
    else
    {
        throw RuntimeError(
            "TrackRunBox::getFirstSampleFlags() according to flags FirstSampleFlagsPresent not present.");
    }
}

void TrackRunBox::addSampleDetails(SampleDetails sampleDetails)
{
    mSampleDetails.push_back(sampleDetails);
}

const Vector<TrackRunBox::SampleDetails>& TrackRunBox::getSampleDetails() const
{
    return mSampleDetails;
}

void TrackRunBox::setSampleDefaults(MOVIEFRAGMENTS::SampleDefaults& sampleDefaults)
{
    mSampleDefaultsSet = true;
    mSampleDefaults    = sampleDefaults;
}

void TrackRunBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mSampleCount);
    if ((getFlags() & TrackRunFlags::DataOffsetPresent) != 0)
    {
        bitstr.write32Bits(static_cast<uint32_t>(mDataOffset));
    }
    if ((getFlags() & TrackRunFlags::FirstSampleFlagsPresent) != 0)
    {
        MOVIEFRAGMENTS::SampleFlags::write(bitstr, mFirstSampleFlags);
    }

    for (uint32_t i = 0; i < mSampleCount; i++)
    {
        if ((getFlags() & TrackRunFlags::SampleDurationPresent) != 0)
        {
            bitstr.write32Bits(mSampleDetails.at(i).version0.sampleDuration);
        }
        if ((getFlags() & TrackRunFlags::SampleSizePresent) != 0)
        {
            bitstr.write32Bits(mSampleDetails.at(i).version0.sampleSize);
        }
        if ((getFlags() & TrackRunFlags::FirstSampleFlagsPresent) == 0)
        {
            if ((getFlags() & TrackRunFlags::SampleFlagsPresent) != 0)
            {
                MOVIEFRAGMENTS::SampleFlags::write(bitstr, mSampleDetails.at(i).version0.sampleFlags);
            }
        }
        if ((getFlags() & TrackRunFlags::SampleCompositionTimeOffsetsPresent) != 0)
        {
            if (getVersion() == 0)
            {
                bitstr.write32Bits(mSampleDetails.at(i).version0.sampleCompositionTimeOffset);
            }
            else
            {
                bitstr.write32Bits(static_cast<uint32_t>(mSampleDetails.at(i).version1.sampleCompositionTimeOffset));
            }
        }
    }
    updateSize(bitstr);
}

void TrackRunBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    mSampleCount = bitstr.read32Bits();
    if (mSampleCount > MP4VR_ABSOLUTE_MAX_SAMPLE_COUNT)
    {
        throw RuntimeError("Over max sample counts from TrackRunBox::parseBox");
    }
    if ((getFlags() & TrackRunFlags::DataOffsetPresent) != 0)
    {
        mDataOffset = static_cast<int32_t>(bitstr.read32Bits());
    }
    if ((getFlags() & TrackRunFlags::FirstSampleFlagsPresent) != 0)
    {
        mFirstSampleFlags = MOVIEFRAGMENTS::SampleFlags::read(bitstr);
    }

    SampleDetails sampleDetails;
    for (uint32_t i = 0; i < mSampleCount; i++)
    {
        if (mSampleDefaultsSet)
        {
            sampleDetails.version0.sampleDuration          = mSampleDefaults.defaultSampleDuration;
            sampleDetails.version0.sampleSize              = mSampleDefaults.defaultSampleSize;
            sampleDetails.version0.sampleFlags.flagsAsUInt = mSampleDefaults.defaultSampleFlags.flagsAsUInt;
        }
        else
        {
            // these should never be used if right boxes are present.
            sampleDetails.version0.sampleDuration          = 0;
            sampleDetails.version0.sampleSize              = 0;
            sampleDetails.version0.sampleFlags.flagsAsUInt = 0;
        }

        if ((getFlags() & TrackRunFlags::SampleDurationPresent) != 0)
        {
            sampleDetails.version0.sampleDuration = bitstr.read32Bits();
        }

        if ((getFlags() & TrackRunFlags::SampleSizePresent) != 0)
        {
            sampleDetails.version0.sampleSize = bitstr.read32Bits();
        }

        if ((getFlags() & TrackRunFlags::FirstSampleFlagsPresent) != 0)
        {
            sampleDetails.version0.sampleFlags.flagsAsUInt = mFirstSampleFlags.flagsAsUInt;

            // Treat the remaining samples as non-sync samples
            if (i > 0)
            {
                sampleDetails.version0.sampleFlags.flags.sample_is_non_sync_sample = 1;
            }
        }
        else if ((getFlags() & TrackRunFlags::SampleFlagsPresent) != 0)
        {
            sampleDetails.version0.sampleFlags = MOVIEFRAGMENTS::SampleFlags::read(bitstr);
        }

        if ((getFlags() & TrackRunFlags::SampleCompositionTimeOffsetsPresent) != 0)
        {
            if (getVersion() == 0)
            {
                sampleDetails.version0.sampleCompositionTimeOffset = bitstr.read32Bits();
            }
            else
            {
                sampleDetails.version1.sampleCompositionTimeOffset = static_cast<int32_t>(bitstr.read32Bits());
            }
        }
        else
        {
            if (getVersion() == 0)
            {
                sampleDetails.version0.sampleCompositionTimeOffset = 0;
            }
            else
            {
                sampleDetails.version1.sampleCompositionTimeOffset = 0;
            }
        }
        mSampleDetails.push_back(sampleDetails);
    }
}
