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

#ifndef DECODEPTS_HPP
#define DECODEPTS_HPP

#include "compositionoffsetbox.hpp"
#include "compositiontodecodebox.hpp"
#include "editbox.hpp"
#include "timetosamplebox.hpp"
#include <algorithm>
#include <map>

/**
 * @brief Class for decoding presentation timestamps for samples of a TrackBox.
 * @todo CompositionToDecodeBox is currently ignored.
 */
class DecodePts
{
public:
    typedef std::int64_t PresentationTime; ///< Sample presentation time in milliseconds
    typedef std::uint64_t SampleIndex;     ///< 0-based sample index

    /** The PMap (Presentation Map) always uses 64 bit key-value regardless
     *  of which version of entries are used by the stts, and ctts boxes. */
    using PMap = std::map<PresentationTime, SampleIndex>;

public:
    DecodePts();
    virtual ~DecodePts() = default;

    /** Set TimeToSampleBox pointer to the class for later use
     * @todo Refactor to avoid saving raw pointers to the object */
    void loadBox(const TimeToSampleBox* timeToSampleBox);

    /** Set CompositionOffsetBox pointer to the class for later use
     * @todo Refactor to avoid saving raw pointers to the object */
    void loadBox(const CompositionOffsetBox* compositionOffsetBox);

    /** Set CompositionToDecodeBox pointer to the class for later use
     * @todo Refactor to avoid saving raw pointers to the object */
    void loadBox(const CompositionToDecodeBox* compositionToDecodeBox);

    /** Set EditListBox pointer to the class for later use
     * @todo Refactor to avoid saving raw pointers to the object */
    void loadBox(const EditListBox* editListBox);

    /**
     * @brief Generate presentation timestamps
     * @pre mTimeToSampleBox has been set
     * @pre mCompositionOffsetBox has been set if CompositionOffsetBox is present
     * @pre mEditListBox has been set if EditListBox is present
     */
    void unravel();

    /**
     * @brief Get duration of the track
     * @return Duration of the track considering possible edit list
     */
    std::uint64_t getSpan() const;

    /**
     * @brief
     * @param timeScale Timescale of the TrackBox
     * @return Presentation timestamps
     */
    PMap getTime(std::uint32_t timeScale) const;

    /**
     * @brief Generate timestamps based on edit list and the wanted total duration given as parameter.
     * @details Timestamps generated from the edit list are looped to fill time until trackDuration.
     * @param timeScale    Timescale of the TrackBox
     * @param trackDuration Wanted duration of the track
     * @return Presentation timestamps
     */
    PMap getTime(std::uint32_t timeScale, const std::uint64_t trackDuration) const;

private:
    const EditListBox*            mEditListBox;
    const TimeToSampleBox*        mTimeToSampleBox;
    const CompositionOffsetBox*   mCompositionOffsetBox;
    const CompositionToDecodeBox* mCompositionToDecodeBox;

    std::uint32_t mMovieOffset;

    PMap mMediaPts; ///< Media presentation timestamps
    PMap mMoviePts; ///< Movie presentation timestamps after EditList has been applied

    /// Template function to apply edits; T can either be (a) EntryVersion0, or (b) EntryVersion1
    template<typename T>
    void applyEdit(T& entry);

    /// Template function to apply empty edits; T can either be (a) EntryVersion0, or (b) EntryVersion1
    template<typename T>
    void applyEmptyEdit(T& entry);

    /// Template function to apply dwell edits; T can either be (a) EntryVersion0, or (b) EntryVersion1
    template<typename T>
    void applyDwellEdit(T& entry);

    /// Template function to apply shift edits; T can either be (a) EntryVersion0, or (b) EntryVersion1
    template<typename T>
    void applyShiftEdit(T& entry);
};

template<typename T>
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

template<typename T>
void DecodePts::applyEmptyEdit(T& entry)
{
    mMovieOffset += entry.mSegmentDuration;
}

template<typename T>
void DecodePts::applyDwellEdit(T& entry)
{
    using PMapIt = PMap::iterator;

    std::pair<PMapIt, PMapIt> bound;
    bound = mMediaPts.equal_range(entry.mMediaTime);

    // If lower bound and upper bound point to the same entry in the map the
    // previous sample in the map needs dwell.
    if (bound.first->first == bound.second->first)
    {
        mMoviePts.insert(std::make_pair(mMovieOffset, std::prev(bound.first)->second));
        mMovieOffset += entry.mSegmentDuration;
    }
    // if the lower and upper bound points to different entries in the map then
    // the first iterator points to the sample that needs dwell.
    else
    {
        mMoviePts.insert(std::make_pair(mMovieOffset, bound.first->second));
        mMovieOffset += entry.mSegmentDuration;
    }
}

/// @todo This function can be optimized further using map ranges.
template<typename T>
void DecodePts::applyShiftEdit(T& entry)
{
    const std::int64_t segmentEndTime = static_cast<std::int64_t>(entry.mMediaTime + entry.mSegmentDuration);

    for (auto it = mMediaPts.cbegin(); it != mMediaPts.cend(); ++it)
    {
        // Find those samples that is presented in this edit
        if (it->first >= static_cast<std::int64_t>(entry.mMediaTime) &&
            it->first < segmentEndTime)
        {
            // If the pts first sample of this edit does not exactly fall in the
            // start of the edit, also include the previous sample and compute
            // the time for which that sample is displayed.
            if (std::prev(it) != mMediaPts.cbegin() &&
                std::prev(it)->first < static_cast<std::int64_t>(entry.mMediaTime) &&
                it->first != static_cast<std::int64_t>(entry.mMediaTime))
            {
                mMoviePts.insert(std::make_pair(mMovieOffset, std::prev(it)->second));
                mMovieOffset += it->first - (std::prev(it)->first + (entry.mMediaTime - std::prev(it)->first));
            }

            // Insert the rest of the samples into the movie edit
            mMoviePts.insert(std::make_pair(mMovieOffset, it->second));

            // If the next sample falls into the edit, update mMovieOffset with
            // the difference of this sample and the next.
            if (std::next(it) != mMediaPts.cend() &&
                std::next(it)->first <= segmentEndTime)
            {
                // This is not the last sample that falls into the edit
                mMovieOffset += std::next(it)->first - it->first;
            }
            else
            {
                // This is the last sample, give it as much time as this edit segment lasts.
                mMovieOffset += segmentEndTime - it->first;
            }
        }
    }
}

#endif /* end of include guard: DECODEPTS_HPP */
