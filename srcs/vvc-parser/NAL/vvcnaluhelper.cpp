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

#include "vvcnaluhelper.hpp"
#include "../vvcparser.hpp"
#include "vvcrbspreader.hpp"
#include <stddef.h>

// general_constraints_info()
// ITU-T H.266 7.3.3.2

void VvcNaluHelper::parseGeneralConstraintsInfo(VvcRbspReader& reader,
                                                VvcParser::VvcPTLRecord& ptl)
{
    ptl.mGciPresentFlag = reader.readFlag();  //bit(1) gci_present_flag

    if (!ptl.mGciPresentFlag)
    {
        // gci_alignment_zero_bit(s) - byte-align and return
        reader.byteAlign();
        return;
    }

    //General
    ptl.mIntraOnlyConstraintFlag = reader.readFlag();  //bit(1) gci_intra_only_constraint_flag
    ptl.mAllLayersIndependentConstraintFlag = reader.readFlag();  //bit(1) gci_all_layers_independent_constraint_flag
    ptl.mOneAuOnlyConstraintFlag = reader.readFlag();  //bit(1) gci_one_au_only_constraint_flag

    //Picture format
    ptl.mSixteenMinusMaxBitDepthConstraintIdc = reader.readBits(4);  //bit(4) gci_sixteen_minus_max_bit_depth_constraint_idc
    ptl.mThreeMinusMaxChromaFormatConstraintIdc = reader.readBits(2);  //bit(2) gci_three_minus_max_chroma_format_constraint_idc

    //NAL unit type related
    ptl.mNoMixedNaluTypesInPicConstraintFlag = reader.readFlag();  //bit(1) gci_no_mixed_nalu_types_in_pic_constraint_flag
    ptl.mNoTrailConstraintFlag = reader.readFlag();  //bit(1) gci_no_trail_constraint_flag
    ptl.mNoStsaConstraintFlag = reader.readFlag();  //bit(1) gci_no_stsa_constraint_flag
    ptl.mNoRaslConstraintFlag = reader.readFlag();  //bit(1) gci_no_rasl_constraint_flag
    ptl.mNoRadlConstraintFlag = reader.readFlag();  //bit(1) gci_no_radl_constraint_flag
    ptl.mNoIdrConstraintFlag = reader.readFlag();  //bit(1) gci_no_idr_constraint_flag
    ptl.mNoCraConstraintFlag = reader.readFlag();  //bit(1) gci_no_cra_constraint_flag
    ptl.mNoGdrConstraintFlag = reader.readFlag();  //bit(1) gci_no_gdr_constraint_flag
    ptl.mNoApsConstraintFlag = reader.readFlag();  //bit(1) gci_no_aps_constraint_flag
    ptl.mNoIdrRplConstraintFlag = reader.readFlag();  //bit(1) gci_no_idr_rpl_constraint_flag

    //Tile, slice, subpicture partitioning
    ptl.mOneTilePerPicConstraintFlag = reader.readFlag();  //bit(1) gci_one_tile_per_pic_constraint_flag
    ptl.mPicHeaderInSliceHeaderConstraintFlag = reader.readFlag();  //bit(1) gci_pic_header_in_slice_header_constraint_flag
    ptl.mOneSlicePerPicConstraintFlag = reader.readFlag();  //bit(1) gci_one_slice_per_pic_constraint_flag
    ptl.mNoRectSliceConstraintFlag = reader.readFlag();  //bit(1) gci_no_rect_slice_constraint_flag
    ptl.mOneSlicePerSubpicConstraintFlag = reader.readFlag();  //bit(1) gci_one_slice_per_subpic_constraint_flag
    ptl.mNoSubpicInfoConstraintFlag = reader.readFlag();  //bit(1) gci_no_subpic_info_constraint_flag

    //CTU and block partitioning
    ptl.mThreeMinusMaxLog2CtuSizeConstraintIdc = reader.readBits(2);  //bit(2) gci_three_minus_max_log2_ctu_size_constraint_idc
    ptl.mNoPartitionConstraintsOverrideConstraintFlag = reader.readFlag();  //bit(1) gci_no_partition_constraints_override_constraint_flag
    ptl.mNoMttConstraintFlag = reader.readFlag();  //bit(1) gci_no_mtt_constraint_flag
    ptl.mNoQtbttDualTreeIntraConstraintFlag = reader.readFlag();  //bit(1) gci_no_qtbtt_dual_tree_intra_constraint_flag

    //Intra
    ptl.mNoPaletteConstraintFlag = reader.readFlag();  //bit(1) gci_no_palette_constraint_flag
    ptl.mNoIbcConstraintFlag = reader.readFlag();  //bit(1) gci_no_ibc_constraint_flag
    ptl.mNoIspConstraintFlag = reader.readFlag();  //bit(1) gci_no_isp_constraint_flag
    ptl.mNoMrlConstraintFlag = reader.readFlag();  //bit(1) gci_no_mrl_constraint_flag
    ptl.mNoMipConstraintFlag = reader.readFlag();  //bit(1) gci_no_mip_constraint_flag
    ptl.mNoCclmConstraintFlag = reader.readFlag();  //bit(1) gci_no_cclm_constraint_flag

    //Inter
    ptl.mNoRefPicResamplingConstraintFlag = reader.readFlag();  //bit(1) gci_no_ref_pic_resampling_constraint_flag
    ptl.mNoResChangeInClvsConstraintFlag = reader.readFlag();  //bit(1) gci_no_res_change_in_clvs_constraint_flag
    ptl.mNoWeightedPredictionConstraintFlag = reader.readFlag();  //bit(1) gci_no_weighted_prediction_constraint_flag
    ptl.mNoRefWraparoundConstraintFlag = reader.readFlag();  //bit(1) gci_no_ref_wraparound_constraint_flag
    ptl.mNoTemporalMvpConstraintFlag = reader.readFlag();  //bit(1) gci_no_temporal_mvp_constraint_flag
    ptl.mNoSbtmvpConstraintFlag = reader.readFlag();  //bit(1) gci_no_sbtmvp_constraint_flag
    ptl.mNoAmvrConstraintFlag = reader.readFlag();  //bit(1) gci_no_amvr_constraint_flag
    ptl.mNoBdofConstraintFlag = reader.readFlag();  //bit(1) gci_no_bdof_constraint_flag
    ptl.mNoSmvdConstraintFlag = reader.readFlag();  //bit(1) gci_no_smvd_constraint_flag
    ptl.mNoDmvrConstraintFlag = reader.readFlag();  //bit(1) gci_no_dmvr_constraint_flag
    ptl.mNoMmvdConstraintFlag = reader.readFlag();  //bit(1) gci_no_mmvd_constraint_flag
    ptl.mNoAffineMotionConstraintFlag = reader.readFlag();  //bit(1) gci_no_affine_motion_constraint_flag
    ptl.mNoProfConstraintFlag = reader.readFlag();  //bit(1) gci_no_prof_constraint_flag
    ptl.mNoBcwConstraintFlag = reader.readFlag();  //bit(1) gci_no_bcw_constraint_flag
    ptl.mNoCiipConstraintFlag = reader.readFlag();  //bit(1) gci_no_ciip_constraint_flag
    ptl.mNoGpmConstraintFlag = reader.readFlag();  //bit(1) gci_no_gpm_constraint_flag

    //Transform, quantization, residual
    ptl.mNoLumaTransformSize64ConstraintFlag = reader.readFlag();  //bit(1) gci_no_luma_transform_size_64_constraint_flag
    ptl.mNoTransformSkipConstraintFlag = reader.readFlag();  //bit(1) gci_no_transform_skip_constraint_flag
    ptl.mNoBDPCMConstraintFlag = reader.readFlag();  //bit(1) gci_no_bdpcm_constraint_flag
    ptl.mNoMtsConstraintFlag = reader.readFlag();  //bit(1) gci_no_mts_constraint_flag
    ptl.mNoLfnstConstraintFlag = reader.readFlag();  //bit(1) gci_no_lfnst_constraint_flag
    ptl.mNoJointCbCrConstraintFlag = reader.readFlag();  //bit(1) gci_no_joint_cbcr_constraint_flag
    ptl.mNoSbtConstraintFlag = reader.readFlag();  //bit(1) gci_no_sbt_constraint_flag
    ptl.mNoActConstraintFlag = reader.readFlag();  //bit(1) gci_no_act_constraint_flag
    ptl.mNoExplicitScaleListConstraintFlag = reader.readFlag();  //bit(1) gci_no_explicit_scale_list_constraint_flag
    ptl.mNoDepQuantConstraintFlag = reader.readFlag();  //bit(1) gci_no_dep_quant_constraint_flag
    ptl.mNoSignDataHidingConstraintFlag = reader.readFlag();  //bit(1) gci_no_sign_data_hiding_constraint_flag
    ptl.mNoCuQpDeltaConstraintFlag = reader.readFlag();  //bit(1) gci_no_cu_qp_delta_constraint_flag
    ptl.mNoChromaQpOffsetConstraintFlag = reader.readFlag();  //bit(1) gci_no_chroma_qp_offset_constraint_flag

    //Loop filter
    ptl.mNoSaoConstraintFlag = reader.readFlag();  //bit(1) gci_no_sao_constraint_flag
    ptl.mNoAlfConstraintFlag = reader.readFlag();  //bit(1) gci_no_alf_constraint_flag
    ptl.mNoCCAlfConstraintFlag = reader.readFlag();  //bit(1) gci_no_ccalf_constraint_flag
    ptl.mNoLmcsConstraintFlag = reader.readFlag();  //bit(1) gci_no_lmcs_constraint_flag
    ptl.mNoLadfConstraintFlag = reader.readFlag();  //bit(1) gci_no_ladf_constraint_flag
    ptl.mNoVirtualBoundariesConstraintFlag = reader.readFlag();  //bit(1) gci_no_virtual_boundaries_constraint_flag

    uint32_t gci_num_reserved_bits = reader.readBits(8);  //bit(8) gci_num_reserved_bits
    // Skip reserved GCI bits
    for (uint32_t i = 0; i < gci_num_reserved_bits; i++)
    {
        reader.readFlag();  //bit(1) gci_reserved_zero_bit
    }

    // gci_alignment_zero_bit(s) - byte-align
    reader.byteAlign();
}

