/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#include "Heif.h"

#include <heifreader.h>
#include <heifwriter.h>

#include "AACDecoderConfiguration.h"
#include "AVCCodedImageItem.h"
#include "AVCDecoderConfiguration.h"
#include "AlternativeTrackGroup.h"
#include "AudioSample.h"
#include "AudioTrack.h"
#include "DescriptiveProperty.h"
#include "EntityGroup.h"
#include "EqivGroup.h"
#include "ErrorContext.h"
#include "ExifItem.h"
#include "GridImageItem.h"
#include "HEVCCodedImageItem.h"
#include "HEVCDecoderConfiguration.h"
#include "LHEVCDecoderConfiguration.h"
#include "IdentityImageItem.h"
#include "ImageSequence.h"
#include "JPEGCodedImageItem.h"
#include "JPEGDecoderConfiguration.h"
#include "MPEG7Item.h"
#include "OverlayImageItem.h"
#include "RawProperty.h"
#include "TransformativeProperty.h"
#include "VideoSample.h"
#include "VideoTrack.h"
#include "VvcCodedImageItem.h"
#include "VvcDecoderConfiguration.h"
#include "XMPItem.h"
#include "CodedImageItem.h"
#include "DecoderConfiguration.h"
#include "ImageItem.h"
#include "Item.h"
#include "ItemProperty.h"
#include "MetaItem.h"
#include "MimeItem.h"
#include "Sample.h"
#include "Track.h"
#include "heifcommondatatypes.h"
#include "heifreaderdatatypes.h"
#include "heifwriterdatatypes.h"
#include "helpers.h"
#include <exception>
#include <stddef.h>
#include <string>

using namespace HEIFPP;

const HEIF::GroupId Heif::InvalidGroup(static_cast<std::uint32_t>(0));
const HEIF::ImageId Heif::InvalidItem(static_cast<std::uint32_t>(0));
const HEIF::SequenceId Heif::InvalidSequence(static_cast<std::uint32_t>(~0));
const HEIF::SequenceImageId Heif::InvalidSequenceImage(static_cast<std::uint32_t>(~0));
const HEIF::PropertyId Heif::InvalidProperty(static_cast<std::uint32_t>(0));
const HEIF::DecoderConfigId Heif::InvalidDecoderConfig(static_cast<std::uint32_t>(0));
const HEIF::MediaDataId Heif::InvalidMediaData(static_cast<std::uint32_t>(0));

#define ISPE_AS_RAW_PROPERTY 0
#define DECODER_CONFIG_AS_RAW_PROPERTY 0


Heif::Heif()
    : mFileinfo{}
    , mMajorBrand()
    , 
     mCompatibleBrands()
    , mItems()
    , mItemsOfType()
    , mProperties()
    , mDecoderConfigs()
    , 
     mItemsLoad()
    , mPropertiesLoad()
    , mDecoderConfigsLoad()
     
{
}

Heif::~Heif()
{
    // OOM-safe teardown (S.10 / R.x companion). reset() walks the owned
    // child collections (mItems, mProperties, mTracks, ...) and re-routes
    // through customAllocate via the standard-container allocator path;
    // under bad_alloc-equivalent pressure that rebound allocator now
    // throws std::bad_alloc, and a destructor allowed to escape with an
    // active exception calls std::terminate. Swallow here so the OOM
    // contract ("do not crash, do not leak") holds at the C++ object
    // boundary the same way Reader::Destroy / Writer::Destroy hold it
    // at the C-tier boundary.
    try
    {
        reset();
    }
    catch (...)
    {
        // Best-effort cleanup: container state may be partial. The
        // owning process is tearing down anyway in OOM scenarios; the
        // remaining child objects are dropped when the std:: containers
        // themselves run their destructors below this frame.
    }
}

void Heif::reset()
{
    for (; !mAltGroups.empty();)
    {
        auto it = mAltGroups.begin();
        delete (*it);
    }
    for (; !mGroups.empty();)
    {
        auto it = mGroups.begin();
        delete (*it);
    }
    for (; !mTracks.empty();)
    {
        auto it = mTracks.begin();
        delete (*it);
    }
    for (; !mSamples.empty();)
    {
        auto it = mSamples.begin();
        delete (*it);
    }
    for (; !mItems.empty();)
    {
        auto it = mItems.begin();
        delete (*it);
    }
    for (; !mProperties.empty();)
    {
        auto it = mProperties.begin();
        delete (*it);
    }
    for (; !mDecoderConfigs.empty();)
    {
        auto it = mDecoderConfigs.begin();
        delete (*it);
    }
    mCompatibleBrands.clear();
    mMajorBrand  = HEIF::FourCC();
    mPrimaryItem = nullptr;
    mFileinfo    = HEIF::FileInformation();
    mMatrix[0]   = 0x10000;
    mMatrix[1]   = 0;
    mMatrix[2]   = 0;
    mMatrix[3]   = 0;
    mMatrix[4]   = 0x10000;
    mMatrix[5]   = 0;
    mMatrix[6]   = 0;
    mMatrix[7]   = 0;
    mMatrix[8]   = 0x40000000;

    if (mReader != nullptr)
    {
        HEIF::Reader::Destroy(mReader);
        mReader = nullptr;
    }
}

void Heif::setContext(const void* aContext)
{
    mContext = aContext;
}

const void* Heif::getContext() const
{
    return mContext;
}

Result Heif::save(const char* aFilename)
{
    return save(aFilename, nullptr);
}

Result Heif::save(HEIF::OutputStreamInterface* aStream)
{
    return save(nullptr, aStream);
}

