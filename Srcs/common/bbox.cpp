/* Copyright (c) 2015-2017, Nokia Technologies Ltd.
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

#include "bbox.hpp"
#include "bitstream.hpp"

using namespace std;

Box::Box(FourCCInt boxType) :
    mSize(8),
    mType(boxType),
    mLargeSize( { 0, 0 }),
    mUserType(),
    mStartLocation(0)
{
}

void Box::setSize(uint32_t size)
{
    mSize = size;
}

uint32_t Box::getSize() const
{
    return mSize;
}

void Box::setType(const FourCCInt type)
{
    mType = type;
}

FourCCInt Box::getType() const
{
    return mType;
}

void Box::increaseSize(uint32_t sizeIncrease)
{
    if (mSize != 1)
    {
        uint32_t low16 = (mSize & 0xffff) + (sizeIncrease & 0xffff);
        uint32_t hi16 = ((mSize >> 16) & 0xffff) + ((sizeIncrease >> 16) & 0xffff) + (low16 >> 16);
        if (hi16 <= 0xffff)
        {
            mSize += sizeIncrease;
        }
        else
        {
            // switch to 64-bit size
            mSize = 1;
            mLargeSize.at(1) = ((hi16 & 0xffff) << 16) | (low16 & 0xffff);
            mLargeSize.at(0) = hi16 >> 16;
            increaseSize(8); // "large size" field adds 8 bytes to the size
        }
    }
    else
    {
        uint32_t low16 = (mLargeSize.at(1) & 0xffff) + (sizeIncrease & 0xffff);
        uint32_t hi16 = ((mLargeSize.at(1) >> 16) & 0xffff) + ((sizeIncrease >> 16) & 0xffff) + (low16 >> 16);
        mLargeSize.at(1) = ((hi16 & 0xffff) << 16) | (low16 & 0xffff);
        mLargeSize.at(0) += (hi16 >> 16);
    }
}

void Box::writeBoxHeader(BitStream& bitstr)
{
    mStartLocation = bitstr.getSize();

    bitstr.write32Bits(mSize);
    bitstr.write32Bits(mType.getUInt32());

    if (mSize == 1)
    {
        bitstr.write32Bits(mLargeSize.at(0));
        bitstr.write32Bits(mLargeSize.at(1));
    }

    /// @todo Make sure that mUserType is properly set before writing.
    if (mType == "uuid")
    {
        bitstr.write32Bits(mUserType.getUInt32());
    }
}

void Box::updateSize(BitStream& bitstr)
{
    mSize = bitstr.getSize() - mStartLocation;

    bitstr.setByte(mStartLocation + 0, (mSize >> 24) & 0xff); // Write updated size to the bitstream.
    bitstr.setByte(mStartLocation + 1, (mSize >> 16) & 0xff);
    bitstr.setByte(mStartLocation + 2, (mSize >> 8) & 0xff);
    bitstr.setByte(mStartLocation + 3, (mSize) & 0xff);
}

void Box::parseBoxHeader(BitStream& bitstr)
{
    mSize = bitstr.read32Bits();
    mType = bitstr.read32Bits();

    if (mSize == 1)
    {
        mLargeSize.at(0) = bitstr.read32Bits();
        mLargeSize.at(1) = bitstr.read32Bits();
    }

    if (mType == "uuid")
    {
        mUserType = bitstr.read32Bits();
    }
}