// profile_tier_level( profileTierPresentFlag, maxNumSublayersMinus1 )
// ITU-T H.266 7.3.3.1

VvcParser::VvcPTLRecord VvcNaluHelper::parseProfileTierLevel(
    VvcRbspReader& reader,
    bool profileTierPresentFlag,
    uint32_t maxNumSublayersMinus1)
{
    VvcParser::VvcPTLRecord ptl = {};

    if (profileTierPresentFlag)
    {
        ptl.mGeneralProfileIdc = static_cast<uint8_t>(reader.readBits(7));  //bit(7) general_profile_idc
        ptl.mGeneralTierFlag = static_cast<uint8_t>(reader.readFlag() ? 1 : 0);  //bit(1) general_tier_flag
    }

    ptl.mGeneralLevelIdc = static_cast<uint8_t>(reader.readBits(8));  //bit(8) general_level_idc

    ptl.mPtlFrameOnlyConstraintFlag = static_cast<uint8_t>(reader.readFlag() ? 1 : 0);  //bit(1) ptl_frame_only_constraint_flag

    ptl.mPtlMultilayerEnabledFlag = static_cast<uint8_t>(reader.readFlag() ? 1 : 0);  //bit(1) ptl_multilayer_enabled_flag

    if (profileTierPresentFlag)
    {
        // general_constraints_info()
        parseGeneralConstraintsInfo(reader, ptl);
    }

    // Sublayer level present flags
    // ptl_sublayer_level_present_flag[ i ] for i = maxNumSublayersMinus1-1 .. 0
    ptl.mPtlSublayerLevelPresentFlag.resize(maxNumSublayersMinus1, false);
    for (int32_t i = static_cast<int32_t>(maxNumSublayersMinus1) - 1; i >= 0; --i)
    {
        ptl.mPtlSublayerLevelPresentFlag[static_cast<size_t>(i)] = reader.readFlag();  //bit(1) ptl_sublayer_level_present_flag
    }

    // while( !byte_aligned( ) )
    //   ptl_reserved_zero_bit  u(1)
    // Per spec Section 7.3.3.1: pad to byte boundary after sublayer level present flags.
    reader.byteAlign();

    // Sublayer level IDC values
    ptl.mSublayerLevelIdc.resize(maxNumSublayersMinus1 + 1, ptl.mGeneralLevelIdc);
    for (int32_t i = static_cast<int32_t>(maxNumSublayersMinus1) - 1; i >= 0; --i)
    {
        if (ptl.mPtlSublayerLevelPresentFlag[static_cast<size_t>(i)])
        {
            // ptl_sublayer_level_idc[ i ] u(8)
            ptl.mSublayerLevelIdc[static_cast<size_t>(i)] =
                static_cast<uint8_t>(reader.readBits(8));
        }
        else
        {
            // Infer: same as the level of the next higher sublayer
            ptl.mSublayerLevelIdc[static_cast<size_t>(i)] =
                ptl.mSublayerLevelIdc[static_cast<size_t>(i) + 1];
        }
    }

    // ptl_num_sub_profiles and general_sub_profile_idc are only present
    // when profileTierPresentFlag is true (ITU-T H.266 Section 7.3.3.1)
    if (profileTierPresentFlag)
    {
        ptl.mNumSubProfiles = static_cast<uint8_t>(reader.readBits(8));  //bit(8) ptl_num_sub_profiles
        ptl.mGeneralSubProfileIdc.resize(ptl.mNumSubProfiles);
        for (uint32_t j = 0; j < ptl.mNumSubProfiles; ++j)
        {
            ptl.mGeneralSubProfileIdc[j] = reader.readBits(32);  //bit(32) general_sub_profile_idc
        }
    }

    return ptl;
}

