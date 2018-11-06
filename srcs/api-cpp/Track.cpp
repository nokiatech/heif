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

#include "Track.h"
#include "AlternativeTrackGroup.h"
#include "DecoderConfiguration.h"
#include "EntityGroup.h"
#include "MetaItem.h"
#include "Sample.h"
#include "VideoTrack.h"
#include "heifreader.h"
#include "heifwriter.h"
using namespace HEIFPP;


Track::Track(Heif* aHeif)
    : mHeif(aHeif)
    , mId(Heif::InvalidSequence)
    , mContext(nullptr)
    , mFeatures(0)
    , mDuration(0)
    , mTimeScale(0)
    , mMaxSampleSize(0)
    , mAltGroup(nullptr)
{
    mFeatures |= HEIF::TrackFeatureEnum::Feature::IsEnabled;
    mFeatures |= HEIF::TrackFeatureEnum::Feature::IsInMovie;
    // default edit list. (infinite loop)
    mEditList.mLooping     = true;
    mEditList.mRepetitions = 0;
    mHeif->addTrack(this);
}
Track::~Track()
{
    // Disconnect all samples
    for (Sample*& smp : mSamples)
    {
        if (smp)
        {
            smp->unlink(this);
            smp = nullptr;
        }
    }
    mSamples.clear();

    //disconnect from groups
    for (; !mGroups.empty();)
    {
        (*mGroups.begin())->removeTrack(this);
    }

    if (mAltGroup)
    {
        mAltGroup->removeTrack(this);
    }
    // remove refs also..
    mHeif->removeTrack(this);
}

HEIF::ErrorCode Track::load(HEIF::Reader* aReader, const HEIF::SequenceId& aId)
{
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;
    auto info             = mHeif->getTrackInformation(aId);
    mFeatures             = info->features;
    for (const auto& at : info->referenceTrackIds)
    {
        for (const auto& d : at.trackIds)
        {
            Track* track = mHeif->constructTrack(aReader, d, error);
            if (HEIF::ErrorCode::OK != error)
            {
                return error;
            }
            if (at.type == HEIF::FourCC("thmb"))
            {
                track->addThumbnail(this);
            }
            else if (at.type == HEIF::FourCC("auxl"))
            {
                track->addAux(this);
            }
        }
    }

    error = aReader->getPlaybackDurationInSecs(mId, mDuration);

    mMaxSampleSize = info->maxSampleSize;
    mTimeScale     = info->timeScale;
    // load samples..
    mSamples.resize(info->sampleProperties.size);
    for (uint32_t id = 0; id < info->sampleProperties.size; id++)
    {
        const auto& at = info->sampleProperties[id];
        Sample* sample = mHeif->constructSample(aReader, mId, at, error);
        if (HEIF::ErrorCode::OK != error)
        {
            return error;
        }
        setSample(id, sample);
    }
    // store the edit list..
    if (mFeatures & HEIF::TrackFeatureEnum::Feature::HasEditList)
    {
        auto edi               = info->editList;
        mEditList.mLooping     = edi.looping;
        mEditList.mRepetitions = edi.repetitions;
        for (auto e : edi.editUnits)
        {
            mEditList.mEditUnits.push_back(e);
        }
    }
    return error;
}
HEIF::ErrorCode Track::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;

    for (auto thmb : mThumbnail)
    {
        if (thmb->getId() == Heif::InvalidSequence)
        {
            error = thmb->save(aWriter);
            if (HEIF::ErrorCode::OK != error)
            {
                return error;
            }
        }
        aWriter->addThumbnails(thmb->getId(), mId);
    }

    if (mAltGroup)
    {
        for (auto alt : (*mAltGroup))
        {
            if (alt->getId() == Heif::InvalidSequence)
            {
                error = alt->save(aWriter);
                if (HEIF::ErrorCode::OK != error)
                {
                    return error;
                }
            }
            error = aWriter->setAlternateGrouping(mId, alt->getId());
            if (HEIF::ErrorCode::OK != error)
            {
                return error;
            }
        }
    }

    if (hasEditList())
    {
        HEIF::EditList edi;
        edi.looping        = mEditList.mLooping;
        edi.repetitions    = mEditList.mRepetitions;
        edi.editUnits      = HEIF::Array<HEIF::EditUnit>(mEditList.mEditUnits.size());
        edi.editUnits.size = 0;
        for (auto e : mEditList.mEditUnits)
        {
            edi.editUnits[edi.editUnits.size] = e;
            edi.editUnits.size++;
        }

        error = aWriter->setEditList(mId, edi);
    }
    return error;
}
std::uint32_t Track::getTimescale()
{
    return mTimeScale;
}
void Track::setTimescale(std::uint32_t aScale)
{
    mTimeScale = aScale;
}
std::uint32_t Track::getAlternativeTrackCount() const
{
    if (mAltGroup)
    {
        return mAltGroup->getTrackCount();
    }
    return 0;
}
const Track* Track::getAlternativeTrack(std::uint32_t aId) const
{
    if (mAltGroup)
    {
        return mAltGroup->getTrack(aId);
    }
    return nullptr;
}
Track* Track::getAlternativeTrack(std::uint32_t aId)
{
    if (mAltGroup)
    {
        return mAltGroup->getTrack(aId);
    }
    return nullptr;
}

