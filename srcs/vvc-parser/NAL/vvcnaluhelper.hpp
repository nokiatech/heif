/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef VVCNALUHELPER_HPP
#define VVCNALUHELPER_HPP

#include <cstdint>
#include <vector>
#include "../vvcparser.hpp"
#include "vvcrbspreader.hpp"

/**
 * @brief Parses common VVC NAL sub-syntax elements (ITU-T H.266 / ISO/IEC 23090-3):
 *  - general_constraints_info()      - 7.3.3.2
 *  - profile_tier_level()            - 7.3.3.1
 *  - dpb_parameters()                - 7.3.3.5
 *  - ref_pic_list_struct()           - 7.3.10
 *  - general_timing_hrd_parameters() - 7.3.5.1
 *  - ols_timing_hrd_parameters()     - 7.3.5.2 (skip-only)
 */
class VvcNaluHelper
{
public:
    /**
     * @brief Parse a profile_tier_level() syntax structure (ITU-T H.266 7.3.3.1)
     *        and populate a VvcPTLRecord.
     *
     * @param reader                  RBSP reader positioned at the start of the
     *                                profile_tier_level() syntax element.
     * @param profileTierPresentFlag  Whether profile/tier/general-constraints
     *                                fields are present (always 1 for SPS, may
     *                                be 0 for inner VPS entries).
     * @param maxNumSublayersMinus1   sps_max_sublayers_minus1 or equivalent.
     * @return Populated VvcPTLRecord.
     */
    static VvcParser::VvcPTLRecord parseProfileTierLevel(
        VvcRbspReader& reader,
        bool profileTierPresentFlag,
        uint32_t maxNumSublayersMinus1);

    /**
     * @brief Parse a general_constraints_info() syntax structure (ITU-T H.266 7.3.3.2)
     *        into ptlOut.
     *
     * Called internally by parseProfileTierLevel() when gci_present_flag == 1,
     * but also callable standalone if needed.
     *
     * @param reader  RBSP reader positioned at the start of general_constraints_info().
     * @param ptlOut  PTL record to fill the GCI fields into.
     */
    static void parseGeneralConstraintsInfo(VvcRbspReader& reader,
                                            VvcParser::VvcPTLRecord& ptlOut);

    /**
     * @brief Per-sublayer DPB sizes as signalled by dpb_parameters() (ITU-T H.266 7.3.3.5).
     */
    struct DpbParameters
    {
        /// dpb_max_dec_pic_buffering_minus1[ i ] for i = 0..maxSubLayersMinus1
        std::vector<uint32_t> maxDecPicBufferingMinus1;
        /// dpb_max_num_reorder_pics[ i ]
        std::vector<uint32_t> maxNumReorderPics;
        /// dpb_max_latency_increase_plus1[ i ]
        std::vector<uint32_t> maxLatencyIncreasePlus1;
    };

    /**
     * @brief Parse a dpb_parameters() syntax element.
     *
     * @param reader              RBSP reader positioned at the start of dpb_parameters().
     * @param maxSubLayersMinus1  Number of sublayers minus 1.
     * @param subLayerInfoFlag    Whether per-sublayer information is present.
     * @return Populated DpbParameters.
     */
    static DpbParameters parseDpbParameters(VvcRbspReader& reader,
                                            uint32_t maxSubLayersMinus1,
                                            bool subLayerInfoFlag);

    /**
     * @brief Parse (and discard) a ref_pic_list_struct() syntax element
     *        (ITU-T H.266 7.3.10). Advances the reader over the structure;
     *        extend if reference picture list information is needed later.
     *
     * @param reader                          RBSP reader positioned at the start of ref_pic_list_struct().
     * @param listIdx                         Which RPL list (0 or 1).
     * @param rplsIdx                         Index of the RPL structure in the SPS list.
     * @param spsNumRefPicLists               sps_num_ref_pic_lists[ listIdx ].
     * @param spsLongTermRefPicsFlag          sps_long_term_ref_pics_flag.
     * @param spsInterLayerPredictionEnabled  sps_inter_layer_prediction_enabled_flag.
     * @param log2MaxPicOrderCntLsbMinus4     sps_log2_max_pic_order_cnt_lsb_minus4.
     */
    static void skipRefPicListStruct(VvcRbspReader& reader,
                                     uint32_t listIdx,
                                     uint32_t rplsIdx,
                                     uint32_t spsNumRefPicLists,
                                     bool spsLongTermRefPicsFlag,
                                     bool spsInterLayerPredictionEnabled,
                                     uint32_t log2MaxPicOrderCntLsbMinus4);

    /**
     * @brief Parsed state from general_timing_hrd_parameters() (ITU-T H.266 7.3.5.1)
     *        needed by ols_timing_hrd_parameters() / sublayer_hrd_parameters().
     */
    struct GeneralTimingHrdInfo
    {
        bool generalNalHrdParamsPresentFlag  = false;
        bool generalVclHrdParamsPresentFlag  = false;
        bool generalDuHrdParamsPresentFlag   = false;
        uint32_t hrdCpbCntMinus1             = 0;
    };

    /**
     * @brief Parse a general_timing_hrd_parameters() syntax element and
     *        return the flags needed by subsequent HRD sub-syntax elements.
     *
     * @param reader  RBSP reader positioned at the start of the element.
     * @return GeneralTimingHrdInfo with the parsed flag values.
     */
    static GeneralTimingHrdInfo parseGeneralTimingHrdParameters(VvcRbspReader& reader);

    /**
     * @brief Skip (advance the reader past) an ols_timing_hrd_parameters()
     *        syntax element (ITU-T H.266 7.3.5.2).
     *
     * @param reader              RBSP reader positioned at the start.
     * @param firstSubLayer       First sublayer index.
     * @param maxSubLayersMinus1  Last sublayer index.
     * @param hrdInfo             Parsed general_timing_hrd_parameters() context.
     */
    static void skipOlsTimingHrdParameters(VvcRbspReader& reader,
                                           uint32_t firstSubLayer,
                                           uint32_t maxSubLayersMinus1,
                                           const GeneralTimingHrdInfo& hrdInfo);

private:
    /**
     * @brief Skip a sublayer_hrd_parameters() syntax element (Section 7.3.5.3).
     */
    static void skipSublayerHrdParameters(VvcRbspReader& reader,
                                          const GeneralTimingHrdInfo& hrdInfo);

    // Prevent instantiation - this is a pure static helper class.
public:
    VvcNaluHelper() = delete;
};

#endif /* VVCNALUHELPER_HPP */
