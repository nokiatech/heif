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

#include "bbox.hpp"
#include <limits>
#include "bitstream.hpp"

using namespace std;

Box::Box(FourCCInt boxType)
    : mSize(8)
    , mType(boxType)
    , mUserType()
    , mStartLocation(0)
    , mLargeSize(false)
{
}

void Box::setSize(uint64_t size)
{
    mSize = size;
}

uint64_t Box::getSize() const
{
    return mSize;
}

void Box::setLargeSize()
{
    mLargeSize = true;
}

bool Box::getLargeSize() const
{
    return mLargeSize;
}

void Box::setType(FourCCInt type)
{
    mType = type;
}

FourCCInt Box::getType() const
{
    return mType;
}

void Box::setUserType(Vector<uint8_t>& usertype)
{
    setType("uuid");
    mUserType = usertype;
}

Vector<uint8_t>& Box::getUserType()
{
    return mUserType;
}

void Box::writeBoxHeader(ISOBMFF::BitStream& bitstr) const
{
    mStartLocation = bitstr.getSize();

    // Note that serialized size values will be dummy values until updateSize() is called.
    if (mLargeSize)
    {
        bitstr.write32Bits(1);
    }
    else
    {
        bitstr.write32Bits(static_cast<uint32_t>(mSize));
    }

    bitstr.write32Bits(mType.getUInt32());

    // Note that serialized size values will be dummy values until updateSize() is called.
    if (mLargeSize)
    {
        bitstr.write64Bits(mSize);
    }

    /// @todo: Make sure that mUserType is properly set before writing...
    if (mType == "uuid")
    {
        bitstr.write8BitsArray(mUserType, 16);
    }
}

void Box::updateSize(ISOBMFF::BitStream& bitstr) const
{
    mSize = bitstr.getSize() - mStartLocation;

    if ((mSize > std::numeric_limits<std::uint32_t>::max()) && (mLargeSize == false))
    {
        throw RuntimeError("Box::updateSize(): Box size exceeds 4GB but large size for 64-bit size field was not set.");
    }

    // Write updated size to the bitstream.
    if (mLargeSize)
    {
        bitstr.setByte(mStartLocation + 8, (mSize >> 56) & 0xff);
        bitstr.setByte(mStartLocation + 9, (mSize >> 48) & 0xff);
        bitstr.setByte(mStartLocation + 10, (mSize >> 40) & 0xff);
        bitstr.setByte(mStartLocation + 11, (mSize >> 32) & 0xff);
        bitstr.setByte(mStartLocation + 12, (mSize >> 24) & 0xff);
        bitstr.setByte(mStartLocation + 13, (mSize >> 16) & 0xff);
        bitstr.setByte(mStartLocation + 14, (mSize >> 8) & 0xff);
        bitstr.setByte(mStartLocation + 15, mSize & 0xff);
    }
    else
    {
        bitstr.setByte(mStartLocation + 0, (mSize >> 24) & 0xff);
        bitstr.setByte(mStartLocation + 1, (mSize >> 16) & 0xff);
        bitstr.setByte(mStartLocation + 2, (mSize >> 8) & 0xff);
        bitstr.setByte(mStartLocation + 3, mSize & 0xff);
    }
}

void Box::parseBoxHeader(ISOBMFF::BitStream& bitstr)
{
    mSize = bitstr.read32Bits();
    mType = bitstr.read32Bits();

    if (mSize == 1)
    {
        mSize      = bitstr.read64Bits();
        mLargeSize = true;
    }

    if (mType == "uuid")
    {
        mUserType.clear();
        for (uint8_t i = 0; i < 16; ++i)
        {
            mUserType.push_back(bitstr.read8Bits());
        }
    }
}
