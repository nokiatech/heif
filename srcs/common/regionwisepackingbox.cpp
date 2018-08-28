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

#include "regionwisepackingbox.hpp"
#include "log.hpp"

RegionWisePackingBox::RegionWisePackingBox()
    : FullBox("rwpk", 0, 0)
    , mConstituentPictureMatchingFlag(false)
    , mProjPictureWidth(0)
    , mProjPictureHeight(0)
    , mPackedPictureWidth(0)
    , mPackedPictureHeight(0)
    , mRegions()
{
}

RegionWisePackingBox::RegionWisePackingBox(const RegionWisePackingBox& box)
    : FullBox(box)
    , mConstituentPictureMatchingFlag(box.mConstituentPictureMatchingFlag)
    , mProjPictureWidth(box.mProjPictureWidth)
    , mProjPictureHeight(box.mProjPictureHeight)
    , mPackedPictureWidth(box.mPackedPictureWidth)
    , mPackedPictureHeight(box.mPackedPictureHeight)
    , mRegions()
{
    for (auto& region : box.mRegions)
    {
        mRegions.push_back(makeCustomUnique<Region, Region>(*region));
    }
}

bool RegionWisePackingBox::getConstituentPictureMatchingFlag() const
{
    return mConstituentPictureMatchingFlag;
}

void RegionWisePackingBox::setConstituentPictureMatchingFlag(bool constituentPictureMatchingFlag)
{
    mConstituentPictureMatchingFlag = constituentPictureMatchingFlag;
}

std::uint32_t RegionWisePackingBox::getProjPictureWidth() const
{
    return mProjPictureWidth;
}

void RegionWisePackingBox::setProjPictureWidth(std::uint32_t width)
{
    mProjPictureWidth = width;
}

std::uint32_t RegionWisePackingBox::getProjPictureHeight() const
{
    return mProjPictureHeight;
}

void RegionWisePackingBox::setProjPictureHeight(std::uint32_t height)
{
    mProjPictureHeight = height;
}

std::uint16_t RegionWisePackingBox::getPackedPictureWidth() const
{
    return mPackedPictureWidth;
}

void RegionWisePackingBox::setPackedPictureWidth(std::uint16_t width)
{
    mPackedPictureWidth = width;
}

std::uint16_t RegionWisePackingBox::getPackedPictureHeight() const
{
    return mPackedPictureHeight;
}

void RegionWisePackingBox::setPackedPictureHeight(std::uint16_t height)
{
    mPackedPictureHeight = height;
}

Vector<RegionWisePackingBox::Region*> RegionWisePackingBox::getRegions() const
{
    Vector<Region*> regions;
    for (auto& region : mRegions)
    {
        regions.push_back(region.get());
    }
    return regions;
}


void RegionWisePackingBox::addRegion(UniquePtr<RegionWisePackingBox::Region> region)
{
    mRegions.push_back(std::move(region));
}

void RegionWisePackingBox::writeBox(BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.write8Bits(mConstituentPictureMatchingFlag ? 0b10000000 : 0x0);
    bitstr.write8Bits(mRegions.size());

    bitstr.write32Bits(mProjPictureWidth);
    bitstr.write32Bits(mProjPictureHeight);
    bitstr.write16Bits(mPackedPictureWidth);
    bitstr.write16Bits(mPackedPictureHeight);

    for (auto& region : mRegions)
    {
        // 4th bit is guardband flag and last 4 bits are packing type
        bitstr.write8Bits((region->guardBandFlag ? 0b00010000 : 0x0) | ((uint8_t) region->packingType & 0x0f));

        if (region->packingType == PackingType::RECTANGULAR)
        {
            auto& packing = region->rectangularPacking;
            bitstr.write32Bits(packing->projRegWidth);
            bitstr.write32Bits(packing->projRegHeight);
            bitstr.write32Bits(packing->projRegTop);
            bitstr.write32Bits(packing->projRegLeft);
            // type in bits 0bXXX00000
            bitstr.write8Bits(packing->transformType << 5);
            bitstr.write16Bits(packing->packedRegWidth);
            bitstr.write16Bits(packing->packedRegHeight);
            bitstr.write16Bits(packing->packedRegTop);
            bitstr.write16Bits(packing->packedRegLeft);

            if (region->guardBandFlag)
            {
                bitstr.write8Bits(packing->leftGbWidth);
                bitstr.write8Bits(packing->rightGbWidth);
                bitstr.write8Bits(packing->topGbHeight);
                bitstr.write8Bits(packing->bottomGbHeight);

                std::uint16_t packed16Bits = packing->gbNotUsedForPredFlag ? (0x1 << 15) : 0x0;
                packed16Bits |= ((packing->gbType0 & 0b111) << 12) | ((packing->gbType1 & 0b111) << 9) |
                                ((packing->gbType2 & 0b111) << 6) | ((packing->gbType3 & 0b111) << 3);
                bitstr.write16Bits(packed16Bits);
            }
        }
    }

    updateSize(bitstr);
}

void RegionWisePackingBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    // read region wise packing struct
    mConstituentPictureMatchingFlag = (bitstr.read8Bits() >> 7) & 0x1;
    std::uint8_t numRegions         = bitstr.read8Bits();
    mProjPictureWidth               = bitstr.read32Bits();
    mProjPictureHeight              = bitstr.read32Bits();
    mPackedPictureWidth             = bitstr.read16Bits();
    mPackedPictureHeight            = bitstr.read16Bits();

    for (int i = 0; i < numRegions; ++i)
    {
        auto region = makeCustomUnique<Region, Region>();

        std::uint8_t packed8Bits = bitstr.read8Bits();
        region->guardBandFlag    = (packed8Bits >> 4) & 0x01;
        region->packingType      = (PackingType)(packed8Bits & 0x0f);

        // If packingType is not RECTANGULAR, the data related to that region cannot be read nor skipped
        // (because we don't know the length of the region). In that case just something
        // corrupted will be read to regions vector and there is nothing we can do about it.
        // We cannot even throw an error, because there might be just region headers wihtout
        // data which can be read correctly.

        if (region->packingType == PackingType::RECTANGULAR)
        {
            auto rectangularPacking = makeCustomUnique<RectangularRegionWisePacking, RectangularRegionWisePacking>();

            // read RectRegionPacking
            rectangularPacking->projRegWidth    = bitstr.read32Bits();
            rectangularPacking->projRegHeight   = bitstr.read32Bits();
            rectangularPacking->projRegTop      = bitstr.read32Bits();
            rectangularPacking->projRegLeft     = bitstr.read32Bits();
            rectangularPacking->transformType   = bitstr.read8Bits() >> 5;
            rectangularPacking->packedRegWidth  = bitstr.read16Bits();
            rectangularPacking->packedRegHeight = bitstr.read16Bits();
            rectangularPacking->packedRegTop    = bitstr.read16Bits();
            rectangularPacking->packedRegLeft   = bitstr.read16Bits();

            if (region->guardBandFlag)
            {
                // read GuardBand
                rectangularPacking->leftGbWidth    = bitstr.read8Bits();
                rectangularPacking->rightGbWidth   = bitstr.read8Bits();
                rectangularPacking->topGbHeight    = bitstr.read8Bits();
                rectangularPacking->bottomGbHeight = bitstr.read8Bits();

                std::uint16_t packed16Bits               = bitstr.read16Bits();
                rectangularPacking->gbNotUsedForPredFlag = packed16Bits >> 15 == 1;
                rectangularPacking->gbType0              = (packed16Bits >> 12) & 0x07;
                rectangularPacking->gbType1              = (packed16Bits >> 9) & 0x07;
                rectangularPacking->gbType2              = (packed16Bits >> 6) & 0x07;
                rectangularPacking->gbType3              = (packed16Bits >> 3) & 0x07;
            }

            region->rectangularPacking = std::move(rectangularPacking);
        }

        mRegions.push_back(std::move(region));
    }
}

