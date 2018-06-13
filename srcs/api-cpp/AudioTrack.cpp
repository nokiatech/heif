/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#include "AudioTrack.h"
#include "AACDecoderConfiguration.h"
#include "AudioSample.h"
#include "MetaItem.h"
#include "heifreader.h"
#include "heifwriter.h"

using namespace HEIFPP;


AudioTrack::AudioTrack(Heif* aHeif)
    : Track(aHeif)
{
    mHandler = HEIF::FourCC("soun");
}
AudioTrack::~AudioTrack()
{
}


HEIF::ErrorCode AudioTrack::load(HEIF::Reader* aReader, const HEIF::SequenceId& aId)
{
    HEIF::ErrorCode ret = Track::load(aReader, aId);
    return ret;
}
HEIF::ErrorCode AudioTrack::save(HEIF::Writer* aWriter)
{
    HEIF::Rational tb;
    HEIF::ErrorCode err;
    tb.num = 1;
    tb.den = mTimeScale;

    HEIF::AudioParams config;
    config.maxBitrate     = 0;
    config.averageBitrate = 0;
    config.channelCount   = 0;
    config.sampleRate     = 0;

    // NOTE: AAC-LC only.
    auto conf = mSamples[0]->getDecoderConfiguration();
    if (conf->getMediaFormat() != HEIF::MediaFormat::AAC)
    {
        return HEIF::ErrorCode::INVALID_MEDIA_FORMAT;
    }
    AACDecoderConfiguration* aac = static_cast<AACDecoderConfiguration*>(conf);
    config.channelCount          = aac->getChannels();
    config.sampleRate            = aac->getSampleRate();

    // This is just a simple guestimate. slightly pessimistic, returns bitrates higher than actually needed.
    uint64_t time = 0, totalTime = 0;
    uint64_t bytes = 0, totalBytes = 0;
    float br  = 0;
    float max = 0;
    for (auto smpl : mSamples)
    {
        totalTime += smpl->getDuration();
        totalBytes += smpl->getSampleDataSize();
        bytes += smpl->getSampleDataSize();
        time += smpl->getDuration();
        if (time >= mTimeScale)
        {
            br = bytes / (time / (float) mTimeScale);  //~one second average.
            time -= mTimeScale;
            bytes = 0;
            if (br > max)
            {
                max = br;
            }
            bytes = smpl->getSampleDataSize();
        }
    }
    config.maxBitrate     = (std::uint32_t)(max * 8);
    config.averageBitrate = (std::uint32_t)(((totalBytes / (totalTime / ((float) mTimeScale))) * 8));

    err = aWriter->addAudioTrack(tb, config, mId);
    for (auto smpl : mSamples)
    {
        if (smpl->getId() == Heif::InvalidSequenceImage)
        {
            err = smpl->save(aWriter);
        }
        if (HEIF::ErrorCode::OK != err)
        {
            break;
        }
    }

    HEIF::ErrorCode ret = Track::save(aWriter);
    return ret;
}
void AudioTrack::addSample(AudioSample* aSample)
{
    Track::addSample(aSample);
}
void AudioTrack::setSample(std::uint32_t aIndex, AudioSample* aSample)
{
    Track::setSample(aIndex, aSample);
}
void AudioTrack::setSample(AudioSample* aOldSample, AudioSample* aNewSample)
{
    Track::setSample(aOldSample, aNewSample);
}
