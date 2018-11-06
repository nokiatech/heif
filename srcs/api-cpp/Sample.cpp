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

#include "Sample.h"
#include <cstring>
#include "DecoderConfiguration.h"
#include "EntityGroup.h"
#include "H26xTools.h"
#include "MetaItem.h"
#include "Track.h"
#include "heifreader.h"
#include "heifwriter.h"

using namespace HEIFPP;

Sample::Sample(Heif* aHeif)
    : mHeif(aHeif)
    , mType(HEIF::FourCC((uint32_t) 0))
    , mId(Heif::InvalidSequenceImage)
    , mSampleType(HEIF::SampleType::OUTPUT_REFERENCE_FRAME)
    , mDuration(0)
    , mCompositionOffset(0)
    , mConfig(0)
    , mIsAudio(false)
    , mIsVideo(false)
    , mMetaItems()
    , mTrack(nullptr)
    , mDecodeDependency()
    , mDecodeDependencyLinks()
    , mGroups()
    , mBufferSize(0)
    , mBuffer(nullptr)
    , mContext(nullptr)
{
    mHeif->addSample(this);
}
Sample::~Sample()
{
    if (mTrack)
    {
        mTrack->setSample(this, nullptr);
    }
    for (MetaItem*& meta : mMetaItems)
    {
        if (meta)
        {
            meta->unlink(this);
            meta = nullptr;
        }
    }
    // disconnect from groups
    for (; !mGroups.empty();)
    {
        (*mGroups.begin())->removeSample(this);
    }
    mMetaItems.clear();
    setDecoderConfiguration(nullptr);

    mHeif->removeSample(this);
    delete[] mBuffer;
    mBuffer     = nullptr;
    mBufferSize = 0;
}

void Sample::setContext(const void* aContext)
{
    mContext = aContext;
}

const void* Sample::getContext() const
{
    return mContext;
}

void Sample::link(Sample* aSample)
{
    mDecodeDependencyLinks.addLink(aSample);
}
void Sample::unlink(Sample* aSample)
{
    mDecodeDependencyLinks.removeLink(aSample);
}

void Sample::link(Track* aTrack)
{
    if (mTrack)
    {
        unlink(mTrack);
    }
    mTrack = aTrack;
}
void Sample::unlink(Track* aTrack)
{
    if (mTrack == aTrack)
    {
        mTrack = nullptr;
    }
}

std::uint32_t Sample::getDecodeDependencyCount() const
{
    return (std::uint32_t) mDecodeDependency.size();
}
void Sample::setDecodeDependency(uint32_t aId, Sample* aSample)
{
    if (aId < mDecodeDependency.size())
    {
        if (mDecodeDependency[aId])
        {
            mDecodeDependency[aId]->unlink(this);
        }
        mDecodeDependency[aId] = aSample;
    }
}

Sample* Sample::getDecodeDependency(uint32_t aId)
{
    if (aId < mDecodeDependency.size())
        return mDecodeDependency[aId];
    return nullptr;
}
const Sample* Sample::getDecodeDependency(uint32_t aId) const
{
    if (aId < mDecodeDependency.size())
        return mDecodeDependency[aId];
    return nullptr;
}

void Sample::addDecodeDependency(Sample* aSample)
{
    if (aSample == this)
    {
        // ignore self reference.
        return;
    }
    // Use AddItemTo helper to make sure that the sample is added only once.
    if (AddItemTo(mDecodeDependency, aSample))
    {
        aSample->link(this);
    }
}

void Sample::removeDecodeDependency(std::uint32_t aId)
{
    Sample* smp = getDecodeDependency(aId);
    if (smp)
    {
        smp->unlink(this);
        mDecodeDependency.erase(mDecodeDependency.begin() + (std::int32_t) aId);
    }
}

void Sample::removeDecodeDependency(Sample* aSample)
{
    if (aSample == nullptr)
        return;
    if (RemoveItemFrom(mDecodeDependency, aSample))
    {
        aSample->unlink(this);
    }
}


std::uint32_t Sample::getMetadataCount() const
{
    return (uint32_t) mMetaItems.size();
}

MetaItem* Sample::getMetadata(uint32_t aIndex)
{
    if (aIndex >= mMetaItems.size())
        return nullptr;
    return mMetaItems[aIndex];
}

const MetaItem* Sample::getMetadata(uint32_t aIndex) const
{
    if (mMetaItems.size() >= aIndex)
        return nullptr;
    return mMetaItems[aIndex];
}

void Sample::addMetadata(MetaItem* aMeta)
{
    if (aMeta == nullptr)
        return;
    if (AddItemTo(mMetaItems, aMeta))
    {
        aMeta->link(this);
    }
}

void Sample::removeMetadata(MetaItem* aMeta)
{
    if (aMeta == nullptr)
        return;
    if (RemoveItemFrom(mMetaItems, aMeta))
    {
        aMeta->unlink(this);
    }
}
bool Sample::isAudio() const
{
    return mIsAudio;
}
bool Sample::isVideo() const
{
    return mIsVideo;
}
void Sample::setId(const HEIF::SequenceImageId& aId)
{
    mId = aId;
}
void Sample::setType(const HEIF::FourCC& aType)
{
    mType = aType;
}

