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
#include "AlternativeTrackGroup.h"
#include "Track.h"

using namespace HEIFPP;

AlternativeTrackGroup::AlternativeTrackGroup(Heif* aHeif)
    : mHeif(aHeif)
    , mContext(nullptr)
{
    mHeif->addAlternativeTrackGroup(this);
}
AlternativeTrackGroup::~AlternativeTrackGroup()
{
    // remove tracks from the group.
    while (!mTracks.empty())
    {
        (*mTracks.begin())->setAlternativeTrackGroup(nullptr);
    }
    mHeif->removeAlternativeTrackGroup(this);
}
/** Sets the custom user data
 *  @param [in] aContext Pointer to the custom data */
void AlternativeTrackGroup::setContext(const void* aContext)
{
    mContext = aContext;
}

/** Returns a pointer to the custom user data. */
const void* AlternativeTrackGroup::getContext() const
{
    return mContext;
}


/** Returns the parent HEIF object of the item */
Heif* AlternativeTrackGroup::getHeif()
{
    return mHeif;
}
const Heif* AlternativeTrackGroup::getHeif() const
{
    return mHeif;
}

std::uint32_t AlternativeTrackGroup::getTrackCount() const
{
    return (std::uint32_t) mTracks.size();
}
Track* AlternativeTrackGroup::getTrack(std::uint32_t aId)
{
    if (aId < mTracks.size())
    {
        return mTracks[aId];
    }
    return nullptr;
}
const Track* AlternativeTrackGroup::getTrack(std::uint32_t aId) const
{
    if (aId < mTracks.size())
    {
        return mTracks[aId];
    }
    return nullptr;
}
Result AlternativeTrackGroup::addTrack(Track* aTrack)
{
    AlternativeTrackGroup* tmp = aTrack->getAlternativeTrackGroup();
    if ((tmp) && (tmp != this))
    {
        // Fail. already in a group.
        return Result::ALREADY_IN_GROUP;
    }
    if (AddItemTo(mTracks, aTrack))
    {
        aTrack->setAlternativeTrackGroup(this);
    }
    return Result::OK;
}
void AlternativeTrackGroup::removeTrack(Track* aTrack)
{
    if (RemoveItemFrom(mTracks, aTrack))
    {
        aTrack->setAlternativeTrackGroup(nullptr);
    }
}
std::vector<Track*>::iterator AlternativeTrackGroup::begin()
{
    return mTracks.begin();
}
std::vector<Track*>::iterator AlternativeTrackGroup::end()
{
    return mTracks.end();
}
