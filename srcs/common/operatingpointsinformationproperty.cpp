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

#include "operatingpointsinformationproperty.hpp"

#include "bitstream.hpp"
#include "fourccint.hpp"

namespace
{
    // Pop count for the 16-bit scalability_mask. Each set bit contributes one
    // dimension_identifier byte per LayerInfo entry.
    std::uint8_t scalabilityDimensionCount(std::uint16_t mask)
    {
        std::uint8_t count = 0;
        for (std::uint8_t bit = 0; bit < 16; ++bit)
        {
            if ((mask >> bit) & 0x1)
            {
                ++count;
            }
        }
        return count;
    }
}  // namespace

OperatingPointsInformationProperty::OperatingPointsInformationProperty()
    : FullBox("oinf", 0, 0)
    , mScalabilityMask(0)
    , mProfileTierLevels()
    , mOperatingPoints()
    , mLayers()
{
}

std::uint16_t OperatingPointsInformationProperty::getScalabilityMask() const
{
    return mScalabilityMask;
}

void OperatingPointsInformationProperty::setScalabilityMask(const std::uint16_t mask)
{
    mScalabilityMask = mask;
}

const Vector<OperatingPointsInformationProperty::ProfileTierLevel>&
OperatingPointsInformationProperty::getProfileTierLevels() const
{
    return mProfileTierLevels;
}

void OperatingPointsInformationProperty::setProfileTierLevels(const Vector<ProfileTierLevel>& ptls)
{
    mProfileTierLevels = ptls;
}

const Vector<OperatingPointsInformationProperty::OperatingPoint>&
OperatingPointsInformationProperty::getOperatingPoints() const
{
    return mOperatingPoints;
}

void OperatingPointsInformationProperty::setOperatingPoints(const Vector<OperatingPoint>& ops)
{
    mOperatingPoints = ops;
}

const Vector<OperatingPointsInformationProperty::LayerInfo>& OperatingPointsInformationProperty::getLayers() const
{
    return mLayers;
}

void OperatingPointsInformationProperty::setLayers(const Vector<LayerInfo>& layers)
{
    mLayers = layers;
}

