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

#ifndef TYPECOMBINATIONBOX_HPP
#define TYPECOMBINATIONBOX_HPP

#include <cstdint>

#include "bbox.hpp"
#include "customallocator.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** The TypeCombinationBox 'tyco' as defined in the ISOBMFF standard. */
class TypeCombinationBox : public Box
{
public:
    TypeCombinationBox();
    ~TypeCombinationBox() override = default;

    /** @brief Adds a compatible brand to the list of compatible brands
     *  @param [in] compatibleBrand Compatible Brand 4CC value, as defined in relevant file format standards.
     */
    void addCompatibleBrand(const FourCCInt& compatibleBrand);

    /** @brief Get the Compatible Brands List
     *  @returns Compatible Brand as vector of 4CC values */
    Vector<FourCCInt> getCompatibleBrands() const;

    /** @brief Check if a brand is among the list of compatible brands
     *  @param [in] brand Brand value to be checked
     *  @returns TRUE: if the brand is in the compatible brands list. FALSE otherwise. */
    bool checkCompatibleBrand(const FourCCInt& brand) const;

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    void writeBox(ISOBMFF::BitStream& output) const override;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    void parseBox(ISOBMFF::BitStream& input) override;

private:
    Vector<FourCCInt> mCompatibleBrands;  ///< Vector containing the Compatible Brands 4CCs
};

#endif
