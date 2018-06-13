/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "avcparser.hpp"
#include "bitstream.hpp"

bool parseHRD(BitStream& bitstr, HRDParameters& retHdr)
{
    HRDParameters hrd{};

    hrd.cpb_cnt_minus1 = bitstr.readExpGolombCode();                // 0 | 5 	ue(v)
    hrd.bit_rate_scale = static_cast<uint8_t>(bitstr.readBits(4));  // 0 | 5 	u(4)
    hrd.cpb_size_scale = static_cast<uint8_t>(bitstr.readBits(4));  // 0 | 5 	u(4)
    hrd.bit_rate_value_minus1.resize(hrd.cpb_cnt_minus1 + 1);
    hrd.cpb_size_value_minus1.resize(hrd.cpb_cnt_minus1 + 1);
    hrd.cbr_flag.resize(hrd.cpb_cnt_minus1 + 1);
    for (size_t SchedSelIdx = 0; SchedSelIdx <= hrd.cpb_cnt_minus1; SchedSelIdx++)
    {
        hrd.bit_rate_value_minus1[SchedSelIdx] = bitstr.readExpGolombCode();                // 0 | 5 	ue(v)
        hrd.cpb_size_value_minus1[SchedSelIdx] = bitstr.readExpGolombCode();                // 0 | 5 	ue(v)
        hrd.cbr_flag[SchedSelIdx]              = static_cast<uint8_t>(bitstr.readBits(1));  // 0 | 5 	u(1)
    }
    hrd.initial_cpb_removal_delay_length_minus1 = static_cast<uint8_t>(bitstr.readBits(5));  // 0 | 5 	u(5)
    hrd.cpb_removal_delay_length_minus1         = static_cast<uint8_t>(bitstr.readBits(5));  // 0 | 5 	u(5)
    hrd.dpb_output_delay_length_minus1          = static_cast<uint8_t>(bitstr.readBits(5));  // 0 | 5 	u(5)
    hrd.time_offset_length                      = static_cast<uint8_t>(bitstr.readBits(5));  // 0 | 5 	u(5)

    retHdr = hrd;
    return true;
}

bool parseVUI(BitStream& bitstr, VUIParameters& retVui)
{
    VUIParameters vui{};
    vui.aspect_ratio_info_present_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1)
    if (vui.aspect_ratio_info_present_flag)
    {
        vui.aspect_ratio_idc = static_cast<uint8_t>(bitstr.readBits(8));  // 0  u(8)
        if (vui.aspect_ratio_idc == 255 /* Extended_SAR */)
        {
            vui.sar_width  = static_cast<uint16_t>(bitstr.readBits(16));  // 0  u(16)
            vui.sar_height = static_cast<uint16_t>(bitstr.readBits(16));  // 0  u(16)
        };
    };
    vui.overscan_info_present_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1)
    if (vui.overscan_info_present_flag)
        vui.overscan_appropriate_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1)
    vui.video_signal_type_present_flag = static_cast<uint8_t>(bitstr.readBits(1));
    ;  // 0  u(1)
    if (vui.video_signal_type_present_flag)
    {
        vui.video_format                    = static_cast<uint8_t>(bitstr.readBits(3));  // 0  u(3)
        vui.video_full_range_flag           = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1)
        vui.colour_description_present_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1)
        if (vui.colour_description_present_flag)
        {
            vui.colour_primaries         = static_cast<uint8_t>(bitstr.readBits(8));  // 0  u(8)
            vui.transfer_characteristics = static_cast<uint8_t>(bitstr.readBits(8));  // 0  u(8)
            vui.matrix_coefficients      = static_cast<uint8_t>(bitstr.readBits(8));  // 0  u(8)
        };
    };
    vui.chroma_loc_info_present_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1)
    if (vui.chroma_loc_info_present_flag)
    {
        vui.chroma_sample_loc_type_top_field    = static_cast<uint8_t>(bitstr.readExpGolombCode());  // 0  ue(v)
        vui.chroma_sample_loc_type_bottom_field = static_cast<uint8_t>(bitstr.readExpGolombCode());  // 0  ue(v)
    };
    vui.timing_info_present_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1)
    if (vui.timing_info_present_flag)
    {
        vui.num_units_in_tick     = bitstr.read32Bits();                       // 0  u(32)
        vui.time_scale            = bitstr.read32Bits();                       // 0  u(32)
        vui.fixed_frame_rate_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1)
    };
    vui.nal_hrd_parameters_present_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1)
    if (vui.nal_hrd_parameters_present_flag)
    {
        if (!parseHRD(bitstr, vui.nal_hrd_parameters))
        {
            return false;
        }
    }
    vui.vcl_hrd_parameters_present_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1)
    if (vui.vcl_hrd_parameters_present_flag)
    {
        if (!parseHRD(bitstr, vui.vcl_hrd_parameters))
        {
            return false;
        }
    }
    if (vui.nal_hrd_parameters_present_flag || vui.vcl_hrd_parameters_present_flag)
    {
        vui.low_delay_hrd_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1);
    }
    vui.pic_struct_present_flag    = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1)
    vui.bitstream_restriction_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1)
    if (vui.bitstream_restriction_flag)
    {
        vui.motion_vectors_over_pic_boundaries_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0  u(1)
        vui.max_bytes_per_pic_denom                 = bitstr.readExpGolombCode();                // 0  ue(v)
        vui.max_bits_per_mb_denom                   = bitstr.readExpGolombCode();                // 0  ue(v)
        vui.log2_max_mv_length_horizontal           = bitstr.readExpGolombCode();                // 0  ue(v)
        vui.log2_max_mv_length_vertical             = bitstr.readExpGolombCode();                // 0  ue(v)
        vui.max_num_reorder_frames                  = bitstr.readExpGolombCode();                // 0  ue(v)
        vui.max_dec_frame_buffering                 = bitstr.readExpGolombCode();                // 0  ue(v)
    }

    retVui = vui;
    return true;
}

