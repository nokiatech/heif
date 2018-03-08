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

#ifndef AVCCOMMONDEFS_HPP
#define AVCCOMMONDEFS_HPP

#include <cstdint>
#include "customallocator.hpp"

/** @brief Common enumeration definitions for decoder configuration record */
enum class AvcNalUnitType : std::uint8_t
{
    UNSPECIFIED_0 = 0,
    CODED_SLICE_NON_IDR,  //1
    CODED_SLICE_DPAR_A,
    CODED_SLICE_DPAR_B,
    CODED_SLICE_DPAR_C,
    CODED_SLICE_IDR,  //5
    SEI,              //6
    SPS,              //7
    PPS,              //8
    ACCESS_UNIT_DELIMITER,
    EOS,  //10
    EOB,  //11
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
