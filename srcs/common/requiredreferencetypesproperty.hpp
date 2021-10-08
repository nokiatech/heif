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

#ifndef REQUIRED_REFERENCE_TYPES_PROPERTY_HPP
#define REQUIRED_REFERENCE_TYPES_PROPERTY_HPP

#include <cstdint>

#include "customallocator.hpp"
#include "fullbox.hpp"

namespace ISOBMFF
{
    class BitStream;
}

/** @brief RequiredReferenceTypesProperty class. Extends from FullBox.
 *  @details 'rref' box contains reference types which the reader is required to process to decode the associated image
 *           item. */
class RequiredReferenceTypesProperty : public FullBox
{
public:
    RequiredReferenceTypesProperty();
    ~RequiredReferenceTypesProperty() override = default;

    /**
     * @brief getReferenceTypes Get required reference types in the box.
     * @return Required reference types in the box.
     */
    Vector<FourCCInt> getReferenceTypes() const;

    /**
     * @brief addReferenceType Append a new required reference type to the box.
     * @param referenceType The reference type to be added.
     */
    void addReferenceType(FourCCInt referenceType);

    /** Write box/property to ISOBMFF::BitStream.
     *  @see Box::writeBox() */
    void writeBox(ISOBMFF::BitStream& output) const override;

    /** Parse box/property from ISOBMFF::BitStream.
     *  @see Box::parseBox() */
    void parseBox(ISOBMFF::BitStream& input) override;

private:
    Vector<FourCCInt> mReferenceTypes;
};

#endif
