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

#include "hevcconfigurationbox.hpp"
#include "bitstream.hpp"

HevcConfigurationBox::HevcConfigurationBox()
    : DecoderConfigurationBox("hvcC")
    , mHevcConfig()
{
}

HevcConfigurationBox::HevcConfigurationBox(const HevcConfigurationBox& box)
    : DecoderConfigurationBox(box.getType())
    , mHevcConfig(box.mHevcConfig)
{
}

const HevcDecoderConfigurationRecord& HevcConfigurationBox::getHevcConfiguration() const
{
    return mHevcConfig;
}

const DecoderConfigurationRecord& HevcConfigurationBox::getConfiguration() const
{
    return mHevcConfig;
}

void HevcConfigurationBox::setConfiguration(const HevcDecoderConfigurationRecord& config)
{
    mHevcConfig = config;
}

void HevcConfigurationBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeBoxHeader(bitstr);
    mHevcConfig.writeDecConfigRecord(bitstr);
    updateSize(bitstr);
}

void HevcConfigurationBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseBoxHeader(bitstr);
    mHevcConfig.parseConfig(bitstr);
}
