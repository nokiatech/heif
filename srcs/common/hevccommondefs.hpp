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

#ifndef HEVCCOMMONDEFS_HPP
#define HEVCCOMMONDEFS_HPP

#include <cstdint>
#include "customallocator.hpp"

/** @brief Common enumeration definitions for decoder configuration record */
enum class HevcNalUnitType : std::uint8_t
{
    CODED_SLICE_TRAIL_N = 0,  // 0
    CODED_SLICE_TRAIL_R,      // 1

    CODED_SLICE_TSA_N,  // 2
    CODED_SLICE_TSA_R,  // 3

    CODED_SLICE_STSA_N,  // 4
    CODED_SLICE_STSA_R,  // 5

    CODED_SLICE_RADL_N,  // 6
    CODED_SLICE_RADL_R,  // 7

    CODED_SLICE_RASL_N,  // 8
    CODED_SLICE_RASL_R,  // 9

    RESERVED_VCL_N10,
    RESERVED_VCL_R11,
    RESERVED_VCL_N12,
    RESERVED_VCL_R13,
    RESERVED_VCL_N14,
    RESERVED_VCL_R15,

    CODED_SLICE_BLA_W_LP,    // 16
    CODED_SLICE_BLA_W_RADL,  // 17
    CODED_SLICE_BLA_N_LP,    // 18
    CODED_SLICE_IDR_W_RADL,  // 19
    CODED_SLICE_IDR_N_LP,    // 20
    CODED_SLICE_CRA,         // 21
    RESERVED_IRAP_VCL22,
    RESERVED_IRAP_VCL23,

    RESERVED_VCL24,
    RESERVED_VCL25,
    RESERVED_VCL26,
    RESERVED_VCL27,
    RESERVED_VCL28,
    RESERVED_VCL29,
    RESERVED_VCL30,
    RESERVED_VCL31,

    VPS,                    // 32
    SPS,                    // 33
    PPS,                    // 34
    ACCESS_UNIT_DELIMITER,  // 35
    EOS,                    // 36
    EOB,                    // 37
    FILLER_DATA,            // 38
    PREFIX_SEI,             // 39
    SUFFIX_SEI,             // 40
    RESERVED_NVCL41,
    RESERVED_NVCL42,
    RESERVED_NVCL43,
    RESERVED_NVCL44,
    RESERVED_NVCL45,
    RESERVED_NVCL46,
    RESERVED_NVCL47,
    UNSPECIFIED_48,
    UNSPECIFIED_49,
    UNSPECIFIED_50,
    UNSPECIFIED_51,
    UNSPECIFIED_52,
    UNSPECIFIED_53,
    UNSPECIFIED_54,
    UNSPECIFIED_55,
    UNSPECIFIED_56,
    UNSPECIFIED_57,
    UNSPECIFIED_58,
    UNSPECIFIED_59,
    UNSPECIFIED_60,
    UNSPECIFIED_61,
    UNSPECIFIED_62,
    UNSPECIFIED_63,
    INVALID
};

#endif
