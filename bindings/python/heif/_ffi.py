"""
Low-level ``cffi`` bridge to ``libheif_shared``.

This module owns:
  * The ``cdef()`` string that declares the reader + error-slot C surface to
    ``cffi``.  Kept in lockstep with ``srcs/api/reader/heifreader_c.h``
    and ``srcs/api/common/heif_error_c.h``.
  * The library lookup logic (``HEIF_LIBRARY_PATH`` env override,
    then platform default).
  * Helpers to decode FourCC ``char[5]`` buffers and to drain the
    error slot into a Python string.

Nothing in here is part of the public API; users of the binding
import :class:`heif.Reader` from the top-level package.

The module is intentionally process-singleton: ``cffi.FFI()`` and
``ffi.dlopen(...)`` are called exactly once at first import, and
the resulting ``ffi`` and ``lib`` are reused.  Re-importing or
forking does not require special handling -- ``cffi`` ABI handles
are pure metadata and the underlying ``dlopen`` reference counts
through ``libdl``.
"""

from __future__ import annotations

import os
import sys
from typing import Tuple

import cffi

# C declarations
#
# Kept in lockstep with:
#   * srcs/api/common/heif_error_c.h  (error slot, 10 entry points)
#   * srcs/api/reader/heifreader_c.h  (reader C-facade + image properties, 32 entry points)
#
# Only the fixed-width integer types appear here -- ``size_t`` is
# expanded to ``uintptr_t`` because cffi's ABI mode does not consult
# system headers for ``size_t`` and we want the binding to be byte-
# identical across 32-bit and 64-bit hosts.  On every supported
# platform the two are layout-equivalent for buffer sizes.
_CDEF = r"""
/* error slot: error-inspection surface */
int32_t      heif_get_last_error_code(void);
int32_t      heif_get_last_error_origin(void);
const char*  heif_get_last_error_origin_name(void);
const char*  heif_get_last_error_code_name(void);
const char*  heif_get_last_error_file(void);
uint32_t     heif_get_last_error_line(void);
size_t       heif_get_last_error_message(char* buf, size_t len);
size_t       heif_get_last_error_cause(char* buf, size_t len);
int32_t      heif_has_last_error(void);
void         heif_clear_last_error(void);

/* reader C-facade: reader facade */
typedef int32_t heif_error_code_t;
typedef struct heif_reader heif_reader_t;

heif_reader_t*    heif_reader_create(void);
void              heif_reader_destroy(heif_reader_t* reader);
heif_error_code_t heif_reader_initialize_file(heif_reader_t* reader, const char* path);

/* file-level accessors */
heif_error_code_t heif_reader_get_major_brand(const heif_reader_t* reader, char out_brand[5]);
heif_error_code_t heif_reader_get_minor_version(const heif_reader_t* reader, uint32_t* out_minor_version);
heif_error_code_t heif_reader_get_compatible_brand_count(const heif_reader_t* reader, uint32_t* out_count);
heif_error_code_t heif_reader_get_compatible_brand_at(const heif_reader_t* reader,
                                                     uint32_t index, char out_brand[5]);

/* primary item */
heif_error_code_t heif_reader_get_primary_item(const heif_reader_t* reader, uint32_t* out_image_id);

/* master-image enumeration */
heif_error_code_t heif_reader_get_master_image_count(const heif_reader_t* reader, uint32_t* out_count);
heif_error_code_t heif_reader_get_master_image_at(const heif_reader_t* reader,
                                                  uint32_t index, uint32_t* out_image_id);

/* type-filtered item enumeration */
heif_error_code_t heif_reader_get_item_list_by_type_count(const heif_reader_t* reader,
                                                          const char* type, uint32_t* out_count);
heif_error_code_t heif_reader_get_item_list_by_type_at(const heif_reader_t* reader,
                                                       const char* type, uint32_t index,
                                                       uint32_t* out_image_id);

/* reference navigation (from-direction) */
heif_error_code_t heif_reader_get_referenced_from_item_list_by_type_count(
    const heif_reader_t* reader, uint32_t from_image_id,
    const char* reference_type, uint32_t* out_count);
heif_error_code_t heif_reader_get_referenced_from_item_list_by_type_at(
    const heif_reader_t* reader, uint32_t from_image_id,
    const char* reference_type, uint32_t index, uint32_t* out_image_id);

/* reference navigation (to-direction) */
heif_error_code_t heif_reader_get_referenced_to_item_list_by_type_count(
    const heif_reader_t* reader, uint32_t to_image_id,
    const char* reference_type, uint32_t* out_count);
heif_error_code_t heif_reader_get_referenced_to_item_list_by_type_at(
    const heif_reader_t* reader, uint32_t to_image_id,
    const char* reference_type, uint32_t index, uint32_t* out_image_id);

/* per-image accessors */
heif_error_code_t heif_reader_get_width(const heif_reader_t* reader, uint32_t image_id, uint32_t* out_width);
heif_error_code_t heif_reader_get_height(const heif_reader_t* reader, uint32_t image_id, uint32_t* out_height);
heif_error_code_t heif_reader_get_item_type(const heif_reader_t* reader, uint32_t image_id, char out_type[5]);

/* bitstream / raw item bytes */
heif_error_code_t heif_reader_get_item_data_with_decoder_parameters(
    heif_reader_t* reader, uint32_t image_id, uint8_t* buffer, uint64_t* size_inout);
heif_error_code_t heif_reader_get_item_data(
    heif_reader_t* reader, uint32_t image_id, uint8_t* buffer, uint64_t* size_inout);

/* image properties -- image properties */
heif_error_code_t heif_reader_get_item_property_count(
    const heif_reader_t* reader, uint32_t image_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_item_property_at(
    const heif_reader_t* reader, uint32_t image_id, uint32_t index,
    uint32_t* out_property_id, char out_type[5], int* out_essential);

heif_error_code_t heif_reader_get_property_ispe(
    const heif_reader_t* reader, uint32_t property_id,
    uint32_t* out_width, uint32_t* out_height);
heif_error_code_t heif_reader_get_property_irot(
    const heif_reader_t* reader, uint32_t property_id, uint32_t* out_angle_ccw);
heif_error_code_t heif_reader_get_property_imir(
    const heif_reader_t* reader, uint32_t property_id, int* out_horizontal_axis);

heif_error_code_t heif_reader_get_property_pixi_channel_count(
    const heif_reader_t* reader, uint32_t property_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_property_pixi_bits_at(
    const heif_reader_t* reader, uint32_t property_id,
    uint32_t channel_index, uint8_t* out_bits);

heif_error_code_t heif_reader_get_property_colr_type(
    const heif_reader_t* reader, uint32_t property_id, char out_type[5]);
heif_error_code_t heif_reader_get_property_colr_nclx(
    const heif_reader_t* reader, uint32_t property_id,
    uint16_t* out_primaries, uint16_t* out_transfer,
    uint16_t* out_matrix, int* out_full_range);
heif_error_code_t heif_reader_get_property_colr_icc(
    const heif_reader_t* reader, uint32_t property_id,
    uint8_t* buffer, uint64_t* size_inout);

heif_error_code_t heif_reader_get_property_pasp(
    const heif_reader_t* reader, uint32_t property_id,
    uint32_t* out_relative_width, uint32_t* out_relative_height);

/* pixi ChannelData -- pixi ChannelData (rules 8 & 9: caller-allocated POD
 * struct + size-on-entry, plus (_count, _at) for nested arrays).
 * The struct is fixed at 12 bytes, descending-alignment, with
 * explicit _reserved[2] padding pinned by HEIF_STATIC_ASSERT in
 * the C header.  cffi mirrors the layout one-for-one.
 */
typedef struct heif_pixi_channel {
    uint32_t struct_size;
    uint8_t  channel_idc;
    uint8_t  component_format;
    uint8_t  subsampling_flag;
    uint8_t  channel_label_flag;
    uint8_t  subsampling_type;
    uint8_t  subsampling_location;
    uint8_t  _reserved[2];
} heif_pixi_channel_t;

heif_error_code_t heif_reader_get_property_pixi_num_channels(
    const heif_reader_t* reader, uint32_t property_id, uint8_t* out_num_channels);
heif_error_code_t heif_reader_get_property_pixi_channel_at(
    const heif_reader_t* reader, uint32_t property_id,
    uint32_t channel_index, heif_pixi_channel_t* out_channel);
heif_error_code_t heif_reader_get_property_pixi_channel_label(
    const heif_reader_t* reader, uint32_t property_id,
    uint32_t channel_index, char* buffer, uint64_t* size_inout);

/* HDR static metadata, derived items, gain-map, file/item
 * discoverability, long-tail properties, entity groups, stream-
 * callback init.  All structs are caller-allocated PODs with
 * leading struct_size and byte-stable layout; variable-length
 * nested data uses (_count, _at) pairs. */

/* HDR static metadata structs */
typedef struct heif_clli {
    uint32_t struct_size;
    uint16_t max_content_light_level;
    uint16_t max_pic_average_light_level;
} heif_clli_t;

typedef struct heif_mdcv {
    uint32_t struct_size;
    uint16_t display_primaries_x[3];
    uint16_t display_primaries_y[3];
    uint16_t white_point_x;
    uint16_t white_point_y;
    uint8_t  _reserved[4];
    uint32_t max_display_mastering_luminance;
    uint32_t min_display_mastering_luminance;
} heif_mdcv_t;

typedef struct heif_cclv {
    uint32_t struct_size;
    uint8_t  primaries_present;
    uint8_t  min_luminance_present;
    uint8_t  max_luminance_present;
    uint8_t  avg_luminance_present;
    uint32_t primaries_x[3];
    uint32_t primaries_y[3];
    uint32_t min_luminance_value;
    uint32_t max_luminance_value;
    uint32_t avg_luminance_value;
} heif_cclv_t;

typedef struct heif_amve {
    uint32_t struct_size;
    uint32_t ambient_illuminance;
    uint16_t ambient_light_x;
    uint16_t ambient_light_y;
} heif_amve_t;

typedef struct heif_reve {
    uint32_t struct_size;
    uint32_t surround_luminance;
    uint16_t surround_light_x;
    uint16_t surround_light_y;
    uint32_t periphery_luminance;
    uint16_t periphery_light_x;
    uint16_t periphery_light_y;
} heif_reve_t;

typedef struct heif_ndwt {
    uint32_t struct_size;
    uint32_t diffuse_white_luminance;
} heif_ndwt_t;

heif_error_code_t heif_reader_get_property_clli(const heif_reader_t* reader, uint32_t property_id, heif_clli_t* out_clli);
heif_error_code_t heif_reader_get_property_mdcv(const heif_reader_t* reader, uint32_t property_id, heif_mdcv_t* out_mdcv);
heif_error_code_t heif_reader_get_property_cclv(const heif_reader_t* reader, uint32_t property_id, heif_cclv_t* out_cclv);
heif_error_code_t heif_reader_get_property_amve(const heif_reader_t* reader, uint32_t property_id, heif_amve_t* out_amve);
heif_error_code_t heif_reader_get_property_reve(const heif_reader_t* reader, uint32_t property_id, heif_reve_t* out_reve);
heif_error_code_t heif_reader_get_property_ndwt(const heif_reader_t* reader, uint32_t property_id, heif_ndwt_t* out_ndwt);

/* grid + overlay -- Grid + Overlay derived items */
typedef struct heif_grid {
    uint32_t struct_size;
    uint32_t output_width;
    uint32_t output_height;
    uint32_t columns;
    uint32_t rows;
} heif_grid_t;

typedef struct heif_overlay {
    uint32_t struct_size;
    uint16_t fill_r;
    uint16_t fill_g;
    uint16_t fill_b;
    uint16_t fill_a;
    uint32_t output_width;
    uint32_t output_height;
} heif_overlay_t;

typedef struct heif_overlay_image {
    uint32_t struct_size;
    uint32_t image_id;
    int32_t  horizontal_offset;
    int32_t  vertical_offset;
} heif_overlay_image_t;

heif_error_code_t heif_reader_get_grid(const heif_reader_t* reader, uint32_t image_id, heif_grid_t* out_grid);
heif_error_code_t heif_reader_get_grid_image_count(const heif_reader_t* reader, uint32_t image_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_grid_image_at(const heif_reader_t* reader, uint32_t image_id, uint32_t index, uint32_t* out_image_id);
heif_error_code_t heif_reader_get_overlay(const heif_reader_t* reader, uint32_t image_id, heif_overlay_t* out_overlay);
heif_error_code_t heif_reader_get_overlay_image_count(const heif_reader_t* reader, uint32_t image_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_overlay_image_at(const heif_reader_t* reader, uint32_t image_id, uint32_t index, heif_overlay_image_t* out_image);

/* gain-map / tonemap -- gain-map / Tonemap */
typedef struct heif_gainmap {
    uint32_t struct_size;
    uint16_t minimum_version;
    uint16_t writer_version;
    uint8_t  is_multichannel;
    uint8_t  use_base_colour_space;
    uint8_t  _reserved[2];
    int32_t  channel_count;
    uint32_t base_hdr_headroom_numerator;
    uint32_t base_hdr_headroom_denominator;
    uint32_t alternate_hdr_headroom_numerator;
    uint32_t alternate_hdr_headroom_denominator;
} heif_gainmap_t;

typedef struct heif_gainmap_channel {
    uint32_t struct_size;
    int32_t  gain_map_min_numerator;
    uint32_t gain_map_min_denominator;
    int32_t  gain_map_max_numerator;
    uint32_t gain_map_max_denominator;
    uint32_t gamma_numerator;
    uint32_t gamma_denominator;
    int32_t  base_offset_numerator;
    uint32_t base_offset_denominator;
    int32_t  alternate_offset_numerator;
    uint32_t alternate_offset_denominator;
} heif_gainmap_channel_t;

heif_error_code_t heif_reader_get_tonemap_gainmap(const heif_reader_t* reader, uint32_t image_id, heif_gainmap_t* out_gainmap);
heif_error_code_t heif_reader_get_tonemap_gainmap_channel_count(const heif_reader_t* reader, uint32_t image_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_tonemap_gainmap_channel_at(const heif_reader_t* reader, uint32_t image_id, uint32_t index, heif_gainmap_channel_t* out_channel);
heif_error_code_t heif_reader_get_tonemap_image_count(const heif_reader_t* reader, uint32_t image_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_tonemap_image_at(const heif_reader_t* reader, uint32_t image_id, uint32_t index, uint32_t* out_image_id);

/* file/metabox/item metadata -- file/item/metabox features + decode helpers */
heif_error_code_t heif_reader_get_file_features(const heif_reader_t* reader, uint32_t* out_features);
heif_error_code_t heif_reader_get_metabox_features(const heif_reader_t* reader, uint32_t* out_features);
heif_error_code_t heif_reader_get_item_features(const heif_reader_t* reader, uint32_t image_id, uint32_t* out_features);
heif_error_code_t heif_reader_get_decoder_code_type(const heif_reader_t* reader, uint32_t image_id, char out_type[5]);
heif_error_code_t heif_reader_get_decode_dependency_count(const heif_reader_t* reader, uint32_t image_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_decode_dependency_at(const heif_reader_t* reader, uint32_t image_id, uint32_t index, uint32_t* out_dep_image_id);
heif_error_code_t heif_reader_get_decoder_param_set_count(const heif_reader_t* reader, uint32_t image_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_decoder_param_set_type_at(const heif_reader_t* reader, uint32_t image_id, uint32_t index, uint8_t* out_type);
heif_error_code_t heif_reader_get_decoder_param_set_data_at(const heif_reader_t* reader, uint32_t image_id, uint32_t index, uint8_t* buffer, uint64_t* size_inout);
heif_error_code_t heif_reader_get_decoder_param_set_all_layers_count(const heif_reader_t* reader, uint32_t image_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_decoder_param_set_all_layers_type_at(const heif_reader_t* reader, uint32_t image_id, uint32_t index, uint8_t* out_type);
heif_error_code_t heif_reader_get_decoder_param_set_all_layers_data_at(const heif_reader_t* reader, uint32_t image_id, uint32_t index, uint8_t* buffer, uint64_t* size_inout);
heif_error_code_t heif_reader_get_item_protection_scheme(heif_reader_t* reader, uint32_t image_id, uint8_t* buffer, uint64_t* size_inout);
heif_error_code_t heif_reader_get_compatible_brand_combo_count(const heif_reader_t* reader, uint32_t* out_count);
heif_error_code_t heif_reader_get_compatible_brand_combo_brand_count(const heif_reader_t* reader, uint32_t combo_index, uint32_t* out_count);
heif_error_code_t heif_reader_get_compatible_brand_combo_brand_at(const heif_reader_t* reader, uint32_t combo_index, uint32_t brand_index, char out_brand[5]);
heif_error_code_t heif_reader_get_matrix_at(const heif_reader_t* reader, uint32_t index, int32_t* out_value);

/* stream-callback -- stream-callback initialise */
typedef struct heif_stream_callbacks {
    uint32_t struct_size;
    int64_t (*read)(void* user_data, char* buffer, int64_t size);
    int     (*absolute_seek)(void* user_data, int64_t offset);
    int64_t (*tell)(void* user_data);
    int64_t (*size)(void* user_data);
    void* user_data;
} heif_stream_callbacks_t;

heif_error_code_t heif_reader_initialize_stream(heif_reader_t* reader, const heif_stream_callbacks_t* callbacks);

/* long-tail properties -- long-tail properties */
typedef struct heif_clap {
    uint32_t struct_size;
    uint32_t width_n;
    uint32_t width_d;
    uint32_t height_n;
    uint32_t height_d;
    uint32_t horizontal_offset_n;
    uint32_t horizontal_offset_d;
    uint32_t vertical_offset_n;
    uint32_t vertical_offset_d;
} heif_clap_t;

heif_error_code_t heif_reader_get_property_clap(const heif_reader_t* reader, uint32_t property_id, heif_clap_t* out_clap);
heif_error_code_t heif_reader_get_property_iscl(const heif_reader_t* reader, uint32_t property_id, uint16_t* out_w_n, uint16_t* out_w_d, uint16_t* out_h_n, uint16_t* out_h_d);
heif_error_code_t heif_reader_get_property_rloc(const heif_reader_t* reader, uint32_t property_id, uint32_t* out_h, uint32_t* out_v);
heif_error_code_t heif_reader_get_property_auxc_type(const heif_reader_t* reader, uint32_t property_id, char* buffer, uint64_t* size_inout);
heif_error_code_t heif_reader_get_property_auxc_subtype(const heif_reader_t* reader, uint32_t property_id, uint8_t* buffer, uint64_t* size_inout);
heif_error_code_t heif_reader_get_property_rref_count(const heif_reader_t* reader, uint32_t property_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_property_rref_at(const heif_reader_t* reader, uint32_t property_id, uint32_t index, char out_type[5]);
heif_error_code_t heif_reader_get_property_udes_field(const heif_reader_t* reader, uint32_t property_id, uint32_t field_index, char* buffer, uint64_t* size_inout);
heif_error_code_t heif_reader_get_property_altt_field(const heif_reader_t* reader, uint32_t property_id, uint32_t field_index, char* buffer, uint64_t* size_inout);
heif_error_code_t heif_reader_get_property_crtt(const heif_reader_t* reader, uint32_t property_id, uint64_t* out_time_us);
heif_error_code_t heif_reader_get_property_mdft(const heif_reader_t* reader, uint32_t property_id, uint64_t* out_time_us);
heif_error_code_t heif_reader_get_property_raw_type(const heif_reader_t* reader, uint32_t property_id, char out_type[5]);
heif_error_code_t heif_reader_get_property_raw_data(const heif_reader_t* reader, uint32_t property_id, uint8_t* buffer, uint64_t* size_inout);

heif_error_code_t heif_reader_get_property_lsel(const heif_reader_t* reader, uint32_t property_id, uint16_t* out_layer_id);
heif_error_code_t heif_reader_get_property_tols(const heif_reader_t* reader, uint32_t property_id, uint16_t* out_target_ols_index);

typedef struct heif_oinf_ptl {
    uint32_t struct_size;
    uint8_t  general_profile_space;
    uint8_t  general_tier_flag;
    uint8_t  general_profile_idc;
    uint8_t  general_level_idc;
    uint32_t general_profile_compatibility_flags;
    uint64_t general_constraint_indicator_flags;
} heif_oinf_ptl_t;

typedef struct heif_oinf_op_layer {
    uint32_t struct_size;
    uint8_t  ptl_idx;
    uint8_t  layer_id;
    uint8_t  is_output_layer;
    uint8_t  is_alternate_output_layer;
} heif_oinf_op_layer_t;

typedef struct heif_oinf_op {
    uint32_t struct_size;
    uint32_t max_bit_rate;
    uint32_t avg_bit_rate;
    uint16_t output_layer_set_idx;
    uint16_t min_pic_width;
    uint16_t min_pic_height;
    uint16_t max_pic_width;
    uint16_t max_pic_height;
    uint16_t avg_frame_rate;
    uint8_t  max_temporal_id;
    uint8_t  max_chroma_format;
    uint8_t  max_bit_depth_minus8;
    uint8_t  frame_rate_info_flag;
    uint8_t  bit_rate_info_flag;
    uint8_t  constant_frame_rate;
} heif_oinf_op_t;

heif_error_code_t heif_reader_get_property_oinf(const heif_reader_t* reader, uint32_t property_id, uint16_t* out_scalability_mask);
heif_error_code_t heif_reader_get_property_oinf_ptl_count(const heif_reader_t* reader, uint32_t property_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_property_oinf_ptl_at(const heif_reader_t* reader, uint32_t property_id, uint32_t index, heif_oinf_ptl_t* out_ptl);
heif_error_code_t heif_reader_get_property_oinf_op_count(const heif_reader_t* reader, uint32_t property_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_property_oinf_op_at(const heif_reader_t* reader, uint32_t property_id, uint32_t index, heif_oinf_op_t* out_op);
heif_error_code_t heif_reader_get_property_oinf_op_layer_count(const heif_reader_t* reader, uint32_t property_id, uint32_t op_index, uint32_t* out_count);
heif_error_code_t heif_reader_get_property_oinf_op_layer_at(const heif_reader_t* reader, uint32_t property_id, uint32_t op_index, uint32_t layer_index, heif_oinf_op_layer_t* out_layer);
heif_error_code_t heif_reader_get_property_oinf_layer_count(const heif_reader_t* reader, uint32_t property_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_property_oinf_layer_at(const heif_reader_t* reader, uint32_t property_id, uint32_t index, uint8_t* out_dependent_layer_id);
heif_error_code_t heif_reader_get_property_oinf_layer_dep_count(const heif_reader_t* reader, uint32_t property_id, uint32_t layer_index, uint32_t* out_count);
heif_error_code_t heif_reader_get_property_oinf_layer_dep_at(const heif_reader_t* reader, uint32_t property_id, uint32_t layer_index, uint32_t dep_index, uint8_t* out_dep_layer_id);
heif_error_code_t heif_reader_get_property_oinf_layer_dim_count(const heif_reader_t* reader, uint32_t property_id, uint32_t layer_index, uint32_t* out_count);
heif_error_code_t heif_reader_get_property_oinf_layer_dim_at(const heif_reader_t* reader, uint32_t property_id, uint32_t layer_index, uint32_t dim_index, uint8_t* out_dimension_id);

typedef struct heif_subs_entry_t {
    uint32_t struct_size;
    uint32_t subsample_size;
    uint8_t  subsample_priority;
    uint8_t  discardable;
    uint8_t  _reserved_pad0;
    uint8_t  _reserved_pad1;
    uint32_t codec_specific_parameters;
} heif_subs_entry_t;

heif_error_code_t heif_reader_get_property_subs_count(const heif_reader_t* reader, uint32_t property_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_property_subs_at(const heif_reader_t* reader, uint32_t property_id, uint32_t index, heif_subs_entry_t* out_entry);

/* track / sequence surface */

typedef struct heif_track_info {
    uint32_t struct_size;
    uint32_t alternate_group_id;
    uint64_t max_sample_size;
    uint32_t track_id;
    uint32_t features;
    uint32_t time_scale;
    uint32_t frame_rate_num;
    uint32_t frame_rate_den;
    uint32_t minor_version;
    uint8_t  has_type_information;
    uint8_t  _reserved[3];
    char     major_brand[5];
    uint8_t  _reserved2[7];
} heif_track_info_t;

typedef struct heif_sample_info {
    uint32_t struct_size;
    uint32_t sample_id;
    uint64_t sample_duration_ts;
    uint64_t size;
    uint64_t earliest_timestamp;
    uint64_t earliest_timestamp_ts;
    int64_t  sample_composition_offset_ts;
    uint32_t sample_description_index;
    uint32_t segment_id;
    uint32_t sample_flags;
    uint8_t  sample_type;
    uint8_t  has_clap;
    uint8_t  has_auxi;
    uint8_t  coding_constraints;
    char     sample_entry_type[5];
    uint8_t  _reserved[3];
} heif_sample_info_t;

typedef struct heif_edit_unit {
    uint32_t struct_size;
    uint8_t  edit_type;
    uint8_t  _reserved[3];
    int64_t  media_time_in_track_ts;
    uint64_t duration_in_movie_ts;
    int16_t  media_rate_integer;
    int16_t  media_rate_fraction;
    uint8_t  _reserved2[4];
} heif_edit_unit_t;

typedef struct heif_sample_eqiv {
    uint32_t struct_size;
    uint32_t sample_group_description_index;
    int16_t  time_offset;
    uint16_t timescale_multiplier;
} heif_sample_eqiv_t;

typedef struct heif_sample_stmi {
    uint32_t struct_size;
    uint32_t sample_group_description_index;
} heif_sample_stmi_t;

typedef struct heif_sample_refs {
    uint32_t struct_size;
    uint32_t sample_group_description_index;
    uint32_t sample_id;
    uint8_t  _reserved[4];
} heif_sample_refs_t;

typedef struct heif_sample_trif {
    uint32_t struct_size;
    uint32_t sample_group_description_index;
    uint16_t group_id;
    uint16_t independent_idc;
    uint16_t horizontal_offset;
    uint16_t vertical_offset;
    uint16_t region_width;
    uint16_t region_height;
    uint8_t  rect_region_flag;
    uint8_t  full_picture;
    uint8_t  filtering_disabled;
    uint8_t  _reserved;
} heif_sample_trif_t;

typedef struct heif_sample_spor {
    uint32_t struct_size;
    uint32_t sample_group_description_index;
    uint16_t subpic_id_bit_pos;
    uint8_t  subpic_id_info_flag;
    uint8_t  subpicd_len_minus1;
    uint8_t  start_code_emul_flag;
    uint8_t  pps_sps_subpic_id_flag;
    uint8_t  pps_id;
    uint8_t  sps_id;
} heif_sample_spor_t;

typedef struct heif_sample_sulm {
    uint32_t struct_size;
    uint32_t sample_group_description_index;
    char     group_id_info_4cc[5];
    uint8_t  _reserved[3];
} heif_sample_sulm_t;

typedef struct heif_sample_minp {
    uint32_t struct_size;
    uint32_t sample_group_description_index;
    uint16_t pps_mix_nalu_types_in_pic_bit_pos;
    uint8_t  pps_id;
    uint8_t  _reserved;
} heif_sample_minp_t;

heif_error_code_t heif_reader_get_track_count(const heif_reader_t* reader, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_id_at(const heif_reader_t* reader, uint32_t index, uint32_t* out_track_id);
heif_error_code_t heif_reader_get_track_info(const heif_reader_t* reader, uint32_t track_id, heif_track_info_t* out_info);
heif_error_code_t heif_reader_get_track_display_width(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_width);
heif_error_code_t heif_reader_get_track_display_height(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_height);
heif_error_code_t heif_reader_get_track_playback_duration_secs(const heif_reader_t* reader, uint32_t track_id, double* out_secs);
heif_error_code_t heif_reader_get_track_alternate_track_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_alternate_track_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, uint32_t* out_alt_track_id);
heif_error_code_t heif_reader_get_track_reference_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_reference_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, char out_reference_type[5], uint32_t* out_referenced_track_id);
heif_error_code_t heif_reader_get_track_group_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_group_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, char out_group_type[5], uint32_t* out_member_track_id);
heif_error_code_t heif_reader_get_track_compatible_brand_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_compatible_brand_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, char out_brand[5]);
heif_error_code_t heif_reader_get_track_sample_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, heif_sample_info_t* out_info);
heif_error_code_t heif_reader_get_track_sample_data_with_decoder_parameters(heif_reader_t* reader, uint32_t track_id, uint32_t sample_id, uint8_t* buffer, uint64_t* size_inout);
heif_error_code_t heif_reader_get_track_sample_data(heif_reader_t* reader, uint32_t track_id, uint32_t sample_id, uint8_t* buffer, uint64_t* size_inout);
heif_error_code_t heif_reader_get_track_sample_decoder_code_type(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_id, char out_type[5]);
heif_error_code_t heif_reader_get_track_sample_item_type(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_id, char out_type[5]);
heif_error_code_t heif_reader_get_track_sample_decoder_param_set_count(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_decoder_param_set_type_at(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_id, uint32_t index, uint8_t* out_type);
heif_error_code_t heif_reader_get_track_sample_decoder_param_set_data_at(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_id, uint32_t index, uint8_t* buffer, uint64_t* size_inout);
heif_error_code_t heif_reader_get_track_decoding_order_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_decoding_order_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, int64_t* out_timestamp_ms, uint32_t* out_sample_id);
heif_error_code_t heif_reader_get_track_sample_decode_dependency_count(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_decode_dependency_at(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_id, uint32_t index, uint32_t* out_dep_sample_id);
heif_error_code_t heif_reader_get_track_sample_width(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_id, uint32_t* out_width);
heif_error_code_t heif_reader_get_track_sample_height(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_id, uint32_t* out_height);
heif_error_code_t heif_reader_get_track_matrix_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, int32_t* out_value);
heif_error_code_t heif_reader_get_track_master_image_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_master_image_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, uint32_t* out_sample_id);
heif_error_code_t heif_reader_get_track_sample_list_by_type_count(const heif_reader_t* reader, uint32_t track_id, uint8_t sample_type, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_list_by_type_at(const heif_reader_t* reader, uint32_t track_id, uint8_t sample_type, uint32_t index, uint32_t* out_sample_id);
heif_error_code_t heif_reader_get_track_sample_clap(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_description_index, heif_clap_t* out_clap);
heif_error_code_t heif_reader_get_track_sample_auxi_type(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_description_index, char* buffer, uint64_t* size_inout);
heif_error_code_t heif_reader_get_track_sample_auxi_subtype(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_description_index, uint8_t* buffer, uint64_t* size_inout);
heif_error_code_t heif_reader_get_track_item_timestamp_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_item_timestamp_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, int64_t* out_timestamp_ms, uint32_t* out_sample_id);
heif_error_code_t heif_reader_get_item_timestamp_count(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_item_timestamp_at(const heif_reader_t* reader, uint32_t track_id, uint32_t sample_id, uint32_t index, int64_t* out_timestamp_ms);
heif_error_code_t heif_reader_get_track_edit_list_meta(const heif_reader_t* reader, uint32_t track_id, int* out_looping, double* out_repetitions);
heif_error_code_t heif_reader_get_track_edit_unit_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_edit_unit_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, heif_edit_unit_t* out_unit);
heif_error_code_t heif_reader_get_track_sample_eqiv_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_eqiv_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, heif_sample_eqiv_t* out_eqiv);
heif_error_code_t heif_reader_get_track_sample_stmi_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_stmi_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, heif_sample_stmi_t* out_stmi);
heif_error_code_t heif_reader_get_track_sample_stmi_metadata_item_count(const heif_reader_t* reader, uint32_t track_id, uint32_t stmi_index, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_stmi_metadata_item_at(const heif_reader_t* reader, uint32_t track_id, uint32_t stmi_index, uint32_t item_index, uint32_t* out_metadata_item_id);
heif_error_code_t heif_reader_get_track_sample_refs_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_refs_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, heif_sample_refs_t* out_refs);
heif_error_code_t heif_reader_get_track_sample_refs_reference_item_count(const heif_reader_t* reader, uint32_t track_id, uint32_t refs_index, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_refs_reference_item_at(const heif_reader_t* reader, uint32_t track_id, uint32_t refs_index, uint32_t item_index, uint32_t* out_reference_item_id);
heif_error_code_t heif_reader_get_track_sample_trif_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_trif_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, heif_sample_trif_t* out_trif);
heif_error_code_t heif_reader_get_track_sample_trif_dependency_count(const heif_reader_t* reader, uint32_t track_id, uint32_t trif_index, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_trif_dependency_at(const heif_reader_t* reader, uint32_t track_id, uint32_t trif_index, uint32_t dep_index, uint16_t* out_dependency_group_id);
heif_error_code_t heif_reader_get_track_sample_spor_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_spor_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, heif_sample_spor_t* out_spor);
heif_error_code_t heif_reader_get_track_sample_spor_subp_track_ref_idx_count(const heif_reader_t* reader, uint32_t track_id, uint32_t spor_index, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_spor_subp_track_ref_idx_at(const heif_reader_t* reader, uint32_t track_id, uint32_t spor_index, uint32_t ref_idx, uint16_t* out_subp_track_ref_idx);
heif_error_code_t heif_reader_get_track_sample_sulm_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_sulm_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, heif_sample_sulm_t* out_sulm);
heif_error_code_t heif_reader_get_track_sample_sulm_group_id_count(const heif_reader_t* reader, uint32_t track_id, uint32_t sulm_index, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_sulm_group_id_at(const heif_reader_t* reader, uint32_t track_id, uint32_t sulm_index, uint32_t group_idx, uint16_t* out_group_id);
heif_error_code_t heif_reader_get_track_sample_minp_count(const heif_reader_t* reader, uint32_t track_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_minp_at(const heif_reader_t* reader, uint32_t track_id, uint32_t index, heif_sample_minp_t* out_minp);
heif_error_code_t heif_reader_get_track_sample_minp_mix_subp_track_idx1_count(const heif_reader_t* reader, uint32_t track_id, uint32_t minp_index, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_minp_mix_subp_track_idx1_at(const heif_reader_t* reader, uint32_t track_id, uint32_t minp_index, uint32_t idx1_index, uint16_t* out_value);
heif_error_code_t heif_reader_get_track_sample_minp_mix_subp_track_idx2_count(const heif_reader_t* reader, uint32_t track_id, uint32_t minp_index, uint32_t* out_count);
heif_error_code_t heif_reader_get_track_sample_minp_mix_subp_track_idx2_at(const heif_reader_t* reader, uint32_t track_id, uint32_t minp_index, uint32_t idx2_index, uint16_t* out_value);

typedef struct heif_segment_info {
    uint32_t struct_size;
    uint32_t segment_id;
    uint64_t earliest_pts_in_ts;
    uint64_t start_data_offset;
    uint32_t reference_id;
    uint32_t timescale;
    uint32_t duration_in_ts;
    uint32_t data_size;
    uint8_t  reference_type;
    uint8_t  starts_with_sap;
    uint8_t  sap_type;
    uint8_t  _reserved[5];
} heif_segment_info_t;

heif_error_code_t heif_reader_get_segment_index_count(heif_reader_t* reader, uint32_t* out_count);
heif_error_code_t heif_reader_get_segment_index_at(heif_reader_t* reader, uint32_t index, heif_segment_info_t* out_segment);

/* entity groups */
typedef struct heif_entity_group {
    uint32_t struct_size;
    uint32_t group_id;
    char     type[5];
    uint8_t  _reserved[3];
} heif_entity_group_t;

heif_error_code_t heif_reader_get_entity_group_count(const heif_reader_t* reader, uint32_t* out_count);
heif_error_code_t heif_reader_get_entity_group_at(const heif_reader_t* reader, uint32_t index, heif_entity_group_t* out_group);
heif_error_code_t heif_reader_get_entity_group_entity_count(const heif_reader_t* reader, uint32_t group_index, uint32_t* out_count);
heif_error_code_t heif_reader_get_entity_group_entity_at(const heif_reader_t* reader, uint32_t group_index, uint32_t entity_index, uint32_t* out_entity_id);
heif_error_code_t heif_reader_get_group_property_count(const heif_reader_t* reader, uint32_t group_id, uint32_t* out_count);
heif_error_code_t heif_reader_get_group_property_at(const heif_reader_t* reader, uint32_t group_id, uint32_t index, uint32_t* out_property_id, char out_type[5], int* out_essential);

/* ---- segmented playback -- opaque stream handle + 4 reader bridges
 *      Lifetime contract mirrors srcs/api/reader/heifreader_c.h:
 *        - parse_initialization_segment: stream pinned for life of reader
 *        - parse_segment(seg_id): stream pinned until invalidate_segment(seg_id)
 *        - parse_segment_index: stream used only for the duration of the call
 *      Bindings MUST hold a strong Python reference to each ffi.callback
 *      table for at least the lifetime window above; Reader does this via
 *      _init_stream + _segment_streams[seg_id].
 */

typedef struct heif_stream heif_stream_t;

heif_stream_t*    heif_stream_create(const heif_stream_callbacks_t* callbacks);
void              heif_stream_destroy(heif_stream_t* stream);
heif_error_code_t heif_reader_parse_initialization_segment(heif_reader_t* reader, heif_stream_t* stream);
heif_error_code_t heif_reader_parse_segment(heif_reader_t* reader, heif_stream_t* stream, uint32_t segment_id, uint64_t earliest_pts_in_ts);
heif_error_code_t heif_reader_invalidate_segment(heif_reader_t* reader, uint32_t segment_id);
heif_error_code_t heif_reader_parse_segment_index(heif_reader_t* reader, heif_stream_t* stream, heif_segment_info_t* out_segments, uint32_t* count_inout);
"""


