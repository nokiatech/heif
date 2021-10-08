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

#ifndef EXTENDEDTYPEBOX_HPP
#define EXTENDEDTYPEBOX_HPP

#include <cstdint>

#include "bbox.hpp"
#include "customallocator.hpp"
#include "typecombinationbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** The Extended type box 'etyp' contains one or more Type combination boxes, as defined in the ISOBMFF standard. */
class ExtendedTypeBox : public Box
{
public:
    ExtendedTypeBox();
    ~ExtendedTypeBox() override = default;

    /** @brief Adds a Type combination box to compatible combinations.
     *  If a Type combination box with similar content is already present, a new box will not be added.
     *  @param [in] typeCombinationBoxes Compatible Brand 4CC value, as defined in relevant file format standards.
     */
    void addTypeCombinationBox(const TypeCombinationBox& typeCombinationBox);

    /** @brief Get Type Combination Boxes
     *  @returns TypeCombinationBoxes as vector */
    Vector<TypeCombinationBox> getTypeCombinationBoxes() const;

    /**
     * @brief Check if brands given as parameter cover all the brands in at least one of the contained
     * TypeCombinationBox.
     * @param [in] supportedBrands Types to look for from compatible combinations in the extended type box.
     * @return True if brands in at least one TypeCombinationBox are listed in supportedBrands. False otherwise.
     */
    bool checkCompatibility(const Vector<FourCCInt>& supportedBrands) const;

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    void writeBox(ISOBMFF::BitStream& output) const override;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    void parseBox(ISOBMFF::BitStream& input) override;

private:
    Vector<TypeCombinationBox> mCompatibleCombinations;  ///< Contained TypeCombinationBoxes.
};

#endif
