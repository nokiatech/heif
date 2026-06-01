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

#include "targetolsproperty.hpp"

#include "bitstream.hpp"
#include "fourccint.hpp"

TargetOlsProperty::TargetOlsProperty()
    : Box("tols")
    , mOlsIdx(0)
{
}

void TargetOlsProperty::setOlsIndex(const std::uint16_t index)
{
    mOlsIdx = index;
}

std::uint16_t TargetOlsProperty::getOlsIndex() const
{
    return mOlsIdx;
}

void TargetOlsProperty::writeBox(BitStream& output) const
{
    writeBoxHeader(output);
    //bit(16) target_ols_index
    output.write16Bits(mOlsIdx);
    updateSize(output);
}

void TargetOlsProperty::parseBox(BitStream& input)
{
    parseBoxHeader(input);
    //bit(16) target_ols_index
    mOlsIdx = input.read16Bits();
}