void OperatingPointsInformationProperty::writeBox(BitStream& output) const
{
    writeFullBoxHeader(output);

    //bit(16) scalability_mask
    output.write16Bits(mScalabilityMask);
    //bit(2)  reserved + bit(6) num_profile_tier_level
    output.writeBits(0, 2);
    output.writeBits(static_cast<unsigned int>(mProfileTierLevels.size()), 6);

    for (const auto& ptl : mProfileTierLevels)
    {
        //bit(2)  general_profile_space
        output.writeBits(ptl.generalProfileSpace, 2);
        //bit(1)  general_tier_flag
        output.writeBits(ptl.generalTierFlag, 1);
        //bit(5)  general_profile_idc
        output.writeBits(ptl.generalProfileIdc, 5);
        //bit(32) general_profile_compatibility_flags
        output.write32Bits(ptl.generalProfileCompatibilityFlags);
        //bit(48) general_constraint_indicator_flags (high 16 then low 32)
        output.writeBits(static_cast<unsigned int>((ptl.generalConstraintIndicatorFlags >> 32) & 0xFFFFu), 16);
        output.write32Bits(static_cast<std::uint32_t>(ptl.generalConstraintIndicatorFlags & 0xFFFFFFFFu));
        //bit(8)  general_level_idc
        output.write8Bits(ptl.generalLevelIdc);
    }

    //bit(16) num_operating_points
    output.write16Bits(static_cast<std::uint16_t>(mOperatingPoints.size()));
    for (const auto& op : mOperatingPoints)
    {
        //bit(16) output_layer_set_idx
        output.write16Bits(op.outputLayerSetIdx);
        //bit(8)  max_temporal_id
        output.write8Bits(op.maxTemporalId);
        //bit(8)  layer_count
        output.write8Bits(static_cast<std::uint8_t>(op.layers.size()));
        for (const auto& layer : op.layers)
        {
            //bit(8) ptl_idx
            output.write8Bits(layer.ptlIdx);
            //bit(6) layer_id
            output.writeBits(layer.layerId, 6);
            //bit(1) is_outputlayer
            output.writeBits(layer.isOutputLayer ? 1u : 0u, 1);
            //bit(1) is_alternate_outputlayer
            output.writeBits(layer.isAlternateOutputLayer ? 1u : 0u, 1);
        }
        //bit(16) minPicWidth, minPicHeight, maxPicWidth, maxPicHeight
        output.write16Bits(op.minPicWidth);
        output.write16Bits(op.minPicHeight);
        output.write16Bits(op.maxPicWidth);
        output.write16Bits(op.maxPicHeight);
        //bit(2) maxChromaFormat + bit(3) maxBitDepthMinus8 + bit(1) reserved + bit(1) frame_rate_info_flag + bit(1) bit_rate_info_flag
        output.writeBits(op.maxChromaFormat, 2);
        output.writeBits(op.maxBitDepthMinus8, 3);
        output.writeBits(0, 1);
        output.writeBits(op.frameRateInfoFlag ? 1u : 0u, 1);
        output.writeBits(op.bitRateInfoFlag ? 1u : 0u, 1);
        if (op.frameRateInfoFlag)
        {
            //bit(16) avgFrameRate + bit(6) reserved + bit(2) constantFrameRate
            output.write16Bits(op.avgFrameRate);
            output.writeBits(0, 6);
            output.writeBits(op.constantFrameRate, 2);
        }
        if (op.bitRateInfoFlag)
        {
            //bit(32) maxBitRate + bit(32) avgBitRate
            output.write32Bits(op.maxBitRate);
            output.write32Bits(op.avgBitRate);
        }
    }

    //bit(8) max_layer_count
    output.write8Bits(static_cast<std::uint8_t>(mLayers.size()));
    const std::uint8_t dimCount = scalabilityDimensionCount(mScalabilityMask);
    for (const auto& layer : mLayers)
    {
        //bit(8) dependent_layerID
        output.write8Bits(layer.dependentLayerId);
        //bit(8) num_layers_dependent_on
        output.write8Bits(static_cast<std::uint8_t>(layer.dependsOnLayerIds.size()));
        for (const auto depId : layer.dependsOnLayerIds)
        {
            //bit(8) dependent_on_layerID
            output.write8Bits(depId);
        }
        // dimension_identifier loop: one bit(8) per set bit in scalability_mask
        for (std::uint8_t i = 0; i < dimCount; ++i)
        {
            const std::uint8_t value = (i < layer.dimensionIdentifiers.size()) ? layer.dimensionIdentifiers[i] : 0;
            //bit(8) dimension_identifier
            output.write8Bits(value);
        }
    }

    updateSize(output);
}

