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

#ifndef IMAGEOVERLAY_HPP
#define IMAGEOVERLAY_HPP

#include <cstdint>
#include <vector>

class BitStream;

/** Image Overlay item struct for derived images of 'iovl' type. */
struct ImageOverlay
{
    std::uint16_t canvasFillValueR = 0; ///< R of RGBA pixel values used to fill canvas (which is not covered by any image)
    std::uint16_t canvasFillValueG = 0; ///< G of RGBA pixel values used to fill canvas (which is not covered by any image)
    std::uint16_t canvasFillValueB = 0; ///< B of RGBA pixel values used to fill canvas (which is not covered by any image)
    std::uint16_t canvasFillValueA = 0; ///< Linear opacity of the canvas fill from 0 (transparent) to 65535 (opaque).

    std::uint32_t outputWidth = 0;  ///< Width of the constructed image.
    std::uint32_t outputHeight = 0; ///< Height of the constructed image.

    struct Offset
    {
        std::int32_t horizontalOffset; ///< Horizontal offset from the top-left corner of the canvas.
        std::int32_t verticalOffset;   ///< Vertical offset from the top-left corner of the canvas.
    };
    std::vector<Offset> offsets; ///< Offsets of images in the grid. Number and order must match to that of 'dimg' references.
};

/** Write ImageOverlay to BitStream.
 *  @param [in]  iovl ImageOverlay to serialize.
 *  @param [out] output BitStream where to serialize ImageOverlay data. */
void writeImageOverlay(const ImageOverlay& iovl, BitStream& output);

/** Parse ImageOverlay from BitStream.
 *  @param [in] input BitStream where to read ImageOverlay data from.
 *  @return Read and filled ImageOverlay. */
ImageOverlay parseImageOverlay(BitStream& input);

#endif /* end of include guard: IMAGEOVERLAY_HPP */
