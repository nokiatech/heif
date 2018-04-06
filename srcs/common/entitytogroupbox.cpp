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

#include "entitytogroupbox.hpp"

EntityToGroupBox::EntityToGroupBox()
    : EntityToGroupBox(0)
{
}

EntityToGroupBox::EntityToGroupBox(FourCCInt type)
    : FullBox(type, 0, 0)
    , mGroupId()
    , mEntityIds()
{
}

void EntityToGroupBox::setGroupId(const std::uint32_t groupId)
{
    mGroupId = groupId;
}

std::uint32_t EntityToGroupBox::getGroupId() const
{
    return mGroupId;
}

void EntityToGroupBox::setEntityIds(const EntityIds& entityIds)
{
    mEntityIds = entityIds;
}

EntityToGroupBox::EntityIds EntityToGroupBox::getEntityIds() const
{
    return mEntityIds;
}

void EntityToGroupBox::writeBox(ISOBMFF::BitStream& bitstr) const
{
    if (getType().getUInt32() == 0)
    {
        throw RuntimeError("Trying to write Entity To Group Box without grouping type.");
    }

    writeFullBoxHeader(bitstr);
    bitstr.write32Bits(mGroupId);
    bitstr.write32Bits(static_cast<std::uint32_t>(mEntityIds.size()));
    for (const auto entityId : mEntityIds)
    {
        bitstr.write32Bits(entityId);
    }
    updateSize(bitstr);
}

void EntityToGroupBox::parseBox(ISOBMFF::BitStream& bitstr)
{
    parseFullBoxHeader(bitstr);

    mGroupId                          = bitstr.read32Bits();
    const uint32_t numEntitiesInGroup = bitstr.read32Bits();
    for (unsigned int i = 0; i < numEntitiesInGroup; ++i)
    {
        mEntityIds.push_back(bitstr.read32Bits());
    }
}
