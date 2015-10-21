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

#include "handlerbox.hpp"
#include "log.hpp"

using namespace std;

HandlerBox::HandlerBox() :
    FullBox("hdlr", 0, 0),
    mHandlerType(),
    mName("")
{
}

void HandlerBox::setHandlerType(const string& handlerType)
{
    mHandlerType = handlerType;
}

const string& HandlerBox::getHandlerType() const
{
    return mHandlerType;
}

void HandlerBox::setName(const string& name)
{
    mName = name;
}

const string& HandlerBox::getName()
{
    return mName;
}

void HandlerBox::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(0); // pre_defined = 0
    bitstr.writeString(mHandlerType);
    bitstr.write32Bits(0); // reserved = 0
    bitstr.write32Bits(0); // reserved = 0
    bitstr.write32Bits(0); // reserved = 0
    bitstr.writeZeroTerminatedString(mName);

    updateSize(bitstr);
}

void HandlerBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    bitstr.read32Bits(); // pre_defined = 0
    bitstr.readStringWithLen(mHandlerType, 4);
    bitstr.read32Bits(); // reserved = 0
    bitstr.read32Bits(); // reserved = 0
    bitstr.read32Bits(); // reserved = 0
    bitstr.readZeroTerminatedString(mName);

    if (mHandlerType != "pict")
    {
        logWarning() << "Unknown media handler in track: " << mHandlerType << endl;
    }
}
