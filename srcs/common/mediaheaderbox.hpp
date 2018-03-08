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

#ifndef MEDIAHEADERBOX_HPP
#define MEDIAHEADERBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"

#include <cstdint>

/** @brief Media Header Box class. Extends from FullBox.
 *  @details 'mdhd' box contains basic inforation about the media data it represents as defined in the ISOBMFF standard. */
class MediaHeaderBox : public FullBox
{
public:
    MediaHeaderBox();
    virtual ~MediaHeaderBox() = default;

    /** @brief Set the creation time of the media.
     *  @param [in] creation_time the creation time in the format as defined in ISOBMFF standard. */
    void setCreationTime(std::uint64_t creation_time);

    /** @brief Get the creation time of the media.
     *  @return Media creation time in the format as defined in ISOBMFF standard. */
    std::uint64_t getCreationTime();

    /** @brief Set the modification time of the media.
     *  @param [in] modification_time the modification time in the format as defined in ISOBMFF standard. */
    void setModificationTime(std::uint64_t modification_time);

    /** @brief Get the modification time of the media.
     *  @return Media modification time in the format as defined in ISOBMFF standard. */
    std::uint64_t getModificationTime();

    /** @brief Set the timescale used for the media.
     *  @param [in] time_scale timescale in the format as defined in ISOBMFF standard. */
    void setTimeScale(std::uint32_t time_scale);

    /** @brief Get the timescale used for the media.
     *  @return Timescale in the format as defined in ISOBMFF standard. */
    std::uint32_t getTimeScale() const;

    /** @brief Set the duration of the media.
     *  @param [in] duration Media duration in the format as defined in ISOBMFF standard. */
    void setDuration(std::uint64_t duration);

    /** @brief Get the duration of the media.
     *  @return duration Media duration in the format as defined in ISOBMFF standard. */
    std::uint64_t getDuration() const;

    /** @brief Set the language of the media.
     *  @param [in] language Language in the format as defined in ISOBMFF standard. */
    void setLanguage(std::uint16_t language);

    /** @brief Get the language of the media.
     *  @return Language in the format as defined in ISOBMFF standard. */
    std::uint32_t getLanguage() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a MediaHeaderBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    std::uint64_t mCreationTime;      ///< Creation time
    std::uint64_t mModificationTime;  ///< Modification time
    std::uint32_t mTimeScale;         ///< Timescale
    std::uint64_t mDuration;          ///< Duration
    std::uint16_t mLanguage;          ///< Language
};

#endif /* end of include guard: MEDIAHEADERBOX_HPP */