def _default_library_name() -> str:
    """Platform-default soname / dll name for ``libheif_shared``."""
    if sys.platform.startswith("linux"):
        # SOVERSION 4 is the v4.x line.  Plain "libheif_shared.so"
        # (without the version suffix) is only present in install
        # prefixes that ship the dev symlink, which a binary
        # consumer cannot rely on.
        return "libheif_shared.so.4"
    if sys.platform == "darwin":
        return "libheif_shared.4.dylib"
    if sys.platform == "win32":
        # Windows DLLs do not carry SOVERSION; the import-library
        # convention names the DLL after the link name.
        return "heif_shared.dll"
    raise RuntimeError(f"unsupported platform: {sys.platform!r}")


def _platform_soname_globs() -> Tuple[str, ...]:
    """Filename globs for ``libheif_shared`` on the current platform,
    in priority order.  Versioned names first, then bare names, then
    development symlinks.

    Used by ``_search_dev_tree`` and the wheel-bundled lookup in
    :func:`_find_library_in_dir`.  Kept as a tuple for consistent
    search-order semantics.
    """
    if sys.platform.startswith("linux"):
        return ("libheif_shared.so.4", "libheif_shared.so.*", "libheif_shared.so")
    if sys.platform == "darwin":
        return ("libheif_shared.4.dylib", "libheif_shared.*.dylib", "libheif_shared.dylib")
    if sys.platform == "win32":
        return ("heif_shared.dll",)
    raise RuntimeError(f"unsupported platform: {sys.platform!r}")


