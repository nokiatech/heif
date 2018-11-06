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

#include "auxiliarytypeinfobox.hpp"
#include "bitstream.hpp"

AuxiliaryTypeInfoBox::AuxiliaryTypeInfoBox()
    : FullBox("auxi", 0, 0)
{
}

void AuxiliaryTypeInfoBox::setAuxType(const String& type)
{
    mAuxType = type;
}

const String& AuxiliaryTypeInfoBox::getAuxType() const
{
    return mAuxType;
}

void AuxiliaryTypeInfoBox::writeBox(ISOBMFF::BitStream& output) const
{
    writeFullBoxHeader(output);
    output.writeZeroTerminatedString(mAuxType);
    updateSize(output);
}

void AuxiliaryTypeInfoBox::parseBox(ISOBMFF::BitStream& input)
{
    parseFullBoxHeader(input);
    input.readZeroTerminatedString(mAuxType);
}
