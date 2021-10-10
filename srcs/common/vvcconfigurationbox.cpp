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

#include "vvcconfigurationbox.hpp"

#include "bitstream.hpp"

VvcConfigurationBox::VvcConfigurationBox()
    : FullBox("vvcC", 0, 0)
    , mVvcConfig()
{
}

VvcConfigurationBox::VvcConfigurationBox(const VvcConfigurationBox& box)
    : FullBox(box.getType(), box.getVersion(), box.getFlags())
    , mVvcConfig(box.mVvcConfig)
{
}

const VvcDecoderConfigurationRecord& VvcConfigurationBox::getVvcConfiguration() const
{
    return mVvcConfig;
}

const DecoderConfigurationRecord& VvcConfigurationBox::getConfiguration() const
{
    return mVvcConfig;
}

void VvcConfigurationBox::setConfiguration(const VvcDecoderConfigurationRecord& config)
{
    mVvcConfig = config;
}

void VvcConfigurationBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);
    mVvcConfig.writeDecConfigRecord(bitstr);
    updateSize(bitstr);
}

void VvcConfigurationBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    mVvcConfig.parseConfig(bitstr);
}
