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

#include "videomediaheaderbox.hpp"
#include "bitstream.hpp"

VideoMediaHeaderBox::VideoMediaHeaderBox()
    : FullBox("vmhd", 0, 1)
{
}

void VideoMediaHeaderBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Write box headers
    writeFullBoxHeader(bitstr);

    bitstr.write16Bits(0);  // graphicsmode = 0
    bitstr.write16Bits(0);  // opcolor = {0, 0, 0}
    bitstr.write16Bits(0);
    bitstr.write16Bits(0);

    // Update the size of the movie box
    updateSize(bitstr);
}

void VideoMediaHeaderBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    //  First parse the box header
    parseFullBoxHeader(bitstr);

    bitstr.read16Bits();  // graphicsmode = 0
    bitstr.read16Bits();  // opcolor = {0, 0, 0}
    bitstr.read16Bits();
    bitstr.read16Bits();
}
