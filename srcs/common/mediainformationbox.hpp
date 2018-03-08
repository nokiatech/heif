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

#ifndef MEDIAINFORMATIONBOX_HPP
#define MEDIAINFORMATIONBOX_HPP

#include "bitstream.hpp"
#include "customallocator.hpp"
#include "datainformationbox.hpp"
#include "nullmediaheaderbox.hpp"
#include "sampletablebox.hpp"
#include "soundmediaheaderbox.hpp"
#include "videomediaheaderbox.hpp"

/** @brief Media Information Box class. Extends from Box.
 *  @details 'minf' box contains the information needed to access media samples and initialize decoders as defined in the ISOBMFF standard. */
class MediaInformationBox : public Box
{
public:
    MediaInformationBox();
    virtual ~MediaInformationBox() = default;

    enum class MediaType
    {
        Null,
        Video,
        Sound
    };

    /** @brief Set the media type for this box */
    void setMediaType(MediaType type);

    /** @brief Get the media type for this box */
    /** @return The media type for this box */
    MediaType getMediaType() const;

    /** @brief Get VideoMediaHeader box.
     *  @return Reference to VideoMediaHeaderBox data structure */
    VideoMediaHeaderBox& getVideoMediaHeaderBox();

    /** @brief Get VideoMediaHeader box.
     *  @return Reference to VideoMediaHeaderBox data structure */
    const VideoMediaHeaderBox& getVideoMediaHeaderBox() const;

    /** @brief Get DataInformationBox.
     *  @return Reference to DataInformationBox data structure */
    DataInformationBox& getDataInformationBox();

    /** @brief Get DataInformationBox.
     *  @return Reference to DataInformationBox data structure */
    const DataInformationBox& getDataInformationBox() const;

    /** @brief Get SampleTableBox.
     *  @return Reference to SampleTableBox data structure */
    SampleTableBox& getSampleTableBox();

    /** @brief Get SampleTableBox.
     *  @return Reference to SampleTableBox data structure */
    const SampleTableBox& getSampleTableBox() const;

    /** @brief Get NullMediaHeaderBox.
     *  @return Reference to NullMediaHeaderBox data structure */
    NullMediaHeaderBox& getNullMediaHeaderBox();

    /** @brief Get NullMediaHeaderBox.
     *  @return Reference to NullMediaHeaderBox data structure */
    const NullMediaHeaderBox& getNullMediaHeaderBox() const;

    /** @brief Get SoundMediaHeaderBox.
     *  @return Reference to SoundMediaHeaderBox data structure */
    SoundMediaHeaderBox& getSoundMediaHeaderBox();

    /** @brief Get SoundMediaHeaderBox.
     *  @return Reference to SoundMediaHeaderBox data structure */
    const SoundMediaHeaderBox& getSoundMediaHeaderBox() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a MediaInformationBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

private:
    MediaType mMediaType;                      ///< Which media type?
    VideoMediaHeaderBox mVideoMediaHeaderBox;  ///< Video media header box
    SoundMediaHeaderBox mSoundMediaHeaderBox;  ///< Sound Media Header box
    NullMediaHeaderBox mNullMediaHeaderBox;    ///< Null Media Header box
    DataInformationBox mDataInformationBox;    ///< Data information box
    SampleTableBox mSampleTableBox;            ///< Sample Table box
};

#endif /* end of include guard: MEDIAINFORMATIONBOX_HPP */
