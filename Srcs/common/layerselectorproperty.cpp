/* Copyright (c) 2017, Nokia Technologies Ltd.
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

#include "layerselectorproperty.hpp"
#include "bitstream.hpp"

LayerSelectorProperty::LayerSelectorProperty() :
    Box("lsel"),
    mLayerId(0)
{
}

LayerSelectorProperty::LayerSelectorProperty(const std::uint16_t layerId) :
    Box("lsel"),
    mLayerId(layerId)
{
}

std::uint16_t LayerSelectorProperty::getLayerId() const
{
    return mLayerId;
}

void LayerSelectorProperty::setLayerId(const std::uint16_t layerId)
{
    mLayerId = layerId;
}

void LayerSelectorProperty::writeBox(BitStream& output)
{
    writeBoxHeader(output);
    output.write16Bits(mLayerId);
    updateSize(output);
}

void LayerSelectorProperty::parseBox(BitStream& input)
{
    parseBoxHeader(input);
    mLayerId = input.read16Bits();
}
