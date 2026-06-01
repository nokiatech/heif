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

#ifndef OPERATINGPOINTSINFORMATIONPROPERTY_HPP
#define OPERATINGPOINTSINFORMATIONPROPERTY_HPP

#include <cstdint>

#include "customallocator.hpp"
#include "fullbox.hpp"
#include "bitstream.hpp"
#include <vector>

namespace ISOBMFF
{
    class BitStream;
}

/** Operating Points Information property `'oinf'`
 *
 *  Carried by L-HEVC `'lhv1'` image items. Enumerates the operating points
 *  (output layer sets) that the bitstream can decode to, the profile / tier
 *  / level descriptors those operating points reference, and the inter-layer
 *  dependency graph for each layer. A `'tols'` property selects which
 *  operating point this image item targets via a 16-bit index that matches
 *  one of the enclosed `OperatingPoint::outputLayerSetIdx` values.
 *
 *  Defined by ISO/IEC 14496-15 Section 11.6.2 (record body) and ISO/IEC 23008-12
 *  Section 6.5.34 (HEIF item-property wrapper). Mandatory exactly once on every
 *  `'lhv1'` item.
 */
class OperatingPointsInformationProperty : public FullBox
{
public:
    struct ProfileTierLevel
    {
        std::uint8_t  generalProfileSpace;               //bit(2)
        std::uint8_t  generalTierFlag;                   //bit(1)
        std::uint8_t  generalProfileIdc;                 //bit(5)
        std::uint32_t generalProfileCompatibilityFlags;  //bit(32)
        std::uint64_t generalConstraintIndicatorFlags;   //bit(48), upper 16 bits zero
        std::uint8_t  generalLevelIdc;                   //bit(8)
    };

    struct OperatingPointLayer
    {
        std::uint8_t ptlIdx;                  //bit(8)  index into mProfileTierLevels (1-based per spec)
        std::uint8_t layerId;                 //bit(6)  layer id within the OLS
        bool         isOutputLayer;           //bit(1)
        bool         isAlternateOutputLayer;  //bit(1)
    };

    struct OperatingPoint
    {
        std::uint16_t outputLayerSetIdx;                //bit(16)
        std::uint8_t  maxTemporalId;                    //bit(8)
        Vector<OperatingPointLayer> layers;             //per-layer descriptors, length == bit(8) layer_count
        std::uint16_t minPicWidth;                      //bit(16)
        std::uint16_t minPicHeight;                     //bit(16)
        std::uint16_t maxPicWidth;                      //bit(16)
        std::uint16_t maxPicHeight;                     //bit(16)
        std::uint8_t  maxChromaFormat;                  //bit(2)
        std::uint8_t  maxBitDepthMinus8;                //bit(3)
        bool          frameRateInfoFlag;                //bit(1)
        bool          bitRateInfoFlag;                  //bit(1)
        std::uint16_t avgFrameRate;                     //bit(16) valid only if frameRateInfoFlag
        std::uint8_t  constantFrameRate;                //bit(2)  valid only if frameRateInfoFlag
        std::uint32_t maxBitRate;                       //bit(32) valid only if bitRateInfoFlag
        std::uint32_t avgBitRate;                       //bit(32) valid only if bitRateInfoFlag
    };

    struct LayerInfo
    {
        std::uint8_t          dependentLayerId;     //bit(8)
        Vector<std::uint8_t>  dependsOnLayerIds;    //bit(8) each, length == bit(8) num_layers_dependent_on
        Vector<std::uint8_t>  dimensionIdentifiers; //bit(8) each, one entry per set bit in mScalabilityMask
    };

    OperatingPointsInformationProperty();
    ~OperatingPointsInformationProperty() override = default;

    /** Get / set the 16-bit scalability mask. The set bits identify which
     *  scalability dimensions are present, controlling how many
     *  `dimensionIdentifiers` entries each `LayerInfo` carries. */
    std::uint16_t getScalabilityMask() const;
    void          setScalabilityMask(std::uint16_t mask);

    const Vector<ProfileTierLevel>& getProfileTierLevels() const;
    void                            setProfileTierLevels(const Vector<ProfileTierLevel>& ptls);

    const Vector<OperatingPoint>& getOperatingPoints() const;
    void                          setOperatingPoints(const Vector<OperatingPoint>& ops);

    const Vector<LayerInfo>& getLayers() const;
    void                     setLayers(const Vector<LayerInfo>& layers);

    void parseBox(BitStream& input) override;
    void writeBox(BitStream& output) const override;

private:
    std::uint16_t            mScalabilityMask;
    Vector<ProfileTierLevel> mProfileTierLevels;
    Vector<OperatingPoint>   mOperatingPoints;
    Vector<LayerInfo>        mLayers;
};

#endif  // OPERATINGPOINTSINFORMATIONPROPERTY_HPP
