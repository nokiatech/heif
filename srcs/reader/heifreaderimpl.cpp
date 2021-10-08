/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#include "heifreaderimpl.hpp"

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>

#include "audiosampleentrybox.hpp"
#include "auxiliarytypeinfobox.hpp"
#include "auxiliarytypeproperty.hpp"
#include "avcconfigurationbox.hpp"
#include "avcdecoderconfigrecord.hpp"
#include "avcsampleentry.hpp"
#include "buildinfo.hpp"
#include "cleanaperturebox.hpp"
#include "codingconstraintsbox.hpp"
#include "directreferencesampleslist.hpp"
#include "extendedtypebox.hpp"
#include "heiffiledatatypesinternal.hpp"
#include "hevccommondefs.hpp"
#include "hevcconfigurationbox.hpp"
#include "hevcdecoderconfigrecord.hpp"
#include "hevcsampleentry.hpp"
#include "imagegrid.hpp"
#include "imagemirror.hpp"
#include "imageoverlay.hpp"
#include "imagerelativelocationproperty.hpp"
#include "imagerotation.hpp"
#include "log.hpp"
#include "mediadatabox.hpp"
#include "metabox.hpp"
#include "moviebox.hpp"
#include "moviefragmentbox.hpp"
#include "mp4audiosampleentrybox.hpp"
#include "requiredreferencetypesproperty.hpp"
#include "sampletometadataitementry.hpp"
#include "segmentindexbox.hpp"
#include "visualequivalenceentry.hpp"

using namespace std;

namespace HEIF
{
    namespace
    {
        Array<FourCCToIds> mapToArray(const TypeToIdsMap& typeToIdsMap)
        {
            Array<FourCCToIds> array(typeToIdsMap.size());
            unsigned int i = 0;
            for (const auto& iter : typeToIdsMap)
            {
                array[i].type     = iter.first;
                array[i].trackIds = makeArray<SequenceId>(iter.second);
                ++i;
            }

            return array;
        }

    }  // anonymous namespace

    /* ********************************************************************** */
    /* ************************* Public API methods ************************* */
    /* ********************************************************************** */

    HeifReaderImpl::HeifReaderImpl()
        : mState(State::UNINITIALIZED)
        , mIsPrimaryItemSet(false)
        , mPrimaryItemId(0)
        , mMetaBoxLoaded(false)
    {
    }

    ErrorCode HeifReaderImpl::initialize(const char* fileName)
    {
        ErrorCode rc;
        auto& io = mFileStream;
        io.fileStream.reset(openFile(fileName));
        rc = initialize(&*io.fileStream);
        if (rc != ErrorCode::OK)
        {
            io.fileStream.reset();
        }
        return rc;
    }

    ErrorCode HeifReaderImpl::initialize(StreamInterface* stream)
    {
        UniquePtr<InternalStream> internalStream(CUSTOM_NEW(InternalStream, (stream)));

        if (!internalStream->good())
        {
            return ErrorCode::FILE_OPEN_ERROR;
        }

        reset();

        SegmentId segmentId = 0;  // Initialization segment id
        auto& io            = mFileProperties.segmentPropertiesMap[segmentId].io;
        io.stream           = std::move(internalStream);
        io.size             = io.stream->size();

        try
        {
            ErrorCode error = readStream();
            if (error != ErrorCode::OK)
            {
                return error;
            }
        }
        catch (const ISOBMFF::Exception& exc)
        {
            logError() << "Error: " << exc.what() << std::endl;
            return ErrorCode::FILE_READ_ERROR;
        }
        catch (const std::exception& e)
        {
            logError() << "Error: " << e.what() << std::endl;
            return ErrorCode::FILE_READ_ERROR;
        }

        mFileInformation = makeFileInformation(mFileProperties);

        return ErrorCode::OK;
    }

    FileInformation HeifReaderImpl::makeFileInformation(const FileInformationInternal& intInfo) const
    {
        FileInformation fileInformation;
        fileInformation.rootMetaBoxInformation = convertRootMetaBoxInformation(intInfo.rootLevelMetaBoxProperties);
        fileInformation.trackInformation       = convertTrackInformation(intInfo.initTrackInfos);
        fileInformation.features               = intInfo.fileFeature.getFeatureMask();
        fileInformation.movieTimescale         = intInfo.moovProperties.movieTimescale;

        return fileInformation;
    }

    void HeifReaderImpl::close()
    {
        reset();
    }