def _find_library_in_dir(directory: str) -> str | None:
    """Return the absolute path of a libheif_shared file in
    *directory* if present, else ``None``.  Search uses the
    platform-specific globs from :func:`_platform_soname_globs`."""
    if not os.path.isdir(directory):
        return None
    import glob
    for pat in _platform_soname_globs():
        hits = sorted(glob.glob(os.path.join(directory, pat)))
        for hit in hits:
            if os.path.isfile(hit) or os.path.islink(hit):
                return os.path.abspath(hit)
    return None


# Build directories that the in-tree dev workflow uses, in priority
# order.  The first one whose ``lib/`` (Linux/macOS) or ``bin/``
# (Windows) holds a libheif_shared file wins.
_DEV_BUILD_DIRS: Tuple[str, ...] = (
    "build",
)


def _search_dev_tree() -> str | None:
    """Walk up from the binding's location looking for an in-tree
    build directory.  Lets ``pip install -e bindings/python/`` find
    the C library a developer just built with
    ``cmake --build build`` without any env-var fiddling.

    Search algorithm: starting at the directory containing this
    module, ascend through parents.  At each level, look for the
    repo-root marker (a directory containing both ``srcs/`` and
    ``bindings/``).  Once found, scan the candidate build dirs
    underneath it.  Stops as soon as the first hit is found.
    Returns ``None`` (not an exception) when nothing is found, so
    the caller can fall through to the next discovery tier.
    """
    here = os.path.dirname(os.path.abspath(__file__))
    candidate = here
    repo_root: str | None = None
    for _ in range(8):  # bounded ascent -- never walk past 8 levels
        srcs   = os.path.join(candidate, "srcs")
        binds  = os.path.join(candidate, "bindings")
        if os.path.isdir(srcs) and os.path.isdir(binds):
            repo_root = candidate
            break
        parent = os.path.dirname(candidate)
        if parent == candidate:
            break
        candidate = parent

    if repo_root is None:
        return None

    libdir = "bin" if sys.platform == "win32" else "lib"
    for build_name in _DEV_BUILD_DIRS:
        hit = _find_library_in_dir(os.path.join(repo_root, build_name, libdir))
        if hit is not None:
            return hit
    return None