void RegionWisePackingBox::dump() const
{
    logInfo() << "---------------------------------- RWPK ------------------------------" << std::endl
              << "mConstituentPictureMatchingFlag: " << (std::uint32_t) mConstituentPictureMatchingFlag << std::endl
              << "mProjPictureWidth: " << (std::uint32_t) mProjPictureWidth << std::endl
              << "mProjPictureHeight: " << (std::uint32_t) mProjPictureHeight << std::endl
              << "mPackedPictureWidth: " << (std::uint32_t) mPackedPictureWidth << std::endl
              << "mPackedPictureHeight: " << (std::uint32_t) mPackedPictureHeight << std::endl
              << "mNumRegions: " << (std::uint32_t) mRegions.size() << std::endl;

    int regionOrderNum = 0;
    for (auto& region : mRegions)
    {
        regionOrderNum++;

        logInfo() << "---------- Region - " << regionOrderNum << std::endl
                  << "mGuarBandFlag: " << (std::uint32_t) region->guardBandFlag << std::endl
                  << "mPackingType: " << (std::uint32_t) region->packingType << std::endl;

        if (region->packingType == PackingType::RECTANGULAR)
        {
            logInfo() << "-------- RectRegionPacking struct" << std::endl
                      << "projRegWidth: " << (std::uint32_t) region->rectangularPacking->projRegWidth << std::endl
                      << "projRegHeight: " << (std::uint32_t) region->rectangularPacking->projRegHeight << std::endl
                      << "projRegTop: " << (std::uint32_t) region->rectangularPacking->projRegTop << std::endl
                      << "projRegLeft: " << (std::uint32_t) region->rectangularPacking->projRegLeft << std::endl
                      << "transformType: " << (std::uint32_t) region->rectangularPacking->transformType << std::endl
                      << "packedRegWidth: " << (std::uint32_t) region->rectangularPacking->packedRegWidth << std::endl
                      << "packedRegHeight: " << (std::uint32_t) region->rectangularPacking->packedRegHeight << std::endl
                      << "packedRegTop: " << (std::uint32_t) region->rectangularPacking->packedRegTop << std::endl
                      << "packedRegLeft: " << (std::uint32_t) region->rectangularPacking->packedRegLeft << std::endl;

            if (region->guardBandFlag)
            {
                logInfo() << "-------- GuardBand struct" << std::endl
                          << "leftGbWidth: " << (std::uint32_t) region->rectangularPacking->leftGbWidth << std::endl
                          << "rightGbWidth: " << (std::uint32_t) region->rectangularPacking->rightGbWidth << std::endl
                          << "topGbHeight: " << (std::uint32_t) region->rectangularPacking->topGbHeight << std::endl
                          << "bottomGbHeight: " << (std::uint32_t) region->rectangularPacking->bottomGbHeight
                          << std::endl
                          << "gbNotUsedForPredFlag: "
                          << (std::uint32_t) region->rectangularPacking->gbNotUsedForPredFlag << std::endl
                          << "gbType0: " << (std::uint32_t) region->rectangularPacking->gbType0 << std::endl
                          << "gbType1: " << (std::uint32_t) region->rectangularPacking->gbType1 << std::endl
                          << "gbType2: " << (std::uint32_t) region->rectangularPacking->gbType2 << std::endl
                          << "gbType3: " << (std::uint32_t) region->rectangularPacking->gbType3 << std::endl;
            }
        }
    }

    logInfo() << "-============================ End Of RWPK ===========================-" << std::endl;
}

RegionWisePackingBox::Region::Region(const Region& region)
    : guardBandFlag(region.guardBandFlag)
    , packingType(region.packingType)
    , rectangularPacking(nullptr)
{
    if (packingType == PackingType::RECTANGULAR)
    {
        rectangularPacking =
            makeCustomUnique<RectangularRegionWisePacking, RectangularRegionWisePacking>(*region.rectangularPacking);
    }
}
