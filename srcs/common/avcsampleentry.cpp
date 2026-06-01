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

#include "avcsampleentry.hpp"
#include "customallocator.hpp"

#include "bitstream.hpp"

#include "log.hpp"
#include "fourccint.hpp"

AvcSampleEntry::AvcSampleEntry()
    : VisualSampleEntryBox("avc1", "AVC Coding")
    , mAvcConfigurationBox()
    , mCodingConstraintsBox()
     
{
}

AvcSampleEntry::AvcSampleEntry(const AvcSampleEntry& box) = default;

AvcConfigurationBox& AvcSampleEntry::getAvcConfigurationBox()
{
    return mAvcConfigurationBox;
}

const AvcConfigurationBox& AvcSampleEntry::getAvcConfigurationBox() const
{
    return mAvcConfigurationBox;
}

void AvcSampleEntry::createCodingConstraintsBox()
{
    mIsCodingConstraintsPresent = true;
}

const CodingConstraintsBox* AvcSampleEntry::getCodingConstraintsBox() const
{
    return (mIsCodingConstraintsPresent ? &mCodingConstraintsBox : nullptr);
}

CodingConstraintsBox* AvcSampleEntry::getCodingConstraintsBox()
{
    return (mIsCodingConstraintsPresent ? &mCodingConstraintsBox : nullptr);
}

void AvcSampleEntry::writeBox(ISOBMFF::BitStream& bitstr) const
{
    VisualSampleEntryBox::writeBox(bitstr);

    mAvcConfigurationBox.writeBox(bitstr);

    if (mIsCodingConstraintsPresent)
    {
        mCodingConstraintsBox.writeBox(bitstr);
    }

    updateSize(bitstr);
}

void AvcSampleEntry::parseBox(ISOBMFF::BitStream& bitstr)
{
    VisualSampleEntryBox::parseBox(bitstr);

    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        FourCCInt boxType;
        BitStream subBitStream = bitstr.readSubBoxBitStream(boxType);

        // Handle this box based on the type
        if (boxType == "avcC")
        {
            mAvcConfigurationBox.parseBox(subBitStream);
        }
        else if (boxType == "ccst")
        {
            mCodingConstraintsBox.parseBox(subBitStream);
            mIsCodingConstraintsPresent = true;
        }
        else
        {
            logWarning() << "Skipping unknown box of type '" << boxType.getString() << "' inside AvcSampleEntry"
                         << '\n';
        }
    }
}

AvcSampleEntry* AvcSampleEntry::clone() const
{
    return customNew<AvcSampleEntry>(*this);
}

const DecoderConfigurationRecord* AvcSampleEntry::getConfigurationRecord() const
{
    return &mAvcConfigurationBox.getConfiguration();
}

const Box* AvcSampleEntry::getConfigurationBox() const
{
    return &mAvcConfigurationBox;
}
