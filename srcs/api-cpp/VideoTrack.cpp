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

#include "VideoTrack.h"
#include "DecoderConfiguration.h"
#include "MetaItem.h"
#include "VideoSample.h"
#include "heifreader.h"
#include "heifwriter.h"

using namespace HEIFPP;


VideoTrack::VideoTrack(Heif* aHeif)
    : Track(aHeif)
    , mMatrix{0x10000, 0, 0, 0, 0x10000, 0, 0, 0, 0x40000000}
    , mWidth(0)
    , mHeight(0)
{
    mHandler = HEIF::FourCC("vide");
}
VideoTrack::~VideoTrack()
{
}

std::uint32_t VideoTrack::getDisplayWidth() const
{
    return mWidth;
}
std::uint32_t VideoTrack::getDisplayHeight() const
{
    return mHeight;
}


HEIF::ErrorCode VideoTrack::load(HEIF::Reader* aReader, const HEIF::SequenceId& aId)
{
    HEIF::ErrorCode ret = Track::load(aReader, aId);
    aReader->getDisplayWidth(aId, mWidth);
    aReader->getDisplayHeight(aId, mHeight);
    HEIF::Array<std::int32_t> mat;
    aReader->getMatrix(aId, mat);
    for (size_t i = 0; i < 9; i++)
    {
        mMatrix[i] = mat[i];
    }
    return ret;
}
HEIF::ErrorCode VideoTrack::save(HEIF::Writer* aWriter)
{
    HEIF::Rational tb;
    HEIF::ErrorCode err;
    tb.num = 1;
    tb.den = mTimeScale;
    if (mId == Heif::InvalidSequence)
    {
        err = aWriter->addVideoTrack(tb, mId);
    }
    HEIF::Array<int32_t> matrix(9);
    for (size_t i = 0; i < 9; i++)
    {
        matrix[i] = mMatrix[i];
    }
    err = aWriter->setMatrix(mId, matrix);

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
void VideoTrack::addSample(VideoSample* aSample)
{
    Track::addSample(aSample);
}
void VideoTrack::setSample(std::uint32_t aIndex, VideoSample* aSample)
{
    Track::setSample(aIndex, aSample);
}
void VideoTrack::setSample(VideoSample* aOldSample, VideoSample* aNewSample)
{
    Track::setSample(aOldSample, aNewSample);
}
