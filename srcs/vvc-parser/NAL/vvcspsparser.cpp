#include "vvcspsparser.hpp"
#include "vvcnaluhelper.hpp"
#include "heiferrorinfo.h"
#include "../vvcparser.hpp"
#include "heifcommondatatypes.h"
#include "vvcrbspreader.hpp"
#include <exception>

VvcParser::SpsInfo VvcSpsParser::parse(const Vector<uint8_t>& spsNalu) {
    VvcParser::SpsInfo info = {};
    
    VvcRbspReader reader;
    if (!reader.prepare(spsNalu)) {
        HEIF_RECORD(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                    ::HEIF::ErrorOrigin::Parser_VvcRbsp,
                    "VvcSpsParser::parse: VvcRbspReader::prepare rejected SPS NAL (too short or NAL header invalid)");
        return info;  // Error case
    }

    try
    {

    // sps_seq_parameter_set_id u(4)
    info.spsId = static_cast<int>(reader.readBits(4));

    // sps_video_parameter_set_id u(4)
    uint32_t sps_video_parameter_set_id = reader.readBits(4);

    // Set OLS index from NAL header nuh_layer_id
    info.mOlsIdx = reader.getNalHeader().nuhLayerId;

    // sps_max_sublayers_minus1 u(3)
    uint32_t sps_max_sublayers_minus1 = reader.readBits(3);
    info.mNumSublayers = static_cast<uint8_t>(sps_max_sublayers_minus1 + 1);

    // sps_chroma_format_idc u(2)
    uint32_t sps_chroma_format_idc = reader.readBits(2);
    info.mChromaFormatIdc = static_cast<uint8_t>(sps_chroma_format_idc);

    // sps_log2_ctu_size_minus5 u(2)
    uint32_t sps_log2_ctu_size_minus5 = reader.readBits(2);
    uint32_t CtbLog2SizeY = sps_log2_ctu_size_minus5 + 5;
    uint32_t CtbSizeY = 1u << CtbLog2SizeY;

    // sps_ptl_dpb_hrd_params_present_flag u(1)
    bool sps_ptl_dpb_hrd_params_present_flag = reader.readFlag();
    info.mPtlPresentFlag = sps_ptl_dpb_hrd_params_present_flag;

    if (sps_ptl_dpb_hrd_params_present_flag) {
        // profile_tier_level( 1, sps_max_sublayers_minus1 )
        info.mNativePtl = VvcNaluHelper::parseProfileTierLevel(
            reader, /*profileTierPresentFlag=*/true, sps_max_sublayers_minus1);
    }

    // sps_gdr_enabled_flag u(1)
    bool sps_gdr_enabled_flag = reader.readFlag();
    (void)sps_gdr_enabled_flag;

    // sps_ref_pic_resampling_enabled_flag u(1)
    bool sps_ref_pic_resampling_enabled_flag = reader.readFlag();

    if (sps_ref_pic_resampling_enabled_flag) {
        // sps_res_change_in_clvs_allowed_flag u(1)
        reader.readFlag();
    }

    // sps_pic_width_max_in_luma_samples ue(v)
    uint32_t sps_pic_width_max_in_luma_samples = reader.readUE();
    info.mMaxPictureWidth = static_cast<uint16_t>(sps_pic_width_max_in_luma_samples);

    // sps_pic_height_max_in_luma_samples ue(v)
    uint32_t sps_pic_height_max_in_luma_samples = reader.readUE();
    info.mMaxPictureHeight = static_cast<uint16_t>(sps_pic_height_max_in_luma_samples);

    // sps_conformance_window_flag u(1)
    bool sps_conformance_window_flag = reader.readFlag();
    if (sps_conformance_window_flag) {
        // sps_conf_win_left_offset ue(v)
        reader.readUE();
        // sps_conf_win_right_offset ue(v)
        reader.readUE();
        // sps_conf_win_top_offset ue(v)
        reader.readUE();
        // sps_conf_win_bottom_offset ue(v)
        reader.readUE();
    }

    // sps_subpic_info_present_flag u(1)
    bool sps_subpic_info_present_flag = reader.readFlag();

    uint32_t sps_num_subpics_minus1 = 0;
    bool sps_independent_subpics_flag = true;   // inferred value when not present
    bool sps_subpic_same_size_flag = false;

    if (sps_subpic_info_present_flag) {
        // sps_num_subpics_minus1 ue(v)
        sps_num_subpics_minus1 = reader.readUE();

        if (sps_num_subpics_minus1 > 0) {
            // sps_independent_subpics_flag u(1)
            sps_independent_subpics_flag = reader.readFlag();
            // sps_subpic_same_size_flag u(1)
            sps_subpic_same_size_flag = reader.readFlag();
        }

        // Derived variable: number of bits for CTU position/size fields
        uint32_t tmpWidthVal = (sps_pic_width_max_in_luma_samples + CtbSizeY - 1) / CtbSizeY;
        uint32_t tmpHeightVal = (sps_pic_height_max_in_luma_samples + CtbSizeY - 1) / CtbSizeY;
        // Ceil(Log2(x)): number of bits needed to represent values 0..x-1
        auto ceilLog2 = [](uint32_t x) -> uint32_t {
            uint32_t bits = 0;
            uint32_t val = x - 1;
            while (val > 0) { val >>= 1; bits++; }
            return bits;
        };
        uint32_t wBits = ceilLog2(tmpWidthVal);
        uint32_t hBits = ceilLog2(tmpHeightVal);

        for (uint32_t i = 0; sps_num_subpics_minus1 > 0 && i <= sps_num_subpics_minus1; i++) {
            if (!sps_subpic_same_size_flag || i == 0) {
                if (i > 0 && sps_pic_width_max_in_luma_samples > CtbSizeY) {
                    // sps_subpic_ctu_top_left_x[ i ] u(v)
                    reader.readBits(wBits);
                }
                if (i > 0 && sps_pic_height_max_in_luma_samples > CtbSizeY) {
                    // sps_subpic_ctu_top_left_y[ i ] u(v)
                    reader.readBits(hBits);
                }
                if (i < sps_num_subpics_minus1 &&
                    sps_pic_width_max_in_luma_samples > CtbSizeY) {
                    // sps_subpic_width_minus1[ i ] u(v)
                    reader.readBits(wBits);
                }
                if (i < sps_num_subpics_minus1 &&
                    sps_pic_height_max_in_luma_samples > CtbSizeY) {
                    // sps_subpic_height_minus1[ i ] u(v)
                    reader.readBits(hBits);
                }
            }
            if (!sps_independent_subpics_flag) {
                // sps_subpic_treated_as_pic_flag[ i ] u(1)
                reader.readFlag();
                // sps_loop_filter_across_subpic_enabled_flag[ i ] u(1)
                reader.readFlag();
            }
        }

        // sps_subpic_id_len_minus1 ue(v)
        uint32_t sps_subpic_id_len_minus1 = reader.readUE();
        info.subpicIdLen = sps_subpic_id_len_minus1 + 1;

        // sps_subpic_id_mapping_explicitly_signalled_flag u(1)
        bool sps_subpic_id_mapping_explicitly_signalled_flag = reader.readFlag();

        if (sps_subpic_id_mapping_explicitly_signalled_flag) {
            // sps_subpic_id_mapping_present_flag u(1)
            bool sps_subpic_id_mapping_present_flag = reader.readFlag();

            if (sps_subpic_id_mapping_present_flag) {
                info.isSubpicMappingPresent = true;

                // Record bit position for subpic_id array
                info.subpicIdBitPos = reader.getBitsRead();

                // Check for emulation prevention bytes before this position
                info.startCodeEmulationFlag = reader.hasEmulationBytesBefore(info.subpicIdBitPos);

                for (uint32_t i = 0; i <= sps_num_subpics_minus1; i++) {
                    // sps_subpic_id[ i ] u(v)
                    reader.readBits(sps_subpic_id_len_minus1 + 1);
                }
            }
        }
    }

    // sps_bitdepth_minus8 ue(v)
    uint32_t sps_bitdepth_minus8 = reader.readUE();
    info.mBitDepthMinus8 = static_cast<uint8_t>(sps_bitdepth_minus8);

    // sps_entropy_coding_sync_enabled_flag u(1)
    reader.readFlag();

    // sps_entry_point_offsets_present_flag u(1)
    reader.readFlag();

    // sps_log2_max_pic_order_cnt_lsb_minus4 u(4)
    uint32_t sps_log2_max_pic_order_cnt_lsb_minus4 = reader.readBits(4);

    // sps_poc_msb_cycle_flag u(1)
    bool sps_poc_msb_cycle_flag = reader.readFlag();
    if (sps_poc_msb_cycle_flag) {
        // sps_poc_msb_cycle_len_minus1 ue(v)
        reader.readUE();
    }

    // sps_num_extra_ph_bytes u(2)
    uint32_t sps_num_extra_ph_bytes = reader.readBits(2);
    for (uint32_t i = 0; i < sps_num_extra_ph_bytes * 8; i++) {
        // sps_extra_ph_bit_present_flag[ i ] u(1)
        reader.readFlag();
    }

    // sps_num_extra_sh_bytes u(2)
    uint32_t sps_num_extra_sh_bytes = reader.readBits(2);
    for (uint32_t i = 0; i < sps_num_extra_sh_bytes * 8; i++) {
        // sps_extra_sh_bit_present_flag[ i ] u(1)
        reader.readFlag();
    }

    if (sps_ptl_dpb_hrd_params_present_flag) {
        bool sps_sublayer_dpb_params_flag = false;
        if (sps_max_sublayers_minus1 > 0) {
            // sps_sublayer_dpb_params_flag u(1)
            sps_sublayer_dpb_params_flag = reader.readFlag();
        }
        // dpb_parameters( sps_max_sublayers_minus1, sps_sublayer_dpb_params_flag )
        VvcNaluHelper::parseDpbParameters(
            reader, sps_max_sublayers_minus1, sps_sublayer_dpb_params_flag);
    }

    // sps_log2_min_luma_coding_block_size_minus2 ue(v)
    reader.readUE();

    // sps_partition_constraints_override_enabled_flag u(1)
    reader.readFlag();

    // sps_log2_diff_min_qt_min_cb_intra_slice_luma ue(v)
    reader.readUE();

    // sps_max_mtt_hierarchy_depth_intra_slice_luma ue(v)
    uint32_t sps_max_mtt_hierarchy_depth_intra_slice_luma = reader.readUE();
    if (sps_max_mtt_hierarchy_depth_intra_slice_luma != 0) {
        // sps_log2_diff_max_bt_min_qt_intra_slice_luma ue(v)
        reader.readUE();
        // sps_log2_diff_max_tt_min_qt_intra_slice_luma ue(v)
        reader.readUE();
    }

    bool sps_qtbtt_dual_tree_intra_flag = false;
    if (sps_chroma_format_idc != 0) {
        // sps_qtbtt_dual_tree_intra_flag u(1)
        sps_qtbtt_dual_tree_intra_flag = reader.readFlag();
    }
    if (sps_qtbtt_dual_tree_intra_flag) {
        // sps_log2_diff_min_qt_min_cb_intra_slice_chroma ue(v)
        reader.readUE();
        // sps_max_mtt_hierarchy_depth_intra_slice_chroma ue(v)
        uint32_t sps_max_mtt_hierarchy_depth_intra_slice_chroma = reader.readUE();
        if (sps_max_mtt_hierarchy_depth_intra_slice_chroma != 0) {
            // sps_log2_diff_max_bt_min_qt_intra_slice_chroma ue(v)
            reader.readUE();
            // sps_log2_diff_max_tt_min_qt_intra_slice_chroma ue(v)
            reader.readUE();
        }
    }

    // sps_log2_diff_min_qt_min_cb_inter_slice ue(v)
    reader.readUE();

    // sps_max_mtt_hierarchy_depth_inter_slice ue(v)
    uint32_t sps_max_mtt_hierarchy_depth_inter_slice = reader.readUE();
    if (sps_max_mtt_hierarchy_depth_inter_slice != 0) {
        // sps_log2_diff_max_bt_min_qt_inter_slice ue(v)
        reader.readUE();
        // sps_log2_diff_max_tt_min_qt_inter_slice ue(v)
        reader.readUE();
    }

    bool sps_max_luma_transform_size_64_flag = false;
    if (CtbSizeY > 32) {
        // sps_max_luma_transform_size_64_flag u(1)
        sps_max_luma_transform_size_64_flag = reader.readFlag();
    }

    // sps_transform_skip_enabled_flag u(1)
    bool sps_transform_skip_enabled_flag = reader.readFlag();
    if (sps_transform_skip_enabled_flag) {
        // sps_log2_transform_skip_max_size_minus2 ue(v)
        reader.readUE();
        // sps_bdpcm_enabled_flag u(1)
        reader.readFlag();
    }

    // sps_mts_enabled_flag u(1)
    bool sps_mts_enabled_flag = reader.readFlag();
    if (sps_mts_enabled_flag) {
        // sps_explicit_mts_intra_enabled_flag u(1)
        reader.readFlag();
        // sps_explicit_mts_inter_enabled_flag u(1)
        reader.readFlag();
    }

    // sps_lfnst_enabled_flag u(1)
    bool sps_lfnst_enabled_flag = reader.readFlag();

    bool sps_joint_cbcr_enabled_flag = false;
    bool sps_same_qp_table_for_chroma_flag = true;
    if (sps_chroma_format_idc != 0) {
        // sps_joint_cbcr_enabled_flag u(1)
        sps_joint_cbcr_enabled_flag = reader.readFlag();
        // sps_same_qp_table_for_chroma_flag u(1)
        sps_same_qp_table_for_chroma_flag = reader.readFlag();

        uint32_t numQpTables = sps_same_qp_table_for_chroma_flag ? 1
                             : (sps_joint_cbcr_enabled_flag ? 3 : 2);

        for (uint32_t i = 0; i < numQpTables; i++) {
            // sps_qp_table_start_minus26[ i ] se(v)
            reader.readSE();
            // sps_num_points_in_qp_table_minus1[ i ] ue(v)
            uint32_t sps_num_points_in_qp_table_minus1 = reader.readUE();
            for (uint32_t j = 0; j <= sps_num_points_in_qp_table_minus1; j++) {
                // sps_delta_qp_in_val_minus1[ i ][ j ] ue(v)
                reader.readUE();
                // sps_delta_qp_diff_val[ i ][ j ] ue(v)
                reader.readUE();
            }
        }
    }

    // sps_sao_enabled_flag u(1)
    reader.readFlag();

    // sps_alf_enabled_flag u(1)
    bool sps_alf_enabled_flag = reader.readFlag();
    if (sps_alf_enabled_flag && sps_chroma_format_idc != 0) {
        // sps_ccalf_enabled_flag u(1)
        reader.readFlag();
    }

    // sps_lmcs_enabled_flag u(1)
    reader.readFlag();

    // sps_weighted_pred_flag u(1)
    reader.readFlag();

    // sps_weighted_bipred_flag u(1)
    reader.readFlag();

    // sps_long_term_ref_pics_flag u(1)
    bool sps_long_term_ref_pics_flag = reader.readFlag();

    bool sps_inter_layer_prediction_enabled_flag = false;
    if (sps_video_parameter_set_id > 0) {
        // sps_inter_layer_prediction_enabled_flag u(1)
        sps_inter_layer_prediction_enabled_flag = reader.readFlag();
    }

    // sps_idr_rpl_present_flag u(1)
    reader.readFlag();

    // sps_rpl1_same_as_rpl0_flag u(1)
    bool sps_rpl1_same_as_rpl0_flag = reader.readFlag();

    uint32_t numRplLists = sps_rpl1_same_as_rpl0_flag ? 1 : 2;
    Vector<uint32_t> sps_num_ref_pic_lists_arr(2, 0);
    for (uint32_t i = 0; i < numRplLists; i++) {
        // sps_num_ref_pic_lists[ i ] ue(v)
        uint32_t sps_num_ref_pic_lists = reader.readUE();
        sps_num_ref_pic_lists_arr[i] = sps_num_ref_pic_lists;
        for (uint32_t j = 0; j < sps_num_ref_pic_lists; j++) {
            // ref_pic_list_struct( i, j )
            VvcNaluHelper::skipRefPicListStruct(reader, i, j,
                sps_num_ref_pic_lists,
                sps_long_term_ref_pics_flag,
                sps_inter_layer_prediction_enabled_flag,
                sps_log2_max_pic_order_cnt_lsb_minus4);
        }
    }

    // sps_ref_wraparound_enabled_flag u(1)
    reader.readFlag();

    // sps_temporal_mvp_enabled_flag u(1)
    bool sps_temporal_mvp_enabled_flag = reader.readFlag();
    if (sps_temporal_mvp_enabled_flag) {
        // sps_sbtmvp_enabled_flag u(1)
        reader.readFlag();
    }

    // sps_amvr_enabled_flag u(1)
    bool sps_amvr_enabled_flag = reader.readFlag();

    // sps_bdof_enabled_flag u(1)
    bool sps_bdof_enabled_flag = reader.readFlag();
    if (sps_bdof_enabled_flag) {
        // sps_bdof_control_present_in_ph_flag u(1)
        reader.readFlag();
    }

    // sps_smvd_enabled_flag u(1)
    reader.readFlag();

    // sps_dmvr_enabled_flag u(1)
    bool sps_dmvr_enabled_flag = reader.readFlag();
    if (sps_dmvr_enabled_flag) {
        // sps_dmvr_control_present_in_ph_flag u(1)
        reader.readFlag();
    }

    // sps_mmvd_enabled_flag u(1)
    bool sps_mmvd_enabled_flag = reader.readFlag();
    if (sps_mmvd_enabled_flag) {
        // sps_mmvd_fullpel_only_enabled_flag u(1)
        reader.readFlag();
    }

    // sps_six_minus_max_num_merge_cand ue(v)
    uint32_t sps_six_minus_max_num_merge_cand = reader.readUE();
    uint32_t MaxNumMergeCand = 6 - sps_six_minus_max_num_merge_cand;

    // sps_sbt_enabled_flag u(1)
    reader.readFlag();

    // sps_affine_enabled_flag u(1)
    bool sps_affine_enabled_flag = reader.readFlag();
    if (sps_affine_enabled_flag) {
        // sps_five_minus_max_num_subblock_merge_cand ue(v)
        reader.readUE();
        // sps_6param_affine_enabled_flag u(1)
        reader.readFlag();
        if (sps_amvr_enabled_flag) {
            // sps_affine_amvr_enabled_flag u(1)
            reader.readFlag();
        }
        // sps_affine_prof_enabled_flag u(1)
        bool sps_affine_prof_enabled_flag = reader.readFlag();
        if (sps_affine_prof_enabled_flag) {
            // sps_prof_control_present_in_ph_flag u(1)
            reader.readFlag();
        }
    }

    // sps_bcw_enabled_flag u(1)
    reader.readFlag();

    // sps_ciip_enabled_flag u(1)
    reader.readFlag();

    if (MaxNumMergeCand >= 2) {
        // sps_gpm_enabled_flag u(1)
        bool sps_gpm_enabled_flag = reader.readFlag();
        if (sps_gpm_enabled_flag && MaxNumMergeCand >= 3) {
            // sps_max_num_merge_cand_minus_max_num_gpm_cand ue(v)
            reader.readUE();
        }
    }

    // sps_log2_parallel_merge_level_minus2 ue(v)
    reader.readUE();

    // sps_isp_enabled_flag u(1)
    reader.readFlag();

    // sps_mrl_enabled_flag u(1)
    reader.readFlag();

    // sps_mip_enabled_flag u(1)
    reader.readFlag();

    if (sps_chroma_format_idc != 0) {
        // sps_cclm_enabled_flag u(1)
        reader.readFlag();
    }

    if (sps_chroma_format_idc == 1) {
        // sps_chroma_horizontal_collocated_flag u(1)
        reader.readFlag();
        // sps_chroma_vertical_collocated_flag u(1)
        reader.readFlag();
    }

    // sps_palette_enabled_flag u(1)
    bool sps_palette_enabled_flag = reader.readFlag();

    bool sps_act_enabled_flag = false;
    if (sps_chroma_format_idc == 3 && !sps_max_luma_transform_size_64_flag) {
        // sps_act_enabled_flag u(1)
        sps_act_enabled_flag = reader.readFlag();
    }

    if (sps_transform_skip_enabled_flag || sps_palette_enabled_flag) {
        // sps_min_qp_prime_ts ue(v)
        reader.readUE();
    }

    // sps_ibc_enabled_flag u(1)
    bool sps_ibc_enabled_flag = reader.readFlag();
    if (sps_ibc_enabled_flag) {
        // sps_six_minus_max_num_ibc_merge_cand ue(v)
        reader.readUE();
    }

    // sps_ladf_enabled_flag u(1)
    bool sps_ladf_enabled_flag = reader.readFlag();
    if (sps_ladf_enabled_flag) {
        // sps_num_ladf_intervals_minus2 u(2)
        uint32_t sps_num_ladf_intervals_minus2 = reader.readBits(2);
        // sps_ladf_lowest_interval_qp_offset se(v)
        reader.readSE();
        for (uint32_t i = 0; i < sps_num_ladf_intervals_minus2 + 1; i++) {
            // sps_ladf_qp_offset[ i ] se(v)
            reader.readSE();
            // sps_ladf_delta_threshold_minus1[ i ] ue(v)
            reader.readUE();
        }
    }

    // sps_explicit_scaling_list_enabled_flag u(1)
    bool sps_explicit_scaling_list_enabled_flag = reader.readFlag();

    if (sps_lfnst_enabled_flag && sps_explicit_scaling_list_enabled_flag) {
        // sps_scaling_matrix_for_lfnst_disabled_flag u(1)
        reader.readFlag();
    }

    bool sps_scaling_matrix_for_alternative_colour_space_disabled_flag = false;
    if (sps_act_enabled_flag && sps_explicit_scaling_list_enabled_flag) {
        // sps_scaling_matrix_for_alternative_colour_space_disabled_flag u(1)
        sps_scaling_matrix_for_alternative_colour_space_disabled_flag = reader.readFlag();
    }
    if (sps_scaling_matrix_for_alternative_colour_space_disabled_flag) {
        // sps_scaling_matrix_designated_colour_space_flag u(1)
        reader.readFlag();
    }

    // sps_dep_quant_enabled_flag u(1)
    reader.readFlag();

    // sps_sign_data_hiding_enabled_flag u(1)
    reader.readFlag();

    // sps_virtual_boundaries_enabled_flag u(1)
    bool sps_virtual_boundaries_enabled_flag = reader.readFlag();
    if (sps_virtual_boundaries_enabled_flag) {
        // sps_virtual_boundaries_present_flag u(1)
        bool sps_virtual_boundaries_present_flag = reader.readFlag();
        if (sps_virtual_boundaries_present_flag) {
            // sps_num_ver_virtual_boundaries ue(v)
            uint32_t sps_num_ver_virtual_boundaries = reader.readUE();
            for (uint32_t i = 0; i < sps_num_ver_virtual_boundaries; i++) {
                // sps_virtual_boundary_pos_x_minus1[ i ] ue(v)
                reader.readUE();
            }
            // sps_num_hor_virtual_boundaries ue(v)
            uint32_t sps_num_hor_virtual_boundaries = reader.readUE();
            for (uint32_t i = 0; i < sps_num_hor_virtual_boundaries; i++) {
                // sps_virtual_boundary_pos_y_minus1[ i ] ue(v)
                reader.readUE();
            }
        }
    }

    if (sps_ptl_dpb_hrd_params_present_flag) {
        // sps_timing_hrd_params_present_flag u(1)
        bool sps_timing_hrd_params_present_flag = reader.readFlag();
        if (sps_timing_hrd_params_present_flag) {
            // general_timing_hrd_parameters()
            auto hrdInfo = VvcNaluHelper::parseGeneralTimingHrdParameters(reader);

            bool sps_sublayer_cpb_params_present_flag = false;
            if (sps_max_sublayers_minus1 > 0) {
                // sps_sublayer_cpb_params_present_flag u(1)
                sps_sublayer_cpb_params_present_flag = reader.readFlag();
            }

            uint32_t firstSubLayer = sps_sublayer_cpb_params_present_flag
                                         ? 0
                                         : sps_max_sublayers_minus1;
            // ols_timing_hrd_parameters( firstSubLayer, sps_max_sublayers_minus1 )
            VvcNaluHelper::skipOlsTimingHrdParameters(
                reader, firstSubLayer, sps_max_sublayers_minus1, hrdInfo);
        }
    }

    // sps_field_seq_flag u(1)
    reader.readFlag();

    // sps_vui_parameters_present_flag u(1)
    bool sps_vui_parameters_present_flag = reader.readFlag();
    if (sps_vui_parameters_present_flag) {
        // sps_vui_payload_size_minus1 ue(v)
        uint32_t sps_vui_payload_size_minus1 = reader.readUE();
        (void)sps_vui_payload_size_minus1;

        // while( !byte_aligned( ) )
        //   sps_vui_alignment_zero_bit f(1)
        reader.byteAlign();

        // vui_payload( sps_vui_payload_size_minus1 + 1 )
        // TODO: implement vui_payload sub-syntax parsing
    }

    // sps_extension_flag u(1)
    // bool sps_extension_flag = reader.readFlag();
    // if( sps_extension_flag )
    //   while( more_rbsp_data( ) )
    //     sps_extension_data_flag u(1)
    // rbsp_trailing_bits( )
    // NOTE: extension data and trailing bits not needed for SpsInfo

    (void)sps_video_parameter_set_id;
    (void)CtbSizeY;

    return info;

    }  // try
    catch (const std::exception& e)
    {
        // Truncated or malformed SPS NALU -- return zeroed info rather than
        // propagating an exception through the Reader API.
        HEIF_RECORD_FROM(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                         ::HEIF::ErrorOrigin::Parser_VvcSps,
                         "VvcSpsParser::parse: exception while reading SPS RBSP (truncated or malformed)",
                         e);
        return {};
    }
}
