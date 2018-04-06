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

#include "auxiliarytypeproperty.hpp"
#include "bitstream.hpp"

AuxiliaryTypeProperty::AuxiliaryTypeProperty()
    : FullBox("auxC", 0, 0)
{
}

void AuxiliaryTypeProperty::setAuxType(const String& type)
{
    mAuxType = type;
}

String AuxiliaryTypeProperty::getAuxType() const
{
    return mAuxType;
}

void AuxiliaryTypeProperty::setAuxSubType(const Vector<std::uint8_t>& subtype)
{
    mAuxSubType = subtype;
}

Vector<std::uint8_t> AuxiliaryTypeProperty::getAuxSubType() const
{
    return mAuxSubType;
}

void AuxiliaryTypeProperty::writeBox(ISOBMFF::BitStream& output) const
{
    writeFullBoxHeader(output);
    output.writeZeroTerminatedString(mAuxType);
    output.write8BitsArray(mAuxSubType, static_cast<unsigned int>(mAuxSubType.size()));
    updateSize(output);
}

void AuxiliaryTypeProperty::parseBox(ISOBMFF::BitStream& input)
{
    parseFullBoxHeader(input);
    input.readZeroTerminatedString(mAuxType);
    const std::uint64_t bytesLeft = input.numBytesLeft();
    mAuxSubType.clear();
    input.read8BitsArray(mAuxSubType, bytesLeft);
}
