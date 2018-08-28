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

#ifndef REGIONWISEPACKINGBOX_HPP
#define REGIONWISEPACKINGBOX_HPP

#include <cstdint>
#include <vector>
#include "fullbox.hpp"

/** @brief Defines how multiple videostreams are located to big single video for encoding/decoding
 *  @details Defined in the OMAF standard. **/
class RegionWisePackingBox : public FullBox
{
public:
    RegionWisePackingBox();
    RegionWisePackingBox(const RegionWisePackingBox&);
    virtual ~RegionWisePackingBox() = default;

    struct RectangularRegionWisePacking
    {
        std::uint32_t projRegWidth;
        std::uint32_t projRegHeight;
        std::uint32_t projRegTop;
        std::uint32_t projRegLeft;
        std::uint8_t transformType;
        std::uint16_t packedRegWidth;
        std::uint16_t packedRegHeight;
        std::uint16_t packedRegTop;
        std::uint16_t packedRegLeft;

        std::uint8_t leftGbWidth;
        std::uint8_t rightGbWidth;
        std::uint8_t topGbHeight;
        std::uint8_t bottomGbHeight;
        bool gbNotUsedForPredFlag;
        std::uint8_t gbType0;
        std::uint8_t gbType1;
        std::uint8_t gbType2;
        std::uint8_t gbType3;
    };

    enum class PackingType : std::uint8_t
    {
        RECTANGULAR = 0
    };

    struct Region
    {
        Region(){};
        Region(const Region&);

        bool guardBandFlag;
        PackingType packingType;
        UniquePtr<RectangularRegionWisePacking> rectangularPacking;
        // ... add future packings here ...
    };

    bool getConstituentPictureMatchingFlag() const;
    void setConstituentPictureMatchingFlag(bool);

    std::uint32_t getProjPictureWidth() const;
    void setProjPictureWidth(std::uint32_t);

    std::uint32_t getProjPictureHeight() const;
    void setProjPictureHeight(std::uint32_t);

    std::uint16_t getPackedPictureWidth() const;
    void setPackedPictureWidth(std::uint16_t);

    std::uint16_t getPackedPictureHeight() const;
    void setPackedPictureHeight(std::uint16_t);

    Vector<RegionWisePackingBox::Region*> getRegions() const;
    void addRegion(UniquePtr<RegionWisePackingBox::Region>);

    /** @see Box::writeBox()
     */
    virtual void writeBox(ISOBMFF::BitStream& bitstr) const;

    /** @see Box::parseBox()
     */
    virtual void parseBox(ISOBMFF::BitStream& bitstr);

    /** @brief Dump box data to logInfo() in human readable format.
     */
    void dump() const;

private:
    bool mConstituentPictureMatchingFlag;
    std::uint32_t mProjPictureWidth;
    std::uint32_t mProjPictureHeight;
    std::uint16_t mPackedPictureWidth;
    std::uint16_t mPackedPictureHeight;

    Vector<UniquePtr<RegionWisePackingBox::Region>> mRegions;
};

#endif
