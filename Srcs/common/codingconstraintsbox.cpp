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

#include "codingconstraintsbox.hpp"

CodingConstraintsBox::CodingConstraintsBox() :
    FullBox("ccst", 0, 0),
    mAllRefPicsIntra(false),
    mIntraPredUsed(false),
    mMaxRefPicUsed(0)
{
}

void CodingConstraintsBox::setAllRefPicsIntra(const bool flag)
{
    mAllRefPicsIntra = flag;
}

bool CodingConstraintsBox::getAllRefPicsIntra() const
{
    return mAllRefPicsIntra;
}

void CodingConstraintsBox::setIntraPredUsed(const bool flag)
{
    mIntraPredUsed = flag;
}

bool CodingConstraintsBox::getIntraPredUsed() const
{
    return mIntraPredUsed;
}

void CodingConstraintsBox::setMaxRefPicUsed(std::uint8_t maxRefPics)
{
    /* The field has only 4 bits. The value 15 is reserved to indicate that any number of reference images
     * permitted by the sample entry may be used. */
    if (maxRefPics > 15)
    {
        maxRefPics = 15;
    }
    mMaxRefPicUsed = maxRefPics;
}

std::uint8_t CodingConstraintsBox::getMaxRefPicUsed() const
{
    return mMaxRefPicUsed;
}

void CodingConstraintsBox::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);

    bitstr.writeBits(mAllRefPicsIntra, 1);
    bitstr.writeBits(mIntraPredUsed, 1);
    bitstr.writeBits(mMaxRefPicUsed, 4);
    bitstr.writeBits(0, 26); // reserved int(26)

    updateSize(bitstr);
}

void CodingConstraintsBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    mAllRefPicsIntra = bitstr.readBits(1);
    mIntraPredUsed = bitstr.readBits(1);
    mMaxRefPicUsed = bitstr.readBits(4);
    bitstr.readBits(26); // discard reserved int(26)
}
