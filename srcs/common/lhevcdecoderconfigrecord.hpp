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

#ifndef LHEVCDECODERCONFIGRECORD_HPP
#define LHEVCDECODERCONFIGRECORD_HPP

#include "customallocator.hpp"
#include "decoderconfigrecord.hpp"
#include "hevccommondefs.hpp"

#include <cstdint>

namespace ISOBMFF
{
    class BitStream;
}

/// L-HEVC decoder configuration record per ISO/IEC 14496-15 Section 9.4.
/// Carries the enhancement-layer parameter sets for an `'lhv1'` image item.
/// The base-layer profile/tier/level, chroma_format, bit_depth and frame-rate
/// information are NOT in this record -- they are conveyed by the associated
/// `'oinf'` property and by the base-layer `'hvcC'`. The trailing
/// numOfArrays / array_completeness / NAL_unit_type / numNalus / nalUnit
/// table mirrors the HEVC variant and typically holds the enhancement-layer
/// VPS / SPS / PPS NAL units.
class LhevcDecoderConfigurationRecord : public DecoderConfigurationRecord
{
public:
    LhevcDecoderConfigurationRecord();
    ~LhevcDecoderConfigurationRecord() override = default;

    /// Add NAL unit to the NAL unit array.
    void addNalUnit(const Vector<std::uint8_t>& nalUnit, HevcNalUnitType nalUnitType, bool arrayCompleteness);

    /// Serialize to bit stream.
    void writeDecConfigRecord(ISOBMFF::BitStream& bitstr) const override;

    /// Deserialize from bit stream.
    void parseConfig(ISOBMFF::BitStream& bitstr);

    /// Append one parameter set of the requested NAL type to byteStream prefixed with a 0x00000001 start code.
    /// If the array does not contain a NAL of the requested type nothing is appended.
    void getOneParameterSet(Vector<std::uint8_t>& byteStream, HevcNalUnitType nalUnitType) const;

    /// Returns configuration parameter map (VPS / SPS / PPS) for this record.
    void getConfigurationMap(ConfigurationMap& aMap, const std::uint16_t& nuh_layer_id = 0) const override;

    std::uint8_t  getConfigurationVersion() const;
    void          setConfigurationVersion(std::uint8_t version);
    std::uint16_t getMinSpatialSegmentationIdc() const;
    void          setMinSpatialSegmentationIdc(std::uint16_t idc);
    std::uint8_t  getParallelismType() const;
    void          setParallelismType(std::uint8_t type);
    std::uint8_t  getNumTemporalLayers() const;
    void          setNumTemporalLayers(std::uint8_t numLayers);
    std::uint8_t  getTemporalIdNested() const;
    void          setTemporalIdNested(std::uint8_t nested);
    std::uint8_t  getLengthSizeMinus1() const;
    void          setLengthSizeMinus1(std::uint8_t lengthSizeMinus1);

private:
    struct NALArray
    {
        bool                          arrayCompleteness = false;
        HevcNalUnitType               nalUnitType       = HevcNalUnitType::INVALID;
        Vector<Vector<std::uint8_t>>  nalList;
    };

    std::uint8_t  mConfigurationVersion{1};
    std::uint16_t mMinSpatialSegmentationIdc{0};
    std::uint8_t  mParallelismType{0};
    std::uint8_t  mNumTemporalLayers{0};
    std::uint8_t  mTemporalIdNested{0};
    std::uint8_t  mLengthSizeMinus1{3};
    Vector<NALArray> mNalArray;
};

#endif  // LHEVCDECODERCONFIGRECORD_HPP
