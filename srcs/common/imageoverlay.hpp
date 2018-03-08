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

#ifndef IMAGEOVERLAY_HPP
#define IMAGEOVERLAY_HPP

#include <cstdint>
#include "customallocator.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** Image Overlay item struct for derived images of 'iovl' type. */
struct ImageOverlay
{
    std::uint16_t canvasFillValueR = 0;  ///< R of RGBA pixel values used to fill canvas (which is not covered by any image)
    std::uint16_t canvasFillValueG = 0;  ///< G of RGBA pixel values used to fill canvas (which is not covered by any image)
    std::uint16_t canvasFillValueB = 0;  ///< B of RGBA pixel values used to fill canvas (which is not covered by any image)
    std::uint16_t canvasFillValueA = 0;  ///< Linear opacity of the canvas fill from 0 (transparent) to 65535 (opaque).

    std::uint32_t outputWidth  = 0;  ///< Width of the constructed image.
    std::uint32_t outputHeight = 0;  ///< Height of the constructed image.

    struct Offset
    {
        std::int32_t horizontalOffset;  ///< Horizontal offset from the top-left corner of the canvas.
        std::int32_t verticalOffset;    ///< Vertical offset from the top-left corner of the canvas.
    };
    Vector<Offset> offsets;  ///< Offsets of images in the grid. Number and order must match to that of 'dimg' references.
};

/** Write ImageOverlay to ISOBMFF::BitStream.
 *  @param [in]  iovl ImageOverlay to serialize.
 *  @param [out] output ISOBMFF::BitStream where to serialize ImageOverlay data. */
void writeImageOverlay(const ImageOverlay& iovl, ISOBMFF::BitStream& output);

/** Parse ImageOverlay from ISOBMFF::BitStream.
 *  @param [in] input ISOBMFF::BitStream where to read ImageOverlay data from.
 *  @return Read and filled ImageOverlay. */
ImageOverlay parseImageOverlay(ISOBMFF::BitStream& input);

#endif /* end of include guard: IMAGEOVERLAY_HPP */
