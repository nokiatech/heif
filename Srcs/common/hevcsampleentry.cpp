/* Copyright (c) 2015, Nokia Technologies Ltd.
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

#include "hevcsampleentry.hpp"
#include "log.hpp"

HevcSampleEntry::HevcSampleEntry() :
    VisualSampleEntryBox("hvc1"),
    mHevcConfigurationBox(),
    mCodingConstraintsBox(),
    mIsCodingConstraintsPresent(false)
{
}

HevcConfigurationBox& HevcSampleEntry::getHevcConfigurationBox()
{
    return mHevcConfigurationBox;
}

void HevcSampleEntry::createCodingConstraintsBox()
{
    mIsCodingConstraintsPresent = true;
}

CodingConstraintsBox* HevcSampleEntry::getCodingConstraintsBox()
{
    return (mIsCodingConstraintsPresent ? &mCodingConstraintsBox : nullptr);
}

void HevcSampleEntry::writeBox(BitStream& bitstr)
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


void HevcSampleEntry::parseBox(BitStream& bitstr)
{
    VisualSampleEntryBox::parseBox(bitstr);

    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        std::string boxType;
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
            logWarning() << "Skipping unknown box of type '" << boxType << "' inside HevcSampleEntry" << std::endl;
        }
    }

}

