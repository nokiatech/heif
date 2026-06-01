#include "vvcppsparser.hpp"
#include "heiferrorinfo.h"
#include "../vvcparser.hpp"
#include "heifcommondatatypes.h"
#include "vvcrbspreader.hpp"
#include <exception>

VvcParser::PpsInfo VvcPpsParser::parse(const Vector<uint8_t>& ppsNalu) {
    VvcParser::PpsInfo info = {};
    
    VvcRbspReader reader;
    if (!reader.prepare(ppsNalu)) {
        HEIF_RECORD(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                    ::HEIF::ErrorOrigin::Parser_VvcRbsp,
                    "VvcPpsParser::parse: VvcRbspReader::prepare rejected PPS NAL (too short or NAL header invalid)");
        return info;  // Error case
    }

    try
    {

    // PPS RBSP syntax (after NAL header):
    
    // pps_pic_parameter_set_id u(6)
    info.ppsId = static_cast<int>(reader.readBits(6));
    
    // pps_seq_parameter_set_id u(4)
    uint32_t pps_seq_parameter_set_id = reader.readBits(4);
    (void)pps_seq_parameter_set_id;
    
    // pps_mixed_nalu_types_in_pic_flag u(1)
    bool pps_mixed_nalu_types_in_pic_flag = reader.readFlag();
    (void)pps_mixed_nalu_types_in_pic_flag;
    
    // pps_pic_width_in_luma_samples ue(v)
    uint32_t pps_pic_width_in_luma_samples = reader.readUE();
    (void)pps_pic_width_in_luma_samples;
    
    // pps_pic_height_in_luma_samples ue(v)
    uint32_t pps_pic_height_in_luma_samples = reader.readUE();
    (void)pps_pic_height_in_luma_samples;
    
    // pps_conformance_window_flag u(1)
    bool pps_conformance_window_flag = reader.readFlag();
    if (pps_conformance_window_flag) {
        // pps_conf_win_left_offset ue(v)
        reader.readUE();
        // pps_conf_win_right_offset ue(v)
        reader.readUE();
        // pps_conf_win_top_offset ue(v)
        reader.readUE();
        // pps_conf_win_bottom_offset ue(v)
        reader.readUE();
    }
    
    // pps_scaling_window_explicit_signalling_flag u(1)
    bool pps_scaling_window_explicit_signalling_flag = reader.readFlag();
    if (pps_scaling_window_explicit_signalling_flag) {
        // pps_scaling_win_left_offset se(v)
        reader.readSE();
        // pps_scaling_win_right_offset se(v)
        reader.readSE();
        // pps_scaling_win_top_offset se(v)
        reader.readSE();
        // pps_scaling_win_bottom_offset se(v)
        reader.readSE();
    }
    
    // pps_output_flag_present_flag u(1)
    bool pps_output_flag_present_flag = reader.readFlag();
    (void)pps_output_flag_present_flag;
    
    // pps_no_pic_partition_flag u(1)
    bool pps_no_pic_partition_flag = reader.readFlag();
    
    // pps_subpic_id_mapping_present_flag u(1)
    bool pps_subpic_id_mapping_present_flag = reader.readFlag();
    
    uint32_t pps_num_subpics_minus1 = 0;
    uint32_t pps_subpic_id_len_minus1 = 0;
    
    if (pps_subpic_id_mapping_present_flag) {
        info.isSubpicMappingPresent = true;
        
        if (!pps_no_pic_partition_flag) {
            // pps_num_subpics_minus1 ue(v)
            pps_num_subpics_minus1 = reader.readUE();
        }
        
        // pps_subpic_id_len_minus1 ue(v)
        pps_subpic_id_len_minus1 = reader.readUE();
        info.subpicIdLen = pps_subpic_id_len_minus1 + 1;
        
        // Record bit position for subpic_id array
        info.subpicIdBitPos = reader.getBitsRead();
        
        // Check for emulation prevention bytes before this position
        info.startCodeEmulationFlag = reader.hasEmulationBytesBefore(info.subpicIdBitPos);
        
        // for( i = 0; i <= pps_num_subpics_minus1; i++ )
        //   pps_subpic_id[ i ] u(v)
        for (uint32_t i = 0; i <= pps_num_subpics_minus1; i++) {
            reader.readBits(pps_subpic_id_len_minus1 + 1);
        }
    }
    
    if (!pps_no_pic_partition_flag) {
        // pps_log2_ctu_size_minus5 u(2)
        uint32_t pps_log2_ctu_size_minus5 = reader.readBits(2);
        uint32_t CtbSizeY = 1u << (pps_log2_ctu_size_minus5 + 5);
        
        // pps_num_exp_tile_columns_minus1 ue(v)
        uint32_t pps_num_exp_tile_columns_minus1 = reader.readUE();
        
        // pps_num_exp_tile_rows_minus1 ue(v)
        uint32_t pps_num_exp_tile_rows_minus1 = reader.readUE();
        
        // Calculate picture dimensions in CTUs
        uint32_t PicWidthInCtbsY = (pps_pic_width_in_luma_samples + CtbSizeY - 1) / CtbSizeY;
        uint32_t PicHeightInCtbsY = (pps_pic_height_in_luma_samples + CtbSizeY - 1) / CtbSizeY;
        
        // for( i = 0; i <= pps_num_exp_tile_columns_minus1; i++ )
        //   pps_tile_column_width_minus1[ i ] ue(v)
        Vector<uint32_t> pps_tile_column_width_minus1;
        pps_tile_column_width_minus1.reserve(pps_num_exp_tile_columns_minus1 + 1);
        for (uint32_t i = 0; i <= pps_num_exp_tile_columns_minus1; i++) {
            pps_tile_column_width_minus1.push_back(reader.readUE());
        }
        
        // for( i = 0; i <= pps_num_exp_tile_rows_minus1; i++ )
        //   pps_tile_row_height_minus1[ i ] ue(v)
        Vector<uint32_t> pps_tile_row_height_minus1;
        pps_tile_row_height_minus1.reserve(pps_num_exp_tile_rows_minus1 + 1);
        for (uint32_t i = 0; i <= pps_num_exp_tile_rows_minus1; i++) {
            pps_tile_row_height_minus1.push_back(reader.readUE());
        }
        
        // Derive NumTileColumns
        uint32_t remainingWidthInCtbsY = PicWidthInCtbsY;
        uint32_t NumTileColumns = 0;
        for (uint32_t i = 0; i <= pps_num_exp_tile_columns_minus1; i++) {
            remainingWidthInCtbsY -= (pps_tile_column_width_minus1[i] + 1);
            NumTileColumns++;
        }
        uint32_t uniformTileColWidth = pps_tile_column_width_minus1[pps_num_exp_tile_columns_minus1] + 1;
        while (remainingWidthInCtbsY >= uniformTileColWidth) {
            remainingWidthInCtbsY -= uniformTileColWidth;
            NumTileColumns++;
        }
        if (remainingWidthInCtbsY > 0) {
            NumTileColumns++;
        }
        
        // Derive NumTileRows
        uint32_t remainingHeightInCtbsY = PicHeightInCtbsY;
        uint32_t NumTileRows = 0;
        for (uint32_t i = 0; i <= pps_num_exp_tile_rows_minus1; i++) {
            remainingHeightInCtbsY -= (pps_tile_row_height_minus1[i] + 1);
            NumTileRows++;
        }
        uint32_t uniformTileRowHeight = pps_tile_row_height_minus1[pps_num_exp_tile_rows_minus1] + 1;
        while (remainingHeightInCtbsY >= uniformTileRowHeight) {
            remainingHeightInCtbsY -= uniformTileRowHeight;
            NumTileRows++;
        }
        if (remainingHeightInCtbsY > 0) {
            NumTileRows++;
        }
        
        uint32_t NumTilesInPic = NumTileColumns * NumTileRows;
        
        bool pps_loop_filter_across_tiles_enabled_flag = false;
        bool pps_rect_slice_flag = true;  // Default is 1 when NumTilesInPic == 1
        
        if (NumTilesInPic > 1) {
            // pps_loop_filter_across_tiles_enabled_flag u(1)
            pps_loop_filter_across_tiles_enabled_flag = reader.readFlag();
            (void)pps_loop_filter_across_tiles_enabled_flag;
            // pps_rect_slice_flag u(1)
            pps_rect_slice_flag = reader.readFlag();
        }
        
        bool pps_single_slice_per_subpic_flag = false;
        if (pps_rect_slice_flag) {
            // pps_single_slice_per_subpic_flag u(1)
            pps_single_slice_per_subpic_flag = reader.readFlag();
        }
        
        uint32_t pps_num_slices_in_pic_minus1 = 0;
        if (pps_rect_slice_flag && !pps_single_slice_per_subpic_flag) {
            // pps_num_slices_in_pic_minus1 ue(v)
            pps_num_slices_in_pic_minus1 = reader.readUE();
            
            bool pps_tile_idx_delta_present_flag = false;
            if (pps_num_slices_in_pic_minus1 > 1) {
                // pps_tile_idx_delta_present_flag u(1)
                pps_tile_idx_delta_present_flag = reader.readFlag();
            }
            
            // Build tile column widths array (full, not just explicit)
            Vector<uint32_t> ColWidthVal(NumTileColumns, 0);
            remainingWidthInCtbsY = PicWidthInCtbsY;
            for (uint32_t i = 0; i < NumTileColumns; i++) {
                if (i <= pps_num_exp_tile_columns_minus1) {
                    ColWidthVal[i] = pps_tile_column_width_minus1[i] + 1;
                } else if (remainingWidthInCtbsY >= uniformTileColWidth) {
                    ColWidthVal[i] = uniformTileColWidth;
                } else {
                    ColWidthVal[i] = remainingWidthInCtbsY;
                }
                remainingWidthInCtbsY -= ColWidthVal[i];
            }
            
            // Build tile row heights array (full, not just explicit)
            Vector<uint32_t> RowHeightVal(NumTileRows, 0);
            remainingHeightInCtbsY = PicHeightInCtbsY;
            for (uint32_t i = 0; i < NumTileRows; i++) {
                if (i <= pps_num_exp_tile_rows_minus1) {
                    RowHeightVal[i] = pps_tile_row_height_minus1[i] + 1;
                } else if (remainingHeightInCtbsY >= uniformTileRowHeight) {
                    RowHeightVal[i] = uniformTileRowHeight;
                } else {
                    RowHeightVal[i] = remainingHeightInCtbsY;
                }
                remainingHeightInCtbsY -= RowHeightVal[i];
            }
            
            // Track slice dimensions for parsing
            Vector<uint32_t> pps_slice_width_in_tiles_minus1(pps_num_slices_in_pic_minus1 + 1, 0);
            Vector<uint32_t> pps_slice_height_in_tiles_minus1(pps_num_slices_in_pic_minus1 + 1, 0);
            Vector<uint32_t> pps_num_exp_slices_in_tile(pps_num_slices_in_pic_minus1 + 1, 0);
            Vector<uint32_t> SliceTopLeftTileIdx(pps_num_slices_in_pic_minus1 + 1, 0);
            Vector<uint32_t> NumSlicesInTile(pps_num_slices_in_pic_minus1 + 1, 1);
            
            for (uint32_t i = 0; i < pps_num_slices_in_pic_minus1; i++) {
                uint32_t tileX = SliceTopLeftTileIdx[i] % NumTileColumns;
                uint32_t tileY = SliceTopLeftTileIdx[i] / NumTileColumns;
                
                if (tileX != NumTileColumns - 1) {
                    // pps_slice_width_in_tiles_minus1[ i ] ue(v)
                    pps_slice_width_in_tiles_minus1[i] = reader.readUE();
                }
                
                if (tileY != NumTileRows - 1 && 
                    (pps_tile_idx_delta_present_flag || tileX == 0)) {
                    // pps_slice_height_in_tiles_minus1[ i ] ue(v)
                    pps_slice_height_in_tiles_minus1[i] = reader.readUE();
                }
                
                if (pps_slice_width_in_tiles_minus1[i] == 0 &&
                    pps_slice_height_in_tiles_minus1[i] == 0 &&
                    RowHeightVal[tileY] > 1) {
                    // pps_num_exp_slices_in_tile[ i ] ue(v)
                    pps_num_exp_slices_in_tile[i] = reader.readUE();
                    
                    uint32_t numExpSlicesInTile = pps_num_exp_slices_in_tile[i];
                    for (uint32_t j = 0; j < numExpSlicesInTile; j++) {
                        // pps_exp_slice_height_in_ctus_minus1[ i ][ j ] ue(v)
                        reader.readUE();
                    }
                    
                    // Derive NumSlicesInTile - simplified approximation
                    // The exact derivation requires tracking slice heights
                    NumSlicesInTile[i] = numExpSlicesInTile + 1;
                    i += NumSlicesInTile[i] - 1;
                }
                
                if (pps_tile_idx_delta_present_flag && i < pps_num_slices_in_pic_minus1) {
                    // pps_tile_idx_delta_val[ i ] se(v)
                    int32_t delta = reader.readSE();
                    if (i + 1 <= pps_num_slices_in_pic_minus1) {
                        SliceTopLeftTileIdx[i + 1] = static_cast<uint32_t>(static_cast<int32_t>(SliceTopLeftTileIdx[i]) + delta);
                    }
                } else {
                    // Default sequential progression
                    if (i + 1 <= pps_num_slices_in_pic_minus1) {
                        uint32_t sliceWidthInTiles = pps_slice_width_in_tiles_minus1[i] + 1;
                        uint32_t sliceHeightInTiles = pps_slice_height_in_tiles_minus1[i] + 1;
                        
                        if (tileX + sliceWidthInTiles < NumTileColumns) {
                            SliceTopLeftTileIdx[i + 1] = SliceTopLeftTileIdx[i] + sliceWidthInTiles;
                        } else {
                            SliceTopLeftTileIdx[i + 1] = (tileY + sliceHeightInTiles) * NumTileColumns;
                        }
                    }
                }
            }
        }
        
        if (!pps_rect_slice_flag || pps_single_slice_per_subpic_flag || pps_num_slices_in_pic_minus1 > 0) {
            // pps_loop_filter_across_slices_enabled_flag u(1)
            reader.readFlag();
        }
    }
    
    // pps_cabac_init_present_flag u(1)
    bool pps_cabac_init_present_flag = reader.readFlag();
    (void)pps_cabac_init_present_flag;
    
    // for( i = 0; i < 2; i++ )
    //   pps_num_ref_idx_default_active_minus1[ i ] ue(v)
    for (int i = 0; i < 2; i++) {
        reader.readUE();
    }
    
    // pps_rpl1_idx_present_flag u(1)
    bool pps_rpl1_idx_present_flag = reader.readFlag();
    (void)pps_rpl1_idx_present_flag;
    
    // pps_weighted_pred_flag u(1)
    bool pps_weighted_pred_flag = reader.readFlag();
    
    // pps_weighted_bipred_flag u(1)
    bool pps_weighted_bipred_flag = reader.readFlag();
    
    // pps_ref_wraparound_enabled_flag u(1)
    bool pps_ref_wraparound_enabled_flag = reader.readFlag();
    if (pps_ref_wraparound_enabled_flag) {
        // pps_pic_width_minus_wraparound_offset ue(v)
        reader.readUE();
    }
    
    // pps_init_qp_minus26 se(v)
    int32_t pps_init_qp_minus26 = reader.readSE();
    (void)pps_init_qp_minus26;
    
    // pps_cu_qp_delta_enabled_flag u(1)
    bool pps_cu_qp_delta_enabled_flag = reader.readFlag();
    (void)pps_cu_qp_delta_enabled_flag;
    
    // pps_chroma_tool_offsets_present_flag u(1)
    bool pps_chroma_tool_offsets_present_flag = reader.readFlag();

    bool pps_deblocking_filter_control_present_flag = false;

    bool pps_joint_cbcr_qp_offset_present_flag = false;
    if (pps_chroma_tool_offsets_present_flag) {
        // pps_cb_qp_offset se(v)
        reader.readSE();
        // pps_cr_qp_offset se(v)
        reader.readSE();
        // pps_joint_cbcr_qp_offset_present_flag u(1)
        pps_joint_cbcr_qp_offset_present_flag = reader.readFlag();
        if (pps_joint_cbcr_qp_offset_present_flag) {
            // pps_joint_cbcr_qp_offset_value se(v)
            reader.readSE();
        }
        // pps_slice_chroma_qp_offsets_present_flag u(1)
        reader.readFlag();
        // pps_cu_chroma_qp_offset_list_enabled_flag u(1)
        bool pps_cu_chroma_qp_offset_list_enabled_flag = reader.readFlag();
        if (pps_cu_chroma_qp_offset_list_enabled_flag) {
            // pps_chroma_qp_offset_list_len_minus1 ue(v)
            uint32_t pps_chroma_qp_offset_list_len_minus1 = reader.readUE();
            for (uint32_t i = 0; i <= pps_chroma_qp_offset_list_len_minus1; i++) {
                // pps_cb_qp_offset_list[ i ] se(v)
                reader.readSE();
                // pps_cr_qp_offset_list[ i ] se(v)
                reader.readSE();
                if (pps_joint_cbcr_qp_offset_present_flag) {
                    // pps_joint_cbcr_qp_offset_list[ i ] se(v)
                    reader.readSE();
                }
            }
        }
    }

    // pps_deblocking_filter_control_present_flag u(1)
    // Unconditionally present per ITU-T H.266 Section 7.3.2.4
    pps_deblocking_filter_control_present_flag = reader.readFlag();
    if (pps_deblocking_filter_control_present_flag) {
        // pps_deblocking_filter_override_enabled_flag u(1)
        bool pps_deblocking_filter_override_enabled_flag = reader.readFlag();
        // pps_deblocking_filter_disabled_flag u(1)
        bool pps_deblocking_filter_disabled_flag = reader.readFlag();
        
        if (!pps_no_pic_partition_flag && pps_deblocking_filter_override_enabled_flag) {
            // pps_dbf_info_in_ph_flag u(1)
            reader.readFlag();
        }
        
        if (!pps_deblocking_filter_disabled_flag) {
            // pps_luma_beta_offset_div2 se(v)
            reader.readSE();
            // pps_luma_tc_offset_div2 se(v)
            reader.readSE();
            
            if (pps_chroma_tool_offsets_present_flag) {
                // pps_cb_beta_offset_div2 se(v)
                reader.readSE();
                // pps_cb_tc_offset_div2 se(v)
                reader.readSE();
                // pps_cr_beta_offset_div2 se(v)
                reader.readSE();
                // pps_cr_tc_offset_div2 se(v)
                reader.readSE();
            }
        }
    }
    
    if (!pps_no_pic_partition_flag) {
        // pps_rpl_info_in_ph_flag u(1)
        bool pps_rpl_info_in_ph_flag = reader.readFlag();
        // pps_sao_info_in_ph_flag u(1)
        reader.readFlag();
        // pps_alf_info_in_ph_flag u(1)
        reader.readFlag();
        
        if ((pps_weighted_pred_flag || pps_weighted_bipred_flag) && pps_rpl_info_in_ph_flag) {
            // pps_wp_info_in_ph_flag u(1)
            reader.readFlag();
        }
        
        // pps_qp_delta_info_in_ph_flag u(1)
        reader.readFlag();
    }
    
    // pps_picture_header_extension_present_flag u(1)
    reader.readFlag();
    // pps_slice_header_extension_present_flag u(1)
    reader.readFlag();
    // pps_extension_flag u(1)
    bool pps_extension_flag = reader.readFlag();
    
    // Extension data parsing omitted (handled by rbsp_trailing_bits)
    (void)pps_extension_flag;

    return info;

    }  // try
    catch (const std::exception& e)
    {
        // Truncated or malformed PPS NALU -- return zeroed info rather than
        // propagating an exception through the Reader API.
        HEIF_RECORD_FROM(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                         ::HEIF::ErrorOrigin::Parser_VvcPps,
                         "VvcPpsParser::parse: exception while reading PPS RBSP (truncated or malformed)",
                         e);
        return {};
    }
}
