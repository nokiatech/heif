/* Copyright (c) 2015, Nokia Technologies Ltd.
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

#include "hevcconfigurationbox.hpp"
#include "bitstream.hpp"

HevcConfigurationBox::HevcConfigurationBox() :
    Box("hvcC"),
    mHevcConfig()
{
}

const HevcDecoderConfigurationRecord& HevcConfigurationBox::getConfiguration() const
{
    return mHevcConfig;
}

void HevcConfigurationBox::setConfiguration(const HevcDecoderConfigurationRecord& config)
{
    mHevcConfig = config;
}

void HevcConfigurationBox::writeBox(BitStream& bitstr)
{
    writeBoxHeader(bitstr);
    mHevcConfig.writeDecConfigRecord(bitstr);
    updateSize(bitstr);
}

void HevcConfigurationBox::parseBox(BitStream& bitstr)
{
    BitStream subBitstr;
    parseBoxHeader(bitstr);
    mHevcConfig.parseConfig(bitstr);
}
