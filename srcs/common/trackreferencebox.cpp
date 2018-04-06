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

#include "trackreferencebox.hpp"
#include "log.hpp"

using namespace std;

TrackReferenceBox::TrackReferenceBox()
    : Box("tref")
    , mTrefTypeBoxes()
{
}

void TrackReferenceBox::clearTrefBoxes()
{
    mTrefTypeBoxes.clear();
}

void TrackReferenceBox::addTrefTypeBox(TrackReferenceTypeBox& trefTypeBox)
{
    mTrefTypeBoxes.push_back(trefTypeBox);
}

const Vector<TrackReferenceTypeBox>& TrackReferenceBox::getTrefTypeBoxes() const
{
    return mTrefTypeBoxes;
}

bool TrackReferenceBox::isReferenceTypePresent(FourCCInt type) const
{
    for (const auto& trackReferenceTypeBox : mTrefTypeBoxes)
    {
        if (trackReferenceTypeBox.getType() == type)
        {
            return true;
        }
    }

    return false;
}

void TrackReferenceBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    // Write box headers
    writeBoxHeader(bitstr);

    // For each track reference type call its writeBox method
    for (auto& trefTypeBox : mTrefTypeBoxes)
    {
        trefTypeBox.writeBox(bitstr);
    }

    updateSize(bitstr);
}

void TrackReferenceBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    // Read sub-boxes until no data is left in this box
    while (bitstr.numBytesLeft() > 0)
    {
        // Extract the bitstream content of this box
        FourCCInt boxType;
        BitStream subBitstr               = bitstr.readSubBoxBitStream(boxType);
        TrackReferenceTypeBox trefTypeBox = TrackReferenceTypeBox(boxType);
        trefTypeBox.parseBox(subBitstr);

        mTrefTypeBoxes.push_back(trefTypeBox);
    }
}
