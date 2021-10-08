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

#ifndef TRACKFRAGMENTHEADERBOX_HPP
#define TRACKFRAGMENTHEADERBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"
#include "moviefragmentsdatatypes.hpp"

/**
 * @brief  Track Fragment Header Box class
 * @details 'tfhd' box implementation as specified in the ISOBMFF specification.
 */
class TrackFragmentHeaderBox : public FullBox
{
public:
    TrackFragmentHeaderBox(std::uint32_t tr_flags = 0);
    ~TrackFragmentHeaderBox() override = default;

    enum TrackFragmentHeaderFlags
    {
        BaseDataOffsetPresent         = 0x000001,
        SampleDescriptionIndexPresent = 0x000002,
        DefaultSampleDurationPresent  = 0x000008,
        DefaultSampleSizePresent      = 0x000010,
        DefaultSampleFlagsPresent     = 0x000020,
        DurationIsEmpty               = 0x010000,
        DefaultBaseIsMoof             = 0x020000
    };

    /** @brief Set Track ID of the TrackFragmentHeaderBox.
     *  @param [in] uint32_t trackId */
    void setTrackId(const uint32_t trackId);

    /** @brief Get Track ID of the TrackFragmentHeaderBox.
     *  @return uint32_t as specified in 8.8.7.1  of ISO/IEC 14496-12:2015(E) */
    uint32_t getTrackId() const;

    /** @brief Set Base Data Offset of the TrackFragmentHeaderBox.
     *  @param [in] uint64_t baseDataOffset */
    void setBaseDataOffset(const uint64_t baseDataOffset);

    /** @brief Get Base Data Offset of the TrackFragmentHeaderBox.
     *  @return uint64_t as specified in 8.8.7.1  of ISO/IEC 14496-12:2015(E) */
    uint64_t getBaseDataOffset() const;

    /** @brief Set Sample Description Index of the TrackFragmentHeaderBox.
     *  @param [in] uint32_t sampleDescriptionIndex */
    void setSampleDescriptionIndex(const uint32_t sampleDescriptionIndex);

    /** @brief Get Sample Description Index of the TrackFragmentHeaderBox.
     *  @return uint32_t as specified in 8.8.7.1 of ISO/IEC 14496-12:2015(E) */
    uint32_t getSampleDescriptionIndex() const;

    /** @brief Set Default Sample Duration of the TrackFragmentHeaderBox.
     *  @param [in] uint32_t defaultSampleDuration **/
    void setDefaultSampleDuration(const uint32_t defaultSampleDuration);

    /** @brief Get Default Sample Duration of the TrackFragmentHeaderBox.
     *  @return uint32_t as specified in 8.8.7.1  of ISO/IEC 14496-12:2015(E) */
    uint32_t getDefaultSampleDuration() const;

    /** @brief Set Default Sample Size of the TrackFragmentHeaderBox.
     *  @param [in] uint32_t defaultSampleSize**/
    void setDefaultSampleSize(const uint32_t defaultSampleSize);

    /** @brief Get Default Sample Size of the TrackFragmentHeaderBox.
     *  @return uint32_t as specified in 8.8.7.1  of ISO/IEC 14496-12:2015(E) */
    uint32_t getDefaultSampleSize() const;

    /** @brief Set  of the TrackFragmentHeaderBox.
     *  @param [in] MOVIEFRAGMENTS::SampleFlags (uint32_t) defaultSampleFlags */
    void setDefaultSampleFlags(const MOVIEFRAGMENTS::SampleFlags defaultSampleFlags);

    /** @brief Get  of the TrackFragmentHeaderBox.
     *  @return MOVIEFRAGMENTS::SampleFlags (uint32_t) as specified in 8.8.3.1 of ISO/IEC 14496-12:2015(E) */
    MOVIEFRAGMENTS::SampleFlags getDefaultSampleFlags() const;

    /**
     * @brief Serialize box data to the ISOBMFF::BitStream.
     * @see Box::writeBox()
     */
    void writeBox(ISOBMFF::BitStream& bitstr) const override;

    /**
     * @brief Deserialize box data from the ISOBMFF::BitStream.
     * @see Box::parseBox()
     */
    void parseBox(ISOBMFF::BitStream& bitstr) override;

private:
    uint32_t mTrackId;

    // Optional fields:
    uint64_t mBaseDataOffset;
    uint32_t mSampleDescriptionIndex;
    uint32_t mDefaultSampleDuration;
    uint32_t mDefaultSampleSize;
    MOVIEFRAGMENTS::SampleFlags mDefaultSampleFlags;
};

#endif /* end of include guard: TRACKFRAGMENTHEADERBOX_HPP */
