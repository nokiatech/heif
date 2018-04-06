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

#include "colourinformationbox.hpp"
#include "bitstream.hpp"

ColourInformationBox::ColourInformationBox()
    : Box("colr")
    , mColourType(0)
    , mColourPrimaries(0)
    , mTransferCharasteristics(0)
    , mMatrixCoefficients(0)
    , mFullRangeFlag(false)
    , mIccProfile()
{
}

void ColourInformationBox::setColourType(const FourCCInt& type)
{
    mColourType = type;
}

FourCCInt ColourInformationBox::getColourType() const
{
    return mColourType;
}

void ColourInformationBox::setColourPrimaries(const uint16_t colourPrimaries)
{
    mColourPrimaries = colourPrimaries;
}

uint16_t ColourInformationBox::getColourPrimaries() const
{
    return mColourPrimaries;
}

void ColourInformationBox::setTransferCharacteristics(const uint16_t transferCharacteristics)
{
    mTransferCharasteristics = transferCharacteristics;
}

uint16_t ColourInformationBox::getTransferCharacteristics() const
{
    return mTransferCharasteristics;
}

void ColourInformationBox::setMatrixCoefficients(const uint16_t matrixCoefficients)
{
    mMatrixCoefficients = matrixCoefficients;
}

uint16_t ColourInformationBox::getMatrixCoefficients() const
{
    return mMatrixCoefficients;
}

void ColourInformationBox::setFullRangeFlag(const bool fullRange)
{
    mFullRangeFlag = fullRange;
}

bool ColourInformationBox::getFullRangeFlag() const
{
    return mFullRangeFlag;
}

Vector<std::uint8_t> ColourInformationBox::getIccProfile() const
{
    return mIccProfile;
}

void ColourInformationBox::setIccProfile(const Vector<std::uint8_t>& iccProfile)
{
    mIccProfile = iccProfile;
}

void ColourInformationBox::writeBox(BitStream& output) const
{
    writeBoxHeader(output);

    output.write32Bits(mColourType.getUInt32());
    if (mColourType == "nclx")
    {
        output.write16Bits(mColourPrimaries);
        output.write16Bits(mTransferCharasteristics);
        output.write16Bits(mMatrixCoefficients);
        output.writeBits(mFullRangeFlag, 1);
        output.writeBits(0, 7);  // reserved (7)
    }
    else if (mColourType == "rICC" || mColourType == "prof")
    {
        output.write8BitsArray(mIccProfile, mIccProfile.size(), 0);
    }

    updateSize(output);
}

void ColourInformationBox::parseBox(BitStream& input)
{
    mIccProfile.clear();

    parseBoxHeader(input);

    mColourType = input.read32Bits();
    if (mColourType == "nclx")
    {
        mColourPrimaries         = input.read16Bits();
        mTransferCharasteristics = input.read16Bits();
        mMatrixCoefficients      = input.read16Bits();
        mFullRangeFlag           = (input.readBits(1) != 0);
        input.readBits(7);  // reserved (7)
    }
    else if (mColourType == "rICC" || mColourType == "prof")
    {
        input.read8BitsArray(mIccProfile, input.numBytesLeft());
    }
}
