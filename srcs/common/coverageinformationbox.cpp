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

#include "coverageinformationbox.hpp"
#include "log.hpp"

CoverageInformationBox::CoverageInformationBox()
    : FullBox("covi", 0, 0)
    , mViewIdcPresenceFlag(false)
    , mDefaultViewIdc(ViewIdcType::INVALID)
    , mSphereRegions()
{
}

CoverageInformationBox::CoverageInformationBox(const CoverageInformationBox& box)
    : FullBox(box)
    , mCoverageShapeType(box.mCoverageShapeType)
    , mViewIdcPresenceFlag(box.mViewIdcPresenceFlag)
    , mDefaultViewIdc(box.mDefaultViewIdc)
    , mSphereRegions()
{
    for (auto& region : box.mSphereRegions)
    {
        mSphereRegions.push_back(makeCustomUnique<CoverageSphereRegion, CoverageSphereRegion>(*region));
    }
}

CoverageInformationBox::CoverageShapeType CoverageInformationBox::getCoverageShapeType() const
{
    return mCoverageShapeType;
}

void CoverageInformationBox::setCoverageShapeType(CoverageShapeType shapeType)
{
    mCoverageShapeType = shapeType;
}

bool CoverageInformationBox::getViewIdcPresenceFlag() const
{
    return mViewIdcPresenceFlag;
}

void CoverageInformationBox::setViewIdcPresenceFlag(bool isViewIdcPresent)
{
    mViewIdcPresenceFlag = isViewIdcPresent;
}

ViewIdcType CoverageInformationBox::getDefaultViewIdc() const
{
    return mDefaultViewIdc;
}

void CoverageInformationBox::setDefaultViewIdc(ViewIdcType defaultViewIdc)
{
    mDefaultViewIdc = defaultViewIdc;
}

Vector<CoverageInformationBox::CoverageSphereRegion*> CoverageInformationBox::getSphereRegions() const
{
    Vector<CoverageSphereRegion*> regions;
    for (auto& region : mSphereRegions)
    {
        regions.push_back(region.get());
    }
    return regions;
}

void CoverageInformationBox::addSphereRegion(UniquePtr<CoverageSphereRegion> region)
{
    mSphereRegions.push_back(std::move(region));
}

void CoverageInformationBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.write8Bits((uint8_t) mCoverageShapeType);
    bitstr.write8Bits((uint8_t) mSphereRegions.size());

    // mDefaultViewIdc is in bits 0b0XX00000
    bitstr.write8Bits(mViewIdcPresenceFlag ? 0b10000000 : (((uint8_t) mDefaultViewIdc & 0b11) << 5));

    for (auto& region : mSphereRegions)
    {
        if (mViewIdcPresenceFlag)
        {
            // viewIdc is in first 2 bits 0bXX000000
            bitstr.write8Bits(((uint8_t) region->viewIdc & 0b0011) << 6);
        }
        region->region.write(bitstr, true);
    }

    updateSize(bitstr);
}

void CoverageInformationBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    mCoverageShapeType       = (CoverageShapeType) bitstr.read8Bits();
    std::uint8_t numRegions  = bitstr.read8Bits();
    std::uint8_t packed8Bits = bitstr.read8Bits();

    mViewIdcPresenceFlag = (packed8Bits >> 7) == 0x01;

    // if not used set all bits to init it with impossible value
    mDefaultViewIdc = (ViewIdcType)(mViewIdcPresenceFlag ? 0xff : ((packed8Bits >> 5) & 0b00000011));

    for (int i = 0; i < numRegions; ++i)
    {
        auto region = makeCustomUnique<CoverageSphereRegion, CoverageSphereRegion>();

        if (mViewIdcPresenceFlag)
        {
            packed8Bits     = bitstr.read8Bits();
            region->viewIdc = (ViewIdcType)((packed8Bits >> 6) & 0b00000011);
        }
        else
        {
            region->viewIdc = ViewIdcType::INVALID;
        }

        region->region.read(bitstr, true);
        mSphereRegions.push_back(std::move(region));
    }
}

void CoverageInformationBox::dump() const
{
    logInfo() << "---------------------------------- COVI ------------------------------" << std::endl
              << "mCoverageShapeType: " << (std::uint32_t) mCoverageShapeType << std::endl
              << "mViewIdcPresenceFlag: " << (std::uint32_t) mViewIdcPresenceFlag << std::endl
              << "mDefaultViewIdc: " << (std::uint32_t) mDefaultViewIdc << std::endl
              << "mNumRegions: " << (std::uint32_t) mSphereRegions.size() << std::endl;

    int regionOrderNum = 0;
    for (auto& region : mSphereRegions)
    {
        regionOrderNum++;

        logInfo() << "---------- Region - " << regionOrderNum << std::endl
                  << "viewIdc: " << (std::uint32_t) region->viewIdc << std::endl
                  << "centreAzimuth: " << region->region.centreAzimuth << std::endl
                  << "centreElevation: " << region->region.centreElevation << std::endl
                  << "centreTilt: " << region->region.centreTilt << std::endl
                  << "azimuthRange: " << region->region.azimuthRange << std::endl
                  << "elevationRange: " << region->region.elevationRange << std::endl
                  << "interpolate: " << region->region.interpolate << std::endl;
    }

    logInfo() << "-============================ End Of COVI ===========================-" << std::endl;
}
