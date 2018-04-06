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

#include "groupslistbox.hpp"
#include "log.hpp"

GroupsListBox::GroupsListBox()
    : Box("grpl")
    , mEntityToGroupBoxVector()
{
}

void GroupsListBox::addEntityToGroupBox(const EntityToGroupBox& box)
{
    mEntityToGroupBoxVector.push_back(box);
}

const Vector<EntityToGroupBox>& GroupsListBox::getEntityToGroupsBoxes() const
{
    return mEntityToGroupBoxVector;
}

void GroupsListBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    if (mEntityToGroupBoxVector.size() == 0)
    {
        return;
    }

    writeBoxHeader(bitstr);
    for (auto& entityToGroupBox : mEntityToGroupBoxVector)
    {
        entityToGroupBox.writeBox(bitstr);
    }

    updateSize(bitstr);
}

void GroupsListBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        FourCCInt boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        EntityToGroupBox entityToGroupBox;
        entityToGroupBox.parseBox(subBitstr);
        mEntityToGroupBoxVector.push_back(entityToGroupBox);
    }
}