const HEIF::FourCC& Sample::getType() const
{
    return mType;
}

void Sample::setCompositionOffset(std::int64_t aOffset)
{
    mCompositionOffset = aOffset;
}
std::int64_t Sample::getCompositionOffset() const
{
    return mCompositionOffset;
}


const HEIF::FourCC& Sample::getDecoderCodeType() const
{
    return mType;
}
void Sample::setSampleType(const HEIF::SampleType& aType)
{
    mSampleType = aType;
}
const HEIF::SampleType& Sample::getSampleType() const
{
    return mSampleType;
}
const HEIF::SequenceImageId& Sample::getId() const
{
    return mId;
}
void Sample::setDuration(std::uint64_t aDuration)
{
    mDuration = aDuration;
}
std::uint64_t Sample::getDuration() const
{
    return mDuration;
}
std::uint32_t Sample::getTimeStampCount() const
{
    return (std::uint32_t) mTimeStamps.size();
}
std::uint64_t Sample::getTimeStamp(uint32_t aIndex) const
{
    if (aIndex < mTimeStamps.size())
    {
        return mTimeStamps[aIndex];
    }
    return 0;
}

HEIF::ErrorCode Sample::loadSampleData(const HEIF::SequenceId& aTrackId)
{
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;
    if (mBufferSize == 0)
    {
        // No data in sample, warn user?
    }
    else
    {
        if ((getHeif() != nullptr) && (getHeif()->getReaderInstance() != nullptr) && (mConfig != nullptr))
        {
            delete[] mBuffer;
            mBuffer = nullptr;
            mBuffer = new std::uint8_t[mBufferSize];
            error   = getHeif()->getReaderInstance()->getItemData(aTrackId, getId(), mBuffer, mBufferSize, false);
            if (HEIF::ErrorCode::OK != error)
            {
                // Could not get the data. fail.
                mBufferSize = 0;
                delete[] mBuffer;
                mBuffer = nullptr;
            }
            else
            {
                switch (mConfig->getMediaFormat())
                {
                case HEIF::MediaFormat::AVC:
                case HEIF::MediaFormat::HEVC:
                {
                    error = NAL_State::convertToByteStream(mBuffer, mBufferSize) ? HEIF::ErrorCode::OK
                                                                                 : HEIF::ErrorCode::MEDIA_PARSING_ERROR;
                    break;
                }
                default:
                {
                    break;
                }
                }
            }
        }
        else
        {
            HEIF_ASSERT(false);
        }
    }
    return error;
}

/** Sets the item data for the image
 * @param [in] aData: A pointer to the data.
 * @param [in] aLength: The amount of data. */
void Sample::setItemData(const std::uint8_t* aData, std::uint64_t aLength)
{
    mBufferSize = aLength;
    delete[] mBuffer;
    mBuffer = nullptr;
    mBuffer = new std::uint8_t[aLength];
    std::memcpy(mBuffer, aData, mBufferSize);
}

/** Returns the size of the sample data */
std::uint64_t Sample::getSampleDataSize() const
{
    return mBufferSize;
}

/** Returns a pointer to the sample data */
const std::uint8_t* Sample::getSampleData()
{
    if (mBuffer == nullptr)
    {
        loadSampleData(mTrack->getId());
    }
    return mBuffer;
}


DecoderConfig* Sample::getDecoderConfiguration()
{
    return mConfig;
}

const DecoderConfig* Sample::getDecoderConfiguration() const
{
    return mConfig;
}
HEIF::ErrorCode Sample::setDecoderConfiguration(DecoderConfig* aConfig)
{
    if (mConfig)
        mConfig->unlink(this);
    if (aConfig)
    {
        if (aConfig->getMediaType() != mType)
        {
            return HEIF::ErrorCode::DECODER_CONFIGURATION_ERROR;
        }
    }
    mConfig = aConfig;
    if (mConfig)
    {
        mConfig->link(this);
    }
    return HEIF::ErrorCode::OK;
}

HEIF::ErrorCode Sample::load(HEIF::Reader* aReader,
                             const HEIF::SequenceId& aTrackId,
                             const HEIF::SampleInformation& aInfo)
{
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;
    aReader->getDecoderCodeType(aTrackId, aInfo.sampleId, mType);
    DecoderConfig* config = mHeif->constructDecoderConfig(aReader, Heif::InvalidItem, aTrackId, aInfo.sampleId, error);
    if (error != HEIF::ErrorCode::OK || (setDecoderConfiguration(config) != HEIF::ErrorCode::OK))
    {
        return error;
    }
    mDuration   = aInfo.sampleDurationTS;
    mSampleType = aInfo.sampleType;
    HEIF::Array<std::int64_t> st;
    aReader->getTimestampsOfItem(aTrackId, aInfo.sampleId, st);
    mTimeStamps.reserve(st.size);
    for (auto t : st)
    {
        mTimeStamps.push_back(static_cast<std::uint64_t>(t));
    }
    mCompositionOffset = aInfo.sampleCompositionOffsetTs;

    mBufferSize = aInfo.size;
    if (getHeif()->mPreLoadMode == Heif::PreloadMode::LOAD_ALL_DATA)
    {
        error = loadSampleData(aTrackId);
    }
    return error;
}

