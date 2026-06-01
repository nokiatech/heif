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

#include "layerselectorproperty.hpp"

#include "bitstream.hpp"
#include "fourccint.hpp"

LayerSelectorProperty::LayerSelectorProperty()
    : Box("lsel")
    , mLayerId(0)
{
}

void LayerSelectorProperty::setLayerId(const std::uint16_t id)
{
    mLayerId = id;
}

std::uint16_t LayerSelectorProperty::getLayerId() const
{
    return mLayerId;
}

void LayerSelectorProperty::writeBox(BitStream& output) const
{
    writeBoxHeader(output);
    //bit(16) layer_id
    output.write16Bits(mLayerId);
    updateSize(output);
}

void LayerSelectorProperty::parseBox(BitStream& input)
{
    parseBoxHeader(input);
    //bit(16) layer_id
    mLayerId = input.read16Bits();
}
