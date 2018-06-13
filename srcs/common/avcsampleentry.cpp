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

#include "avcsampleentry.hpp"
#include "log.hpp"

AvcSampleEntry::AvcSampleEntry()
    : VisualSampleEntryBox("avc1", "AVC Coding")
    , mAvcConfigurationBox()
    , mCodingConstraintsBox()
    , mIsCodingConstraintsPresent(false)
{
}

AvcSampleEntry::AvcSampleEntry(const AvcSampleEntry& box)
    : VisualSampleEntryBox(box)
    , mAvcConfigurationBox(box.mAvcConfigurationBox)
    , mCodingConstraintsBox(box.mCodingConstraintsBox)
    , mIsCodingConstraintsPresent(box.mIsCodingConstraintsPresent)
{
}

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

    // Update the size of the movie box
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
            logWarning() << "Skipping unknown box of type '" << boxType << "' inside AvcSampleEntry" << std::endl;
        }
    }
}

AvcSampleEntry* AvcSampleEntry::clone() const
{
    return CUSTOM_NEW(AvcSampleEntry, (*this));
}

const DecoderConfigurationRecord* AvcSampleEntry::getConfigurationRecord() const
{
    return &mAvcConfigurationBox.getConfiguration();
}

const Box* AvcSampleEntry::getConfigurationBox() const
{
    return &mAvcConfigurationBox;
}
