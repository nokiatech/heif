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

#include "layeredhevcconfigurationitemproperty.hpp"
#include "bitstream.hpp"
#include "lhevcdecoderconfigrecord.hpp"

LayeredHevcConfigurationItemProperty::LayeredHevcConfigurationItemProperty() :
    Box("lhvC")
{
}

LHevcDecoderConfigurationRecord LayeredHevcConfigurationItemProperty::getConfiguration() const
{
    return mLHevcConfig;
}

void LayeredHevcConfigurationItemProperty::setConfiguration(const LHevcDecoderConfigurationRecord& config)
{
    mLHevcConfig = config;
}

void LayeredHevcConfigurationItemProperty::writeBox(BitStream& output)
{
    writeBoxHeader(output);
    mLHevcConfig.writeDecConfigRecord(output);
    updateSize(output);
}

void LayeredHevcConfigurationItemProperty::parseBox(BitStream& input)
{
    parseBoxHeader(input);
    mLHevcConfig.parseConfig(input);
}
