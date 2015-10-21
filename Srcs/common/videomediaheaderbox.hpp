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

#ifndef VIDEOMEDIAHEADERBOX_HPP
#define VIDEOMEDIAHEADERBOX_HPP

#include "fullbox.hpp"

class BitStream;

/** @brief VideoMediaHeaderBox class. Extends from FullBox.
 *  @details 'vmhd' box contains general presentation information related to the track as defined in the ISOBMFF and HEIF standards.
 *  @details Currently all fields are hard-coded to default values. **/
class VideoMediaHeaderBox : public FullBox
{
public:
    VideoMediaHeaderBox();
    virtual ~VideoMediaHeaderBox() = default;

    /** @brief Creates the bitstream that represents the box in the ISOBMFF file
     *  @param [out] bitstr Bitstream that contains the box data. */
    void writeBox(BitStream& bitstr);

    /** @brief Parses a VideoMediaHeaderBox bitstream and fills in the necessary member variables
     *  @param [in]  bitstr Bitstream that contains the box data */
    void parseBox(BitStream& bitstr);
};

#endif /* end of include guard: VIDEOMEDIAHEADERBOX_HPP */
