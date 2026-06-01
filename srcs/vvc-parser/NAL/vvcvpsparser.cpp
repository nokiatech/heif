#include "vvcvpsparser.hpp"
#include "vvcnaluhelper.hpp"
#include "heiferrorinfo.h"
#include "../vvcparser.hpp"
#include "heifcommondatatypes.h"
#include "vvcrbspreader.hpp"
#include <exception>
#include <stddef.h>
#include <vector>

// C6262: MSVC /analyze reports ~60 KB stack.  VPS parsing requires
// several fixed-size arrays (up to 64 OLS × 64 layers per VVC spec
// Table A.2) as local variables.  These are spec-mandated dimensions
// and cannot be heap-allocated without losing the direct correspondence
// to the VVC syntax element parse loop.  60 KB is well within the
// 1 MB default stack.
#if defined(_MSC_VER)
#pragma warning(suppress : 6262)
#endif
VvcParser::VpsInfo VvcVpsParser::parse(const Vector<uint8_t>& vpsNalu) {
    VvcParser::VpsInfo info = {};
    
    VvcRbspReader reader;
    if (!reader.prepare(vpsNalu)) {
        HEIF_RECORD(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                    ::HEIF::ErrorOrigin::Parser_VvcRbsp,
                    "VvcVpsParser::parse: VvcRbspReader::prepare rejected VPS NAL (too short or NAL header invalid)");
        return info;  // Error case
    }

    try
    {

    // video_parameter_set_rbsp()  -  ITU-T H.266 7.3.2.2

    uint8_t vps_video_parameter_set_id = static_cast<uint8_t>(reader.readBits(4));       // u(4)
    uint8_t vps_max_layers_minus1      = static_cast<uint8_t>(reader.readBits(6));       // u(6)
    uint8_t vps_max_sublayers_minus1   = static_cast<uint8_t>(reader.readBits(3));       // u(3)

    info.mVpsId          = vps_video_parameter_set_id;
    info.mMaxLayers      = vps_max_layers_minus1 + 1;
    info.mVpsMaxSubLayers = vps_max_sublayers_minus1 + 1;

    // --- vps_default_ptl_dpb_hrd_max_tid_flag ---
    bool vps_default_ptl_dpb_hrd_max_tid_flag = true;  // inferred when not present
    if (vps_max_layers_minus1 > 0 && vps_max_sublayers_minus1 > 0) {
        vps_default_ptl_dpb_hrd_max_tid_flag = reader.readFlag();
    }
    info.mVpsDefaultPtlDpbHrdMaxTidFlag = vps_default_ptl_dpb_hrd_max_tid_flag;

    // --- vps_all_independent_layers_flag ---
    bool vps_all_independent_layers_flag = true;  // inferred when not present
    if (vps_max_layers_minus1 > 0) {
        vps_all_independent_layers_flag = reader.readFlag();
    }
    info.mVpsAllIndependentLayersFlag = vps_all_independent_layers_flag;

    // --- per-layer loop ---
    Vector<uint8_t> vps_layer_id(vps_max_layers_minus1 + 1, 0);
    Vector<bool> vps_independent_layer_flag(vps_max_layers_minus1 + 1, true);
    Vector<bool> vps_max_tid_ref_present_flag(vps_max_layers_minus1 + 1, false);
    Vector<Vector<bool>> vps_direct_ref_layer_flag(vps_max_layers_minus1 + 1,
                                                    Vector<bool>(vps_max_layers_minus1 + 1, false));
    Vector<Vector<uint8_t>> vps_max_tid_il_ref_pics_plus1(vps_max_layers_minus1 + 1,
                                                           Vector<uint8_t>(vps_max_layers_minus1 + 1,
                                                                           vps_max_sublayers_minus1 + 1));

    for (size_t i = 0; i <= vps_max_layers_minus1; i++) {
        vps_layer_id[i] = static_cast<uint8_t>(reader.readBits(6));                      // u(6)
        info.mVpsLayerId[i] = vps_layer_id[i];

        if (i > 0 && !vps_all_independent_layers_flag) {
            vps_independent_layer_flag[i] = reader.readFlag();     // u(1)
            if (!vps_independent_layer_flag[i]) {
                vps_max_tid_ref_present_flag[i] = reader.readFlag(); // u(1)
                for (size_t j = 0; j < i; j++) {
                    vps_direct_ref_layer_flag[i][j] = reader.readFlag(); // u(1)
                    if (vps_max_tid_ref_present_flag[i] && vps_direct_ref_layer_flag[i][j]) {
                        vps_max_tid_il_ref_pics_plus1[i][j] = static_cast<uint8_t>(reader.readBits(3)); // u(3)
                    }
                }
            }
        }
        info.mVpsIndependentLayerFlag[i] = vps_independent_layer_flag[i];
        for (size_t j = 0; j <= vps_max_layers_minus1; j++) {
            info.mVpsDirectRefLayerFlag[i][j] = vps_direct_ref_layer_flag[i][j];
        }
    }

    // --- OLS signalling ---
    bool vps_each_layer_is_an_ols_flag = true;  // inferred
    uint8_t vps_ols_mode_idc = 2;               // inferred when all_independent and each_layer_is_ols
    uint8_t vps_num_output_layer_sets_minus2 = 0;

    uint8_t vps_num_ptls_minus1 = 0;

    if (vps_max_layers_minus1 > 0) {
        if (vps_all_independent_layers_flag) {
            vps_each_layer_is_an_ols_flag = reader.readFlag();     // u(1)
        }
        info.mVpsEachLayerIsAnOlsFlag = vps_each_layer_is_an_ols_flag;

        if (!vps_each_layer_is_an_ols_flag) {
            if (!vps_all_independent_layers_flag) {
                vps_ols_mode_idc = static_cast<uint8_t>(reader.readBits(2));             // u(2)
            }
            info.mVpsOlsModeIdc = vps_ols_mode_idc;

            if (vps_ols_mode_idc == 2) {
                vps_num_output_layer_sets_minus2 = static_cast<uint8_t>(reader.readBits(8)); // u(8)
                for (size_t i = 1; i <= static_cast<size_t>(vps_num_output_layer_sets_minus2) + 1; i++) {
                    for (size_t j = 0; j <= vps_max_layers_minus1; j++) {
                        info.mVpsOlsOutputLayerFlag[i][j] = reader.readFlag(); // u(1)
                    }
                }
            }
        }
        vps_num_ptls_minus1 = static_cast<uint8_t>(reader.readBits(8));                  // u(8)
    } else {
        info.mVpsEachLayerIsAnOlsFlag = vps_each_layer_is_an_ols_flag;
    }

    // --- Derive TotalNumOlss (ITU-T H.266 7.4.3.2) ---
    uint16_t TotalNumOlss = 0;
    if (vps_each_layer_is_an_ols_flag) {
        TotalNumOlss = vps_max_layers_minus1 + 1;
    } else if (vps_ols_mode_idc == 0 || vps_ols_mode_idc == 1) {
        TotalNumOlss = vps_max_layers_minus1 + 1;
    } else if (vps_ols_mode_idc == 2) {
        TotalNumOlss = vps_num_output_layer_sets_minus2 + 2;
    }
    info.mTotalNumOLSs = TotalNumOlss;

    // Derive LayerIdInOls, NumLayersInOls, NumOutputLayersInOls, and
    // NumMultiLayerOlss per ITU-T H.266 Section 7.4.3.2
    info.mLayerIdInOls.resize(TotalNumOlss);
    info.mNumLayersInOls.resize(TotalNumOlss, 0);
    info.mNumOutputLayersInOls.resize(TotalNumOlss, 0);
    info.mOutputLayerIdInOls.resize(TotalNumOlss);

    // Derive NumDirectRefLayers[i] and DirectRefLayerIdx[i][d] (needed for mode 0/1/2 derivation)
    Vector<uint8_t> NumDirectRefLayers(vps_max_layers_minus1 + 1, 0);
    Vector<Vector<uint8_t>> DirectRefLayerIdx(vps_max_layers_minus1 + 1);
    for (size_t i = 0; i <= vps_max_layers_minus1; i++) {
        DirectRefLayerIdx[i].resize(vps_max_layers_minus1 + 1, 0);
        uint8_t d = 0;
        for (size_t j = 0; j < i; j++) {
            if (vps_direct_ref_layer_flag[i][j]) {
                DirectRefLayerIdx[i][d] = static_cast<uint8_t>(j);
                d++;
            }
        }
        NumDirectRefLayers[i] = d;
    }

    // Derive GeneralLayerIdx: mapping from layer_id to layer index
    for (size_t i = 0; i <= vps_max_layers_minus1; i++) {
        info.mGeneralLayerIdx[vps_layer_id[i]] = static_cast<uint32_t>(i);
    }

    uint16_t NumMultiLayerOlss = 0;

    if (vps_each_layer_is_an_ols_flag) {
        // Each layer is its own OLS
        for (uint16_t i = 0; i < TotalNumOlss; i++) {
            info.mNumLayersInOls[i] = 1;
            info.mLayerIdInOls[i].push_back(vps_layer_id[i]);
            info.mNumOutputLayersInOls[i] = 1;
            info.mOutputLayerIdInOls[i].push_back(vps_layer_id[i]);
        }
        NumMultiLayerOlss = 0;
    } else if (vps_ols_mode_idc == 0 || vps_ols_mode_idc == 1) {
        // Mode 0: each OLS i contains layers 0..i, all are output layers
        // Mode 1: each OLS i contains layers 0..i, only highest layer is output
        for (uint16_t olsIdx = 0; olsIdx < TotalNumOlss; olsIdx++) {
            info.mNumLayersInOls[olsIdx] = static_cast<uint8_t>(olsIdx + 1);
            for (uint16_t j = 0; j <= olsIdx; j++) {
                info.mLayerIdInOls[olsIdx].push_back(vps_layer_id[j]);
            }
            if (vps_ols_mode_idc == 0) {
                // All layers are output layers
                info.mNumOutputLayersInOls[olsIdx] = static_cast<uint8_t>(olsIdx + 1);
                for (uint16_t j = 0; j <= olsIdx; j++) {
                    info.mOutputLayerIdInOls[olsIdx].push_back(vps_layer_id[j]);
                }
            } else {
                // Only the highest layer is an output layer
                info.mNumOutputLayersInOls[olsIdx] = 1;
                info.mOutputLayerIdInOls[olsIdx].push_back(vps_layer_id[olsIdx]);
            }
        }
        NumMultiLayerOlss = vps_max_layers_minus1;  // OLSs 1..MaxLayers-1
    } else {  // vps_ols_mode_idc == 2
        // Explicitly signalled output layer sets.
        // OLS 0 always contains only layer 0 (the base layer).
        info.mNumOutputLayersInOls[0] = 1;
        info.mOutputLayerIdInOls[0].push_back(vps_layer_id[0]);
        info.mLayerIdInOls[0].push_back(vps_layer_id[0]);
        info.mNumLayersInOls[0] = 1;

        for (uint16_t olsIdx = 1; olsIdx < TotalNumOlss; olsIdx++) {
            // Determine output layers from the signalled flags
            // and include all reference layers recursively.
            Vector<bool> layerIncluded(vps_max_layers_minus1 + 1, false);

            // First pass: mark output layers
            uint8_t numOutputLayers = 0;
            for (size_t j = 0; j <= vps_max_layers_minus1; j++) {
                if (info.mVpsOlsOutputLayerFlag[olsIdx][j]) {
                    layerIncluded[j] = true;
                    info.mOutputLayerIdInOls[olsIdx].push_back(vps_layer_id[j]);
                    numOutputLayers++;
                }
            }
            info.mNumOutputLayersInOls[olsIdx] = numOutputLayers;

            // Second pass: for each included layer, recursively include
            // its direct reference layers (iterate from highest to lowest
            // to catch transitive dependencies)
            for (uint8_t j = vps_max_layers_minus1 + 1; j-- > 0; ) {
                if (layerIncluded[j]) {
                    for (uint8_t k = 0; k < NumDirectRefLayers[j]; k++) {
                        layerIncluded[DirectRefLayerIdx[j][k]] = true;
                    }
                }
            }

            // Collect all included layers in order
            uint8_t numLayers = 0;
            for (size_t j = 0; j <= vps_max_layers_minus1; j++) {
                if (layerIncluded[j]) {
                    info.mLayerIdInOls[olsIdx].push_back(vps_layer_id[j]);
                    numLayers++;
                }
            }
            info.mNumLayersInOls[olsIdx] = numLayers;
        }

        // Count multi-layer OLSs
        NumMultiLayerOlss = 0;
        for (uint16_t i = 0; i < TotalNumOlss; i++) {
            if (info.mNumLayersInOls[i] > 1) {
                info.mMultiLayerOlsIdx[NumMultiLayerOlss] = static_cast<uint32_t>(i);
                NumMultiLayerOlss++;
            }
        }
    }
    info.mNumMultiLayeredOlss = NumMultiLayerOlss;
    info.mVpsNumOutputLayerSets = static_cast<uint32_t>(TotalNumOlss);

    // --- PTL loop ---
    Vector<uint8_t> vps_ptl_max_tid(vps_num_ptls_minus1 + 1, vps_max_sublayers_minus1);
    Vector<bool> vps_pt_present_flag(vps_num_ptls_minus1 + 1, true);  // i==0 inferred as 1

    for (size_t i = 0; i <= vps_num_ptls_minus1; i++) {
        if (i > 0) {
            vps_pt_present_flag[i] = reader.readFlag();            // u(1)
        }
        if (!vps_default_ptl_dpb_hrd_max_tid_flag) {
            vps_ptl_max_tid[i] = static_cast<uint8_t>(reader.readBits(3));               // u(3)
        }
        info.mPtPresentFlag[i]   = vps_pt_present_flag[i];
        info.mPtlMaxTemporalId[i] = vps_ptl_max_tid[i];
    }

    // --- Byte alignment before profile_tier_level ---
    reader.byteAlign();

    // --- Parse profile_tier_level() for each PTL entry ---
    info.mPtlRecords.reserve(vps_num_ptls_minus1 + 1);
    for (size_t i = 0; i <= vps_num_ptls_minus1; i++) {
        info.mPtlRecords.push_back(VvcNaluHelper::parseProfileTierLevel(
            reader, vps_pt_present_flag[i], vps_ptl_max_tid[i]));
    }
    info.mNativePtl = info.mPtlRecords[0];

    // --- vps_ols_ptl_idx ---
    for (uint16_t i = 0; i < TotalNumOlss; i++) {
        if (vps_num_ptls_minus1 > 0 &&
            static_cast<uint16_t>(vps_num_ptls_minus1 + 1) != TotalNumOlss) {
            info.mOlsPtlIdx[i] = static_cast<uint8_t>(reader.readBits(8));              // u(8)
        }
    }

    // --- DPB parameters (only when not each_layer_is_an_ols) ---
    if (!vps_each_layer_is_an_ols_flag) {
        uint32_t vps_num_dpb_params_minus1 = reader.readUE();      // ue(v)

        bool vps_sublayer_dpb_params_present_flag = false;
        if (vps_max_sublayers_minus1 > 0) {
            vps_sublayer_dpb_params_present_flag = reader.readFlag(); // u(1)
        }
        info.mSublayerDpbParamsPresentFlag = vps_sublayer_dpb_params_present_flag;

        // VpsNumDpbParams (derived, see spec 7.4.3.2)
        uint16_t VpsNumDpbParams = static_cast<uint16_t>(vps_num_dpb_params_minus1 + 1);
        info.mNumDpbParams = VpsNumDpbParams;

        Vector<uint8_t> vps_dpb_max_tid(VpsNumDpbParams, vps_max_sublayers_minus1);
        info.mDpbMaxTemporalId.resize(VpsNumDpbParams);

        for (uint16_t i = 0; i < VpsNumDpbParams; i++) {
            if (!vps_default_ptl_dpb_hrd_max_tid_flag) {
                vps_dpb_max_tid[i] = static_cast<uint8_t>(reader.readBits(3));           // u(3)
            }
            info.mDpbMaxTemporalId[i] = vps_dpb_max_tid[i];

            // dpb_parameters( vps_dpb_max_tid[i], vps_sublayer_dpb_params_present_flag )
            VvcNaluHelper::parseDpbParameters(
                reader, vps_dpb_max_tid[i], vps_sublayer_dpb_params_present_flag);
        }

        // --- Per multi-layer-OLS DPB info ---
        for (uint16_t i = 0; i < NumMultiLayerOlss; i++) {
            /*uint32_t vps_ols_dpb_pic_width  =*/ reader.readUE(); // ue(v)
            /*uint32_t vps_ols_dpb_pic_height =*/ reader.readUE(); // ue(v)
            /*uint8_t  vps_ols_dpb_chroma_fmt =*/ reader.readBits(2); // u(2)
            uint32_t vps_ols_dpb_bitdepth_minus8 = reader.readUE(); // ue(v)
            info.mOlsDpbBitDepthMinus8.push_back(static_cast<uint8_t>(vps_ols_dpb_bitdepth_minus8));

            if (VpsNumDpbParams > 1 && VpsNumDpbParams != NumMultiLayerOlss) {
                uint16_t idx = static_cast<uint16_t>(reader.readUE());       // ue(v)
                info.mOlsDpbParamsIdx.push_back(idx);
            }
        }

        // --- Timing HRD parameters ---
        bool vps_timing_hrd_params_present_flag = reader.readFlag(); // u(1)
        info.mVpsGeneralHrdParamsPresentFlag = vps_timing_hrd_params_present_flag;

        if (vps_timing_hrd_params_present_flag) {
            // general_timing_hrd_parameters()
            auto hrdInfo = VvcNaluHelper::parseGeneralTimingHrdParameters(reader);

            bool vps_sublayer_cpb_params_present_flag = false;
            if (vps_max_sublayers_minus1 > 0) {
                vps_sublayer_cpb_params_present_flag = reader.readFlag(); // u(1)
            }
            info.mVpsSublayerCpbParamsPresentFlag = vps_sublayer_cpb_params_present_flag;

            uint32_t vps_num_ols_timing_hrd_params_minus1 = reader.readUE(); // ue(v)
            info.mNumOlsTimingHrdParamsMinus1 = vps_num_ols_timing_hrd_params_minus1;

            Vector<uint8_t> vps_hrd_max_tid(vps_num_ols_timing_hrd_params_minus1 + 1,
                                             vps_max_sublayers_minus1);
            for (uint32_t i = 0; i <= vps_num_ols_timing_hrd_params_minus1; i++) {
                if (!vps_default_ptl_dpb_hrd_max_tid_flag) {
                    vps_hrd_max_tid[i] = static_cast<uint8_t>(reader.readBits(3));       // u(3)
                }
                info.mHrdMaxTid[i] = vps_hrd_max_tid[i];

                uint32_t firstSubLayer = vps_sublayer_cpb_params_present_flag
                                             ? 0
                                             : vps_hrd_max_tid[i];
                // ols_timing_hrd_parameters( firstSubLayer, vps_hrd_max_tid[i] )
                VvcNaluHelper::skipOlsTimingHrdParameters(
                    reader, firstSubLayer, vps_hrd_max_tid[i], hrdInfo);
            }

            if (vps_num_ols_timing_hrd_params_minus1 > 0 &&
                static_cast<uint16_t>(vps_num_ols_timing_hrd_params_minus1 + 1) != NumMultiLayerOlss) {
                for (uint16_t i = 0; i < NumMultiLayerOlss; i++) {
                    info.mOlsTimingHrdIdx[i] = reader.readUE();   // ue(v)
                }
            }
        }
    }

    // --- vps_extension_flag ---
    info.mVpsExtensionFlag = reader.readFlag();                    // u(1)
    // If extension data is present, skip it (not parsed)
    // rbsp_trailing_bits() - not consumed here

    return info;

    }  // try
    catch (const std::exception& e)
    {
        // Truncated or malformed VPS NALU -- return zeroed info rather than
        // propagating an exception through the Reader API.
        HEIF_RECORD_FROM(::HEIF::ErrorCode::INVALID_FUNCTION_PARAMETER,
                         ::HEIF::ErrorOrigin::Parser_VvcVps,
                         "VvcVpsParser::parse: exception while reading VPS RBSP (truncated or malformed)",
                         e);
        return {};
    }
}
