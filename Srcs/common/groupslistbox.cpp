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

#include "groupslistbox.hpp"
#include "log.hpp"

GroupsListBox::GroupsListBox():
    Box("grpl"),
    mEntityToGroupBoxVector()
{
}

void GroupsListBox::addEntityToGroupBox(const EntityToGroupBox& box)
{
    mEntityToGroupBoxVector.push_back(box);
}

const std::vector<EntityToGroupBox>& GroupsListBox::getEntityToGroupsBoxes() const
{
    return mEntityToGroupBoxVector;
}

void GroupsListBox::writeBox(BitStream& bitstr)
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

void GroupsListBox::parseBox(BitStream& bitstr)
{
    parseBoxHeader(bitstr);

    while (bitstr.numBytesLeft() > 0)
    {
        // Extract contained box bitstream and type
        std::string boxType;
        BitStream subBitstr = bitstr.readSubBoxBitStream(boxType);

        // Handle this box based on the type
        if (boxType == "altr")
        {
            EntityToGroupBox entityToGroupBox;
            entityToGroupBox.parseBox(subBitstr);
            mEntityToGroupBoxVector.push_back(entityToGroupBox);
        }
        else
        {
            logWarning() << "Discarding unknown box in GroupsListBox of type '" << boxType << "'" << std::endl;
        }
    }
}
