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

#include <Track.h>

namespace HEIFPP
{
    class VideoTrack : public Track
    {
    public:
        VideoTrack(Heif* aHeif);
        ~VideoTrack();

        void addSample(VideoSample* aSample);
        void setSample(std::uint32_t, VideoSample* aSample);
        void setSample(VideoSample* aOldSample, VideoSample* aNewSample);

        void setDisplayWidth(std::uint32_t) const;
        void setDisplayHeight(std::uint32_t) const;
        std::uint32_t getDisplayWidth() const;
        std::uint32_t getDisplayHeight() const;

        void setMatrix(std::uint32_t aMatrix[9]);
        void getMatrix(std::uint32_t aMatrix[9]);

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::SequenceId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;
        std::int32_t mMatrix[9];
        std::uint32_t mWidth, mHeight;

    private:
        VideoTrack& operator=(const VideoTrack&) = delete;
        VideoTrack& operator=(VideoTrack&&)      = delete;
        VideoTrack(const VideoTrack&)            = delete;
        VideoTrack(VideoTrack&&)                 = delete;
        VideoTrack()                             = delete;
    };
}  // namespace HEIFPP
