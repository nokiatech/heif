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
#include "EntityGroup.h"
#include "Item.h"
#include "Sample.h"
#include "Track.h"

using namespace HEIFPP;

EntityGroup::Entity::Entity(Item* aItem)
    : mItem(aItem)
    , mSample(nullptr)
    , mTrack(nullptr)
{
}
EntityGroup::Entity::Entity(Sample* aSample)
    : mItem(nullptr)
    , mSample(aSample)
    , mTrack(nullptr)
{
}
EntityGroup::Entity::Entity(Track* aTrack)
    : mItem(nullptr)
    , mSample(nullptr)
    , mTrack(aTrack)
{
}
EntityGroup::Entity::~Entity()
{
}
bool EntityGroup::Entity::isItem() const
{
    return (mItem != nullptr);
}
bool EntityGroup::Entity::isTrack() const
{
    return (mTrack != nullptr);
}
bool EntityGroup::Entity::isSample() const
{
    return (mSample != nullptr);
}
Item* EntityGroup::Entity::item()
{
    return mItem;
}
Track* EntityGroup::Entity::track()
{
    return mTrack;
}
Sample* EntityGroup::Entity::sample()
{
    return mSample;
}
const Item* EntityGroup::Entity::item() const
{
    return mItem;
}
const Track* EntityGroup::Entity::track() const
{
    return mTrack;
}
const Sample* EntityGroup::Entity::sample() const
{
    return mSample;
}

void EntityGroup::Entity::addToGroup(EntityGroup* aGroup)
{
    if (mItem)
        mItem->addToGroup(aGroup);
    if (mTrack)
        mTrack->addToGroup(aGroup);
    if (mSample)
        mSample->addToGroup(aGroup);
}
void EntityGroup::Entity::removeFromGroup(EntityGroup* aGroup)
{
    if (mItem)
        mItem->removeFromGroup(aGroup);
    if (mTrack)
        mTrack->removeFromGroup(aGroup);
    if (mSample)
        mSample->removeFromGroup(aGroup);
}

EntityGroup::EntityGroup(Heif* aHeif, const HEIF::FourCC& aType)
    : mHeif(aHeif)
    , mId(Heif::InvalidGroup)
    , mType(aType)
    , mContext(nullptr)
{
    mHeif->addGroup(this);
}
EntityGroup::~EntityGroup()
{
    while (!mItems.empty())
    {
        Entity& aItem = mItems[0];
        std::int32_t index;
        removeEntity(aItem.item(), aItem.track(), aItem.sample(), index);
    }
    mHeif->removeGroup(this);
}

void EntityGroup::setContext(const void* aContext)
{
    mContext = aContext;
}
const void* EntityGroup::getContext() const
{
    return mContext;
}

