/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#pragma once

#include <Track.h>
#include "heifcommondatatypes.h"
#include <cstdint>

namespace HEIFPP
{
    class AudioSample;

    class AudioTrack : public Track
    {
    public:
        explicit AudioTrack(Heif* aHeif);
        ~AudioTrack() override;
        void addSample(AudioSample* aSample);
        void setSample(std::uint32_t, AudioSample* aSample);
        void setSample(AudioSample* aOldSample, AudioSample* aNewSample);

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::SequenceId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    public:
        AudioTrack& operator=(const AudioTrack&) = delete;
        AudioTrack& operator=(AudioTrack&&) = delete;
        AudioTrack(const AudioTrack&)       = delete;
        AudioTrack(AudioTrack&&)            = delete;
        AudioTrack()                        = delete;
    };

}  // namespace HEIFPP
