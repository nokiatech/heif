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

#include "jpegconfigurationbox.hpp"
#include "bitstream.hpp"

JpegConfigurationBox::JpegConfigurationBox()
    : Box("jpgC")
    , mJpegPrefix()
{
}

Vector<uint8_t> JpegConfigurationBox::getPrefix() const
{
    return mJpegPrefix;
}


void JpegConfigurationBox::setPrefix(const Vector<uint8_t>& data)
{
    mJpegPrefix = data;
}

void JpegConfigurationBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeBoxHeader(bitstr);
    bitstr.write8BitsArray(mJpegPrefix, mJpegPrefix.size());
    updateSize(bitstr);
}

void JpegConfigurationBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseBoxHeader(bitstr);
    const auto length = bitstr.numBytesLeft();
    bitstr.read8BitsArray(mJpegPrefix, length);
}
