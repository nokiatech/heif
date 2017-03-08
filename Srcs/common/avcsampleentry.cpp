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

#include "avcsampleentry.hpp"
#include "log.hpp"

AvcSampleEntry::AvcSampleEntry() :
    VisualSampleEntryBox("avc1", "AVC Coding"),
    mAvcConfigurationBox(),
    mCodingConstraintsBox(),
    mIsCodingConstraintsPresent(false)
{
}

AvcConfigurationBox& AvcSampleEntry::getAvcConfigurationBox()
{
    return mAvcConfigurationBox;
}

void AvcSampleEntry::createCodingConstraintsBox()
{
    mIsCodingConstraintsPresent = true;
}

CodingConstraintsBox* AvcSampleEntry::getCodingConstraintsBox()
{
    return (mIsCodingConstraintsPresent ? &mCodingConstraintsBox : nullptr);
}

void AvcSampleEntry::writeBox(BitStream& bitstr)
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


void AvcSampleEntry::parseBox(BitStream& bitstr)
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

