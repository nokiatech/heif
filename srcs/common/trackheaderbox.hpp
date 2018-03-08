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

#ifndef TRACKHEADERBOX_HPP_CWQQ8ZSB
#define TRACKHEADERBOX_HPP_CWQQ8ZSB

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "fullbox.hpp"

#include <cstdint>
#include <iostream>

/** @brief TrackBox class. Extends from Box.
 *  @details 'tkhd' box contains information related to the track properties as defined in the ISOBMFF standard. */
class TrackHeaderBox : public FullBox
{
public:
    TrackHeaderBox();
    virtual ~TrackHeaderBox() = default;

    /** @brief Sets track's creation time as defined in ISOBMFF
     *  @param [in] creation_time track's creation time */
    void setCreationTime(std::uint64_t creation_time);

    /** @brief Get track's creation time as defined in ISOBMFF
     *  @returns Track's creation time */
    std::uint64_t getCreationTime() const;

    /** @brief Sets track's modification time as defined in ISOBMFF
     *  @param [in] modification_time track's modification time */
    void setModificationTime(std::uint64_t modification_time);

    /** @brief Get track's modification time as defined in ISOBMFF
     *  @returns Track's modification time */
    std::uint64_t getModificationTime() const;

    /** @brief Sets track's ID as defined in ISOBMFF
     *  @param [in] track_ID track's given ID */
    void setTrackID(std::uint32_t track_ID);

    /** @brief Gets track's ID as defined in ISOBMFF
     *  @returns Track's given Id */
    std::uint32_t getTrackID() const;

    /** @brief Sets track's duration as defined in ISOBMFF
     *  @param [in] duration track's duration */
    void setDuration(std::uint64_t duration);

    /** @brief Get track's duration as defined in ISOBMFF
     *  @returns Track's duration */
    std::uint64_t getDuration() const;

    /** @brief Sets track's display width as defined in ISOBMFF
     *  @param [in] width track's display width */
    void setWidth(std::uint32_t width);

    /** @brief Gets track's display width as defined in ISOBMFF
     *  @returns Track's display width */
    std::uint32_t getWidth() const;

    /** @brief Sets track's display height as defined in ISOBMFF
     *  @param [in] height track's display height */
    void setHeight(std::uint32_t height);

    /** @brief Gets track's display height as defined in ISOBMFF
     *  @returns Track's display height */
    std::uint32_t getHeight() const;

    /** @brief Sets track's alternate group Id as defined in ISOBMFF
     *  @param [in] groupID track's alternate group id */
    void setAlternateGroup(std::uint16_t groupID);

    /** @brief Get track's alternate group Id as defined in ISOBMFF
     *  @returns track's alternate group id */
    std::uint16_t getAlternateGroup() const;

    /** @brief Sets track's volume as defined in ISOBMFF
    *  @param [in] volume track's volume */
    void setVolume(const std::uint16_t volume);

    /** @brief Get track's volume as defined in ISOBMFF
    *  @returns track's volume */
    std::uint16_t getVolume() const;

    /** @brief Sets track's matrix as defined in ISOBMFF
    *  @param [in] groupID track's alternate group id */
    void setMatrix(const Vector<int32_t>& matrix);

    /** @brief Get track's matrix as defined in ISOBMFF
    *  @returns track's alternate group id */
    Vector<int32_t> getMatrix() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a TrackHeaderBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr);

private:
    std::uint64_t mCreationTime;      ///< Creation time
    std::uint64_t mModificationTime;  ///< Modificaiton time
    std::uint32_t mTrackID;           ///< Track ID
    std::uint64_t mDuration;          ///< Track's duration
    std::uint32_t mWidth;             ///< Track display width
    std::uint32_t mHeight;            ///< Track display height
    std::uint16_t mAlternateGroup;    ///< Alternate group Id of the track
    std::uint16_t mVolume;            ///< Volume (for audio tracks)
    Vector<std::int32_t> mMatrix;     ///< Matrix
};

#endif /* end of include guard: TRACKHEADERBOX_HPP_CWQQ8ZSB */
