/*
 * heif.js -- JavaScript binding for the HEIF library C facade.
 *
 *   const heif = await HEIF.load();          // load the wasm/asm.js module
 *   const reader = heif.open(uint8Array);    // open a file from memory
 *   console.log(reader.majorBrand);          // "heic", "mif1", "msf1", ...
 *   console.log(reader.compatibleBrands);    // ["mif1", "heic", ...]
 *   console.log(reader.primaryItemId);       // 1
 *   reader.close();                          // releases native resources
 *
 * Design notes for adopters
 *   - We do NOT compile any C++ glue. This file is the entire binding.
 *     Everything calls into `Module._heif_*` via cwrap. To add a new
 *     library function, just wrap it here -- no rebuild of native code.
 *
 *   - Every native call that returns a `heif_error_code_t` is funnelled
 *     through `_check()`, which throws a descriptive `HEIFError` carrying
 *     the code, code-name, origin, line and message pulled from the
 *     library's per-thread last-error state: non-zero return ->
 *     drain `heif_get_last_error_*`.
 *
 *   - String outputs follow the C facade's documented conventions:
 *       * fixed FourCC: `char out[5]` -- we allocate 5 bytes, call,
 *         then `UTF8ToString` the result.
 *       * variable-length: size-probe (call once with NULL/0 to size,
 *         allocate, call again). Not used in this commit yet.
 *
 *   - Streaming (heif_reader_initialize_stream + addFunction trampolines)
 *     is implemented in `_initFromBytes`. For now this just wraps an
 *     in-memory Uint8Array; the same callback table will later support
 *     ranged HTTP fetches without changing the public API.
 */