Result Heif::save(const char* aFileName, HEIF::OutputStreamInterface* aStream)
{
    // Drain any stale error context from a previous call so that, if this
    // call succeeds, an embedder won't see a misleading "last error" from
    // an unrelated earlier failure on this thread.
    clearLastError();

    if (mMajorBrand == HEIF::FourCC())
    {
        return HEIFPP_FAIL(Result::BRANDS_NOT_SET, ErrorSource::Api_State,
                           "Heif::save: major brand has not been set (call setMajorBrand first)");
    }
    if (mPrimaryItem == nullptr)
    {
        if (mTracks.empty())
        {
            return HEIFPP_FAIL(Result::PRIMARY_ITEM_NOT_SET, ErrorSource::Api_State,
                               "Heif::save: no primary item set and no tracks present");
        }
    }
    else
    {
        if (mPrimaryItem->isHidden())
        {
            return HEIFPP_FAIL(Result::HIDDEN_PRIMARY_ITEM, ErrorSource::Api_State,
                               "Heif::save: primary item is hidden, cannot save");
        }
    }

    if (mReader != nullptr)
    {
        if (mPreLoadMode != PreloadMode::LOAD_ALL_DATA)
        {
            for (auto* item : mItems)
            {
                if (item->isImageItem() && static_cast<ImageItem*>(item)->isCodedImage())
                {
                    static_cast<CodedImageItem*>(item)->getItemData();
                }
                else if (mPreLoadMode == PreloadMode::LOAD_ON_DEMAND)
                {
                    if (item->isExifItem())
                    {
                        static_cast<ExifItem*>(item)->getData();
                    }
                    else if (item->isMimeItem())
                    {
                        static_cast<MimeItem*>(item)->getData();
                    }
                }
            }

            for (auto* track : mTracks)
            {
                uint32_t samplecount = track->getSampleCount();
                for (uint32_t index = 0; index < samplecount; index++)
                {
                    track->getSample(index)->getSampleData();
                }
            }
        }
        HEIF::Reader::Destroy(mReader);
        mReader = nullptr;
    }

    HEIF::ErrorCode error = HEIF::ErrorCode::OK;
    HEIF::Writer* writer  = HEIF::Writer::Create();
    // Cross-language firewall: mirrors the try/catch in Heif::load. The
    // writer C-API does not currently install a top-level handler, so any
    // ISOBMFF / std::exception that escapes the writer pipeline would
    // otherwise propagate into the host (JNI/Obj-C++/COM) and abort the
    // process. Catch here, destroy the writer, surface as ErrorContext.
    try
    {
    HEIF::OutputConfig output;
    output.fileName         = aFileName;
    output.outputStream     = aStream;
    output.majorBrand       = mMajorBrand;
    output.compatibleBrands = HEIF::Array<HEIF::FourCC>(mCompatibleBrands.size());
    output.progressiveFile  = false;
    for (std::uint32_t i = 0; i < mCompatibleBrands.size(); i++)
    {
        output.compatibleBrands[i] = mCompatibleBrands[i];
    }
    error = writer->initialize(output);
    if (HEIF::ErrorCode::OK == error)
    {
        // Invalidate all id's since writer will create new ones.
        for (auto* item : mItems)
        {
            // set an "invalid" id.
            item->setId(InvalidItem);
        }
        for (auto* property : mProperties)
        {
            // set an "invalid" id.
            property->setId(InvalidProperty);
        }
        for (auto* config : mDecoderConfigs)
        {
            // set an "invalid" id.
            config->setId(InvalidDecoderConfig);
        }
        for (auto* track : mTracks)
        {
            track->setId(InvalidSequence);
        }
        for (auto* sample : mSamples)
        {
            sample->setId(InvalidSequenceImage);
        }
        for (auto grp : mGroups)
        {
            grp->setId(InvalidGroup);
        }

        HEIF::Array<int32_t> matrix(9);
        for (size_t i = 0; i < 9; i++)
        {
            matrix[i] = mMatrix[i];
        }
        error = writer->setMatrix(matrix);


        if (HEIF::ErrorCode::OK == error)
        {
            for (auto* item : mItems)
            {
                if (item->getId() == Heif::InvalidItem)
                {
                    error = item->save(writer);
                    if (HEIF::ErrorCode::OK != error)
                    {
                        break;
                    }
                }
            }
        }
        if ((HEIF::ErrorCode::OK == error) && (mPrimaryItem))
        {
            error = writer->setPrimaryItem(mPrimaryItem->getId());
        }

        // write samples?/tracks?
        if (HEIF::ErrorCode::OK == error)
        {
            for (auto* track : mTracks)
            {
                if (track->getId() == Heif::InvalidSequence)
                {
                    error = track->save(writer);
                    if (HEIF::ErrorCode::OK != error)
                    {
                        break;
                    }
                }
            }
        }
        // save groups.
        if (HEIF::ErrorCode::OK == error)
        {
            for (auto grp : mGroups)
            {
                auto type = grp->getType();
                HEIF::GroupId gid;
                error = writer->createEntityGroup(type, gid);
                if (HEIF::ErrorCode::OK != error)
                {
                    break;
                }
                grp->setId(gid);

                for (std::uint32_t i = 0; i < grp->getEntityCount(); i++)
                {
                    if (grp->isItem(i))
                    {
                        error = writer->addToGroup(gid, grp->getItem(i)->getId());
                    }
                    else if (grp->isTrack(i))
                    {
                        error = writer->addToGroup(gid, grp->getTrack(i)->getId());
                    }
                    else if (grp->isSample(i))
                    {
                        Sample* smp = grp->getSample(i);
                        if (type == "eqiv")
                        {
                            HEIF::EquivalenceTimeOffset eqi;
                            eqi.timeOffset          = static_cast<EquivalenceGroup*>(grp)->getOffset(smp);
                            eqi.timescaleMultiplier = static_cast<EquivalenceGroup*>(grp)->getMultiplier(smp);
                            error = writer->addToEquivalenceGroup(gid, smp->getTrack()->getId(), smp->getId(), eqi);
                        }
                        else
                        {
                            // TODO: currently there is only one way to add samples to groups..
                        }
                    }
                    if (HEIF::ErrorCode::OK != error)
                    {
                        break;
                    }
                }
            }
        }

        if (HEIF::ErrorCode::OK == error)
        {
            error = writer->finalize();
        }
    }
    }
    catch (const std::exception& e)
    {
        // Writer pipeline threw past its own (non-existent) handlers.
        // Capture the cause and surface as a generic writer failure.
        HEIF::Writer::Destroy(writer);
        return HEIFPP_FAIL_CAUSED_BY(Result::ERROR_UNDEFINED,
                                     ErrorSource::Writer_BoxBuild,
                                     "Heif::save: writer threw while assembling output",
                                     e);
    }
    catch (...)
    {
        HEIF::Writer::Destroy(writer);
        HEIFPP::ErrorContext ctx;
        ctx.code      = Result::ERROR_UNDEFINED;
        ctx.source    = ErrorSource::Writer_BoxBuild;
        ctx.message   = "Heif::save: writer threw a non-std exception";
        ctx.causeWhat = "(unknown C++ exception)";
        ctx.file      = __FILE__;
        ctx.line      = __LINE__;
        ctx.valid     = true;
        HEIFPP::setLastError(ctx);
        return Result::ERROR_UNDEFINED;
    }
    HEIF::Writer::Destroy(writer);
    return HEIFPP_FAIL_CONVERTED(error, ErrorSource::Writer_FileIO,
                                 "Heif::save: writer pipeline returned an error");
}
const HEIF::FileInformation* Heif::getFileInformation() const
{
    return &mFileinfo;
}
const HEIF::ItemInformation* Heif::getItemInformation(const HEIF::ImageId& aItemId) const
{
    for (const auto& i : mFileinfo.rootMetaBoxInformation.itemInformations)
    {
        if (i.itemId == aItemId)
        {
            return &i;
        }
    }
    return nullptr;
}
const HEIF::TrackInformation* Heif::getTrackInformation(const HEIF::SequenceId& aItemId) const
{
    for (const auto& i : mFileinfo.trackInformation)
    {
        if (i.trackId == aItemId)
        {
            return &i;
        }
    }
    return nullptr;
}

Result Heif::load(const char* aFilename, PreloadMode loadMode)
{
    return load(aFilename, nullptr, loadMode);
}

