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

#include "itemdatabox.hpp"

#include <cstring>

ItemDataBox::ItemDataBox() :
    Box("idat"),
    mData()
{
}

void ItemDataBox::read(std::vector<std::uint8_t>& destination, const size_t offset, const size_t length) const
{
    destination.insert(destination.end(), mData.cbegin() + offset, mData.cbegin() + offset + length);
}

unsigned int ItemDataBox::addData(const std::vector<std::uint8_t>& data)
{
    const unsigned int offset = mData.size();
    mData.insert(mData.end(), data.cbegin(), data.cend());

    return offset;
}

void ItemDataBox::writeBox(BitStream& bitstr)
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

void ItemDataBox::parseBox(BitStream& bitstr)
{
    parseBoxHeader(bitstr);
    bitstr.read8BitsArray(mData, bitstr.numBytesLeft());
}
