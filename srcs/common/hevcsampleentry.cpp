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

#include "hevcsampleentry.hpp"
#include "log.hpp"

HevcSampleEntry::HevcSampleEntry()
    : VisualSampleEntryBox("hvc1", "HEVC Coding")
    , mHevcConfigurationBox()
    , mCodingConstraintsBox()
    , mIsCodingConstraintsPresent(false)
{
}

HevcSampleEntry::HevcSampleEntry(const HevcSampleEntry& box)
    : VisualSampleEntryBox(box)
    , mHevcConfigurationBox(box.mHevcConfigurationBox)
    , mCodingConstraintsBox(box.mCodingConstraintsBox)
    , mIsCodingConstraintsPresent(box.mIsCodingConstraintsPresent)
{
}

HevcConfigurationBox& HevcSampleEntry::getHevcConfigurationBox()
{
    return mHevcConfigurationBox;
}

const HevcConfigurationBox& HevcSampleEntry::getHevcConfigurationBox() const
{
    return mHevcConfigurationBox;
}

void HevcSampleEntry::createCodingConstraintsBox()
{
    mIsCodingConstraintsPresent = true;
}

const CodingConstraintsBox* HevcSampleEntry::getCodingConstraintsBox() const
{
    return (mIsCodingConstraintsPresent ? &mCodingConstraintsBox : nullptr);
}

CodingConstraintsBox* HevcSampleEntry::getCodingConstraintsBox()
{
    return (mIsCodingConstraintsPresent ? &mCodingConstraintsBox : nullptr);
}

void HevcSampleEntry::writeBox(ISOBMFF::BitStream& bitstr) const
{
    VisualSampleEntryBox::writeBox(bitstr);

    mHevcConfigurationBox.writeBox(bitstr);

    if (mIsCodingConstraintsPresent)
    {
        mCodingConstraintsBox.writeBox(bitstr);
    }

    // Update the size of the movie box
    updateSize(bitstr);
}

void HevcSampleEntry::parseBox(ISOBMFF::BitStream& bitstr)
{
    VisualSampleEntryBox::parseBox(bitstr);

    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        FourCCInt boxType;
        BitStream subBitStream = bitstr.readSubBoxBitStream(boxType);

        // Handle this box based on the type
        if (boxType == "hvcC")
        {
            mHevcConfigurationBox.parseBox(subBitStream);
        }
        else if (boxType == "ccst")
        {
            mCodingConstraintsBox.parseBox(subBitStream);
            mIsCodingConstraintsPresent = true;
        }
        else
        {
            logWarning() << "Skipping unknown box of type '" << boxType.getString() << "' inside HevcSampleEntry" << std::endl;
        }
    }
}

HevcSampleEntry* HevcSampleEntry::clone() const
{
    return CUSTOM_NEW(HevcSampleEntry, (*this));
}

const Box* HevcSampleEntry::getConfigurationBox() const
{
    return &mHevcConfigurationBox;
}

const DecoderConfigurationRecord* HevcSampleEntry::getConfigurationRecord() const
{
    return &mHevcConfigurationBox.getConfiguration();
}
