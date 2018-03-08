/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

#ifndef DECODEPTS_HPP
#define DECODEPTS_HPP

#include <algorithm>
#include <map>
#include "compositionoffsetbox.hpp"
#include "compositiontodecodebox.hpp"
#include "customallocator.hpp"
#include "editbox.hpp"
#include "timetosamplebox.hpp"
#include "trackrunbox.hpp"
#include "writeoncemap.hpp"

/**
 * @brief Class for decoding presentation timestamps for samples of a TrackBox.
 * @todo CompositionToDecodeBox is currently ignored.
 */
class DecodePts
{
public:
    typedef std::int64_t PresentationTime;    ///< Sample presentation time in milliseconds
    typedef std::int64_t PresentationTimeTS;  ///< Sample presentation time in time scale units
    typedef std::uint64_t SampleIndex;        ///< 0-based sample index

    /** The PMap (Presentation Map) always uses 64 bit key-value regardless
     *  of which version of entries are used by the stts, and ctts boxes. */
    using PMap = WriteOnceMap<PresentationTime, SampleIndex>;

    /** Same but with time scale units */
    using PMapTS = WriteOnceMap<PresentationTimeTS, SampleIndex>;

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
    void loadBox(const EditListBox* editListBox, std::uint32_t movieTimescale, std::uint32_t mediaTimescale);

    /** Set TrackRunBox pointer to the class for later use
     * @todo Refactor to avoid saving raw pointers to the object */
    void loadBox(const TrackRunBox* trackRunBox);

    /**
     * @brief Generate presentation timestamps
     * @pre mTimeToSampleBox has been set
     * @pre mCompositionOffsetBox has been set if CompositionOffsetBox is present
     * @pre mEditListBox has been set if EditListBox is present
     * @return true is unravel succeeded, false if the was an error
     */
    bool unravel();

    /**
     * @brief Generate presentation timestamps
     * @pre TrackRunBox has been set
     */
    void unravelTrackRun();

    /**
     * @brief Get duration of the track in media timescale units.
     * @return Duration of the track considering possible edit list
     */
    std::uint64_t getSpan() const;

    /**
     * @brief Applies local time transformations and edit list (if it exists)
     * @param ptsOffset The presentation time offset to apply to each track sample (mMoviePtsTS)
     * @param fixed The pts offset is fixed and the edit list will not be applied to it
     */
    void applyLocalTime(std::uint64_t ptsOffset);

    /**
     * @brief
     * @param timeScale Timescale of the TrackBox
     * @return Presentation timestamps
     */
    PMap getTime(std::uint32_t timeScale) const;
    PMapTS getTimeTS() const;

    /**
     * @brief
     * @param timeScale Timescale of the TrackBox
     * @pre TrackRunBox has been set
     * @return Presentation timestamps
     */
    void getTimeTrackRun(std::uint32_t timeScale, PMap& oldPMap) const;
    void getTimeTrackRunTS(PMapTS& oldPMapTS) const;

private:
    const EditListBox* mEditListBox;
    std::uint32_t mMovieTimescale;
    std::uint32_t mMediaTimescale;
    const TimeToSampleBox* mTimeToSampleBox;
    const CompositionOffsetBox* mCompositionOffsetBox;
    const CompositionToDecodeBox* mCompositionToDecodeBox;
    const TrackRunBox* mTrackRunBox;

    std::uint64_t mMovieOffset;     ///< Movie offset in media timescale units.
    std::int64_t mMediaOffset = 0;  ///< keep track of the input offset, set from BaseMediaDecodeTime

    PMapTS mMediaPtsTS;  ///< Media presentation timestamps
    PMapTS mMoviePtsTS;  ///< Movie presentation timestamps after EditList has been applied

    /// Determine the duration of the last sample; or 0 if no samples
    std::uint64_t lastSampleDuration() const;

    /// Template function to apply edits; Entry can either be (a) EntryVersion0, or (b) EntryVersion1
    template <typename Entry>
    void applyEdit(Entry& entry);

    /// Template function to apply empty edits; Entry can either be (a) EntryVersion0, or (b) EntryVersion1
    template <typename Entry>
    void applyEmptyEdit(Entry& entry);

    /// Template function to apply dwell edits; Entry can either be (a) EntryVersion0, or (b) EntryVersion1
    template <typename Entry>
    void applyDwellEdit(Entry& entry);

    /// Template function to apply shift edits; Entry can either be (a) EntryVersion0, or (b) EntryVersion1
    template <typename Entry>
    void applyShiftEdit(Entry& entry);

    template <typename Time>
    std::uint64_t fromMovieToMediaTS(Time movieTS) const;

    // Apply edit list; requires the mEditListBox field to be set
    void applyEditList();
};

#endif /* end of include guard: DECODEPTS_HPP */
