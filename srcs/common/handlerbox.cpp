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

#include "handlerbox.hpp"
#include "log.hpp"

using namespace std;

HandlerBox::HandlerBox()
    : FullBox("hdlr", 0, 0)
    , mHandlerType()
    , mName("")
{
}

void HandlerBox::setHandlerType(FourCCInt handlerType)
{
    mHandlerType = handlerType;
}

FourCCInt HandlerBox::getHandlerType() const
{
    return mHandlerType;
}

void HandlerBox::setName(const String& name)
{
    mName = name;
}

const String& HandlerBox::getName() const
{
    return mName;
}

void HandlerBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(0);  // pre_defined = 0
    bitstr.write32Bits(mHandlerType.getUInt32());
    bitstr.write32Bits(0);  // reserved = 0
    bitstr.write32Bits(0);  // reserved = 0
    bitstr.write32Bits(0);  // reserved = 0
    bitstr.writeZeroTerminatedString(mName);

    updateSize(bitstr);
}

void HandlerBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    bitstr.read32Bits();  // pre_defined = 0
    mHandlerType = bitstr.read32Bits();
    bitstr.read32Bits();  // reserved = 0
    bitstr.read32Bits();  // reserved = 0
    bitstr.read32Bits();  // reserved = 0
    bitstr.readZeroTerminatedString(mName);
}
