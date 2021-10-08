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

#include "requiredreferencetypesproperty.hpp"

#include "bitstream.hpp"

RequiredReferenceTypesProperty::RequiredReferenceTypesProperty()
    : FullBox("rref", 0, 0)
{
}


Vector<FourCCInt> RequiredReferenceTypesProperty::getReferenceTypes() const
{
    return mReferenceTypes;
}

void RequiredReferenceTypesProperty::addReferenceType(const FourCCInt referenceType)
{
    mReferenceTypes.push_back(referenceType);
}


void RequiredReferenceTypesProperty::writeBox(BitStream& output) const
{
    writeFullBoxHeader(output);

    output.write32Bits(static_cast<unsigned int>(mReferenceTypes.size()));
    for (auto referenceType : mReferenceTypes)
    {
        output.write32Bits(referenceType.getUInt32());
    }

    updateSize(output);
}

void RequiredReferenceTypesProperty::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);

    const std::uint32_t entryCount = input.read32Bits();
    for (std::uint32_t i = 0; i < entryCount; ++i)
    {
        mReferenceTypes.push_back(input.read32Bits());
    }
}