void Track::setAlternativeTrackGroup(AlternativeTrackGroup* aAlt)
{
    if (mAltGroup)
    {
        // track can only belong to one group at a time.
        if (mAltGroup == aAlt)
        {
            // all ready IN that group.
            return;
        }
        mAltGroup->removeTrack(this);
    }
    mAltGroup = aAlt;
    if (mAltGroup)
    {
        mAltGroup->addTrack(this);
    }
}
AlternativeTrackGroup* Track::getAlternativeTrackGroup()
{
    return mAltGroup;
}
const AlternativeTrackGroup* Track::getAlternativeTrackGroup() const
{
    return mAltGroup;
}
bool Track::isMasterImageSequence() const  ///< Track handler type is 'pict', and the track is not referencing any
                                           ///< another track.
{
    if (mHandler != "pict")
    {
        return false;
    }
    // not video or audio track. must be imagesequence then?
    if (mIsRefTo.empty())
    {
        return true;
    }
    return false;
}
bool Track::isThumbnailImageSequence() const  ///< Is referencing another track with a 'thmb' type track reference.
{
    return !mIsThumbnailTo.empty();
}
bool Track::isAuxiliaryImageSequence() const  ///< Is referencing another track with an 'auxl' type track reference.
{
    return !mIsAuxiliaryTo.empty();
}
bool Track::isEnabled() const  ///< The track is enabled.
{
    return (mFeatures & HEIF::TrackFeatureEnum::Feature::IsEnabled) != 0;
}
bool Track::isInMovie() const  ///< The track is used in the presentation.
{
    return (mFeatures & HEIF::TrackFeatureEnum::Feature::IsInMovie) != 0;
}
bool Track::isInPreview() const  ///< The track is used when previewing the presentation.
{
    return (mFeatures & HEIF::TrackFeatureEnum::Feature::IsInPreview) != 0;
}
bool Track::hasAlternatives() const  ///< The track has alternative track or tracks.
{
    return (mAltGroup != nullptr);
}
bool Track::hasCodingConstraints() const  ///< From Coding Constraints Box in HevcSampleEntry
{
    return (mFeatures & HEIF::TrackFeatureEnum::Feature::HasCodingConstraints) != 0;
}
bool Track::hasSampleGroups() const  ///< The track has SampleToGroupBoxes.
{
    return (mFeatures & HEIF::TrackFeatureEnum::Feature::HasSampleGroups) != 0;
}
bool Track::hasLinkedAuxiliaryImageSequence() const  ///< There is a 'auxl' track reference pointing to this track.
{
    return !mAux.empty();
}
bool Track::hasLinkedThumbnailImageSequence() const  ///< There is a 'thmb' track reference pointing to this track.
{
    return !mThumbnail.empty();
}
bool Track::hasSampleToItemGrouping()
    const  ///< The track has one or more SampleToMetadataItemEntry ('stmi') sample groups present.
{
    // if anysample has metadata return true.
    return (mFeatures & HEIF::TrackFeatureEnum::Feature::HasSampleToItemGrouping) != 0;
}
bool Track::hasExifSampleEntry() const  ///< From SampleEntryBox, not implemented yet
{
    return (mFeatures & HEIF::TrackFeatureEnum::Feature::HasExifSampleEntry) != 0;
}
bool Track::hasXmlSampleEntry() const  ///< From SampleEntryBox, not implemented yet
{
    return (mFeatures & HEIF::TrackFeatureEnum::Feature::HasXmlSampleEntry) != 0;
}
bool Track::hasEditList() const  ///< Track has an edit list
{
    return !mEditList.mEditUnits.empty();
}
bool Track::hasInfiniteLoopPlayback() const  ///< Infinite looping has been set on in the edit list
{
    if (hasEditList())
    {
        return ((mEditList.mLooping) && (mEditList.mRepetitions == 0.0));
    }
    return false;
}
bool Track::hasSampleEquivalenceGrouping() const  ///< The track has one or more VisualSampleGroupEntry ('eqiv') sample
                                                  ///< group entries present.
{
    bool ret = false;
    // See if we have equivalence grouping.
    for (auto group : mGroups)
    {
        if (group->getType() == HEIF::FourCC("eqiv"))
        {
            // has equivalence group.
            // but does it have samples from this track?
            for (uint32_t i = 0; i < group->getEntityCount(); i++)
            {
                const Sample* smp = group->getSample(i);
                if (smp)
                {
                    if (smp->getTrack() == this)
                    {
                        ret = true;
                        break;
                    }
                }
            }
        }
    }
    if (ret != ((mFeatures & HEIF::TrackFeatureEnum::Feature::HasSampleEquivalenceGrouping) != 0))
    {
        HEIF_ASSERT(false);
    }
    return ret;
}
const HEIF::FourCC& Track::getHandler() const
{
    return mHandler;
}
bool Track::isAudioTrack() const  ///< Track is an audio track (handler type 'soun').
{
    return (mHandler == "soun");
}
bool Track::isVideoTrack() const  ///< Track is a video track (handler type 'vide').
{
    return (mHandler == "vide");
}
bool Track::isImageSequence() const
{
    return (mHandler == "pict");
}
bool Track::displayAllSamples() const  ///< Edit List presentation indicates 0 or 1 samples. The player should ignore
                                       ///< timestamps and display all non-hidden samples.
{
    return (mFeatures & HEIF::TrackFeatureEnum::Feature::DisplayAllSamples) != 0;
}