const HEIF::GroupId& EntityGroup::getId() const
{
    return mId;
}
const HEIF::FourCC& EntityGroup::getType() const
{
    return mType;
}
std::uint32_t EntityGroup::getEntityCount() const
{
    return (std::uint32_t) mItems.size();
}
bool EntityGroup::isItem(std::uint32_t aIndex) const
{
    if (aIndex < mItems.size())
    {
        return mItems[aIndex].isItem();
    }
    return false;
}
bool EntityGroup::isTrack(std::uint32_t aIndex) const
{
    if (aIndex < mItems.size())
    {
        return mItems[aIndex].isTrack();
    }
    return false;
}
bool EntityGroup::isSample(std::uint32_t aIndex) const
{
    if (aIndex < mItems.size())
    {
        return mItems[aIndex].isSample();
    }
    return false;
}
Item* EntityGroup::getItem(std::uint32_t aIndex)
{
    if (isItem(aIndex))
        return mItems[aIndex].item();
    return nullptr;
}
Track* EntityGroup::getTrack(std::uint32_t aIndex)
{
    if (isTrack(aIndex))
        return mItems[aIndex].track();
    return nullptr;
}
Sample* EntityGroup::getSample(std::uint32_t aIndex)
{
    if (isSample(aIndex))
        return mItems[aIndex].sample();
    return nullptr;
}
const Item* EntityGroup::getItem(std::uint32_t aIndex) const
{
    if (isItem(aIndex))
        return mItems[aIndex].item();
    return nullptr;
}
const Track* EntityGroup::getTrack(std::uint32_t aIndex) const
{
    if (isTrack(aIndex))
        return mItems[aIndex].track();
    return nullptr;
}
const Sample* EntityGroup::getSample(std::uint32_t aIndex) const
{
    if (isSample(aIndex))
        return mItems[aIndex].sample();
    return nullptr;
}
void EntityGroup::addItem(Item* aItem)
{
    std::int32_t index;
    if (addEntity(aItem, nullptr, nullptr, index))
    {
    }
}
void EntityGroup::addTrack(Track* aTrack)
{
    std::int32_t index;
    if (addEntity(nullptr, aTrack, nullptr, index))
    {
    }
}
void EntityGroup::addSample(Sample* aSample)
{
    std::int32_t index;
    if (addEntity(nullptr, nullptr, aSample, index))
    {
    }
}
bool EntityGroup::hasItems() const
{
    // TODO: instead of scanning, keep a count of types.
    for (auto& ent : mItems)
    {
        if (ent.isItem())
        {
            return true;
        }
    }
    return false;
}
bool EntityGroup::hasTracks() const
{
    // TODO: instead of scanning, keep a count of types.
    for (auto& ent : mItems)
    {
        if (ent.isSample())
        {
            return true;
        }
    }
    return false;
}
bool EntityGroup::hasSamples() const
{
    // TODO: instead of scanning, keep a count of types.
    for (auto& ent : mItems)
    {
        if (ent.isTrack())
        {
            return true;
        }
    }
    return false;
}

bool EntityGroup::addEntity(Item* aItem, Track* aTrack, Sample* aSample, std::int32_t& aIndex)
{
    // NOTE: only one of aItem,aTrack,aSample is allowed to be non-null.
    if ((aItem == nullptr) && (aTrack == nullptr) && (aSample == nullptr))
    {
        return false;
    }
    for (auto& ent : mItems)
    {
        if ((aItem) && (aItem == ent.item()))
        {
            return false;
        }
        if ((aTrack) && (aTrack == ent.track()))
        {
            return false;
        }
        if ((aSample) && (aSample == ent.sample()))
        {
            return false;
        }
    }
    // okay. does not exist yet so.
    if (aItem)
    {
        aItem->addToGroup(this);
        mItems.push_back(aItem);
    }
    else if (aTrack)
    {
        aTrack->addToGroup(this);
        mItems.push_back(aTrack);
    }
    else if (aSample)
    {
        aSample->addToGroup(this);
        mItems.push_back(aSample);
    }
    else
    {
        HEIF_ASSERT(false);
    }

    aIndex = (std::int32_t)(mItems.size() - 1);
    return true;
}
bool EntityGroup::removeEntity(Item* aItem, Track* aTrack, Sample* aSample, std::int32_t& aIndex)
{
    if ((aItem == nullptr) && (aTrack == nullptr) && (aSample == nullptr))
    {
        return false;
    }
    aIndex = 0;
    for (auto ent : mItems)
    {
        if ((aItem == ent.item()) && (aTrack == ent.track()) && (aSample == ent.sample()))
        {
            ent.removeFromGroup(this);
            mItems.erase(mItems.begin() + aIndex);
            return true;
        }
        aIndex++;
    }
    return false;
}


void EntityGroup::removeEntity(std::uint32_t aIndex)
{
    if (aIndex < mItems.size())
    {
        mItems[aIndex].removeFromGroup(this);
        mItems.erase(mItems.begin() + (std::int64_t) aIndex);
    }
}
void EntityGroup::removeItem(Item* aItem)
{
    std::int32_t index;
    removeEntity(aItem, nullptr, nullptr, index);
}
void EntityGroup::removeTrack(Track* aTrack)
{
    std::int32_t index;
    removeEntity(nullptr, aTrack, nullptr, index);
}
void EntityGroup::removeSample(Sample* aSample)
{
    std::int32_t index;
    removeEntity(nullptr, nullptr, aSample, index);
}

void EntityGroup::setId(const HEIF::GroupId& aId)
{
    mId = aId;
}
