/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "contentlightlevelbox.hpp"

#include "bitstream.hpp"
#include "fourccint.hpp"

ContentLightLevelBox::ContentLightLevelBox()
    : Box("clli")
{
}

void ContentLightLevelBox::setMaxContentLightLevel(uint16_t mcll)
{
    mMaxContentLightLevel = mcll;
}

uint16_t ContentLightLevelBox::getMaxContentLightLevel() const
{
    return mMaxContentLightLevel;
}

void ContentLightLevelBox::setMaxPicAverageLightLevel(uint16_t mpall)
{
    mMaxPicAverageLightLevel = mpall;
}

uint16_t ContentLightLevelBox::getMaxPicAverageLightLevel() const
{
    return mMaxPicAverageLightLevel;
}

void ContentLightLevelBox::writeBox(ISOBMFF::BitStream& output) const
{
    writeBoxHeader(output);
    output.write16Bits(mMaxContentLightLevel);
    output.write16Bits(mMaxPicAverageLightLevel);
    updateSize(output);
}

void ContentLightLevelBox::parseBox(ISOBMFF::BitStream& input)
{
    parseBoxHeader(input);
    setMaxContentLightLevel(input.read16Bits());
    setMaxPicAverageLightLevel(input.read16Bits());
}
