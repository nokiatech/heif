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

#include "extendedtypebox.hpp"

#include <algorithm>

#include "bitstream.hpp"
#include "log.hpp"

ExtendedTypeBox::ExtendedTypeBox()
    : Box("etyp")
{
}

void ExtendedTypeBox::addTypeCombinationBox(const TypeCombinationBox& typeCombinationBox)
{
    // Do not add if similar TypeCombinationBox already exists.
    const auto& newCompatibleBrands          = typeCombinationBox.getCompatibleBrands();
    const std::set<FourCCInt> newCombination = {newCompatibleBrands.begin(), newCompatibleBrands.end()};
    for (const auto& oldCombination : mCompatibleCombinations)
    {
        const auto& existingCompatibleBrands          = oldCombination.getCompatibleBrands();
        const std::set<FourCCInt> existingCombination = {existingCompatibleBrands.begin(),
                                                         existingCompatibleBrands.end()};
        if (newCombination == existingCombination)
        {
            return;
        }
    }

    mCompatibleCombinations.push_back(typeCombinationBox);
}

Vector<TypeCombinationBox> ExtendedTypeBox::getTypeCombinationBoxes() const
{
    return mCompatibleCombinations;
}

bool ExtendedTypeBox::checkCompatibility(const Vector<FourCCInt>& supportedBrands) const
{
    for (const auto& tyco : mCompatibleCombinations)
    {
        const auto& brandsInTyco = tyco.getCompatibleBrands();
        if (brandsInTyco.empty())
        {
            // Probably this should not happen.
            continue;
        }
        bool allBrandsSupported = true;
        for (const auto& brandInTyco : brandsInTyco)
        {
            if (std::find(supportedBrands.cbegin(), supportedBrands.cend(), brandInTyco) == supportedBrands.cend())
            {
                allBrandsSupported = false;
                break;
            }
        }
        if (allBrandsSupported)
        {
            return true;
        }
    }

    return false;
}

void ExtendedTypeBox::writeBox(BitStream& output) const
{
    writeBoxHeader(output);
    for (auto& tyco : mCompatibleCombinations)
    {
        tyco.writeBox(output);
    }
    updateSize(output);
}

void ExtendedTypeBox::parseBox(BitStream& input)
{
    parseBoxHeader(input);

    // Read as many tyco boxes as there is
    while (input.numBytesLeft() > 8)
    {
        FourCCInt boxType;
        BitStream subBitStream = input.readSubBoxBitStream(boxType);

        if (boxType == "tyco")
        {
            TypeCombinationBox tyco;
            tyco.parseBox(subBitStream);
            mCompatibleCombinations.push_back(tyco);
        }
        else
        {
            logWarning() << "Skipping unknown box inside ExtendedTypeBox: '" << boxType.getString() << "'" << std::endl;
        }
    }
}
