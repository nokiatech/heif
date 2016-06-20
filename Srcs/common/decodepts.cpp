/* Copyright (c) 2015, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#include "decodepts.hpp"
#include "compositionoffsetbox.hpp"
#include "compositiontodecodebox.hpp"
#include "editbox.hpp"
#include "timetosamplebox.hpp"
#include <algorithm>
#include <iterator>

DecodePts::DecodePts() :
    mEditListBox(nullptr),
    mTimeToSampleBox(nullptr),
    mCompositionOffsetBox(nullptr),
    mCompositionToDecodeBox(nullptr),
    mMovieOffset(0)
{
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

void DecodePts::loadBox(const EditListBox* editListBox)
{
    mEditListBox = editListBox;
}

void DecodePts::unravel()
{
    /** @todo Currently it has been assumed that only version 0 entries are used
     * in the time to sample box. To be more generic the getSampleTimes method
     * in TimeToSample box has to be templated. */

    // First fetch the decode time stamps
    std::vector<std::uint32_t> mediaDts;
    mediaDts = mTimeToSampleBox->getSampleTimes();

    // If composition offset box is present then add the deltas to the decode
    // time stamps.
    std::vector<std::int64_t> mediaPts;
    if (mCompositionOffsetBox != nullptr)
    {
        std::vector<std::int32_t> ptsDelta;
        ptsDelta = mCompositionOffsetBox->getSampleCompositionOffsets();

        std::transform(mediaDts.begin(), mediaDts.end(), ptsDelta.begin(), std::back_inserter(mediaPts),
            [](std::uint32_t theMediaDts, std::int32_t thePtsDelta)
        {
            return (theMediaDts + thePtsDelta);
        });

    }
    // Else the presentation time stamp is the decoding time stamp
    else
    {
        // This is done to convert a vector of one type to a vector of another type.
        std::copy(mediaDts.begin(), mediaDts.end(), std::back_inserter(mediaPts));
    }

    // Link the presentation times to the sampleIds that are presented
    std::uint64_t sampleId = 0;
    for (auto pts : mediaPts)
    {
        mMediaPts.insert(std::make_pair(pts, sampleId++));
    }

    // If edits are present then apply the edits in order to make a movie timeline.
    if (mEditListBox != nullptr)
    {
        for (std::uint32_t i = 0; i < mEditListBox->numEntry(); i++)
        {
            std::uint32_t version = mEditListBox->getVersion();
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
    // Else the media presentation times are the movie presentation times.
    else
    {
        mMoviePts = mMediaPts;

        if (mMoviePts.size() > 1)
        {
            auto last = std::prev(mMoviePts.end(), 1);
            auto prev = std::prev(last, 1);
            mMovieOffset = last->first + (last->first - prev->first);
        }
        else
        {
            mMovieOffset = 0;
        }
    }
}

DecodePts::PMap DecodePts::getTime(const std::uint32_t timeScale) const
{
    PMap pMap;
    for (const auto& entry : mMoviePts)
    {
        pMap.insert(std::make_pair(((entry.first * 1000) / timeScale), entry.second));
    }
    return pMap;
}

DecodePts::PMap DecodePts::getTime(const std::uint32_t timeScale, const std::uint64_t trackDuration) const
{
    PMap pMap;
    std::uint64_t endTime = 0;
    std::uint32_t repNumb = 1;

    bool done = false;
    do
    {
        for (const auto& entry : mMoviePts)
        {
            pMap.insert(std::make_pair(((entry.first * 1000) / timeScale) + endTime, entry.second));
        }
        if (mMovieOffset == 0)
        {
            // There is only 1 sample probably or unravel hasn't been called for some reason.
            break;
        }
        endTime = ((mMovieOffset * 1000) / timeScale) * repNumb;
        repNumb = repNumb + 1;
        if (endTime >= trackDuration)
        {
            done = true;
        }
    }
    while(!done);
    return pMap;
}

std::uint64_t DecodePts::getSpan() const
{
    return mMovieOffset;
}

