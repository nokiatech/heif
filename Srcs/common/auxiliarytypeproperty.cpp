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

#include "auxiliarytypeproperty.hpp"
#include "bitstream.hpp"

AuxiliaryTypeProperty::AuxiliaryTypeProperty() :
    FullBox("auxC", 0, 0)
{
}

void AuxiliaryTypeProperty::setAuxType(const std::string& type)
{
    mAuxType = type;
}

std::string AuxiliaryTypeProperty::getAuxType() const
{
    return mAuxType;
}

void AuxiliaryTypeProperty::setAuxSubType(const std::vector<std::uint8_t>& subtype)
{
    mAuxSubType = subtype;
}

std::vector<std::uint8_t> AuxiliaryTypeProperty::getAuxSubType() const
{
    return mAuxSubType;
}

void AuxiliaryTypeProperty::writeBox(BitStream& output)
{
    writeFullBoxHeader(output);
    output.writeZeroTerminatedString(mAuxType);
    output.write8BitsArray(mAuxSubType, mAuxSubType.size());
    updateSize(output);
}

void AuxiliaryTypeProperty::parseBox(BitStream& input)
{
    parseFullBoxHeader(input);
    input.readZeroTerminatedString(mAuxType);
    const size_t bytesLeft = input.numBytesLeft();
    mAuxSubType.clear();
    input.read8BitsArray(mAuxSubType, bytesLeft);
}
