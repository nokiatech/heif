/* Copyright (c) 2015, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#include "filetypebox.hpp"
#include "log.hpp"
#include <algorithm>

FileTypeBox::FileTypeBox() :
    Box("ftyp"),
    mMajorBrand(),
    mMinorVersion(0),
    mCompatibleBrands()
{
}

void FileTypeBox::setMajorBrand(const std::string& majorBrand)
{
    mMajorBrand = majorBrand;
}

const std::string& FileTypeBox::getMajorBrand() const
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

void FileTypeBox::addCompatibleBrand(const std::string& compatibleBrand)
{
    mCompatibleBrands.push_back(compatibleBrand);
}

std::vector<std::string> FileTypeBox::getCompatibleBrands() const
{
    return mCompatibleBrands;
}

bool FileTypeBox::checkCompatibleBrand(const std::string& brand) const
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

void FileTypeBox::writeBox(BitStream& bitstr)
{
    // Write the box header
    writeBoxHeader(bitstr);  // write parent box

    // major_brand
    bitstr.writeString(mMajorBrand);
    // minor_version
    bitstr.write32Bits(mMinorVersion);
    // compatible_brands[]
    for (auto const& brand : mCompatibleBrands)
    {
        bitstr.writeString(brand);
    }

    // Update the size field of this box in the bitstream
    updateSize(bitstr);
}

void FileTypeBox::parseBox(BitStream& bitstr)
{
    // Parse the box header
    parseBoxHeader(bitstr);

    // major_brand
    bitstr.readStringWithLen(mMajorBrand, 4);
    // minor_version
    mMinorVersion = bitstr.read32Bits();
    // compatible_brands[]
    while (bitstr.numBytesLeft() >= 4)
    {
        std::string compatibleBrand;
        bitstr.readStringWithLen(compatibleBrand, 4);
        mCompatibleBrands.push_back(compatibleBrand);
    }
}