std::uint64_t Track::getMaxSampleSize()
{
    return mMaxSampleSize;
}

std::uint32_t Track::getReferenceCount() const
{
    std::uint32_t cnt = 0;
    for (auto t : mRefs)
    {
        cnt += (std::uint32_t) t.second.size();
    }
    return cnt;
}
const std::pair<const HEIF::FourCC, const Track*> Track::getReference(std::uint32_t aIndex) const
{
    std::uint32_t cnt = 0;
    for (auto t : mRefs)
    {
        if (aIndex < t.second.size())
        {
            return std::pair<const HEIF::FourCC, const Track*>(t.first, t.second[aIndex]);
        }
        cnt += (std::uint32_t) t.second.size();
        aIndex -= (std::uint32_t) t.second.size();
    }
    return std::pair<const HEIF::FourCC, const Track*>("", nullptr);
}
const std::pair<const HEIF::FourCC, Track*> Track::getReference(std::uint32_t aIndex)
{
    std::uint32_t cnt = 0;
    for (auto t : mRefs)
    {
        if (aIndex < t.second.size())
        {
            return std::pair<const HEIF::FourCC, Track*>(t.first, t.second[aIndex]);
        }
        cnt += (std::uint32_t) t.second.size();
        aIndex -= (std::uint32_t) t.second.size();
    }
    return std::pair<const HEIF::FourCC, Track*>("", nullptr);
}