(function (root) {
    'use strict';

    // Module loader
    //
    // Resolves to a `heif` object. Caller is expected to load
    // heif-raw.js first (the emcc-produced module factory), which sets
    // `globalThis.HEIFModule`. We then await it and bind cwrap signatures.
    // Cache: one HEIF instance per factory. Every reader created via
    // heif.open() shares the same wasm Module, so we don't instantiate
    // a fresh 128 MiB heap per file open.
    let _cachedHeif = null;

    function load(opts) {
        if (_cachedHeif) return Promise.resolve(_cachedHeif);
        const factory = (opts && opts.factory) || root.HEIFModule;
        if (typeof factory !== 'function') {
            throw new Error(
                'heif.js: HEIFModule factory not found. ' +
                'Load heif-raw.js before heif.js, or pass {factory} explicitly.'
            );
        }
        return factory(opts && opts.moduleArgs).then(function (Module) {
            _cachedHeif = new HEIF(Module);
            return _cachedHeif;
        });
    }

    // HEIF: top-level handle, owns the cwrap'd entry points
    function HEIF(Module) {
        if (typeof BigInt === 'undefined') {
            throw new Error(
                'heif.js requires BigInt support (all modern browsers and Node.js >= 10.3). ' +
                'This build was compiled with -sWASM_BIGINT.'
            );
        }
        this._m = Module;

        // Cache cwrap lookups. cwrap itself is cheap, but doing this once
        // up front catches symbol-name typos at construction rather than
        // at call time, and is the right place to add per-feature symbol
        // probes later (e.g. gating optional entry points by presence).
        const cwrap = Module.cwrap;
        this._fn = {
            // Diagnostics
            runtimeVersion:      cwrap('heif_get_runtime_version_string', 'number', []),
            errorCode:           cwrap('heif_get_last_error_code',        'number', []),
            errorCodeName:       cwrap('heif_get_last_error_code_name',   'number', []),
            errorOrigin:         cwrap('heif_get_last_error_origin',      'number', []),
            errorOriginName:     cwrap('heif_get_last_error_origin_name', 'number', []),
            errorLine:           cwrap('heif_get_last_error_line',        'number', []),
            errorMessage:        cwrap('heif_get_last_error_message',     'number', ['number', 'number']),
            clearError:          cwrap('heif_clear_last_error',           null,     []),

            // Reader lifecycle
            readerCreate:        cwrap('heif_reader_create',              'number', []),
            readerDestroy:       cwrap('heif_reader_destroy',             null,     ['number']),
            readerInitStream:    cwrap('heif_reader_initialize_stream',   'number', ['number', 'number']),

            // File-level identity
            getMajorBrand:       cwrap('heif_reader_get_major_brand',     'number', ['number', 'number']),
            getMinorVersion:     cwrap('heif_reader_get_minor_version',   'number', ['number', 'number']),
            getCompatBrandCount: cwrap('heif_reader_get_compatible_brand_count', 'number', ['number', 'number']),
            getCompatBrandAt:    cwrap('heif_reader_get_compatible_brand_at',    'number', ['number', 'number', 'number']),
            getPrimaryItem:      cwrap('heif_reader_get_primary_item',    'number', ['number', 'number']),

            // Item enumeration
            getMasterCount:      cwrap('heif_reader_get_master_image_count',     'number', ['number', 'number']),
            getMasterAt:         cwrap('heif_reader_get_master_image_at',        'number', ['number', 'number', 'number']),
            getByTypeCount:      cwrap('heif_reader_get_item_list_by_type_count','number', ['number', 'string', 'number']),
            getByTypeAt:         cwrap('heif_reader_get_item_list_by_type_at',   'number', ['number', 'string', 'number', 'number']),
            getRefFromCount:     cwrap('heif_reader_get_referenced_from_item_list_by_type_count', 'number', ['number', 'number', 'string', 'number']),
            getRefFromAt:        cwrap('heif_reader_get_referenced_from_item_list_by_type_at',    'number', ['number', 'number', 'string', 'number', 'number']),
            getRefToCount:       cwrap('heif_reader_get_referenced_to_item_list_by_type_count',   'number', ['number', 'number', 'string', 'number']),
            getRefToAt:          cwrap('heif_reader_get_referenced_to_item_list_by_type_at',      'number', ['number', 'number', 'string', 'number', 'number']),

            // Per-image accessors
            getItemType:         cwrap('heif_reader_get_item_type',       'number', ['number', 'number', 'number']),
            getItemFeatures:     cwrap('heif_reader_get_item_features',   'number', ['number', 'number', 'number']),
            getWidth:            cwrap('heif_reader_get_width',           'number', ['number', 'number', 'number']),
            getHeight:           cwrap('heif_reader_get_height',          'number', ['number', 'number', 'number']),

            // Bytestream extraction (size-probe convention)
            getItemDataDP:       cwrap('heif_reader_get_item_data_with_decoder_parameters', 'number', ['number', 'number', 'number', 'number']),
            getItemData:         cwrap('heif_reader_get_item_data',       'number', ['number', 'number', 'number', 'number']),

            // Item properties (irot/imir/ispe/pixi/colr/auxC/clap/iscl/lsel)
            getPropCount:        cwrap('heif_reader_get_item_property_count', 'number', ['number', 'number', 'number']),
            getPropAt:           cwrap('heif_reader_get_item_property_at',    'number', ['number', 'number', 'number', 'number', 'number', 'number']),
            getPropIspe:         cwrap('heif_reader_get_property_ispe',       'number', ['number', 'number', 'number', 'number']),
            getPropIrot:         cwrap('heif_reader_get_property_irot',       'number', ['number', 'number', 'number']),
            getPropImir:         cwrap('heif_reader_get_property_imir',       'number', ['number', 'number', 'number']),
            getPropClap:         cwrap('heif_reader_get_property_clap',       'number', ['number', 'number', 'number']),
            getPropIscl:         cwrap('heif_reader_get_property_iscl',       'number', ['number', 'number', 'number', 'number', 'number', 'number']),
            getPropLsel:         cwrap('heif_reader_get_property_lsel',       'number', ['number', 'number', 'number']),
            getPropTols:         cwrap('heif_reader_get_property_tols',       'number', ['number', 'number', 'number']),
            getPropRloc:         cwrap('heif_reader_get_property_rloc',       'number', ['number', 'number', 'number', 'number']),
            getPropPixiCount:    cwrap('heif_reader_get_property_pixi_channel_count', 'number', ['number', 'number', 'number']),
            getPropPixiBitsAt:   cwrap('heif_reader_get_property_pixi_bits_at',       'number', ['number', 'number', 'number', 'number']),
            getPropPixiNumCh:    cwrap('heif_reader_get_property_pixi_num_channels',  'number', ['number', 'number', 'number']),
            getPropPixiChAt:     cwrap('heif_reader_get_property_pixi_channel_at',    'number', ['number', 'number', 'number', 'number']),
            getPropPixiChLabel:  cwrap('heif_reader_get_property_pixi_channel_label', 'number', ['number', 'number', 'number', 'number', 'number']),
            getPropColrType:     cwrap('heif_reader_get_property_colr_type',  'number', ['number', 'number', 'number']),
            getPropColrNclx:     cwrap('heif_reader_get_property_colr_nclx',  'number', ['number', 'number', 'number', 'number', 'number', 'number']),
            getPropColrIcc:      cwrap('heif_reader_get_property_colr_icc',   'number', ['number', 'number', 'number', 'number']),
            getPropAuxcType:     cwrap('heif_reader_get_property_auxc_type',  'number', ['number', 'number', 'number', 'number']),

            // Operating-Points-Info ('oinf') -- 13 entry points
            getPropOinf:              cwrap('heif_reader_get_property_oinf',                'number', ['number', 'number', 'number']),
            getPropOinfPtlCount:      cwrap('heif_reader_get_property_oinf_ptl_count',      'number', ['number', 'number', 'number']),
            getPropOinfPtlAt:         cwrap('heif_reader_get_property_oinf_ptl_at',         'number', ['number', 'number', 'number', 'number']),
            getPropOinfOpCount:       cwrap('heif_reader_get_property_oinf_op_count',       'number', ['number', 'number', 'number']),
            getPropOinfOpAt:          cwrap('heif_reader_get_property_oinf_op_at',          'number', ['number', 'number', 'number', 'number']),
            getPropOinfOpLayerCount:  cwrap('heif_reader_get_property_oinf_op_layer_count', 'number', ['number', 'number', 'number', 'number']),
            getPropOinfOpLayerAt:     cwrap('heif_reader_get_property_oinf_op_layer_at',    'number', ['number', 'number', 'number', 'number', 'number']),
            getPropOinfLayerCount:    cwrap('heif_reader_get_property_oinf_layer_count',    'number', ['number', 'number', 'number']),
            getPropOinfLayerAt:       cwrap('heif_reader_get_property_oinf_layer_at',       'number', ['number', 'number', 'number', 'number']),
            getPropOinfLayerDepCount: cwrap('heif_reader_get_property_oinf_layer_dep_count','number', ['number', 'number', 'number', 'number']),
            getPropOinfLayerDepAt:    cwrap('heif_reader_get_property_oinf_layer_dep_at',   'number', ['number', 'number', 'number', 'number', 'number']),
            getPropOinfLayerDimCount: cwrap('heif_reader_get_property_oinf_layer_dim_count','number', ['number', 'number', 'number', 'number']),
            getPropOinfLayerDimAt:    cwrap('heif_reader_get_property_oinf_layer_dim_at',   'number', ['number', 'number', 'number', 'number', 'number']),

            // Item-level decoder parameter sets (VPS/SPS/PPS/APS)
            getItemPsCount:          cwrap('heif_reader_get_decoder_param_set_count',       'number', ['number', 'number', 'number']),
            getItemPsTypeAt:         cwrap('heif_reader_get_decoder_param_set_type_at',     'number', ['number', 'number', 'number', 'number']),
            getItemPsDataAt:         cwrap('heif_reader_get_decoder_param_set_data_at',     'number', ['number', 'number', 'number', 'number', 'number']),
            getItemPsAllCount:       cwrap('heif_reader_get_decoder_param_set_all_layers_count',   'number', ['number', 'number', 'number']),
            getItemPsAllTypeAt:      cwrap('heif_reader_get_decoder_param_set_all_layers_type_at', 'number', ['number', 'number', 'number', 'number']),
            getItemPsAllDataAt:      cwrap('heif_reader_get_decoder_param_set_all_layers_data_at', 'number', ['number', 'number', 'number', 'number', 'number']),

            // Composition (grid / overlay)
            getGrid:             cwrap('heif_reader_get_grid',                'number', ['number', 'number', 'number']),
            getGridImageCount:   cwrap('heif_reader_get_grid_image_count',    'number', ['number', 'number', 'number']),
            getGridImageAt:      cwrap('heif_reader_get_grid_image_at',       'number', ['number', 'number', 'number', 'number']),
            getOverlay:          cwrap('heif_reader_get_overlay',             'number', ['number', 'number', 'number']),
            getOverlayImageCount:cwrap('heif_reader_get_overlay_image_count', 'number', ['number', 'number', 'number']),
            getOverlayImageAt:   cwrap('heif_reader_get_overlay_image_at',    'number', ['number', 'number', 'number', 'number']),

            // Tonemap / gainmap (`tmap` derived items)
            getTmapGainmap:        cwrap('heif_reader_get_tonemap_gainmap',                'number', ['number', 'number', 'number']),
            getTmapChannelCount:   cwrap('heif_reader_get_tonemap_gainmap_channel_count',  'number', ['number', 'number', 'number']),
            getTmapChannelAt:      cwrap('heif_reader_get_tonemap_gainmap_channel_at',     'number', ['number', 'number', 'number', 'number']),
            getTmapImageCount:     cwrap('heif_reader_get_tonemap_image_count',            'number', ['number', 'number', 'number']),
            getTmapImageAt:        cwrap('heif_reader_get_tonemap_image_at',               'number', ['number', 'number', 'number', 'number']),

            // Entity groups (`GroupsListBox`: stereo pairs, alternate, equiv, etc.)
            getEntGrpCount:        cwrap('heif_reader_get_entity_group_count',         'number', ['number', 'number']),
            getEntGrpAt:           cwrap('heif_reader_get_entity_group_at',            'number', ['number', 'number', 'number']),
            getEntGrpEntCount:     cwrap('heif_reader_get_entity_group_entity_count',  'number', ['number', 'number', 'number']),
            getEntGrpEntAt:        cwrap('heif_reader_get_entity_group_entity_at',     'number', ['number', 'number', 'number', 'number']),

            // Tracks (msf1 image sequences and similar)
            getTrackCount:       cwrap('heif_reader_get_track_count',         'number', ['number', 'number']),
            getTrackIdAt:        cwrap('heif_reader_get_track_id_at',         'number', ['number', 'number', 'number']),
            getTrackInfo:        cwrap('heif_reader_get_track_info',          'number', ['number', 'number', 'number']),
            getTrackDispW:       cwrap('heif_reader_get_track_display_width', 'number', ['number', 'number', 'number']),
            getTrackDispH:       cwrap('heif_reader_get_track_display_height','number', ['number', 'number', 'number']),
            getTrackDuration:    cwrap('heif_reader_get_track_playback_duration_secs', 'number', ['number', 'number', 'number']),
            getTrackSampleCount: cwrap('heif_reader_get_track_sample_count',  'number', ['number', 'number', 'number']),
            getTrackSampleAt:    cwrap('heif_reader_get_track_sample_at',     'number', ['number', 'number', 'number', 'number']),
            getTrackTsCount:     cwrap('heif_reader_get_track_item_timestamp_count', 'number', ['number', 'number', 'number']),
            getTrackTsAt:        cwrap('heif_reader_get_track_item_timestamp_at',    'number', ['number', 'number', 'number', 'number', 'number']),
            getTrackRefCount:    cwrap('heif_reader_get_track_reference_count','number', ['number', 'number', 'number']),
            getTrackRefAt:       cwrap('heif_reader_get_track_reference_at',  'number', ['number', 'number', 'number', 'number', 'number']),

            // Track-sample bytestream extraction. Both variants now use the
            // standard size-probe protocol; the C facade may surface
            // the probe miss as either BUFFER_SIZE_TOO_SMALL or
            // MEMORY_TOO_SMALL_BUFFER (both spellings handled in fetchSized).
            getTrackSampleDataDP:cwrap('heif_reader_get_track_sample_data_with_decoder_parameters', 'number', ['number', 'number', 'number', 'number', 'number']),
            getTrackSampleData:  cwrap('heif_reader_get_track_sample_data',  'number', ['number', 'number', 'number', 'number', 'number']),

            // Track edit list (looping/repetitions scalars + heif_edit_unit_t array)
            getTrackEditMeta:    cwrap('heif_reader_get_track_edit_list_meta','number', ['number', 'number', 'number', 'number']),
            getTrackEditCount:   cwrap('heif_reader_get_track_edit_unit_count','number', ['number', 'number', 'number']),
            getTrackEditAt:      cwrap('heif_reader_get_track_edit_unit_at',  'number', ['number', 'number', 'number', 'number']),

            // Alternate tracks (alternateGroupId siblings), track groups, track-type compatible brands
            getTrackAltCount:    cwrap('heif_reader_get_track_alternate_track_count', 'number', ['number', 'number', 'number']),
            getTrackAltAt:       cwrap('heif_reader_get_track_alternate_track_at',    'number', ['number', 'number', 'number', 'number']),
            getTrackGroupCount:  cwrap('heif_reader_get_track_group_count',           'number', ['number', 'number', 'number']),
            getTrackGroupAt:     cwrap('heif_reader_get_track_group_at',              'number', ['number', 'number', 'number', 'number', 'number']),
            getTrackBrandCount:  cwrap('heif_reader_get_track_compatible_brand_count','number', ['number', 'number', 'number']),
            getTrackBrandAt:     cwrap('heif_reader_get_track_compatible_brand_at',   'number', ['number', 'number', 'number', 'number']),

            // Per-sample timestamps (one image, possibly N display times under an edit list)
            getItemTsCount:      cwrap('heif_reader_get_item_timestamp_count', 'number', ['number', 'number', 'number', 'number']),
            getItemTsAt:         cwrap('heif_reader_get_item_timestamp_at',    'number', ['number', 'number', 'number', 'number', 'number']),

            // DASH-style segment index (file-level, from `sidx` boxes)
            getSegCount:         cwrap('heif_reader_get_segment_index_count',  'number', ['number', 'number']),
            getSegAt:            cwrap('heif_reader_get_segment_index_at',     'number', ['number', 'number', 'number']),

            // Per-sample geometry (coded width / height of one sample, distinct from track display dims)
            getTrackSampleW:     cwrap('heif_reader_get_track_sample_width',  'number', ['number', 'number', 'number', 'number']),
            getTrackSampleH:     cwrap('heif_reader_get_track_sample_height', 'number', ['number', 'number', 'number', 'number']),

            // Per-track 3x3 tkhd display matrix (9 calls; one per index)
            getTrackMatrixAt:    cwrap('heif_reader_get_track_matrix_at',     'number', ['number', 'number', 'number', 'number']),

            // Per-track master images (sync / random-access samples)
            getTrackMasterCount: cwrap('heif_reader_get_track_master_image_count', 'number', ['number', 'number', 'number']),
            getTrackMasterAt:    cwrap('heif_reader_get_track_master_image_at',    'number', ['number', 'number', 'number', 'number']),

            // Sample-id list filtered by HEIF::TrackSampleType
            getTrackByTypeCount: cwrap('heif_reader_get_track_sample_list_by_type_count', 'number', ['number', 'number', 'number', 'number']),
            getTrackByTypeAt:    cwrap('heif_reader_get_track_sample_list_by_type_at',    'number', ['number', 'number', 'number', 'number', 'number']),

            // Sample-description-entry properties (sdi is 1-based per ISOBMFF)
            getTrackSampleClap:    cwrap('heif_reader_get_track_sample_clap',         'number', ['number', 'number', 'number', 'number']),
            getTrackSampleAuxiTy:  cwrap('heif_reader_get_track_sample_auxi_type',    'number', ['number', 'number', 'number', 'number', 'number']),
            getTrackSampleAuxiSub: cwrap('heif_reader_get_track_sample_auxi_subtype', 'number', ['number', 'number', 'number', 'number', 'number']),

            // Per-sample decoder config (code type FourCC + parameter sets, for out-of-band feeding)
            getTrackSampleCodeTy:  cwrap('heif_reader_get_track_sample_decoder_code_type',          'number', ['number', 'number', 'number', 'number']),
            getTrackSamplePsCount: cwrap('heif_reader_get_track_sample_decoder_param_set_count',    'number', ['number', 'number', 'number', 'number']),
            getTrackSamplePsTypeAt:cwrap('heif_reader_get_track_sample_decoder_param_set_type_at',  'number', ['number', 'number', 'number', 'number', 'number']),
            getTrackSamplePsDataAt:cwrap('heif_reader_get_track_sample_decoder_param_set_data_at',  'number', ['number', 'number', 'number', 'number', 'number', 'number']),

            // Decoding-order list (presentation-order is the default; this is the order to feed the decoder)
            getTrackDecOrderCount: cwrap('heif_reader_get_track_decoding_order_count', 'number', ['number', 'number', 'number']),
            getTrackDecOrderAt:    cwrap('heif_reader_get_track_decoding_order_at',    'number', ['number', 'number', 'number', 'number', 'number']),

            // Decode-dependencies for one sample (sample ids that must be decoded first; for seek/scrub)
            getTrackDepCount:      cwrap('heif_reader_get_track_sample_decode_dependency_count', 'number', ['number', 'number', 'number', 'number']),
            getTrackDepAt:         cwrap('heif_reader_get_track_sample_decode_dependency_at',    'number', ['number', 'number', 'number', 'number', 'number']),

            // Item-level decode dependencies
            getDecoderCodeType:    cwrap('heif_reader_get_decoder_code_type',     'number', ['number', 'number', 'number']),
            getDecDepCount:        cwrap('heif_reader_get_decode_dependency_count','number', ['number', 'number', 'number']),
            getDecDepAt:           cwrap('heif_reader_get_decode_dependency_at',  'number', ['number', 'number', 'number', 'number']),

            // HDR metadata properties
            getPropClli:           cwrap('heif_reader_get_property_clli',  'number', ['number', 'number', 'number']),
            getPropMdcv:           cwrap('heif_reader_get_property_mdcv',  'number', ['number', 'number', 'number']),
            getPropCclv:           cwrap('heif_reader_get_property_cclv',  'number', ['number', 'number', 'number']),
            getPropAmve:           cwrap('heif_reader_get_property_amve',  'number', ['number', 'number', 'number']),
            getPropReve:           cwrap('heif_reader_get_property_reve',  'number', ['number', 'number', 'number']),
            getPropNdwt:           cwrap('heif_reader_get_property_ndwt',  'number', ['number', 'number', 'number']),

            // Descriptive / miscellaneous properties
            getPropPasp:           cwrap('heif_reader_get_property_pasp',          'number', ['number', 'number', 'number', 'number']),
            getPropCrtt:           cwrap('heif_reader_get_property_crtt',          'number', ['number', 'number', 'number']),
            getPropMdft:           cwrap('heif_reader_get_property_mdft',          'number', ['number', 'number', 'number']),
            getPropUdesField:      cwrap('heif_reader_get_property_udes_field',    'number', ['number', 'number', 'number', 'number', 'number']),
            getPropAlttField:      cwrap('heif_reader_get_property_altt_field',    'number', ['number', 'number', 'number', 'number', 'number']),
            getPropRrefCount:      cwrap('heif_reader_get_property_rref_count',    'number', ['number', 'number', 'number']),
            getPropRrefAt:         cwrap('heif_reader_get_property_rref_at',       'number', ['number', 'number', 'number', 'number']),
            getPropRawType:        cwrap('heif_reader_get_property_raw_type',      'number', ['number', 'number', 'number']),
            getPropRawData:        cwrap('heif_reader_get_property_raw_data',      'number', ['number', 'number', 'number', 'number']),
            getPropAuxcSubtype:    cwrap('heif_reader_get_property_auxc_subtype',  'number', ['number', 'number', 'number', 'number']),
            getPropSubsCount:      cwrap('heif_reader_get_property_subs_count',    'number', ['number', 'number', 'number']),
            getPropSubsAt:         cwrap('heif_reader_get_property_subs_at',       'number', ['number', 'number', 'number', 'number']),

            // File / metabox features
            getFileFeatures:       cwrap('heif_reader_get_file_features',          'number', ['number', 'number']),
            getMetaboxFeatures:    cwrap('heif_reader_get_metabox_features',       'number', ['number', 'number']),
            getProtectionScheme:   cwrap('heif_reader_get_item_protection_scheme', 'number', ['number', 'number', 'number', 'number']),
            getGroupPropCount:     cwrap('heif_reader_get_group_property_count',   'number', ['number', 'number', 'number']),
            getGroupPropAt:        cwrap('heif_reader_get_group_property_at',      'number', ['number', 'number', 'number', 'number', 'number', 'number']),
            getMatrixAt:           cwrap('heif_reader_get_matrix_at',              'number', ['number', 'number', 'number']),
            getBrandComboCount:    cwrap('heif_reader_get_compatible_brand_combo_count',       'number', ['number', 'number']),
            getBrandComboBrandCnt: cwrap('heif_reader_get_compatible_brand_combo_brand_count', 'number', ['number', 'number', 'number']),
            getBrandComboBrandAt:  cwrap('heif_reader_get_compatible_brand_combo_brand_at',    'number', ['number', 'number', 'number', 'number']),

            // Track sample sub-structures
            getTrackSampleItemType:    cwrap('heif_reader_get_track_sample_item_type',          'number', ['number', 'number', 'number', 'number']),
            getTrackEqivCount:         cwrap('heif_reader_get_track_sample_eqiv_count',         'number', ['number', 'number', 'number']),
            getTrackEqivAt:            cwrap('heif_reader_get_track_sample_eqiv_at',            'number', ['number', 'number', 'number', 'number']),
            getTrackStmiCount:         cwrap('heif_reader_get_track_sample_stmi_count',         'number', ['number', 'number', 'number']),
            getTrackStmiAt:            cwrap('heif_reader_get_track_sample_stmi_at',            'number', ['number', 'number', 'number', 'number']),
            getTrackStmiMetaCount:     cwrap('heif_reader_get_track_sample_stmi_metadata_item_count', 'number', ['number', 'number', 'number', 'number']),
            getTrackStmiMetaAt:        cwrap('heif_reader_get_track_sample_stmi_metadata_item_at',    'number', ['number', 'number', 'number', 'number', 'number']),
            getTrackRefsCount:         cwrap('heif_reader_get_track_sample_refs_count',                'number', ['number', 'number', 'number']),
            getTrackRefsAt:            cwrap('heif_reader_get_track_sample_refs_at',                   'number', ['number', 'number', 'number', 'number']),
            getTrackRefsRefItemCount:  cwrap('heif_reader_get_track_sample_refs_reference_item_count', 'number', ['number', 'number', 'number', 'number']),
            getTrackRefsRefItemAt:     cwrap('heif_reader_get_track_sample_refs_reference_item_at',    'number', ['number', 'number', 'number', 'number', 'number']),
            getTrackTrifCount:         cwrap('heif_reader_get_track_sample_trif_count',                'number', ['number', 'number', 'number']),
            getTrackTrifAt:            cwrap('heif_reader_get_track_sample_trif_at',                   'number', ['number', 'number', 'number', 'number']),
            getTrackTrifDepCount:      cwrap('heif_reader_get_track_sample_trif_dependency_count',     'number', ['number', 'number', 'number', 'number']),
            getTrackTrifDepAt:         cwrap('heif_reader_get_track_sample_trif_dependency_at',        'number', ['number', 'number', 'number', 'number', 'number']),
            getTrackSporCount:         cwrap('heif_reader_get_track_sample_spor_count',                'number', ['number', 'number', 'number']),
            getTrackSporAt:            cwrap('heif_reader_get_track_sample_spor_at',                   'number', ['number', 'number', 'number', 'number']),
            getTrackSporSubpCount:     cwrap('heif_reader_get_track_sample_spor_subp_track_ref_idx_count', 'number', ['number', 'number', 'number', 'number']),
            getTrackSporSubpAt:        cwrap('heif_reader_get_track_sample_spor_subp_track_ref_idx_at',    'number', ['number', 'number', 'number', 'number', 'number']),
            getTrackSulmCount:         cwrap('heif_reader_get_track_sample_sulm_count',                'number', ['number', 'number', 'number']),
            getTrackSulmAt:            cwrap('heif_reader_get_track_sample_sulm_at',                   'number', ['number', 'number', 'number', 'number']),
            getTrackSulmGidCount:      cwrap('heif_reader_get_track_sample_sulm_group_id_count',       'number', ['number', 'number', 'number', 'number']),
            getTrackSulmGidAt:         cwrap('heif_reader_get_track_sample_sulm_group_id_at',          'number', ['number', 'number', 'number', 'number', 'number']),
            getTrackMinpCount:         cwrap('heif_reader_get_track_sample_minp_count',                'number', ['number', 'number', 'number']),
            getTrackMinpAt:            cwrap('heif_reader_get_track_sample_minp_at',                   'number', ['number', 'number', 'number', 'number']),
            getTrackMinpIdx1Count:     cwrap('heif_reader_get_track_sample_minp_mix_subp_track_idx1_count', 'number', ['number', 'number', 'number', 'number']),
            getTrackMinpIdx1At:        cwrap('heif_reader_get_track_sample_minp_mix_subp_track_idx1_at',    'number', ['number', 'number', 'number', 'number', 'number']),
            getTrackMinpIdx2Count:     cwrap('heif_reader_get_track_sample_minp_mix_subp_track_idx2_count', 'number', ['number', 'number', 'number', 'number']),
            getTrackMinpIdx2At:        cwrap('heif_reader_get_track_sample_minp_mix_subp_track_idx2_at',    'number', ['number', 'number', 'number', 'number', 'number']),

            // DASH / segmented parsing
            initFile:              cwrap('heif_reader_initialize_file',              'number', ['number', 'string']),
            streamCreate:          cwrap('heif_stream_create',                       'number', ['number']),
            streamDestroy:         cwrap('heif_stream_destroy',                      null,     ['number']),
            parseInitSeg:          cwrap('heif_reader_parse_initialization_segment', 'number', ['number', 'number']),
            parseSegment:          cwrap('heif_reader_parse_segment',                'number', ['number', 'number', 'number', 'bigint']),
            invalidateSegment:     cwrap('heif_reader_invalidate_segment',           'number', ['number', 'number']),
            parseSegmentIndex:     cwrap('heif_reader_parse_segment_index',          'number', ['number', 'number', 'number', 'number']),

            // Additional error / version helpers
            hasLastError:          cwrap('heif_has_last_error',              'number', []),
            errorCause:            cwrap('heif_get_last_error_cause',        'number', []),
            errorFile:             cwrap('heif_get_last_error_file',         'number', []),
            errCodeNameStatic:     cwrap('heif_error_code_name',            'number', ['number']),
            errOriginNameStatic:   cwrap('heif_error_origin_name',          'number', ['number']),
            runtimeVersionTriple:  cwrap('heif_get_runtime_version',        null,     ['number', 'number', 'number']),
            runtimeVersionNumber:  cwrap('heif_get_runtime_version_number', 'number', []),
            abiHandshake:          cwrap('heif_get_abi_handshake',          'number', ['number']),
        };
    }

    HEIF.prototype.runtimeVersion = function () {
        return this._m.UTF8ToString(this._fn.runtimeVersion());
    };

    /** Runtime version as {major, minor, patch}. */
    HEIF.prototype.runtimeVersionTriple = function () {
        var m = this._m;
        var a = m._malloc(4), b = m._malloc(4), c = m._malloc(4);
        try {
            this._fn.runtimeVersionTriple(a, b, c);
            return {
                major: m.HEAPU32[a >> 2],
                minor: m.HEAPU32[b >> 2],
                patch: m.HEAPU32[c >> 2],
            };
        } finally {
            m._free(a); m._free(b); m._free(c);
        }
    };

    /** Runtime version as a single uint32 (major*10000 + minor*100 + patch). */
    HEIF.prototype.runtimeVersionNumber = function () {
        return this._fn.runtimeVersionNumber();
    };

    /** ABI handshake: returns {major, minor, patch, flags}. */
    HEIF.prototype.abiHandshake = function () {
        var m = this._m;
        var structSize = 20;
        var ptr = m._malloc(structSize);
        try {
            m.HEAPU32[ptr >> 2] = structSize;
            var rc = this._fn.abiHandshake(ptr);
            if (rc !== 0) {
                throw new Error('heif_get_abi_handshake failed (' + rc + ')');
            }
            return {
                major: m.HEAPU32[(ptr >> 2) + 1],
                minor: m.HEAPU32[(ptr >> 2) + 2],
                patch: m.HEAPU32[(ptr >> 2) + 3],
                flags: m.HEAPU32[(ptr >> 2) + 4],
            };
        } finally {
            m._free(ptr);
        }
    };

    /** Static error code name lookup (no reader needed). */
    HEIF.prototype.errorCodeName = function (code) {
        return this._m.UTF8ToString(this._fn.errCodeNameStatic(code));
    };

    /** Static error origin name lookup (no reader needed). */
    HEIF.prototype.errorOriginName = function (origin) {
        return this._m.UTF8ToString(this._fn.errOriginNameStatic(origin));
    };

    /** True if the per-thread last-error slot is populated. */
    HEIF.prototype.hasLastError = function () {
        return this._fn.hasLastError() !== 0;
    };

    /** Chained cause message from the last error (if any). */
    HEIF.prototype.lastErrorCause = function () {
        var ptr = this._fn.errorCause();
        return ptr ? this._m.UTF8ToString(ptr) : '';
    };

    /** Source file path from the last error (library internal). */
    HEIF.prototype.lastErrorFile = function () {
        var ptr = this._fn.errorFile();
        return ptr ? this._m.UTF8ToString(ptr) : '';
    };

    HEIF.prototype.open = function (bytes) {
        if (!(bytes instanceof Uint8Array)) {
            throw new TypeError('heif.open(): expected a Uint8Array');
        }
        const reader = new HEIFReader(this);
        try {
            reader._initFromBytes(bytes);
        } catch (e) {
            reader.close();
            throw e;
        }
        return reader;
    };

    // Throws HEIFError if `code` is non-zero. Drains the per-thread
    // last-error state so the caller gets line/origin/message info.
    HEIF.prototype._check = function (code, what) {
        if (code === 0) return;
        const fn = this._fn;
        const m  = this._m;

        const codeName   = m.UTF8ToString(fn.errorCodeName());
        const originName = m.UTF8ToString(fn.errorOriginName());
        const line       = fn.errorLine();

        // heif_get_last_error_message uses the size-probe convention:
        // call with (NULL, 0) to learn the required buffer size.
        const needed = fn.errorMessage(0, 0);
        let message = '';
        if (needed > 0) {
            const buf = m._malloc(needed);
            try {
                fn.errorMessage(buf, needed);
                message = m.UTF8ToString(buf);
            } finally {
                m._free(buf);
            }
        }
        fn.clearError();
        throw new HEIFError(code, codeName, originName, line, message, what);
    };

    // HEIFReader: one open file
    function HEIFReader(heif) {
        this._heif = heif;
        this._handle = heif._fn.readerCreate();
        if (this._handle === 0) {
            throw new Error('heif: heif_reader_create returned NULL');
        }
        // Streaming-state pointers, populated by _initFromBytes.
        // Kept on the instance so close() can release them.
        this._userDataPtr   = 0;       // void* user_data the library holds
        this._callbackPtrs  = [];      // function-table indices to removeFunction on close
        this._stagedBytesPtr = 0;      // wasm-heap copy of the file bytes
    }

    // Shared helper: stage bytes into the wasm heap, build the four
    // stream callbacks (read/seek/tell/size) via addFunction, and pack
    // a heif_stream_callbacks_t struct on the heap.  Returns a bag of
    // pointers that the caller hands to either heif_reader_initialize_stream
    // (whole-file path) or heif_stream_create (DASH path).  The caller
    // is responsible for freeing tblPtr after the C call copies it.
    function _buildStreamCallbacks(heif, bytes) {
        var m = heif._m;
        var len = bytes.byteLength;
        var dataPtr = m._malloc(len);
        m.HEAPU8.set(bytes, dataPtr);

        // user_data: [base, size, cursor, reserved]  (16 bytes)
        var ud = m._malloc(16);
        m.HEAPU32[(ud >> 2) + 0] = dataPtr;
        m.HEAPU32[(ud >> 2) + 1] = len;
        m.HEAPU32[(ud >> 2) + 2] = 0;
        m.HEAPU32[(ud >> 2) + 3] = 0;

        // i64 mode detection -- Emscripten delivers BigInt under wasm,
        // plain Number under asm.js.  Sniff on first inbound call.
        var fromI64 = function (v) { return (typeof v === 'bigint') ? Number(v) : (v | 0); };
        var _mode = null;
        var sniff = function (v) { if (_mode === null) _mode = (typeof v === 'bigint') ? 'bigint' : 'number'; };
        var toI64 = function (n) { return (_mode === 'bigint') ? BigInt(n) : n; };

        // Every callback re-reads m.HEAPU32 on entry because
        // Emscripten replaces the typed-array view on heap growth.
        var readCb = m.addFunction(function (userData, buffer, size) {
            sniff(size);
            var sz = fromI64(size);
            var H = m.HEAPU32;
            var base = H[(userData >> 2) + 0], total = H[(userData >> 2) + 1], cursor = H[(userData >> 2) + 2];
            var n = Math.min(sz, total - cursor);
            if (n > 0) { m.HEAPU8.set(m.HEAPU8.subarray(base + cursor, base + cursor + n), buffer); H[(userData >> 2) + 2] = cursor + n; }
            return toI64(n);
        }, 'jiij');
        var seekCb = m.addFunction(function (userData, offset) {
            sniff(offset);
            var off = fromI64(offset), H = m.HEAPU32, total = H[(userData >> 2) + 1];
            // Seeking past EOF is allowed per HEIF::StreamInterface; clamp.
            H[(userData >> 2) + 2] = (off < 0) ? 0 : (off > total ? total : off);
            return 1;
        }, 'iij');
        var tellCb = m.addFunction(function (userData) {
            return toI64(m.HEAPU32[(userData >> 2) + 2]);
        }, 'ji');
        var sizeCb = m.addFunction(function (userData) {
            return toI64(m.HEAPU32[(userData >> 2) + 1]);
        }, 'ji');

        var cbPtrs = [readCb, seekCb, tellCb, sizeCb];

        // Pack heif_stream_callbacks_t on the heap (24 bytes on wasm32).
        var tblSize = 4 + 5 * 4;
        var tbl = m._malloc(tblSize);
        m.HEAPU32[(tbl >> 2) + 0] = tblSize;
        m.HEAPU32[(tbl >> 2) + 1] = readCb;
        m.HEAPU32[(tbl >> 2) + 2] = seekCb;
        m.HEAPU32[(tbl >> 2) + 3] = tellCb;
        m.HEAPU32[(tbl >> 2) + 4] = sizeCb;
        m.HEAPU32[(tbl >> 2) + 5] = ud;

        return { tblPtr: tbl, callbackPtrs: cbPtrs, userDataPtr: ud, stagedBytesPtr: dataPtr };
    }

    // Cleanup helper for a _buildStreamCallbacks bag when neither
    // heif_reader_initialize_stream nor heif_stream_create succeeded.
    function _cleanupCallbacks(heif, bag) {
        var m = heif._m;
        for (var i = 0; i < bag.callbackPtrs.length; i++) m.removeFunction(bag.callbackPtrs[i]);
        m._free(bag.userDataPtr);
        m._free(bag.stagedBytesPtr);
    }

    HEIFReader.prototype._initFromBytes = function (bytes) {
        var bag = _buildStreamCallbacks(this._heif, bytes);
        try {
            this._heif._check(
                this._heif._fn.readerInitStream(this._handle, bag.tblPtr),
                'heif_reader_initialize_stream');
        } catch (e) {
            _cleanupCallbacks(this._heif, bag);
            throw e;
        } finally {
            // The callback table is copied during initialize_stream;
            // safe to free immediately.
            this._heif._m._free(bag.tblPtr);
        }
        this._callbackPtrs   = bag.callbackPtrs;
        this._userDataPtr    = bag.userDataPtr;
        this._stagedBytesPtr = bag.stagedBytesPtr;
    };

    HEIFReader.prototype.close = function () {
        if (!this._heif) return;
        const m = this._heif._m;
        if (this._handle !== 0) {
            this._heif._fn.readerDestroy(this._handle);
            this._handle = 0;
        }
        for (let i = 0; i < this._callbackPtrs.length; i++) {
            m.removeFunction(this._callbackPtrs[i]);
        }
        this._callbackPtrs = [];
        if (this._dashStreams) {
            for (let i = 0; i < this._dashStreams.length; i++) {
                var entry = this._dashStreams[i];
                var s = entry.stream || entry;
                _destroyStream(this._heif, s);
            }
            this._dashStreams = [];
        }
        if (this._userDataPtr) { m._free(this._userDataPtr); this._userDataPtr = 0; }
        if (this._stagedBytesPtr) { m._free(this._stagedBytesPtr); this._stagedBytesPtr = 0; }
        this._heif = null;
    };

    // File-level identity getters

    function fourCC(m, fn, handle) {
        const buf = m._malloc(5);
        try {
            const code = fn(handle, buf);
            if (code !== 0) return null;
            return m.UTF8ToString(buf);
        } finally {
            m._free(buf);
        }
    }

    Object.defineProperty(HEIFReader.prototype, 'majorBrand', {
        get: function () {
            return fourCC(this._heif._m, this._heif._fn.getMajorBrand, this._handle);
        }
    });

    Object.defineProperty(HEIFReader.prototype, 'minorVersion', {
        get: function () {
            const m = this._heif._m;
            const out = m._malloc(4);
            try {
                this._heif._check(
                    this._heif._fn.getMinorVersion(this._handle, out),
                    'heif_reader_get_minor_version'
                );
                return m.HEAPU32[out >> 2];
            } finally {
                m._free(out);
            }
        }
    });

    Object.defineProperty(HEIFReader.prototype, 'compatibleBrands', {
        get: function () {
            const m = this._heif._m;
            const fn = this._heif._fn;
            const out = m._malloc(4);
            let count;
            try {
                this._heif._check(
                    fn.getCompatBrandCount(this._handle, out),
                    'heif_reader_get_compatible_brand_count'
                );
                count = m.HEAPU32[out >> 2];
            } finally {
                m._free(out);
            }
            const result = new Array(count);
            const buf = m._malloc(5);
            try {
                for (let i = 0; i < count; i++) {
                    this._heif._check(
                        fn.getCompatBrandAt(this._handle, i, buf),
                        'heif_reader_get_compatible_brand_at'
                    );
                    result[i] = m.UTF8ToString(buf);
                }
            } finally {
                m._free(buf);
            }
            return result;
        }
    });

    Object.defineProperty(HEIFReader.prototype, 'primaryItemId', {
        get: function () {
            const m = this._heif._m;
            const out = m._malloc(4);
            try {
                this._heif._check(
                    this._heif._fn.getPrimaryItem(this._handle, out),
                    'heif_reader_get_primary_item'
                );
                return m.HEAPU32[out >> 2];
            } finally {
                m._free(out);
            }
        }
    });

    // File / metabox features

    /** File-level feature bitmask. Bits map to
     *  HEIF::FileFeatureEnum::Feature. */
    HEIFReader.prototype.fileFeatures = function () {
        const m = this._heif._m;
        const out = m._malloc(4);
        try {
            this._heif._check(this._heif._fn.getFileFeatures(this._handle, out),
                              'heif_reader_get_file_features');
            return m.HEAPU32[out >> 2];
        } finally {
            m._free(out);
        }
    };

    /** Metabox-level feature bitmask. Bits map to
     *  HEIF::MetaBoxFeatureEnum::Feature. */
    HEIFReader.prototype.metaboxFeatures = function () {
        const m = this._heif._m;
        const out = m._malloc(4);
        try {
            this._heif._check(this._heif._fn.getMetaboxFeatures(this._handle, out),
                              'heif_reader_get_metabox_features');
            return m.HEAPU32[out >> 2];
        } finally {
            m._free(out);
        }
    };

    /** Item protection scheme bytes (size-probe). */
    HEIFReader.prototype.itemProtectionScheme = function (itemId) {
        return fetchSized(this._heif, this._heif._fn.getProtectionScheme,
                          [this._handle, itemId],
                          'heif_reader_get_item_protection_scheme');
    };

    /** Properties associated with an entity group (by group_id).
     *  Returns [{propertyId, type, essential}]. */
    HEIFReader.prototype.groupProperties = function (groupId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let n;
        try {
            this._heif._check(fn.getGroupPropCount(this._handle, groupId, cnt),
                              'heif_reader_get_group_property_count');
            n = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const pid = m._malloc(4);
        const buf = m._malloc(5);
        const ess = m._malloc(4);
        const result = new Array(n);
        try {
            for (var i = 0; i < n; i++) {
                this._heif._check(
                    fn.getGroupPropAt(this._handle, groupId, i, pid, buf, ess),
                    'heif_reader_get_group_property_at');
                result[i] = {
                    propertyId: m.HEAPU32[pid >> 2],
                    type:       m.UTF8ToString(buf),
                    essential:  m.HEAPU32[ess >> 2] !== 0,
                };
            }
        } finally {
            m._free(pid); m._free(buf); m._free(ess);
        }
        return result;
    };

    /** File-level 3x3 transformation matrix (9 int32 values from
     *  the movie-header box). */
    HEIFReader.prototype.matrix = function () {
        const m = this._heif._m;
        const out = m._malloc(4);
        const result = new Int32Array(9);
        try {
            for (var i = 0; i < 9; i++) {
                this._heif._check(this._heif._fn.getMatrixAt(this._handle, i, out),
                                  'heif_reader_get_matrix_at');
                result[i] = m.HEAP32[out >> 2];
            }
        } finally {
            m._free(out);
        }
        return result;
    };

    /** Compatible brand combinations -- array of arrays of FourCC
     *  strings.  Each inner array is one combination. */
    HEIFReader.prototype.compatibleBrandCombos = function () {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let nCombos;
        try {
            this._heif._check(fn.getBrandComboCount(this._handle, cnt),
                              'heif_reader_get_compatible_brand_combo_count');
            nCombos = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const result = new Array(nCombos);
        const inner = m._malloc(4);
        const buf = m._malloc(5);
        try {
            for (var ci = 0; ci < nCombos; ci++) {
                this._heif._check(fn.getBrandComboBrandCnt(this._handle, ci, inner),
                                  'heif_reader_get_compatible_brand_combo_brand_count');
                var nBrands = m.HEAPU32[inner >> 2];
                result[ci] = new Array(nBrands);
                for (var bi = 0; bi < nBrands; bi++) {
                    this._heif._check(fn.getBrandComboBrandAt(this._handle, ci, bi, buf),
                                      'heif_reader_get_compatible_brand_combo_brand_at');
                    result[ci][bi] = m.UTF8ToString(buf);
                }
            }
        } finally {
            m._free(inner); m._free(buf);
        }
        return result;
    };

    // Item enumeration

    // Generic helper for the (count, at) pair pattern used by every
    // collection accessor on the reader.  The library guarantees an
    // O(n) total walk via its single-entry cache (see heifreader_c.h
    // doc on getItemListByType), so a straight forward for-loop is the
    // canonical idiom.
    function collectU32(heif, countFn, atFn /*, ...keys*/) {
        const m = heif._m;
        const args = Array.prototype.slice.call(arguments, 3);
        const out = m._malloc(4);
        let count;
        try {
            heif._check(countFn.apply(null, args.concat(out)), 'count');
            count = m.HEAPU32[out >> 2];
            const result = new Uint32Array(count);
            for (let i = 0; i < count; i++) {
                heif._check(atFn.apply(null, args.concat([i, out])), 'at');
                result[i] = m.HEAPU32[out >> 2];
            }
            return result;
        } finally {
            m._free(out);
        }
    }

    /** All master image item ids (excludes thumbnails, hidden items,
     *  iden/grid/iovl derivations etc.).  Mirrors getMasterImages(). */
    HEIFReader.prototype.masterImages = function () {
        const fn = this._heif._fn;
        return collectU32(this._heif, fn.getMasterCount, fn.getMasterAt, this._handle);
    };

    /** All item ids matching a 4cc item type.  Common values:
     *  "hvc1", "avc1", "vvc1", "jpeg", "grid", "iovl", "iden",
     *  "tmap", "Exif", "mime".  Mirrors getItemListByType(). */
    HEIFReader.prototype.itemsByType = function (fourCC) {
        if (typeof fourCC !== 'string' || fourCC.length < 4) {
            throw new TypeError('itemsByType: fourCC must be a string of length >= 4');
        }
        const fn = this._heif._fn;
        return collectU32(this._heif, fn.getByTypeCount, fn.getByTypeAt,
                          this._handle, fourCC);
    };

    /** Items that itemId references via a given 4cc reference type
     *  ("thmb", "cdsc", "auxl", "dimg", "base").  Mirrors
     *  getReferencedFromItemListByType(). */
    HEIFReader.prototype.referencedFrom = function (itemId, refType) {
        const fn = this._heif._fn;
        return collectU32(this._heif, fn.getRefFromCount, fn.getRefFromAt,
                          this._handle, itemId, refType);
    };

    /** Items that reference itemId via a given 4cc reference type.
     *  Useful for "given this thumbnail, what is its parent?".
     *  Mirrors getReferencedToItemListByType(). */
    HEIFReader.prototype.referencedTo = function (itemId, refType) {
        const fn = this._heif._fn;
        return collectU32(this._heif, fn.getRefToCount, fn.getRefToAt,
                          this._handle, itemId, refType);
    };

    // Per-image accessors

    /** 4cc item type (e.g. "hvc1", "grid"). */
    HEIFReader.prototype.itemType = function (itemId) {
        const m = this._heif._m;
        const buf = m._malloc(5);
        try {
            this._heif._check(
                this._heif._fn.getItemType(this._handle, itemId, buf),
                'heif_reader_get_item_type'
            );
            return m.UTF8ToString(buf);
        } finally {
            m._free(buf);
        }
    };

    /** Per-item feature bitmask (HEIF::ItemFeatureEnum::Feature).
     *  Returns the raw uint32 bitmask. */
    HEIFReader.prototype.itemFeatures = function (itemId) {
        const fn = this._heif._fn;
        return readU32At(this._heif, fn.getItemFeatures, this._handle, itemId,
                         'heif_reader_get_item_features');
    };

    // ItemFeatureEnum bit positions (from heifreaderdatatypes.h)
    var ITEM_FEATURE_IS_HIDDEN = 1 << 6;
    var ITEM_FEATURE_IS_PREDICTIVELY_CODED = 1 << 21;

    /** Returns true if the item has the IsHiddenImage flag set. */
    HEIFReader.prototype.itemIsHidden = function (itemId) {
        return (this.itemFeatures(itemId) & ITEM_FEATURE_IS_HIDDEN) !== 0;
    };

    function readU32At(heif, fn, handle, itemId, what) {
        const m = heif._m;
        const out = m._malloc(4);
        try {
            heif._check(fn(handle, itemId, out), what);
            return m.HEAPU32[out >> 2];
        } finally {
            m._free(out);
        }
    }

    HEIFReader.prototype.itemWidth = function (itemId) {
        const fn = this._heif._fn;
        return readU32At(this._heif, fn.getWidth, this._handle, itemId,
                         'heif_reader_get_width');
    };

    HEIFReader.prototype.itemHeight = function (itemId) {
        const fn = this._heif._fn;
        return readU32At(this._heif, fn.getHeight, this._handle, itemId,
                         'heif_reader_get_height');
    };

    /** {width, height} convenience wrapper. */
    HEIFReader.prototype.itemSize = function (itemId) {
        return {
            width:  this.itemWidth(itemId),
            height: this.itemHeight(itemId),
        };
    };

    // Bytestream extraction

    // Mirrors HEIF::ErrorCode entries that signal "buffer too small,
    // here is the required size". Two synonyms exist in the enum:
    //   BUFFER_SIZE_TOO_SMALL  = 4   (item-side and most size-probe calls)
    //   MEMORY_TOO_SMALL_BUFFER = 26 (track-sample raw variant)
    // Treat both as the probe-not-enough signal.
    const ERR_BUFFER_SIZE_TOO_SMALL  = 4;
    const ERR_MEMORY_TOO_SMALL_BUFFER = 26;
    function isProbeShort(code) {
        return code === ERR_BUFFER_SIZE_TOO_SMALL ||
               code === ERR_MEMORY_TOO_SMALL_BUFFER;
    }

    // Implements the size-probe pattern used by every variable-
    // length getter on the C facade:
    //   1. call with (NULL, &size=0) -> BUFFER_SIZE_TOO_SMALL + size set
    //   2. allocate
    //   3. call with (buf, &size=cap) -> OK + size set to bytes written
    function fetchSized(heif, fn, callArgs, what) {
        const m = heif._m;
        // size_inout is uint64_t. Under -sWASM=0 this is two consecutive
        // i32 words in the heap (low, high); we only ever care about the
        // low word for in-memory items (< 4 GiB).
        const sizePtr = m._malloc(8);
        m.HEAPU32[(sizePtr >> 2) + 0] = 0;
        m.HEAPU32[(sizePtr >> 2) + 1] = 0;
        try {
            // Probe.
            let code = fn.apply(null, callArgs.concat([0, sizePtr]));
            if (code !== 0 && !isProbeShort(code)) {
                heif._check(code, what + ' (probe)');
            }
            const needed = m.HEAPU32[(sizePtr >> 2) + 0];
            if (needed === 0) {
                return new Uint8Array(0);
            }
            // Fetch.
            const bufPtr = m._malloc(needed);
            try {
                m.HEAPU32[(sizePtr >> 2) + 0] = needed;
                m.HEAPU32[(sizePtr >> 2) + 1] = 0;
                code = fn.apply(null, callArgs.concat([bufPtr, sizePtr]));
                heif._check(code, what + ' (fetch)');
                const written = m.HEAPU32[(sizePtr >> 2) + 0];
                // Copy out of the wasm heap so the caller owns a stable
                // Uint8Array independent of any later HEAPU8 invalidation
                // (memory growth detaches the underlying ArrayBuffer view).
                return new Uint8Array(m.HEAPU8.buffer, bufPtr, written).slice();
            } finally {
                m._free(bufPtr);
            }
        } finally {
            m._free(sizePtr);
        }
    }

    /** Decoder-ready bitstream for an encoded image item.
     *  For hvc1/avc1/vvc1 this includes the codec config record(s)
     *  followed by the item data with NAL bytestream headers
     *  (0x000001) substituted for length prefixes. Hand the result
     *  straight to libde265/vvdec.
     *
     *  Mirrors HEIF::Reader::getItemDataWithDecoderParameters().  */
    HEIFReader.prototype.itemBitstream = function (itemId) {
        const fn = this._heif._fn;
        return fetchSized(this._heif, fn.getItemDataDP, [this._handle, itemId],
                          'heif_reader_get_item_data_with_decoder_parameters');
    };

    /** Raw item bytes -- the on-wire payload as stored in the file.
     *  Use for non-image items (Exif, XMP/'mime', grid/iovl
     *  derivation records). For coded image items prefer
     *  itemBitstream() unless you really want the headerless data.
     *
     *  Mirrors HEIF::Reader::getItemData().  */
    HEIFReader.prototype.itemRawData = function (itemId) {
        const fn = this._heif._fn;
        return fetchSized(this._heif, fn.getItemData, [this._handle, itemId],
                          'heif_reader_get_item_data');
    };

    /** Decoder code type FourCC for an encoded image item (e.g.
     *  "hvc1", "hev1", "vvc1", "avc1").  Tells you which decoder
     *  to feed the bitstream to. */
    HEIFReader.prototype.decoderCodeType = function (itemId) {
        const m = this._heif._m;
        const buf = m._malloc(5);
        try {
            this._heif._check(
                this._heif._fn.getDecoderCodeType(this._handle, itemId, buf),
                'heif_reader_get_decoder_code_type');
            return m.UTF8ToString(buf);
        } finally {
            m._free(buf);
        }
    };

    /** Item ids that must be decoded before itemId (for seek/scrub
     *  in predictively-coded items).  Returns Uint32Array. */
    HEIFReader.prototype.decodeDependencies = function (itemId) {
        const fn = this._heif._fn;
        return collectU32(this._heif, fn.getDecDepCount, fn.getDecDepAt,
                          this._handle, itemId);
    };

    // Item properties (irot / imir / ispe / pixi / colr)

    /** All properties associated with itemId, as an array of
     *  {id, type, essential} triples. The `id` is opaque -- feed it
     *  into propertyIspe(id), propertyIrot(id) etc. The `type` is
     *  the 4cc you switch on; `essential` is true iff the file
     *  marks the property as required for correct rendering. */
    HEIFReader.prototype.itemProperties = function (itemId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let count;
        try {
            this._heif._check(fn.getPropCount(this._handle, itemId, cnt),
                              'heif_reader_get_item_property_count');
            count = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const result = new Array(count);
        const pid = m._malloc(4);
        const buf = m._malloc(5);
        const ess = m._malloc(4);
        try {
            for (let i = 0; i < count; i++) {
                this._heif._check(
                    fn.getPropAt(this._handle, itemId, i, pid, buf, ess),
                    'heif_reader_get_item_property_at'
                );
                result[i] = {
                    id:        m.HEAPU32[pid >> 2],
                    type:      m.UTF8ToString(buf),
                    essential: m.HEAPU32[ess >> 2] !== 0,
                };
            }
        } finally {
            m._free(pid); m._free(buf); m._free(ess);
        }
        return result;
    };

    /** ispe -- encoded extents (pre-irot/imir). {width, height}. */
    HEIFReader.prototype.propertyIspe = function (propertyId) {
        const m = this._heif._m;
        const w = m._malloc(4), h = m._malloc(4);
        try {
            this._heif._check(this._heif._fn.getPropIspe(this._handle, propertyId, w, h),
                              'heif_reader_get_property_ispe');
            return { width: m.HEAPU32[w >> 2], height: m.HEAPU32[h >> 2] };
        } finally {
            m._free(w); m._free(h);
        }
    };

    /** irot -- counter-clockwise rotation in degrees (0/90/180/270). */
    HEIFReader.prototype.propertyIrot = function (propertyId) {
        const m = this._heif._m;
        const out = m._malloc(4);
        try {
            this._heif._check(this._heif._fn.getPropIrot(this._handle, propertyId, out),
                              'heif_reader_get_property_irot');
            return m.HEAPU32[out >> 2];
        } finally {
            m._free(out);
        }
    };

    /** imir -- {axis: 'horizontal' | 'vertical'}. The horizontal axis
     *  flag means top<->bottom flip; vertical means left<->right. */
    HEIFReader.prototype.propertyImir = function (propertyId) {
        const m = this._heif._m;
        const out = m._malloc(4);
        try {
            this._heif._check(this._heif._fn.getPropImir(this._handle, propertyId, out),
                              'heif_reader_get_property_imir');
            return { axis: (m.HEAPU32[out >> 2] !== 0) ? 'horizontal' : 'vertical' };
        } finally {
            m._free(out);
        }
    };

    /** pixi -- array of bits-per-channel (typically [8,8,8] or [10,10,10]). */
    /** clap -- clean aperture (rational crop box). */
    HEIFReader.prototype.propertyClap = function (propertyId) {
        const m = this._heif._m;
        const p = m._malloc(36);
        try {
            m.HEAPU32[p >> 2] = 36;  // struct_size
            this._heif._check(this._heif._fn.getPropClap(this._handle, propertyId, p),
                              'heif_reader_get_property_clap');
            return readClap(m, p);
        } finally {
            m._free(p);
        }
    };

    /** iscl -- image scaling (rational target width/height). */
    HEIFReader.prototype.propertyIscl = function (propertyId) {
        const m = this._heif._m;
        const wn = m._malloc(2), wd = m._malloc(2), hn = m._malloc(2), hd = m._malloc(2);
        try {
            this._heif._check(
                this._heif._fn.getPropIscl(this._handle, propertyId, wn, wd, hn, hd),
                'heif_reader_get_property_iscl');
            return {
                targetWidthN:  m.HEAPU16[wn >> 1],
                targetWidthD:  m.HEAPU16[wd >> 1],
                targetHeightN: m.HEAPU16[hn >> 1],
                targetHeightD: m.HEAPU16[hd >> 1],
            };
        } finally {
            m._free(wn); m._free(wd); m._free(hn); m._free(hd);
        }
    };

    /** lsel -- layer selector (single uint16 layer_id). */
    HEIFReader.prototype.propertyLsel = function (propertyId) {
        const m = this._heif._m;
        const out = m._malloc(2);
        try {
            this._heif._check(this._heif._fn.getPropLsel(this._handle, propertyId, out),
                              'heif_reader_get_property_lsel');
            return { layerId: m.HEAPU16[out >> 1] };
        } finally {
            m._free(out);
        }
    };

    /** tols -- target output layer set (single uint16 target_ols_index). */
    HEIFReader.prototype.propertyTols = function (propertyId) {
        const m = this._heif._m;
        const out = m._malloc(2);
        try {
            this._heif._check(this._heif._fn.getPropTols(this._handle, propertyId, out),
                              'heif_reader_get_property_tols');
            return { targetOlsIndex: m.HEAPU16[out >> 1] };
        } finally {
            m._free(out);
        }
    };

    /** rloc -- relative location (horizontal/vertical pixel offset). */
    HEIFReader.prototype.propertyRloc = function (propertyId) {
        const m = this._heif._m;
        const hOut = m._malloc(4);
        const vOut = m._malloc(4);
        try {
            this._heif._check(this._heif._fn.getPropRloc(this._handle, propertyId, hOut, vOut),
                              'heif_reader_get_property_rloc');
            return {
                horizontalOffset: m.HEAPU32[hOut >> 2],
                verticalOffset:   m.HEAPU32[vOut >> 2],
            };
        } finally {
            m._free(hOut); m._free(vOut);
        }
    };

    /** oinf -- Operating-Points-Info table (multi-layer L-HEVC).
     *  Returns {scalabilityMask, ptl[], operatingPoints[], layers[]}.
     *  Each operating point has nested layer entries; each layer has
     *  dependency and dimension ID arrays. */
    HEIFReader.prototype.propertyOinf = function (propertyId) {
        const m  = this._heif._m;
        const fn = this._heif._fn;
        const u16 = m._malloc(2);
        const u32 = m._malloc(4);
        const u8  = m._malloc(1);

        try {
            // Scalability mask
            this._heif._check(fn.getPropOinf(this._handle, propertyId, u16),
                              'heif_reader_get_property_oinf');
            const scalabilityMask = m.HEAPU16[u16 >> 1];

            // PTL array -- heif_oinf_ptl_t is 24 bytes
            this._heif._check(fn.getPropOinfPtlCount(this._handle, propertyId, u32),
                              'heif_reader_get_property_oinf_ptl_count');
            const ptlCount = m.HEAPU32[u32 >> 2];
            const ptlBuf = m._malloc(24);
            const ptl = [];
            try {
                for (let i = 0; i < ptlCount; i++) {
                    m.HEAPU32[ptlBuf >> 2] = 24;  // size-prefix: caller writes struct_size
                    this._heif._check(fn.getPropOinfPtlAt(this._handle, propertyId, i, ptlBuf),
                                      'heif_reader_get_property_oinf_ptl_at');
                    ptl.push({
                        generalProfileSpace:              m.HEAPU8[ptlBuf + 4],
                        generalTierFlag:                  m.HEAPU8[ptlBuf + 5],
                        generalProfileIdc:                m.HEAPU8[ptlBuf + 6],
                        generalLevelIdc:                  m.HEAPU8[ptlBuf + 7],
                        generalProfileCompatibilityFlags: m.HEAPU32[(ptlBuf + 8) >> 2],
                        // constraint flags: low 32 + high 32
                        generalConstraintIndicatorFlags:
                            m.HEAPU32[(ptlBuf + 12) >> 2] +
                            m.HEAPU32[(ptlBuf + 16) >> 2] * 0x100000000,
                    });
                }
            } finally { m._free(ptlBuf); }

            // Operating points -- heif_oinf_op_t is 32 bytes
            this._heif._check(fn.getPropOinfOpCount(this._handle, propertyId, u32),
                              'heif_reader_get_property_oinf_op_count');
            const opCount = m.HEAPU32[u32 >> 2];
            const opBuf = m._malloc(32);
            const opLayerBuf = m._malloc(8); // heif_oinf_op_layer_t
            const operatingPoints = [];
            try {
                for (let i = 0; i < opCount; i++) {
                    m.HEAPU32[opBuf >> 2] = 32;  // size-prefix
                    this._heif._check(fn.getPropOinfOpAt(this._handle, propertyId, i, opBuf),
                                      'heif_reader_get_property_oinf_op_at');
                    const op = {
                        maxBitRate:          m.HEAPU32[(opBuf + 4) >> 2],
                        avgBitRate:          m.HEAPU32[(opBuf + 8) >> 2],
                        outputLayerSetIdx:   m.HEAPU16[(opBuf + 12) >> 1],
                        minPicWidth:         m.HEAPU16[(opBuf + 14) >> 1],
                        minPicHeight:        m.HEAPU16[(opBuf + 16) >> 1],
                        maxPicWidth:         m.HEAPU16[(opBuf + 18) >> 1],
                        maxPicHeight:        m.HEAPU16[(opBuf + 20) >> 1],
                        avgFrameRate:        m.HEAPU16[(opBuf + 22) >> 1],
                        maxTemporalId:       m.HEAPU8[opBuf + 24],
                        maxChromaFormat:     m.HEAPU8[opBuf + 25],
                        maxBitDepthMinus8:   m.HEAPU8[opBuf + 26],
                        frameRateInfoFlag:   m.HEAPU8[opBuf + 27],
                        bitRateInfoFlag:     m.HEAPU8[opBuf + 28],
                        constantFrameRate:   m.HEAPU8[opBuf + 29],
                        layers: [],
                    };
                    // Nested layers per operating point
                    this._heif._check(fn.getPropOinfOpLayerCount(this._handle, propertyId, i, u32),
                                      'heif_reader_get_property_oinf_op_layer_count');
                    const layerCount = m.HEAPU32[u32 >> 2];
                    for (let j = 0; j < layerCount; j++) {
                        m.HEAPU32[opLayerBuf >> 2] = 8;  // size-prefix
                        this._heif._check(fn.getPropOinfOpLayerAt(this._handle, propertyId, i, j, opLayerBuf),
                                          'heif_reader_get_property_oinf_op_layer_at');
                        op.layers.push({
                            ptlIdx:                  m.HEAPU8[opLayerBuf + 4],
                            layerId:                 m.HEAPU8[opLayerBuf + 5],
                            isOutputLayer:           m.HEAPU8[opLayerBuf + 6] !== 0,
                            isAlternateOutputLayer:  m.HEAPU8[opLayerBuf + 7] !== 0,
                        });
                    }
                    operatingPoints.push(op);
                }
            } finally { m._free(opBuf); m._free(opLayerBuf); }

            // Layer info array
            this._heif._check(fn.getPropOinfLayerCount(this._handle, propertyId, u32),
                              'heif_reader_get_property_oinf_layer_count');
            const lCount = m.HEAPU32[u32 >> 2];
            const layers = [];
            for (let i = 0; i < lCount; i++) {
                this._heif._check(fn.getPropOinfLayerAt(this._handle, propertyId, i, u8),
                                  'heif_reader_get_property_oinf_layer_at');
                const layer = { dependentLayerId: m.HEAPU8[u8], deps: [], dims: [] };
                // Dependencies
                this._heif._check(fn.getPropOinfLayerDepCount(this._handle, propertyId, i, u32),
                                  'heif_reader_get_property_oinf_layer_dep_count');
                const depCount = m.HEAPU32[u32 >> 2];
                for (let j = 0; j < depCount; j++) {
                    this._heif._check(fn.getPropOinfLayerDepAt(this._handle, propertyId, i, j, u8),
                                      'heif_reader_get_property_oinf_layer_dep_at');
                    layer.deps.push(m.HEAPU8[u8]);
                }
                // Dimension IDs
                this._heif._check(fn.getPropOinfLayerDimCount(this._handle, propertyId, i, u32),
                                  'heif_reader_get_property_oinf_layer_dim_count');
                const dimCount = m.HEAPU32[u32 >> 2];
                for (let j = 0; j < dimCount; j++) {
                    this._heif._check(fn.getPropOinfLayerDimAt(this._handle, propertyId, i, j, u8),
                                      'heif_reader_get_property_oinf_layer_dim_at');
                    layer.dims.push(m.HEAPU8[u8]);
                }
                layers.push(layer);
            }

            return { scalabilityMask, ptl, operatingPoints, layers };
        } finally {
            m._free(u16); m._free(u32); m._free(u8);
        }
    };

    /** Item-level decoder parameter sets (VPS/SPS/PPS/APS).
     *  Returns [{type, data: Uint8Array}, ...]. Type values:
     *  HEVC_VPS=32, HEVC_SPS=33, HEVC_PPS=34, VVC_VPS=14, etc. */
    HEIFReader.prototype.itemDecoderParamSets = function (itemId) {
        return this._readParamSets(itemId, 'getItemPsCount', 'getItemPsTypeAt', 'getItemPsDataAt',
                                   'heif_reader_get_decoder_param_set');
    };

    /** All-layers decoder parameter sets for multi-layer items.
     *  For single-layer items this returns the same as itemDecoderParamSets. */
    HEIFReader.prototype.itemDecoderParamSetsAllLayers = function (itemId) {
        return this._readParamSets(itemId, 'getItemPsAllCount', 'getItemPsAllTypeAt', 'getItemPsAllDataAt',
                                   'heif_reader_get_decoder_param_set_all_layers');
    };

    /** Shared implementation for item/track decoder-param-set reading. */
    HEIFReader.prototype._readParamSets = function (id, countKey, typeKey, dataKey, what) {
        const m  = this._heif._m;
        const fn = this._heif._fn;
        const u32 = m._malloc(4);
        const u8  = m._malloc(1);
        let count;
        try {
            this._heif._check(fn[countKey](this._handle, id, u32), what + '_count');
            count = m.HEAPU32[u32 >> 2];
        } finally {
            m._free(u32);
        }
        const result = [];
        for (let i = 0; i < count; i++) {
            try {
                this._heif._check(fn[typeKey](this._handle, id, i, u8), what + '_type_at');
                const type = m.HEAPU8[u8];
                const data = fetchSized(this._heif, fn[dataKey], [this._handle, id, i],
                                        what + '_data_at');
                result.push({ type, data });
            } finally {}
        }
        m._free(u8);
        return result;
    };

    HEIFReader.prototype.propertyPixi = function (propertyId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let n;
        try {
            this._heif._check(fn.getPropPixiCount(this._handle, propertyId, cnt),
                              'heif_reader_get_property_pixi_channel_count');
            n = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const out = m._malloc(1);
        const result = new Array(n);
        try {
            for (let i = 0; i < n; i++) {
                this._heif._check(fn.getPropPixiBitsAt(this._handle, propertyId, i, out),
                                  'heif_reader_get_property_pixi_bits_at');
                result[i] = m.HEAPU8[out];
            }
        } finally {
            m._free(out);
        }
        return result;
    };

    /** pixi (rich) -- full channel info including channel_idc,
     *  component_format, subsampling, and optional label string.
     *  [{channelIdc, componentFormat, subsamplingFlag, channelLabelFlag,
     *    subsamplingType, subsamplingLocation, label?}]. */
    HEIFReader.prototype.propertyPixiChannels = function (propertyId) {
        var m = this._heif._m;
        var fn = this._heif._fn;
        var numOut = m._malloc(1);
        var n;
        try {
            this._heif._check(fn.getPropPixiNumCh(this._handle, propertyId, numOut),
                              'heif_reader_get_property_pixi_num_channels');
            n = m.HEAPU8[numOut];
        } finally {
            m._free(numOut);
        }
        var S = 12;  // heif_pixi_channel_t = 12 bytes
        var p = m._malloc(S);
        var result = new Array(n);
        try {
            for (var i = 0; i < n; i++) {
                m.HEAPU32[p >> 2] = S;
                this._heif._check(fn.getPropPixiChAt(this._handle, propertyId, i, p),
                                  'heif_reader_get_property_pixi_channel_at');
                var ch = {
                    channelIdc:          m.HEAPU8[p + 4],
                    componentFormat:     m.HEAPU8[p + 5],
                    subsamplingFlag:     m.HEAPU8[p + 6] !== 0,
                    channelLabelFlag:    m.HEAPU8[p + 7] !== 0,
                    subsamplingType:     m.HEAPU8[p + 8],
                    subsamplingLocation: m.HEAPU8[p + 9],
                };
                if (ch.channelLabelFlag) {
                    ch.label = fetchStringField(this._heif, fn.getPropPixiChLabel,
                                                [this._handle, propertyId, i],
                                                'heif_reader_get_property_pixi_channel_label');
                }
                result[i] = ch;
            }
        } finally {
            m._free(p);
        }
        return result;
    };

    /** colr -- {type, ...payload}.  Branch on .type:
     *  - 'nclx': {colourPrimaries, transferCharacteristics, matrixCoefficients, fullRangeFlag}
     *  - 'rICC'/'prof': {iccProfile: Uint8Array} */
    HEIFReader.prototype.propertyColr = function (propertyId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const buf = m._malloc(5);
        let type;
        try {
            this._heif._check(fn.getPropColrType(this._handle, propertyId, buf),
                              'heif_reader_get_property_colr_type');
            type = m.UTF8ToString(buf);
        } finally {
            m._free(buf);
        }
        if (type === 'nclx') {
            const p = m._malloc(2), t = m._malloc(2), x = m._malloc(2), fr = m._malloc(4);
            try {
                this._heif._check(fn.getPropColrNclx(this._handle, propertyId, p, t, x, fr),
                                  'heif_reader_get_property_colr_nclx');
                return {
                    type:                    'nclx',
                    colourPrimaries:         m.HEAPU16[p >> 1],
                    transferCharacteristics: m.HEAPU16[t >> 1],
                    matrixCoefficients:      m.HEAPU16[x >> 1],
                    fullRangeFlag:           m.HEAPU32[fr >> 2] !== 0,
                };
            } finally {
                m._free(p); m._free(t); m._free(x); m._free(fr);
            }
        }
        if (type === 'rICC' || type === 'prof') {
            const icc = fetchSized(this._heif, fn.getPropColrIcc,
                                   [this._handle, propertyId],
                                   'heif_reader_get_property_colr_icc');
            return { type: type, iccProfile: icc };
        }
        return { type: type };
    };

    /** Convenience: itemId -> flattened {ispe, irot, imir, pixi,
     *  colr, raw}.  Walks itemProperties() once and dispatches to
     *  the typed accessors. Properties absent from the file are
     *  absent from the returned object. */
    HEIFReader.prototype.itemPropertiesResolved = function (itemId) {
        const props = this.itemProperties(itemId);
        const out = { raw: props };
        for (let i = 0; i < props.length; i++) {
            const p = props[i];
            switch (p.type) {
                case 'ispe': out.ispe = this.propertyIspe(p.id); break;
                case 'irot': out.irot = this.propertyIrot(p.id); break;
                case 'imir': out.imir = this.propertyImir(p.id); break;
                case 'pixi': out.pixi = this.propertyPixi(p.id); break;
                case 'colr': out.colr = this.propertyColr(p.id); break;
                case 'auxC': out.auxC = this.propertyAuxcType(p.id); break;
                case 'clap': out.clap = this.propertyClap(p.id); break;
                case 'iscl': out.iscl = this.propertyIscl(p.id); break;
                case 'lsel': out.lsel = this.propertyLsel(p.id); break;
                case 'tols': out.tols = this.propertyTols(p.id); break;
                case 'rloc': out.rloc = this.propertyRloc(p.id); break;
                case 'oinf': out.oinf = this.propertyOinf(p.id); break;
                case 'clli': out.clli = this.propertyClli(p.id); break;
                case 'mdcv': out.mdcv = this.propertyMdcv(p.id); break;
                case 'cclv': out.cclv = this.propertyCclv(p.id); break;
                case 'amve': out.amve = this.propertyAmve(p.id); break;
                case 'reve': out.reve = this.propertyReve(p.id); break;
                case 'ndwt': out.ndwt = this.propertyNdwt(p.id); break;
                case 'pasp': out.pasp = this.propertyPasp(p.id); break;
                case 'crtt': out.crtt = this.propertyCrtt(p.id); break;
                case 'mdft': out.mdft = this.propertyMdft(p.id); break;
                case 'udes': out.udes = this.propertyUdes(p.id); break;
                case 'altt': out.altt = this.propertyAltt(p.id); break;
                case 'rref': out.rref = this.propertyRref(p.id); break;
                case 'subs': out.subs = this.propertySubs(p.id); break;
                default:
                    // Unrecognised property -- expose via raw accessor
                    if (!out.unknown) out.unknown = [];
                    try {
                        out.unknown.push(this.propertyRaw(p.id));
                    } catch (_) {
                        out.unknown.push({ type: p.type, data: null });
                    }
                    break;
            }
        }
        return out;
    };

    /** auxC -- auxiliary type URN string (e.g.
     *  "urn:mpeg:hevc:2015:auxid:1" for alpha,
     *  "urn:com:apple:photo:2020:aux:hdrgainmap" for gainmap).
     *  Returns the NUL-terminated type string via size-probe. */
    HEIFReader.prototype.propertyAuxcType = function (propertyId) {
        const fn = this._heif._fn;
        const bytes = fetchSized(this._heif, fn.getPropAuxcType,
                                 [this._handle, propertyId],
                                 'heif_reader_get_property_auxc_type');
        if (!bytes || bytes.byteLength === 0) return '';
        // Strip trailing NUL if present.
        var len = bytes.byteLength;
        if (bytes[len - 1] === 0) len--;
        var s = '';
        for (var i = 0; i < len; i++) s += String.fromCharCode(bytes[i]);
        return s;
    };

    /** auxC subtype -- raw bytes of the auxiliary type info box's
     *  subtype field.  Returns Uint8Array via size-probe.
     *  Most callers want propertyAuxcType() instead (the URN string);
     *  this accessor is for the rare case where the raw subtype
     *  payload carries binary data beyond the NUL terminator. */
    HEIFReader.prototype.propertyAuxcSubtype = function (propertyId) {
        const fn = this._heif._fn;
        return fetchSized(this._heif, fn.getPropAuxcSubtype,
                          [this._handle, propertyId],
                          'heif_reader_get_property_auxc_subtype');
    };

    // HDR metadata properties

    /** clli -- Content Light Level Info.
     *  {maxContentLightLevel, maxPicAverageLightLevel}. */
    HEIFReader.prototype.propertyClli = function (propertyId) {
        const m = this._heif._m;
        const p = m._malloc(8);    // heif_clli_t = 8 bytes
        try {
            m.HEAPU32[p >> 2] = 8;  // struct_size
            this._heif._check(this._heif._fn.getPropClli(this._handle, propertyId, p),
                              'heif_reader_get_property_clli');
            return {
                maxContentLightLevel:    m.HEAPU16[(p >> 1) + 2],
                maxPicAverageLightLevel: m.HEAPU16[(p >> 1) + 3],
            };
        } finally {
            m._free(p);
        }
    };

    /** mdcv -- Mastering Display Colour Volume.
     *  {displayPrimariesX[3], displayPrimariesY[3], whitePointX,
     *   whitePointY, maxDisplayMasteringLuminance,
     *   minDisplayMasteringLuminance}. */
    HEIFReader.prototype.propertyMdcv = function (propertyId) {
        const m = this._heif._m;
        const p = m._malloc(32);   // heif_mdcv_t = 32 bytes
        try {
            m.HEAPU32[p >> 2] = 32;
            this._heif._check(this._heif._fn.getPropMdcv(this._handle, propertyId, p),
                              'heif_reader_get_property_mdcv');
            // Layout: u32 struct_size | u16[3] dpx | u16[3] dpy | u16 wpx | u16 wpy | u8[4] pad | u32 max | u32 min
            var h = p >> 1;
            return {
                displayPrimariesX:             [m.HEAPU16[h + 2], m.HEAPU16[h + 3], m.HEAPU16[h + 4]],
                displayPrimariesY:             [m.HEAPU16[h + 5], m.HEAPU16[h + 6], m.HEAPU16[h + 7]],
                whitePointX:                   m.HEAPU16[h + 8],
                whitePointY:                   m.HEAPU16[h + 9],
                maxDisplayMasteringLuminance:   m.HEAPU32[(p >> 2) + 6],   // offset 24
                minDisplayMasteringLuminance:   m.HEAPU32[(p >> 2) + 7],   // offset 28
            };
        } finally {
            m._free(p);
        }
    };

    /** cclv -- Content Colour Volume (extended HDR metadata).
     *  Presence flags gate which fields carry useful data. */
    HEIFReader.prototype.propertyCclv = function (propertyId) {
        const m = this._heif._m;
        const p = m._malloc(44);   // heif_cclv_t = 44 bytes
        try {
            m.HEAPU32[p >> 2] = 44;
            this._heif._check(this._heif._fn.getPropCclv(this._handle, propertyId, p),
                              'heif_reader_get_property_cclv');
            // Layout: u32 ss | u8 pp | u8 minp | u8 maxp | u8 avgp | u32[3] px | u32[3] py | u32 min | u32 max | u32 avg
            var w = p >> 2;
            return {
                primariesPresent:      m.HEAPU8[p + 4],
                minLuminancePresent:   m.HEAPU8[p + 5],
                maxLuminancePresent:   m.HEAPU8[p + 6],
                avgLuminancePresent:   m.HEAPU8[p + 7],
                primariesX:            [m.HEAPU32[w + 2], m.HEAPU32[w + 3], m.HEAPU32[w + 4]],
                primariesY:            [m.HEAPU32[w + 5], m.HEAPU32[w + 6], m.HEAPU32[w + 7]],
                minLuminanceValue:     m.HEAPU32[w + 8],
                maxLuminanceValue:     m.HEAPU32[w + 9],
                avgLuminanceValue:     m.HEAPU32[w + 10],
            };
        } finally {
            m._free(p);
        }
    };

    /** amve -- Ambient Viewing Environment.
     *  {ambientIlluminance, ambientLightX, ambientLightY}. */
    HEIFReader.prototype.propertyAmve = function (propertyId) {
        const m = this._heif._m;
        const p = m._malloc(12);   // heif_amve_t = 12 bytes
        try {
            m.HEAPU32[p >> 2] = 12;
            this._heif._check(this._heif._fn.getPropAmve(this._handle, propertyId, p),
                              'heif_reader_get_property_amve');
            return {
                ambientIlluminance: m.HEAPU32[(p >> 2) + 1],
                ambientLightX:      m.HEAPU16[(p >> 1) + 4],
                ambientLightY:      m.HEAPU16[(p >> 1) + 5],
            };
        } finally {
            m._free(p);
        }
    };

    /** reve -- Reference Viewing Environment.
     *  {surroundLuminance, surroundLightX, surroundLightY,
     *   peripheryLuminance, peripheryLightX, peripheryLightY}. */
    HEIFReader.prototype.propertyReve = function (propertyId) {
        const m = this._heif._m;
        const p = m._malloc(20);   // heif_reve_t = 20 bytes
        try {
            m.HEAPU32[p >> 2] = 20;
            this._heif._check(this._heif._fn.getPropReve(this._handle, propertyId, p),
                              'heif_reader_get_property_reve');
            return {
                surroundLuminance:  m.HEAPU32[(p >> 2) + 1],
                surroundLightX:     m.HEAPU16[(p >> 1) + 4],
                surroundLightY:     m.HEAPU16[(p >> 1) + 5],
                peripheryLuminance: m.HEAPU32[(p >> 2) + 3],
                peripheryLightX:    m.HEAPU16[(p >> 1) + 8],
                peripheryLightY:    m.HEAPU16[(p >> 1) + 9],
            };
        } finally {
            m._free(p);
        }
    };

    /** ndwt -- Nominal Diffuse White luminance (single uint32). */
    HEIFReader.prototype.propertyNdwt = function (propertyId) {
        const m = this._heif._m;
        const p = m._malloc(8);    // heif_ndwt_t = 8 bytes
        try {
            m.HEAPU32[p >> 2] = 8;
            this._heif._check(this._heif._fn.getPropNdwt(this._handle, propertyId, p),
                              'heif_reader_get_property_ndwt');
            return { diffuseWhiteLuminance: m.HEAPU32[(p >> 2) + 1] };
        } finally {
            m._free(p);
        }
    };

    // Descriptive / miscellaneous properties

    /** pasp -- pixel aspect ratio. {relativeWidth, relativeHeight}. */
    HEIFReader.prototype.propertyPasp = function (propertyId) {
        const m = this._heif._m;
        const w = m._malloc(4), h = m._malloc(4);
        try {
            this._heif._check(this._heif._fn.getPropPasp(this._handle, propertyId, w, h),
                              'heif_reader_get_property_pasp');
            return { relativeWidth: m.HEAPU32[w >> 2], relativeHeight: m.HEAPU32[h >> 2] };
        } finally {
            m._free(w); m._free(h);
        }
    };

    /** crtt -- creation time (microseconds since 1904-01-01 UTC).
     *  Returns a BigInt for full 64-bit precision. */
    HEIFReader.prototype.propertyCrtt = function (propertyId) {
        const m = this._heif._m;
        const p = m._malloc(8);
        try {
            this._heif._check(this._heif._fn.getPropCrtt(this._handle, propertyId, p),
                              'heif_reader_get_property_crtt');
            var lo = m.HEAPU32[p >> 2];
            var hi = m.HEAPU32[(p >> 2) + 1];
            return BigInt(hi) * BigInt(0x100000000) + BigInt(lo);
        } finally {
            m._free(p);
        }
    };

    /** mdft -- modification time (microseconds since 1904-01-01 UTC).
     *  Returns a BigInt for full 64-bit precision. */
    HEIFReader.prototype.propertyMdft = function (propertyId) {
        const m = this._heif._m;
        const p = m._malloc(8);
        try {
            this._heif._check(this._heif._fn.getPropMdft(this._handle, propertyId, p),
                              'heif_reader_get_property_mdft');
            var lo = m.HEAPU32[p >> 2];
            var hi = m.HEAPU32[(p >> 2) + 1];
            return BigInt(hi) * BigInt(0x100000000) + BigInt(lo);
        } finally {
            m._free(p);
        }
    };

    // Helper: read a variable-length string field via size-probe.
    function fetchStringField(heif, fn, args, what) {
        var bytes = fetchSized(heif, fn, args, what);
        if (!bytes || bytes.byteLength === 0) return '';
        var len = bytes.byteLength;
        if (bytes[len - 1] === 0) len--;
        var s = '';
        for (var i = 0; i < len; i++) s += String.fromCharCode(bytes[i]);
        return s;
    }

    /** udes -- user description. {lang, name, description, tags}. */
    HEIFReader.prototype.propertyUdes = function (propertyId) {
        var fn = this._heif._fn.getPropUdesField;
        var h = this._handle;
        return {
            lang:        fetchStringField(this._heif, fn, [h, propertyId, 0], 'udes.lang'),
            name:        fetchStringField(this._heif, fn, [h, propertyId, 1], 'udes.name'),
            description: fetchStringField(this._heif, fn, [h, propertyId, 2], 'udes.description'),
            tags:        fetchStringField(this._heif, fn, [h, propertyId, 3], 'udes.tags'),
        };
    };

    /** altt -- accessibility text. {text, lang}. */
    HEIFReader.prototype.propertyAltt = function (propertyId) {
        var fn = this._heif._fn.getPropAlttField;
        var h = this._handle;
        return {
            text: fetchStringField(this._heif, fn, [h, propertyId, 0], 'altt.text'),
            lang: fetchStringField(this._heif, fn, [h, propertyId, 1], 'altt.lang'),
        };
    };

    /** rref -- required reference types. Array of FourCC strings. */
    HEIFReader.prototype.propertyRref = function (propertyId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let n;
        try {
            this._heif._check(fn.getPropRrefCount(this._handle, propertyId, cnt),
                              'heif_reader_get_property_rref_count');
            n = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const buf = m._malloc(5);
        const result = new Array(n);
        try {
            for (var i = 0; i < n; i++) {
                this._heif._check(fn.getPropRrefAt(this._handle, propertyId, i, buf),
                                  'heif_reader_get_property_rref_at');
                result[i] = m.UTF8ToString(buf);
            }
        } finally {
            m._free(buf);
        }
        return result;
    };

    /** raw -- unrecognised property. {type, data}.
     *  type is the original FourCC; data is the full box payload
     *  as Uint8Array. */
    HEIFReader.prototype.propertyRaw = function (propertyId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const buf = m._malloc(5);
        let type;
        try {
            this._heif._check(fn.getPropRawType(this._handle, propertyId, buf),
                              'heif_reader_get_property_raw_type');
            type = m.UTF8ToString(buf);
        } finally {
            m._free(buf);
        }
        const data = fetchSized(this._heif, fn.getPropRawData,
                                [this._handle, propertyId],
                                'heif_reader_get_property_raw_data');
        return { type: type, data: data };
    };

    /** subs -- sub-sample information. Array of
     *  {subsampleSize, subsamplePriority, discardable}. */
    HEIFReader.prototype.propertySubs = function (propertyId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let n;
        try {
            this._heif._check(fn.getPropSubsCount(this._handle, propertyId, cnt),
                              'heif_reader_get_property_subs_count');
            n = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const STRUCT_SIZE = 16;   // heif_subs_entry_t = 16 bytes
        const p = m._malloc(STRUCT_SIZE);
        const result = new Array(n);
        try {
            for (var i = 0; i < n; i++) {
                m.HEAPU32[p >> 2] = STRUCT_SIZE;
                this._heif._check(fn.getPropSubsAt(this._handle, propertyId, i, p),
                                  'heif_reader_get_property_subs_at');
                result[i] = {
                    subsampleSize:     m.HEAPU32[(p >> 2) + 1],
                    subsamplePriority: m.HEAPU8[p + 8],
                    discardable:       m.HEAPU8[p + 9] !== 0,
                };
            }
        } finally {
            m._free(p);
        }
        return result;
    };

    // Composition: grid / overlay

    /** Grid derivation header + tile list.
     *  Returns {outputWidth, outputHeight, columns, rows, tileIds}.
     *  `tileIds` is laid out row-major: tileIds[row * columns + col].
     *
     *  Pre-canvas size = (tileWidth * columns, tileHeight * rows).
     *  Output size may be smaller (the grid is cropped to fit). All
     *  tiles must be the same encoded size (per ISO/IEC 23008-12). */
    HEIFReader.prototype.grid = function (itemId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        // heif_grid_t: 5 x uint32 = 20 bytes
        const hdr = m._malloc(20);
        m.HEAPU32[(hdr >> 2) + 0] = 20;  // struct_size
        let outputWidth, outputHeight, columns, rows;
        try {
            this._heif._check(fn.getGrid(this._handle, itemId, hdr),
                              'heif_reader_get_grid');
            outputWidth  = m.HEAPU32[(hdr >> 2) + 1];
            outputHeight = m.HEAPU32[(hdr >> 2) + 2];
            columns      = m.HEAPU32[(hdr >> 2) + 3];
            rows         = m.HEAPU32[(hdr >> 2) + 4];
        } finally {
            m._free(hdr);
        }
        const tileIds = collectU32(this._heif, fn.getGridImageCount, fn.getGridImageAt,
                                   this._handle, itemId);
        return { outputWidth, outputHeight, columns, rows, tileIds: Array.from(tileIds) };
    };

    /** Overlay derivation header + child list.
     *  Returns {outputWidth, outputHeight, fillRgba, images}, where
     *  `fillRgba = [r,g,b,a]` are 16-bit sRGB fill components for
     *  empty canvas regions (a = 0 transparent, 65535 opaque), and
     *  `images = [{imageId, hOffset, vOffset}, ...]` lists each child
     *  layer with its top-left offset relative to the canvas origin.
     *  Offsets are signed (children may extend off-canvas). */
    HEIFReader.prototype.overlay = function (itemId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        // heif_overlay_t: u32 + 4xu16 + 2xu32 = 20 bytes
        const hdr = m._malloc(20);
        m.HEAPU32[(hdr >> 2) + 0] = 20;
        let header;
        try {
            this._heif._check(fn.getOverlay(this._handle, itemId, hdr),
                              'heif_reader_get_overlay');
            // u16 fields start at offset 4
            const u16 = (m.HEAPU16);
            header = {
                fillRgba: [
                    u16[(hdr + 4)  >> 1],
                    u16[(hdr + 6)  >> 1],
                    u16[(hdr + 8)  >> 1],
                    u16[(hdr + 10) >> 1],
                ],
                outputWidth:  m.HEAPU32[(hdr + 12) >> 2],
                outputHeight: m.HEAPU32[(hdr + 16) >> 2],
            };
        } finally {
            m._free(hdr);
        }

        // Image-list walk: heif_overlay_image_t = 4 x i32 = 16 bytes
        const cnt = m._malloc(4);
        let count;
        try {
            this._heif._check(fn.getOverlayImageCount(this._handle, itemId, cnt),
                              'heif_reader_get_overlay_image_count');
            count = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const entry = m._malloc(16);
        const images = new Array(count);
        try {
            for (let i = 0; i < count; i++) {
                m.HEAPU32[(entry >> 2) + 0] = 16;  // struct_size
                this._heif._check(
                    fn.getOverlayImageAt(this._handle, itemId, i, entry),
                    'heif_reader_get_overlay_image_at'
                );
                images[i] = {
                    imageId: m.HEAPU32[(entry >> 2) + 1],
                    hOffset: m.HEAP32 ? m.HEAP32[(entry >> 2) + 2] : (m.HEAPU32[(entry >> 2) + 2] | 0),
                    vOffset: m.HEAP32 ? m.HEAP32[(entry >> 2) + 3] : (m.HEAPU32[(entry >> 2) + 3] | 0),
                };
            }
        } finally {
            m._free(entry);
        }
        return Object.assign(header, { images });
    };

    // Tonemap / gainmap (tmap derived items)

    /** GainMapData header for a `tmap` derived item.
     *  Returns the per-`tmap` POD plus the per-channel records:
     *    {
     *      minimumVersion, writerVersion,
     *      isMultichannel, useBaseColourSpace,
     *      channelCount,                            // 1 or 3 per spec
     *      baseHdrHeadroomNumerator,   baseHdrHeadroomDenominator,
     *      alternateHdrHeadroomNumerator, alternateHdrHeadroomDenominator,
     *      channels: [{                             // length === channelCount
     *        gainMapMinNumerator, gainMapMinDenominator,
     *        gainMapMaxNumerator, gainMapMaxDenominator,
     *        gammaNumerator,      gammaDenominator,
     *        baseOffsetNumerator, baseOffsetDenominator,
     *        alternateOffsetNumerator, alternateOffsetDenominator,
     *      }, ...]
     *    }
     *  Field names mirror the GainMapData wire format (per-channel
     *  rationals); convert num/denom -> float at the consumer.
     *  Walks getTmapGainmap() once + getTmapChannelCount/At per
     *  channel; one tmap = 2 (single-channel) or 4 (3-channel) wasm
     *  round-trips total. Caller-cached: tmap headers do not change
     *  during a reader's lifetime. */
    HEIFReader.prototype.tonemapGainmap = function (itemId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        // heif_gainmap_t: 32 bytes (see heifreader_c.h:1114).
        //   +0   u32  struct_size
        //   +4   u16  minimum_version
        //   +6   u16  writer_version
        //   +8   u8   is_multichannel
        //   +9   u8   use_base_colour_space
        //   +10  u8[2] reserved
        //   +12  i32  channel_count
        //   +16  u32  base_hdr_headroom_numerator
        //   +20  u32  base_hdr_headroom_denominator
        //   +24  u32  alternate_hdr_headroom_numerator
        //   +28  u32  alternate_hdr_headroom_denominator
        const hdr = m._malloc(32);
        m.HEAPU32[(hdr >> 2) + 0] = 32;  // struct_size
        let header;
        try {
            this._heif._check(fn.getTmapGainmap(this._handle, itemId, hdr),
                              'heif_reader_get_tonemap_gainmap');
            header = {
                minimumVersion:                  m.HEAPU16[(hdr +  4) >> 1],
                writerVersion:                   m.HEAPU16[(hdr +  6) >> 1],
                isMultichannel:                  m.HEAPU8[hdr +  8] !== 0,
                useBaseColourSpace:              m.HEAPU8[hdr +  9] !== 0,
                channelCount:                    (m.HEAP32 ? m.HEAP32[(hdr + 12) >> 2]
                                                            : (m.HEAPU32[(hdr + 12) >> 2] | 0)),
                baseHdrHeadroomNumerator:        m.HEAPU32[(hdr + 16) >> 2],
                baseHdrHeadroomDenominator:      m.HEAPU32[(hdr + 20) >> 2],
                alternateHdrHeadroomNumerator:   m.HEAPU32[(hdr + 24) >> 2],
                alternateHdrHeadroomDenominator: m.HEAPU32[(hdr + 28) >> 2],
            };
        } finally {
            m._free(hdr);
        }
        // Per-channel records: heif_gainmap_channel_t = 44 bytes.
        //   +0   u32  struct_size
        //   +4   i32  gain_map_min_numerator
        //   +8   u32  gain_map_min_denominator
        //   +12  i32  gain_map_max_numerator
        //   +16  u32  gain_map_max_denominator
        //   +20  u32  gamma_numerator
        //   +24  u32  gamma_denominator
        //   +28  i32  base_offset_numerator
        //   +32  u32  base_offset_denominator
        //   +36  i32  alternate_offset_numerator
        //   +40  u32  alternate_offset_denominator
        const cnt = m._malloc(4);
        let n;
        try {
            this._heif._check(fn.getTmapChannelCount(this._handle, itemId, cnt),
                              'heif_reader_get_tonemap_gainmap_channel_count');
            n = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const ch = m._malloc(44);
        const channels = new Array(n);
        const i32 = m.HEAP32;
        const i32at = (off) => (i32 ? i32[(ch + off) >> 2] : (m.HEAPU32[(ch + off) >> 2] | 0));
        try {
            for (let i = 0; i < n; i++) {
                m.HEAPU32[(ch >> 2) + 0] = 44;
                this._heif._check(
                    fn.getTmapChannelAt(this._handle, itemId, i, ch),
                    'heif_reader_get_tonemap_gainmap_channel_at'
                );
                channels[i] = {
                    gainMapMinNumerator:        i32at(4),
                    gainMapMinDenominator:      m.HEAPU32[(ch +  8) >> 2],
                    gainMapMaxNumerator:        i32at(12),
                    gainMapMaxDenominator:      m.HEAPU32[(ch + 16) >> 2],
                    gammaNumerator:             m.HEAPU32[(ch + 20) >> 2],
                    gammaDenominator:           m.HEAPU32[(ch + 24) >> 2],
                    baseOffsetNumerator:        i32at(28),
                    baseOffsetDenominator:      m.HEAPU32[(ch + 32) >> 2],
                    alternateOffsetNumerator:   i32at(36),
                    alternateOffsetDenominator: m.HEAPU32[(ch + 40) >> 2],
                };
            }
        } finally {
            m._free(ch);
        }
        header.channels = channels;
        return header;
    };

    /** The `dimg`-referenced image-id list for a `tmap` item.
     *  By Apple-style HDR-gainmap convention this is
     *  [baseImageId, gainmapImageId]; downstream code should branch
     *  on length rather than assume the order. */
    HEIFReader.prototype.tonemapImages = function (itemId) {
        const fn = this._heif._fn;
        return Array.from(collectU32(this._heif,
                                     fn.getTmapImageCount, fn.getTmapImageAt,
                                     this._handle, itemId));
    };

    /** Entity groups (`GroupsListBox` payload).
     *
     *  Returns an array of `{groupId, type, entityIds}` for every
     *  group declared in the file. The group `type` is the 4-CC the
     *  GroupsListBox uses to label the relationship: `ster` (stereo
     *  pair), `tsyn` (time-synchronised, e.g. front+back camera),
     *  `pano`, `slid`, `prgr`, `iaug` (image-with-audio), `aebr`/
     *  `wbbr`/`fobr`/`afbr`/`dobr` (auto-exposure/white-balance/focus/
     *  flash/depth-of-field brackets), `albc`, `favc`, `brst`, etc.
     *  `entityIds` is the ordered list of item or track ids the group
     *  binds together. Empty array if the file has no GroupsListBox.
     *
     *  The C facade walks `groups[i]` then `entities[j]` per group;
     *  this binding does the same and returns a flat JS shape so the
     *  website's classifiers can index by type. */
    HEIFReader.prototype.entityGroups = function () {
        const m  = this._heif._m;
        const fn = this._heif._fn;
        const handle = this._handle;

        // heif_entity_group_t = {u32 struct_size, u32 group_id, char[5] type, u8 _r[3]} = 16 bytes
        const hdr = m._malloc(16);
        const cnt = m._malloc(4);
        try {
            this._heif._check(fn.getEntGrpCount(handle, cnt), 'heif_reader_get_entity_group_count');
            const groupCount = m.HEAPU32[cnt >> 2];
            const out = new Array(groupCount);
            for (let gi = 0; gi < groupCount; gi++) {
                m.HEAPU32[hdr >> 2] = 16;  // struct_size
                this._heif._check(fn.getEntGrpAt(handle, gi, hdr), 'heif_reader_get_entity_group_at');
                const groupId = m.HEAPU32[(hdr + 4) >> 2];
                const type    = m.UTF8ToString(hdr + 8);  // char[5]
                const entityIds = Array.from(collectU32(this._heif,
                                                        fn.getEntGrpEntCount, fn.getEntGrpEntAt,
                                                        handle, gi));
                out[gi] = { groupId: groupId, type: type, entityIds: entityIds };
            }
            return out;
        } finally {
            m._free(cnt);
            m._free(hdr);
        }
    };

    // Tracks (msf1 image sequences)

    // Layout of heif_track_info_t (56 bytes total, see heifreader_c.h):
    //   u32 struct_size
    //   u32 alternate_group_id
    //   u64 max_sample_size                (8-byte aligned)
    //   u32 track_id
    //   u32 features
    //   u32 time_scale
    //   u32 frame_rate_num
    //   u32 frame_rate_den
    //   u32 minor_version
    //   u8  has_type_information; u8 _r[3]
    //   char major_brand[5]; u8 _r[7]    -> trailing padding to 56
    function readTrackInfo(m, p) {
        // u64 fields: under -sWASM=0 the heap is little-endian, two i32 words.
        // We assume the high word is 0 for sane sample sizes (< 4 GiB).
        return {
            alternateGroupId:  m.HEAPU32[(p +  4) >> 2],
            maxSampleSize:     m.HEAPU32[(p +  8) >> 2] +
                               m.HEAPU32[(p + 12) >> 2] * 0x100000000,
            trackId:           m.HEAPU32[(p + 16) >> 2],
            features:          m.HEAPU32[(p + 20) >> 2],
            timeScale:         m.HEAPU32[(p + 24) >> 2],
            frameRate:         { num: m.HEAPU32[(p + 28) >> 2],
                                 den: m.HEAPU32[(p + 32) >> 2] },
            minorVersion:      m.HEAPU32[(p + 36) >> 2],
            hasTypeInformation:m.HEAPU8[p + 40] !== 0,
            majorBrand:        m.UTF8ToString(p + 44),  // char[5]
        };
    }

    // Layout of heif_sample_info_t (72 bytes total).
    function readSampleInfo(m, p) {
        return {
            sampleId:                  m.HEAPU32[(p +  4) >> 2],
            sampleDurationTs:          m.HEAPU32[(p +  8) >> 2] +
                                       m.HEAPU32[(p + 12) >> 2] * 0x100000000,
            size:                      m.HEAPU32[(p + 16) >> 2] +
                                       m.HEAPU32[(p + 20) >> 2] * 0x100000000,
            earliestTimestampMs:       m.HEAPU32[(p + 24) >> 2] +
                                       m.HEAPU32[(p + 28) >> 2] * 0x100000000,
            earliestTimestampTs:       m.HEAPU32[(p + 32) >> 2] +
                                       m.HEAPU32[(p + 36) >> 2] * 0x100000000,
            sampleCompositionOffsetTs: (m.HEAPU32[(p + 40) >> 2] +
                                       (m.HEAP32 ? m.HEAP32[(p + 44) >> 2]
                                                 : (m.HEAPU32[(p + 44) >> 2] | 0))
                                       * 0x100000000),
            sampleDescriptionIndex:    m.HEAPU32[(p + 48) >> 2],
            segmentId:                 m.HEAPU32[(p + 52) >> 2],
            sampleFlags:               m.HEAPU32[(p + 56) >> 2],
            sampleType:                m.HEAPU8[p + 60],   // SampleType enum: 0=OUT_NON_REF, 1=OUT_REF, 2=NON_OUT_REF
            hasClap:                   m.HEAPU8[p + 61] !== 0,
            hasAuxi:                   m.HEAPU8[p + 62] !== 0,
            codingConstraints:         m.HEAPU8[p + 63],
            sampleEntryType:           m.UTF8ToString(p + 64),  // char[5]
        };
    }

    /** All track ids in the file. Empty for pure still-image HEIFs.
     *  Returns a Uint32Array. */
    HEIFReader.prototype.tracks = function () {
        const fn = this._heif._fn;
        return collectU32(this._heif, fn.getTrackCount, fn.getTrackIdAt, this._handle);
    };

    /** Per-track scalars: alternateGroupId, maxSampleSize, trackId,
     *  features (bitmask), timeScale, frameRate {num,den},
     *  hasTypeInformation, majorBrand. Mirrors HEIF::TrackInformation
     *  scalars; companion arrays via this.trackReferences() etc. */
    HEIFReader.prototype.trackInfo = function (trackId) {
        const m = this._heif._m;
        const p = m._malloc(56);
        // Caller must set struct_size before the call.
        m.HEAPU32[p >> 2] = 56;
        try {
            this._heif._check(this._heif._fn.getTrackInfo(this._handle, trackId, p),
                              'heif_reader_get_track_info');
            return readTrackInfo(m, p);
        } finally {
            m._free(p);
        }
    };

    /** Display width from the track header. */
    HEIFReader.prototype.trackDisplayWidth = function (trackId) {
        return readU32At(this._heif, this._heif._fn.getTrackDispW,
                         this._handle, trackId, 'heif_reader_get_track_display_width');
    };

    /** Display height from the track header. */
    HEIFReader.prototype.trackDisplayHeight = function (trackId) {
        return readU32At(this._heif, this._heif._fn.getTrackDispH,
                         this._handle, trackId, 'heif_reader_get_track_display_height');
    };

    /** Number of samples in the track. Single-cwrap fast path that
     *  avoids walking the full sample-info table when the caller only
     *  needs the count (e.g. metadata panels reporting frame count
     *  on a 600-sample sequence). For the full per-sample PODs use
     *  trackSamples(); for one sample use trackSampleAt(). */
    HEIFReader.prototype.trackSampleCount = function (trackId) {
        return readU32At(this._heif, this._heif._fn.getTrackSampleCount,
                         this._handle, trackId, 'heif_reader_get_track_sample_count');
    };

    /** Playback duration in seconds, honouring the edit list. */
    HEIFReader.prototype.trackPlaybackDurationSecs = function (trackId) {
        const m = this._heif._m;
        const p = m._malloc(8);
        try {
            this._heif._check(this._heif._fn.getTrackDuration(this._handle, trackId, p),
                              'heif_reader_get_track_playback_duration_secs');
            // double -- use getValue(p, 'double')
            return m.getValue(p, 'double');
        } finally {
            m._free(p);
        }
    };

    /** All samples in the track, as an array of sample-info objects.
     *  Each has sampleId, size, sampleDurationTs, earliestTimestampMs,
     *  sampleType, sampleEntryType (4cc), sampleFlags, etc.
     *  See readSampleInfo above for the full field set. */
    HEIFReader.prototype.trackSamples = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let count;
        try {
            this._heif._check(fn.getTrackSampleCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_sample_count');
            count = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const p = m._malloc(72);
        const result = new Array(count);
        try {
            for (let i = 0; i < count; i++) {
                m.HEAPU32[p >> 2] = 72;  // struct_size
                this._heif._check(fn.getTrackSampleAt(this._handle, trackId, i, p),
                                  'heif_reader_get_track_sample_at');
                result[i] = readSampleInfo(m, p);
            }
        } finally {
            m._free(p);
        }
        return result;
    };

    /** Per-track presentation timestamps as [{timestampMs, sampleId}, ...].
     *  Honours the edit list, so this is the right thing to drive a
     *  playback loop with. Mirrors HEIF::Reader::getItemTimestamps. */
    HEIFReader.prototype.trackTimestamps = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let count;
        try {
            this._heif._check(fn.getTrackTsCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_item_timestamp_count');
            count = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        // Each call writes (uint64 timestamp_ms, uint32 sample_id) to
        // separate out pointers.
        const tsLo = m._malloc(8);
        const sid  = m._malloc(4);
        const result = new Array(count);
        try {
            for (let i = 0; i < count; i++) {
                this._heif._check(fn.getTrackTsAt(this._handle, trackId, i, tsLo, sid),
                                  'heif_reader_get_track_item_timestamp_at');
                result[i] = {
                    timestampMs: m.HEAPU32[tsLo >> 2] +
                                 m.HEAPU32[(tsLo >> 2) + 1] * 0x100000000,
                    sampleId:    m.HEAPU32[sid >> 2],
                };
            }
        } finally {
            m._free(tsLo); m._free(sid);
        }
        return result;
    };

    /** Track-reference (`tref`) edges as [{type, referencedTrackId}, ...].
     *  Common types: 'thmb' (thumbnail of), 'auxl' (auxiliary of). */
    HEIFReader.prototype.trackReferences = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let count;
        try {
            this._heif._check(fn.getTrackRefCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_reference_count');
            count = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const buf = m._malloc(5);
        const out = m._malloc(4);
        const result = new Array(count);
        try {
            for (let i = 0; i < count; i++) {
                this._heif._check(fn.getTrackRefAt(this._handle, trackId, i, buf, out),
                                  'heif_reader_get_track_reference_at');
                result[i] = {
                    type:               m.UTF8ToString(buf),
                    referencedTrackId:  m.HEAPU32[out >> 2],
                };
            }
        } finally {
            m._free(buf); m._free(out);
        }
        return result;
    };

    /** Decoder-ready bitstream for one sample of an image-sequence
     *  track. Includes the codec config record(s) for the sample's
     *  description entry followed by the sample payload with NAL
     *  bytestream headers (0x000001) substituted for length prefixes.
     *  Hand the result straight to libde265/vvdec.
     *
     *  `sampleId` is the sample-table index (0-based), as returned
     *  in heif_sample_info_t.sampleId by trackSamples(). It is NOT
     *  an item id from the 'meta' box; the two id spaces are disjoint.
     *
     *  Mirrors HEIF::Reader::getItemDataWithDecoderParameters(
     *  SequenceId, SequenceImageId, ...). Uses the standard size-probe
     *  size-probe (see fetchSized); the alternate spelling
     *  MEMORY_TOO_SMALL_BUFFER (26) on probe miss is handled
     *  transparently. */
    HEIFReader.prototype.trackSampleBitstream = function (trackId, sampleId) {
        const fn = this._heif._fn;
        return fetchSized(this._heif, fn.getTrackSampleDataDP,
                          [this._handle, trackId, sampleId],
                          'heif_reader_get_track_sample_data_with_decoder_parameters');
    };

    /** Raw on-wire bytes for one sample of an image-sequence track,
     *  without the decoder-config prefix. For 'hvc1'/'avc1' the
     *  nal_length fields are still substituted with bytestream
     *  headers (0x000001). Use trackSampleBitstream() instead when
     *  feeding a decoder.
     *
     *  Mirrors HEIF::Reader::getItemData(SequenceId, SequenceImageId,
     *  ..., bytestreamHeaders=true). The raw variant DOES propagate
     *  probe size correctly so we use the standard size-probe helper. */
    HEIFReader.prototype.trackSampleRawData = function (trackId, sampleId) {
        const fn = this._heif._fn;
        return fetchSized(this._heif, fn.getTrackSampleData,
                          [this._handle, trackId, sampleId],
                          'heif_reader_get_track_sample_data');
    };

    // Track edit list, alternates, groups, compatible brands

    // Layout of heif_edit_unit_t (32 bytes total).
    function readEditUnit(m, p) {
        const editTypes = ['EMPTY', 'DWELL', 'SHIFT', 'RAW'];
        const editTypeRaw = m.HEAPU8[p + 4];
        const mediaTimeLo = m.HEAPU32[(p +  8) >> 2];
        const mediaTimeHi = m.HEAP32 [(p + 12) >> 2];
        const durationLo  = m.HEAPU32[(p + 16) >> 2];
        const durationHi  = m.HEAPU32[(p + 20) >> 2];
        const rateInt     = (m.HEAP16 ? m.HEAP16[(p + 24) >> 1]
                                      : (m.HEAPU16[(p + 24) >> 1] << 16) >> 16);
        const rateFrac    = (m.HEAP16 ? m.HEAP16[(p + 26) >> 1]
                                      : (m.HEAPU16[(p + 26) >> 1] << 16) >> 16);
        return {
            editType:           editTypes[editTypeRaw] || ('?' + editTypeRaw),
            mediaTimeTrackTs:   mediaTimeLo + mediaTimeHi * 0x100000000,  // signed (hi is i32)
            durationMovieTs:    durationLo  + durationHi  * 0x100000000,
            mediaRateInteger:   rateInt,
            mediaRateFraction:  rateFrac,
            // 16.16 fixed-point combined value for convenience.
            mediaRate:          rateInt + rateFrac / 65536,
        };
    }

    /** EditList wrapper scalars: {looping, repetitions}. `looping`
     *  is true/false; `repetitions` is the loop count (0 means
     *  infinite when `looping` is true). The actual edit operations
     *  are returned by trackEditUnits().
     *
     *  Tracks without an edit list: the C call returns
     *  `NOT_APPLICABLE` (enum 27); we surface that as
     *  `{looping: false, repetitions: 0}` rather than throwing. */
    HEIFReader.prototype.trackEditListMeta = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const lp = m._malloc(4);
        const rp = m._malloc(8);
        try {
            const code = fn.getTrackEditMeta(this._handle, trackId, lp, rp);
            if (code === 27 /* NOT_APPLICABLE */) {
                this._heif._fn.clearError();
                return { looping: false, repetitions: 0 };
            }
            this._heif._check(code, 'heif_reader_get_track_edit_list_meta');
            return {
                looping:     m.HEAP32[lp >> 2] !== 0,
                repetitions: m.getValue(rp, 'double'),
            };
        } finally {
            m._free(lp); m._free(rp);
        }
    };

    /** Edit-list operations as an array of edit units, each with
     *  {editType, mediaTimeTrackTs, durationMovieTs, mediaRate*}.
     *  An empty array means there is no edit list. Mirrors the
     *  HEIF::EditList::editUnits vector. */
    HEIFReader.prototype.trackEditUnits = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let count;
        try {
            this._heif._check(fn.getTrackEditCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_edit_unit_count');
            count = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const p = m._malloc(32);
        const result = new Array(count);
        try {
            for (let i = 0; i < count; i++) {
                m.HEAPU32[p >> 2] = 32;  // struct_size
                this._heif._check(fn.getTrackEditAt(this._handle, trackId, i, p),
                                  'heif_reader_get_track_edit_unit_at');
                result[i] = readEditUnit(m, p);
            }
        } finally {
            m._free(p);
        }
        return result;
    };

    /** Track ids of all alternate tracks sharing this track's
     *  alternateGroupId (the `alternateGroupId` value itself is on
     *  trackInfo()). Empty if the track stands alone. Returns a
     *  Uint32Array. */
    HEIFReader.prototype.trackAlternates = function (trackId) {
        const fn = this._heif._fn;
        return collectU32(this._heif, fn.getTrackAltCount, fn.getTrackAltAt,
                          this._handle, trackId);
    };

    /** Track-group memberships as [{type, memberTrackId}, ...]. The
     *  4cc `type` is the grouping kind (e.g. 'ster' for stereo
     *  pair). Mirrors HEIF::TrackInformation::trackGroupIds. */
    HEIFReader.prototype.trackGroups = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let count;
        try {
            this._heif._check(fn.getTrackGroupCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_group_count');
            count = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const buf = m._malloc(5);
        const out = m._malloc(4);
        const result = new Array(count);
        try {
            for (let i = 0; i < count; i++) {
                this._heif._check(fn.getTrackGroupAt(this._handle, trackId, i, buf, out),
                                  'heif_reader_get_track_group_at');
                result[i] = {
                    type:           m.UTF8ToString(buf),
                    memberTrackId:  m.HEAPU32[out >> 2],
                };
            }
        } finally {
            m._free(buf); m._free(out);
        }
        return result;
    };

    /** Per-track compatible brands (from TrackTypeInformation /
     *  `tyco` in `tref`). Returns an array of 4cc strings. Empty
     *  when trackInfo().hasTypeInformation is false. */
    HEIFReader.prototype.trackCompatibleBrands = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let count;
        try {
            this._heif._check(fn.getTrackBrandCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_compatible_brand_count');
            count = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const buf = m._malloc(5);
        const result = new Array(count);
        try {
            for (let i = 0; i < count; i++) {
                this._heif._check(fn.getTrackBrandAt(this._handle, trackId, i, buf),
                                  'heif_reader_get_track_compatible_brand_at');
                result[i] = m.UTF8ToString(buf);
            }
        } finally {
            m._free(buf);
        }
        return result;
    };

    // Per-sample timestamps + DASH segments

    /** All presentation timestamps (milliseconds) at which the given
     *  sample should be displayed. An image may be shown more than
     *  once under an edit list (e.g. a DWELL unit holds a frame on
     *  screen across multiple wall-clock points), hence an array.
     *  Mirrors HEIF::Reader::getTimestampsOfItem. */
    HEIFReader.prototype.itemTimestamps = function (trackId, sampleId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let count;
        try {
            this._heif._check(fn.getItemTsCount(this._handle, trackId, sampleId, cnt),
                              'heif_reader_get_item_timestamp_count');
            count = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const tsLo = m._malloc(8);
        const result = new Array(count);
        try {
            for (let i = 0; i < count; i++) {
                this._heif._check(fn.getItemTsAt(this._handle, trackId, sampleId, i, tsLo),
                                  'heif_reader_get_item_timestamp_at');
                // int64_t timestamp_ms -- split into low/high i32 words.
                const lo = m.HEAPU32[tsLo >> 2];
                const hi = m.HEAP32 [(tsLo + 4) >> 2];
                result[i] = lo + hi * 0x100000000;
            }
        } finally {
            m._free(tsLo);
        }
        return result;
    };

    // Layout of heif_segment_info_t (48 bytes total).
    function readSegmentInfo(m, p) {
        return {
            segmentId:        m.HEAPU32[(p +  4) >> 2],
            earliestPtsTs:    m.HEAPU32[(p +  8) >> 2] +
                              m.HEAPU32[(p + 12) >> 2] * 0x100000000,
            startDataOffset:  m.HEAPU32[(p + 16) >> 2] +
                              m.HEAPU32[(p + 20) >> 2] * 0x100000000,
            referenceId:      m.HEAPU32[(p + 24) >> 2],
            timescale:        m.HEAPU32[(p + 28) >> 2],
            durationTs:       m.HEAPU32[(p + 32) >> 2],
            dataSize:         m.HEAPU32[(p + 36) >> 2],
            referenceType:    m.HEAPU8 [p + 40],   // 0=media, 1=other sidx
            startsWithSap:    m.HEAPU8 [p + 41] !== 0,
            sapType:          m.HEAPU8 [p + 42],
        };
    }

    /** DASH-style file-level segment index, parsed from `sidx` boxes.
     *  Returns an array of segment-info objects in document order;
     *  empty for files that do not carry a sidx. Mirrors
     *  HEIF::Reader::getSegmentIndex. */
    HEIFReader.prototype.segments = function () {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let count;
        try {
            this._heif._check(fn.getSegCount(this._handle, cnt),
                              'heif_reader_get_segment_index_count');
            count = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const p = m._malloc(48);
        const result = new Array(count);
        try {
            for (let i = 0; i < count; i++) {
                m.HEAPU32[p >> 2] = 48;  // struct_size
                this._heif._check(fn.getSegAt(this._handle, i, p),
                                  'heif_reader_get_segment_index_at');
                result[i] = readSegmentInfo(m, p);
            }
        } finally {
            m._free(p);
        }
        return result;
    };

    // Per-sample geometry, tkhd matrix, master images, sample-by-type

    /** Coded width of a single sequence sample (may differ from the
     *  track-level display width once the tkhd matrix / clap / track-
     *  header scaling are applied). */
    HEIFReader.prototype.trackSampleWidth = function (trackId, sampleId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const out = m._malloc(4);
        try {
            this._heif._check(fn.getTrackSampleW(this._handle, trackId, sampleId, out),
                              'heif_reader_get_track_sample_width');
            return m.HEAPU32[out >> 2];
        } finally {
            m._free(out);
        }
    };

    /** Coded height of a single sequence sample. */
    HEIFReader.prototype.trackSampleHeight = function (trackId, sampleId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const out = m._malloc(4);
        try {
            this._heif._check(fn.getTrackSampleH(this._handle, trackId, sampleId, out),
                              'heif_reader_get_track_sample_height');
            return m.HEAPU32[out >> 2];
        } finally {
            m._free(out);
        }
    };

    /** {width, height} for a single sequence sample. */
    HEIFReader.prototype.trackSampleSize = function (trackId, sampleId) {
        return {
            width:  this.trackSampleWidth (trackId, sampleId),
            height: this.trackSampleHeight(trackId, sampleId),
        };
    };

    /** Per-track 9-entry tkhd display matrix as an Int32Array of
     *  length 9 (a, b, u, c, d, v, x, y, w -- ISOBMFF order).
     *  Identity is `[0x10000, 0, 0, 0, 0x10000, 0, 0, 0, 0x40000000]`
     *  (16.16 fixed for the 6 affine entries; 2.30 for the 3
     *  perspective entries, per ISO/IEC 14496-12 Section 6.2.2).
     *  Mirrors `Reader::getMatrix`. */
    HEIFReader.prototype.trackMatrix = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const out = m._malloc(4);
        const result = new Int32Array(9);
        try {
            for (let i = 0; i < 9; i++) {
                this._heif._check(fn.getTrackMatrixAt(this._handle, trackId, i, out),
                                  'heif_reader_get_track_matrix_at');
                result[i] = m.HEAP32[out >> 2];
            }
            return result;
        } finally {
            m._free(out);
        }
    };

    /** Sample ids of master / sync samples in the track (random-access
     *  points; useful for seek anchors and thumbnail strips).
     *  Mirrors `Reader::getMasterImages(SequenceId, ...)`. */
    HEIFReader.prototype.trackMasterImages = function (trackId) {
        const fn = this._heif._fn;
        return collectU32(this._heif, fn.getTrackMasterCount, fn.getTrackMasterAt,
                          this._handle, trackId);
    };

    /** Sample ids filtered by `HEIF::TrackSampleType` classification
     *  (the same uint8 enum exposed via `heif_sample_info_t.sample_type`
     *  on each sample-info). Mirrors `Reader::getItemListByType` for
     *  sequences. */
    HEIFReader.prototype.trackSamplesByType = function (trackId, sampleType) {
        const fn = this._heif._fn;
        return collectU32(this._heif, fn.getTrackByTypeCount, fn.getTrackByTypeAt,
                          this._handle, trackId, sampleType);
    };

    // Sample-description-entry properties (clap, auxi)

    // Layout of heif_clap_t (36 bytes total: struct_size + 4 uint32 + 2 int32 + 2 uint32).
    // Offset numerators are int32_t (signed per ISO/IEC 14496-12 Section 12.1.4.1).
    function readClap(m, p) {
        return {
            widthN:             m.HEAPU32[(p +  4) >> 2],
            widthD:             m.HEAPU32[(p +  8) >> 2],
            heightN:            m.HEAPU32[(p + 12) >> 2],
            heightD:            m.HEAPU32[(p + 16) >> 2],
            horizontalOffsetN:  m.HEAP32 [(p + 20) >> 2],
            horizontalOffsetD:  m.HEAPU32[(p + 24) >> 2],
            verticalOffsetN:    m.HEAP32 [(p + 28) >> 2],
            verticalOffsetD:    m.HEAPU32[(p + 32) >> 2],
        };
    }

    // The sample-description-entry property accessors (clap, auxi) return a
    // family of "no such property" codes rather than a single sentinel. The
    // library's own smoke test treats any non-OK as "absent". Match that:
    // INVALID_SAMPLE_DESCRIPTION_INDEX (20) -- sdi has no entry of this kind,
    // NOT_APPLICABLE (27)               -- entry exists, no such property,
    // UNINITIALIZED (35)                -- sample-table not built (no samples).
    function isPropertyAbsent(code) {
        return code === 20 || code === 27 || code === 35;
    }

    /** Clean-aperture ('clap') from a track sample-description entry.
     *  `sampleDescriptionIndex` is 1-based per ISOBMFF (NOT a sample id).
     *  Returns `null` if the sample-description entry has no clap. */
    HEIFReader.prototype.trackSampleClap = function (trackId, sampleDescriptionIndex) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const p = m._malloc(36);
        try {
            m.HEAPU32[p >> 2] = 36;  // struct_size
            const code = fn.getTrackSampleClap(this._handle, trackId,
                                               sampleDescriptionIndex, p);
            if (isPropertyAbsent(code)) {
                this._heif._fn.clearError();
                return null;
            }
            this._heif._check(code, 'heif_reader_get_track_sample_clap');
            return readClap(m, p);
        } finally {
            m._free(p);
        }
    };

    /** Auxiliary-type-info ('auxi') from a track sample-description entry.
     *  Returns `{type, subtype}` where `type` is the NUL-terminated string
     *  (e.g. "urn:mpeg:mpegB:cicp:systems:auxiliary:alpha") and `subtype`
     *  is a Uint8Array of raw bytes. Returns `null` if the entry has no
     *  auxi. `sampleDescriptionIndex` is 1-based per ISOBMFF. */
    HEIFReader.prototype.trackSampleAuxi = function (trackId, sampleDescriptionIndex) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const args = [this._handle, trackId, sampleDescriptionIndex];
        // Probe the type once to detect "absent" before allocating.
        const sizePtr = m._malloc(8);
        try {
            m.HEAPU32[(sizePtr >> 2) + 0] = 0;
            m.HEAPU32[(sizePtr >> 2) + 1] = 0;
            const probe = fn.getTrackSampleAuxiTy.apply(null, args.concat([0, sizePtr]));
            if (isPropertyAbsent(probe)) {
                this._heif._fn.clearError();
                return null;
            }
            if (probe !== 0 && !isProbeShort(probe)) {
                this._heif._check(probe, 'heif_reader_get_track_sample_auxi_type (probe)');
            }
        } finally {
            m._free(sizePtr);
        }
        const typeBytes = fetchSized(this._heif, fn.getTrackSampleAuxiTy, args,
                                     'heif_reader_get_track_sample_auxi_type');
        const subBytes  = fetchSized(this._heif, fn.getTrackSampleAuxiSub, args,
                                     'heif_reader_get_track_sample_auxi_subtype');
        // The type buffer is NUL-terminated per the header; trim the
        // trailing NUL before stringifying so the JS string matches the
        // C-side `strlen()`.
        let typeLen = typeBytes.length;
        while (typeLen > 0 && typeBytes[typeLen - 1] === 0) { typeLen--; }
        const decoder = (typeof TextDecoder !== 'undefined')
                        ? new TextDecoder('utf-8')
                        : null;
        const type = decoder
                     ? decoder.decode(typeBytes.subarray(0, typeLen))
                     : String.fromCharCode.apply(null, typeBytes.subarray(0, typeLen));
        return { type: type, subtype: subBytes };
    };

    // Per-sample decoder config + decoding-order + dependencies

    /** Decoder code-type FourCC for a single sequence sample (e.g.
     *  `"hvc1"` vs `"hev1"`). May differ between samples in a track
     *  that switches mid-stream; call per sample if dispatching on it. */
    HEIFReader.prototype.trackSampleDecoderCodeType = function (trackId, sampleId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const buf = m._malloc(5);
        try {
            this._heif._check(
                fn.getTrackSampleCodeTy(this._handle, trackId, sampleId, buf),
                'heif_reader_get_track_sample_decoder_code_type');
            return m.UTF8ToString(buf);
        } finally {
            m._free(buf);
        }
    };

    /** Decoder parameter sets for one sample, as
     *  `[{type, data}, ...]` where `type` is the
     *  `HEIF::DecoderSpecInfoType` numeric value (HEVC_VPS=32, SPS=33,
     *  PPS=34; AVC_SPS=7, PPS=8; ...) and `data` is a Uint8Array of the
     *  parameter-set payload bytes. Use this to feed parameter sets
     *  out-of-band, e.g. WebCodecs `VideoDecoder.configure({description:
     *  ...})`, instead of relying on the inline-prefix bitstream from
     *  `trackSampleBitstreamWithParameters`. */
    HEIFReader.prototype.trackSampleDecoderParamSets = function (trackId, sampleId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let count;
        try {
            this._heif._check(
                fn.getTrackSamplePsCount(this._handle, trackId, sampleId, cnt),
                'heif_reader_get_track_sample_decoder_param_set_count');
            count = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const tyOut = m._malloc(1);
        const result = new Array(count);
        try {
            for (let i = 0; i < count; i++) {
                this._heif._check(
                    fn.getTrackSamplePsTypeAt(this._handle, trackId, sampleId, i, tyOut),
                    'heif_reader_get_track_sample_decoder_param_set_type_at');
                const data = fetchSized(this._heif, fn.getTrackSamplePsDataAt,
                                        [this._handle, trackId, sampleId, i],
                                        'heif_reader_get_track_sample_decoder_param_set_data_at');
                result[i] = { type: m.HEAPU8[tyOut], data: data };
            }
        } finally {
            m._free(tyOut);
        }
        return result;
    };

    /** Decoding-order list for a sequence track: `[{timestampMs,
     *  sampleId}, ...]` in the order samples must be **decoded** (which
     *  differs from presentation order whenever B-frames or non-zero
     *  composition offsets exist). Mirrors
     *  `HEIF::Reader::getItemsInDecodingOrder`. */
    HEIFReader.prototype.trackDecodingOrder = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let count;
        try {
            this._heif._check(fn.getTrackDecOrderCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_decoding_order_count');
            count = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const tsLo = m._malloc(8);
        const sid  = m._malloc(4);
        const result = new Array(count);
        try {
            for (let i = 0; i < count; i++) {
                this._heif._check(
                    fn.getTrackDecOrderAt(this._handle, trackId, i, tsLo, sid),
                    'heif_reader_get_track_decoding_order_at');
                const lo = m.HEAPU32[tsLo >> 2];
                const hi = m.HEAP32 [(tsLo + 4) >> 2];
                result[i] = {
                    timestampMs: lo + hi * 0x100000000,
                    sampleId:    m.HEAPU32[sid >> 2],
                };
            }
        } finally {
            m._free(tsLo); m._free(sid);
        }
        return result;
    };

    /** Decode-dependency list for one sample: sample ids whose decoded
     *  output must be available before this sample can be decoded. Used
     *  for seek/scrub -- after seeking to sample N, feed every sample
     *  reported here into the decoder before sample N itself. Mirrors
     *  `Reader::getDecodeDependencies` for sequences. */
    HEIFReader.prototype.trackSampleDecodeDependencies = function (trackId, sampleId) {
        const fn = this._heif._fn;
        return collectU32(this._heif, fn.getTrackDepCount, fn.getTrackDepAt,
                          this._handle, trackId, sampleId);
    };

    // Track sample sub-structures

    /** 4cc item type for a track sample (e.g. "hvc1", "vvc1"). */
    HEIFReader.prototype.trackSampleItemType = function (trackId, sampleId) {
        const m = this._heif._m;
        const buf = m._malloc(5);
        try {
            this._heif._check(
                this._heif._fn.getTrackSampleItemType(this._handle, trackId, sampleId, buf),
                'heif_reader_get_track_sample_item_type');
            return m.UTF8ToString(buf);
        } finally {
            m._free(buf);
        }
    };

    /** eqiv -- visual equivalence sample grouping entries.
     *  [{sampleGroupDescriptionIndex, timeOffset, timescaleMultiplier}]. */
    HEIFReader.prototype.trackSampleEqiv = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let n;
        try {
            this._heif._check(fn.getTrackEqivCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_sample_eqiv_count');
            n = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const S = 12;  // heif_sample_eqiv_t = 12 bytes
        const p = m._malloc(S);
        const result = new Array(n);
        try {
            for (var i = 0; i < n; i++) {
                m.HEAPU32[p >> 2] = S;
                this._heif._check(fn.getTrackEqivAt(this._handle, trackId, i, p),
                                  'heif_reader_get_track_sample_eqiv_at');
                result[i] = {
                    sampleGroupDescriptionIndex: m.HEAPU32[(p >> 2) + 1],
                    timeOffset:                  m.HEAP16[(p >> 1) + 4],
                    timescaleMultiplier:         m.HEAPU16[(p >> 1) + 5],
                };
            }
        } finally {
            m._free(p);
        }
        return result;
    };

    /** stmi -- sample-to-metadata-item grouping.
     *  [{sampleGroupDescriptionIndex, metadataItemIds[]}]. */
    HEIFReader.prototype.trackSampleStmi = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let n;
        try {
            this._heif._check(fn.getTrackStmiCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_sample_stmi_count');
            n = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const S = 8;   // heif_sample_stmi_t = 8 bytes
        const p = m._malloc(S);
        const result = new Array(n);
        try {
            for (var i = 0; i < n; i++) {
                m.HEAPU32[p >> 2] = S;
                this._heif._check(fn.getTrackStmiAt(this._handle, trackId, i, p),
                                  'heif_reader_get_track_sample_stmi_at');
                var sgdi = m.HEAPU32[(p >> 2) + 1];
                // Nested: metadata item ids
                var metaIds = collectU32(this._heif,
                    fn.getTrackStmiMetaCount, fn.getTrackStmiMetaAt,
                    this._handle, trackId, i);
                result[i] = {
                    sampleGroupDescriptionIndex: sgdi,
                    metadataItemIds: Array.from(metaIds),
                };
            }
        } finally {
            m._free(p);
        }
        return result;
    };

    /** refs -- reference sample grouping.
     *  [{sampleGroupDescriptionIndex, sampleId, referenceItemIds[]}]. */
    HEIFReader.prototype.trackSampleRefs = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let n;
        try {
            this._heif._check(fn.getTrackRefsCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_sample_refs_count');
            n = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const S = 16;   // heif_sample_refs_t = 16 bytes
        const p = m._malloc(S);
        const result = new Array(n);
        try {
            for (var i = 0; i < n; i++) {
                m.HEAPU32[p >> 2] = S;
                this._heif._check(fn.getTrackRefsAt(this._handle, trackId, i, p),
                                  'heif_reader_get_track_sample_refs_at');
                var refIds = collectU32(this._heif,
                    fn.getTrackRefsRefItemCount, fn.getTrackRefsRefItemAt,
                    this._handle, trackId, i);
                result[i] = {
                    sampleGroupDescriptionIndex: m.HEAPU32[(p >> 2) + 1],
                    sampleId:                    m.HEAPU32[(p >> 2) + 2],
                    referenceItemIds:            Array.from(refIds),
                };
            }
        } finally {
            m._free(p);
        }
        return result;
    };

    /** trif -- rectangular region (tile) sample grouping.
     *  Each entry carries region geometry + a dependency list. */
    HEIFReader.prototype.trackSampleTrif = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let n;
        try {
            this._heif._check(fn.getTrackTrifCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_sample_trif_count');
            n = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        // heif_sample_trif_t layout (wasm32):
        // u32 struct_size | u32 sgdi | u16 group_id | u16 independent_idc |
        // u16 h_off | u16 v_off | u16 rw | u16 rh |
        // u8 rect_region_flag | u8 full_picture | u8 filtering_disabled | u8 pad
        const S = 24;
        const p = m._malloc(S);
        const result = new Array(n);
        try {
            for (var i = 0; i < n; i++) {
                m.HEAPU32[p >> 2] = S;
                this._heif._check(fn.getTrackTrifAt(this._handle, trackId, i, p),
                                  'heif_reader_get_track_sample_trif_at');
                var deps = collectU32(this._heif,
                    fn.getTrackTrifDepCount, fn.getTrackTrifDepAt,
                    this._handle, trackId, i);
                var h = p >> 1;
                result[i] = {
                    sampleGroupDescriptionIndex: m.HEAPU32[(p >> 2) + 1],
                    groupId:           m.HEAPU16[h + 4],
                    independentIdc:    m.HEAPU16[h + 5],
                    horizontalOffset:  m.HEAPU16[h + 6],
                    verticalOffset:    m.HEAPU16[h + 7],
                    regionWidth:       m.HEAPU16[h + 8],
                    regionHeight:      m.HEAPU16[h + 9],
                    rectRegionFlag:    m.HEAPU8[p + 20] !== 0,
                    fullPicture:       m.HEAPU8[p + 21] !== 0,
                    filteringDisabled: m.HEAPU8[p + 22] !== 0,
                    dependencies:      Array.from(deps),
                };
            }
        } finally {
            m._free(p);
        }
        return result;
    };

    /** spor -- sub-picture order sample grouping.
     *  Each entry carries VVC sub-picture parameters + sub-track ref indices. */
    HEIFReader.prototype.trackSampleSpor = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let n;
        try {
            this._heif._check(fn.getTrackSporCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_sample_spor_count');
            n = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const S = 16;  // heif_sample_spor_t = 16 bytes
        const p = m._malloc(S);
        const result = new Array(n);
        try {
            for (var i = 0; i < n; i++) {
                m.HEAPU32[p >> 2] = S;
                this._heif._check(fn.getTrackSporAt(this._handle, trackId, i, p),
                                  'heif_reader_get_track_sample_spor_at');
                var subpIdxs = collectU32(this._heif,
                    fn.getTrackSporSubpCount, fn.getTrackSporSubpAt,
                    this._handle, trackId, i);
                result[i] = {
                    sampleGroupDescriptionIndex: m.HEAPU32[(p >> 2) + 1],
                    subpicIdBitPos:        m.HEAPU16[(p >> 1) + 4],
                    subpicIdInfoFlag:      m.HEAPU8[p + 10] !== 0,
                    subpicdLenMinus1:      m.HEAPU8[p + 11],
                    startCodeEmulFlag:     m.HEAPU8[p + 12] !== 0,
                    ppsSpsSubpicIdFlag:    m.HEAPU8[p + 13] !== 0,
                    ppsId:                 m.HEAPU8[p + 14],
                    spsId:                 m.HEAPU8[p + 15],
                    subpTrackRefIdxs:      Array.from(subpIdxs),
                };
            }
        } finally {
            m._free(p);
        }
        return result;
    };

    /** sulm -- sub-layer multiview sample grouping.
     *  Each entry carries a group_id FourCC + nested group id list. */
    HEIFReader.prototype.trackSampleSulm = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let n;
        try {
            this._heif._check(fn.getTrackSulmCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_sample_sulm_count');
            n = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const S = 16;  // heif_sample_sulm_t = 16 bytes
        const p = m._malloc(S);
        const result = new Array(n);
        try {
            for (var i = 0; i < n; i++) {
                m.HEAPU32[p >> 2] = S;
                this._heif._check(fn.getTrackSulmAt(this._handle, trackId, i, p),
                                  'heif_reader_get_track_sample_sulm_at');
                var gids = collectU32(this._heif,
                    fn.getTrackSulmGidCount, fn.getTrackSulmGidAt,
                    this._handle, trackId, i);
                // FourCC at p+8..p+12 (char[5] with NUL at [4])
                result[i] = {
                    sampleGroupDescriptionIndex: m.HEAPU32[(p >> 2) + 1],
                    groupIdInfo4cc: m.UTF8ToString(p + 8),
                    groupIds: Array.from(gids),
                };
            }
        } finally {
            m._free(p);
        }
        return result;
    };

    /** minp -- mixed NALU-unit-type picture sample grouping.
     *  Each entry carries PPS info + two sub-picture track index arrays. */
    HEIFReader.prototype.trackSampleMinp = function (trackId) {
        const m = this._heif._m;
        const fn = this._heif._fn;
        const cnt = m._malloc(4);
        let n;
        try {
            this._heif._check(fn.getTrackMinpCount(this._handle, trackId, cnt),
                              'heif_reader_get_track_sample_minp_count');
            n = m.HEAPU32[cnt >> 2];
        } finally {
            m._free(cnt);
        }
        const S = 12;  // heif_sample_minp_t = 12 bytes
        const p = m._malloc(S);
        const result = new Array(n);
        try {
            for (var i = 0; i < n; i++) {
                m.HEAPU32[p >> 2] = S;
                this._heif._check(fn.getTrackMinpAt(this._handle, trackId, i, p),
                                  'heif_reader_get_track_sample_minp_at');
                var idx1 = collectU32(this._heif,
                    fn.getTrackMinpIdx1Count, fn.getTrackMinpIdx1At,
                    this._handle, trackId, i);
                var idx2 = collectU32(this._heif,
                    fn.getTrackMinpIdx2Count, fn.getTrackMinpIdx2At,
                    this._handle, trackId, i);
                result[i] = {
                    sampleGroupDescriptionIndex: m.HEAPU32[(p >> 2) + 1],
                    ppsMixNaluTypesInPicBitPos:  m.HEAPU16[(p >> 1) + 4],
                    ppsId:                       m.HEAPU8[p + 10],
                    mixSubpTrackIdx1:            Array.from(idx1),
                    mixSubpTrackIdx2:            Array.from(idx2),
                };
            }
        } finally {
            m._free(p);
        }
        return result;
    };

    // DASH / segmented parsing

    // Internal: create a heif_stream_t* from a Uint8Array, using
    // _buildStreamCallbacks + heif_stream_create. Returns
    // {streamPtr, callbackPtrs[], userDataPtr, stagedBytesPtr} so
    // the caller can manage lifetime.
    function _createStream(heif, bytes) {
        var bag = _buildStreamCallbacks(heif, bytes);
        var streamPtr = heif._fn.streamCreate(bag.tblPtr);
        heif._m._free(bag.tblPtr);

        if (streamPtr === 0) {
            _cleanupCallbacks(heif, bag);
            throw new Error('heif_stream_create returned NULL');
        }
        return { streamPtr: streamPtr, callbackPtrs: bag.callbackPtrs, userDataPtr: bag.userDataPtr, stagedBytesPtr: bag.stagedBytesPtr };
    }

    // Internal: tear down a stream created by _createStream.
    function _destroyStream(heif, s) {
        var m = heif._m;
        heif._fn.streamDestroy(s.streamPtr);
        for (var i = 0; i < s.callbackPtrs.length; i++) m.removeFunction(s.callbackPtrs[i]);
        m._free(s.userDataPtr);
        m._free(s.stagedBytesPtr);
    }

    /** Open a file from a filesystem path (wasm sandboxed FS only).
     *  For in-memory bytes, use heif.open(bytes) instead. */
    HEIFReader.prototype.openFile = function (path) {
        this._heif._check(
            this._heif._fn.initFile(this._handle, path),
            'heif_reader_initialize_file');
    };

    /** Parse a DASH initialization segment from in-memory bytes.
     *  The reader retains the stream -- do not reuse the bytes buffer
     *  until the reader is destroyed. */
    HEIFReader.prototype.parseInitSegment = function (bytes) {
        var s = _createStream(this._heif, bytes);
        try {
            this._heif._check(
                this._heif._fn.parseInitSeg(this._handle, s.streamPtr),
                'heif_reader_parse_initialization_segment');
        } catch (e) {
            _destroyStream(this._heif, s);
            throw e;
        }
        // Library retains the stream until reader destroy -- track for cleanup
        if (!this._dashStreams) this._dashStreams = [];
        this._dashStreams.push(s);
    };

    /** Parse a DASH media segment from in-memory bytes.
     *  segmentId must be unique across all parsed segments.
     *  earliestPts is the earliest presentation timestamp in
     *  timescale units. */
    HEIFReader.prototype.parseSegment = function (bytes, segmentId, earliestPts) {
        var s = _createStream(this._heif, bytes);
        try {
            this._heif._check(
                this._heif._fn.parseSegment(this._handle, s.streamPtr, segmentId, BigInt(earliestPts)),
                'heif_reader_parse_segment');
        } catch (e) {
            _destroyStream(this._heif, s);
            throw e;
        }
        // Library retains the stream until invalidateSegment
        if (!this._dashStreams) this._dashStreams = [];
        this._dashStreams.push({ segmentId: segmentId, stream: s });
    };

    /** Invalidate a previously-parsed media segment, releasing its
     *  stream resources. */
    HEIFReader.prototype.invalidateSegment = function (segmentId) {
        this._heif._check(
            this._heif._fn.invalidateSegment(this._handle, segmentId),
            'heif_reader_invalidate_segment');
        // Release the stream associated with this segment
        if (this._dashStreams) {
            for (var i = this._dashStreams.length - 1; i >= 0; i--) {
                var entry = this._dashStreams[i];
                if (entry.segmentId === segmentId) {
                    _destroyStream(this._heif, entry.stream);
                    this._dashStreams.splice(i, 1);
                    break;
                }
            }
        }
    };

    /** Parse a DASH segment index (sidx box) and return the segment
     *  info array.  The stream is consumed and released immediately. */
    HEIFReader.prototype.parseSegmentIndex = function (bytes) {
        var m = this._heif._m;
        var fn = this._heif._fn;
        var s = _createStream(this._heif, bytes);
        try {
            // count_inout probe: pass 0 to learn how many entries
            var cntPtr = m._malloc(4);
            try {
                m.HEAPU32[cntPtr >> 2] = 0;
                var code = fn.parseSegmentIndex(this._handle, s.streamPtr, 0, cntPtr);
                // Accept BUFFER_SIZE_TOO_SMALL as a probe response
                if (code !== 0 && !isProbeShort(code)) {
                    this._heif._check(code, 'heif_reader_parse_segment_index (probe)');
                }
                var count = m.HEAPU32[cntPtr >> 2];
                if (count === 0) return [];

                // Allocate array of heif_segment_info_t (48 bytes each)
                var SEG_SIZE = 48;
                var arr = m._malloc(SEG_SIZE * count);
                try {
                    // Set struct_size on each entry
                    for (var i = 0; i < count; i++) {
                        m.HEAPU32[(arr + i * SEG_SIZE) >> 2] = SEG_SIZE;
                    }
                    m.HEAPU32[cntPtr >> 2] = count;
                    this._heif._check(
                        fn.parseSegmentIndex(this._handle, s.streamPtr, arr, cntPtr),
                        'heif_reader_parse_segment_index');
                    var result = new Array(count);
                    for (var i = 0; i < count; i++) {
                        var base = arr + i * SEG_SIZE;
                        var w = base >> 2;
                        result[i] = {
                            segmentId:        m.HEAPU32[w + 1],
                            earliestPtsInTs:  m.HEAPU32[w + 2] + m.HEAPU32[w + 3] * 0x100000000,
                            startDataOffset:  m.HEAPU32[w + 4] + m.HEAPU32[w + 5] * 0x100000000,
                            referenceId:      m.HEAPU32[w + 6],
                            timescale:        m.HEAPU32[w + 7],
                            durationInTs:     m.HEAPU32[w + 8],
                            dataSize:         m.HEAPU32[w + 9],
                            referenceType:    m.HEAPU8[base + 40],
                        };
                    }
                    return result;
                } finally {
                    m._free(arr);
                }
            } finally {
                m._free(cntPtr);
            }
        } finally {
            _destroyStream(this._heif, s);
        }
    };

    // Sample flags
    // Pure JS replicas of the static-inline C bit-extractors from
    // heifreader_c.h.  No cwrap needed -- these are just bit-shifts.

    /** @namespace HEIF.sampleFlags */
    var sampleFlags = {
        isLeading:                 function (f) { return (f >>> 26) & 0x3; },
        sampleDependsOn:           function (f) { return (f >>> 24) & 0x3; },
        sampleIsDependedOn:        function (f) { return (f >>> 22) & 0x3; },
        sampleHasRedundancy:       function (f) { return (f >>> 20) & 0x3; },
        samplePaddingValue:        function (f) { return (f >>> 17) & 0x7; },
        sampleIsNonSyncSample:     function (f) { return (f >>> 16) & 0x1; },
        sampleDegradationPriority: function (f) { return f & 0xFFFF; },
    };

    // HEIFError
    function HEIFError(code, codeName, originName, line, message, what) {
        const detail = '[' + originName + ':' + line + '] ' +
                       codeName + (message ? ' -- ' + message : '');
        const summary = (what ? what + ': ' : '') + detail;
        const err = new Error(summary);
        err.name = 'HEIFError';
        err.code = code;
        err.codeName = codeName;
        err.origin = originName;
        err.line = line;
        err.detail = message;
        return err;
    }

    // Public surface
    const HEIFNamespace = {
        load: load,
        HEIF: HEIF,
        HEIFReader: HEIFReader,
        HEIFError: HEIFError,
        sampleFlags: sampleFlags,
    };

    if (typeof module !== 'undefined' && module.exports) {
        module.exports = HEIFNamespace;
    }
    root.HEIF = HEIFNamespace;

})(typeof globalThis !== 'undefined' ? globalThis : this);
