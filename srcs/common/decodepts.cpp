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

#include "decodepts.hpp"
#include <algorithm>
#include <iterator>
#include "compositionoffsetbox.hpp"
#include "compositiontodecodebox.hpp"
#include "editbox.hpp"
#include "timetosamplebox.hpp"

DecodePts::DecodePts()
    : mEditListBox(nullptr)
    , mMovieTimescale(0)
    , mMediaTimescale(0)
    , mTimeToSampleBox(nullptr)
    , mCompositionOffsetBox(nullptr)
    , mCompositionToDecodeBox(nullptr)
    , mTrackRunBox(nullptr)
    , mMovieOffset(0)
{
}

template <typename T>
void DecodePts::applyEdit(T& entry)
{
    if (entry.mMediaTime == -1)
    {
        applyEmptyEdit(entry);
    }
    if (entry.mMediaRateInteger == 0)
    {
        applyDwellEdit(entry);
    }
    if (entry.mMediaTime >= 0 && entry.mMediaRateInteger != 0)
    {
        applyShiftEdit(entry);
    }
}

template <typename T>
std::uint64_t DecodePts::fromMovieToMediaTS(T movieTS) const
{
    if (mMovieTimescale)
    {
        return static_cast<std::uint64_t>(movieTS) * mMediaTimescale / mMovieTimescale;
    }
    else
    {
        return static_cast<std::uint64_t>(movieTS);
    }
}

template <typename T>
void DecodePts::applyEmptyEdit(T& entry)
{
    mMovieOffset += fromMovieToMediaTS(entry.mSegmentDuration);
}

template <typename T>
void DecodePts::applyDwellEdit(T& entry)
{
    using PMapIt = PMap::iterator;

    std::pair<PMapIt, PMapIt> bound;
    bound = mMediaPtsTS.equal_range(entry.mMediaTime);

    if (bound.first != mMediaPtsTS.end())
    {
        // If lower bound and upper bound point to the same entry in the map the
        // previous sample in the map needs dwell.
        if ((bound.second != mMediaPtsTS.end()) && (bound.first->first == bound.second->first))
        {
            mMoviePtsTS.insert(std::make_pair(mMovieOffset, std::prev(bound.first)->second));
            mMovieOffset += fromMovieToMediaTS(entry.mSegmentDuration);
        }
        // if the lower and upper bound points to different entries in the map then
        // the first iterator points to the sample that needs dwell.
        else
        {
            mMoviePtsTS.insert(std::make_pair(mMovieOffset, bound.first->second));
            mMovieOffset += fromMovieToMediaTS(entry.mSegmentDuration);
        }
    }
}

std::uint64_t DecodePts::lastSampleDuration() const
{
    std::uint64_t lastSampleDuration{};
    if (mTrackRunBox)
    {
        auto& sampleDetails = mTrackRunBox->getSampleDetails();
        if (sampleDetails.size())
        {
            auto& last = *sampleDetails.rbegin();
            lastSampleDuration =
                mTrackRunBox->getVersion() == 0 ? last.version0.sampleDuration : last.version1.sampleDuration;
        }
    }
    else
    {
        const auto& sampleDeltas = mTimeToSampleBox->getSampleDeltas();
        if (sampleDeltas.size())
        {
            lastSampleDuration = *sampleDeltas.rbegin();
        }
    }
    return lastSampleDuration;
}

/// @todo This function can be optimized further using map ranges.
template <typename T>
void DecodePts::applyShiftEdit(T& entry)
{
    std::int64_t segmentEndTime(INT64_MAX);

    if (entry.mSegmentDuration != 0)
    {
        segmentEndTime = static_cast<std::int64_t>(static_cast<std::uint64_t>(entry.mMediaTime) +
                                                   (fromMovieToMediaTS(entry.mSegmentDuration)));
    }

    if (mMediaPtsTS.size())
    {
        // this may end up being "negative", but hopefully we'll find enough samples in the loop to come back to
        // "positive"
        mMovieOffset += static_cast<std::uint64_t>(mMediaPtsTS.begin()->first + mMediaOffset - entry.mMediaTime);
    }

    for (auto it = mMediaPtsTS.cbegin(); it != mMediaPtsTS.cend(); ++it)
    {
        // Find those samples that is presented in this edit
        if (it->first + mMediaOffset >= static_cast<std::int64_t>(entry.mMediaTime) &&
            it->first + mMediaOffset < segmentEndTime)
        {
            // If the pts first sample of this edit does not exactly fall in the
            // start of the edit, also include the previous sample and compute
            // the time for which that sample is displayed.
            if (it != mMediaPtsTS.cbegin() &&
                std::prev(it)->first + mMediaOffset < static_cast<std::int64_t>(entry.mMediaTime) &&
                it->first + mMediaOffset != static_cast<std::int64_t>(entry.mMediaTime))
            {
                mMoviePtsTS.insert(std::make_pair(mMovieOffset, std::prev(it)->second));
                mMovieOffset += static_cast<std::uint64_t>(
                    it->first - (std::prev(it)->first + (entry.mMediaTime - std::prev(it)->first)));
            }

            // Insert the rest of the samples into the movie edit
            mMoviePtsTS.insert(std::make_pair(mMovieOffset, it->second));
        }

        // If the next sample falls into the edit, update mMovieOffset with
        // the difference of this sample and the next.
        if (std::next(it) != mMediaPtsTS.cend())
        {
            mMovieOffset += static_cast<std::uint64_t>(std::next(it)->first - it->first);
        }
        else
        {
            mMovieOffset += lastSampleDuration();
        }
    }

    if (mMediaPtsTS.size() && entry.mSegmentDuration != 0)
    {
        // do corresponding fix at the end of the media
        mMovieOffset -= static_cast<std::uint64_t>(mMediaPtsTS.rbegin()->first + mMediaOffset +
                                                   static_cast<std::int64_t>(lastSampleDuration()) - segmentEndTime);
    }
}

