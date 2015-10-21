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

#include "entitytogroupbox.hpp"
#include "idspace.hpp"

EntityToGroupBox::EntityToGroupBox() :
    FullBox("altr", 0, 0),
    mGroupId(IdSpace::getValue()),
    mEntityIds()
{
}

void EntityToGroupBox::setEntityIds(const EntityIds& entityIds)
{
    mEntityIds = entityIds;
}

EntityToGroupBox::EntityIds EntityToGroupBox::getEntityIds() const
{
    return mEntityIds;
}

void EntityToGroupBox::writeBox(BitStream& bitstr)
{
    writeFullBoxHeader(bitstr);
    bitstr.write32Bits(mGroupId);
    bitstr.write32Bits(mEntityIds.size());
    for (const auto entityId : mEntityIds)
    {
        bitstr.write32Bits(entityId);
    }
    updateSize(bitstr);
}

void EntityToGroupBox::parseBox(BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    mGroupId = bitstr.read32Bits();
    const uint32_t numEntitiesInGroup = bitstr.read32Bits();
    for (unsigned int i = 0; i < numEntitiesInGroup; ++i)
    {
        mEntityIds.push_back(bitstr.read32Bits());
    }
}