    ErrorCode HeifReaderImpl::invalidateSegment(const SegmentId segmentId)
    {
        const bool isSegment = (mFileProperties.segmentPropertiesMap.count(segmentId) == 1u);
        if (isSegment == false)
        {
            return ErrorCode::INVALID_SEGMENT;
        }

        auto& segmentProperties = mFileProperties.segmentPropertiesMap.at(segmentId);
        auto& sequenceToSegment = mFileProperties.sequenceToSegment;
        for (const auto& sequence : segmentProperties.sequences)
        {
            sequenceToSegment.erase(sequence);
        }
        mFileProperties.segmentPropertiesMap.erase(segmentId);

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getSegmentIndex(Array<SegmentInformation>& segmentIndex)
    {
        segmentIndex = mFileProperties.segmentIndex;
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::parseSegmentIndex(StreamInterface* streamInterface,
                                                Array<SegmentInformation>& segmentIndex)
    {
        StreamIO io;
        io.stream.reset(CUSTOM_NEW(InternalStream, (streamInterface)));
        if (io.stream->peekEof())
        {
            io.stream.reset();
            return ErrorCode::FILE_READ_ERROR;
        }
        io.size = streamInterface->size();

        ErrorCode error        = ErrorCode::OK;
        bool segmentIndexFound = false;
        try
        {
            while (error != ErrorCode::OK && !io.stream->peekEof())
            {
                String boxType;
                std::int64_t boxSize = 0;
                BitStream bitstream;
                error = readBoxParameters(io, boxType, boxSize);
                if (error == ErrorCode::OK)
                {
                    if (boxType == "sidx")
                    {
                        error = readBox(io, bitstream);
                        if (error == ErrorCode::OK)
                        {
                            SegmentIndexBox sidx;
                            sidx.parseBox(bitstream);
                            makeSegmentIndex(sidx, segmentIndex, io.stream->tell());
                            segmentIndexFound = true;
                            break;
                        }
                    }
                    else if (boxType == "styp" || boxType == "moof" || boxType == "mdat")
                    {
                        // skip as we are not interested in it.
                        error = skipBox(io);
                    }
                    else
                    {
                        logWarning() << "Skipping root level box of unknown type '" << boxType << "'" << std::endl;
                        error = skipBox(io);
                    }
                }
            }
        }
        catch (ISOBMFF::Exception& exc)
        {
            logError() << "parseSegmentIndex Exception Error: " << exc.what() << std::endl;
            error = ErrorCode::FILE_READ_ERROR;
        }
        catch (std::exception& e)
        {
            logError() << "parseSegmentIndex std::exception Error:: " << e.what() << std::endl;
            error = ErrorCode::FILE_READ_ERROR;
        }

        if (!segmentIndexFound)
        {
            logError() << "parseSegmentIndex couldn't find sidx box: " << std::endl;
            error = ErrorCode::INVALID_SEGMENT;
        }

        if (error == ErrorCode::OK)
        {
            // peek() sets eof bit for the stream. Clear stream to make sure it is still accessible. seekg() in C++11
            // should clear stream after eof, but this does not seem to be always happening.
            if ((!io.stream->good()) && (!io.stream->eof()))
            {
                return ErrorCode::FILE_READ_ERROR;
            }
            io.stream->clear();
        }
        return error;
    }

    ErrorCode HeifReaderImpl::parseSegment(StreamInterface* streamInterface,
                                           SegmentId segmentId,
                                           uint64_t earliestPTSinTS)
    {
        if (mFileProperties.segmentPropertiesMap.count(segmentId) != 0u)
        {
            return ErrorCode::OK;
        }

        State prevState = mState;
        mState          = State::INITIALIZING;

        SegmentProperties& segmentProperties = mFileProperties.segmentPropertiesMap[segmentId];
        StreamIO& io                         = segmentProperties.io;
        io.stream.reset(CUSTOM_NEW(InternalStream, (streamInterface)));
        if (io.stream->peekEof())
        {
            mState = prevState;
            io.stream.reset();
            return ErrorCode::FILE_READ_ERROR;
        }
        io.size = streamInterface->size();

        segmentProperties.segmentId = segmentId;

        bool stypFound       = false;
        bool earliestPTSRead = false;
        Map<SequenceId, DecodePts::PresentationTimeTS> earliestPTSTS;

        ErrorCode error = ErrorCode::OK;
        try
        {
            while (error == ErrorCode::OK && !io.stream->peekEof())
            {
                String boxType;
                std::int64_t boxSize = 0;
                BitStream bitstream;
                error = readBoxParameters(io, boxType, boxSize);
                if (error == ErrorCode::OK)
                {
                    if (boxType == "styp")
                    {
                        error = readBox(io, bitstream);
                        if (error == ErrorCode::OK)
                        {
                            SegmentTypeBox styp;
                            styp.parseBox(bitstream);

                            if (!stypFound)
                            {
                                // only save first styp of the segment - rest can be ignored per spec.
                                segmentProperties.styp = styp;
                                stypFound              = true;
                            }
                        }
                    }
                    else if (boxType == "sidx")
                    {
                        error = readBox(io, bitstream);
                        if (error == ErrorCode::OK)
                        {
                            SegmentIndexBox sidx;
                            sidx.parseBox(bitstream);

                            if (!earliestPTSRead)
                            {
                                earliestPTSRead = true;
                                for (auto& initTrackInfo : mFileProperties.initTrackInfos)
                                {
                                    auto trackId = initTrackInfo.first;
                                    earliestPTSTS[trackId] =
                                        DecodePts::PresentationTimeTS(sidx.getEarliestPresentationTime());
                                }
                            }
                        }
                    }
                    else if (boxType == "moof")
                    {
                        error = handleSegmentMoof(io, segmentId, earliestPTSRead, earliestPTSTS, earliestPTSinTS);
                    }
                    else if (boxType == "mdat")
                    {
                        error = skipBox(io);
                    }
                    else
                    {
                        logWarning() << "Skipping root level box of unknown type '" << boxType << "'" << std::endl;
                        error = skipBox(io);
                    }
                }
            }
        }
        catch (ISOBMFF::Exception& exc)
        {
            logError() << "parseSegment Exception Error: " << exc.what() << std::endl;
            error = ErrorCode::FILE_READ_ERROR;
        }
        catch (std::exception& e)
        {
            logError() << "parseSegment std::exception Error:: " << e.what() << std::endl;
            error = ErrorCode::FILE_READ_ERROR;
        }

        if (error == ErrorCode::OK)
        {
            for (auto& trackInfo : segmentProperties.trackInfos)
            {
                setupSegmentSidxFallback(std::make_pair(segmentId, trackInfo.first));
            }

            updateCompositionTimes(segmentId);

            // peek() sets eof bit for the stream. Clear stream to make sure it is still accessible. seekg() in C++11
            // should clear stream after eof, but this does not seem to be always happening.
            if ((!io.stream->good()) && (!io.stream->eof()))
            {
                return ErrorCode::FILE_READ_ERROR;
            }
            io.stream->clear();

            mFileInformation = makeFileInformation(mFileProperties);

            mState = State::READY;
        }
        else
        {
            invalidateSegment(segmentId);
        }
        return error;
    }

    HEIF_DLL_PUBLIC ErrorCode Reader::SetCustomAllocator(CustomAllocator* customAllocator)
    {
        if (!setCustomAllocator(customAllocator))
        {
            return ErrorCode::ALLOCATOR_ALREADY_SET;
        }

        return ErrorCode::OK;
    }

    HEIF_DLL_PUBLIC Reader* Reader::Create()
    {
        return CUSTOM_NEW(HeifReaderImpl, ());
    }

    HEIF_DLL_PUBLIC void Reader::Destroy(Reader* imageFileInterface)
    {
        CUSTOM_DELETE(imageFileInterface, Reader);
    }

    HEIF_DLL_PUBLIC const char* Reader::GetVersion()
    {
        return BuildInfo::Version;
    }

    /* ********************************************************************** */
    /* *********************** Common private methods *********************** */
    /* ********************************************************************** */

    Segments HeifReaderImpl::segmentsBySequence()
    {
        return {*this};
    }

    ConstSegments HeifReaderImpl::segmentsBySequence() const
    {
        return {*this};
    }

    ErrorCode HeifReaderImpl::isInitialized() const
    {
        if (!(mState == State::INITIALIZING || mState == State::READY))
        {
            return ErrorCode::UNINITIALIZED;
        }
        return ErrorCode::OK;
    }

    void HeifReaderImpl::reset()
    {
        mState = State::UNINITIALIZED;

        mFileInformation  = {};
        mFileProperties   = {};
        mFtyp             = {};
        mIsPrimaryItemSet = false;
        mMetaBox          = {};
        mMetaBoxInfo      = {};
        mMetaBoxLoaded    = false;
        mPrimaryItemId    = 0;

        mImageItemCodeTypeMap.clear();
        mImageItemParameterSetMap.clear();
        mImageToParameterSetMap.clear();
    }

    MetaBoxInformation HeifReaderImpl::convertRootMetaBoxInformation(const MetaBoxProperties& metaboxProperties) const
    {
        MetaBoxInformation metaBoxInformation;
        metaBoxInformation.features = metaboxProperties.metaBoxFeature.getFeatureMask();

        Array<ItemInformation> itemInformation(metaboxProperties.itemFeaturesMap.size());
        size_t i = 0;
        for (const auto& item : metaboxProperties.itemFeaturesMap)
        {
            const ItemInfo& iteminfo = mMetaBoxInfo.itemInfoMap.at(item.first.get());

            itemInformation[i].itemId                      = item.first.get();
            itemInformation[i].type                        = iteminfo.type.getUInt32();
            itemInformation[i].description.name            = makeArray<char>(iteminfo.name);
            itemInformation[i].description.contentType     = makeArray<char>(iteminfo.contentType);
            itemInformation[i].description.contentEncoding = makeArray<char>(iteminfo.contentEncoding);
            itemInformation[i].features                    = item.second.getFeatureMask();

            List<ImageId> pastReferences;
            if ((getItemLength(mMetaBox, item.first.get(), itemInformation[i].size, pastReferences) != ErrorCode::OK) ||
                static_cast<int64_t>(itemInformation[i].size) > mFileProperties.segmentPropertiesMap.at(0).io.size)
            {
                itemInformation[i].size = 0;
            }
            ++i;
        }
        metaBoxInformation.itemInformations = itemInformation;

        Array<EntityGrouping> entityGrouping(metaboxProperties.entityGroupings.size());
        i = 0;
        for (const EntityGrouping& groupings : metaboxProperties.entityGroupings)
        {
            entityGrouping[i].type      = groupings.type;
            entityGrouping[i].entityIds = groupings.entityIds;
            entityGrouping[i].groupId   = groupings.groupId;
            ++i;
        }
        metaBoxInformation.entityGroupings = entityGrouping;

        return metaBoxInformation;
    }

    Array<TrackInformation> HeifReaderImpl::convertTrackInformation(const InitTrackInfoMap& initTrackInfoMap) const
    {
        Array<TrackInformation> trackInfoOut(initTrackInfoMap.size());

        unsigned int i = 0;
        for (const auto& trackInfo : initTrackInfoMap)
        {
            const InitTrackInfo& initTrackInfo = trackInfo.second;
            trackInfoOut[i].trackId            = trackInfo.first;
            trackInfoOut[i].alternateGroupId   = initTrackInfo.alternateGroupId;
            trackInfoOut[i].features           = initTrackInfo.trackFeature.getFeatureMask();
            trackInfoOut[i].alternateTrackIds  = makeArray<SequenceId>(initTrackInfo.alternateTrackIds);
            trackInfoOut[i].referenceTrackIds  = mapToArray(initTrackInfo.referenceTrackIds);
            trackInfoOut[i].sampleGroups       = initTrackInfo.groupedSamples;
            trackInfoOut[i].equivalences       = initTrackInfo.equivalences;
            trackInfoOut[i].metadatas          = initTrackInfo.metadatas;
            trackInfoOut[i].referenceSamples   = initTrackInfo.referenceSamples;
            trackInfoOut[i].maxSampleSize      = initTrackInfo.maxSampleSize;
            trackInfoOut[i].timeScale          = initTrackInfo.timeScale;
            trackInfoOut[i].editList           = initTrackInfo.editList;

            trackInfoOut[i].trackGroupIds = Array<FourCCToIds>(initTrackInfo.trackGroupInfoMap.size());
            unsigned int k                = 0;
            for (auto const& group : initTrackInfo.trackGroupInfoMap)
            {
                trackInfoOut[i].trackGroupIds[k].type     = FourCC(group.first.getUInt32());
                trackInfoOut[i].trackGroupIds[k].trackIds = makeArray<SequenceId>(group.second.ids);
                ++k;
            }

            const SegmentId intializationSegmentId = 0;
            const SamplePropertyVector& samplePropertyVector =
                mFileProperties.segmentPropertiesMap.at(intializationSegmentId)
                    .trackInfos.at(trackInfoOut[i].trackId)
                    .samples;
            Array<SampleInformation> sampleInfo(samplePropertyVector.size());
            unsigned int j = 0;
            for (const auto& sampleProp : samplePropertyVector)
            {
                sampleInfo[j].sampleId                  = sampleProp.sampleId;
                sampleInfo[j].sampleEntryType           = sampleProp.sampleEntryType.getUInt32();
                sampleInfo[j].sampleDescriptionIndex    = sampleProp.sampleDescriptionIndex.get();
                sampleInfo[j].sampleType                = sampleProp.sampleType;
                sampleInfo[j].sampleDurationTS          = sampleProp.sampleDurationTS;
                sampleInfo[j].sampleCompositionOffsetTs = sampleProp.sampleCompositionOffsetTs;
                sampleInfo[j].hasClap                   = sampleProp.hasClap;
                sampleInfo[j].hasAuxi                   = sampleProp.hasAuxi;
                sampleInfo[j].codingConstraints         = sampleProp.codingConstraints;
                sampleInfo[j].size                      = sampleProp.dataLength;
                ++j;
            }
            trackInfoOut[i].sampleProperties = sampleInfo;
            ++i;
        }

        return trackInfoOut;
    }

    ErrorCode HeifReaderImpl::handleMeta(StreamIO& io)
    {
        BitStream bitstream;
        auto error = readBox(io, bitstream);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        MetaBox& metaBox = mMetaBox;
        metaBox.parseBox(bitstream);

        mFileProperties.rootLevelMetaBoxProperties = extractMetaBoxProperties(metaBox);
        mMetaBoxInfo                               = extractItems(metaBox);
        processDecoderConfigProperties(metaBox.getItemPropertiesBox(),
                                       mFileProperties.rootLevelMetaBoxProperties.itemFeaturesMap,
                                       mImageItemParameterSetMap, mImageToParameterSetMap, mImageItemCodeTypeMap);

        Array<ImageId> masterImages;
        getMasterImages(masterImages);
        mMetaBoxInfo.displayableMasterImages = static_cast<uint32_t>(masterImages.size);

        for (const auto& itemEntry : mFileProperties.rootLevelMetaBoxProperties.itemFeaturesMap)
        {
            if (itemEntry.second.hasFeature(ItemFeatureEnum::IsPrimaryImage))
            {
                mIsPrimaryItemSet = true;
                mPrimaryItemId    = itemEntry.first;
            }
        }

        mMetaBoxLoaded = true;

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::handleMoov(StreamIO& io)
    {
        BitStream bitstream;
        SegmentId initializationSegmentId = 0;

        auto error = readBox(io, bitstream);
        if (error == ErrorCode::OK)
        {
            MovieBox moov;
            moov.parseBox(bitstream);

            mFileProperties.moovProperties = extractMoovProperties(moov);
            fillSegmentPropertiesMap(initializationSegmentId, moov, mFileProperties.segmentPropertiesMap);
            mFileProperties.initTrackInfos =
                extractInitTrackInfos(initializationSegmentId, moov, mFileProperties.segmentPropertiesMap);
            mFileProperties.moovProperties.movieTimescale = moov.getMovieHeaderBox().getTimeScale();
            mFileProperties.moovProperties.mMatrix        = moov.getMovieHeaderBox().getMatrix();
        }

        return error;
    }

    ErrorCode HeifReaderImpl::handleSegmentMoof(StreamIO& io,
                                                const SegmentId segmentId,
                                                bool& earliestPTSRead,
                                                Map<SequenceId, DecodePts::PresentationTimeTS>& earliestPTSTS,
                                                uint64_t& earliestPTSinTS)
    {
        // we need to save moof start byte for possible trun dataoffset depending on its flags.
        const StreamInterface::offset_t moofFirstByte = io.stream->tell();

        BitStream bitstream;
        auto error = readBox(io, bitstream);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        MovieFragmentBox moof(mFileProperties.moovProperties.fragmentSampleDefaults);
        moof.setMoofFirstByteOffset(static_cast<uint64_t>(moofFirstByte));
        moof.parseBox(bitstream);

        if (!earliestPTSRead)
        {
            // no sidx information about pts available. Use previous segment last sample pts.
            for (auto& initTrackInfo : mFileProperties.initTrackInfos)
            {
                SequenceId trackId = initTrackInfo.first;
                if (earliestPTSinTS != UINT64_MAX)
                {
                    earliestPTSTS[trackId] = DecodePts::PresentationTimeTS(earliestPTSinTS);
                }
                else if (const TrackInfoInSegment* precTrackInfo =
                             getPrecedingTrackInfo(SegmentTrackId(segmentId, trackId)))
                {
                    earliestPTSTS[trackId] = precTrackInfo->noSidxFallbackPTSTS;
                }
                else
                {
                    earliestPTSTS[trackId] = 0;
                }
            }

            earliestPTSRead = true;
        }

        SequenceIdPresentationTimeTSMap earliestPTSTSForTrack;
        for (auto& trackFragmentBox : moof.getTrackFragmentBoxes())
        {
            SequenceId trackId = trackFragmentBox->getTrackFragmentHeaderBox().getTrackId();
            earliestPTSTSForTrack.insert(std::make_pair(trackId, earliestPTSTS.at(trackId)));
        }

        addToTrackProperties(segmentId, moof, earliestPTSTSForTrack);

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::handleInitSegmentMoof(StreamIO& io, const SegmentId segmentId)
    {
        BitStream bitstream;

        // we need to save moof start byte for possible trun dataoffset depending on its flags.
        const StreamInterface::offset_t moofFirstByte = io.stream->tell();

        auto error = readBox(io, bitstream);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        MovieFragmentBox moof(mFileProperties.moovProperties.fragmentSampleDefaults);
        moof.setMoofFirstByteOffset(static_cast<uint64_t>(moofFirstByte));
        moof.parseBox(bitstream);

        SequenceIdPresentationTimeTSMap earliestPTSTSForTrack;
        addToTrackProperties(segmentId, moof, earliestPTSTSForTrack);

        // Check sample dataoffsets against fragment data size
        SegmentProperties& segmentProperties = mFileProperties.segmentPropertiesMap[segmentId];
        for (auto& trackFragmentBox : moof.getTrackFragmentBoxes())
        {
            SequenceId trackId            = trackFragmentBox->getTrackFragmentHeaderBox().getTrackId();
            TrackInfoInSegment& trackInfo = segmentProperties.trackInfos[trackId];

            if (!trackInfo.samples.empty())
            {
                auto sampleDataEndOffset = static_cast<int64_t>(trackInfo.samples.rbegin()->dataOffset +
                                                                trackInfo.samples.rbegin()->dataLength);
                if (sampleDataEndOffset > io.size || sampleDataEndOffset < 0)
                {
                    throw RuntimeError(
                        "HeifReaderImpl::readStream sample data offset outside of movie fragment "
                        "data");
                }
            }
        }

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::handleFtyp(StreamIO& io)
    {
        BitStream bitstream;
        auto error = readBox(io, bitstream);
        if (error == ErrorCode::OK)
        {
            FileTypeBox ftyp;
            ftyp.parseBox(bitstream);

            // Check supported brands
            Set<String> supportedBrands;
            ftyp.addCompatibleBrand(ftyp.getMajorBrand());
            if (ftyp.checkCompatibleBrand("msf1"))  // contains image sequence
            {
                if (ftyp.checkCompatibleBrand("hevc"))
                {
                    supportedBrands.insert("[msf1/hevc] HEVC image sequence");
                }
                if (ftyp.checkCompatibleBrand("avcs"))
                {
                    supportedBrands.insert("[msf1/avcs] AVC image sequence");
                }
            }
            if (ftyp.checkCompatibleBrand("mif1"))  // contains image collection
            {
                if (ftyp.checkCompatibleBrand("heic"))
                {
                    supportedBrands.insert("[mif1/heic] HEVC image and image collection");
                }
                if (ftyp.checkCompatibleBrand("heix"))
                {
                    supportedBrands.insert("[mif1/heix] HEVC image and image collection");
                }
                if (ftyp.checkCompatibleBrand("avic"))
                {
                    supportedBrands.insert("[mif1/avic] AVC image and image collection");
                }
                if (ftyp.checkCompatibleBrand("jpeg"))
                {
                    supportedBrands.insert("[mif1/jpeg] JPEG image and image collection");
                }
            }

            if (supportedBrands.empty())
            {
                logInfo() << "No supported brands found. Trying to continue parsing anyway." << std::endl;
            }
            else
            {
                logInfo() << "Compatible brands found:" << std::endl;
                for (const auto& brand : supportedBrands)
                {
                    logInfo() << " " << brand << std::endl;
                }
            }
            mFtyp = ftyp;
        }

        return error;
    }

    ErrorCode HeifReaderImpl::handleEtyp(StreamIO& io)
    {
        BitStream bitstream;
        auto error = readBox(io, bitstream);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        mEtyp.parseBox(bitstream);
        if (mEtyp.checkCompatibility({"avcs", "avic", "heic", "heix", "jpeg", "msf1"}) == false)
        {
            logWarning() << "No supported brand combination found from Extended Type Box. Continuing parsing anyway."
                         << std::endl;
        }

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::readStream()
    {
        State prevState = mState;
        mState          = State::INITIALIZING;

        StreamIO& io = mFileProperties.segmentPropertiesMap.at(0).io;
        if (io.stream->peekEof())
        {
            mState = prevState;
            io.stream.reset();
            return ErrorCode::FILE_READ_ERROR;
        }
        io.size = io.stream->size();

        bool ftypFound = false;
        bool etypFound = false;
        bool metaFound = false;
        bool moovFound = false;

        ErrorCode error = ErrorCode::OK;
        if (io.stream->peekEof())
        {
            error = ErrorCode::FILE_HEADER_ERROR;
        }

        try
        {
            while ((error == ErrorCode::OK) && !io.stream->peekEof())
            {
                String boxType;
                std::int64_t boxSize = 0;
                BitStream bitstream;
                error = readBoxParameters(io, boxType, boxSize);
                if (error == ErrorCode::OK)
                {
                    if (boxType == "ftyp")
                    {
                        if (ftypFound)
                        {
                            return ErrorCode::FILE_READ_ERROR;  // Multiple ftyp boxes.
                        }
                        ftypFound = true;
                        error     = handleFtyp(io);
                    }
                    else if (boxType == "etyp")
                    {
                        if (!ftypFound || etypFound)
                        {
                            return ErrorCode::FILE_READ_ERROR;  // Multiple etyp boxes, also must be after ftyp.
                        }
                        etypFound = true;
                        error     = handleEtyp(io);
                    }
                    else if (boxType == "meta")
                    {
                        if (metaFound)
                        {
                            return ErrorCode::FILE_READ_ERROR;  // Multiple root-level meta boxes.
                        }
                        metaFound = true;
                        error     = handleMeta(io);
                    }
                    else if (boxType == "moov")
                    {
                        if (moovFound)
                        {
                            error = ErrorCode::FILE_READ_ERROR;
                            break;
                        }
                        moovFound = true;
                        addSegmentSequence(0, mNextSequence);
                        error = handleMoov(io);
                    }
                    else if (boxType == "moof")
                    {
                        // 0 index of segmentPropertiesMap is reserved for initialization segment data
                        const SegmentId initializationSegmentId = 0;
                        error                                   = handleInitSegmentMoof(io, initializationSegmentId);
                    }
                    else if (boxType == "mdat" || boxType == "free" || boxType == "skip")
                    {
                        // skip 'mdat' as it is handled elsewhere, 'free' can be skipped
                        error = skipBox(io);
                    }
                    else
                    {
                        logWarning() << "Skipping root level box of unknown type '" << boxType << "'" << std::endl;
                        error = skipBox(io);
                    }
                }
            }
        }
        catch (const ISOBMFF::Exception& exc)
        {
            logError() << "readStream Exception Error: " << exc.what() << std::endl;
            error = ErrorCode::FILE_READ_ERROR;
        }
        catch (const std::exception& e)
        {
            logError() << "readStream std::exception Error:: " << e.what() << std::endl;
            error = ErrorCode::FILE_READ_ERROR;
        }

        // Set error if parsing was OK, but either ftyp was missing and neither meta nor moov was found.
        if (((error == ErrorCode::OK) && !ftypFound) || ((error == ErrorCode::OK) && !moovFound && !metaFound))
        {
            error = ErrorCode::FILE_HEADER_ERROR;
        }

        if (error == ErrorCode::OK)
        {
            updateCompositionTimes(0);

            // peek() sets eof bit for the stream. Clear stream to make sure it is still accessible. seekg() in C++11
            // should clear stream after eof, but this does not seem to be always happening.
            if ((!io.stream->good()) && (!io.stream->eof()))
            {
                return ErrorCode::FILE_READ_ERROR;
            }
            io.stream->clear();
            mFileProperties.fileFeature = getFileFeatures();
            mState                      = State::READY;
        }

        return error;
    }

    HeifReaderImpl::ItemInfoMap HeifReaderImpl::extractItemInfoMap(const MetaBox& metaBox)
    {
        ItemInfoMap itemInfoMap;
        const auto& itemIds = metaBox.getItemInfoBox().getItemIds();
        for (const auto itemId : itemIds)
        {
            const ItemInfoEntry& item = metaBox.getItemInfoBox().getItemById(itemId);
            ItemInfo itemInfo         = makeItemInfo(item);

            if (isImageItem(makeItemInfo(item)))
            {
                const ItemPropertiesBox& iprp = metaBox.getItemPropertiesBox();
                const std::uint32_t ispeIndex = iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::ISPE, itemId);
                if (ispeIndex != 0u)
                {
                    auto imageSpatialExtentsProperties =
                        static_cast<const ImageSpatialExtentsProperty*>(iprp.getPropertyByIndex(ispeIndex - 1));
                    itemInfo.height = imageSpatialExtentsProperties->getDisplayHeight();
                    itemInfo.width  = imageSpatialExtentsProperties->getDisplayWidth();
                }
                else
                {
                    logWarning() << "No ImageSpatialExtentsPropertyIndex found for image item id " << itemId
                                 << std::endl;
                }
            }

            itemInfoMap.insert({itemId, itemInfo});
        }

        return itemInfoMap;
    }

    FileFeature HeifReaderImpl::getFileFeatures() const
    {
        FileFeature fileFeature;

        if (mMetaBoxLoaded)
        {
            fileFeature.setFeature(FileFeatureEnum::HasRootLevelMetaBox);
            Array<ImageId> masterImages;
            getMasterImages(masterImages);

            if (masterImages.size == 1)
            {
                fileFeature.setFeature(FileFeatureEnum::HasSingleImage);
            }
            else if (masterImages.size > 1)
            {
                fileFeature.setFeature(FileFeatureEnum::HasImageCollection);
            }
        }
        for (const auto& trackProperties : mFileProperties.initTrackInfos)
        {
            if (trackProperties.second.trackFeature.hasFeature(TrackFeatureEnum::IsMasterImageSequence))
            {
                fileFeature.setFeature(FileFeatureEnum::HasImageSequence);
            }
            if (trackProperties.second.trackFeature.hasFeature(TrackFeatureEnum::HasAlternatives))
            {
                fileFeature.setFeature(FileFeatureEnum::HasAlternateTracks);
            }
        }

        return fileFeature;
    }

    ErrorCode HeifReaderImpl::readBytes(StreamIO& io, const unsigned int count, std::int64_t& result)
    {
        std::int64_t value = 0;

        for (unsigned int i = 0; i < count; ++i)
        {
            value = (value << 8) | static_cast<int64_t>(io.stream->get());
            if (!io.stream->good())
            {
                return ErrorCode::FILE_READ_ERROR;
            }
        }

        result = value;
        return ErrorCode::OK;
    }

    void HeifReaderImpl::seekInput(StreamIO& io, const std::int64_t pos)
    {
        if (io.stream->tell() != pos)
        {
            io.stream->seek(pos);
        }
    }

    ErrorCode HeifReaderImpl::skipBox(StreamIO& io)
    {
        const std::int64_t startLocation = io.stream->tell();

        String boxType;
        std::int64_t boxSize = 0;
        ErrorCode error      = readBoxParameters(io, boxType, boxSize);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        seekInput(io, startLocation + boxSize);
        if (!io.stream->good())
        {
            return ErrorCode::FILE_READ_ERROR;
        }
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::readBox(StreamIO& io, BitStream& bitstream)
    {
        String boxType;
        std::int64_t boxSize = 0;

        ErrorCode error = readBoxParameters(io, boxType, boxSize);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        Vector<uint8_t> data(static_cast<std::uint64_t>(boxSize));
        io.stream->read(reinterpret_cast<char*>(data.data()), boxSize);
        if (!io.stream->good())
        {
            return ErrorCode::FILE_READ_ERROR;
        }
        bitstream.clear();
        bitstream.reset();
        bitstream.write8BitsArray(data, std::uint64_t(boxSize));
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::readBoxParameters(StreamIO& io, String& boxType, std::int64_t& boxSize)
    {
        const std::int64_t startLocation = io.stream->tell();

        // Read the 32-bit length field of the box
        ErrorCode error = readBytes(io, 4, boxSize);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        // Read the four character string for boxType
        static const size_t TYPE_LENGTH = 4;
        boxType.resize(TYPE_LENGTH);
        io.stream->read(&boxType[0], TYPE_LENGTH);
        if (!io.stream->good())
        {
            return ErrorCode::FILE_READ_ERROR;
        }

        // Check if 64-bit largesize field is used
        if (boxSize == 1)
        {
            error = readBytes(io, 8, boxSize);
            if (error != ErrorCode::OK)
            {
                return error;
            }
        }

        int64_t boxEndOffset = startLocation + boxSize;
        if (boxSize < 8 || (boxEndOffset < 8) || ((io.size > 0) && (boxEndOffset > io.size)))
        {
            return ErrorCode::FILE_READ_ERROR;
        }

        // Seek to box beginning
        seekInput(io, startLocation);
        if (!io.stream->good())
        {
            return ErrorCode::FILE_READ_ERROR;
        }
        return ErrorCode::OK;
    }

    ParameterSetMap HeifReaderImpl::makeDecoderParameterSetMap(const DecoderConfigurationRecord& record)
    {
        ParameterSetMap pm;
        DecoderConfigurationRecord::ConfigurationMap tmp;
        record.getConfigurationMap(tmp);
        for (auto t : tmp)
        {
            DecoderSpecInfoType type;
            switch (t.first)
            {
            case DecoderConfigurationRecord::AVC_SPS:
            {
                type = DecoderSpecInfoType::AVC_SPS;
                break;
            }
            case DecoderConfigurationRecord::AVC_PPS:
            {
                type = DecoderSpecInfoType::AVC_PPS;
                break;
            }
            case DecoderConfigurationRecord::HEVC_VPS:
            {
                type = DecoderSpecInfoType::HEVC_VPS;
                break;
            }
            case DecoderConfigurationRecord::HEVC_SPS:
            {
                type = DecoderSpecInfoType::HEVC_SPS;
                break;
            }
            case DecoderConfigurationRecord::HEVC_PPS:
            {
                type = DecoderSpecInfoType::HEVC_PPS;
                break;
            }
            case DecoderConfigurationRecord::AudioSpecificConfig:
            {
                type = DecoderSpecInfoType::AudioSpecificConfig;
                break;
            }
            case DecoderConfigurationRecord::JPEG:
            {
                type = DecoderSpecInfoType::JPEG;
                break;
            }
            default:
            {
                type = static_cast<DecoderSpecInfoType>(t.first);
                break;
            }
            }
            auto& e = pm[type];
            e.insert(e.begin(), t.second.begin(), t.second.end());
        }
        return pm;
    }

    void HeifReaderImpl::getCollectionItems(Vector<ImageId>& items) const
    {
        items.clear();
        for (const auto& imageInfo : mMetaBoxInfo.itemInfoMap)
        {
            if (isImageItem(imageInfo.second))
            {
                items.push_back(imageInfo.first);
            }
        }
    }

    ErrorCode HeifReaderImpl::getSamples(SequenceId sequenceId, Vector<SequenceImageId>& samples) const
    {
        ErrorCode error = isValidTrack(sequenceId);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        samples.clear();
        for (const auto& segment : segmentsBySequence())
        {
            SegmentId segmentId       = segment.segmentId;
            SegmentTrackId segTrackId = std::make_pair(segmentId, sequenceId);

            if (hasTrackInfo(segTrackId))
            {
                samples.reserve(getTrackInfo(segTrackId).samples.size());
                for (const auto& sampleInfo : getTrackInfo(segTrackId).samples)
                {
                    samples.push_back(sampleInfo.sampleId);
                }
            }
        }
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProtection(const ImageId itemId, bool& isProtected) const
    {
        ItemInfoEntry entry;
        try
        {
            entry = mMetaBox.getItemInfoBox().getItemById(itemId.get());
        }
        catch (...)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }

        isProtected = false;
        if (entry.getItemProtectionIndex() > 0)
        {
            isProtected = true;
        }
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::processAvcItemData(uint8_t* memoryBuffer, uint64_t& memoryBufferSize)
    {
        uint32_t outputOffset = 0;
        uint32_t byteOffset   = 0;

        while (outputOffset < memoryBufferSize)
        {
            uint32_t nalLength                      = memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 0;
            byteOffset++;
            nalLength                               = (nalLength << 8) | memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 0;
            byteOffset++;
            nalLength                               = (nalLength << 8) | memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 0;
            byteOffset++;
            nalLength                               = (nalLength << 8) | memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 1;
            // byteOffset++;
            // AvcNalUnitType naluType = AvcNalUnitType((uint8_t)memoryBuffer[outputOffset + byteOffset] & 0x1f);
            outputOffset += nalLength + 4;  // 4 bytes of nal length information
            byteOffset = 0;
        }
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::processHevcItemData(uint8_t* memoryBuffer, uint64_t& memoryBufferSize)
    {
        uint32_t outputOffset = 0;
        uint32_t byteOffset   = 0;

        while (outputOffset < memoryBufferSize)
        {
            uint32_t nalLength                      = memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 0;
            byteOffset++;
            nalLength                               = (nalLength << 8) | memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 0;
            byteOffset++;
            nalLength                               = (nalLength << 8) | memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 0;
            byteOffset++;
            nalLength                               = (nalLength << 8) | memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 1;
            // byteOffset++;
            // HevcNalUnitType naluType = HevcNalUnitType(((uint8_t)memoryBuffer[outputOffset + byteOffset] >> 1) &
            // 0x3f);
            outputOffset += nalLength + 4;  // 4 bytes of nal length information
            byteOffset = 0;
        }
        return ErrorCode::OK;
    }

    /* ********************************************************************** */
    /* *********************** Meta-specific methods  *********************** */
    /* ********************************************************************** */

    ErrorCode HeifReaderImpl::isValidImageItem(const ImageId& imageId) const
    {
        ErrorCode error;
        if ((error = isInitialized()) != ErrorCode::OK)
        {
            return error;
        }

        ItemInfoEntry item;
        try
        {
            item = mMetaBox.getItemInfoBox().getItemById(imageId.get());
        }
        catch (...)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }

        if (isImageItem(makeItemInfo(item)))
        {
            return ErrorCode::OK;
        }
        return ErrorCode::INVALID_ITEM_ID;
    }

    ErrorCode HeifReaderImpl::isValidItem(const ImageId& imageId) const
    {
        ErrorCode error;
        if ((error = isInitialized()) != ErrorCode::OK)
        {
            return error;
        }
        try
        {
            mMetaBox.getItemInfoBox().getItemById(imageId.get()).getItemType();
        }
        catch (...)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }

        return ErrorCode::OK;
    }

    MetaBoxProperties HeifReaderImpl::extractMetaBoxProperties(const MetaBox& metaBox)
    {
        MetaBoxProperties metaBoxProperties;
        metaBoxProperties.itemFeaturesMap = extractMetaBoxItemPropertiesMap(metaBox);
        metaBoxProperties.entityGroupings = extractMetaBoxEntityToGroupMaps(metaBox);
        metaBoxProperties.metaBoxFeature =
            extractMetaBoxFeatures(metaBoxProperties.itemFeaturesMap, metaBoxProperties.entityGroupings);

        return metaBoxProperties;
    }

    MetaBoxFeature HeifReaderImpl::extractMetaBoxFeatures(const ItemFeaturesMap& imageFeatures,
                                                          const Groupings& groupings)
    {
        MetaBoxFeature metaBoxFeature;

        if (!groupings.empty())
        {
            metaBoxFeature.setFeature(MetaBoxFeatureEnum::HasGroupLists);
        }

        if (imageFeatures.size() == 1)
        {
            metaBoxFeature.setFeature(MetaBoxFeatureEnum::IsSingleImage);
        }
        else if (imageFeatures.size() > 1)
        {
            metaBoxFeature.setFeature(MetaBoxFeatureEnum::IsImageCollection);
        }

        for (const auto& i : imageFeatures)
        {
            const ItemFeature features = i.second;

            if (features.hasFeature(ItemFeatureEnum::IsMasterImage))
            {
                metaBoxFeature.setFeature(MetaBoxFeatureEnum::HasMasterImages);
            }
            if (features.hasFeature(ItemFeatureEnum::IsThumbnailImage))
            {
                metaBoxFeature.setFeature(MetaBoxFeatureEnum::HasThumbnails);
            }
            if (features.hasFeature(ItemFeatureEnum::IsAuxiliaryImage))
            {
                metaBoxFeature.setFeature(MetaBoxFeatureEnum::HasAuxiliaryImages);
            }
            if (features.hasFeature(ItemFeatureEnum::IsDerivedImage))
            {
                metaBoxFeature.setFeature(MetaBoxFeatureEnum::HasDerivedImages);
            }
            if (features.hasFeature(ItemFeatureEnum::IsPreComputedDerivedImage))
            {
                metaBoxFeature.setFeature(MetaBoxFeatureEnum::HasPreComputedDerivedImages);
            }
            if (features.hasFeature(ItemFeatureEnum::IsHiddenImage))
            {
                metaBoxFeature.setFeature(MetaBoxFeatureEnum::HasHiddenImages);
            }
        }

        return metaBoxFeature;
    }

    Groupings HeifReaderImpl::extractMetaBoxEntityToGroupMaps(const MetaBox& metaBox)
    {
        Groupings groupings;

        const auto& entityToGroupBoxes = metaBox.getGroupsListBox().getEntityToGroupsBoxes();
        for (const EntityToGroupBox& box : entityToGroupBoxes)
        {
            EntityGrouping entityGrouping;
            entityGrouping.groupId   = box.getGroupId();
            entityGrouping.type      = FourCC(box.getType().getUInt32());
            entityGrouping.entityIds = makeArray<uint32_t>(box.getEntityIds());
            groupings.push_back(entityGrouping);
        }

        return groupings;
    }

    ItemFeaturesMap HeifReaderImpl::extractMetaBoxItemPropertiesMap(const MetaBox& metaBox)
    {
        ItemFeaturesMap itemFeaturesMap;
        const auto& itemIds = metaBox.getItemInfoBox().getItemIds();

        for (const auto itemId : itemIds)
        {
            const ItemInfoEntry& item = metaBox.getItemInfoBox().getItemById(itemId);

            ItemFeature itemFeatures;
            if (isImageItem(makeItemInfo(item)))
            {
                if (item.getItemProtectionIndex() > 0)
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsProtected);
                }

                if (doReferencesFromItemIdExist(metaBox, itemId, FourCCInt("pred")))
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsPredictivelyCodedImage);
                }
                if (doReferencesFromItemIdExist(metaBox, itemId, FourCCInt("thmb")))
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsThumbnailImage);
                }
                if (doReferencesFromItemIdExist(metaBox, itemId, FourCCInt("auxl")))
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsAuxiliaryImage);
                }
                if (doReferencesFromItemIdExist(metaBox, itemId, FourCCInt("base")))
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsPreComputedDerivedImage);
                }
                if (doReferencesFromItemIdExist(metaBox, itemId, FourCCInt("dimg")))
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsDerivedImage);
                }
                // Is this master image (<=> not a thumb and not an auxiliary image)
                if (!itemFeatures.hasFeature(ItemFeatureEnum::IsThumbnailImage) &&
                    !itemFeatures.hasFeature(ItemFeatureEnum::IsAuxiliaryImage))
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsMasterImage);
                }

                if (doReferencesToItemIdExist(metaBox, itemId, FourCCInt("thmb")))
                {
                    itemFeatures.setFeature(ItemFeatureEnum::HasLinkedThumbnails);
                }
                if (doReferencesToItemIdExist(metaBox, itemId, FourCCInt("auxl")))
                {
                    itemFeatures.setFeature(ItemFeatureEnum::HasLinkedAuxiliaryImage);
                }
                if (doReferencesToItemIdExist(metaBox, itemId, FourCCInt("cdsc")))
                {
                    itemFeatures.setFeature(ItemFeatureEnum::HasLinkedMetadata);
                }
                if (doReferencesToItemIdExist(metaBox, itemId, FourCCInt("base")))
                {
                    itemFeatures.setFeature(ItemFeatureEnum::HasLinkedPreComputedDerivedImage);
                }
                if (doReferencesToItemIdExist(metaBox, itemId, FourCCInt("tbas")))
                {
                    itemFeatures.setFeature(ItemFeatureEnum::HasLinkedTiles);
                }
                if (doReferencesToItemIdExist(metaBox, itemId, FourCCInt("dimg")))
                {
                    itemFeatures.setFeature(ItemFeatureEnum::HasLinkedDerivedImage);
                }

                if (metaBox.getPrimaryItemBox().getItemId() == itemId)
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsPrimaryImage);
                    itemFeatures.setFeature(ItemFeatureEnum::IsCoverImage);
                }

                static const uint32_t HIDDEN_IMAGE_MASK = 0x1;
                if ((item.getFlags() & HIDDEN_IMAGE_MASK) != 0u)
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsHiddenImage);
                }
            }
            else
            {
                const auto type = item.getItemType();
                if (item.getItemProtectionIndex() > 0)
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsProtected);
                }

                if (doReferencesFromItemIdExist(metaBox, itemId, "cdsc"))
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsMetadataItem);
                }

                if (type == "Exif")
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsExifItem);
                }
                else if (type == "mime")
                {
                    if (item.getContentType() == "application/rdf+xml")
                    {
                        itemFeatures.setFeature(ItemFeatureEnum::IsXMPItem);
                    }
                    else
                    {
                        itemFeatures.setFeature(ItemFeatureEnum::IsMPEG7Item);
                    }
                }
                else if (type == "hvt1")
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsTileImageItem);
                }
            }

            {
                const auto& iprp              = metaBox.getItemPropertiesBox();
                const std::uint32_t rrefIndex = iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::RREF, itemId);
                if (rrefIndex != 0u)
                {
                    auto rref =
                        static_cast<const RequiredReferenceTypesProperty*>(iprp.getPropertyByIndex(rrefIndex - 1));
                    auto referenceTypes                                     = rref->getReferenceTypes();
                    static const vector<FourCCInt> EXPECTED_REFERENCE_TYPES = {"auxl", "base", "thmb",
                                                                               "exbl", "tbas", "pred"};
                    for (const auto referenceType : referenceTypes)
                    {
                        if (find(EXPECTED_REFERENCE_TYPES.cbegin(), EXPECTED_REFERENCE_TYPES.cend(), referenceType) ==
                            EXPECTED_REFERENCE_TYPES.cend())
                        {
                            logWarning() << "Reading image item id " << itemId << " with unknown reference type '"
                                         << referenceType.getString()
                                         << "' in associated RequiredReferenceTypesProperty." << endl;
                            itemFeatures.setFeature(ItemFeatureEnum::HasUnrecognzedRequiredReferences);
                        }
                    }
                }
            }

            itemFeaturesMap[itemId] = itemFeatures;
        }

        return itemFeaturesMap;
    }

    static ItemPropertyType itemPropertyTypeFromPropertyType(const ItemPropertiesBox::PropertyType& aType)
    {
#define PROPERTY_MAPPING(X) ItemPropertiesBox::PropertyType::X, ItemPropertyType::X
        const Map<ItemPropertiesBox::PropertyType, ItemPropertyType> typeMapping{
            {PROPERTY_MAPPING(RAW)},  {PROPERTY_MAPPING(ALTT)}, {PROPERTY_MAPPING(AUXC)}, {PROPERTY_MAPPING(AVCC)},
            {PROPERTY_MAPPING(CLAP)}, {PROPERTY_MAPPING(COLR)}, {PROPERTY_MAPPING(CRTT)}, {PROPERTY_MAPPING(HVCC)},
            {PROPERTY_MAPPING(IMIR)}, {PROPERTY_MAPPING(IROT)}, {PROPERTY_MAPPING(ISCL)}, {PROPERTY_MAPPING(ISPE)},
            {PROPERTY_MAPPING(JPGC)}, {PROPERTY_MAPPING(MDFT)}, {PROPERTY_MAPPING(PASP)}, {PROPERTY_MAPPING(PIXI)},
            {PROPERTY_MAPPING(RLOC)}, {PROPERTY_MAPPING(RREF)}, {PROPERTY_MAPPING(UDES)},
        };
#undef PROPERTY_MAPPING

        const auto type = typeMapping.find(aType);
        if (type != typeMapping.cend())
        {
            return type->second;
        }

        return ItemPropertyType::RAW;
    }

    HeifReaderImpl::Properties HeifReaderImpl::processItemProperties() const
    {
        Properties propertyMap;
        const ItemPropertiesBox& iprp = mMetaBox.getItemPropertiesBox();

        // Collect item properties
        const auto& itemIds = mMetaBox.getItemInfoBox().getItemIds();
        for (const auto itemId : itemIds)
        {
            const ItemPropertiesBox::PropertyInfos& propertyVector = iprp.getItemProperties(itemId);

            // The following loop copies item property information to interface. Data structures are essentially
            // identical in ItemPropertiesBox and the reader API, but it is not desirable to expose ItemPropertiesBox in
            // the API, or include reader interface as part of ItemPropertiesBox.
            PropertyTypeVector propertyTypeVector;
            for (const auto& property : propertyVector)
            {
                ItemPropertyInfo info;
                info.essential = property.essential;
                info.index     = property.index;
                info.type      = itemPropertyTypeFromPropertyType(property.type);
                propertyTypeVector.push_back(info);
            }
            propertyMap.insert(std::make_pair(itemId, propertyTypeVector));
        }

        // Collect entity group properties
        std::vector<uint32_t> groupIds;
        const auto& entityToGroupBoxes = mMetaBox.getGroupsListBox().getEntityToGroupsBoxes();
        groupIds.reserve(entityToGroupBoxes.size());

        for (const EntityToGroupBox& box : entityToGroupBoxes)
        {
            groupIds.push_back(box.getGroupId());
        }
        for (const auto groupId : groupIds)
        {
            const ItemPropertiesBox::PropertyInfos& propertyVector = iprp.getItemProperties(groupId);

            // The following loop copies item property information to interface. Data structures are essentially
            // identical in ItemPropertiesBox and the reader API, but it is not desirable to expose ItemPropertiesBox in
            // the API, or include reader interface as part of ItemPropertiesBox.
            PropertyTypeVector propertyTypeVector;
            for (const auto& property : propertyVector)
            {
                ItemPropertyInfo info;
                info.essential = property.essential;
                info.index     = property.index;
                info.type      = itemPropertyTypeFromPropertyType(property.type);
                propertyTypeVector.push_back(info);
            }
            propertyMap.insert(std::make_pair(groupId, propertyTypeVector));
        }

        return propertyMap;
    }

    void HeifReaderImpl::processDecoderConfigProperties(const ItemPropertiesBox& iprp,
                                                        const ItemFeaturesMap& itemFeaturesMap,
                                                        Map<DecoderConfigId, ParameterSetMap>& imageItemParameterSetMap,
                                                        Map<ImageId, DecoderConfigId>& imageToParameterSetMap,
                                                        Map<ImageId, FourCCInt>& imageItemCodeTypeMap)
    {
        for (const auto& imageProperties : itemFeaturesMap)
        {
            const ImageId imageId = imageProperties.first;
            const std::uint32_t hvccIndex =
                iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::HVCC, imageId.get());
            const std::uint32_t avccIndex =
                iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::AVCC, imageId.get());
            const std::uint32_t jpegIndex =
                iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::JPGC, imageId.get());

            FourCCInt type;
            DecoderConfigId configIndex           = 0;
            const DecoderConfigurationBox* record = nullptr;
            if (hvccIndex != 0u)
            {
                configIndex = hvccIndex;
                type        = "hvc1";
            }
            else if (avccIndex != 0u)
            {
                configIndex = avccIndex;
                type        = "avc1";
            }
            else if (jpegIndex != 0u)
            {
                configIndex = jpegIndex;
                type        = "jpeg";
            }
            else
            {
                continue;
            }
            record = static_cast<const DecoderConfigurationBox*>(iprp.getPropertyByIndex(configIndex.get() - 1));
            if (imageItemParameterSetMap.count(configIndex) == 0u)
            {
                imageItemParameterSetMap[configIndex] = makeDecoderParameterSetMap(record->getConfiguration());
            }
            imageToParameterSetMap[imageId] = configIndex;
            imageItemCodeTypeMap[imageId]   = type;
        }
    }

    HeifReaderImpl::MetaBoxInfo HeifReaderImpl::extractItems(const MetaBox& metaBox) const
    {
        MetaBoxInfo metaBoxInfo;
        const auto& itemIds = metaBox.getItemInfoBox().getItemIds();
        for (const auto itemId : itemIds)
        {
            const ItemInfoEntry& item = metaBox.getItemInfoBox().getItemById(itemId);
            const auto type           = item.getItemType();
            if (type == "grid" || type == "iovl")
            {
                bool isProtected = false;
                ErrorCode error  = getProtection(itemId, isProtected);
                if (error != ErrorCode::OK || isProtected)
                {
                    continue;
                }

                BitStream bitstream;
                error = loadItemData(metaBox, itemId, bitstream.getStorage());
                if (error != ErrorCode::OK)
                {
                    continue;
                }

                if (type == "grid")
                {
                    const ImageGrid& imageGrid = parseImageGrid(bitstream);
                    Grid grid;
                    grid.columns      = imageGrid.columnsMinusOne + 1u;
                    grid.rows         = imageGrid.rowsMinusOne + 1u;
                    grid.outputWidth  = imageGrid.outputWidth;
                    grid.outputHeight = imageGrid.outputHeight;
                    getReferencedFromItemListByType(itemId, "dimg", grid.imageIds);
                    metaBoxInfo.gridItems.insert({itemId, grid});
                }
                if (type == "iovl")
                {
                    const ImageOverlay& imageOverlay = parseImageOverlay(bitstream);
                    Overlay iovl;
                    iovl.r            = imageOverlay.canvasFillValueR;
                    iovl.g            = imageOverlay.canvasFillValueG;
                    iovl.b            = imageOverlay.canvasFillValueB;
                    iovl.a            = imageOverlay.canvasFillValueA;
                    iovl.outputWidth  = imageOverlay.outputWidth;
                    iovl.outputHeight = imageOverlay.outputHeight;

                    iovl.offsets = Array<Overlay::Offset>(imageOverlay.offsets.size());
                    for (size_t i = 0; i < imageOverlay.offsets.size(); ++i)
                    {
                        iovl.offsets[i] = Overlay::Offset{imageOverlay.offsets[i].horizontalOffset,
                                                          imageOverlay.offsets[i].verticalOffset};
                    }

                    getReferencedFromItemListByType(itemId, "dimg", iovl.imageIds);
                    metaBoxInfo.iovlItems.insert({itemId, iovl});
                }
            }
        }

        metaBoxInfo.properties  = processItemProperties();
        metaBoxInfo.itemInfoMap = extractItemInfoMap(metaBox);

        return metaBoxInfo;
    }

    ErrorCode HeifReaderImpl::loadItemData(const MetaBox& metaBox, const ImageId itemId, DataVector& data) const
    {
        const auto& io = mFileProperties.segmentPropertiesMap.at(0).io;

        const streampos oldPosition = io.stream->tell();

        uint64_t itemLength(0);
        List<ImageId> pastReferences;
        ErrorCode error = getItemLength(metaBox, itemId, itemLength, pastReferences);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        if (static_cast<int64_t>(itemLength) > io.size)

        {
            return ErrorCode::FILE_HEADER_ERROR;
        }
        data.resize(itemLength);

        uint8_t* dataPtr = data.data();
        error            = readItem(metaBox, itemId, dataPtr, itemLength);
        io.stream->seek(oldPosition);
        return error;
    }

    ErrorCode HeifReaderImpl::getItemLength(const MetaBox& metaBox,
                                            const ImageId& itemId,
                                            std::uint64_t& itemLength,
                                            List<ImageId>& pastReferences) const
    {
        ErrorCode error = isValidItem(itemId);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        const auto& io = mFileProperties.segmentPropertiesMap.at(0).io;

        // to prevent infinite loop of items <-> subitem
        auto findIter = std::find(pastReferences.begin(), pastReferences.end(), itemId);
        if (findIter == pastReferences.end())
        {
            pastReferences.push_back(itemId);
        }
        else
        {
            return ErrorCode::FILE_HEADER_ERROR;
        }

        const ItemLocationBox& iloc = metaBox.getItemLocationBox();
        const unsigned int version  = iloc.getVersion();
        if (!iloc.hasItemIdEntry(itemId.get()))
        {
            itemLength = 0;
            return ErrorCode::INVALID_ITEM_ID;
        }
        const ItemLocation& itemLocation                          = iloc.getItemLocationForID(itemId.get());
        const ItemLocation::ConstructionMethod constructionMethod = itemLocation.getConstructionMethod();
        const ExtentList& extentList                              = itemLocation.getExtentList();

        if (extentList.empty())
        {
            return ErrorCode::FILE_READ_ERROR;  // No extents given for an item.
        }

        // The size of the item is the sum of the extent lengths.
        itemLength = 0;
        if ((version >= 1) && constructionMethod == ItemLocation::ConstructionMethod::ITEM_OFFSET)
        {
            // Request list of 'iloc' type item references, and assemble the length of the item recursively.
            const auto& allIlocReferences = metaBox.getItemReferenceBox().getReferencesOfType("iloc");
            auto isWantedItemId           = [itemId](const SingleItemTypeReferenceBox& item) {
                return item.getFromItemID() == itemId;
            };
            const auto& ilocReference =
                std::find_if(allIlocReferences.cbegin(), allIlocReferences.cend(), isWantedItemId);
            if (ilocReference == allIlocReferences.end())
            {
                return ErrorCode::FILE_READ_ERROR;
            }
            const auto& toItemIds = ilocReference->getToItemIds();

            // Iterate extents
            for (const auto& extent : extentList)
            {
                //  If index_size is 0, then the value 1 of 'iloc' type reference index is implied.
                uint64_t extentSourceItemIndex = 1;
                if (iloc.getIndexSize() != 0)
                {
                    extentSourceItemIndex = extent.mExtentIndex;
                }

                uint64_t subItemLength(0);
                const ImageId subItemId = toItemIds.at(extentSourceItemIndex - 1);
                if (itemId == subItemId)
                {
                    return ErrorCode::FILE_HEADER_ERROR;  // avoid looping references.
                }

                error = getItemLength(metaBox, subItemId, subItemLength, pastReferences);
                if (error != ErrorCode::OK)
                {
                    return error;
                }
                if (static_cast<int64_t>(subItemLength) > io.size)

                {
                    return ErrorCode::FILE_HEADER_ERROR;
                }

                // If extent_length value = 0, length is the length of the entire item.
                if (extent.mExtentLength == 0)
                {
                    itemLength += subItemLength;
                }
                else
                {
                    itemLength += extent.mExtentLength;
                }
            }
        }
        else
        {
            for (const auto& extent : extentList)
            {
                itemLength += extent.mExtentLength;
            }
        }

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::readItem(const MetaBox& metaBox,
                                       const ImageId itemId,
                                       uint8_t* memoryBuffer,
                                       uint64_t maxSize) const
    {
        ErrorCode error = isValidItem(itemId);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        const auto& io = mFileProperties.segmentPropertiesMap.at(0).io;

        const ItemLocationBox& iloc = metaBox.getItemLocationBox();
        const unsigned int version  = iloc.getVersion();
        if (!iloc.hasItemIdEntry(itemId.get()))
        {
            return ErrorCode::INVALID_ITEM_ID;
        }
        const ItemLocation& itemLocation                          = iloc.getItemLocationForID(itemId.get());
        const ItemLocation::ConstructionMethod constructionMethod = itemLocation.getConstructionMethod();
        const ExtentList& extentList                              = itemLocation.getExtentList();
        const std::uint64_t baseOffset                            = itemLocation.getBaseOffset();

        if (extentList.empty())
        {
            return ErrorCode::FILE_READ_ERROR;  // No extents given for an item.
        }

        uint64_t totalLenght(0);
        if (version == 0 || ((version >= 1) && constructionMethod == ItemLocation::ConstructionMethod::FILE_OFFSET))
        {
            for (const auto& extent : extentList)
            {
                const auto offset = static_cast<std::int64_t>(baseOffset + extent.mExtentOffset);
                io.stream->seek(offset);
                if (!io.stream->good())
                {
                    return ErrorCode::FILE_READ_ERROR;
                }
                if (totalLenght + extent.mExtentLength > maxSize)
                {
                    return ErrorCode::FILE_READ_ERROR;
                }
                io.stream->read(reinterpret_cast<char*>(memoryBuffer), std::streamsize(extent.mExtentLength));
                if (!io.stream->good())
                {
                    return ErrorCode::FILE_READ_ERROR;
                }
                totalLenght += extent.mExtentLength;
                memoryBuffer += extent.mExtentLength;
            }
        }
        else if ((version >= 1) && (constructionMethod == ItemLocation::ConstructionMethod::IDAT_OFFSET))
        {
            for (const auto& extent : extentList)
            {
                const size_t offset = baseOffset + extent.mExtentOffset;
                if (totalLenght + extent.mExtentLength > maxSize)
                {
                    return ErrorCode::FILE_READ_ERROR;
                }
                if (!metaBox.getItemDataBox().read(memoryBuffer, offset, extent.mExtentLength))
                {
                    return ErrorCode::FILE_READ_ERROR;
                }
                totalLenght += extent.mExtentLength;
                memoryBuffer += extent.mExtentLength;
            }
        }
        else if ((version >= 1) && (constructionMethod == ItemLocation::ConstructionMethod::ITEM_OFFSET))
        {
            // Request list of 'iloc' type item references, and assemble the data of the item recursively.
            const auto& allIlocReferences = metaBox.getItemReferenceBox().getReferencesOfType("iloc");
            auto isWantedItemId           = [itemId](const SingleItemTypeReferenceBox& item) {
                return item.getFromItemID() == itemId;
            };
            const auto& ilocReference =
                std::find_if(allIlocReferences.cbegin(), allIlocReferences.cend(), isWantedItemId);
            const auto& toItemIds = ilocReference->getToItemIds();

            // Iterate extents
            for (const auto& extent : extentList)
            {
                //  If index_size is 0, then the value 1 of 'iloc' type reference index is implied.
                uint64_t extentSourceItemIndex = 1;
                if (iloc.getIndexSize() != 0)
                {
                    extentSourceItemIndex = extent.mExtentIndex;
                }

                const ImageId subItemId = toItemIds.at(extentSourceItemIndex - 1);
                if (itemId == subItemId)
                {
                    return ErrorCode::FILE_HEADER_ERROR;
                }
                uint64_t subItemLength(0);
                List<ImageId> pastReferences;
                pastReferences.push_back(itemId);
                error = getItemLength(metaBox, subItemId, subItemLength, pastReferences);
                if (error != ErrorCode::OK)
                {
                    return error;
                }
                if (static_cast<int64_t>(subItemLength) > io.size)

                {
                    return ErrorCode::FILE_HEADER_ERROR;
                }

                Vector<std::uint8_t> subItemData;
                subItemData.resize(subItemLength);

                uint8_t* subItemDataPtr = subItemData.data();
                error                   = readItem(metaBox, subItemId, subItemDataPtr, subItemLength);
                if (error != ErrorCode::OK)
                {
                    return error;
                }

                // If extent_length value = 0, length is the length of the entire item.
                if (extent.mExtentLength == 0)
                {
                    if (totalLenght + subItemData.size() > maxSize)
                    {
                        return ErrorCode::FILE_READ_ERROR;
                    }
                    std::memcpy(memoryBuffer, subItemData.data(), subItemData.size());
                    totalLenght += subItemData.size();
                    memoryBuffer += subItemData.size();
                }
                else
                {
                    if (totalLenght + extent.mExtentLength > maxSize ||
                        extent.mExtentOffset + extent.mExtentLength > subItemData.size())
                    {
                        return ErrorCode::FILE_READ_ERROR;
                    }
                    std::memcpy(memoryBuffer, subItemData.data() + static_cast<int64_t>(extent.mExtentOffset),
                                extent.mExtentLength);
                    totalLenght += extent.mExtentLength;
                    memoryBuffer += extent.mExtentLength;
                }
            }
        }
        else
        {
            return ErrorCode::FILE_READ_ERROR;
        }

        return ErrorCode::OK;
    }


    /* *********************************************************************** */
    /* *********************** Track-specific methods  *********************** */
    /* *********************************************************************** */

    void HeifReaderImpl::updateDecoderCodeTypeMap(const SamplePropertyVector& sampleInfo,
                                                  WriteOnceMap<SequenceImageId, FourCCInt>& decoderCodeTypeMap,
                                                  std::size_t prevSampleInfoSize)
    {
        for (std::size_t sampleIndex = prevSampleInfoSize; sampleIndex < sampleInfo.size(); ++sampleIndex)
        {
            const auto& info = sampleInfo[sampleIndex];
            decoderCodeTypeMap.insert(
                std::make_pair(info.sampleId, info.sampleEntryType));  // Store decoder type for track data decoding
        }
    }

    ErrorCode HeifReaderImpl::isValidTrack(const SequenceId& sequenceId) const
    {
        ErrorCode error;
        if ((error = isInitialized()) != ErrorCode::OK)
        {
            return error;
        }
        if (mFileProperties.initTrackInfos.count(sequenceId) != 0)
        {
            return ErrorCode::OK;
        }
        return ErrorCode::INVALID_SEQUENCE_ID;
    }

    ErrorCode HeifReaderImpl::isValidSample(const SequenceId& sequenceId, const SequenceImageId& sequenceImageId) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }

        SegmentId segmentId;
        error = segmentIdOf(sequenceId, sequenceImageId, segmentId);

        return error;
    }

    InitTrackInfoMap HeifReaderImpl::extractInitTrackInfos(SegmentId segmentId,
                                                           const MovieBox& moovBox,
                                                           const SegmentPropertiesMap& segmentPropertiesMap)
    {
        InitTrackInfoMap initTrackInfoMap;

        const Vector<UniquePtr<TrackBox>>& trackBoxes = moovBox.getTrackBoxes();
        for (const auto& trackBoxP : trackBoxes)
        {
            const TrackBox* trackBox = trackBoxP.get();
            const SampleDescriptionBox& stsdBox =
                trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox().getSampleDescriptionBox();
            InitTrackInfo initTrackInfo = extractInitTrackInfo(trackBox);
            SequenceId sequenceId       = trackBox->getTrackHeaderBox().getTrackID();
            const auto& trackInfo       = segmentPropertiesMap.at(segmentId).trackInfos.at(sequenceId);

            std::uint64_t maxSampleSize = 0;
            makeSamplePropertyVector(trackBox, maxSampleSize);  //  discard return value, just get max sample size

            fillSampleEntryMap(stsdBox, initTrackInfo);

            initTrackInfo.trackId           = sequenceId.get();
            initTrackInfo.trackFeature      = getTrackFeatures(trackBox);
            initTrackInfo.referenceTrackIds = getReferenceTrackIds(trackBox);
            initTrackInfo.trackGroupInfoMap = getTrackGroupInfoMap(trackBox);
            initTrackInfo.groupedSamples    = getSampleGroupings(trackBox);
            initTrackInfo.equivalences      = getEquivalenceGroups(trackBox);
            initTrackInfo.metadatas         = getSampleToMetadataItemGroups(trackBox);
            initTrackInfo.referenceSamples  = getDirectReferenceSamplesGroups(trackBox);
            initTrackInfo.alternateTrackIds = getAlternateTrackIds(trackBox, moovBox);
            initTrackInfo.alternateGroupId  = trackBox->getTrackHeaderBox().getAlternateGroup();
            initTrackInfo.maxSampleSize     = maxSampleSize;
            initTrackInfo.timeScale         = trackBox->getMediaBox().getMediaHeaderBox().getTimeScale();
            initTrackInfo.editList          = getEditList(trackBox, trackInfo.repetitions);
            initTrackInfo.editBox           = trackBox->getEditBox();

            if (initTrackInfo.trackFeature.hasFeature(TrackFeatureEnum::HasEditList) && (trackInfo.pMap.size() <= 1))
            {
                initTrackInfo.trackFeature.setFeature(TrackFeatureEnum::DisplayAllSamples);
            }

            initTrackInfoMap[sequenceId] = initTrackInfo;
        }

        // Some TrackFeatures are easiest to set after a part of properties have already been filled.
        for (auto& mapEntry : initTrackInfoMap)
        {
            const SequenceId trackId   = mapEntry.first;
            TrackFeature& trackFeature = mapEntry.second.trackFeature;
            if (isAnyLinkedToWithType(initTrackInfoMap, trackId, "thmb"))
            {
                trackFeature.setFeature(TrackFeatureEnum::HasLinkedThumbnailImageSequence);
            }
            if (isAnyLinkedToWithType(initTrackInfoMap, trackId, "auxl"))
            {
                trackFeature.setFeature(TrackFeatureEnum::HasLinkedAuxiliaryImageSequence);
            }
        }

        return initTrackInfoMap;
    }

    void HeifReaderImpl::fillSegmentPropertiesMap(SegmentId segmentId,
                                                  const MovieBox& moovBox,
                                                  SegmentPropertiesMap& segmentPropertiesMap)
    {
        const Vector<UniquePtr<TrackBox>>& trackBoxes = moovBox.getTrackBoxes();
        for (const auto& trackBoxP : trackBoxes)
        {
            const TrackBox* trackBox = trackBoxP.get();
            TrackInfoInSegment trackInfo =
                createTrackInfoInSegment(trackBox, moovBox.getMovieHeaderBox().getTimeScale());
            SequenceId sequenceId = trackBox->getTrackHeaderBox().getTrackID();

            std::uint64_t maxSampleSize = 0;
            trackInfo.samples           = makeSamplePropertyVector(trackBox, maxSampleSize);

            updateSampleToParametersSetMap(segmentPropertiesMap[segmentId].sampleToParameterSetMap, sequenceId,
                                           trackInfo.samples);

            updateDecoderCodeTypeMap(trackInfo.samples, trackInfo.decoderCodeTypeMap);

            segmentPropertiesMap[segmentId].trackInfos[sequenceId] = trackInfo;
        }
    }


    Vector<SequenceId> HeifReaderImpl::getAlternateTrackIds(const TrackBox* trackBox, const MovieBox& moovBox)
    {
        Vector<SequenceId> trackIds;
        const std::uint16_t alternateGroupId = trackBox->getTrackHeaderBox().getAlternateGroup();

        if (alternateGroupId == 0)
        {
            return trackIds;
        }

        const SequenceId trackId                      = trackBox->getTrackHeaderBox().getTrackID();
        const Vector<UniquePtr<TrackBox>>& trackBoxes = moovBox.getTrackBoxes();
        for (const auto& trackboxP : trackBoxes)
        {
            const TrackBox* trackbox          = trackboxP.get();
            const SequenceId alternateTrackId = trackbox->getTrackHeaderBox().getTrackID();
            if ((trackId != alternateTrackId) &&
                (alternateGroupId == trackbox->getTrackHeaderBox().getAlternateGroup()))
            {
                trackIds.push_back(alternateTrackId);
            }
        }

        return trackIds;
    }

    bool HeifReaderImpl::isAnyLinkedToWithType(const InitTrackInfoMap& trackPropertiesMap,
                                               const SequenceId trackId,
                                               const FourCCInt referenceType)
    {
        for (const auto& trackProperties : trackPropertiesMap)
        {
            for (const auto& reference : trackProperties.second.referenceTrackIds)
            {
                if ((reference.first == FourCC(referenceType.getUInt32())) &&
                    (std::find(reference.second.begin(), reference.second.end(), trackId) != reference.second.end()))
                {
                    return true;
                }
            }
        }

        return false;
    }

    TrackFeature HeifReaderImpl::getTrackFeatures(const TrackBox* trackBox)
    {
        TrackFeature trackFeature;

        const TrackHeaderBox& tkhdBox       = trackBox->getTrackHeaderBox();
        const HandlerBox& handlerBox        = trackBox->getMediaBox().getHandlerBox();
        const SampleTableBox& stblBox       = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
        const SampleDescriptionBox& stsdBox = stblBox.getSampleDescriptionBox();

        if (handlerBox.getHandlerType() == "pict" || handlerBox.getHandlerType() == "auxv" ||
            handlerBox.getHandlerType() == "vide" || handlerBox.getHandlerType() == "soun")
        {
            std::bitset<24> flags(tkhdBox.getFlags());
            if (flags.test(0))
            {
                trackFeature.setFeature(TrackFeatureEnum::IsEnabled);
            }
            if (flags.test(1))
            {
                trackFeature.setFeature(TrackFeatureEnum::IsInMovie);
            }
            if (flags.test(2))
            {
                trackFeature.setFeature(TrackFeatureEnum::IsInPreview);
            }

            if (tkhdBox.getAlternateGroup() != 0)
            {
                trackFeature.setFeature(TrackFeatureEnum::HasAlternatives);
            }

            if (!trackBox->getHasTrackReferences() && handlerBox.getHandlerType() == "pict")
            {
                // sample entry box has 'pict', and is not referencing any another track
                trackFeature.setFeature(TrackFeatureEnum::IsMasterImageSequence);
            }
            else
            {
                if (trackBox->getTrackReferenceBox().isReferenceTypePresent("thmb"))
                {
                    trackFeature.setFeature(TrackFeatureEnum::IsThumbnailImageSequence);
                }

                if (trackBox->getTrackReferenceBox().isReferenceTypePresent("auxl"))
                {
                    trackFeature.setFeature(TrackFeatureEnum::IsAuxiliaryImageSequence);
                }
            }

            if (handlerBox.getHandlerType() == "soun")
            {
                trackFeature.setFeature(TrackFeatureEnum::Feature::IsAudioTrack);
            }

            else if (handlerBox.getHandlerType() == "vide")
            {
                trackFeature.setFeature(TrackFeatureEnum::Feature::IsVideoTrack);
            }

            if (handlerBox.getHandlerType() != "soun")
            {
                const auto& sampleEntries = stsdBox.getSampleEntries();
                for (const auto& sampleEntry : sampleEntries)
                {
                    auto visualSampleEntry = static_cast<const VisualSampleEntryBox*>(sampleEntry.get());
                    if ((visualSampleEntry != nullptr) && visualSampleEntry->isCodingConstraintsBoxPresent())
                    {
                        trackFeature.setFeature(TrackFeatureEnum::HasCodingConstraints);
                        break;
                    }
                }
            }

            // Sample groupings
            if (!stblBox.getSampleToGroupBoxes().empty())
            {
                // HasSampleGroups
                trackFeature.setFeature(TrackFeatureEnum::HasSampleGroups);

                // HasSampleToItemGrouping, HasSampleEquivalenceGrouping
                const auto& boxes = stblBox.getSampleToGroupBoxes();
                for (const SampleToGroupBox& box : boxes)
                {
                    const auto groupingType = box.getGroupingType();

                    if (groupingType == "stmi")
                    {
                        trackFeature.setFeature(TrackFeatureEnum::HasSampleToItemGrouping);
                    }
                    else if (groupingType == "eqiv")
                    {
                        trackFeature.setFeature(TrackFeatureEnum::HasSampleEquivalenceGrouping);
                    }
                }
            }

            std::shared_ptr<const EditBox> editBox = trackBox->getEditBox();
            if (editBox)
            {
                const EditListBox* editListBox = editBox->getEditListBox();
                if (editListBox != nullptr)
                {
                    trackFeature.setFeature(TrackFeatureEnum::HasEditList);
                    // Edit list box flag == 1 determines infinite looping
                    if (editListBox->getFlags() == 1 && tkhdBox.getDuration() == 0xffffffff)
                    {
                        trackFeature.setFeature(TrackFeatureEnum::HasInfiniteLoopPlayback);
                    }
                }
            }
        }

        return trackFeature;
    }

    TypeToIdsMap HeifReaderImpl::getReferenceTrackIds(const TrackBox* trackBox)
    {
        TypeToIdsMap trackReferenceMap;

        const auto& trackReferenceTypeBoxes = trackBox->getTrackReferenceBox().getTrefTypeBoxes();
        for (const auto& trackReferenceTypeBox : trackReferenceTypeBoxes)
        {
            const auto& trackIds = trackReferenceTypeBox.getTrackIds();
            const Vector<SequenceId> ids(trackIds.cbegin(), trackIds.end());
            trackReferenceMap[trackReferenceTypeBox.getType().getUInt32()] = ids;
        }

        return trackReferenceMap;
    }

    EditList HeifReaderImpl::getEditList(const TrackBox* trackBox, const double repetitions)
    {
        EditList editlist{};
        const std::shared_ptr<const EditBox> editBox = trackBox->getEditBox();
        // const auto mediaTimescale                    = trackBox->getMediaBox().getMediaHeaderBox().getTimeScale();

        if (editBox != nullptr)
        {
            const EditListBox* editListBox = editBox->getEditListBox();
            std::uint32_t version          = editListBox->getVersion();
            if ((editListBox->getFlags() & 1) == 1)
            {
                editlist.looping  = true;
                uint64_t duration = trackBox->getTrackHeaderBox().getDuration();
                if (((trackBox->getTrackHeaderBox().getVersion() == 0) &&
                     (duration == std::numeric_limits<uint32_t>::max())) ||
                    (duration == std::numeric_limits<uint64_t>::max()))
                {
                    editlist.repetitions = 0.0;
                }
                else
                {
                    editlist.repetitions = repetitions;
                }
            }
            else
            {
                editlist.repetitions = 1.0;
            }

            Vector<EditUnit> editUnits;
            editUnits.reserve(editListBox->numEntry());
            for (std::uint32_t i = 0; i < editListBox->numEntry(); i++)
            {
                EditUnit editUnit{};
                if (version == 0)
                {
                    const auto& mEntryVersion0 = editListBox->getEntry<EditListBox::EntryVersion0>(i);
                    if (mEntryVersion0.mMediaTime == -1)
                    {
                        editUnit.editType           = EditType::EMPTY;
                        editUnit.mediaTimeInTrackTS = 0;
                    }
                    else if (mEntryVersion0.mMediaRateInteger == 0)
                    {
                        editUnit.editType           = EditType::DWELL;
                        editUnit.mediaTimeInTrackTS = mEntryVersion0.mMediaTime;
                    }
                    else
                    {
                        editUnit.editType           = EditType::SHIFT;
                        editUnit.mediaTimeInTrackTS = mEntryVersion0.mMediaTime;
                    }
                    editUnit.durationInMovieTS = mEntryVersion0.mSegmentDuration;
                    editUnit.mediaRateInteger  = mEntryVersion0.mMediaRateInteger;
                    editUnit.mediaRateFraction = mEntryVersion0.mMediaRateFraction;
                }
                else
                {
                    const auto& mEntryVersion1 = editListBox->getEntry<EditListBox::EntryVersion1>(i);
                    if (mEntryVersion1.mMediaTime == -1)
                    {
                        editUnit.editType           = EditType::EMPTY;
                        editUnit.mediaTimeInTrackTS = 0;
                    }
                    else if (mEntryVersion1.mMediaRateInteger == 0)
                    {
                        editUnit.editType           = EditType::DWELL;
                        editUnit.mediaTimeInTrackTS = mEntryVersion1.mMediaTime;
                    }
                    else
                    {
                        editUnit.editType           = EditType::SHIFT;
                        editUnit.mediaTimeInTrackTS = mEntryVersion1.mMediaTime;
                    }
                    editUnit.durationInMovieTS = mEntryVersion1.mSegmentDuration;
                    editUnit.mediaRateInteger  = mEntryVersion1.mMediaRateInteger;
                    editUnit.mediaRateFraction = mEntryVersion1.mMediaRateFraction;
                }

                editUnits.push_back(editUnit);
            }
            editlist.editUnits = makeArray<EditUnit>(editUnits);
        }
        return editlist;
    }

    Array<SampleGrouping> HeifReaderImpl::getSampleGroupings(const TrackBox* trackBox)
    {
        Vector<SampleGrouping> groupings;

        const SampleTableBox& stblBox = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
        const Vector<SampleToGroupBox>& sampleToGroupBoxes = stblBox.getSampleToGroupBoxes();
        for (const auto& sampleToGroupBox : sampleToGroupBoxes)
        {
            Vector<SampleAndEntryIds> sampleIdWithGroupIds;
            const unsigned int numberOfSamples = sampleToGroupBox.getNumberOfSamples();
            for (unsigned int i = 0; i < numberOfSamples; ++i)
            {
                const auto groupDescriptionIndex = sampleToGroupBox.getSampleGroupDescriptionIndex(i);
                if (groupDescriptionIndex != 0)
                {
                    SampleAndEntryIds entry;
                    entry.sampleId                    = i;
                    entry.sampleGroupDescriptionIndex = groupDescriptionIndex;

                    sampleIdWithGroupIds.push_back(entry);
                }
            }
            SampleGrouping groupedSamples;
            groupedSamples.type          = sampleToGroupBox.getGroupingType().getUInt32();
            groupedSamples.typeParameter = sampleToGroupBox.getGroupingTypeParameter();
            groupedSamples.samples       = makeArray<SampleAndEntryIds>(sampleIdWithGroupIds);

            groupings.push_back(groupedSamples);
        }

        return makeArray<SampleGrouping>(groupings);
    }

    Array<SampleVisualEquivalence> HeifReaderImpl::getEquivalenceGroups(const TrackBox* trackBox)
    {
        const SampleTableBox& stblBox         = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
        const SampleGroupDescriptionBox* sgpd = stblBox.getSampleGroupDescriptionBox("eqiv");
        if (sgpd == nullptr)
        {
            return Array<SampleVisualEquivalence>();
        }
        const auto entries = sgpd->getEntryCount();
        Array<SampleVisualEquivalence> eqivInfos(entries);
        for (unsigned int groupIndex = 1; groupIndex < (entries + 1); ++groupIndex)
        {
            auto eqiv = static_cast<const VisualEquivalenceEntry*>(sgpd->getEntry(groupIndex));

            eqivInfos[groupIndex - 1].sampleGroupDescriptionIndex = groupIndex;
            eqivInfos[groupIndex - 1].timeOffset                  = eqiv->getTimeOffset();
            eqivInfos[groupIndex - 1].timescaleMultiplier         = eqiv->getTimescaleMultiplier();
        }

        return eqivInfos;
    }

    Array<SampleToMetadataItem> HeifReaderImpl::getSampleToMetadataItemGroups(const TrackBox* trackBox)
    {
        const SampleTableBox& stblBox         = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
        const SampleGroupDescriptionBox* sgpd = stblBox.getSampleGroupDescriptionBox("stmi");
        if (sgpd == nullptr)
        {
            return Array<SampleToMetadataItem>();
        }
        const auto entries = sgpd->getEntryCount();
        Array<SampleToMetadataItem> stmiInfos(entries);
        for (unsigned int groupIndex = 1; groupIndex < (entries + 1); ++groupIndex)
        {
            auto stmi = static_cast<const SampleToMetadataItemEntry*>(sgpd->getEntry(groupIndex));
            if (stmi->getMetaBoxHandlerType() == "pict")
            {
                stmiInfos[groupIndex - 1].sampleGroupDescriptionIndex = groupIndex;
                stmiInfos[groupIndex - 1].metadataItemIds             = makeArray<ImageId>(stmi->getItemIds());
            }
        }
        return stmiInfos;
    }

    Array<DirectReferenceSamples> HeifReaderImpl::getDirectReferenceSamplesGroups(const TrackBox* trackBox)
    {
        const SampleTableBox& stblBox         = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
        const SampleGroupDescriptionBox* sgpd = stblBox.getSampleGroupDescriptionBox("refs");
        if (sgpd == nullptr)
        {
            return Array<DirectReferenceSamples>();
        }
        const auto entries = sgpd->getEntryCount();
        Array<DirectReferenceSamples> refsInfos(entries);
        for (unsigned int groupIndex = 1; groupIndex < (entries + 1); ++groupIndex)
        {
            auto refs = static_cast<const DirectReferenceSamplesList*>(sgpd->getEntry(groupIndex));

            refsInfos[groupIndex - 1].sampleGroupDescriptionIndex = groupIndex;
            refsInfos[groupIndex - 1].sampleId                    = refs->getSampleId();
            refsInfos[groupIndex - 1].referenceItemIds =
                makeArray<SequenceImageId>(refs->getDirectReferenceSampleIds());
        }
        return refsInfos;
    }

    InitTrackInfo HeifReaderImpl::extractInitTrackInfo(const TrackBox* trackBox)
    {
        InitTrackInfo initTrackInfo;

        const MediaHeaderBox& mdhdBox        = trackBox->getMediaBox().getMediaHeaderBox();
        const SampleTableBox& stblBox        = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
        const TrackHeaderBox& trackHeaderBox = trackBox->getTrackHeaderBox();
        std::shared_ptr<const CompositionOffsetBox> compositionOffsetBox = stblBox.getCompositionOffsetBox();

        initTrackInfo.matrix = trackHeaderBox.getMatrix();

        initTrackInfo.width  = trackHeaderBox.getWidth() >> 16;
        initTrackInfo.height = trackHeaderBox.getHeight() >> 16;

        const SampleDescriptionBox& stsdBox = stblBox.getSampleDescriptionBox();
        FourCCInt handlerType               = trackBox->getMediaBox().getHandlerBox().getHandlerType();

        if (handlerType == "vide" || handlerType == "pict")
        {
            auto sampleEntry = static_cast<const VisualSampleEntryBox*>(
                stsdBox.getSampleEntry(1));  // get 1 index as truns sampleEntryType wont care
            if (sampleEntry != nullptr)
            {
                initTrackInfo.sampleEntryType = sampleEntry->getType();
            }
        }
        else if (handlerType == "soun")
        {
            auto sampleEntry = static_cast<const AudioSampleEntryBox*>(
                stsdBox.getSampleEntry(1));  // get 1 index as truns sampleEntryType wont care
            if (sampleEntry != nullptr)
            {
                initTrackInfo.sampleEntryType = sampleEntry->getType();
            }
        }

        initTrackInfo.timeScale = mdhdBox.getTimeScale();  // The number of time units that pass in a second

        return initTrackInfo;
    }

    TrackInfoInSegment HeifReaderImpl::createTrackInfoInSegment(const TrackBox* trackBox, const uint32_t movieTimescale)
    {
        TrackInfoInSegment trackInfo;

        const MediaHeaderBox& mdhdBox          = trackBox->getMediaBox().getMediaHeaderBox();
        const SampleTableBox& stblBox          = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
        const TrackHeaderBox& trackHeaderBox   = trackBox->getTrackHeaderBox();
        const TimeToSampleBox& timeToSampleBox = stblBox.getTimeToSampleBox();
        std::shared_ptr<const CompositionOffsetBox> compositionOffsetBox = stblBox.getCompositionOffsetBox();

        const uint32_t mediaTimeScale = mdhdBox.getTimeScale();
        const uint64_t tkhdDuration   = trackHeaderBox.getDuration();  // Duration is in timescale units

        std::shared_ptr<const EditBox> editBox = trackBox->getEditBox();
        DecodePts decodePts;
        decodePts.loadBox(&timeToSampleBox);
        decodePts.loadBox(compositionOffsetBox.get());
        if (editBox != nullptr)
        {
            trackInfo.hasEditList          = true;
            const EditListBox* editListBox = editBox->getEditListBox();
            decodePts.loadBox(editListBox, movieTimescale, mediaTimeScale);
        }
        if (!decodePts.unravel())
        {
            throw FileReaderException(ErrorCode::FILE_HEADER_ERROR);
        }

        // Always generate track duration regardless of the information in the header
        trackInfo.durationTS = DecodePts::PresentationTimeTS(decodePts.getSpan());
        trackInfo.pMap       = decodePts.getTime(mediaTimeScale);
        trackInfo.pMapTS     = decodePts.getTimeTS();

        static const uint32_t DURATION_FROM_EDIT_LIST = 0xffffffff;
        if (tkhdDuration == DURATION_FROM_EDIT_LIST)
        {
            trackInfo.duration = static_cast<double>(decodePts.getSpan()) / mediaTimeScale;
        }
        else
        {
            trackInfo.duration = tkhdDuration / static_cast<double>(movieTimescale);
        }

        if (editBox != nullptr)
        {
            // HEIF defines that when (flags & 1) is equal to 1, the entire edit list is repeated a sufficient
            // number of times to equal the track duration.
            if ((editBox->getEditListBox()->getFlags() & 1) == 1)
            {
                DecodePts::PMap repeatingPMap;
                DecodePts::PMapTS repeatingPMapTS;
                const auto trackDuration      = static_cast<int64_t>(trackInfo.duration * 1000u);
                const auto editListDuration   = static_cast<int64_t>(decodePts.getSpan() * 1000u / mediaTimeScale);
                const auto editListDurationTS = decodePts.getSpan();
                trackInfo.repetitions         = double(trackDuration) / double(editListDuration);
                auto iter                     = trackInfo.pMap.cbegin();
                int64_t nextSampleTimestamp   = iter->first;
                int64_t offset                = 0;
                auto iterTS                   = trackInfo.pMapTS.cbegin();
                int64_t nextSampleTimestampTS = iterTS->first;
                int64_t offsetTS              = 0;

                while (nextSampleTimestamp < trackDuration)
                {
                    repeatingPMap.insert(nextSampleTimestamp, iter->second);
                    ++iter;

                    repeatingPMapTS.insert(nextSampleTimestampTS, iterTS->second);
                    ++iterTS;

                    // Increase timestamp offset and skip to begin if the end was reached.
                    if (iter == trackInfo.pMap.cend())
                    {
                        iter = trackInfo.pMap.cbegin();
                        offset += editListDuration;
                    }
                    nextSampleTimestamp = iter->first + offset;

                    if (iterTS == trackInfo.pMapTS.cend())
                    {
                        iterTS = trackInfo.pMapTS.cbegin();
                        offsetTS += editListDurationTS;
                    }
                    nextSampleTimestampTS = iterTS->first + offsetTS;
                }

                trackInfo.pMap   = repeatingPMap;
                trackInfo.pMapTS = repeatingPMapTS;
            }
        }

        // Read track type if exists
        if (trackBox->getHasTrackTypeBox())
        {
            trackInfo.hasTtyp = true;
            trackInfo.ttyp    = trackBox->getTrackTypeBox();
        }

        return trackInfo;
    }

    SamplePropertyVector HeifReaderImpl::makeSamplePropertyVector(const TrackBox* trackBox,
                                                                  std::uint64_t& maxSampleSize)
    {
        SamplePropertyVector sampleInfoVector;

        const SampleTableBox& stblBox       = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
        const SampleDescriptionBox& stsdBox = stblBox.getSampleDescriptionBox();
        const SampleToChunkBox& stscBox     = stblBox.getSampleToChunkBox();
        const ChunkOffsetBox& stcoBox       = stblBox.getChunkOffsetBox();
        const SampleSizeBox& stszBox        = stblBox.getSampleSizeBox();
        const FourCCInt handlerType         = trackBox->getMediaBox().getHandlerBox().getHandlerType();

        const Vector<uint32_t>& sampleSizeEntries          = stszBox.getEntrySize();
        const Vector<uint64_t>& chunkOffsets               = stcoBox.getChunkOffsets();
        const Vector<SampleToGroupBox>& sampleToGroupBoxes = stblBox.getSampleToGroupBoxes();

        const TimeToSampleBox& sttsBox           = stblBox.getTimeToSampleBox();
        const Vector<std::uint32_t> sampleDeltas = sttsBox.getSampleDeltas();

        const unsigned int sampleCount = stszBox.getSampleCount();

        if (sampleCount > sampleSizeEntries.size())
        {
            throw FileReaderException(ErrorCode::FILE_HEADER_ERROR);
        }

        std::uint32_t previousChunkIndex = 0;  // Index is 1-based so 0 will not be used.
        std::uint64_t maxSize            = 0;
        for (uint32_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        {
            SampleProperties sampleProperties{};
            std::uint32_t sampleDescriptionIndex = 0;
            if (!stscBox.getSampleDescriptionIndex(sampleIndex, sampleDescriptionIndex))
            {
                throw FileReaderException(ErrorCode::FILE_HEADER_ERROR);
            }

            // Set basic sample information
            sampleProperties.sampleId               = sampleIndex;
            sampleProperties.dataLength             = sampleSizeEntries.at(sampleIndex);
            sampleProperties.sampleDescriptionIndex = sampleDescriptionIndex;

            if (sampleProperties.dataLength > maxSize)
            {
                maxSize = sampleProperties.dataLength;
            }

            std::uint32_t chunkIndex = 0;
            if (!stscBox.getSampleChunkIndex(sampleIndex, chunkIndex))
            {
                throw FileReaderException(ErrorCode::FILE_HEADER_ERROR);
            }

            if (chunkIndex == previousChunkIndex)
            {
                sampleProperties.dataOffset = sampleInfoVector.back().dataOffset + sampleInfoVector.back().dataLength;
            }
            else
            {
                sampleProperties.dataOffset = chunkOffsets.at(chunkIndex - 1);
                previousChunkIndex          = chunkIndex;
            }

            sampleProperties.sampleDurationTS = sampleDeltas.at(sampleIndex);

            // Set dimensions
            if (handlerType == "pict" || handlerType == "vide" || handlerType == "auxv")
            {
                auto sampleEntry =
                    static_cast<const VisualSampleEntryBox*>(stsdBox.getSampleEntry(sampleDescriptionIndex));
                if (sampleEntry != nullptr)
                {
                    sampleProperties.sampleEntryType = sampleEntry->getType();
                    sampleProperties.width           = sampleEntry->getWidth();
                    sampleProperties.height          = sampleEntry->getHeight();
                    const auto& ccst                 = sampleEntry->getCodingConstraintsBox();
                    if (ccst != nullptr)
                    {
                        // Store values from CodingConstraintsBox
                        sampleProperties.codingConstraints.allRefPicsIntra = ccst->getAllRefPicsIntra();
                        sampleProperties.codingConstraints.intraPredUsed   = ccst->getIntraPredUsed();
                        sampleProperties.codingConstraints.maxRefPerPic    = ccst->getMaxRefPicUsed();
                    }
                    else
                    {
                        logError() << "Error: Coding Constraints Box not present in a sample description entry.";
                    }
                    sampleProperties.hasClap = (sampleEntry->getClap() != nullptr);
                    sampleProperties.hasAuxi = (sampleEntry->getAuxi() != nullptr);
                }
                else
                {
                    // unknown sample entry, set to zero:
                    sampleProperties.width             = 0;
                    sampleProperties.height            = 0;
                    sampleProperties.hasClap           = false;
                    sampleProperties.hasAuxi           = false;
                    sampleProperties.codingConstraints = {};
                }
            }
            else  // non-visual track
            {
                sampleProperties.width  = 0;
                sampleProperties.height = 0;
            }

            if (stblBox.hasSyncSampleBox())
            {
                // will be filled later based on sync sample box.
                sampleProperties.sampleType = OUTPUT_NON_REFERENCE_FRAME;
            }
            else
            {
                // By default, set as output reference frame (groupings can change this later)
                // By definition, an output reference frame MAY be used as a reference for other samples.
                sampleProperties.sampleType = OUTPUT_REFERENCE_FRAME;
            }

            if (handlerType == "pict" || handlerType == "vide" || handlerType == "auxv")
            {
                for (const auto& sampleToGroupBox : sampleToGroupBoxes)
                {
                    const uint32_t sampleGroupDescriptionIndex =
                        sampleToGroupBox.getSampleGroupDescriptionIndex(sampleIndex);
                    if (sampleGroupDescriptionIndex != 0)
                    {
                        const auto groupingType = FourCC(sampleToGroupBox.getGroupingType().getUInt32());
                        if (groupingType == "refs")
                        {
                            // If there is a "refs" sample grouping, then there is a chance that the sample is an output
                            // non-reference frame If there is no "refs" sample group, then all samples are potentially
                            // output reference frames.
                            const auto sgdb = stblBox.getSampleGroupDescriptionBox(FourCCInt(groupingType.value));
                            if (sgdb == nullptr)
                            {
                                throw FileReaderException(ErrorCode::FILE_HEADER_ERROR);
                            }
                            auto entry = static_cast<const DirectReferenceSamplesList*>(
                                sgdb->getEntry(sampleGroupDescriptionIndex));
                            if (entry->getSampleId() == 0)
                            {
                                // This means that there is at least one reference sample of this sample, hence it is a
                                // non-reference frame.
                                sampleProperties.sampleType = OUTPUT_NON_REFERENCE_FRAME;
                            }
                        }
                    }
                }
            }
            else if (handlerType == "soun")
            {
                auto sampleEntry = static_cast<const SampleEntryBox*>(
                    stsdBox.getSampleEntry(sampleProperties.sampleDescriptionIndex.get()));
                if (sampleEntry != nullptr)
                {
                    sampleProperties.sampleEntryType = sampleEntry->getType();
                }
                // all samples are reference frames for audio:
                sampleProperties.sampleType = OUTPUT_REFERENCE_FRAME;
            }

            // Figure out decode dependencies
            for (const auto& sampleToGroupBox : sampleToGroupBoxes)
            {
                const uint32_t sampleGroupDescriptionIndex =
                    sampleToGroupBox.getSampleGroupDescriptionIndex(sampleIndex);
                if (sampleGroupDescriptionIndex != 0)
                {
                    const FourCCInt groupingType = sampleToGroupBox.getGroupingType();

                    if (groupingType == FourCCInt("refs"))
                    {
                        const SampleGroupDescriptionBox* sgdb = stblBox.getSampleGroupDescriptionBox(groupingType);
                        if (sgdb == nullptr)
                        {
                            throw FileReaderException(ErrorCode::FILE_HEADER_ERROR);
                        }
                        sampleProperties.decodeDependencies =
                            getSampleDirectDependencies(sampleIndex, sgdb, sampleToGroupBox);
                    }
                }
            }

            sampleInfoVector.push_back(sampleProperties);
        }

        if (stblBox.hasSyncSampleBox())
        {
            const Vector<std::uint32_t>& syncSamples = stblBox.getSyncSampleBox()->getSyncSampleIds();
            for (unsigned int i : syncSamples)
            {
                std::uint32_t syncSample    = i - 1;
                auto& sampleProperties      = sampleInfoVector.at(syncSample);
                sampleProperties.sampleType = OUTPUT_REFERENCE_FRAME;
            }
        }

        // handle hidden samples:
        const CompositionOffsetBox* ctts = stblBox.getCompositionOffsetBox().get();
        if (ctts != nullptr)
        {
            const Vector<int32_t>& offsets = ctts->getSampleCompositionOffsets();
            const int32_t min              = std::numeric_limits<int32_t>::min();
            for (size_t i = 0; i < offsets.size(); i++)
            {
                if (sampleInfoVector.size() > static_cast<uint32_t>(i))
                {
                    sampleInfoVector.at(static_cast<uint32_t>(i)).sampleCompositionOffsetTs = offsets.at(i);
                    if (offsets.at(i) == min)
                    {
                        sampleInfoVector.at(static_cast<uint32_t>(i)).sampleType =
                            SampleType::NON_OUTPUT_REFERENCE_FRAME;
                    }
                }
            }
        }

        maxSampleSize = maxSize;
        return sampleInfoVector;
    }

    bool HeifReaderImpl::hasTrackInfo(SegmentTrackId segTrackId) const
    {
        const auto& segmentProperties = mFileProperties.segmentPropertiesMap.find(segTrackId.first);
        if (segmentProperties != mFileProperties.segmentPropertiesMap.end())
        {
            const auto& trackInfo = segmentProperties->second.trackInfos.find(segTrackId.second);
            return trackInfo != segmentProperties->second.trackInfos.end();
        }

        return false;
    }

    const TrackInfoInSegment& HeifReaderImpl::getTrackInfo(SegmentTrackId segTrackId) const
    {
        return mFileProperties.segmentPropertiesMap.at(segTrackId.first).trackInfos.at(segTrackId.second);
    }

    HEIF::ErrorCode HeifReaderImpl::getSampleInfo(SequenceId sequenceId,
                                                  SequenceImageId sequenceImageId,
                                                  SampleProperties& sampleInfo) const
    {
        SegmentId segmentId;
        ErrorCode error = segmentIdOf(sequenceId, sequenceImageId, segmentId);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        SegmentTrackId segTrackId = std::make_pair(segmentId, sequenceId);

        SequenceImageId sampleBase;
        const auto& sampleInfoVector = getSampleInfo(segTrackId, sampleBase);
        sampleInfo                   = sampleInfoVector.at(sequenceImageId.get() - sampleBase.get());

        return ErrorCode::OK;
    }

    const SamplePropertyVector& HeifReaderImpl::getSampleInfo(SegmentTrackId segTrackId,
                                                              SequenceImageId& sampleBase) const
    {
        const auto& segmentProperties = mFileProperties.segmentPropertiesMap.at(segTrackId.first);
        sampleBase                    = getTrackInfo(segTrackId).itemIdBase;
        return segmentProperties.trackInfos.at(segTrackId.second).samples;
    }

    Vector<SequenceImageId> HeifReaderImpl::getSampleDirectDependencies(const SequenceImageId itemId,
                                                                        const SampleGroupDescriptionBox* sgpd,
                                                                        const SampleToGroupBox& sampleToGroupBox)
    {
        const uint32_t index = sampleToGroupBox.getSampleGroupDescriptionIndex(itemId.get());
        auto entry           = static_cast<const DirectReferenceSamplesList*>(sgpd->getEntry(index));

        const auto& sampleIds = entry->getDirectReferenceSampleIds();

        // IDs from entry are not sample IDs (in item decoding order), they have be mapped to sample ids
        Vector<SequenceImageId> ids;
        for (auto entryId : sampleIds)
        {
            const uint32_t entryIndex = sgpd->getEntryIndexOfSampleId(entryId);
            ids.push_back(sampleToGroupBox.getSampleId(entryIndex));
        }

        return ids;
    }

    void HeifReaderImpl::fillSampleEntryMap(const SampleDescriptionBox& stsdBox, InitTrackInfo& initTrackInfo)
    {
        const auto& sampleEntries = stsdBox.getSampleEntries();
        auto& parameterSetMaps    = initTrackInfo.parameterSetMaps;
        unsigned int index        = 1;
        for (const auto& entryBox : sampleEntries)
        {
            const SampleEntryBox* entry = entryBox.get();
            // FourCCInt type = entry->getType();
            if (entry != nullptr)
            {
                parameterSetMaps[index] = makeDecoderParameterSetMap(*entry->getConfigurationRecord());

                if (entry->isVisual())
                {
                    auto visual                     = static_cast<const VisualSampleEntryBox*>(entry);
                    const CleanApertureBox* clapBox = visual->getClap();
                    if (clapBox != nullptr)
                    {
                        initTrackInfo.clapProperties.insert(std::make_pair(index, makeClap(clapBox)));
                    }
                    const AuxiliaryTypeInfoBox* auxiBox = visual->getAuxi();
                    if (auxiBox != nullptr)
                    {
                        initTrackInfo.auxiProperties.insert(std::make_pair(index, makeAuxi(auxiBox)));
                    }

                    SampleSizeInPixels size = {visual->getWidth(), visual->getHeight()};
                    initTrackInfo.sampleSizeInPixels.insert(std::make_pair(index, size));
                }
            }
            ++index;
        }
    }

    Vector<TimestampIDPair>
    HeifReaderImpl::addDecodingDependencies(const SequenceId sequenceId,
                                            const Vector<TimestampIDPair>& itemDecodingOrder) const
    {
        Vector<TimestampIDPair> output;

        // Add dependencies for each sample
        for (const auto& entry : itemDecodingOrder)
        {
            Array<SequenceImageId> dependencies;
            getDecodeDependencies(sequenceId, entry.itemId, dependencies);

            // If only one dependency is given, it is the sample itself, so it is not added.
            if (!(dependencies.size == 1 && dependencies[0] == entry.itemId))
            {
                for (const auto& sampleId : dependencies)
                {
                    output.push_back(TimestampIDPair{0xffffffff, sampleId});
                }
            }
            output.push_back(entry);
        }

        return output;
    }

    SequenceImageId HeifReaderImpl::getFollowingSequenceImageId(SegmentTrackId segTrackId) const
    {
        SegmentId segmentId            = segTrackId.first;
        SequenceId trackId             = segTrackId.second;
        SequenceImageId nextItemIdBase = 0;
        if (mFileProperties.segmentPropertiesMap.count(segmentId) != 0u)
        {
            const auto& segmentProperties = mFileProperties.segmentPropertiesMap.at(segmentId);
            if (segmentProperties.trackInfos.count(trackId) != 0u)
            {
                const auto& trackInfo = segmentProperties.trackInfos.at(trackId);
                if (!trackInfo.samples.empty())
                {
                    nextItemIdBase = trackInfo.samples.rbegin()->sampleId.get() + 1;
                }
                else
                {
                    nextItemIdBase = trackInfo.itemIdBase;
                }
            }
        }
        return nextItemIdBase;
    }

    SequenceImageId HeifReaderImpl::getPrecedingSequenceImageId(SegmentTrackId segTrackId) const
    {
        SegmentId precedingSegmentId;
        SequenceId trackId     = segTrackId.second;
        SequenceImageId itemId = 0;
        SegmentId curSegmentId = segTrackId.first;

        // Go backwards segments till we find an item id, or we run out of segments
        while (getPrecedingSegment(curSegmentId, precedingSegmentId) && itemId == 0)
        {
            if (mFileProperties.segmentPropertiesMap.at(precedingSegmentId).trackInfos.count(trackId) != 0u)
            {
                itemId = getFollowingSequenceImageId(SegmentTrackId(precedingSegmentId, trackId));
            }
            curSegmentId = precedingSegmentId;
        }
        return itemId;
    }

    bool HeifReaderImpl::getPrecedingSegment(SegmentId curSegmentId, SegmentId& precedingSegmentId) const
    {
        const auto& segmentProperties = mFileProperties.segmentPropertiesMap.at(curSegmentId);
        auto iterator                 = segmentProperties.sequences.empty()
                                            ? mFileProperties.sequenceToSegment.end()
                                            : mFileProperties.sequenceToSegment.find(*segmentProperties.sequences.begin());

        // This can only happen when processing init segment
        if (iterator == mFileProperties.sequenceToSegment.end())
        {
            return false;
        }

        if (iterator != mFileProperties.sequenceToSegment.begin())

        {
            --iterator;

            precedingSegmentId = iterator->second;

            return true;
        }


        return false;
    }

    const InitTrackInfo& HeifReaderImpl::getInitTrackInfo(SequenceId sequenceId) const
    {
        return mFileProperties.initTrackInfos.at(sequenceId);
    }

    InitTrackInfo& HeifReaderImpl::getInitTrackInfo(SequenceId sequenceId)
    {
        return mFileProperties.initTrackInfos.at(sequenceId);
    }

    void HeifReaderImpl::addSegmentSequence(SegmentId segmentId, Sequence sequence)
    {
        mFileProperties.segmentPropertiesMap[segmentId].sequences.insert(sequence);
        mFileProperties.sequenceToSegment.insert(std::make_pair(sequence, segmentId));
    }

    void HeifReaderImpl::addToTrackProperties(SegmentId segmentId,
                                              MovieFragmentBox& moofBox,
                                              const SequenceIdPresentationTimeTSMap& earliestPTSTS)
    {
        SegmentProperties& segmentProperties = mFileProperties.segmentPropertiesMap[segmentId];

        addSegmentSequence(segmentId, mNextSequence);
        mNextSequence = mNextSequence.get() + 1;

        std::uint64_t trackFragmentSampleDataOffset = 0;
        bool firstTrackFragment                     = true;

        Vector<TrackFragmentBox*> trackFragmentBoxes = moofBox.getTrackFragmentBoxes();
        for (auto& trackFragmentBox : trackFragmentBoxes)
        {
            SequenceId trackId = trackFragmentBox->getTrackFragmentHeaderBox().getTrackId();
            // item id base in case of adding multiple subsegments?!
            TrackInfoInSegment& trackInfo = segmentProperties.trackInfos[trackId];
            size_t prevSampleInfoSize     = trackInfo.samples.size();
            bool hasSamples               = prevSampleInfoSize > 0;
            if (auto* timeBox = trackFragmentBox->getTrackFragmentBaseMediaDecodeTimeBox())
            {
                trackInfo.nextPTSTS = DecodePts::PresentationTimeTS(timeBox->getBaseMediaDecodeTime());
            }
            else if (!hasSamples)
            {
                auto it = earliestPTSTS.find(trackId);
                if (it != earliestPTSTS.end())
                {
                    trackInfo.nextPTSTS = it->second;
                }
                else
                {
                    trackInfo.nextPTSTS = 0;
                }
            }
            SequenceImageId segmentItemIdBase =
                hasSamples ? trackInfo.itemIdBase : getPrecedingSequenceImageId(SegmentTrackId(segmentId, trackId));
            const InitTrackInfo& initTrackInfo = getInitTrackInfo(trackId);
            uint32_t sampleDescriptionIndex = trackFragmentBox->getTrackFragmentHeaderBox().getSampleDescriptionIndex();

            Vector<TrackRunBox*> trackRunBoxes = trackFragmentBox->getTrackRunBoxes();
            for (const auto trackRunBox : trackRunBoxes)
            {
                SequenceImageId trackrunItemIdBase =
                    !trackInfo.samples.empty() ? trackInfo.samples.back().sampleId.get() + 1 : segmentItemIdBase;
                // figure out what is the base data offset for the samples in this trun box:
                std::uint64_t baseDataOffset = 0;
                if ((trackFragmentBox->getTrackFragmentHeaderBox().getFlags() &
                     TrackFragmentHeaderBox::BaseDataOffsetPresent) != 0)
                {
                    baseDataOffset = trackFragmentBox->getTrackFragmentHeaderBox().getBaseDataOffset();
                }
                else if ((trackFragmentBox->getTrackFragmentHeaderBox().getFlags() &
                          TrackFragmentHeaderBox::DefaultBaseIsMoof) != 0)
                {
                    baseDataOffset = moofBox.getMoofFirstByteOffset();
                }
                else
                {
                    if (firstTrackFragment)
                    {
                        baseDataOffset = moofBox.getMoofFirstByteOffset();
                    }
                    else
                    {
                        // use previous trackfragment last sample end as data offset for this trackfragment
                        baseDataOffset = trackFragmentSampleDataOffset;
                    }
                }
                if ((trackRunBox->getFlags() & TrackRunBox::DataOffsetPresent) != 0)
                {
                    baseDataOffset += std::uint32_t(trackRunBox->getDataOffset());
                }

                // @note: if we need to support more than one trun for track we need to calculate last sample PTS +
                // duration as new basePTSOffset for addSamplesToTrackInfo for next trun in same track. Need to take
                // care of case where the might be several different tracks inside same segment. using
                // segmentProperties.earliestPTSTS for all truns works for now as example clip only have 1 moof, 1x traf
                // for each track with 1 trun each.
                addSamplesToTrackInfo(trackInfo, mFileProperties, initTrackInfo, baseDataOffset, sampleDescriptionIndex,
                                      segmentItemIdBase, trackrunItemIdBase, trackRunBox);
            }
            trackInfo.itemIdBase = segmentItemIdBase;
            auto& decoderCodeTypeMap =
                mFileProperties.segmentPropertiesMap.at(segmentId).trackInfos.at(trackId).decoderCodeTypeMap;
            updateDecoderCodeTypeMap(trackInfo.samples, decoderCodeTypeMap, prevSampleInfoSize);
            updateSampleToParametersSetMap(mFileProperties.segmentPropertiesMap[segmentId].sampleToParameterSetMap,
                                           trackId, trackInfo.samples, prevSampleInfoSize);

            if (!trackInfo.samples.empty())
            {
                // update sample data offset in case it is needed to read next track fragment data offsets (base offset
                // not defined)
                trackFragmentSampleDataOffset =
                    trackInfo.samples.rbegin()->dataOffset + trackInfo.samples.rbegin()->dataLength;
            }
            firstTrackFragment = false;
        }
    }


    void HeifReaderImpl::updateSampleToParametersSetMap(SampleToParameterSetMap& sampleToParameterSetMap,
                                                        const SequenceId sequenceId,
                                                        const SamplePropertyVector& sampleInfo,
                                                        const std::size_t prevSampleInfoSize)
    {
        for (std::size_t sampleIndex = prevSampleInfoSize; sampleIndex < sampleInfo.size(); ++sampleIndex)
        {
            auto itemId            = SequenceImageIdPair(sequenceId, sampleIndex);
            auto parameterSetMapId = sampleInfo[sampleIndex].sampleDescriptionIndex;
            sampleToParameterSetMap.insert(std::make_pair(itemId, parameterSetMapId));
        }
    }

    void HeifReaderImpl::addSamplesToTrackInfo(TrackInfoInSegment& trackInfo,
                                               const FileInformationInternal& fileInformation,
                                               const InitTrackInfo& initTrackInfo,
                                               const std::uint64_t baseDataOffset,
                                               const uint32_t sampleDescriptionIndex,
                                               const SequenceImageId itemIdBase,
                                               const SequenceImageId trackrunItemIdBase,
                                               const TrackRunBox* trackRunBox)
    {
        const Vector<TrackRunBox::SampleDetails>& samples = trackRunBox->getSampleDetails();
        const auto sampleCount                            = static_cast<uint32_t>(samples.size());
        const DecodePts::SampleIndex itemIdOffset         = trackrunItemIdBase.get() - itemIdBase.get();

        // figure out PTS
        DecodePts decodePts;
        if (initTrackInfo.editBox)
        {
            trackInfo.hasEditList = true;
            decodePts.loadBox(initTrackInfo.editBox->getEditListBox(), fileInformation.moovProperties.movieTimescale,
                              initTrackInfo.timeScale);
        }
        decodePts.loadBox(trackRunBox);
        decodePts.unravelTrackRun();
        DecodePts::PMap localPMap;
        DecodePts::PMapTS localPMapTS;
        decodePts.applyLocalTime(static_cast<std::uint64_t>(trackInfo.nextPTSTS));
        decodePts.getTimeTrackRun(initTrackInfo.timeScale, localPMap);
        decodePts.getTimeTrackRunTS(localPMapTS);
        for (const auto& mapping : localPMap)
        {
            trackInfo.pMap.insert(std::make_pair(mapping.first, mapping.second + itemIdOffset));
        }
        for (const auto& mapping : localPMapTS)
        {
            trackInfo.pMapTS.insert(std::make_pair(mapping.first, mapping.second + itemIdOffset));
        }

        std::int64_t durationTS        = 0;
        std::uint64_t sampleDataOffset = baseDataOffset;
        for (std::uint32_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        {
            SampleProperties sampleProperties{};
            sampleProperties.sampleId               = trackrunItemIdBase.get() + sampleIndex;
            sampleProperties.sampleEntryType        = initTrackInfo.sampleEntryType;
            sampleProperties.sampleDescriptionIndex = sampleDescriptionIndex;
            // sampleInfo.compositionTimes is filled at end of segment parsing (in ::parseSegment)
            sampleProperties.dataOffset = sampleDataOffset;
            sampleProperties.dataLength = samples.at(sampleIndex).version0.sampleSize;
            sampleDataOffset += sampleProperties.dataLength;
            if (initTrackInfo.sampleSizeInPixels.count(sampleDescriptionIndex) != 0u)
            {
                sampleProperties.width  = initTrackInfo.sampleSizeInPixels.at(sampleDescriptionIndex).width;
                sampleProperties.height = initTrackInfo.sampleSizeInPixels.at(sampleDescriptionIndex).height;
            }
            else
            {
                sampleProperties.width  = 0;
                sampleProperties.height = 0;
            }
            sampleProperties.sampleDurationTS = samples.at(sampleIndex).version0.sampleDuration;
            sampleProperties.sampleType =
                samples.at(sampleIndex).version0.sampleFlags.flags.sample_is_non_sync_sample == 0
                    ? OUTPUT_REFERENCE_FRAME
                    : OUTPUT_NON_REFERENCE_FRAME;
            sampleProperties.sampleFlags.flagsAsUInt = samples.at(sampleIndex).version0.sampleFlags.flagsAsUInt;
            durationTS += samples.at(sampleIndex).version0.sampleDuration;
            trackInfo.samples.push_back(sampleProperties);
        }
        trackInfo.durationTS += durationTS;
        trackInfo.nextPTSTS += durationTS;
    }


    MoovProperties HeifReaderImpl::extractMoovProperties(const MovieBox& moovBox)
    {
        MoovProperties moovProperties;
        moovProperties.fragmentDuration = 0;

        if (moovBox.isMovieExtendsBoxPresent())
        {
            const MovieExtendsBox* mvexBox = moovBox.getMovieExtendsBox();
            if (mvexBox->isMovieExtendsHeaderBoxPresent())
            {
                const MovieExtendsHeaderBox& mehdBox = mvexBox->getMovieExtendsHeaderBox();
                moovProperties.fragmentDuration      = mehdBox.getFragmentDuration();
            }

            moovProperties.fragmentSampleDefaults.clear();
            for (const auto& track : mvexBox->getTrackExtendsBoxes())
            {
                moovProperties.fragmentSampleDefaults.push_back(track->getFragmentSampleDefaults());
            }
        }

        return moovProperties;
    }

    /* *********************************************************************** */
    /* ************************* Segment parsing methods ********************* */
    /* *********************************************************************** */

    ErrorCode HeifReaderImpl::parseInitializationSegment(StreamInterface* streamInterface)
    {
        SegmentId segmentId = 0;  // all "segment" info for initialization segment goes to key=0 of SegmentPropertiesMap

        State prevState = mState;
        mState          = State::INITIALIZING;

        SegmentProperties& segmentProperties = mFileProperties.segmentPropertiesMap[segmentId];
        StreamIO& io                         = segmentProperties.io;
        io.stream.reset(CUSTOM_NEW(InternalStream, (streamInterface)));
        if (io.stream->peekEof())
        {
            mState = prevState;
            io.stream.reset();
            return ErrorCode::FILE_READ_ERROR;
        }
        io.size = streamInterface->size();

        segmentProperties.segmentId = segmentId;

        bool ftypFound       = false;
        bool metaFound       = false;
        bool moovFound       = false;
        bool earliestPTSRead = false;

        ErrorCode error = ErrorCode::OK;
        if (io.stream->peekEof())
        {
            error = ErrorCode::FILE_HEADER_ERROR;
        }

        try
        {
            while ((error == ErrorCode::OK) && !io.stream->peekEof())
            {
                String boxType;
                std::int64_t boxSize = 0;
                BitStream bitstream;
                error = readBoxParameters(io, boxType, boxSize);
                if (error == ErrorCode::OK)
                {
                    if (boxType == "ftyp")
                    {
                        if (ftypFound)
                        {
                            return ErrorCode::FILE_READ_ERROR;  // Multiple ftyp boxes.
                        }
                        ftypFound = true;
                        error     = handleFtyp(io);
                    }
                    else if (boxType == "meta")
                    {
                        if (metaFound)
                        {
                            return ErrorCode::FILE_READ_ERROR;  // Multiple root-level meta boxes.
                        }
                        metaFound = true;
                        error     = handleMeta(io);
                    }
                    else if (boxType == "sidx")
                    {
                        error = readBox(io, bitstream);
                        if (error == ErrorCode::OK)
                        {
                            SegmentIndexBox sidx;
                            sidx.parseBox(bitstream);

                            if (!earliestPTSRead)
                            {
                                earliestPTSRead = true;
                            }
                            makeSegmentIndex(sidx, mFileProperties.segmentIndex, io.stream->tell());
                        }
                    }
                    else if (boxType == "moov")
                    {
                        if (moovFound)
                        {
                            error = ErrorCode::FILE_READ_ERROR;
                            break;
                        }
                        moovFound = true;
                        error     = handleMoov(io);
                    }
                    else if (boxType == "moof")
                    {
                        logWarning() << "Skipping root level 'moof' box - not allowed in Initialization Segment"
                                     << std::endl;
                        error = skipBox(io);
                    }
                    else if (boxType == "mdat")
                    {
                        // skip mdat as its handled elsewhere
                        error = skipBox(io);
                    }
                    else
                    {
                        logWarning() << "Skipping root level box of unknown type '" << boxType << "'" << std::endl;
                        error = skipBox(io);
                    }
                }
            }
        }
        catch (ISOBMFF::Exception& exc)
        {
            logError() << "parseInitializationSegment Exception Error: " << exc.what() << std::endl;
            error = ErrorCode::FILE_READ_ERROR;
        }
        catch (std::exception& e)
        {
            logError() << "parseInitializationSegment std::exception Error:: " << e.what() << std::endl;
            error = ErrorCode::FILE_READ_ERROR;
        }

        if (error == ErrorCode::OK && (!ftypFound || !moovFound))
        {
            error = ErrorCode::FILE_HEADER_ERROR;
        }

        if (error == ErrorCode::OK)
        {
            for (auto& trackInfo : segmentProperties.trackInfos)
            {
                setupSegmentSidxFallback(std::make_pair(segmentId, trackInfo.first));
            }

            updateCompositionTimes(segmentId);

            // peek() sets eof bit for the stream. Clear stream to make sure it is still accessible. seekg() in C++11
            // should clear stream after eof, but this does not seem to be always happening.
            if ((!io.stream->good()) && (!io.stream->eof()))
            {
                return ErrorCode::FILE_READ_ERROR;
            }
            io.stream->clear();

            mFileProperties.fileFeature = getFileFeatures();
            mFileInformation            = makeFileInformation(mFileProperties);

            mState = State::READY;
        }
        else
        {
            /// @todo reset reader?
        }
        return error;
    }

    void HeifReaderImpl::setupSegmentSidxFallback(SegmentTrackId segTrackId)
    {
        SegmentId segmentId     = segTrackId.first;
        SequenceId trackId      = segTrackId.second;
        auto& segmentProperties = mFileProperties.segmentPropertiesMap.at(segmentId);

        DecodePts::PresentationTimeTS segmentDurationTS = 0;
        for (auto& trackInfo : segmentProperties.trackInfos)
        {
            segmentDurationTS = std::max(segmentDurationTS, trackInfo.second.durationTS);
        }

        DecodePts::PresentationTimeTS curSegmentStartTS = 0;
        if (const TrackInfoInSegment* trackInfo = getPrecedingTrackInfo(segTrackId))
        {
            curSegmentStartTS = trackInfo->noSidxFallbackPTSTS;
        }
        else
        {
            curSegmentStartTS = 0;
        }
        DecodePts::PresentationTimeTS afterSampleDurationPTSTS = curSegmentStartTS + segmentDurationTS;

        if (afterSampleDurationPTSTS > segmentProperties.trackInfos.at(trackId).noSidxFallbackPTSTS)
        {
            segmentProperties.trackInfos.at(trackId).noSidxFallbackPTSTS = afterSampleDurationPTSTS;
        }
    }

    void HeifReaderImpl::updateCompositionTimes(SegmentId segmentId)
    {
        // store information about overall segment:
        for (auto& trackTrackInfo : mFileProperties.segmentPropertiesMap.at(segmentId).trackInfos)
        {
            auto& trackInfo = trackTrackInfo.second;
            if (trackInfo.pMap.size() != 0u)
            {
                // Set composition times from Pmap, which considers also edit lists
                for (const auto& pair : trackInfo.pMap)
                {
                    if (pair.first < 0)  // negative time implies a hidden sample
                    {
                        continue;
                    }
                    trackInfo.samples.at(pair.second).compositionTimes.push_back(pair.first);
                }
                for (const auto& pair : trackInfo.pMapTS)
                {
                    if (pair.first < 0)  // negative time implies a hidden sample
                    {
                        continue;
                    }
                    trackInfo.samples.at(pair.second).compositionTimesTS.push_back(std::uint64_t(pair.first));
                }
            }
        }
    }


    const TrackInfoInSegment* HeifReaderImpl::getPrecedingTrackInfo(SegmentTrackId segTrackId) const
    {
        SegmentId curSegmentId = segTrackId.first;
        SequenceId trackId     = segTrackId.second;
        SegmentId precedingSegmentId;
        const TrackInfoInSegment* trackInfo = nullptr;
        while ((trackInfo == nullptr) && getPrecedingSegment(curSegmentId, precedingSegmentId))
        {
            const auto& segmentProperties = mFileProperties.segmentPropertiesMap.at(precedingSegmentId);
            if (segmentProperties.trackInfos.count(trackId) != 0u)
            {
                trackInfo = &segmentProperties.trackInfos.at(trackId);
            }
            curSegmentId = precedingSegmentId;
        }
        return trackInfo;
    }

    void HeifReaderImpl::makeSegmentIndex(const SegmentIndexBox& sidxBox,
                                          SegmentIndex& segmentIndex,
                                          const int64_t dataOffsetAnchor)
    {
        uint64_t offset             = static_cast<uint64_t>(dataOffsetAnchor) + sidxBox.getFirstOffset();
        uint64_t cumulativeDuration = sidxBox.getEarliestPresentationTime();

        Vector<SegmentIndexBox::Reference> references = sidxBox.getReferences();
        segmentIndex                                  = Array<SegmentInformation>(references.size());
        for (uint32_t index = 0; index < references.size(); index++)
        {
            segmentIndex[index].segmentId = index + 1;  // leave 0 index for possible initseg 0 index with local files.
            segmentIndex[index].referenceId     = sidxBox.getReferenceId();
            segmentIndex[index].timescale       = sidxBox.getTimescale();
            segmentIndex[index].referenceType   = references.at(index).referenceType;
            segmentIndex[index].earliestPTSinTS = cumulativeDuration;
            cumulativeDuration += references.at(index).subsegmentDuration;
            segmentIndex[index].durationInTS    = references.at(index).subsegmentDuration;
            segmentIndex[index].startDataOffset = offset;
            segmentIndex[index].dataSize        = references.at(index).referencedSize;
            offset += segmentIndex[index].dataSize;
            segmentIndex[index].startsWithSAP = references.at(index).startsWithSAP;
            segmentIndex[index].SAPType       = references.at(index).sapType;
        }
    }

    const ParameterSetMap* HeifReaderImpl::getParameterSetMap(const SequenceId sequenceId,
                                                              const SequenceImageId sampleId) const
    {
        SegmentId segmentId;
        ErrorCode result = segmentIdOf(sequenceId, sampleId, segmentId);
        if (result != ErrorCode::OK)
        {
            return nullptr;
        }
        const auto& segmentProperties  = mFileProperties.segmentPropertiesMap.at(segmentId);
        const unsigned int sampleIndex = sampleId.get() - getTrackInfo({segmentId, sequenceId}).itemIdBase.get();
        const auto& itemIndexIterator  = segmentProperties.sampleToParameterSetMap.find({sequenceId, sampleIndex});
        if (itemIndexIterator != segmentProperties.sampleToParameterSetMap.end())
        {
            if (mFileProperties.initTrackInfos.at(sequenceId).parameterSetMaps.count(itemIndexIterator->second))
            {
                return &mFileProperties.initTrackInfos.at(sequenceId).parameterSetMaps.at(itemIndexIterator->second);
            }
        }

        return nullptr;
    }


    /* *********************************************************************** */
    /* ************************* Helper functions **************************** */
    /* *********************************************************************** */

    ErrorCode getRawItemType(const MetaBox& metaBox, const ImageId itemId, FourCCInt& type)
    {
        try
        {
            type = metaBox.getItemInfoBox().getItemById(itemId.get()).getItemType();
        }
        catch (...)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }

        return ErrorCode::OK;
    }

    CleanAperture makeClap(const CleanApertureBox* clapBox)
    {
        assert(clapBox);

        CleanAperture clap{};
        clap.widthN            = clapBox->getWidth().numerator;
        clap.widthD            = clapBox->getWidth().denominator;
        clap.heightN           = clapBox->getHeight().numerator;
        clap.heightD           = clapBox->getHeight().denominator;
        clap.horizontalOffsetN = clapBox->getHorizOffset().numerator;
        clap.horizontalOffsetD = clapBox->getHorizOffset().denominator;
        clap.verticalOffsetN   = clapBox->getVertOffset().numerator;
        clap.verticalOffsetD   = clapBox->getVertOffset().denominator;

        return clap;
    }

    AuxiliaryType makeAuxi(const AuxiliaryTypeInfoBox* auxiBox)
    {
        assert(auxiBox);
        String auxTypeStr = auxiBox->getAuxType();

        AuxiliaryType auxi;
        auxi.auxType = Array<char>(auxTypeStr.size());
        for (String::const_iterator it = auxTypeStr.begin(); it != auxTypeStr.end(); ++it)
        {
            auxi.auxType.elements[it - auxTypeStr.begin()] = char(*it);
        }
        auxi.subType = {};
        return auxi;
    }

    bool HeifReaderImpl::isImageItem(const ItemInfo& info)
    {
        static const std::set<FourCCInt> IMAGE_TYPES = {"avc1", "hvc1", "grid", "iovl", "iden", "jpeg"};

        auto type = info.type;

        return (IMAGE_TYPES.count(type) != 0u) || (type == "mime" && info.contentType == "image/jpeg");
    }

    HeifReaderImpl::ItemInfo HeifReaderImpl::makeItemInfo(const ItemInfoEntry& item)
    {
        ItemInfo itemInfo{};
        itemInfo.type            = item.getItemType();
        itemInfo.name            = item.getItemName();
        itemInfo.contentType     = item.getContentType();
        itemInfo.contentEncoding = item.getContentEncoding();
        return itemInfo;
    }

    TrackGroupInfoMap HeifReaderImpl::getTrackGroupInfoMap(const TrackBox* trackBox)
    {
        TrackGroupInfoMap trackGroupInfoMap;

        if (trackBox->getHasTrackGroup())
        {
            const auto& trackGroupTypeBoxes = trackBox->getTrackGroupBox().getTrackGroupTypeBoxes();
            for (auto trackGroupTypeBox : trackGroupTypeBoxes)
            {
                Vector<SequenceId> trackGroupId;
                const TrackGroupTypeBox& box = trackGroupTypeBox;
                trackGroupId.push_back(box.getTrackGroupId());
                TrackGroupInfo trackGroupInfo{};
                trackGroupInfo.ids               = trackGroupId;
                trackGroupInfoMap[box.getType()] = trackGroupInfo;
            }
        }

        return trackGroupInfoMap;
    }


    bool doReferencesFromItemIdExist(const MetaBox& metaBox, const ImageId itemId, const FourCCInt& referenceType)
    {
        const Vector<SingleItemTypeReferenceBox>& references =
            metaBox.getItemReferenceBox().getReferencesOfType(FourCCInt(referenceType));
        for (const auto& singleItemTypeReferenceBox : references)
        {
            if (singleItemTypeReferenceBox.getFromItemID() == itemId)
            {
                return true;
            }
        }
        return false;
    }

    bool doReferencesToItemIdExist(const MetaBox& metaBox, const ImageId itemId, const FourCCInt& referenceType)
    {
        const Vector<SingleItemTypeReferenceBox>& references =
            metaBox.getItemReferenceBox().getReferencesOfType(FourCCInt(referenceType));
        for (const auto& singleItemTypeReferenceBox : references)
        {
            const Vector<uint32_t>& toIds = singleItemTypeReferenceBox.getToItemIds();
            const auto id                 = find(toIds.cbegin(), toIds.cend(), itemId);
            if (id != toIds.cend())
            {
                return true;
            }
        }
        return false;
    }

    template <typename T, typename Container>
    Array<T> makeArray(const Container& container)
    {
        Array<T> array(container.size());
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            array.elements[it - container.begin()] = T(*it);
        }
        return array;
    }

    template Array<ImageId> makeArray(const Vector<ImageId>& container);
    template Array<ImageId> makeArray(const Vector<uint32_t>& container);
    template Array<ItemPropertyInfo> makeArray(const Vector<ItemPropertyInfo>& container);
    template Array<SampleGrouping> makeArray(const Vector<SampleGrouping>& container);
    template Array<SequenceId> makeArray(const Vector<SequenceId>& container);
    template Array<SequenceImageId> makeArray(const Vector<SequenceImageId>& container);
    template Array<TimestampIDPair> makeArray(const Vector<TimestampIDPair>& container);
    template Array<char> makeArray(const String& container);
    template Array<std::int32_t> makeArray(const Vector<std::int32_t>& container);
    template Array<std::uint32_t> makeArray(const Vector<std::uint32_t>& container);
    template Array<std::int64_t> makeArray(const Vector<std::int64_t>& container);
    template Array<std::uint64_t> makeArray(const Vector<std::uint64_t>& container);
    template Array<std::uint8_t> makeArray(const Vector<std::uint8_t>& container);
    template Array<EditUnit> makeArray(const Vector<EditUnit>& container);
    template Array<FourCC> makeArray(const Vector<FourCC>& container);

}  // namespace HEIF
