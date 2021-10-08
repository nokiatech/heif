/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
#include <iostream>
#include <iterator>
#include <limits>

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
    if (entry.mMediaRateInteger == 0 && entry.mMediaRateFraction == 0)
    {
        applyDwellEdit(entry);
    }
    if (entry.mMediaTime >= 0 &&
        (entry.mMediaRateInteger == 1 || (entry.mMediaRateInteger == 0 && entry.mMediaRateFraction > 0)))
    {
        applyShiftEditForward(entry);
    }
    if (entry.mMediaTime >= 0 && entry.mMediaRateInteger == -1)
    {
        applyShiftEditReverse(entry);
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
void DecodePts::applyShiftEditForward(T& entry)
{
    auto segmentBeginTime = static_cast<std::int64_t>(entry.mMediaTime);
    std::int64_t segmentEndTime(std::numeric_limits<std::int64_t>::max());

    // Adjust each inserted sample's time ratio by this amount.
    // ie. entry.mMediaTime == half of (its maximum value + 1) (+1 is handled by the value of .mMediaRateInteger being
    // 1) then the duration is each sample's is doubled, ie. sampleTimeRatio == 2.0
    const auto sampleTimeRatio =
        (entry.mMediaRateInteger == 1)
            ? 1.0
            : (static_cast<double>(std::numeric_limits<std::int16_t>::max()) + 1) / entry.mMediaRateFraction;

    if (entry.mSegmentDuration != 0)
    {
        segmentBeginTime = entry.mMediaTime;
        segmentEndTime =
            entry.mMediaTime + static_cast<std::int64_t>(fromMovieToMediaTS(entry.mSegmentDuration / sampleTimeRatio));
    }

    std::int64_t lastInsertedSampleT1 = segmentBeginTime;

    for (auto it = mMediaPtsTS.begin(); it != mMediaPtsTS.end(); ++it)
    {
        std::int64_t sampleDuration = std::next(it) == mMediaPtsTS.end()
                                          ? static_cast<std::int64_t>(lastSampleDuration())
                                          : std::next(it)->first - it->first;

        auto sampleId = it->second;
        auto sampleT0 = it->first + mMediaOffset;
        auto sampleT1 = sampleT0 + sampleDuration;

        if (sampleT0 >= segmentBeginTime)
        {
            if (sampleT0 < segmentEndTime)
            {
                std::int64_t insertedSampleDuration = 0;
                mMovieOffset += static_cast<std::uint64_t>(sampleTimeRatio * (sampleT0 - lastInsertedSampleT1));
                mMoviePtsTS.insert(std::make_pair(mMovieOffset, sampleId));
                if (sampleT1 <= segmentEndTime)
                {
                    insertedSampleDuration = sampleDuration;
                    lastInsertedSampleT1   = sampleT1;
                }
                else  // sampleT1 > segmentEndTime; we need to cut it from the end
                {
                    insertedSampleDuration = segmentEndTime - sampleT0;
                    lastInsertedSampleT1   = segmentEndTime;
                }
                mMovieOffset += static_cast<std::uint64_t>(sampleTimeRatio * (insertedSampleDuration));
            }
            else
            {
                // ignore sample
            }
        }
        else  // sampleT0 < segmentBeginTime
        {
            if (sampleT1 > segmentBeginTime)
            {
                mMovieOffset += static_cast<std::uint64_t>(sampleTimeRatio * (segmentBeginTime - lastInsertedSampleT1));
                std::int64_t insertedSampleDuration = 0;
                mMoviePtsTS.insert(std::make_pair(mMovieOffset, sampleId));
                if (sampleT1 >= segmentEndTime)
                {
                    insertedSampleDuration = segmentEndTime - segmentBeginTime;
                    lastInsertedSampleT1   = segmentEndTime;
                }
                else
                {
                    insertedSampleDuration = sampleT1 - segmentBeginTime;
                    lastInsertedSampleT1   = sampleT1;
                }
                mMovieOffset += static_cast<std::uint64_t>(sampleTimeRatio * (insertedSampleDuration));
            }
            else
            {
                // ignore sample
            }
        }
    }
    if (entry.mSegmentDuration)
    {
        // gap the distance between last inserted sample and the segment end time
        mMovieOffset += static_cast<std::uint64_t>(sampleTimeRatio * (segmentEndTime - lastInsertedSampleT1));
    }
}

/// @todo This function can be optimized further using map ranges.
template <typename T>
void DecodePts::applyShiftEditReverse(T& entry)
{
    auto segmentBeginTime = static_cast<std::int64_t>(entry.mMediaTime);
    std::int64_t segmentEndTime(std::numeric_limits<std::int64_t>::max());

    std::int64_t lastInsertedSampleT0 = segmentBeginTime;
    if (entry.mSegmentDuration != 0)
    {
        segmentEndTime       = entry.mMediaTime;
        segmentBeginTime     = entry.mMediaTime - static_cast<std::int64_t>(fromMovieToMediaTS(entry.mSegmentDuration));
        lastInsertedSampleT0 = segmentEndTime;
    }
    else
    {
        if (mMediaPtsTS.size())
        {
            lastInsertedSampleT0 = mMediaPtsTS.back().first + static_cast<std::int64_t>(lastSampleDuration());
        }
    }

    for (auto it = mMediaPtsTS.rbegin(); it != mMediaPtsTS.rend(); ++it)
    {
        std::int64_t sampleDuration = it == mMediaPtsTS.rbegin() ? static_cast<std::int64_t>(lastSampleDuration())
                                                                 : std::prev(it)->first - it->first;

        auto sampleId = it->second;
        auto sampleT0 = it->first + mMediaOffset;
        auto sampleT1 = sampleT0 + sampleDuration;

        if (sampleT0 >= segmentBeginTime)
        {
            if (sampleT0 < segmentEndTime)
            {
                std::int64_t insertedSampleDuration = 0;
                mMoviePtsTS.insert(std::make_pair(mMovieOffset, sampleId));
                if (sampleT1 <= segmentEndTime)
                {
                    insertedSampleDuration = sampleDuration;
                    mMovieOffset += static_cast<std::uint64_t>(lastInsertedSampleT0 - sampleT1);
                }
                else  // sampleT1 > segmentEndTime; we need to cut it from the end
                {
                    auto cutSampleDuration = segmentEndTime - sampleT0;
                    insertedSampleDuration = cutSampleDuration;
                    mMovieOffset += static_cast<std::uint64_t>(lastInsertedSampleT0 - segmentEndTime);
                }
                lastInsertedSampleT0 = sampleT0;  // sampleT0 is within range -> just use that
                mMovieOffset += static_cast<std::uint64_t>(insertedSampleDuration);
            }
            else
            {
                // ignore sample
            }
        }
        else  // sampleT0 < segmentBeginTime
        {
            if (sampleT1 > segmentBeginTime)
            {
                std::int64_t insertedSampleDuration = 0;
                mMoviePtsTS.insert(std::make_pair(mMovieOffset, sampleId));
                if (sampleT1 >= segmentEndTime)
                {
                    mMovieOffset += static_cast<std::uint64_t>(lastInsertedSampleT0 - segmentEndTime);
                    insertedSampleDuration = segmentEndTime - segmentBeginTime;
                }
                else
                {
                    mMovieOffset += static_cast<std::uint64_t>(lastInsertedSampleT0 - sampleT1);
                    insertedSampleDuration = sampleT1 - segmentBeginTime;
                }
                // sampleT0 is before segmentBeginTime -> cap it to segmentBeginTime
                lastInsertedSampleT0 = segmentBeginTime;
                mMovieOffset += static_cast<std::uint64_t>(insertedSampleDuration);
            }
            else
            {
                // ignore sample
            }
        }
    }
    if (entry.mSegmentDuration)
    {
        // gap the distance between last inserted sample and the segment end time
        mMovieOffset += static_cast<std::uint64_t>(lastInsertedSampleT0 - segmentBeginTime);
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
            std::transform(mediaDtsTS.begin(), mediaDtsTS.end(), ptsDelta.begin(), std::back_inserter(mediaPtsTS),
                           [](std::uint64_t theMediaDts, std::int32_t thePtsDelta) {
                               return std::uint64_t(std::int32_t(theMediaDts) + thePtsDelta);
                           });
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
        std::transform(mediaDts.begin(), mediaDts.end(), ptsDelta.begin(), std::back_inserter(mediaPts),
                       [](std::uint32_t theMediaDts, std::int32_t thePtsDelta) {
                           return std::uint32_t(std::int32_t(theMediaDts) + thePtsDelta);
                       });
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