def _search_wheel_bundle() -> str | None:
    """When the binding is installed from a wheel that bundled the
    library next to ``_ffi.py`` (wheel bundles), ``dlopen`` it
    by absolute path so the system linker does not need an
    LD_LIBRARY_PATH / @rpath dance.

    Currently a no-op for editable installs, but the file shape
    works the moment a wheel-build process drops the library here.
    """
    return _find_library_in_dir(os.path.dirname(os.path.abspath(__file__)))


def _resolve_library_path() -> Tuple[str, list[str]]:
    """Multi-tier discovery of the HEIF shared library.

    Resolution order -- first hit wins:

      1. ``HEIF_LIBRARY_PATH`` env var (absolute path).  CI pipelines
         and anyone targeting a specific build flavour use this to
         pin the answer.
      2. Wheel-bundled library next to ``_ffi.py``.  Lights up
         under discoverabilityw when ``pip install nokia-heif`` ships a real
         binary wheel.
      3. In-tree dev build directory (``build/``)
         under the repo root.  Lets a fresh
         ``git clone && cmake --build build && pip install -e .``
         work without any env vars.
      4. Platform-default soname looked up by the dynamic linker
         (``ld.so`` / ``LoadLibrary`` / ``dyld``).  Catches users
         who installed the library via ``cmake --install`` or a
         system package.

    Returns ``(target, attempted)`` where ``target`` is the value
    handed to ``ffi.dlopen`` and ``attempted`` is the trace of
    locations checked, used to compose a useful diagnostic when
    every tier misses.
    """
    attempted: list[str] = []

    # Tier 1 -- explicit override.
    override = os.environ.get("HEIF_LIBRARY_PATH")
    if override:
        attempted.append(f"HEIF_LIBRARY_PATH={override!r}")
        return (override, attempted)

    # Tier 2 -- wheel-bundled.
    bundled = _search_wheel_bundle()
    if bundled is not None:
        attempted.append(f"wheel-bundled: {bundled!r}")
        return (bundled, attempted)
    attempted.append(
        "wheel-bundled (next to heif/_ffi.py): not present"
    )

    # Tier 3 -- in-tree dev build search.
    dev = _search_dev_tree()
    if dev is not None:
        attempted.append(f"in-tree dev build: {dev!r}")
        return (dev, attempted)
    attempted.append(
        "in-tree dev build (build*/lib/, walking up from heif/_ffi.py): "
        "no libheif_shared found"
    )

    # Tier 4 -- system dynamic-linker default.
    soname = _default_library_name()
    attempted.append(f"system dynamic linker: {soname!r}")
    return (soname, attempted)


