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

#include "trackreferencebox.hpp"
#include "log.hpp"

using namespace std;

TrackReferenceBox::TrackReferenceBox() :
    Box("tref"),
    mTrefTypeBoxes()
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

const std::vector<TrackReferenceTypeBox>& TrackReferenceBox::getTrefTypeBoxes() const
{
    return mTrefTypeBoxes;
}

bool TrackReferenceBox::isReferenceTypePresent(const std::string& type) const
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

void TrackReferenceBox::writeBox(BitStream& bitstr)
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

void TrackReferenceBox::parseBox(BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    // Read sub-boxes until no data is left in this box
    while (bitstr.numBytesLeft() > 0)
    {
        // Extract the bitstream content of this box
        string boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);
        TrackReferenceTypeBox trefTypeBox = TrackReferenceTypeBox(boxType.c_str());
        trefTypeBox.parseBox(subBitstr);

        mTrefTypeBoxes.push_back(trefTypeBox);
    }
}

