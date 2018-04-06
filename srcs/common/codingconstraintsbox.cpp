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

#include "codingconstraintsbox.hpp"

CodingConstraintsBox::CodingConstraintsBox()
    : FullBox("ccst", 0, 0)
    , mAllRefPicsIntra(false)
    , mIntraPredUsed(false)
    , mMaxRefPicUsed(0)
{
}

CodingConstraintsBox::CodingConstraintsBox(const CodingConstraintsBox& other)
    : FullBox(other.getType(), 0, 0)
    , mAllRefPicsIntra(other.mAllRefPicsIntra)
    , mIntraPredUsed(other.mIntraPredUsed)
    , mMaxRefPicUsed(other.mMaxRefPicUsed)
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

void CodingConstraintsBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    writeFullBoxHeader(bitstr);

    bitstr.writeBits(mAllRefPicsIntra, 1);
    bitstr.writeBits(mIntraPredUsed, 1);
    bitstr.writeBits(mMaxRefPicUsed, 4);
    bitstr.writeBits(0, 26);  // reserved int(26)

    updateSize(bitstr);
}

void CodingConstraintsBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);
    mAllRefPicsIntra = (bitstr.readBits(1) != 0);
    mIntraPredUsed   = (bitstr.readBits(1) != 0);
    mMaxRefPicUsed   = (std::uint8_t) bitstr.readBits(4);
    bitstr.readBits(26);  // discard reserved int(26)
}
