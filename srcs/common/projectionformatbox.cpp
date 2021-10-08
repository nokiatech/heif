/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include <cstdint>

#include "bitstream.hpp"
#include "projectionformatbox.hpp"

const uint8_t PROJECTION_TYPE_MASK = 0x1f;

ProjectionFormatBox::ProjectionFormatBox()
    : FullBox("prfr", 0, 0)
    , mProjectionType(0)
{
}

ProjectionFormatBox::ProjectionType ProjectionFormatBox::getProjectionType() const
{
    return (ProjectionType) mProjectionType;
}

void ProjectionFormatBox::setProjectionType(ProjectionType projectionType)
{
    mProjectionType = (uint8_t) projectionType & PROJECTION_TYPE_MASK;
}

void ProjectionFormatBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);
    bitstr.write8Bits(mProjectionType & PROJECTION_TYPE_MASK);
    updateSize(bitstr);
}

void ProjectionFormatBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    // projection type is stored in 5 least significant bits
    mProjectionType = bitstr.read8Bits() & PROJECTION_TYPE_MASK;
}
