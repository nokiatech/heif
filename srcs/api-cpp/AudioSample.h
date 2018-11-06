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

#pragma once

#include <Sample.h>

namespace HEIFPP
{
    class AudioSample : public Sample
    {
    public:
        AudioSample(Heif* aHeif);

    private:
        // serialization methods.
        HEIF::ErrorCode load(HEIF::Reader* aReader,
                             const HEIF::SequenceId& aTrack,
                             const HEIF::SampleInformation& aInfo) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    private:
        AudioSample& operator=(const AudioSample&) = delete;
        AudioSample& operator=(AudioSample&&)      = delete;
        AudioSample(const AudioSample&)            = delete;
        AudioSample(AudioSample&&)                 = delete;
        AudioSample()                              = delete;
    };
}  // namespace HEIFPP