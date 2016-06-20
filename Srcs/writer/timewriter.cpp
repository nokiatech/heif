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

#include "timewriter.hpp"
#include "compositionoffsetbox.hpp"
#include "compositiontodecodebox.hpp"
#include <algorithm>
#include <iterator>
#include <tuple>

TimeWriter::TimeWriter(std::uint32_t clockTicks) :
    mDisplayRate(0),
    mClockTicks(clockTicks),
    mDecodeStartTime(0),
    mDisplayStartTime(0),
    mIsCompositionOffsetBoxRequired(false),
    mIsCompositionToDecodeBoxRequired(false)
{
}


void TimeWriter::setDisplayRate(std::uint32_t displayRate)
{
    mDisplayRate = displayRate;
}


void TimeWriter::loadOrder(const std::vector<uint32_t>& decodeOrder, const std::vector<uint32_t>& displayOrder)
{
    mDecodeOrder = decodeOrder;
    mDisplayOrder = displayOrder;

    // Convert decode start time from milliseconds to units of clockticks
    mDecodeStartTime = (mDecodeStartTime * mClockTicks) / 1000;
    mDecodeTime = mDecodeOrder;
    std::for_each(mDecodeTime.begin(), mDecodeTime.end(), [this](std::uint32_t& decodeTime)
    {
        decodeTime *= (mClockTicks/mDisplayRate) + mDecodeStartTime;
    });

    // Convert display start time from milliseconds to units of clockticks
    mDisplayStartTime = (mDisplayStartTime * mClockTicks) / 1000;
    mDisplayTime = mDisplayOrder;
    std::for_each(mDisplayTime.begin(), mDisplayTime.end(), [this](std::uint32_t& displayTime)
    {
        displayTime *= (mClockTicks/mDisplayRate) + mDisplayStartTime;
    });

    std::transform(mDecodeTime.begin(), mDecodeTime.end(), mDisplayTime.begin(), std::back_inserter(mDisplayOffset),
        [](std::uint32_t decodeTime, std::uint32_t displayTime)
        {
            return (displayTime - decodeTime);
        });

    bool allZeros = std::all_of(mDisplayOffset.begin(), mDisplayOffset.end(), [](std::int32_t offset)
    {
        return (offset == 0);
    });
    mIsCompositionOffsetBoxRequired = (allZeros == true) ? false : true;

    bool allPositive = std::all_of(mDisplayOffset.begin(), mDisplayOffset.end(), [](std::int32_t offset)
    {
        return (offset >= 0);
    });
    mIsCompositionToDecodeBoxRequired = (allPositive == true) ? false : true;
}


void TimeWriter::fillTimeToSampleBox(TimeToSampleBox& timeToSampleBox)
{

    TimeToSampleBox::EntryVersion0& decodeDeltaEntryVersion0 = timeToSampleBox.getDecodeDeltaEntry();
    decodeDeltaEntryVersion0.mSampleCount = mDecodeOrder.size();
    decodeDeltaEntryVersion0.mSampleDelta = (mClockTicks / mDisplayRate);
}


bool TimeWriter::isCompositionOffsetBoxRequired() const
{
    return mIsCompositionOffsetBoxRequired;
}


bool TimeWriter::isCompositionToDecodeBoxRequired() const
{
    return mIsCompositionToDecodeBoxRequired;
}


void TimeWriter::fillCompositionOffsetBox(CompositionOffsetBox& compositionOffsetBox)
{
    // If CompositionToDecodeBox is required it means that there are negative
    // display offset times.
    if (mIsCompositionToDecodeBoxRequired == true)
    {
        // Compute the (offset, run) pairs
        std::vector<std::tuple<std::int32_t, std::uint32_t>> entryList;
        std::int32_t prevOffset = mDisplayOffset[0];
        std::uint32_t offsetRuns = 0;
        for (auto currOffset : mDisplayOffset)
        {
            if (currOffset == prevOffset)
            {
                offsetRuns += 1;
            }
            else
            {
                entryList.push_back(std::make_tuple(prevOffset, offsetRuns));
                prevOffset = currOffset;
                offsetRuns = 1;
            }
        }
        entryList.push_back(std::make_tuple(prevOffset, offsetRuns));

        // Version 1 of the CompositionOffsetBox is used because of signed
        // offsets
        compositionOffsetBox.setVersion(1);

        // Fill entries into the box
        for (auto entry : entryList)
        {
            std::int32_t offset;
            std::uint32_t run;
            std::tie(offset, run) = entry;
            CompositionOffsetBox::EntryVersion1 offsetEntry;
            offsetEntry.mSampleCount = run;
            offsetEntry.mSampleOffset = offset;
            compositionOffsetBox.addCompositionOffsetEntryVersion1(offsetEntry);
        }
    }
    else
    {
        // Compute the (offset, run) pairs
        std::vector<std::tuple<std::int32_t, std::uint32_t>> entryList;
        std::uint32_t prevOffset = mDisplayOffset[0];
        std::uint32_t offsetRuns = 0;
        for (std::uint32_t currOffset : mDisplayOffset)
        {
            if (currOffset == prevOffset)
            {
                offsetRuns += 1;
            }
            else
            {
                entryList.push_back(std::make_tuple(prevOffset, offsetRuns));
                prevOffset = currOffset;
                offsetRuns = 1;
            }
        }
        entryList.push_back(std::make_tuple(prevOffset, offsetRuns));

        // Fill entries into the box
        for (auto entry : entryList)
        {
            std::uint32_t offset;
            std::uint32_t run;
            std::tie(offset, run) = entry;
            CompositionOffsetBox::EntryVersion0 offsetEntry;
            offsetEntry.mSampleCount = run;
            offsetEntry.mSampleOffset = offset;
            compositionOffsetBox.addCompositionOffsetEntryVersion0(offsetEntry);
        }
    }
}


void TimeWriter::fillCompositionToDecodeBox(CompositionToDecodeBox& compositionToDecodeBox)
{
    std::int32_t compositionToDtsShift = *std::min_element(mDisplayOffset.cbegin(), mDisplayOffset.cend());
    std::int32_t leastDecodeToDisplayDelta = *std::min_element(mDisplayOffset.cbegin(), mDisplayOffset.cend());
    std::int32_t greatestDecodeToDisplayDelta = *std::max_element(mDisplayOffset.cbegin(), mDisplayOffset.cend());
    std::int32_t compositionEndTime = 0;

    compositionToDecodeBox.setCompositionToDtsShift(compositionToDtsShift);
    compositionToDecodeBox.setLeastDecodeToDisplayDelta(leastDecodeToDisplayDelta);
    compositionToDecodeBox.setGreatestDecodeToDisplayDelta(greatestDecodeToDisplayDelta);
    compositionToDecodeBox.setCompositionStartTime(mDisplayStartTime);
    compositionToDecodeBox.setCompositionEndTime(compositionEndTime);
}
