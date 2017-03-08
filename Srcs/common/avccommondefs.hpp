/* Copyright (c) 2017, Nokia Technologies Ltd.
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

#ifndef AVCCOMMONDEFS_HPP
#define AVCCOMMONDEFS_HPP

#include <cstdint>

/** @brief Common enumeration definitions for decoder configuration record */
enum class AvcNalUnitType: std::uint8_t
{
    UNSPECIFIED_0 = 0,
    CODED_SLICE_NON_IDR,        //1
    CODED_SLICE_DPAR_A,
    CODED_SLICE_DPAR_B,
    CODED_SLICE_DPAR_C,
    CODED_SLICE_IDR,            //5
    SEI,                        //6
    SPS,                        //7
    PPS,                        //8
    ACCESS_UNIT_DELIMITER,
    EOS,                        //10
    EOB,                        //11
    FILLER_DATA,
    SPS_EXT,
    PREFIX_NALU,
    SUB_SPS,
    DPS,
    RESERVED_17,
    RESERVED_18,
    SLICE_AUX_NOPAR,
    SLICE_EXT,
    SLICE_EXT_3D,
    RESERVED_22,
    RESERVED_23,
    UNSPECIFIED_24,
    UNSPECIFIED_25,
    UNSPECIFIED_26,
    UNSPECIFIED_27,
    UNSPECIFIED_28,
    UNSPECIFIED_29,
    UNSPECIFIED_30,
    UNSPECIFIED_31,
    INVALID
};

#endif
