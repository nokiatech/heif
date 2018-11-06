/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */
#include "EqivGroup.h"

using namespace HEIFPP;

EquivalenceGroup::EquivalenceGroup(Heif* aHeif)
    : EntityGroup(aHeif, "eqiv")
{
}
void EquivalenceGroup::addSample(Sample* aSample, int16_t aOffset, uint16_t aMultiplier)
{
    std::int32_t index;
    if (addEntity(nullptr, nullptr, aSample, index))
    {
        mOffsets[aSample] = std::pair<std::int16_t, std::uint16_t>(aOffset, aMultiplier);
    }
}
std::int16_t EquivalenceGroup::getOffset(Sample* aItem)
{
    auto it = mOffsets.find(aItem);
    if (it != mOffsets.end())
    {
        return it->second.first;
    }
    return 0;
}
std::uint16_t EquivalenceGroup::getMultiplier(Sample* aItem)
{
    auto it = mOffsets.find(aItem);
    if (it != mOffsets.end())
    {
        return it->second.second;
    }
    return 0;
}
void EquivalenceGroup::removeEntity(std::uint32_t aIndex)
{
    if (aIndex < getEntityCount())
    {
        Sample* sample = getSample(aIndex);
        if (sample)
        {
            mOffsets.erase(sample);
        }
        EntityGroup::removeEntity(aIndex);
    }
}
void EquivalenceGroup::removeSample(Sample* aSample)
{
    std::int32_t index;
    if (EntityGroup::removeEntity(nullptr, nullptr, aSample, index))
    {
        mOffsets.erase(aSample);
    }
}
