/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "typecombinationbox.hpp"

#include <algorithm>

#include "bitstream.hpp"

TypeCombinationBox::TypeCombinationBox()
    : Box("tyco")
{
}

bool TypeCombinationBox::checkCompatibleBrand(const FourCCInt& brand) const
{
    if (std::find(mCompatibleBrands.begin(), mCompatibleBrands.end(), brand) != mCompatibleBrands.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void TypeCombinationBox::addCompatibleBrand(const FourCCInt& compatibleBrand)
{
    if (!checkCompatibleBrand(compatibleBrand))
    {
        mCompatibleBrands.push_back(compatibleBrand);
    }
}

Vector<FourCCInt> TypeCombinationBox::getCompatibleBrands() const
{
    return mCompatibleBrands;
}

void TypeCombinationBox::writeBox(BitStream& output) const
{
    writeBoxHeader(output);

    // compatible_brands[]
    for (auto const& brand : mCompatibleBrands)
    {
        output.write32Bits(brand.getUInt32());
    }

    updateSize(output);
}

void TypeCombinationBox::parseBox(BitStream& input)
{
    parseBoxHeader(input);

    // compatible_brands[]
    while (input.numBytesLeft() >= 4)
    {
        FourCCInt compatibleBrand = FourCCInt(input.read32Bits());
        mCompatibleBrands.push_back(compatibleBrand);
    }
}
