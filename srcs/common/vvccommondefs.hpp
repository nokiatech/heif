/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef VVCCOMMONDEFS_HPP
#define VVCCOMMONDEFS_HPP

#include <cstdint>

#include "customallocator.hpp"

/** @brief Common enumeration definitions for decoder configuration record */
enum class VvcNalUnitType : std::uint8_t
{
    TRAIL_NUT = 0,  // 0
    STSA_NUT,       // 1
    RADL_NUT,       // 2
    RASL_NUT,       // 3

    RSV_VCL_4,  // 4
    RSV_VCL_5,  // 5
    RSV_VCL_6,  // 6

    IDR_W_RADL,  // 7
    IDR_N_LP,    // 8

    CRA_NUT,  // 9
    GDR_NUT,
    RSV_IRAP_11,
    OPI_NUT,
    DCI_NUT,
    VPS_NUT,
    SPS_NUT,
    PPS_NUT,  // 16

    PREFIX_APS_NUT,  // 17
    SUFFIX_APS_NUT,  // 18

    PH_NUT,   // 19
    AUD_NUT,  // 20 Access Unit delimiter
    EOS_NUT,  // 21
    EOB_NUT,

    PREFIX_SEI_NUT,
    SUFFIX_SEI_NUT,

    FD_NUT,

    RSV_NVCL_26,
    RSV_NVCL_27,

    UNSPEC_28,
    UNSPEC_29,
    UNSPEC_30,
    UNSPEC_31,

    UNSPECIFIED_32,
    UNSPECIFIED_33,
    UNSPECIFIED_34,
    UNSPECIFIED_35,
    UNSPECIFIED_36,
    UNSPECIFIED_37,
    UNSPECIFIED_38,
    UNSPECIFIED_39,
    UNSPECIFIED_40,
    UNSPECIFIED_41,
    UNSPECIFIED_42,
    UNSPECIFIED_43,
    UNSPECIFIED_44,
    UNSPECIFIED_45,
    UNSPECIFIED_46,
    UNSPECIFIED_47,
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
