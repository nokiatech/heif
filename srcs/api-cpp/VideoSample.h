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
    class VideoSample : public Sample
    {
    public:
        VideoSample(Heif* aHeif);
        std::uint32_t getWidth() const;
        std::uint32_t getHeight() const;

    private:
        // serialization methods.
        HEIF::ErrorCode load(HEIF::Reader* aReader,
                             const HEIF::SequenceId& aTrack,
                             const HEIF::SampleInformation& aInfo) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;
        std::uint32_t mWidth, mHeight;

    private:
        VideoSample& operator=(const VideoSample&) = delete;
        VideoSample& operator=(VideoSample&&)      = delete;
        VideoSample(const VideoSample&)            = delete;
        VideoSample(VideoSample&&)                 = delete;
        VideoSample()                              = delete;
    };
}  // namespace HEIFPP
