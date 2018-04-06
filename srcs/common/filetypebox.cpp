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

#include "filetypebox.hpp"
#include <algorithm>
#include "log.hpp"

FileTypeBox::FileTypeBox()
    : Box("ftyp")
    , mMajorBrand()
    , mMinorVersion(0)
    , mCompatibleBrands()
{
}

void FileTypeBox::setMajorBrand(const FourCCInt& majorBrand)
{
    mMajorBrand = majorBrand;
}

const FourCCInt& FileTypeBox::getMajorBrand() const
{
    return mMajorBrand;
}

void FileTypeBox::setMinorVersion(uint32_t minorVersion)
{
    mMinorVersion = minorVersion;
}

uint32_t FileTypeBox::getMinorVersion() const
{
    return mMinorVersion;
}

void FileTypeBox::addCompatibleBrand(const FourCCInt& compatibleBrand)
{
    if (!checkCompatibleBrand(compatibleBrand))
    {
        mCompatibleBrands.push_back(compatibleBrand);
    }
}

Vector<FourCCInt> FileTypeBox::getCompatibleBrands() const
{
    return mCompatibleBrands;
}

bool FileTypeBox::checkCompatibleBrand(const FourCCInt& brand) const
{
    if (find(mCompatibleBrands.begin(), mCompatibleBrands.end(), brand) != mCompatibleBrands.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void FileTypeBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Write the box header
    writeBoxHeader(bitstr);  // write parent box

    // major_brand
    bitstr.write32Bits(mMajorBrand.getUInt32());
    // minor_version
    bitstr.write32Bits(mMinorVersion);
    // compatible_brands[]
    for (auto const& brand : mCompatibleBrands)
    {
        bitstr.write32Bits(brand.getUInt32());
    }

    // Update the size field of this box in the bitstream
    updateSize(bitstr);
}

void FileTypeBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    // Parse the box header
    parseBoxHeader(bitstr);

    // major_brand
    mMajorBrand = FourCCInt(bitstr.read32Bits());
    // minor_version
    mMinorVersion = bitstr.read32Bits();
    // compatible_brands[]
    while (bitstr.numBytesLeft() >= 4)
    {
        FourCCInt compatibleBrand = FourCCInt(bitstr.read32Bits());
        mCompatibleBrands.push_back(compatibleBrand);
    }
}