std::uint32_t Track::getReferenceTypeCount() const
{
    return (std::uint32_t) mRefs.size();
}
const HEIF::FourCC Track::getReferenceType(std::uint32_t aIndex) const
{
    std::uint32_t cnt = 0;
    for (auto t : mRefs)
    {
        if (aIndex == cnt)
            return t.first;
        cnt++;
    }
    return "";
}
HEIF::FourCC Track::getReferenceType(std::uint32_t aIndex)
{
    std::uint32_t cnt = 0;
    for (auto t : mRefs)
    {
        if (aIndex == cnt)
            return t.first;
        cnt++;
    }
    return "";
}

std::uint32_t Track::getReferenceCount(const HEIF::FourCC& aType) const
{
    auto it = mRefs.find(aType);
    if (it != mRefs.end())
    {
        return (std::uint32_t) it->second.size();
    }
    return 0;
}
const Track* Track::getReference(const HEIF::FourCC& aType, std::uint32_t aIndex) const
{
    auto it = mRefs.find(aType);
    if (it != mRefs.end())
    {
        if (aIndex < it->second.size())
        {
            return it->second[aIndex];
        }
    }
    return nullptr;
}
Track* Track::getReference(const HEIF::FourCC& aType, std::uint32_t aIndex)
{
    auto it = mRefs.find(aType);
    if (it != mRefs.end())
    {
        if (aIndex < it->second.size())
        {
            return it->second[aIndex];
        }
    }
    return nullptr;
}


void Track::addReference(const HEIF::FourCC& aType, Track* aTrack)
{
    if (aTrack)
    {
        if (AddItemTo(mRefs[aType], aTrack))
        {
            aTrack->mIsRefTo[aType].addLink(this);
        }
    }
}
void Track::removeReference(const HEIF::FourCC& aType, Track* aTrack)
{
    if (aTrack)
    {
        if (RemoveItemFrom(mRefs[aType], aTrack))
        {
            aTrack->mIsRefTo[aType].addLink(this);
        }
    }
}

std::uint32_t Track::getThumbnailCount() const
{
    return (std::uint32_t) mThumbnail.size();
}
Track* Track::getThumbnail(uint32_t aIndex)
{
    if (aIndex < mThumbnail.size())
    {
        return mThumbnail[aIndex];
    }
    return nullptr;
}
const Track* Track::getThumbnail(uint32_t aIndex) const
{
    if (aIndex < mThumbnail.size())
    {
        return mThumbnail[aIndex];
    }
    return nullptr;
}

void Track::addThumbnail(Track* aTrack)
{
    if (aTrack)
    {
        if (AddItemTo(mThumbnail, aTrack))
        {
            aTrack->mIsThumbnailTo.addLink(this);
            addReference(HEIF::FourCC("thmb"), aTrack);
        }
    }
}

void Track::removeThumbnail(Track* aTrack)
{
    if (aTrack)
    {
        if (RemoveItemFrom(mThumbnail, aTrack))
        {
            aTrack->mIsThumbnailTo.removeLink(this);
            removeReference(HEIF::FourCC("thmb"), aTrack);
        }
    }
}

std::uint32_t Track::getAuxCount() const
{
    return (std::uint32_t) mAux.size();
}
Track* Track::getAux(uint32_t aIndex)
{
    if (aIndex < mAux.size())
    {
        return mAux[aIndex];
    }
    return nullptr;
}
const Track* Track::getAux(uint32_t aIndex) const
{
    if (aIndex < mAux.size())
    {
        return mAux[aIndex];
    }
    return nullptr;
}