def _load_library() -> Tuple[cffi.FFI, "cffi.api.FFI.CData"]:
    """Build the ``FFI`` and ``dlopen`` the HEIF shared library.

    Discovery is delegated to :func:`_resolve_library_path`; see that
    function's docstring for the four-tier order.  On total failure
    every tier's attempted location is reported in the ``RuntimeError``
    message so the user knows exactly what to fix (build the library,
    set ``HEIF_LIBRARY_PATH``, or ``cmake --install`` it).
    """
    ffi = cffi.FFI()
    ffi.cdef(_CDEF)

    target, attempted = _resolve_library_path()
    try:
        lib = ffi.dlopen(target)
    except OSError as exc:
        trace = "\n  ".join(attempted)
        raise RuntimeError(
            f"failed to load HEIF shared library: {exc}\n"
            f"resolution chain (in order):\n  {trace}\n"
            "Fix one of the tiers above, e.g.:\n"
            "  - build it in-tree:    cmake -S . -B build && cmake --build build\n"
            "  - install it system-wide: sudo cmake --install build\n"
            "  - point at a custom build: export HEIF_LIBRARY_PATH=/abs/path/to/libheif_shared.so.4"
        ) from exc
    return ffi, lib


# Process-singleton.  Built lazily on first import; subsequent
# imports reuse the same handles.
ffi, lib = _load_library()