// dpb_parameters( maxSubLayersMinus1, subLayerInfoFlag )
// ITU-T H.266 7.3.3.5

VvcNaluHelper::DpbParameters VvcNaluHelper::parseDpbParameters(
    VvcRbspReader& reader,
    uint32_t maxSubLayersMinus1,
    bool subLayerInfoFlag)
{
    DpbParameters dpb;
    dpb.maxDecPicBufferingMinus1.resize(maxSubLayersMinus1 + 1, 0);
    dpb.maxNumReorderPics.resize(maxSubLayersMinus1 + 1, 0);
    dpb.maxLatencyIncreasePlus1.resize(maxSubLayersMinus1 + 1, 0);

    // When subLayerInfoFlag == false only the highest sublayer entry is coded;
    // the spec says to infer lower sublayer values equal to the highest one.
    uint32_t firstLayer = subLayerInfoFlag ? 0 : maxSubLayersMinus1;

    for (uint32_t i = firstLayer; i <= maxSubLayersMinus1; ++i)
    {
        dpb.maxDecPicBufferingMinus1[i] = reader.readUE();  //ue(v) dpb_max_dec_pic_buffering_minus1
        dpb.maxNumReorderPics[i] = reader.readUE();  //ue(v) dpb_max_num_reorder_pics
        dpb.maxLatencyIncreasePlus1[i] = reader.readUE();  //ue(v) dpb_max_latency_increase_plus1
    }

    // Propagate highest-sublayer values down to lower sublayers when not present
    if (!subLayerInfoFlag && maxSubLayersMinus1 > 0)
    {
        for (uint32_t i = 0; i < maxSubLayersMinus1; ++i)
        {
            dpb.maxDecPicBufferingMinus1[i] = dpb.maxDecPicBufferingMinus1[maxSubLayersMinus1];
            dpb.maxNumReorderPics[i]        = dpb.maxNumReorderPics[maxSubLayersMinus1];
            dpb.maxLatencyIncreasePlus1[i]  = dpb.maxLatencyIncreasePlus1[maxSubLayersMinus1];
        }
    }

    return dpb;
}