Result Heif::load(HEIF::StreamInterface* aStream, PreloadMode loadMode)
{
    return load(nullptr, aStream, loadMode);
}
Result Heif::load(const char* aFilename, HEIF::StreamInterface* aStream, PreloadMode loadMode)
{
    // Drain stale per-thread error context so a successful load() does not
    // leave an embedder reading a context from an unrelated earlier failure.
    clearLastError();

    if (mMajorBrand != HEIF::FourCC())
    {
        return HEIFPP_FAIL(Result::ALREADY_INITIALIZED, ErrorSource::Api_State,
                           "Heif::load: instance already has a major brand; call reset() before reloading");
    }
    if (!mCompatibleBrands.empty())
    {
        return HEIFPP_FAIL(Result::ALREADY_INITIALIZED, ErrorSource::Api_State,
                           "Heif::load: instance already has compatible brands; call reset() before reloading");
    }
    if (!mItems.empty())
    {
        return HEIFPP_FAIL(Result::ALREADY_INITIALIZED, ErrorSource::Api_State,
                           "Heif::load: instance already has items; call reset() before reloading");
    }
    if (!mItemsOfType.empty())
    {
        return HEIFPP_FAIL(Result::ALREADY_INITIALIZED, ErrorSource::Api_State,
                           "Heif::load: instance already has typed items; call reset() before reloading");
    }
    if (!mProperties.empty())
    {
        return HEIFPP_FAIL(Result::ALREADY_INITIALIZED, ErrorSource::Api_State,
                           "Heif::load: instance already has properties; call reset() before reloading");
    }
    if (!mDecoderConfigs.empty())
    {
        return HEIFPP_FAIL(Result::ALREADY_INITIALIZED, ErrorSource::Api_State,
                           "Heif::load: instance already has decoder configs; call reset() before reloading");
    }
    if (mPrimaryItem)
    {
        return HEIFPP_FAIL(Result::ALREADY_INITIALIZED, ErrorSource::Api_State,
                           "Heif::load: instance already has a primary item; call reset() before reloading");
    }

    if (mReader != nullptr)
    {
        HEIF::Reader::Destroy(mReader);
        mReader = nullptr;
    }

    mReader               = HEIF::Reader::Create();
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;
    // The lower-level reader catches its own ISOBMFF / parser exceptions
    // and converts them to ErrorCode (the original what() string is logged
    // to stderr and discarded). Wrap the call in our own try/catch so that
    // any exception that DOES escape - e.g. a bug or a path the reader
    // does not currently guard - still lands in the ErrorContext with its
    // what() preserved as the cause. This is belt-and-braces: in the happy
    // case nothing is thrown and the reader-returned ErrorCode flows
    // through HEIFPP_FAIL_CONVERTED below.
    try
    {
        if (aStream)
        {
            error = mReader->initialize(aStream);
        }
        else
        {
            error = mReader->initialize(aFilename);
        }
        if (HEIF::ErrorCode::OK == error)
        {
            mPreLoadMode = loadMode;
            error        = load(mReader);
        }
    }
    catch (const std::exception& e)
    {
        // Reader threw past its own catch handlers. Capture cause and
        // surface as FILE_READ_ERROR so existing callers' switch
        // statements continue to work.
        reset();
        if (mPreLoadMode == PreloadMode::LOAD_ALL_DATA && mReader)
        {
            HEIF::Reader::Destroy(mReader);
            mReader = nullptr;
        }
        return HEIFPP_FAIL_CAUSED_BY(Result::FILE_READ_ERROR,
                                     ErrorSource::Reader_BoxParse,
                                     aStream ? "Heif::load: reader threw while parsing stream"
                                             : "Heif::load: reader threw while parsing file",
                                     e);
    }
    catch (...)
    {
        // Non-std exception escaped the reader. Same cleanup as above
        // but synthesise an ErrorContext directly since we have no
        // std::exception to feed HEIFPP_FAIL_CAUSED_BY.
        reset();
        if (mPreLoadMode == PreloadMode::LOAD_ALL_DATA && mReader)
        {
            HEIF::Reader::Destroy(mReader);
            mReader = nullptr;
        }
        HEIFPP::ErrorContext ctx;
        ctx.code      = Result::FILE_READ_ERROR;
        ctx.source    = ErrorSource::Reader_BoxParse;
        ctx.message   = aStream ? "Heif::load: reader threw a non-std exception (stream)"
                                : "Heif::load: reader threw a non-std exception (file)";
        ctx.causeWhat = "(unknown C++ exception)";
        ctx.file      = __FILE__;
        ctx.line      = __LINE__;
        ctx.valid     = true;
        HEIFPP::setLastError(ctx);
        return Result::FILE_READ_ERROR;
    }
    mItemsLoad.clear();
    mTracksLoad.clear();
    mPropertiesLoad.clear();
    mDecoderConfigsLoad.clear();
    mSamplesLoad.clear();
    mGroupsLoad.clear();
    mAltGroupsLoad.clear();
    if (HEIF::ErrorCode::OK != error)
    {
        // clean up on error.
        reset();
    }

    if (mPreLoadMode == PreloadMode::LOAD_ALL_DATA)
    {
        // Cleanup ordering note (audit P2): the *Load lookup maps above
        // were drained BEFORE we destroy the reader on purpose. Those
        // maps store raw HEIFPP wrapper pointers keyed by reader-side
        // ids (mItemsLoad: ImageId -> Item*, etc.); they are scratch
        // state used only during load() to wire references between
        // wrappers as the box tree is walked. Once load() returns
        // (or fails) the maps are dead, but the wrapper objects they
        // point to are owned by the persistent mItems / mTracks / ...
        // vectors and survive past Reader::Destroy. Order is therefore:
        //   1. Drain *Load maps         (above)
        //   2. reset() if errored       (above; tears down wrapper objects)
        //   3. Destroy the reader       (here, in LOAD_ALL_DATA mode only)
        // In LOAD_ON_DEMAND / LOAD_PREVIEW_DATA modes the reader is kept
        // alive past load() so subsequent getItemData() / getSampleData()
        // calls can still page bytes from the underlying stream; it is
        // destroyed in save() / dtor / reset() instead.
        HEIF::Reader::Destroy(mReader);
        mReader = nullptr;
    }

    // Pick a source category from the ErrorCode so embedders can branch
    // on "file not found" vs "file corrupt" without parsing strings.
    ErrorSource src = ErrorSource::Reader_BoxParse;
    switch (error)
    {
        case HEIF::ErrorCode::FILE_OPEN_ERROR:
        case HEIF::ErrorCode::FILE_READ_ERROR:
        case HEIF::ErrorCode::FILE_HEADER_ERROR:
        case HEIF::ErrorCode::BYTESTREAM_READ_ERROR:
            src = ErrorSource::Reader_FileIO;
            break;
        case HEIF::ErrorCode::DECODER_CONFIGURATION_ERROR:
            src = ErrorSource::Reader_DecoderConfig;
            break;
        case HEIF::ErrorCode::MEDIA_PARSING_ERROR:
        case HEIF::ErrorCode::INVALID_MEDIA_FORMAT:
        case HEIF::ErrorCode::UNSUPPORTED_CODE_TYPE:
            src = ErrorSource::Reader_BoxParse;
            break;
        default:
            break;
    }
    return HEIFPP_FAIL_CONVERTED(error, src,
                                 aStream ? "Heif::load: reader pipeline returned an error (stream)"
                                         : "Heif::load: reader pipeline returned an error (file)");
}

