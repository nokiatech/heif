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

#include "audiosampleentrybox.hpp"
#include <stdexcept>
#include <string>

AudioSampleEntryBox::AudioSampleEntryBox(FourCCInt codingname)
    : SampleEntryBox(codingname)
    , mVersion(0)
    , mChannelCount(0)
    , mSampleSize(0)
    , mSampleRate(0)
    , mHasChannelLayoutBox(false)
    , mHasSamplingRateBox(false)
    , mChannelLayoutBox()
    , mSamplingRateBox()
{
}

AudioSampleEntryBox::AudioSampleEntryBox(const AudioSampleEntryBox& box)
    : SampleEntryBox(box)
    , mVersion(box.mVersion)
    , mChannelCount(box.mChannelCount)
    , mSampleSize(box.mSampleSize)
    , mSampleRate(box.mSampleRate)
    , mHasChannelLayoutBox(box.mHasChannelLayoutBox)
    , mHasSamplingRateBox(box.mHasSamplingRateBox)
    , mChannelLayoutBox(box.mChannelLayoutBox)
    , mSamplingRateBox(box.mSamplingRateBox)
{
}


void AudioSampleEntryBox::setVersion(std::uint16_t version)
{
    if (version == 0 || version == 1)
    {
        mVersion = version;
    }
    else
    {
        throw RuntimeError("AudioSampleEntryBox::setVersion Error: trying to set value other than 0 or 1");
    }
}

std::uint16_t AudioSampleEntryBox::getVersion() const
{
    return mVersion;
}

std::uint16_t AudioSampleEntryBox::getChannelCount() const
{
    return mChannelCount;
}

void AudioSampleEntryBox::setChannelCount(std::uint16_t channelcount)
{
    mChannelCount = channelcount;
}

std::uint16_t AudioSampleEntryBox::getSampleSize() const
{
    return mSampleSize;
}

void AudioSampleEntryBox::setSampleSize(std::uint16_t samplesize)
{
    mSampleSize = samplesize;
}

std::uint32_t AudioSampleEntryBox::getSampleRate() const
{
    if (mVersion == 1 && mHasSamplingRateBox)
    {
        return mSamplingRateBox.getSamplingRate();
    }
    else
    {
        return mSampleRate;
    }
}

void AudioSampleEntryBox::setSampleRate(std::uint32_t samplerate)
{
    mSampleRate = samplerate;
}

bool AudioSampleEntryBox::hasChannelLayoutBox()
{
    return mHasChannelLayoutBox;
}

ChannelLayoutBox& AudioSampleEntryBox::getChannelLayoutBox()
{
    return mChannelLayoutBox;
}

void AudioSampleEntryBox::setChannelLayoutBox(ChannelLayoutBox& channelLayoutBox)
{
    mChannelLayoutBox    = channelLayoutBox;
    mHasChannelLayoutBox = true;
}

bool AudioSampleEntryBox::hasSamplingRateBox()
{
    if (mVersion == 1)
    {
        return mHasSamplingRateBox;
    }
    else
    {
        return false;
    }
}

SamplingRateBox& AudioSampleEntryBox::getSamplingRateBox()
{
    if (mVersion == 1)
    {
        return mSamplingRateBox;
    }
    else
    {
        throw RuntimeError(
            "AudioSampleEntryBox::getSamplingRateBox Error: trying to getSamplingRateBox from version other than 1");
    }
}

void AudioSampleEntryBox::setSamplingRateBox(SamplingRateBox& samplingRateBox)
{
    this->setVersion(1);
    mSamplingRateBox    = samplingRateBox;
    mHasSamplingRateBox = true;
}

void AudioSampleEntryBox::writeBox(BitStream& bitstr) const
{
    SampleEntryBox::writeBox(bitstr);

    if (mVersion == 1)
    {
        bitstr.write16Bits(mVersion);
        bitstr.write16Bits(0);  // reserved = 0
    }
    else
    {
        bitstr.write32Bits(0);  // reserved = 0
    }
    bitstr.write32Bits(0);                  // reserved = 0
    bitstr.write16Bits(mChannelCount);      // number of channels 1 (mono) or 2 (stereo)
    bitstr.write16Bits(mSampleSize);        // in bits and takes default value of 16
    bitstr.write16Bits(0);                  // pre_defined = 0
    bitstr.write16Bits(0);                  // reserved = 0
    bitstr.write32Bits(mSampleRate << 16);  // 32bit field expressed as 16.16 fixed-point number (hi.lo)

    if (mVersion == 1)
    {
        if (mHasSamplingRateBox)
        {
            mSamplingRateBox.writeBox(bitstr);
        }
    }

    if (mHasChannelLayoutBox)
    {
        mChannelLayoutBox.writeBox(bitstr);
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void AudioSampleEntryBox::parseBox(BitStream& bitstr)
{
    SampleEntryBox::parseBox(bitstr);

    mVersion = bitstr.read16Bits();  // in case of v0 box this is first half of 32bit reserved = 0
    if (mVersion != 1 && mVersion != 0)
    {
        throw RuntimeError("AudioSampleEntryV1Box::parseBox Error: trying to read version other than 0 or 1");
    }

    bitstr.read16Bits();                        // reserved = 0 (other half of 32bits in v0 audio sample entry
    bitstr.read32Bits();                        // reserved = 0
    mChannelCount = bitstr.read16Bits();        // number of channels 1 (mono) or 2 (stereo)
    mSampleSize   = bitstr.read16Bits();        // in bits and takes default value of 16
    bitstr.read16Bits();                        // pre_defined = 0
    bitstr.read16Bits();                        // reserved = 0
    mSampleRate = (bitstr.read32Bits() >> 16);  // 32bit field expressed as 16.16 fixed-point number (hi.lo)

    std::uint64_t revertOffset = ~0u;

    // read optional boxes if present - break out if 'esds' found instead.
    while (bitstr.numBytesLeft() > 0)
    {
        const std::uint64_t startOffset = bitstr.getPos();
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        if (mVersion == 1 && boxType == "srat")
        {
            mHasSamplingRateBox = true;
            mSamplingRateBox.parseBox(subBitstr);
        }
        else if (boxType == "chnl")
        {
            mHasChannelLayoutBox = true;
            mChannelLayoutBox.setChannelCount(mChannelCount);
            mChannelLayoutBox.parseBox(subBitstr);
        }
        else if (boxType == "esds")
        {
            // we need to reset bitstream position as esds belongs to box extending this box
            revertOffset = startOffset;
        }
        // unsupported boxes are skipped
    }

    if (revertOffset != ~0u)
    {
        bitstr.setPosition(revertOffset);
    }
}

AudioSampleEntryBox* AudioSampleEntryBox::clone() const
{
    return nullptr;
}

bool AudioSampleEntryBox::isVisual() const
{
    return false;
}