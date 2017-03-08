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

#include "avcconfigurationbox.hpp"
#include "avccommondefs.hpp"
#include "bitstream.hpp"

AvcConfigurationBox::AvcConfigurationBox() :
    Box("avcC"),
    mAvcConfig()
{
}

const AvcDecoderConfigurationRecord& AvcConfigurationBox::getConfiguration() const
{
    return mAvcConfig;
}

void AvcConfigurationBox::setConfiguration(const AvcDecoderConfigurationRecord& config)
{
    mAvcConfig = config;
}

void AvcConfigurationBox::writeBox(BitStream& bitstr)
{
    writeBoxHeader(bitstr);
    mAvcConfig.writeDecConfigRecord(bitstr);
    updateSize(bitstr);
}

void AvcConfigurationBox::parseBox(BitStream& bitstr)
{
    parseBoxHeader(bitstr);
    mAvcConfig.parseConfig(bitstr);
}