// ref_pic_list_struct( listIdx, rplsIdx )
// ITU-T H.266 7.3.7.4
// Only advances the reader - extend as needed to capture RPL data.

void VvcNaluHelper::skipRefPicListStruct(VvcRbspReader& reader,
                                         uint32_t /*listIdx*/,
                                         uint32_t rplsIdx,
                                         uint32_t spsNumRefPicLists,
                                         bool spsLongTermRefPicsFlag,
                                         bool spsInterLayerPredictionEnabled,
                                         uint32_t log2MaxPicOrderCntLsbMinus4)
{
    // ITU-T H.266 Section 7.3.10  ref_pic_list_struct( listIdx, rplsIdx )

    uint32_t numRefEntries = reader.readUE();  //ue(v) num_ref_entries

    // ltrp_in_header_flag[ listIdx ][ rplsIdx ] u(1)
    // Present when: sps_long_term_ref_pics_flag && rplsIdx < sps_num_ref_pic_lists[ listIdx ]
    //               && num_ref_entries[ listIdx ][ rplsIdx ] > 0
    bool ltrpInHeaderFlag = true;  // inferred as 1 when not present
    if (spsLongTermRefPicsFlag && rplsIdx < spsNumRefPicLists && numRefEntries > 0)
    {
        ltrpInHeaderFlag = reader.readFlag();
    }

    uint32_t pocLsbLtBits = log2MaxPicOrderCntLsbMinus4 + 4;

    for (uint32_t i = 0; i < numRefEntries; ++i)
    {
        // inter_layer_ref_pic_flag[ listIdx ][ rplsIdx ][ i ] u(1)
        // Present only when sps_inter_layer_prediction_enabled_flag; inferred 0 otherwise.
        bool interLayerRefPicFlag = false;
        if (spsInterLayerPredictionEnabled)
        {
            interLayerRefPicFlag = reader.readFlag();
        }

        if (!interLayerRefPicFlag)
        {
            // st_ref_pic_flag[ listIdx ][ rplsIdx ][ i ] u(1)
            // Present only when sps_long_term_ref_pics_flag; inferred 1 otherwise.
            bool stRefPicFlag = true;
            if (spsLongTermRefPicsFlag)
            {
                stRefPicFlag = reader.readFlag();
            }

            if (stRefPicFlag)
            {
                uint32_t absDeltaPocSt = reader.readUE();  //ue(v) abs_delta_poc_st

                // Derive AbsDeltaPocSt per spec Section 7.4.11:
                // If rplsIdx < sps_num_ref_pic_lists[listIdx]:
                //   AbsDeltaPocSt = abs_delta_poc_st + 1
                // Else (rplsIdx == sps_num_ref_pic_lists[listIdx], i.e. in header):
                //   AbsDeltaPocSt = abs_delta_poc_st
                uint32_t AbsDeltaPocSt = (rplsIdx < spsNumRefPicLists)
                                           ? (absDeltaPocSt + 1)
                                           : absDeltaPocSt;

                // strp_entry_sign_flag[ listIdx ][ rplsIdx ][ i ] u(1)
                // Present only when AbsDeltaPocSt > 0
                if (AbsDeltaPocSt > 0)
                {
                    reader.readFlag();
                }
            }
            else
            {
                // Long-term ref pic
                if (!ltrpInHeaderFlag)
                {
                    // rpls_poc_lsb_lt[ listIdx ][ rplsIdx ][ j++ ] u(v)
                    // Bit-width = sps_log2_max_pic_order_cnt_lsb_minus4 + 4
                    reader.readBits(pocLsbLtBits);
                }
            }
        }
        else
        {
            reader.readUE();  //ue(v) ilrp_idx
        }
    }
}

