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

#include "VideoSample.h"
#include "heifreader.h"

using namespace HEIFPP;


VideoSample::VideoSample(Heif* aHeif)
    : Sample(aHeif)
    , mWidth(0)
    , mHeight(0)
{
    mIsVideo = true;
}

HEIF::ErrorCode VideoSample::load(HEIF::Reader* aReader,
                                  const HEIF::SequenceId& aTrackId,
                                  const HEIF::SampleInformation& aInfo)
{
    HEIF::ErrorCode errorcode = HEIF::ErrorCode::OK;
    errorcode                 = Sample::load(aReader, aTrackId, aInfo);
    aReader->getWidth(aTrackId, aInfo.sampleId, mWidth);
    aReader->getHeight(aTrackId, aInfo.sampleId, mHeight);
    return errorcode;
}
HEIF::ErrorCode VideoSample::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error;
    error = Sample::save(aWriter);
    return error;
}

std::uint32_t VideoSample::getWidth() const
{
    return mWidth;
}
std::uint32_t VideoSample::getHeight() const
{
    return mHeight;
}
