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

#ifndef MOOVWRITER_HPP
#define MOOVWRITER_HPP

#include "moviebox.hpp"
#include <memory>

class TrackBox;

/**
 * @brief Create a MovieBox containing a MovieHeaderBox, 1 or more TrackBoxes, and serialize it to BitStream.
 */
class MoovWriter
{
public:
    /**
     * Add a TrackBox to the MovieBox.
     * @param trackBox   Pointer to a TrackBox.
     * @param mdatOffset File offset of the MediaDataBox related to this track.
     */
    void addTrack(std::unique_ptr<TrackBox> trackBox, unsigned int mdatOffset);

    /**
     * Serialize MovieBox data to the BitStream.
     * @param bitStream Output BitStream
     */
    void serialize(BitStream& bitStream);

private:
    std::uint32_t mTimeScale = 0; ///< timescale in MovieHeaderBox contained by the MovieBox
    std::uint32_t mDuration = 0;  ///< duration in MovieHeaderBox contained by the MovieBox

    MovieBox mMoovBox; ///< MovieBox to be filled and serialized
};

#endif /* end of include guard: MOOVWRITER_HPP */