// general_timing_hrd_parameters()
// ITU-T H.266 7.3.5.1
// Advances the reader past the fixed-size fields; sub_layer_hrd_parameters
// are handled in skipOlsTimingHrdParameters.

VvcNaluHelper::GeneralTimingHrdInfo VvcNaluHelper::parseGeneralTimingHrdParameters(VvcRbspReader& reader)
{
    GeneralTimingHrdInfo info;

    reader.readBits(32);  //bit(32) num_units_in_tick
    reader.readBits(32);  //bit(32) time_scale

    info.generalNalHrdParamsPresentFlag = reader.readFlag();  //bit(1) general_nal_hrd_params_present_flag
    info.generalVclHrdParamsPresentFlag = reader.readFlag();  //bit(1) general_vcl_hrd_params_present_flag

    if (info.generalNalHrdParamsPresentFlag || info.generalVclHrdParamsPresentFlag)
    {
        reader.readFlag();  //bit(1) general_same_pic_timing_in_all_ols_flag
        info.generalDuHrdParamsPresentFlag = reader.readFlag();  //bit(1) general_du_hrd_params_present_flag
        if (info.generalDuHrdParamsPresentFlag)
        {
            reader.readBits(8);  //bit(8) tick_divisor_minus2
        }
        reader.readBits(4);  //bit(4) bit_rate_scale
        reader.readBits(4);  //bit(4) cpb_size_scale
        if (info.generalDuHrdParamsPresentFlag)
        {
            reader.readBits(4);  //bit(4) cpb_size_du_scale
        }
        info.hrdCpbCntMinus1 = reader.readUE();  //ue(v) hrd_cpb_cnt_minus1
    }

    return info;
}

