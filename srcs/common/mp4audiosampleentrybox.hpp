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

#ifndef MP4AUDIOSAMPLEENTRYBOX_HPP
#define MP4AUDIOSAMPLEENTRYBOX_HPP

#include "audiosampleentrybox.hpp"
#include "bitstream.hpp"
#include "customallocator.hpp"
#include "elementarystreamdescriptorbox.hpp"

/** @brief AudioSampleEntryBox class. Extends from SampleEntryBox.
 *  @details This box contains information related to the mp4 audio samples of the track
 *  @details as defined in the ISO/IEC FDIS 14496-14 standard. */

class MP4AudioSampleEntryBox : public AudioSampleEntryBox
{
public:
    MP4AudioSampleEntryBox();
    virtual ~MP4AudioSampleEntryBox() = default;

    /** @brief Gets sample's ElementaryStreamDescritorBox as defined in ISO/IEC FDIS 14496-14 standard
     *  @returns Sample's ElementaryStreamDescritorBox */
    ElementaryStreamDescriptorBox& getESDBox();

    /** @brief Gets sample's ElementaryStreamDescritorBox as defined in ISO/IEC FDIS 14496-14 standard
     *  @returns Sample's ElementaryStreamDescritorBox */
    const ElementaryStreamDescriptorBox& getESDBox() const;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a AudioSampleEntryBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

    virtual MP4AudioSampleEntryBox* clone() const;

private:
    ElementaryStreamDescriptorBox mESDBox;
};

#endif  // MP4AUDIOSAMPLEENTRYBOX_HPP