HEIF::ErrorCode Heif::load(HEIF::Reader* aReader)
{
    HEIF::ErrorCode error;
    error = aReader->getMajorBrand(mMajorBrand);
    if (HEIF::ErrorCode::OK == error)
    {
        error = aReader->getMinorVersion(mMinorVersion);
        if (HEIF::ErrorCode::OK == error)
        {
            HEIF::Array<HEIF::FourCC> brands;
            error = aReader->getCompatibleBrands(brands);
            if (HEIF::ErrorCode::OK == error)
            {
                mCompatibleBrands.reserve(brands.size);
                for (const auto& b : brands)
                {
                    addCompatibleBrand(b);
                }
                error = aReader->getFileInformation(mFileinfo);
                if (HEIF::ErrorCode::OK == error)
                {
                    if (mFileinfo.features & HEIF::FileFeatureEnum::HasRootLevelMetaBox)
                    {
                        HEIF::ImageId prim = InvalidItem;
                        error              = aReader->getPrimaryItem(prim);
                        if (HEIF::ErrorCode::PRIMARY_ITEM_NOT_SET == error)
                        {
                            // Warning: no primary item specified.
                            // Ignore this.
                            error = HEIF::ErrorCode::OK;
                        }
                        else if (HEIF::ErrorCode::OK != error)
                        {
                            // other error.
                            return error;
                        }

                        for (const auto& i : mFileinfo.rootMetaBoxInformation.itemInformations)
                        {
                            ImageItem* image = constructImageItem(aReader, i.itemId, &i, error);
                            if (HEIF::ErrorCode::OK != error)
                            {
                                return error;
                            }
                            if (image)
                            {
                                if (i.itemId == prim)
                                {
                                    if (image->isImageItem())
                                    {
                                        setPrimaryItem(static_cast<ImageItem*>(image));
                                    }
                                    else
                                    {
                                        return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
                                    }
                                }
                            }
                            else
                            {
                                constructMetaItem(aReader, i.itemId, error);
                                if (HEIF::ErrorCode::OK != error)
                                {
                                    return error;
                                }
                            }
                        }
                    }
                    for (const auto& i : mFileinfo.trackInformation)
                    {
                        mSamples.reserve(mSamples.size() + i.sampleProperties.size);
                        constructTrack(aReader, i.trackId, error);
                        if (HEIF::ErrorCode::OK != error)
                        {
                            return error;
                        }
                    }

                    // handle grouping here. and not in track/image/sample.

                    // first link items and tracks...
                    for (const auto& g : mFileinfo.rootMetaBoxInformation.entityGroupings)
                    {
                        Track* track = nullptr;
                        Item* item   = nullptr;
                        if (g.type == "stmi")
                        {
                            // stmi (sample to media group) linking is processed later on.
                        }
                        else
                        {
                            EntityGroup* eg = constructGroup(g.type);
                            eg->setId(g.groupId);
                            mGroupsLoad[g.groupId] = eg;
                            for (const auto& i : g.entityIds)
                            {
                                if (mTracksLoad.find(HEIF::SequenceId(i)) != mTracksLoad.end())
                                {
                                    // image sequence?
                                    track = mTracksLoad[HEIF::SequenceId(i)];
                                    eg->addTrack(track);
                                }
                                else if (mItemsLoad.find(HEIF::ImageId(i)) != mItemsLoad.end())
                                {
                                    // Image/item then..
                                    item = mItemsLoad[HEIF::ImageId(i)];
                                    eg->addItem(item);
                                }
                                else
                                {
                                    // TODO: fail or warn?
                                    // the grouped entity does not exist
                                }
                            }
                        }
                    }

                    // and now check if tracks are linked with out specifying them in the entitygroupings.
                    for (const auto& ii : mFileinfo.trackInformation)
                    {
                        auto* info = &ii;
                        if (info->alternateGroupId != 0)
                        {
                            auto res = mAltGroupsLoad.insert({info->alternateGroupId, nullptr});
                            if (res.second)
                            {
                                res.first->second = new AlternativeTrackGroup(this);
                            }

                            auto it = mTracksLoad.find(ii.trackId);
                            if (it != mTracksLoad.end())
                            {
                                Track* track = it->second;
                                res.first->second->addTrack(track);
                            }
                        }
                    }

                    // and finally handle sample groupings (metadata or other..)
                    for (const auto& ii : mFileinfo.trackInformation)
                    {
                        auto* info = &ii;
                        if (mTracksLoad.find(ii.trackId) != mTracksLoad.end())
                        {
                            std::map<std::uint32_t, HEIF::SampleVisualEquivalence> groupIdToEqu;
                            std::map<std::uint32_t, HEIF::SampleToMetadataItem> groupIdToMeta;

                            for (const auto& at : info->equivalences)
                            {
                                groupIdToEqu[at.sampleGroupDescriptionIndex] = at;
                            }
                            for (const auto& at : info->metadatas)
                            {
                                groupIdToMeta[at.sampleGroupDescriptionIndex] = at;
                            }
                            for (const auto& at : info->sampleGroups)
                            {
                                if (at.type == "stmi")
                                {
                                    // handle sample to meta specially.
                                    for (const auto& sampleId : at.samples)
                                    {
                                        Sample* s        = mSamplesLoad[{ii.trackId, sampleId.sampleId}];
                                        const auto& meta = groupIdToMeta[sampleId.sampleGroupDescriptionIndex];
                                        for (auto metaId : meta.metadataItemIds)
                                        {
                                            MetaItem* m = static_cast<MetaItem*>(mItemsLoad[metaId.get()]);
                                            s->addMetadata(m);
                                        }
                                    }
                                }
                                else if (at.type == "refs")
                                {
                                    // handled differently.. (see decode deps later)
                                }
                                else
                                {
                                    EntityGroup* group = nullptr;
                                    auto it            = mGroupsLoad.insert({HEIF::GroupId(at.typeParameter), nullptr});
                                    if (at.type == "eqiv")
                                    {
                                        if (it.second)
                                        {
                                            it.first->second = constructGroup(at.type);
                                            it.first->second->setId(HEIF::GroupId(at.typeParameter));
                                        }
                                        group = it.first->second;
                                        // verify type of group.
                                        if (group->getType() != "eqiv")
                                        {
                                            // warning: corrupted file..
                                            continue;
                                        }
                                        auto* eg = static_cast<EquivalenceGroup*>(group);
                                        for (const auto& sampleId : at.samples)
                                        {
                                            Sample* s       = mSamplesLoad[{ii.trackId, sampleId.sampleId}];
                                            const auto& equ = groupIdToEqu[sampleId.sampleGroupDescriptionIndex];
                                            eg->addSample(s, equ.timeOffset, equ.timescaleMultiplier);
                                        }
                                    }
                                    else
                                    {
                                        // link sample to generic group. NOTE: information might be missing since we
                                        // don't actually know the group type.
                                        if (it.second)
                                        {
                                            it.first->second = constructGroup(at.type);
                                            it.first->second->setId(HEIF::GroupId(at.typeParameter));
                                        }
                                        group = it.first->second;
                                        for (const auto& sampleId : at.samples)
                                        {
                                            Sample* s = mSamplesLoad[{ii.trackId, sampleId.sampleId}];
                                            group->addSample(s);
                                        }
                                    }
                                }
                            }

                            for (const auto& sampleId : info->sampleProperties)
                            {
                                Sample* s = mSamplesLoad[{ii.trackId, sampleId.sampleId}];
                                HEIF::Array<HEIF::SequenceImageId> dependencies;
                                aReader->getDecodeDependencies(ii.trackId, sampleId.sampleId, dependencies);
                                for (auto sid : dependencies)
                                {
                                    s->addDecodeDependency(mSamplesLoad[{ii.trackId, sid}]);
                                }
                            }
                        }
                    }

                    if (HEIF::ErrorCode::OK == error)
                    {
                        HEIF::Array<std::int32_t> mat;
                        error = aReader->getMatrix(mat);
                        if (HEIF::ErrorCode::NOT_APPLICABLE == error)
                        {
                            // no matrix in file. no error.
                            error = HEIF::ErrorCode::OK;
                        }
                        else
                        {
                            if (HEIF::ErrorCode::OK == error)
                            {
                                if (mat.size == 9)
                                {
                                    for (size_t i = 0; i < 9; i++)
                                    {
                                        mMatrix[i] = mat[i];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return error;
}
const HEIF::FourCC& Heif::getMajorBrand() const
{
    return mMajorBrand;
}
void Heif::setMajorBrand(const HEIF::FourCC& aBrand)
{
    mMajorBrand = aBrand;
}
std::uint32_t Heif::getMinorVersion() const
{
    return mMinorVersion;
}
void Heif::setMinorVersion(std::uint32_t aVersion)
{
    mMinorVersion = aVersion;
}
std::uint32_t Heif::compatibleBrands() const
{
    return static_cast<std::uint32_t>(mCompatibleBrands.size());
}
const HEIF::FourCC& Heif::getCompatibleBrand(std::uint32_t aId) const
{
    if (aId < mCompatibleBrands.size())
    {
        return mCompatibleBrands[aId];
    }
    static HEIF::FourCC aInvalidFourCC(static_cast<std::uint32_t>(0));
    return aInvalidFourCC;
}

void Heif::addCompatibleBrand(const HEIF::FourCC& brand)
{
    if (!AddItemTo(mCompatibleBrands, brand))
    {
        // Tried to add a duplicate compatible brand.
    }
}
void Heif::removeCompatibleBrand(std::uint32_t aId)
{
    if (aId < mCompatibleBrands.size())
    {
        mCompatibleBrands.erase(mCompatibleBrands.begin() + static_cast<std::int32_t>(aId));
    }
}
void Heif::removeCompatibleBrand(const HEIF::FourCC& brand)
{
    if (!RemoveItemFrom(mCompatibleBrands, brand))
    {
        // tried to remove brand that was not added.
    }
}

EntityGroup* Heif::constructGroup(const HEIF::FourCC& aType)
{
    if (aType == "eqiv")
    {
        return new EquivalenceGroup(this);
    }
    return new EntityGroup(this, aType);
}

Sample* Heif::constructSample(HEIF::Reader* aReader,
                              const HEIF::SequenceId& aTrack,
                              const HEIF::SampleInformation& aInfo,
                              HEIF::ErrorCode& aErrorCode)
{
    auto it = mSamplesLoad.insert({{aTrack, aInfo.sampleId}, nullptr});
    if (it.second)
    {
        auto info    = getTrackInformation(aTrack);
        Sample* item = nullptr;

        if ((info->features & HEIF::TrackFeatureEnum::Feature::IsVideoTrack) ||
            (info->features & HEIF::TrackFeatureEnum::Feature::IsMasterImageSequence) ||
            (info->features & HEIF::TrackFeatureEnum::Feature::IsThumbnailImageSequence) ||
            (info->features & HEIF::TrackFeatureEnum::Feature::IsAuxiliaryImageSequence))
        {
            item = new VideoSample(this);
        }
        else if (info->features & HEIF::TrackFeatureEnum::Feature::IsAudioTrack)
        {
            item = new AudioSample(this);
        }
        else
        {
            // unknown sample type. ignore.
        }
        if (item)
        {
            item->setId(aInfo.sampleId);
            it.first->second = item;
            aErrorCode       = item->load(aReader, aTrack, aInfo);
            return item;
        }
#ifdef FAIL_ON_UNKNOWN_ITEM
        aErrorCode = HEIF::ErrorCode::MEDIA_PARSING_ERROR;
#endif
        // invalid state.
        mSamplesLoad.erase({aTrack, aInfo.sampleId});
        return nullptr;
    }
    aErrorCode = HEIF::ErrorCode::OK;
    return it.first->second;
}

Track* Heif::constructTrack(HEIF::Reader* aReader, const HEIF::SequenceId& aTrackId, HEIF::ErrorCode& aErrorCode)
{
    auto it = mTracksLoad.insert({aTrackId, nullptr});
    if (it.second)
    {
        auto info = getTrackInformation(aTrackId);
        if (info == nullptr)
        {
            aErrorCode = HEIF::ErrorCode::MEDIA_PARSING_ERROR;
            return nullptr;
        }

        Track* item = nullptr;
        if (info->features & HEIF::TrackFeatureEnum::Feature::IsVideoTrack)
        {
            item = new VideoTrack(this);
        }
        else if ((info->features & HEIF::TrackFeatureEnum::Feature::IsMasterImageSequence) ||
                 (info->features & HEIF::TrackFeatureEnum::Feature::IsThumbnailImageSequence) ||
                 (info->features & HEIF::TrackFeatureEnum::Feature::IsAuxiliaryImageSequence))
        {
            item = new ImageSequence(this);
        }
        else if (info->features & HEIF::TrackFeatureEnum::Feature::IsAudioTrack)
        {
            item = new AudioTrack(this);
        }
        else
        {
            // unknown track type. ignore
        }
        if (item)
        {
            item->setId(aTrackId);
            it.first->second = item;
            aErrorCode       = item->load(aReader, aTrackId);
            return item;
        }
#ifdef FAIL_ON_UNKNOWN_ITEM
        aErrorCode = HEIF::ErrorCode::MEDIA_PARSING_ERROR;
#endif
        // invalid state.
        mTracksLoad.erase(aTrackId);
        return nullptr;
    }
    aErrorCode = HEIF::ErrorCode::OK;
    return it.first->second;
}

ImageItem* Heif::constructImageItem(HEIF::Reader* aReader,
                                    const HEIF::ImageId& aItemId,
                                    const HEIF::ItemInformation* aItemInfo,
                                    HEIF::ErrorCode& aErrorCode)
{
    auto it = mItemsLoad.insert({aItemId, nullptr});
    if (it.second)
    {
        HEIF::FourCC type;
        aErrorCode = aReader->getItemType(aItemId, type);
        if (HEIF::ErrorCode::OK != aErrorCode)
        {
            return nullptr;
        }
        Item* item = nullptr;
        if (type == HEIF::FourCC("avc1"))
        {
            item = new AVCCodedImageItem(this);
        }
        else if (type == HEIF::FourCC("hvc1"))
        {
            item = new HEVCCodedImageItem(this);
        }
        else if (type == HEIF::FourCC("lhv1"))
        {
            // L-HEVC enhancement-layer items are structurally HEVC coded
            // image items (same NAL unit format). A dedicated
            // LHEVCCodedImageItem may be added in a future cut; for now
            // reusing HEVCCodedImageItem with the correct four-cc and
            // media format is sufficient because the decoder configuration
            // dispatch already picks LHEVCDecoderConfiguration based on
            // the "lhv1" four-cc returned by mediaFormatFromFourCC.
            item = new HEVCCodedImageItem(this, HEIF::FourCC("lhv1"), HEIF::MediaFormat::LHEVC);
        }
        else if (type == HEIF::FourCC("vvc1"))
        {
            item = new VVCCodedImageItem(this);
        }
        else if (type == HEIF::FourCC("jpeg"))
        {
            // mime image/jpeg handled in separate case
            item = new JPEGCodedImageItem(this);
        }
        else if (type == HEIF::FourCC("iden"))
        {
            item = new IdentityImageItem(this);
        }
        else if (type == HEIF::FourCC("iovl"))
        {
            item = new OverlayImageItem(this);
        }
        else if (type == HEIF::FourCC("grid"))
        {
            item = new GridImageItem(this);
        }
        else if (type == HEIF::FourCC("mime") && aItemInfo)
        {
            auto contentType       = aItemInfo->description.contentType;
            auto contentTypeString = std::string(contentType.begin(), contentType.end());
            if (contentTypeString == "image/jpeg")
            {
                // How bad is this? Should really do MimeItem, but what about compatibility. Special
                // support in JPEGCodedImageItem maybe required.
                item = new JPEGCodedImageItem(this);
            }
            else
            {
                // Handle possible other future image mime types
            }
        }
        if (item)
        {
            item->setId(aItemId);
            it.first->second = item;
            aErrorCode       = item->load(aReader, aItemId);
            return static_cast<ImageItem*>(item);
        }
#ifdef FAIL_ON_UNKNOWN_ITEM
        aErrorCode = HEIF::ErrorCode::MEDIA_PARSING_ERROR;
#endif
        // invalid state.
        mItemsLoad.erase(aItemId);
        return nullptr;
    }
    aErrorCode = HEIF::ErrorCode::OK;
    if (it.first->second->isImageItem())
    {
        return static_cast<ImageItem*>(it.first->second);
    }
    else
    {
        return nullptr;
    }
}


MetaItem* Heif::constructMetaItem(HEIF::Reader* aReader, const HEIF::ImageId& aItemId, HEIF::ErrorCode& aErrorCode)
{
    auto it = mItemsLoad.insert({aItemId, nullptr});
    if (it.second)  // whether insert resulted
    {
        const HEIF::ItemInformation* info = getItemInformation(aItemId);
        HEIF::FourCC type;
        aErrorCode = aReader->getItemType(aItemId, type);
        if (HEIF::ErrorCode::OK != aErrorCode)
        {
            return nullptr;
        }
        Item* item = nullptr;
        if (type == HEIF::FourCC("Exif"))
        {
            item = new ExifItem(this);
        }
        else if (type == HEIF::FourCC("mime"))
        {
            // TODO: when reader exposes the item content-type, use that.
            std::string mimetype(info->description.contentType.begin(), info->description.contentType.end());
            if (mimetype == "text/xml")
            {
                // TODO: should actually check the content schema for "urn:mpeg:mpeg7:schema:2001" etcetc.
                item = new MPEG7Item(this);
            }
            else if (mimetype == "application/rdf+xml")
            {
                item = new XMPItem(this);
            }
            else
            {
                // Creates a generic mime item.
                item = new MimeItem(this);
            }
        }
        if (item)
        {
            item->setId(aItemId);
            it.first->second = item;
            aErrorCode       = item->load(aReader, aItemId);
            return static_cast<MetaItem*>(item);
        }
#ifdef FAIL_ON_UNKNOWN_ITEM
        aErrorCode = HEIF::ErrorCode::MEDIA_PARSING_ERROR;
#endif
        // invalid state.
        mItemsLoad.erase(aItemId);
        return nullptr;
    }
    aErrorCode = HEIF::ErrorCode::OK;
    if (it.first->second->isMetadataItem())
    {
        return static_cast<MetaItem*>(it.first->second);
    }
    else
    {
        return nullptr;
    }
}

ItemProperty* Heif::constructItemProperty(HEIF::Reader* aReader,
                                          const HEIF::ItemPropertyInfo& aItemInfo,
                                          HEIF::ErrorCode& aErrorCode)
{
    // method is valid only during load
    auto it = mPropertiesLoad.insert({aItemInfo.index, nullptr});
    if (it.second)
    {
        ItemProperty* p = nullptr;
        switch (aItemInfo.type)
        {
        case HEIF::ItemPropertyType::CLAP:
        {
            p = new CleanApertureProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::IROT:
        {
            p = new RotateProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::IMIR:
        {
            p = new MirrorProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::PASP:
        {
            p = new PixelAspectRatioProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::COLR:
        {
            p = new ColourInformationProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::PIXI:
        {
            p = new PixelInformationProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::RLOC:
        {
            p = new RelativeLocationProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::LSEL:
        {
            p = new LayerSelectorProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::TOLS:
        {
            p = new TargetOlsProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::OINF:
        {
            p = new OperatingPointsProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::SUBS:
        {
            p = new SubSampleProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::AUXC:
        {
            p = new AuxiliaryProperty(this);
            break;
        }
        case HEIF::ItemPropertyType::ISPE:
        {
            // not accessible directly, use HEIF::Reader::getWidth/HEIF::Reader::getHeight
#if ISPE_AS_RAW_PROPERTY
            // so construct as RawProperty..
            p = new RawProperty(this);
#else
            // ignore the property
            aErrorCode = HEIF::ErrorCode::OK;
            return nullptr;
#endif
            break;
        }
        case HEIF::ItemPropertyType::HVCC:
        case HEIF::ItemPropertyType::AVCC:
        case HEIF::ItemPropertyType::JPGC:
        {
            // not accessible directly, use HEIF::Reader::getDecoderParameterSets?
            // so construct as raw..
#if DECODER_CONFIG_AS_RAW_PROPERTY
            // so construct as RawProperty..
            p = new RawProperty(this);
#else
            // ignore the property
            aErrorCode = HEIF::ErrorCode::OK;
            return nullptr;
#endif
            break;
        }
        case HEIF::ItemPropertyType::INVALID:
        {
            aErrorCode = HEIF::ErrorCode::MEDIA_PARSING_ERROR;
            return nullptr;
        }
        case HEIF::ItemPropertyType::RAW:
        default:
        {
            // construct as raw..
            p = new RawProperty(this);
            break;
        }
        }
        if (p)
        {
            p->setId(aItemInfo.index);
            it.first->second = p;
            aErrorCode       = p->load(aReader, aItemInfo.index);
            return p;
        }
#ifdef FAIL_ON_UNKNOWN_PROPERTY
        aErrorCode = HEIF::ErrorCode::MEDIA_PARSING_ERROR;
#endif
        mPropertiesLoad.erase(aItemInfo.index);
        return nullptr;
    }
    aErrorCode = HEIF::ErrorCode::OK;
    return it.first->second;
}

void Heif::addProperty(ItemProperty* aItemProperty)
{
    aItemProperty->setId((static_cast<std::uint32_t>(mProperties.size())) + 1);
    mProperties.push_back(aItemProperty);
}
void Heif::addItem(Item* aItem)
{
    aItem->setId((static_cast<std::uint32_t>(mItems.size())) + 1);
    mItems.push_back(aItem);
    mItemsOfType[aItem->getType()].push_back(aItem);
}

std::uint32_t Heif::getPropertyCount() const
{
    return static_cast<std::uint32_t>(mProperties.size());
}

ItemProperty* Heif::getProperty(std::uint32_t aIndex)
{
    if (aIndex < mProperties.size())
    {
        return mProperties[aIndex];
    }
    return nullptr;
}
const ItemProperty* Heif::getProperty(std::uint32_t aIndex) const
{
    if (aIndex < mProperties.size())
    {
        return mProperties[aIndex];
    }
    return nullptr;
}

void Heif::removeProperty(ItemProperty* aProperty)
{
    if (!RemoveItemFrom(mProperties, aProperty))
    {
        // tried to remove property that was not added
        HEIF_LOG_INVARIANT(false, "Heif::removeProperty: property was not in mProperties");
    }
}
void Heif::removeItem(Item* aItem)
{
    // check if primary
    if (mPrimaryItem == aItem)
    {
        mPrimaryItem = nullptr;
    }
    // firstly remove it from the all items list.
    if (!RemoveItemFrom(mItems, aItem))
    {
        // Tried to remove an item that was not added!
        HEIF_LOG_INVARIANT(false, "Heif::removeItem: item was not in mItems");
    }

    // secondly remove the item from the mItemsOfType lists..
    auto tmp = mItemsOfType.find(aItem->getType());
    if (tmp != mItemsOfType.end())
    {
        if (!RemoveItemFrom(tmp->second, aItem))
        {
            // Tried to remove an item that was not added!
            HEIF_LOG_INVARIANT(false, "Heif::removeItem: item missing from mItemsOfType bucket");
        }
        // and cleanup the type map if it was the last of it's type
        if (tmp->second.empty())
        {
            mItemsOfType.erase(tmp);
        }
    }
}

std::uint32_t Heif::getItemsOfTypeCount(const HEIF::FourCC& aType) const
{
    const auto& it = mItemsOfType.find(aType);
    if (it != mItemsOfType.end())
    {
        return static_cast<std::uint32_t>(it->second.size());
    }
    return 0;
}

Item* Heif::getItemOfType(const HEIF::FourCC& aType, std::uint32_t aId)
{
    const auto& it = mItemsOfType.find(aType);
    if (it != mItemsOfType.end())
    {
        if (aId < it->second.size())
        {
            return it->second[aId];
        }
    }
    return nullptr;
}
const Item* Heif::getItemOfType(const HEIF::FourCC& aType, std::uint32_t aId) const
{
    const auto& it = mItemsOfType.find(aType);
    if (it != mItemsOfType.end())
    {
        if (aId < it->second.size())
        {
            return it->second[aId];
        }
    }
    return nullptr;
}

ImageItem* Heif::getPrimaryItem()
{
    return mPrimaryItem;
}
const ImageItem* Heif::getPrimaryItem() const
{
    return mPrimaryItem;
}

void Heif::setPrimaryItem(ImageItem* aPrimary)
{
    mPrimaryItem = aPrimary;
}

std::uint32_t Heif::getMasterImageCount() const
{
    std::uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if ((it->isImageItem()) && ((static_cast<ImageItem*>(it))->isMasterImage()))
        {
            ++cnt;
        }
    }
    return cnt;
}

ImageItem* Heif::getMasterImage(std::uint32_t aId)
{
    std::uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if ((it->isImageItem()) && ((static_cast<ImageItem*>(it))->isMasterImage()))
        {
            if (cnt == aId)
            {
                return static_cast<ImageItem*>(it);
            }
            ++cnt;
        }
    }
    return nullptr;
}

const ImageItem* Heif::getMasterImage(std::uint32_t aId) const
{
    std::uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if ((it->isImageItem()) && ((static_cast<ImageItem*>(it))->isMasterImage()))
        {
            if (cnt == aId)
            {
                return static_cast<ImageItem*>(it);
            }
            ++cnt;
        }
    }
    return nullptr;
}

std::uint32_t Heif::getItemCount() const
{
    std::uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if ((!it->isImageItem()) || (!(static_cast<ImageItem*>(it))->isCodedImage()))
        {
            ++cnt;
        }
    }
    return cnt;
}

Item* Heif::getItem(std::uint32_t aId)
{
    std::uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if ((!it->isImageItem()) || (!(static_cast<ImageItem*>(it))->isCodedImage()))
        {
            if (cnt == aId)
            {
                return it;
            }
            ++cnt;
        }
    }
    return nullptr;
}

const Item* Heif::getItem(std::uint32_t aId) const
{
    std::uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if ((!it->isImageItem()) || (!(static_cast<ImageItem*>(it))->isCodedImage()))
        {
            if (cnt == aId)
            {
                return it;
            }
            ++cnt;
        }
    }
    return nullptr;
}

std::uint32_t Heif::getImageCount() const
{
    std::uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if (it->isImageItem())
        {
            ++cnt;
        }
    }
    return cnt;
}

ImageItem* Heif::getImage(std::uint32_t aId)
{
    std::uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if (it->isImageItem())
        {
            if (cnt == aId)
            {
                return static_cast<ImageItem*>(it);
            }
            ++cnt;
        }
    }
    return nullptr;
}

const ImageItem* Heif::getImage(std::uint32_t aId) const
{
    std::uint32_t cnt = 0;
    for (const auto& it : mItems)
    {
        if (it->isImageItem())
        {
            if (cnt == aId)
            {
                return static_cast<ImageItem*>(it);
            }
            ++cnt;
        }
    }
    return nullptr;
}


bool Heif::hasSingleImage()
{
    return getMasterImageCount() == 1;
}
bool Heif::hasImageCollection()
{
    return getMasterImageCount() > 1;
}

DecoderConfig* Heif::constructDecoderConfig(HEIF::Reader* aReader,
                                            const HEIF::SequenceId& aTrackId,
                                            const HEIF::SequenceImageId& aTrackImageId,
                                            HEIF::ErrorCode& aErrorCode)
{
    return constructDecoderConfig(aReader, Heif::InvalidItem, aTrackId, aTrackImageId, aErrorCode);
}
DecoderConfig* Heif::constructDecoderConfig(HEIF::Reader* aReader,
                                            const HEIF::ImageId& aImageId,
                                            HEIF::ErrorCode& aErrorCode)
{
    return constructDecoderConfig(aReader, aImageId, Heif::InvalidSequence, Heif::InvalidSequenceImage, aErrorCode);
}
DecoderConfig* Heif::constructDecoderConfig(HEIF::Reader* aReader,
                                            const HEIF::ImageId& aItemId,
                                            const HEIF::SequenceId& aTrackId,
                                            const HEIF::SequenceImageId& aTrackImageId,
                                            HEIF::ErrorCode& aErrorCode)
{
    // method is only valid during load
    HEIF::DecoderConfiguration cfg;
    std::pair<HEIF::SequenceId, HEIF::DecoderConfigId> key;
    HEIF::FourCC aType;
    if (aItemId == InvalidItem)
    {
        aErrorCode = aReader->getDecoderCodeType(aTrackId, aTrackImageId, aType);
        if (HEIF::ErrorCode::OK != aErrorCode)
        {
            return nullptr;
        }
        aErrorCode = aReader->getDecoderParameterSets(aTrackId, aTrackImageId, cfg);
        if (HEIF::ErrorCode::OK != aErrorCode)
        {
            return nullptr;
        }
        key.first  = aTrackId;
        key.second = cfg.decoderConfigId;
    }
    else
    {
        aErrorCode = aReader->getDecoderCodeType(aItemId, aType);
        if (HEIF::ErrorCode::OK != aErrorCode)
        {
            return nullptr;
        }
        aErrorCode = aReader->getDecoderParameterSets(aItemId, cfg);
        if (HEIF::ErrorCode::OK != aErrorCode)
        {
            return nullptr;
        }
        key.first  = InvalidSequence;
        key.second = cfg.decoderConfigId;
    }
    auto it = mDecoderConfigsLoad.insert({key, nullptr});
    if (it.second)
    {
        DecoderConfig* config = nullptr;
        switch (mediaFormatFromFourCC(aType))
        {
        case HEIF::MediaFormat::AVC:
        {
            config = new AVCDecoderConfiguration(this, aType);
            break;
        }
        case HEIF::MediaFormat::HEVC:
        {
            config = new HEVCDecoderConfiguration(this, aType);
            break;
        }
        case HEIF::MediaFormat::LHEVC:
        {
            config = new LHEVCDecoderConfiguration(this, aType);
            break;
        }
        case HEIF::MediaFormat::VVC:
        {
            config = new VVCDecoderConfiguration(this, aType);
            break;
        }

        case HEIF::MediaFormat::AAC:
        {
            config = new AACDecoderConfiguration(this, aType);
            break;
        }
        case HEIF::MediaFormat::JPEG:
        {
            config = new JPEGDecoderConfiguration(this, aType);
            break;
        }
        default:
        {
            config = nullptr;
        }
        }

        if (config)
        {
            aErrorCode = config->setConfig(cfg.decoderSpecificInfo);
            if (HEIF::ErrorCode::OK == aErrorCode)
            {
                config->setId(cfg.decoderConfigId);  // save the original id.
                it.first->second = config;
                return config;
            }
            delete config;
        }
#ifdef FAIL_ON_UNKNOWN_CONFIGTYPE
        aErrorCode = HEIF::ErrorCode::MEDIA_PARSING_ERROR;
#endif
        mDecoderConfigsLoad.erase(key);
        return nullptr;
    }
    aErrorCode = HEIF::ErrorCode::OK;
    return it.first->second;
}

std::uint32_t Heif::getDecoderConfigCount() const
{
    return static_cast<std::uint32_t>(mDecoderConfigs.size());
}

DecoderConfig* Heif::getDecoderConfig(std::uint32_t aId)
{
    if (aId < mDecoderConfigs.size())
    {
        return mDecoderConfigs[aId];
    }
    return nullptr;
}
const DecoderConfig* Heif::getDecoderConfig(std::uint32_t aId) const
{
    if (aId < mDecoderConfigs.size())
    {
        return mDecoderConfigs[aId];
    }
    return nullptr;
}

void Heif::addDecoderConfig(DecoderConfig* aItem)
{
    aItem->setId((static_cast<std::uint32_t>(mDecoderConfigs.size())) + 1);
    mDecoderConfigs.push_back(aItem);
}

void Heif::removeDecoderConfig(DecoderConfig* aDecoderConfig)
{
    if (!RemoveItemFrom(mDecoderConfigs, aDecoderConfig))
    {
        // Tried to remove a non added DecoderConfiguration
        HEIF_LOG_INVARIANT(false, "Heif::removeDecoderConfig: config was not in mDecoderConfigs");
    }
}

std::uint32_t Heif::getTrackCount() const
{
    return static_cast<std::uint32_t>(mTracks.size());
}

Track* Heif::getTrack(std::uint32_t aTrack)
{
    if (aTrack < mTracks.size())
    {
        return mTracks[aTrack];
    }
    return nullptr;
}

const Track* Heif::getTrack(std::uint32_t aTrack) const
{
    if (aTrack < mTracks.size())
    {
        return mTracks[aTrack];
    }
    return nullptr;
}

void Heif::addTrack(Track* aTrack)
{
    aTrack->setId((static_cast<std::uint32_t>(mTracks.size())) + 1);
    mTracks.push_back(aTrack);
}
void Heif::removeTrack(Track* aTrack)
{
    // firstly remove it from the all items list.
    if (!RemoveItemFrom(mTracks, aTrack))
    {
        // Tried to remove an item that was not added!
        HEIF_LOG_INVARIANT(false, "Heif::removeTrack: track was not in mTracks");
    }
}

void Heif::addSample(Sample* aSample)
{
    aSample->setId((static_cast<std::uint32_t>(mSamples.size())) + 1);
    mSamples.push_back(aSample);
}
void Heif::removeSample(Sample* aSample)
{
    // firstly remove it from the all items list.
    if (!RemoveItemFrom(mSamples, aSample))
    {
        // Tried to remove an item that was not added!
        HEIF_LOG_INVARIANT(false, "Heif::removeSample: sample was not in mSamples");
    }
}

void Heif::addGroup(EntityGroup* aItem)
{
    aItem->setId((static_cast<std::uint32_t>(mGroups.size())) + 1);
    mGroups.push_back(aItem);
    mGroupsOfType[aItem->getType()].push_back(aItem);
}

void Heif::removeGroup(EntityGroup* aItem)
{
    // firstly remove it from the all items list.
    if (!RemoveItemFrom(mGroups, aItem))
    {
        // Tried to remove an item that was not added!
        HEIF_LOG_INVARIANT(false, "Heif::removeGroup: group was not in mGroups");
    }

    // secondly remove the item from the mItemsOfType lists..
    auto tmp = mGroupsOfType.find(aItem->getType());
    if (tmp != mGroupsOfType.end())
    {
        if (!RemoveItemFrom(tmp->second, aItem))
        {
            // Tried to remove an item that was not added!
            HEIF_LOG_INVARIANT(false, "Heif::removeGroup: group missing from mGroupsOfType bucket");
        }
        // and cleanup the type map if it was the last of it's type
        if (tmp->second.empty())
        {
            mGroupsOfType.erase(tmp);
        }
    }
}
std::uint32_t Heif::getAlternativeTrackGroupCount() const
{
    return static_cast<std::uint32_t>(mAltGroups.size());
}
AlternativeTrackGroup* Heif::getAlternativeTrackGroup(std::uint32_t aId)
{
    if (aId < mAltGroups.size())
    {
        return mAltGroups[aId];
    }
    return nullptr;
}
const AlternativeTrackGroup* Heif::getAlternativeTrackGroup(std::uint32_t aId) const
{
    if (aId < mAltGroups.size())
    {
        return mAltGroups[aId];
    }
    return nullptr;
}
void Heif::removeAlternativeTrackGroup(AlternativeTrackGroup* aGroup)
{
    RemoveItemFrom(mAltGroups, aGroup);
}
void Heif::addAlternativeTrackGroup(AlternativeTrackGroup* aGroup)
{
    mAltGroups.push_back(aGroup);
}

HEIF::Reader* HEIFPP::Heif::getReaderInstance()
{
    return mReader;
}

std::uint32_t Heif::getGroupCount() const
{
    return static_cast<std::uint32_t>(mGroups.size());
}
EntityGroup* Heif::getGroup(std::uint32_t aId)
{
    if (aId < mGroups.size())
    {
        return mGroups[aId];
    }
    return nullptr;
}
const EntityGroup* Heif::getGroup(std::uint32_t aId) const
{
    if (aId < mGroups.size())
    {
        return mGroups[aId];
    }
    return nullptr;
}
std::uint32_t Heif::getGroupByTypeCount(const HEIF::FourCC& aType) const
{
    auto tmp = mGroupsOfType.find(aType);
    if (tmp != mGroupsOfType.end())
    {
        return static_cast<std::uint32_t>(tmp->second.size());
    }
    return 0;
}
EntityGroup* Heif::getGroupByType(const HEIF::FourCC& aType, std::uint32_t aId)
{
    auto tmp = mGroupsOfType.find(aType);
    if (tmp != mGroupsOfType.end())
    {
        if (aId < tmp->second.size())
        {
            return tmp->second[aId];
        }
    }
    return nullptr;
}
EntityGroup* Heif::getGroupById(const HEIF::GroupId& aId)
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
const EntityGroup* Heif::getGroupByType(const HEIF::FourCC& aType, std::uint32_t aId) const
{
    auto tmp = mGroupsOfType.find(aType);
    if (tmp != mGroupsOfType.end())
    {
        if (aId < tmp->second.size())
        {
            return tmp->second[aId];
        }
    }
    return nullptr;
}
const EntityGroup* Heif::getGroupById(const HEIF::GroupId& aId) const
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

HEIF::MediaFormat Heif::mediaFormatFromFourCC(const HEIF::FourCC& aType)
{
    if (aType == "mp4a")
    {
        return HEIF::MediaFormat::AAC;
    }
    else if (aType == "hvc1")
    {
        return HEIF::MediaFormat::HEVC;
    }
    else if (aType == "hev1")
    {
        return HEIF::MediaFormat::HEVC;
    }
    else if (aType == "lhv1")
    {
        return HEIF::MediaFormat::LHEVC;
    }
    else if (aType == "avc1")
    {
        return HEIF::MediaFormat::AVC;
    }
    else if (aType == "avc3")
    {
        return HEIF::MediaFormat::AVC;
    }
    else if (aType == "jpeg")
    {
        return HEIF::MediaFormat::JPEG;
    }
    else if (aType == "jpgs")
    {
        return HEIF::MediaFormat::JPEG;
    }
    else if (aType == "mjpg")
    {
        return HEIF::MediaFormat::JPEG;
    }
    else if (aType == "vvc1")
    {
        return HEIF::MediaFormat::VVC;
    }
    else if (aType == "vvi1")
    {
        return HEIF::MediaFormat::VVC;
    }
    else
    {
        return HEIF::MediaFormat::INVALID;
    }
}