void Track::addAux(Track* aTrack)
{
    if (aTrack)
    {
        if (AddItemTo(mAux, aTrack))
        {
            aTrack->mIsAuxiliaryTo.addLink(this);
            addReference(HEIF::FourCC("auxl"), aTrack);
        }
    }
}

void Track::removeAux(Track* aTrack)
{
    if (aTrack)
    {
        if (RemoveItemFrom(mAux, aTrack))
        {
            aTrack->mIsAuxiliaryTo.removeLink(this);
            removeReference(HEIF::FourCC("auxl"), aTrack);
        }
    }
}

std::uint32_t Track::getSampleCount() const
{
    return (std::uint32_t) mSamples.size();
}
Sample* Track::getSample(std::uint32_t aId)
{
    if (aId < mSamples.size())
    {
        return mSamples[aId];
    }
    return nullptr;
}
Sample* Track::getSample(std::uint32_t aId) const
{
    if (aId < mSamples.size())
    {
        return mSamples[aId];
    }
    return nullptr;
}
Sample* Track::getSampleByType(HEIF::TrackSampleType, std::uint32_t)
{
    return nullptr;
}
Sample* Track::getSampleByType(HEIF::TrackSampleType, std::uint32_t) const
{
    return nullptr;
}
void Track::setSample(std::uint32_t aId, Sample* aSample)
{
    if (aId < mSamples.size())
    {
        Sample*& s = mSamples.at(aId);
        if (s)
        {
            s->unlink(this);
        }
        s = aSample;
        if (s)
        {
            s->link(this);
        }
    }
}
void Track::setSample(Sample* aOldSample, Sample* aNewSample)
{
    for (Sample*& s : mSamples)
    {
        if (s == aOldSample)
        {
            if (s)
            {
                s->unlink(this);
            }
            s = aNewSample;
            if (s)
            {
                s->link(this);
            }
        }
    }
}

void Track::addSample(Sample* aSample)
{
    if (aSample == nullptr)
        return;

    if (AddItemTo(mSamples, aSample))
    {
        aSample->link(this);
    }
}

void Track::removeSample(Sample* aSample)
{
    if (RemoveItemFrom(mSamples, aSample))
    {
        aSample->unlink(this);
    }
}

double Track::getDuration()
{
    return mDuration;
}

void Track::setContext(const void* aContext)
{
    mContext = aContext;
}

const void* Track::getContext() const
{
    return mContext;
}

const HEIF::SequenceId& Track::getId() const
{
    return mId;
}

Heif* Track::getHeif()
{
    return mHeif;
}
const Heif* Track::getHeif() const
{
    return mHeif;
}

void Track::setId(const HEIF::SequenceId& aId)
{
    mId = aId;
}

void Track::addToGroup(EntityGroup* aGroup)
{
    AddItemTo(mGroups, aGroup);
}
void Track::removeFromGroup(EntityGroup* aGroup)
{
    RemoveItemFrom(mGroups, aGroup);
}
std::uint32_t Track::getGroupCount() const
{
    return (std::uint32_t) mGroups.size();
}
EntityGroup* Track::getGroup(uint32_t aId)
{
    if (aId < mGroups.size())
    {
        return mGroups[aId];
    }
    return nullptr;
}
std::uint32_t Track::getGroupByTypeCount(const HEIF::FourCC& aType)
{
    std::uint32_t cnt = 0;
    for (auto grp : mGroups)
    {
        if (grp->getType() == aType)
        {
            cnt++;
        }
    }
    return cnt;
}
EntityGroup* Track::getGroupByType(const HEIF::FourCC& aType, std::uint32_t aId)
{
    std::uint32_t cnt = 0;
    for (auto grp : mGroups)
    {
        if (grp->getType() == aType)
        {
            if (aId == cnt)
                return grp;
            cnt++;
        }
    }
    return nullptr;
}
EntityGroup* Track::getGroupById(const HEIF::GroupId& aId)
{
    for (auto grp : mGroups)
    {
        if (grp->getId() == aId)
        {
            return grp;
        }
    }
    return nullptr;
}