void DecodePts::loadBox(const TimeToSampleBox* timeToSampleBox)
{
    mTimeToSampleBox = timeToSampleBox;
}

void DecodePts::loadBox(const CompositionOffsetBox* compositionOffsetBox)
{
    mCompositionOffsetBox = compositionOffsetBox;
}

void DecodePts::loadBox(const CompositionToDecodeBox* compositionToDecodeBox)
{
    mCompositionToDecodeBox = compositionToDecodeBox;
}

void DecodePts::loadBox(const EditListBox* editListBox, std::uint32_t movieTimescale, std::uint32_t mediaTimescale)
{
    mEditListBox    = editListBox;
    mMovieTimescale = movieTimescale;
    mMediaTimescale = mediaTimescale;
}

void DecodePts::loadBox(const TrackRunBox* trackRunBox)
{
    mTrackRunBox = trackRunBox;
}

void DecodePts::applyEditList()
{
    if (mMediaPtsTS.size())
    {
        std::uint32_t version = mEditListBox->getVersion();
        for (std::uint32_t i = 0; i < mEditListBox->numEntry(); i++)
        {
            switch (version)
            {
            case 0:
                applyEdit(mEditListBox->getEntry<EditListBox::EntryVersion0>(i));
                break;
            case 1:
                applyEdit(mEditListBox->getEntry<EditListBox::EntryVersion1>(i));
                break;
            }
        }
    }
}

bool DecodePts::unravel()
{
    bool success = true;

    /** @todo Currently it has been assumed that only version 0 entries are used
     * in the time to sample box. To be more generic the getSampleTimes method
     * in TimeToSample box has to be templated. */

    // First fetch the decode time stamps
    Vector<std::uint32_t> mediaDtsTS;
    mediaDtsTS = mTimeToSampleBox->getSampleTimes();

    // If composition offset box is present then add the deltas to the decode
    // time stamps.
    Vector<std::int64_t> mediaPtsTS;
    mediaPtsTS.reserve(mediaDtsTS.size());
    if (mCompositionOffsetBox != nullptr)
    {
        Vector<std::int32_t> ptsDelta;
        ptsDelta = mCompositionOffsetBox->getSampleCompositionOffsets();

        if (ptsDelta.size() == mediaDtsTS.size())
        {
            for (size_t i = 0; i < mediaDtsTS.size(); i++)
            {
                mediaPtsTS.push_back(std::int64_t(std::int32_t(mediaDtsTS.at(i)) + ptsDelta.at(i)));
            }
        }
        else
        {
            success = false;
        }
    }
    // Else the presentation time stamp is the decoding time stamp
    else
    {
        // This is done to convert a vector of one type to a vector of another type.
        std::copy(mediaDtsTS.begin(), mediaDtsTS.end(), std::back_inserter(mediaPtsTS));
    }

    if (success)
    {
        std::uint64_t sampleId = 0;
        for (auto pts : mediaPtsTS)
        {
            mMediaPtsTS.insert(std::make_pair(pts, sampleId++));
        }

        if (mEditListBox != nullptr)
        {
            applyEditList();
        }
        else
        {
            mMoviePtsTS = mMediaPtsTS;

            if (mMoviePtsTS.size() > 0)
            {
                auto last    = std::prev(mMoviePtsTS.end(), 1);
                mMovieOffset = static_cast<std::uint64_t>(last->first) + lastSampleDuration();
            }
            else
            {
                mMovieOffset = 0;
            }
        }
    }

    return success;
}

