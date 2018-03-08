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

#ifndef VIDEOMEDIAHEADERBOX_HPP
#define VIDEOMEDIAHEADERBOX_HPP

#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** @brief VideoMediaHeaderBox class. Extends from FullBox.
 *  @details 'vmhd' box contains general presentation information related to the track as defined in the ISOBMFF standard.
 *  @details Currently all fields are hard-coded to default values. */
class VideoMediaHeaderBox : public FullBox
{
public:
    VideoMediaHeaderBox();
    virtual ~VideoMediaHeaderBox() = default;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @brief Parses a VideoMediaHeaderBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(ISOBMFF::BitStream& bitstr);
};

#endif /* end of include guard: VIDEOMEDIAHEADERBOX_HPP */