HEIF::ErrorCode Sample::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode err = HEIF::ErrorCode::OK;
    HEIF::MediaDataId mediaDataId;
    HEIF::SampleInfo sampleInfo;
    std::uint8_t* aData = nullptr;
    std::uint64_t aSize = 0;
    HEIF::Data data;

    data.mediaFormat = mConfig->getMediaFormat();
    switch (data.mediaFormat)
    {
    case HEIF::MediaFormat::AVC:
    case HEIF::MediaFormat::HEVC:
    {
        err = NAL_State::convertFromByteStream(mBuffer, mBufferSize, aData, aSize)
                  ? HEIF::ErrorCode::OK
                  : HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        data.data = aData;
        data.size = mBufferSize;
        break;
    }
    default:
    {
        data.data = mBuffer;
        data.size = mBufferSize;
        break;
    }
    }
    if (HEIF::ErrorCode::OK != err)
    {
        return err;
    }
    if (mConfig)
    {
        if (mConfig->getId() == Heif::InvalidDecoderConfig)
        {
            err = mConfig->save(aWriter);
            if (HEIF::ErrorCode::OK != err)
                return err;
        }
        data.decoderConfigId = mConfig->getId();
    }
    else
    {
        return HEIF::ErrorCode::INVALID_DECODER_CONFIG_ID;
    }

    sampleInfo.duration          = mDuration;  ///< duration of sample in ImageSequence timeBase units.
    sampleInfo.compositionOffset = mCompositionOffset;
    sampleInfo.isSyncSample      = (mSampleType == HEIF::OUTPUT_REFERENCE_FRAME) ||
                              (mSampleType == HEIF::NON_OUTPUT_REFERENCE_FRAME);  ///< whether sample is sync sample
    if (!mDecodeDependency.empty())
    {
        sampleInfo.referenceSamples = HEIF::Array<HEIF::SequenceImageId>(
            mDecodeDependency.size());  ///< list of samples that this sample has direct decode dependency on.
        sampleInfo.referenceSamples.size = 0;
        for (auto ref : mDecodeDependency)
        {
            if (ref->getId() == Heif::InvalidSequenceImage)
            {
                // samples should always refer to previous samples.
                // so it is not possible to have a reference to a sample not written yet.
                return HEIF::ErrorCode::INVALID_SEQUENCE_IMAGE_ID;
            }
            sampleInfo.referenceSamples[sampleInfo.referenceSamples.size] = ref->getId();
            sampleInfo.referenceSamples.size++;
        }
    }


    err = aWriter->feedMediaData(data, mediaDataId);

    // free temp buffer
    delete[] aData;

    if (HEIF::ErrorCode::OK != err)
        return err;

    if (mTrack->mFeatures & HEIF::TrackFeatureEnum::IsAudioTrack)
    {
        err = aWriter->addAudio(mTrack->getId(), mediaDataId, sampleInfo, mId);
    }
    else if (mTrack->mFeatures & HEIF::TrackFeatureEnum::IsVideoTrack)
    {
        err = aWriter->addVideo(mTrack->getId(), mediaDataId, sampleInfo, mId);
    }
    else
    {
        err = aWriter->addImage(mTrack->getId(), mediaDataId, sampleInfo, mId);
    }

    if (HEIF::ErrorCode::OK != err)
        return err;

    // add meta item links here.
    if (!mMetaItems.empty())
    {
        for (auto meta : mMetaItems)
        {
            err = aWriter->addMetadataItemReference(HEIF::MetadataItemId(meta->getId().get()), mTrack->getId(), mId);
            if (HEIF::ErrorCode::OK != err)
                return err;
        }
    }

    return err;
}

Heif* Sample::getHeif()
{
    return mHeif;
}
const Heif* Sample::getHeif() const
{
    return mHeif;
}

Track* Sample::getTrack()
{
    return mTrack;
}
const Track* Sample::getTrack() const
{
    return mTrack;
}

void Sample::addToGroup(EntityGroup* aGroup)
{
    AddItemTo(mGroups, aGroup);
}
void Sample::removeFromGroup(EntityGroup* aGroup)
{
    RemoveItemFrom(mGroups, aGroup);
}
std::uint32_t Sample::getGroupCount() const
{
    return (std::uint32_t) mGroups.size();
}
EntityGroup* Sample::getGroup(uint32_t aId)
{
    if (aId < mGroups.size())
    {
        return mGroups[aId];
    }
    return nullptr;
}
std::uint32_t Sample::getGroupByTypeCount(const HEIF::FourCC& aType)
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
EntityGroup* Sample::getGroupByType(const HEIF::FourCC& aType, std::uint32_t aId)
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
EntityGroup* Sample::getGroupById(const HEIF::GroupId& aId)
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