void DecodePts::unravelTrackRun()
{
    // First fetch the decode time stamps
    Vector<std::uint32_t> mediaDts;
    uint32_t time                     = 0;
    bool processCompositionTimeOffset = false;
    Vector<std::int32_t> ptsDelta;

    if ((mTrackRunBox->getFlags() & TrackRunBox::SampleCompositionTimeOffsetsPresent) != 0)
    {
        processCompositionTimeOffset = true;
    }

    const auto& sampleDetails = mTrackRunBox->getSampleDetails();
    mediaDts.reserve(sampleDetails.size());
    if (processCompositionTimeOffset)
    {
        ptsDelta.reserve(sampleDetails.size());
    }
    for (const auto& sample : sampleDetails)
    {
        mediaDts.push_back(time);
        time += sample.version0.sampleDuration;

        if (processCompositionTimeOffset)
        {
            if (mTrackRunBox->getVersion() == 0)
            {
                ptsDelta.push_back(static_cast<std::int32_t>(sample.version0.sampleCompositionTimeOffset));
            }
            else
            {
                ptsDelta.push_back(sample.version1.sampleCompositionTimeOffset);
            }
        }
    }

    Vector<std::int64_t> mediaPts;
    mediaPts.reserve(mediaDts.size());
    if (processCompositionTimeOffset)
    {
        for (size_t i = 0; i < mediaPts.size(); i++)
        {
            mediaPts.push_back(std::int64_t(std::int32_t(mediaDts.at(i)) + ptsDelta.at(i)));
        }
    }
    else
    {
        std::copy(mediaDts.begin(), mediaDts.end(), std::back_inserter(mediaPts));
    }

    // Link the presentation times to the sampleIds that are presented
    std::uint64_t sampleId = 0;
    mMediaPtsTS.reserve(mMediaPtsTS.size() + mediaPts.size());
    for (auto pts : mediaPts)
    {
        mMediaPtsTS.insert(std::make_pair(pts, sampleId++));
    }
}

void DecodePts::applyLocalTime(std::uint64_t ptsOffset)
{
    if (mEditListBox != nullptr)
    {
        mMediaOffset = static_cast<std::int64_t>(ptsOffset);
        applyEditList();
    }
    else
    {
        for (const auto& entry : mMediaPtsTS)
        {
            mMoviePtsTS.insert(std::make_pair(PresentationTimeTS(ptsOffset) + entry.first, entry.second));
        }

        if (mMoviePtsTS.size() > 0)
        {
            auto last    = std::prev(mMoviePtsTS.end(), 1);
            mMovieOffset = static_cast<std::uint64_t>(last->first) + lastSampleDuration();
        }
        else
        {
            mMovieOffset = 0;
        }
    }
}

DecodePts::PMap DecodePts::getTime(const std::uint32_t timeScale) const
{
    if (timeScale == 0)
    {
        throw RuntimeError("DecodePts::getTime: timeScale == 0");
    }
    PMap pMap;
    for (const auto& entry : mMoviePtsTS)
    {
        pMap.insert(std::make_pair(((entry.first * 1000) / timeScale), entry.second));
    }
    return pMap;
}

DecodePts::PMapTS DecodePts::getTimeTS() const
{
    PMapTS pMapTS;
    for (const auto& entry : mMoviePtsTS)
    {
        pMapTS.insert(std::make_pair(entry.first, entry.second));
    }
    return pMapTS;
}

void DecodePts::getTimeTrackRun(const std::uint32_t timeScale, PMap& oldPMap) const
{
    if (timeScale == 0)
    {
        throw RuntimeError("DecodePts::getTimeTrackRun: timeScale == 0");
    }
    std::uint64_t sampleIndexBase = 0;
    if (oldPMap.size())
    {
        sampleIndexBase = oldPMap.rbegin()->second + 1;
    }
    for (const auto& entry : mMoviePtsTS)
    {
        oldPMap.insert(std::make_pair(((entry.first * 1000) / timeScale), sampleIndexBase + entry.second));
    }
}

void DecodePts::getTimeTrackRunTS(PMapTS& oldPMapTS) const
{
    std::uint64_t sampleIndexBase = 0;
    if (oldPMapTS.size())
    {
        sampleIndexBase = oldPMapTS.rbegin()->second + 1;
    }
    for (const auto& entry : mMoviePtsTS)
    {
        oldPMapTS.insert(std::make_pair(entry.first, sampleIndexBase + entry.second));
    }
}

std::uint64_t DecodePts::getSpan() const
{
    return mMovieOffset;
}
