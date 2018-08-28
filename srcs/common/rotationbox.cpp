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

#include "rotationbox.hpp"
#include "log.hpp"

RotationBox::RotationBox()
    : FullBox("rotn", 0, 0)
    , mRotation({})
{
}

RotationBox::RotationBox(const RotationBox& box)
    : FullBox(box)
    , mRotation(box.mRotation)
{
}

RotationBox::Rotation RotationBox::getRotation() const
{
    return mRotation;
}

void RotationBox::setRotation(Rotation rotation)
{
    mRotation = rotation;
}

void RotationBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.write32Bits(mRotation.yaw);
    bitstr.write32Bits(mRotation.pitch);
    bitstr.write32Bits(mRotation.roll);

    updateSize(bitstr);
}

void RotationBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    mRotation.yaw   = bitstr.read32Bits();
    mRotation.pitch = bitstr.read32Bits();
    mRotation.roll  = bitstr.read32Bits();
}
