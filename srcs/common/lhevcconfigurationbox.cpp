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

#include "lhevcconfigurationbox.hpp"

#include "fourccint.hpp"

LhevcConfigurationBox::LhevcConfigurationBox()
    : Box("lhvC")
    , mLhevcConfig()
{
}

LhevcConfigurationBox::LhevcConfigurationBox(const LhevcConfigurationBox& box)
    : Box(box.getType())
    , mLhevcConfig(box.mLhevcConfig)
{
}

const LhevcDecoderConfigurationRecord& LhevcConfigurationBox::getLhevcConfiguration() const
{
    return mLhevcConfig;
}

const DecoderConfigurationRecord& LhevcConfigurationBox::getConfiguration() const
{
    return mLhevcConfig;
}

void LhevcConfigurationBox::setConfiguration(const LhevcDecoderConfigurationRecord& config)
{
    mLhevcConfig = config;
}

void LhevcConfigurationBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeBoxHeader(bitstr);
    mLhevcConfig.writeDecConfigRecord(bitstr);
    updateSize(bitstr);
}

void LhevcConfigurationBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseBoxHeader(bitstr);
    mLhevcConfig.parseConfig(bitstr);
}
