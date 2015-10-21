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

#include "videomediaheaderbox.hpp"
#include "bitstream.hpp"

VideoMediaHeaderBox::VideoMediaHeaderBox() :
    FullBox("vmhd", 0, 0)
{
}

void VideoMediaHeaderBox::writeBox(BitStream& bitstr)
{
    // Write box headers
    writeFullBoxHeader(bitstr);

    bitstr.write16Bits(0); // graphicsmode = 0
    bitstr.write16Bits(0); // opcolor = {0, 0, 0}
    bitstr.write16Bits(0);
    bitstr.write16Bits(0);

    // Update the size of the movie box
    updateSize(bitstr);
}

void VideoMediaHeaderBox::parseBox(BitStream& bitstr)
{
    BitStream subBitstr;

    //  First parse the box header
    parseFullBoxHeader(bitstr);

    bitstr.read16Bits(); // graphicsmode = 0
    bitstr.read16Bits(); // opcolor = {0, 0, 0}
    bitstr.read16Bits();
    bitstr.read16Bits();
}