// sublayer_hrd_parameters( subLayerId )
// ITU-T H.266 7.3.5.3

void VvcNaluHelper::skipSublayerHrdParameters(VvcRbspReader& reader,
                                              const GeneralTimingHrdInfo& hrdInfo)
{
    for (uint32_t j = 0; j <= hrdInfo.hrdCpbCntMinus1; ++j)
    {
        reader.readUE();  //ue(v) bit_rate_value_minus1
        reader.readUE();  //ue(v) cpb_size_value_minus1
        if (hrdInfo.generalDuHrdParamsPresentFlag)
        {
            reader.readUE();  //ue(v) cpb_size_du_value_minus1
            reader.readUE();  //ue(v) bit_rate_du_value_minus1
        }
        reader.readFlag();  //bit(1) cbr_flag
    }
}

// ols_timing_hrd_parameters( firstSubLayer, maxSubLayersVal )
// ITU-T H.266 7.3.5.2

// firstSubLayer / maxSubLayersMinus1 are H.266 spec field names from ITU-T 7.3.5.2;
// renaming or wrapping them would obscure the 1:1 mapping to the standard pseudocode.
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
void VvcNaluHelper::skipOlsTimingHrdParameters(VvcRbspReader& reader,
                                                uint32_t firstSubLayer,
                                                uint32_t maxSubLayersMinus1,
                                                const GeneralTimingHrdInfo& hrdInfo)
// NOLINTEND(bugprone-easily-swappable-parameters)
{
    for (uint32_t i = firstSubLayer; i <= maxSubLayersMinus1; ++i)
    {
        bool fixedPicRateGeneralFlag = reader.readFlag();  //bit(1) fixed_pic_rate_general_flag
        bool fixedPicRateWithinCvsFlag = fixedPicRateGeneralFlag;  // inferred equal when not present

        if (!fixedPicRateGeneralFlag)
        {
            fixedPicRateWithinCvsFlag = reader.readFlag();  //bit(1) fixed_pic_rate_within_cvs_flag
        }

        if (fixedPicRateWithinCvsFlag)
        {
            reader.readUE();  //ue(v) elemental_duration_in_tc_minus1
        }
        else if ((hrdInfo.generalNalHrdParamsPresentFlag ||
                  hrdInfo.generalVclHrdParamsPresentFlag) &&
                 hrdInfo.hrdCpbCntMinus1 == 0)
        {
            reader.readFlag();  //bit(1) low_delay_hrd_flag
        }

        // sublayer_hrd_parameters( i ) - called once for NAL, once for VCL
        if (hrdInfo.generalNalHrdParamsPresentFlag)
        {
            skipSublayerHrdParameters(reader, hrdInfo);
        }
        if (hrdInfo.generalVclHrdParamsPresentFlag)
        {
            skipSublayerHrdParameters(reader, hrdInfo);
        }
    }
}