def fourcc_to_str(buf) -> str:
    """Decode a 5-byte FourCC ``char[5]`` buffer to a Python ``str``.

    The C contract guarantees a NUL terminator at index 4, so we read
    the four payload bytes and decode as ASCII.  Non-printable bytes
    are escaped with ``backslashreplace`` so a malformed asset cannot
    raise during diagnostic printing.
    """
    raw = ffi.buffer(buf, 4)[:]
    return raw.decode("ascii", errors="backslashreplace")


def drain_last_error() -> dict:
    """Snapshot the thread-local error slot into a Python ``dict``.

    Called immediately after a non-OK return from any reader C-facade entry
    point.  Returns the structured failure context that
    :class:`heif.HeifError` wraps.  Does NOT clear the slot -- the
    caller is free to re-read it; ``heif.Reader`` does call
    ``heif_clear_last_error`` after raising so the slot does not
    leak across calls.
    """
    # Probe the message length first (snprintf-style), then allocate
    # exactly the right buffer.
    needed_msg = lib.heif_get_last_error_message(ffi.NULL, 0)
    msg_buf = ffi.new("char[]", needed_msg + 1)
    lib.heif_get_last_error_message(msg_buf, needed_msg + 1)

    needed_cause = lib.heif_get_last_error_cause(ffi.NULL, 0)
    cause_buf = ffi.new("char[]", needed_cause + 1)
    lib.heif_get_last_error_cause(cause_buf, needed_cause + 1)

    def _str(p) -> str:
        # heif_get_last_error_{file,origin_name,code_name} return
        # static-storage strings; they are never NULL by contract.
        return ffi.string(p).decode("utf-8", errors="backslashreplace")

    return {
        "code":        int(lib.heif_get_last_error_code()),
        "code_name":   _str(lib.heif_get_last_error_code_name()),
        "origin":      int(lib.heif_get_last_error_origin()),
        "origin_name": _str(lib.heif_get_last_error_origin_name()),
        "file":        _str(lib.heif_get_last_error_file()),
        "line":        int(lib.heif_get_last_error_line()),
        "message":     ffi.string(msg_buf).decode("utf-8", errors="backslashreplace"),
        "cause":       ffi.string(cause_buf).decode("utf-8", errors="backslashreplace"),
    }
