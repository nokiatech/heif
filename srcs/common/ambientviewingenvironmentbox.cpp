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

#include "ambientviewingenvironmentbox.hpp"

#include "bitstream.hpp"
#include "fourccint.hpp"

AmbientViewingEnvironmentBox::AmbientViewingEnvironmentBox()
    :Box("amve")
{
}

void AmbientViewingEnvironmentBox::setAmbientIlluminance(const uint32_t value)
{
    mAmbientIlluminance = value;
}

uint32_t AmbientViewingEnvironmentBox::getAmbientIlluminance() const
{
    return mAmbientIlluminance;
}

void AmbientViewingEnvironmentBox::setAmbientLightX(uint16_t value)
{
    mAmbientLightX = value;
}

uint16_t AmbientViewingEnvironmentBox::getAmbientLightX() const
{
    return mAmbientLightX;
}

void AmbientViewingEnvironmentBox::setAmbientLightY(uint16_t value)
{
    mAmbientLightY = value;
}

uint16_t AmbientViewingEnvironmentBox::getAmbientLightY() const
{
    return mAmbientLightY;
}

void AmbientViewingEnvironmentBox::writeBox(ISOBMFF::BitStream &output) const
{
    writeBoxHeader(output);
    output.write32Bits(mAmbientIlluminance);
    output.write16Bits(mAmbientLightX);
    output.write16Bits(mAmbientLightY);
    updateSize(output);
}

void AmbientViewingEnvironmentBox::parseBox(ISOBMFF::BitStream &input)
{
    parseBoxHeader(input);
    mAmbientIlluminance = input.read32Bits();
    mAmbientLightX = input.read16Bits();
    mAmbientLightY = input.read16Bits();
}