void OperatingPointsInformationProperty::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);

    //bit(16) scalability_mask
    mScalabilityMask = input.read16Bits();
    //bit(2)  reserved
    input.readBits(2);
    //bit(6)  num_profile_tier_level
    const std::uint8_t numPtl = static_cast<std::uint8_t>(input.readBits(6));

    mProfileTierLevels.clear();
    mProfileTierLevels.reserve(numPtl);
    for (std::uint8_t i = 0; i < numPtl; ++i)
    {
        ProfileTierLevel ptl{};
        //bit(2)  general_profile_space
        ptl.generalProfileSpace = static_cast<std::uint8_t>(input.readBits(2));
        //bit(1)  general_tier_flag
        ptl.generalTierFlag = static_cast<std::uint8_t>(input.readBits(1));
        //bit(5)  general_profile_idc
        ptl.generalProfileIdc = static_cast<std::uint8_t>(input.readBits(5));
        //bit(32) general_profile_compatibility_flags
        ptl.generalProfileCompatibilityFlags = input.read32Bits();
        //bit(48) general_constraint_indicator_flags
        const std::uint64_t hi = input.readBits(16);
        const std::uint64_t lo = input.read32Bits();
        ptl.generalConstraintIndicatorFlags = (hi << 32) | lo;
        //bit(8)  general_level_idc
        ptl.generalLevelIdc = input.read8Bits();
        mProfileTierLevels.push_back(ptl);
    }

    //bit(16) num_operating_points
    const std::uint16_t numOps = input.read16Bits();
    mOperatingPoints.clear();
    mOperatingPoints.reserve(numOps);
    for (std::uint16_t i = 0; i < numOps; ++i)
    {
        OperatingPoint op{};
        //bit(16) output_layer_set_idx
        op.outputLayerSetIdx = input.read16Bits();
        //bit(8)  max_temporal_id
        op.maxTemporalId = input.read8Bits();
        //bit(8)  layer_count
        const std::uint8_t layerCount = input.read8Bits();
        op.layers.reserve(layerCount);
        for (std::uint8_t j = 0; j < layerCount; ++j)
        {
            OperatingPointLayer layer{};
            //bit(8) ptl_idx
            layer.ptlIdx = input.read8Bits();
            //bit(6) layer_id
            layer.layerId = static_cast<std::uint8_t>(input.readBits(6));
            //bit(1) is_outputlayer
            layer.isOutputLayer = (input.readBits(1) != 0);
            //bit(1) is_alternate_outputlayer
            layer.isAlternateOutputLayer = (input.readBits(1) != 0);
            op.layers.push_back(layer);
        }
        //bit(16) minPicWidth / minPicHeight / maxPicWidth / maxPicHeight
        op.minPicWidth  = input.read16Bits();
        op.minPicHeight = input.read16Bits();
        op.maxPicWidth  = input.read16Bits();
        op.maxPicHeight = input.read16Bits();
        //bit(2) maxChromaFormat
        op.maxChromaFormat = static_cast<std::uint8_t>(input.readBits(2));
        //bit(3) maxBitDepthMinus8
        op.maxBitDepthMinus8 = static_cast<std::uint8_t>(input.readBits(3));
        //bit(1) reserved
        input.readBits(1);
        //bit(1) frame_rate_info_flag
        op.frameRateInfoFlag = (input.readBits(1) != 0);
        //bit(1) bit_rate_info_flag
        op.bitRateInfoFlag = (input.readBits(1) != 0);
        if (op.frameRateInfoFlag)
        {
            //bit(16) avgFrameRate
            op.avgFrameRate = input.read16Bits();
            //bit(6)  reserved
            input.readBits(6);
            //bit(2)  constantFrameRate
            op.constantFrameRate = static_cast<std::uint8_t>(input.readBits(2));
        }
        if (op.bitRateInfoFlag)
        {
            //bit(32) maxBitRate
            op.maxBitRate = input.read32Bits();
            //bit(32) avgBitRate
            op.avgBitRate = input.read32Bits();
        }
        mOperatingPoints.push_back(op);
    }

    //bit(8) max_layer_count
    const std::uint8_t maxLayerCount = input.read8Bits();
    const std::uint8_t dimCount      = scalabilityDimensionCount(mScalabilityMask);
    mLayers.clear();
    mLayers.reserve(maxLayerCount);
    for (std::uint8_t i = 0; i < maxLayerCount; ++i)
    {
        LayerInfo layer{};
        //bit(8) dependent_layerID
        layer.dependentLayerId = input.read8Bits();
        //bit(8) num_layers_dependent_on
        const std::uint8_t numDeps = input.read8Bits();
        layer.dependsOnLayerIds.reserve(numDeps);
        for (std::uint8_t j = 0; j < numDeps; ++j)
        {
            //bit(8) dependent_on_layerID
            layer.dependsOnLayerIds.push_back(input.read8Bits());
        }
        layer.dimensionIdentifiers.reserve(dimCount);
        for (std::uint8_t j = 0; j < dimCount; ++j)
        {
            //bit(8) dimension_identifier
            layer.dimensionIdentifiers.push_back(input.read8Bits());
        }
        mLayers.push_back(layer);
    }
}
