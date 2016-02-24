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

#ifndef MEDIAHEADERBOX_HPP
#define MEDIAHEADERBOX_HPP

#include "bitstream.hpp"
#include "fullbox.hpp"

#include <cstdint>

/** @brief Media Header Box class. Extends from FullBox.
 *  @details 'mdhd' box contains basic inforation about the media data it represents as defined in the ISOBMFF and HEIF standards. **/
class MediaHeaderBox : public FullBox
{
public:
    MediaHeaderBox();
    virtual ~MediaHeaderBox() = default;

    /** @brief Set the creation time of the media.
     *  @param [in] creation_time the creation time in the format as defined in ISOBMFF standard. **/
    void setCreationTime(std::uint32_t creation_time);

    /** @brief Get the creation time of the media.
     *  @return Media creation time in the format as defined in ISOBMFF standard. **/
    std::uint32_t getCreationTime();

    /** @brief Set the modification time of the media.
     *  @param [in] modification_time the modification time in the format as defined in ISOBMFF standard. **/
    void setModificationTime(std::uint32_t modification_time);

    /** @brief Get the modification time of the media.
     *  @return Media modification time in the format as defined in ISOBMFF standard. **/
    std::uint32_t getModificationTime();

    /** @brief Set the timescale used for the media.
     *  @param [in] time_scale timescale in the format as defined in ISOBMFF standard. **/
    void setTimeScale(std::uint32_t time_scale);

    /** @brief Get the timescale used for the media.
     *  @return Timescale in the format as defined in ISOBMFF standard. **/
    std::uint32_t getTimeScale();

    /** @brief Set the duration of the media.
     *  @param [in] duration Media duration in the format as defined in ISOBMFF standard. **/
    void setDuration(std::uint32_t duration);

    /** @brief Get the duration of the media.
     *  @return duration Media duration in the format as defined in ISOBMFF standard. **/
    std::uint32_t getDuration();

    /** @brief Set the language of the media.
     *  @param [in] language Language in the format as defined in ISOBMFF standard. **/
    void setLanguage(std::uint16_t language);

    /** @brief Get the language of the media.
     *  @return Language in the format as defined in ISOBMFF standard. **/
    std::uint32_t getLanguage();

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(BitStream& bitstr);

    /** @brief Parses a MediaHeaderBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(BitStream& bitstr);

private:
    std::uint32_t mCreationTime; ///< Creation time
    std::uint32_t mModificationTime; ///< Modification time
    std::uint32_t mTimeScale; ///< Timescale
    std::uint32_t mDuration; ///< Duration
    std::uint16_t mLanguage; ///< Language
};

#endif /* end of include guard: MEDIAHEADERBOX_HPP */

