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

#ifndef TRACKHEADERBOX_HPP_CWQQ8ZSB
#define TRACKHEADERBOX_HPP_CWQQ8ZSB

#include "bitstream.hpp"
#include "fullbox.hpp"

#include <cstdint>
#include <iostream>
#include <string>

/** @brief TrackBox class. Extends from Box.
 *  @details 'tkhd' box contains information related to the track properties as defined in the ISOBMFF standard. **/
class TrackHeaderBox : public FullBox
{
public:
    TrackHeaderBox();
    virtual ~TrackHeaderBox() = default;

    /** @brief Sets track's creation time as defined in ISOBMFF
     *  @param [in] creation_time track's creation time **/
    void setCreationTime(std::uint32_t creation_time);

    /** @brief Get track's creation time as defined in ISOBMFF
     *  @returns Track's creation time **/
    std::uint32_t getCreationTime() const;

    /** @brief Sets track's modification time as defined in ISOBMFF
     *  @param [in] modification_time track's modification time **/
    void setModificationTime(std::uint32_t modification_time);

    /** @brief Get track's modification time as defined in ISOBMFF
     *  @returns Track's modification time **/
    std::uint32_t getModificationTime() const;

    /** @brief Sets track's ID as defined in ISOBMFF
     *  @param [in] track_ID track's given ID **/
    void setTrackID(std::uint32_t track_ID);

    /** @brief Gets track's ID as defined in ISOBMFF
     *  @returns Track's given Id **/
    std::uint32_t getTrackID() const;

    /** @brief Sets track's duration as defined in ISOBMFF
     *  @param [in] duration track's duration **/
    void setDuration(std::uint32_t duration);

    /** @brief Get track's duration as defined in ISOBMFF
     *  @returns Track's duration **/
    std::uint32_t getDuration() const;

    /** @brief Sets track's display width as defined in ISOBMFF
     *  @param [in] width track's display width **/
    void setWidth(std::uint32_t width);

    /** @brief Gets track's display width as defined in ISOBMFF
     *  @returns Track's display width **/
    std::uint32_t getWidth() const;

    /** @brief Sets track's display height as defined in ISOBMFF
     *  @param [in] height track's display height **/
    void setHeight(std::uint32_t height);

    /** @brief Gets track's display height as defined in ISOBMFF
     *  @returns Track's display height **/
    std::uint32_t getHeight() const;

    /** @brief Sets track's alternate group Id as defined in ISOBMFF
     *  @param [in] groupID track's alternate group id **/
    void setAlternateGroup(std::uint16_t groupID);

    /** @brief Get track's alternate group Id as defined in ISOBMFF
     *  @returns track's alternate group id **/
    std::uint16_t getAlternateGroup() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(BitStream& bitstr);

    /** @brief Parses a TrackHeaderBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(BitStream& bitstr);

private:
    std::uint32_t mCreationTime;     ///< Creation time
    std::uint32_t mModificationTime; ///< Modificaiton time
    std::uint32_t mTrackID;          ///< Track ID
    std::uint64_t mDuration;         ///< Track's duration
    std::uint32_t mWidth;            ///< Track display width
    std::uint32_t mHeight;           ///<  Track display height
    std::uint16_t mAlternateGroup;   ///< Alternate group Id of the track
};

#endif /* end of include guard: TRACKHEADERBOX_HPP_CWQQ8ZSB */
