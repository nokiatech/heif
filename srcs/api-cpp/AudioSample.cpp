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

#include "AudioSample.h"

using namespace HEIFPP;

AudioSample::AudioSample(Heif* aHeif)
    : Sample(aHeif)
{
    mIsAudio = true;
}

HEIF::ErrorCode AudioSample::load(HEIF::Reader* aReader,
                                  const HEIF::SequenceId& aTrackId,
                                  const HEIF::SampleInformation& aInfo)
{
    HEIF::ErrorCode errorcode = HEIF::ErrorCode::OK;
    errorcode                 = Sample::load(aReader, aTrackId, aInfo);
    return errorcode;
}
HEIF::ErrorCode AudioSample::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error;
    error = Sample::save(aWriter);
    return error;
}