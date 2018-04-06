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

#include "itemdatabox.hpp"

#include <cstring>

ItemDataBox::ItemDataBox()
    : Box("idat")
    , mData()
{
}

bool ItemDataBox::read(Vector<std::uint8_t>& destination, const std::uint64_t offset, const std::uint64_t length) const
{
    if ((offset + length) > mData.size())
    {
        return false;
    }

    destination.insert(destination.end(), mData.cbegin() + static_cast<int64_t>(offset),
                       mData.cbegin() + static_cast<int64_t>(offset + length));
    return true;
}

bool ItemDataBox::read(uint8_t* destination, const std::uint64_t offset, const std::uint64_t length) const
{
    if ((offset + length) > mData.size() || destination == nullptr)
    {
        return false;
    }

    std::memcpy(destination, mData.data() + static_cast<int64_t>(offset), length);
    return true;
}

std::uint64_t ItemDataBox::addData(const Vector<std::uint8_t>& data)
{
    const std::uint64_t offset = mData.size();
    mData.insert(mData.end(), data.cbegin(), data.cend());

    return offset;
}

void ItemDataBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Do not write an empty box at all
    if (mData.size() == 0)
    {
        return;
    }

    writeBoxHeader(bitstr);
    bitstr.write8BitsArray(mData, mData.size());
    updateSize(bitstr);
}

void ItemDataBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseBoxHeader(bitstr);
    bitstr.read8BitsArray(mData, bitstr.numBytesLeft());
}
