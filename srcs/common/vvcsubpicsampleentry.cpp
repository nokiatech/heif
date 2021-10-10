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

#include "vvcsubpicsampleentry.hpp"

#include "log.hpp"

VvcSubpicSampleEntry::VvcSubpicSampleEntry()
    : VisualSampleEntryBox("vvs1", "VVC Coding")
    , mConfig()
{
}

VvcSubpicSampleEntry::VvcSubpicSampleEntry(const VvcSubpicSampleEntry& box) = default;

VvcNaluConfigBox& VvcSubpicSampleEntry::getVvcNaluConfigBox()
{
    return mConfig;
}

const VvcNaluConfigBox& VvcSubpicSampleEntry::getVvcNaluConfigBox() const
{
    return mConfig;
}

void VvcSubpicSampleEntry::setVvcNaluConfigBox(const VvcNaluConfigBox& config)
{
    mConfig = config;
}

void VvcSubpicSampleEntry::writeBox(ISOBMFF::BitStream& bitstr) const
{
    VisualSampleEntryBox::writeBox(bitstr);

    mConfig.writeBox(bitstr);

    // Update the size of the movie box
    updateSize(bitstr);
}

void VvcSubpicSampleEntry::parseBox(ISOBMFF::BitStream& bitstr)
{
    VisualSampleEntryBox::parseBox(bitstr);

    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        FourCCInt boxType;
        BitStream subBitStream = bitstr.readSubBoxBitStream(boxType);

        // Handle this box based on the type
        if (boxType == "vvnC")
        {
            mConfig.parseBox(subBitStream);
        }
        else
        {
            logWarning() << "Skipping unknown box of type '" << boxType.getString() << "' inside VvcSubpicSampleEntry"
                         << std::endl;
        }
    }
}

const DecoderConfigurationRecord* VvcSubpicSampleEntry::getConfigurationRecord() const
{
    return nullptr;
}

const Box* VvcSubpicSampleEntry::getConfigurationBox() const
{
    return &mConfig;
}

VvcSubpicSampleEntry* VvcSubpicSampleEntry::clone() const
{
    return CUSTOM_NEW(VvcSubpicSampleEntry, (*this));
}