bool parseSPS(BitStream& bitstr, SPSConfigValues& retSps)
{
    SPSConfigValues sps{};
    sps.profile_idc           = static_cast<uint8_t>(bitstr.readBits(8));  // 0 u(8)
    sps.profile_compatibility = static_cast<uint8_t>(bitstr.readBits(8));  // contains a bunch of flags
    sps.level_idc             = static_cast<uint8_t>(bitstr.readBits(8));  // 0 u(8)
    sps.seq_parameter_set_id  = bitstr.readExpGolombCode();                // 0 ue(v)
    if (sps.profile_idc == 100 || sps.profile_idc == 110 || sps.profile_idc == 122 || sps.profile_idc == 244 ||
        sps.profile_idc == 44 || sps.profile_idc == 83 || sps.profile_idc == 86 || sps.profile_idc == 118 ||
        sps.profile_idc == 128 || sps.profile_idc == 138 || sps.profile_idc == 139 || sps.profile_idc == 134 ||
        sps.profile_idc == 135)
    {
        sps.chroma_format_idc = bitstr.readExpGolombCode();  // 0  ue(v)
        if (sps.chroma_format_idc == 3)
        {
            sps.separate_colour_plane_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0 u(1)
        }
        sps.bit_depth_luma_minus8                = bitstr.readExpGolombCode();                // 0 ue(v)
        sps.bit_depth_chroma_minus8              = bitstr.readExpGolombCode();                // 0 ue(v)
        sps.qpprime_y_zero_transform_bypass_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0 u(1)
        sps.seq_scaling_matrix_present_flag      = static_cast<uint8_t>(bitstr.readBits(1));  // 0 u(1)
        if (sps.seq_scaling_matrix_present_flag)
        {
            // unsupported
            return false;
#if 0
            for (size_t i = 0; i < ((chroma_format_idc != 3) ? 8 : 12); i++)
            {
                uint8_t seq_scaling_matrix_present_flag = bitStr.readBits(1); //0 u(1)
                if (seq_scaling_list_present_flag)
                {
                    if (i < 6)
                    {
                        scaling_list( ScalingList4x4[ i ], 16, UseDefaultScalingMatrix4x4Flag[ i ]);
                    }
                    else
                    {
                        scaling_list( ScalingList8x8[ i-6 ], 64, UseDefaultScalingMatrix8x8Flag[ i-6 ] );
                    }
                }
            }
#endif
        }
    }
    sps.log2_max_frame_num_minus4 = bitstr.readExpGolombCode();  // 0 ue(v)
    sps.pic_order_cnt_type        = bitstr.readExpGolombCode();  // 0 ue(v)
    if (sps.pic_order_cnt_type == 0)
    {
        sps.log2_max_pic_order_cnt_lsb_minus4 = bitstr.readExpGolombCode();  // 0 ue(v)
    }
    else
    {
        if (sps.pic_order_cnt_type == 1)
        {
            sps.delta_pic_order_always_zero_flag      = static_cast<uint8_t>(bitstr.readBits(1));  // 0 u(1)
            sps.offset_for_non_ref_pic                = bitstr.readSignedExpGolombCode();          // 0 se(v)
            sps.offset_for_top_to_bottom_field        = bitstr.readSignedExpGolombCode();          // 0 se(v)
            sps.num_ref_frames_in_pic_order_cnt_cycle = bitstr.readExpGolombCode();                // 0 ue(v)
            sps.offset_for_ref_frame.resize(sps.num_ref_frames_in_pic_order_cnt_cycle);
            for (size_t i = 0; i < sps.num_ref_frames_in_pic_order_cnt_cycle; i++)
            {
                sps.offset_for_ref_frame[i] = bitstr.readSignedExpGolombCode();  // 0 se(v)
            }
        }
    }
    sps.max_num_ref_frames                   = bitstr.readExpGolombCode();                // 0  ue(v)
    sps.gaps_in_frame_num_value_allowed_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0        u(1)
    sps.pic_width_in_mbs_minus1              = bitstr.readExpGolombCode();                // 0     ue(v)
    sps.pic_height_in_map_units_minus1       = bitstr.readExpGolombCode();                // 0      ue(v)
    sps.frame_mbs_only_flag                  = static_cast<uint8_t>(bitstr.readBits(1));  // 0 u(1)
    if (!sps.frame_mbs_only_flag)
    {
        sps.mb_adaptive_frame_field_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0        u(1)
    }
    sps.direct_8x8_inference_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0   u(1)
    sps.frame_cropping_flag       = static_cast<uint8_t>(bitstr.readBits(1));  // 0 u(1)
    if (sps.frame_cropping_flag)
    {
        sps.frame_crop_left_offset   = bitstr.readExpGolombCode();  // 0  ue(v)
        sps.frame_crop_right_offset  = bitstr.readExpGolombCode();  // 0 ue(v)
        sps.frame_crop_top_offset    = bitstr.readExpGolombCode();  // 0   ue(v)
        sps.frame_crop_bottom_offset = bitstr.readExpGolombCode();  // 0        ue(v)
    }
    sps.vui_parameters_present_flag = static_cast<uint8_t>(bitstr.readBits(1));  // 0 u(1)
    if (sps.vui_parameters_present_flag)
    {
        // perhaps doesn't work
#if 0
        if (!parseVUI(bitstr, sps.vui_parameters))
        {
            return false;
        }
#endif
    }
    retSps = sps;
    return true;
}
