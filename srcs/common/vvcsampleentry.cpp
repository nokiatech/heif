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

#include "vvcsampleentry.hpp"

#include "log.hpp"

VvcSampleEntry::VvcSampleEntry()
    : VisualSampleEntryBox("vvc1", "VVC Coding")
    , mVvcConfigurationBox()
{
}

VvcSampleEntry::VvcSampleEntry(const VvcSampleEntry& box) = default;

VvcConfigurationBox& VvcSampleEntry::getVvcConfigurationBox()
{
    return mVvcConfigurationBox;
}

const VvcConfigurationBox& VvcSampleEntry::getVvcConfigurationBox() const
{
    return mVvcConfigurationBox;
}

void VvcSampleEntry::writeBox(ISOBMFF::BitStream& bitstr) const
{
    VisualSampleEntryBox::writeBox(bitstr);
    mVvcConfigurationBox.writeBox(bitstr);
    updateSize(bitstr);
}

void VvcSampleEntry::parseBox(ISOBMFF::BitStream& bitstr)
{
    VisualSampleEntryBox::parseBox(bitstr);

    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        FourCCInt boxType;
        BitStream subBitStream = bitstr.readSubBoxBitStream(boxType);

        // Handle this box based on the type
        if (boxType == "vvcC")
        {
            mVvcConfigurationBox.parseBox(subBitStream);
        }
        else
        {
            logWarning() << "Skipping unknown box of type '" << boxType.getString() << "' inside VvcSampleEntry"
                         << std::endl;
        }
    }
}

VvcSampleEntry* VvcSampleEntry::clone() const
{
    return CUSTOM_NEW(VvcSampleEntry, (*this));
}

const Box* VvcSampleEntry::getConfigurationBox() const
{
    return &mVvcConfigurationBox;
}

const DecoderConfigurationRecord* VvcSampleEntry::getConfigurationRecord() const
{
    return &mVvcConfigurationBox.getConfiguration();
}
