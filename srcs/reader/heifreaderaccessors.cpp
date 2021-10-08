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

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>

#include "accessibilitytext.hpp"
#include "auxiliarytypeproperty.hpp"
#include "avcconfigurationbox.hpp"
#include "avcdecoderconfigrecord.hpp"
#include "avcsampleentry.hpp"
#include "buildinfo.hpp"
#include "cleanaperturebox.hpp"
#include "codingconstraintsbox.hpp"
#include "colourinformationbox.hpp"
#include "creationtimeinformation.hpp"
#include "heiffiledatatypesinternal.hpp"
#include "heifreaderimpl.hpp"
#include "hevccommondefs.hpp"
#include "hevcconfigurationbox.hpp"
#include "hevcdecoderconfigrecord.hpp"
#include "hevcsampleentry.hpp"
#include "imagegrid.hpp"
#include "imagemirror.hpp"
#include "imageoverlay.hpp"
#include "imagerelativelocationproperty.hpp"
#include "imagerotation.hpp"
#include "imagescaling.hpp"
#include "log.hpp"
#include "mediadatabox.hpp"
#include "metabox.hpp"
#include "modificationtimeinformation.hpp"
#include "moviebox.hpp"
#include "pixelaspectratiobox.hpp"
#include "pixelinformationproperty.hpp"
#include "rawpropertybox.hpp"
#include "requiredreferencetypesproperty.hpp"
#include "userdescriptionproperty.hpp"

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

    ErrorCode HeifReaderImpl::getFileInformation(FileInformation& fileInfo) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        fileInfo = mFileInformation;

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getMajorBrand(FourCC& majorBrand) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        majorBrand = FourCC(mFtyp.getMajorBrand().getUInt32());
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getMinorVersion(uint32_t& minorVersion) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        minorVersion = mFtyp.getMinorVersion();
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getCompatibleBrands(Array<FourCC>& compatibleBrands) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        const auto& brands = mFtyp.getCompatibleBrands();

        compatibleBrands = Array<FourCC>(brands.size());

        for (uint32_t i = 0; i < brands.size(); ++i)
        {
            compatibleBrands[i] = FourCC(brands[i].getUInt32());
        }

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getCompatibleBrandCombinations(Array<Array<FourCC>>& compatibleBrandCombinations) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        const auto& tycos           = mEtyp.getTypeCombinationBoxes();
        compatibleBrandCombinations = Array<Array<FourCC>>(tycos.size());
        unsigned int i              = 0;
        for (const auto& tyco : tycos)
        {
            const auto& brandsFourCCint    = tyco.getCompatibleBrands();
            compatibleBrandCombinations[i] = Array<FourCC>(brandsFourCCint.size());
            unsigned int j                 = 0;
            for (const auto fourCCIntBrand : brandsFourCCint)
            {
                compatibleBrandCombinations[i][j] = FourCC(fourCCIntBrand.getUInt32());
                ++j;
            }
            ++i;
        }

        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getDisplayWidth(const SequenceId& sequenceId, uint32_t& displayWidth) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }
        displayWidth = getInitTrackInfo(sequenceId).width;
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getDisplayHeight(const SequenceId& sequenceId, uint32_t& displayHeight) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }
        displayHeight = getInitTrackInfo(sequenceId).height;
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getWidth(const ImageId& itemId, uint32_t& width) const
    {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }
        width = mMetaBoxInfo.itemInfoMap.at(itemId.get()).width;
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getWidth(const SequenceId& sequenceId,
                                       const SequenceImageId& itemId,
                                       uint32_t& width) const
    {
        ErrorCode error;
        SampleProperties sampleInfo;
        if ((error = getSampleInfo(sequenceId, itemId, sampleInfo)) != ErrorCode::OK)
        {
            return error;
        }
        width = sampleInfo.width;

        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getHeight(const ImageId& itemId, uint32_t& height) const
    {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }
        height = mMetaBoxInfo.itemInfoMap.at(itemId.get()).height;
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getHeight(const SequenceId& sequenceId,
                                        const SequenceImageId& itemId,
                                        uint32_t& height) const
    {
        ErrorCode error;
        SampleProperties sampleInfo;
        if ((error = getSampleInfo(sequenceId, itemId, sampleInfo)) != ErrorCode::OK)
        {
            return error;
        }
        height = sampleInfo.height;

        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getMatrix(Array<std::int32_t>& matrix) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (mFileProperties.moovProperties.mMatrix.empty())
        {
            return ErrorCode::NOT_APPLICABLE;
        }

        matrix = makeArray<int32_t>(mFileProperties.moovProperties.mMatrix);
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getMatrix(const SequenceId& sequenceId, Array<int32_t>& matrix) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }
        matrix = makeArray<int32_t>(mFileProperties.initTrackInfos.at(sequenceId.get()).matrix);
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getPlaybackDurationInSecs(const SequenceId& sequenceId, double& durationInSecs) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }

        std::int64_t maxTimeUs  = 0;
        std::uint32_t timescale = mFileProperties.initTrackInfos.at(sequenceId).timeScale;
        for (const auto& segment : mFileProperties.segmentPropertiesMap)
        {
            const auto& trackInfo = segment.second.trackInfos.find(sequenceId);
            if (trackInfo != segment.second.trackInfos.end())
            {
                for (const auto& sample : trackInfo->second.samples)
                {
                    for (const auto& compositionTimesTS : sample.compositionTimesTS)
                    {
                        maxTimeUs = std::max(
                            maxTimeUs, int64_t((compositionTimesTS + sample.sampleDurationTS) * 1000000 / timescale));
                    }
                }
            }
        }
        durationInSecs = maxTimeUs / 1000000.0;

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getMasterImages(Array<ImageId>& itemIds) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        Vector<ImageId> allItems;
        getCollectionItems(allItems);

        Vector<ImageId> masterItemIds;
        for (auto itemId : allItems)
        {
            const auto rawType = mMetaBox.getItemInfoBox().getItemById(itemId.get()).getItemType().getString();
            if ((rawType == "avc1") || (rawType == "hvc1"))
            {
                // A master image is an image that is not an auxiliary image or a thumbnail image.
                if (!doReferencesFromItemIdExist(mMetaBox, itemId.get(), "auxl") &&
                    !doReferencesFromItemIdExist(mMetaBox, itemId.get(), "thmb"))
                {
                    masterItemIds.push_back(itemId);
                }
            }
        }
        itemIds = makeArray<ImageId>(masterItemIds);

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getMasterImages(const SequenceId& sequenceId, Array<SequenceImageId>& itemIds) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }

        Vector<SequenceImageId> allImages;
        getSamples(sequenceId, allImages);
        if (mFileProperties.initTrackInfos.at(sequenceId.get())
                .trackFeature.hasFeature(TrackFeatureEnum::IsMasterImageSequence))
        {
            itemIds = makeArray<SequenceImageId>(allImages);
        }

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getItemListByType(const FourCC& itemType, Array<ImageId>& itemIds) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        Vector<ImageId> itemIdVector;
        Vector<ImageId> allItems;
        getCollectionItems(allItems);
        for (auto itemId : allItems)
        {
            FourCC thisType;
            getItemType(itemId, thisType);
            if (thisType == itemType)
            {
                itemIdVector.push_back(itemId);
            }
        }
        itemIds = makeArray<ImageId>(itemIdVector);

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getItemListByType(const SequenceId& sequenceId,
                                                const TrackSampleType& sampleType,
                                                Array<SequenceImageId>& sampleIdsApi) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }

        Vector<SequenceImageId> allSamples;
        error = getSamples(sequenceId, allSamples);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        Vector<SequenceImageId> matches;
        bool foundSegment = false;
        for (const auto& segment : segmentsBySequence())
        {
            foundSegment              = true;
            SegmentTrackId segTrackId = std::make_pair(segment.segmentId, sequenceId);
            if (sampleType == TrackSampleType::out_ref)
            {
                if (hasTrackInfo(segTrackId))
                {
                    SequenceImageId sampleBase;
                    const SamplePropertyVector& sampleInfo = getSampleInfo(segTrackId, sampleBase);
                    for (uint32_t index = 0; index < sampleInfo.size(); ++index)
                    {
                        if (sampleInfo[index].sampleType == SampleType::OUTPUT_REFERENCE_FRAME)
                        {
                            matches.push_back(index + sampleBase.get());
                        }
                    }
                }
            }
            else if (sampleType == TrackSampleType::non_out_ref)
            {
                if (hasTrackInfo(segTrackId))
                {
                    SequenceImageId sampleBase;
                    const SamplePropertyVector& sampleInfo = getSampleInfo(segTrackId, sampleBase);
                    for (uint32_t index = 0; index < sampleInfo.size(); ++index)
                    {
                        if (sampleInfo[index].sampleType == SampleType::NON_OUTPUT_REFERENCE_FRAME)
                        {
                            matches.push_back(index + sampleBase.get());
                        }
                    }
                }
            }
            else if (sampleType == TrackSampleType::out_non_ref)
            {
                if (hasTrackInfo(segTrackId))
                {
                    SequenceImageId sampleBase;
                    const SamplePropertyVector& sampleInfo = getSampleInfo(segTrackId, sampleBase);
                    for (uint32_t index = 0; index < sampleInfo.size(); ++index)
                    {
                        if (sampleInfo[index].sampleType == SampleType::OUTPUT_NON_REFERENCE_FRAME)
                        {
                            matches.push_back(index + sampleBase.get());
                        }
                    }
                }
            }
            else if (sampleType == TrackSampleType::display)
            {
                if (hasTrackInfo(segTrackId))
                {
                    Vector<SequenceImageId> sampleIds;
                    // Collect frames to display
                    SequenceImageId sampleBase;
                    const SamplePropertyVector& sampleInfo = getSampleInfo(segTrackId, sampleBase);
                    for (uint32_t index = 0; index < sampleInfo.size(); ++index)
                    {
                        if (sampleInfo[index].sampleType == SampleType::OUTPUT_NON_REFERENCE_FRAME ||
                            sampleInfo[index].sampleType == SampleType::OUTPUT_REFERENCE_FRAME)
                        {
                            sampleIds.push_back(index + sampleBase.get());
                        }
                    }

                    // Collect every presentation time stamp of every sample
                    Vector<ItemIdTimestampPair> samplePresentationTimes;
                    for (auto sampleId : sampleIds)
                    {
                        const auto& trackInfo = getTrackInfo(segTrackId);
                        const auto singleSamplePresentationTimes =
                            trackInfo.samples.at(sampleId.get() - trackInfo.itemIdBase.get()).compositionTimes;
                        for (auto sampleTime : singleSamplePresentationTimes)
                        {
                            samplePresentationTimes.push_back(std::make_pair(sampleId, sampleTime));
                        }
                    }

                    // Sort to display order using composition times
                    std::sort(samplePresentationTimes.begin(), samplePresentationTimes.end(),
                              [&](ItemIdTimestampPair a, ItemIdTimestampPair b) { return a.second < b.second; });

                    // Push sample ids to the result
                    for (auto pair : samplePresentationTimes)
                    {
                        matches.push_back(pair.first.get());
                    }
                }
            }
            else if (sampleType == TrackSampleType::samples)
            {
                matches = allSamples;
            }
            else
            {
                return ErrorCode::INVALID_FUNCTION_PARAMETER;
            }
        }

        sampleIdsApi = makeArray<SequenceImageId>(matches);

        if (foundSegment == false)
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getItemType(const ImageId& itemId, FourCC& type) const
    {
        ErrorCode error;
        if ((error = isInitialized()) != ErrorCode::OK)
        {
            return error;
        }

        if (mMetaBoxInfo.itemInfoMap.count(itemId.get()) != 0u)
        {
            type = mMetaBoxInfo.itemInfoMap.at(itemId.get()).type.getUInt32();
            return ErrorCode::OK;
        }

        return ErrorCode::INVALID_ITEM_ID;
    }

    ErrorCode HeifReaderImpl::getItemType(const SequenceId& sequenceId,
                                          const SequenceImageId& sequenceImageId,
                                          FourCC& type) const
    {
        ErrorCode error;
        SampleProperties sampleInfo;
        if ((error = getSampleInfo(sequenceId, sequenceImageId, sampleInfo)) != ErrorCode::OK)
        {
            return error;
        }
        type = sampleInfo.sampleEntryType.getUInt32();

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getReferencedFromItemListByType(const ImageId& fromItemId,
                                                              const FourCC& referenceType,
                                                              Array<ImageId>& itemIds) const
    {
        ErrorCode error;
        if ((error = isValidImageItem(fromItemId)) != ErrorCode::OK)
        {
            return error;
        }

        const ItemReferenceBox& itemReferenceBox = mMetaBox.getItemReferenceBox();
        const Vector<SingleItemTypeReferenceBox>& references =
            itemReferenceBox.getReferencesOfType(FourCCInt(referenceType.value));

        Vector<ImageId> itemIdVector;
        for (const auto& reference : references)
        {
            if (reference.getFromItemID() == fromItemId.get())
            {
                const auto& toIds = reference.getToItemIds();
                itemIdVector.insert(itemIdVector.end(), toIds.cbegin(), toIds.cend());
            }
        }

        itemIds = makeArray<ImageId>(itemIdVector);
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getReferencedToItemListByType(const ImageId& toItemId,
                                                            const FourCC& referenceType,
                                                            Array<ImageId>& itemIds) const
    {
        ErrorCode error;
        if ((error = isValidImageItem(toItemId)) != ErrorCode::OK)
        {
            return error;
        }

        const ItemReferenceBox& itemReferenceBox = mMetaBox.getItemReferenceBox();
        const Vector<SingleItemTypeReferenceBox>& references =
            itemReferenceBox.getReferencesOfType(FourCCInt(referenceType.value));

        Vector<ImageId> itemIdVector;
        for (const auto& reference : references)
        {
            const auto& toIds = reference.getToItemIds();
            for (const auto id : toIds)
            {
                if (id == toItemId.get())
                {
                    itemIdVector.push_back(reference.getFromItemID());
                }
            }
        }

        itemIds = makeArray<ImageId>(itemIdVector);
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getPrimaryItem(ImageId& itemId) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        if (!mIsPrimaryItemSet)
        {
            return ErrorCode::PRIMARY_ITEM_NOT_SET;
        }

        itemId = mPrimaryItemId;

        return ErrorCode::OK;
    }

    /// @todo Avoid data copying.
    ErrorCode HeifReaderImpl::getItemData(const ImageId& itemId,
                                          uint8_t* memoryBuffer,
                                          uint64_t& memoryBufferSize,
                                          bool bytestreamHeaders) const
    {
        ErrorCode error;
        if ((error = isValidItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }

        std::uint64_t itemLength(0);
        try
        {
            List<ImageId> pastReferences;
            error = getItemLength(mMetaBox, itemId.get(), itemLength, pastReferences);
            if (error != ErrorCode::OK)
            {
                return error;
            }

            if (static_cast<int64_t>(itemLength) > mFileProperties.segmentPropertiesMap.at(0).io.size)
            {
                return ErrorCode::FILE_HEADER_ERROR;
            }
        }
        catch (...)
        {
            return ErrorCode::FILE_READ_ERROR;
        }

        if (memoryBufferSize < itemLength)
        {
            memoryBufferSize = static_cast<uint32_t>(itemLength);
            return ErrorCode::BUFFER_SIZE_TOO_SMALL;
        }
        memoryBufferSize = static_cast<uint32_t>(itemLength);

        // read NAL data to bitstream object
        bool processData = false;
        try
        {
            error = readItem(mMetaBox, itemId, memoryBuffer, memoryBufferSize);
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

        FourCCInt rawType;
        error = getRawItemType(mMetaBox, itemId, rawType);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        bool isProtected = false;
        error            = getProtection(itemId, isProtected);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        if (!isProtected && ((rawType == "hvc1") || (rawType == "avc1")))
        {
            processData = true;
        }

        if (processData && bytestreamHeaders)
        {
            // Process bitstream by codec
            FourCC codeType;
            error = getDecoderCodeType(itemId, codeType);
            if (error != ErrorCode::OK)
            {
                return error;
            }

            if (codeType == FourCC("avc1"))
            {
                // Get item data from AVC bitstream
                error = processAvcItemData(memoryBuffer, memoryBufferSize);
                if (error != ErrorCode::OK)
                {
                    return error;
                }
            }
            else if (codeType == FourCC("hvc1"))
            {
                // Get item data from HEVC bitstream
                error = processHevcItemData(memoryBuffer, memoryBufferSize);
                if (error != ErrorCode::OK)
                {
                    return error;
                }
            }
            else
            {
                // Code type not supported
                return ErrorCode::UNSUPPORTED_CODE_TYPE;
            }
        }
        return ErrorCode::OK;
    }

    /// @todo Avoid data copying.
    ErrorCode HeifReaderImpl::getItemData(const SequenceId& sequenceId,
                                          const SequenceImageId& itemId,
                                          uint8_t* memoryBuffer,
                                          uint64_t& memoryBufferSize,
                                          bool bytestreamHeaders)
    {
        ErrorCode error;
        if ((error = isValidSample(sequenceId, itemId)) != ErrorCode::OK)
        {
            return error;
        }

        // read NAL data to bitstream object
        error = getTrackSampleData(sequenceId, itemId, memoryBuffer, memoryBufferSize);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        // Process bitstream by codec
        FourCC codeType;
        error = getDecoderCodeType(sequenceId, itemId, codeType);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        if (bytestreamHeaders)
        {
            if ((codeType == FourCC("avc1")) || (codeType == FourCC("avc3")))
            {
                // Get item data from AVC bitstream
                error = processAvcItemData(memoryBuffer, memoryBufferSize);
                if (error != ErrorCode::OK)
                {
                    return error;
                }
            }
            else if ((codeType == FourCC("hvc1")) || (codeType == FourCC("hev1")))
            {
                // Get item data from HEVC bitstream
                error = processHevcItemData(memoryBuffer, memoryBufferSize);
                if (error != ErrorCode::OK)
                {
                    return error;
                }
            }
            else if ((codeType == "mp4a") || (codeType == "mp4v"))
            {
                // already valid data - do nothing.
                return ErrorCode::OK;
            }
            else
            {
                // Code type not supported
                return ErrorCode::UNSUPPORTED_CODE_TYPE;
            }
        }
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getTrackSampleData(const SequenceId& trackId,
                                                 const SequenceImageId& itemIdApi,
                                                 uint8_t* memoryBuffer,
                                                 uint64_t& memoryBufferSize)
    {
        ErrorCode error;
        if ((error = isInitialized()) != ErrorCode::OK)
        {
            return error;
        }

        SegmentId segmentId;
        ErrorCode result = segmentIdOf(trackId, itemIdApi, segmentId);
        if (result != ErrorCode::OK)
        {
            return result;
        }
        SegmentTrackId segTrackId = std::make_pair(segmentId, trackId);
        SequenceImageId itemId    = itemIdApi.get() - getTrackInfo(segTrackId).itemIdBase.get();

        auto& io = mFileProperties.segmentPropertiesMap.at(segmentId).io;
        // read NAL data to bitstream object

        // The requested frame should be one that is available
        if (itemId.get() >= getTrackInfo(segTrackId).samples.size())
        {
            return ErrorCode::INVALID_ITEM_ID;
        }

        const uint32_t sampleLength = getTrackInfo(segTrackId).samples.at(itemId.get()).dataLength;
        if (memoryBufferSize < sampleLength)
        {
            memoryBufferSize = sampleLength;
            return ErrorCode::MEMORY_TOO_SMALL_BUFFER;
        }

        seekInput(io, static_cast<std::int64_t>(getTrackInfo(segTrackId).samples.at(itemId.get()).dataOffset));
        io.stream->read(reinterpret_cast<char*>(memoryBuffer), sampleLength);
        memoryBufferSize = sampleLength;

        if (!io.stream->good())
        {
            return ErrorCode::FILE_READ_ERROR;
        }

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::segmentIdOf(SequenceId sequenceId, SequenceImageId itemId, SegmentId& segmentId) const
    {
        bool wentPast = false;

        if (mFileProperties.segmentPropertiesMap.empty())
        {
            return ErrorCode::INVALID_SEGMENT;
        }

        const auto& segs = segmentsBySequence();
        for (auto segmentIt = segs.begin(); !wentPast && segmentIt != segs.end(); ++segmentIt)
        {
            auto track = segmentIt->trackInfos.find(sequenceId);
            if (track != segmentIt->trackInfos.end())
            {
                if (track->second.itemIdBase > itemId)
                {
                    wentPast = true;
                }
                else
                {
                    segmentId = segmentIt->segmentId;
                }
            }
        }

        auto ret     = ErrorCode::INVALID_SEQUENCE_IMAGE_ID;
        auto segment = mFileProperties.segmentPropertiesMap.find(segmentId);
        if (segment != mFileProperties.segmentPropertiesMap.end())
        {
            auto track = segment->second.trackInfos.find(sequenceId);
            if (track != segment->second.trackInfos.end())
            {
                if (itemId.get() - track->second.itemIdBase.get() < std::uint32_t(track->second.samples.size()))
                {
                    ret = ErrorCode::OK;
                }
            }
        }
        return ret;
    }


    ErrorCode HeifReaderImpl::getItem(const ImageId& itemId, Overlay& iovlItem) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        bool isProtected = false;
        ErrorCode error  = getProtection(itemId, isProtected);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        if (isProtected)
        {
            return ErrorCode::PROTECTED_ITEM;
        }
        if (mMetaBoxInfo.iovlItems.count(itemId) == 0)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }
        iovlItem = mMetaBoxInfo.iovlItems.at(itemId);
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getItem(const ImageId& itemId, Grid& gridItem) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        bool isProtected = false;
        ErrorCode error  = getProtection(itemId, isProtected);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        if (isProtected)
        {
            return ErrorCode::PROTECTED_ITEM;
        }
        if (mMetaBoxInfo.gridItems.count(itemId) == 0)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }
        gridItem = mMetaBoxInfo.gridItems.at(itemId);
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, Scale& iscl) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "iscl")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        const auto isclPtr = static_cast<const ImageScaling*>(boxPtr);
        iscl.targetWidthD  = isclPtr->getWidth().denominator;
        iscl.targetWidthN  = isclPtr->getWidth().numerator;
        iscl.targetHeightD = isclPtr->getHeight().denominator;
        iscl.targetHeightN = isclPtr->getHeight().numerator;
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, CreationTimeInformation& crtt) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "crtt")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        const auto crttPtr = static_cast<const CreationTimeProperty*>(boxPtr);
        crtt.time          = crttPtr->getCreationTime();
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, ModificationTimeInformation& mdft) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "mdft")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        const auto mdftPtr = static_cast<const ModificationTimeProperty*>(boxPtr);
        mdft.time          = mdftPtr->getModificationTime();
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, AccessibilityText& altt) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "altt")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        const auto alttPtr = static_cast<const AccessibilityTextProperty*>(boxPtr);
        altt.lang          = makeArray<char>(alttPtr->getLang());
        altt.text          = makeArray<char>(alttPtr->getText());
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, RequiredReferenceTypes& rref) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "rref")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        auto rrefPtr    = static_cast<const RequiredReferenceTypesProperty*>(boxPtr);
        auto references = rrefPtr->getReferenceTypes();

        rref.referenceTypes = Array<FourCC>(references.size());
        for (uint64_t i = 0; i < references.size(); ++i)
        {
            rref.referenceTypes[i] = references[i].getUInt32();
        }
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, UserDescription& udes) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "udes")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        auto udesPtr = static_cast<const UserDescriptionProperty*>(boxPtr);

        udes.lang        = makeArray<char>(udesPtr->getLang());
        udes.name        = makeArray<char>(udesPtr->getName());
        udes.description = makeArray<char>(udesPtr->getDescription());
        udes.tags        = makeArray<char>(udesPtr->getTags());
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, AuxiliaryType& auxc) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "auxC")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        auto auxcPtr = static_cast<const AuxiliaryTypeProperty*>(boxPtr);

        const String& auxType = auxcPtr->getAuxType();
        auxc.auxType          = makeArray<char>(auxType);

        const auto& subType = auxcPtr->getAuxSubType();
        auxc.subType        = Array<uint8_t>(subType.size());
        for (uint64_t i = 0; i < subType.size(); ++i)
        {
            auxc.subType[i] = subType[i];
        }
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const SequenceId& sequenceId,
                                          const std::uint32_t index,
                                          AuxiliaryType& auxc) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }
        if (mFileProperties.initTrackInfos.at(sequenceId).auxiProperties.count(index) == 0)
        {
            return ErrorCode::INVALID_SAMPLE_DESCRIPTION_INDEX;
        }

        auxc.auxType = mFileProperties.initTrackInfos.at(sequenceId).auxiProperties.at(index).auxType;
        auxc.subType = {};  /// @todo Should read also SEI messages from sample entry to here.
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, Mirror& imir) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "imir")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        auto imirBox = static_cast<const ImageMirror*>(boxPtr);

        imir.horizontalAxis = imirBox->getHorizontalAxis();
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, Rotate& irot) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "irot")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        auto irotBox = static_cast<const ImageRotation*>(boxPtr);

        irot.angle = irotBox->getAngle();
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, RelativeLocation& rloc) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "rloc")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        auto rlocPtr = static_cast<const ImageRelativeLocationProperty*>(boxPtr);

        rloc.horizontalOffset = rlocPtr->getHorizontalOffset();
        rloc.verticalOffset   = rlocPtr->getVerticalOffset();
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, PixelInformation& pixi) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "pixi")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        auto pixiPtr = static_cast<const PixelInformationProperty*>(boxPtr);

        const auto& bitsPerChannels = pixiPtr->getBitsPerChannels();
        pixi.bitsPerChannel         = makeArray<uint8_t>(bitsPerChannels);
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, ColourInformation& colr) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "colr")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        auto colrPtr = static_cast<const ColourInformationBox*>(boxPtr);

        colr.colourType              = colrPtr->getColourType().getUInt32();
        colr.colourPrimaries         = colrPtr->getColourPrimaries();
        colr.transferCharacteristics = colrPtr->getTransferCharacteristics();
        colr.matrixCoefficients      = colrPtr->getMatrixCoefficients();
        colr.fullRangeFlag           = colrPtr->getFullRangeFlag();
        colr.iccProfile              = makeArray<uint8_t>(colrPtr->getIccProfile());

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, PixelAspectRatio& pasp) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "pasp")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        auto paspPtr = static_cast<const PixelAspectRatioBox*>(boxPtr);

        pasp.relativeWidth  = paspPtr->getRelativeWidth();
        pasp.relativeHeight = paspPtr->getRelativeHeight();
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, CleanAperture& clap) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "clap")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        auto clapPtr = static_cast<const CleanApertureBox*>(boxPtr);
        clap         = makeClap(clapPtr);
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, RawProperty& property) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        const auto data = mMetaBox.getItemPropertiesBox().getPropertyDataByIndex(index.get());
        if (data.empty())
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }

        property.data = makeArray<std::uint8_t>(data);
        property.type = FourCC(reinterpret_cast<const char*>(&data[4]));

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const SequenceId& sequenceId, std::uint32_t index, CleanAperture& clap) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }
        if (mFileProperties.initTrackInfos.at(sequenceId).clapProperties.count(index) == 0)
        {
            return ErrorCode::INVALID_SAMPLE_DESCRIPTION_INDEX;
        }

        clap = mFileProperties.initTrackInfos.at(sequenceId).clapProperties.at(index);

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::isValidEntityGroup(const GroupId& groupId) const
    {
        const auto& entityGroupings = mFileProperties.rootLevelMetaBoxProperties.entityGroupings;

        auto found = std::find_if(entityGroupings.cbegin(), entityGroupings.cend(),
                                  [&groupId](const EntityGrouping& x) { return x.groupId == groupId; });
        if (found == entityGroupings.cend())
        {
            return ErrorCode::INVALID_GROUP_ID;
        }

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getItemProperties(const GroupId& groupId, Array<ItemPropertyInfo>& propertyTypes) const
    {
        ErrorCode error;

        if ((error = isValidEntityGroup(groupId)) != ErrorCode::OK)
        {
            return error;
        }

        if (mMetaBoxInfo.properties.count(groupId.get()) == 0)
        {
            propertyTypes = Array<ItemPropertyInfo>();
            return ErrorCode::OK;
        }

        propertyTypes = makeArray<ItemPropertyInfo>(mMetaBoxInfo.properties.at(groupId.get()));
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getItemProperties(const ImageId& itemId, Array<ItemPropertyInfo>& propertyTypes) const
    {
        ErrorCode error;
        if ((error = isValidItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }

        if (mMetaBoxInfo.properties.count(itemId.get()) == 0)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }

        propertyTypes = makeArray<ItemPropertyInfo>(mMetaBoxInfo.properties.at(itemId.get()));
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getItemDataWithDecoderParameters(const ImageId& itemId,
                                                               uint8_t* memoryBuffer,
                                                               uint64_t& memoryBufferSize) const
    {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }

        bool isProtected = false;
        error            = getProtection(itemId, isProtected);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        if (isProtected)
        {
            return ErrorCode::PROTECTED_ITEM;
        }

        FourCC codeType;
        error = getDecoderCodeType(itemId, codeType);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        if ((codeType != FourCC("hvc1")) && (codeType != FourCC("avc1")))
        {
            // No other code types supported
            return ErrorCode::UNSUPPORTED_CODE_TYPE;
        }

        DecoderConfiguration decoderInfos;
        error = getDecoderParameterSets(itemId, decoderInfos);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        // Calculate size of parameter sets, but do not copy the data yet.
        size_t parameterSize = 0;
        for (auto& config : decoderInfos.decoderSpecificInfo)
        {
            parameterSize += config.decSpecInfoData.size;
        }

        uint64_t itemSize(0);
        if (memoryBufferSize > parameterSize)
        {
            // Copy parameter data to the beginning of the buffer.
            size_t offset = 0;
            for (const auto& config : decoderInfos.decoderSpecificInfo)
            {
                std::memcpy(memoryBuffer + offset, config.decSpecInfoData.begin(), config.decSpecInfoData.size);
                offset += config.decSpecInfoData.size;
            }
            itemSize = memoryBufferSize - static_cast<uint32_t>(parameterSize);
        }

        // Copy item data to the buffer
        error = getItemData(itemId, &memoryBuffer[parameterSize], itemSize);
        if (error != ErrorCode::OK)
        {
            if (error == ErrorCode::BUFFER_SIZE_TOO_SMALL)
            {
                memoryBufferSize = itemSize + static_cast<uint32_t>(parameterSize);
            }
            return error;
        }

        memoryBufferSize = itemSize + static_cast<uint32_t>(parameterSize);
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getItemDataWithDecoderParameters(const SequenceId& sequenceId,
                                                               const SequenceImageId& itemId,
                                                               uint8_t* memoryBuffer,
                                                               uint64_t& memoryBufferSize)
    {
        ErrorCode error;
        if ((error = isValidSample(sequenceId, itemId)) != ErrorCode::OK)
        {
            return error;
        }

        FourCC codeType;
        error = getDecoderCodeType(sequenceId, itemId, codeType);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        if ((codeType != FourCC("hvc1")) && (codeType != FourCC("hev1")) && (codeType != FourCC("avc1")) &&
            (codeType != FourCC("avc3")))
        {
            // No other code types supported
            return ErrorCode::UNSUPPORTED_CODE_TYPE;
        }

        DecoderConfiguration decoderConfiguration;
        error = getDecoderParameterSets(sequenceId, itemId, decoderConfiguration);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        // Calculate size of parameter sets, but do not copy the data yet.
        size_t parameterSize = 0;
        for (auto& config : decoderConfiguration.decoderSpecificInfo)
        {
            parameterSize += config.decSpecInfoData.size;
        }

        // Copy item data to the buffer
        uint64_t itemSize(0);
        if (memoryBufferSize > parameterSize)
        {
            // Copy parameter data to the beginning of the buffer.
            size_t offset = 0;
            for (const auto& config : decoderConfiguration.decoderSpecificInfo)
            {
                std::memcpy(memoryBuffer + offset, config.decSpecInfoData.begin(), config.decSpecInfoData.size);
                offset += config.decSpecInfoData.size;
            }
            itemSize = memoryBufferSize - static_cast<uint32_t>(parameterSize);
        }

        error = getItemData(sequenceId, itemId, &memoryBuffer[parameterSize], itemSize);
        if (error != ErrorCode::OK)
        {
            if (error == ErrorCode::BUFFER_SIZE_TOO_SMALL)
            {
                memoryBufferSize = itemSize + static_cast<uint32_t>(parameterSize);
            }
            return error;
        }
        memoryBufferSize = itemSize + static_cast<uint32_t>(parameterSize);
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getItemTimestamps(const SequenceId& sequenceId, Array<TimestampIDPair>& timestamps) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }

        WriteOnceMap<Timestamp, SequenceImageId> timestampMap;

        for (const auto& segment : mFileProperties.segmentPropertiesMap)
        {
            SegmentId segmentId       = segment.first;
            SegmentTrackId segTrackId = std::make_pair(segmentId, sequenceId);
            if (hasTrackInfo(segTrackId))
            {
                for (const auto& sampleInfo : getTrackInfo(segTrackId).samples)
                {
                    if (sampleInfo.sampleType == SampleType::NON_OUTPUT_REFERENCE_FRAME)
                    {
                        continue;
                    }
                    for (auto compositionTime : sampleInfo.compositionTimes)
                    {
                        timestampMap.insert(std::make_pair(compositionTime, sampleInfo.sampleId));
                    }
                }
            }
        }

        timestamps = Array<TimestampIDPair>(timestampMap.size());
        uint32_t i = 0;
        for (auto const& entry : timestampMap)
        {
            TimestampIDPair pair;
            pair.timeStamp           = entry.first;
            pair.itemId              = entry.second;
            timestamps.elements[i++] = pair;
        }

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getTimestampsOfItem(const SequenceId& sequenceId,
                                                  const SequenceImageId& itemIdApi,
                                                  Array<int64_t>& timestamps) const
    {
        ErrorCode error;
        if ((error = isValidSample(sequenceId, itemIdApi)) != ErrorCode::OK)
        {
            return error;
        }

        SegmentId segmentId;
        error = segmentIdOf(sequenceId, itemIdApi, segmentId);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        SequenceImageId itemId = itemIdApi.get() - getTrackInfo(SegmentTrackId(segmentId, sequenceId)).itemIdBase.get();
        SegmentTrackId segTrackId = std::make_pair(segmentId, sequenceId);

        const auto& displayTimes = getTrackInfo(segTrackId).samples.at(itemId.get()).compositionTimes;

        timestamps = makeArray<int64_t>(displayTimes);
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getItemsInDecodingOrder(const SequenceId& sequenceId,
                                                      Array<TimestampIDPair>& decodingOrder) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }

        DecodingOrderVector decodingOrderVector;

        for (const auto& segment : mFileProperties.segmentPropertiesMap)
        {
            SegmentId segmentId       = segment.first;
            SegmentTrackId segTrackId = std::make_pair(segmentId, sequenceId);

            if (hasTrackInfo(segTrackId))
            {
                const auto& samples = getTrackInfo(segTrackId).samples;
                decodingOrderVector.reserve(samples.size());
                for (const auto& sample : samples)
                {
                    for (const auto compositionTime : sample.compositionTimes)
                    {
                        decodingOrderVector.push_back({compositionTime, sample.sampleId});
                    }
                }

                // Sort using composition times
                std::sort(decodingOrderVector.begin(), decodingOrderVector.end(),
                          [&](TimestampIDPair a, TimestampIDPair b) { return a.timeStamp < b.timeStamp; });
            }
        }

        // Add possible decoding dependencies
        decodingOrderVector = addDecodingDependencies(sequenceId, decodingOrderVector);

        decodingOrder = makeArray<TimestampIDPair>(decodingOrderVector);
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getDecodeDependencies(const ImageId& imageId, Array<ImageId>& dependencies) const
    {
        return getReferencedFromItemListByType(imageId, "pred", dependencies);
    }

    ErrorCode HeifReaderImpl::getDecodeDependencies(const SequenceId& sequenceId,
                                                    const SequenceImageId& itemId,
                                                    Array<SequenceImageId>& dependencies) const
    {
        ErrorCode error;
        SampleProperties sampleInfo;
        if ((error = getSampleInfo(sequenceId, itemId, sampleInfo)) != ErrorCode::OK)
        {
            return error;
        }

        Vector<SequenceImageId> dependencyVector;
        const auto& decodeDependencies = sampleInfo.decodeDependencies;
        dependencyVector.insert(dependencyVector.begin(), decodeDependencies.cbegin(), decodeDependencies.cend());

        // For I-frames return item id itself.
        if (dependencyVector.empty())
        {
            dependencyVector.push_back(itemId);
        }

        dependencies = makeArray<SequenceImageId>(dependencyVector);
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getDecoderCodeType(const ImageId& itemId, FourCC& type) const
    {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }

        const auto iter = mImageItemCodeTypeMap.find(itemId);
        if (iter != mImageItemCodeTypeMap.end())
        {
            type = FourCC(iter->second.getUInt32());
            return ErrorCode::OK;
        }

        return ErrorCode::INVALID_ITEM_ID;
    }

    ErrorCode HeifReaderImpl::getDecoderCodeType(const SequenceId& trackId,
                                                 const SequenceImageId& sampleId,
                                                 FourCC& decoderCodeType) const
    {
        ErrorCode error;
        if ((error = isValidTrack(trackId)) != ErrorCode::OK)
        {
            return error;
        }

        SegmentId segmentId;
        ErrorCode result = segmentIdOf(trackId, sampleId, segmentId);
        if (result != ErrorCode::OK)
        {
            return result;
        }
        SegmentTrackId segTrackId      = std::make_pair(segmentId, trackId);
        const auto segmentPropertiesIt = mFileProperties.segmentPropertiesMap.find(segmentId);
        if (segmentPropertiesIt == mFileProperties.segmentPropertiesMap.end())
        {
            return ErrorCode::INVALID_SEQUENCE_IMAGE_ID;
        }
        const auto& segmentProperties = segmentPropertiesIt->second;
        const auto trackInfoIt        = segmentProperties.trackInfos.find(segTrackId.second);
        if (trackInfoIt == segmentProperties.trackInfos.end())
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }
        const TrackInfoInSegment& trackInfo = trackInfoIt->second;
        const auto& decoderCodeTypeMap      = trackInfo.decoderCodeTypeMap;

        auto iter = decoderCodeTypeMap.find(sampleId);
        if (iter != decoderCodeTypeMap.end())
        {
            decoderCodeType = iter->second.getUInt32();
            return ErrorCode::OK;
        }

        return ErrorCode::INVALID_SEQUENCE_IMAGE_ID;
    }

    ErrorCode HeifReaderImpl::getDecoderParameterSets(const ImageId& itemId, DecoderConfiguration& decoderInfos) const
    {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }

        if (mImageToParameterSetMap.count(itemId) == 0)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }
        DecoderConfigId parameterSetId = mImageToParameterSetMap.at(itemId);

        const auto iter = mImageItemParameterSetMap.find(parameterSetId);
        if (iter == mImageItemParameterSetMap.cend())
        {
            return ErrorCode::FILE_HEADER_ERROR;
        }

        const auto& parameterSetMap      = iter->second;
        decoderInfos.decoderConfigId     = parameterSetId;
        decoderInfos.decoderSpecificInfo = Array<DecoderSpecificInfo>(parameterSetMap.size());

        int i = 0;
        for (auto const& entry : parameterSetMap)
        {
            DecoderSpecificInfo decSpecInfo;
            decSpecInfo.decSpecInfoType                    = entry.first;
            decSpecInfo.decSpecInfoData                    = makeArray<unsigned char>(entry.second);
            decoderInfos.decoderSpecificInfo.elements[i++] = decSpecInfo;
        }

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getDecoderParameterSets(const SequenceId& sequenceId,
                                                      const SequenceImageId& itemId,
                                                      DecoderConfiguration& decoderInfos) const
    {
        ErrorCode error;
        if ((error = isValidSample(sequenceId, itemId)) != ErrorCode::OK)
        {
            return error;
        }

        if (auto* parameterSetMap = getParameterSetMap(sequenceId, itemId))
        {
            decoderInfos.decoderConfigId     = 0;  /// @todo check this
            decoderInfos.decoderSpecificInfo = Array<DecoderSpecificInfo>(parameterSetMap->size());

            unsigned int i = 0;
            for (auto const& entry : *parameterSetMap)
            {
                DecoderSpecificInfo decSpecInfo;
                decSpecInfo.decSpecInfoType                    = entry.first;
                decSpecInfo.decSpecInfoData                    = makeArray<unsigned char>(entry.second);
                decoderInfos.decoderSpecificInfo.elements[i++] = decSpecInfo;
            }
            return ErrorCode::OK;
        }
        return ErrorCode::INVALID_SEQUENCE_IMAGE_ID;
    }

    ErrorCode HeifReaderImpl::getItemProtectionScheme(const ImageId& itemId,
                                                      uint8_t* memoryBuffer,
                                                      uint64_t& memoryBufferSize) const
    {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }

        bool isProtected = false;
        error            = getProtection(itemId.get(), isProtected);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        if (!isProtected)
        {
            return ErrorCode::UNPROTECTED_ITEM;
        }

        const auto& meta     = mMetaBox;
        const uint16_t index = static_cast<uint16_t>(
            meta.getItemInfoBox().getItemById(itemId.get()).getItemProtectionIndex() - 1);  // convert index to 0-based

        Vector<std::uint8_t> sinf = meta.getProtectionSchemeInfoBox(index).getData();

        if (sinf.size() > memoryBufferSize)
        {
            memoryBufferSize = static_cast<uint32_t>(sinf.size());
            return ErrorCode::BUFFER_SIZE_TOO_SMALL;
        }
        std::memcpy(memoryBuffer, sinf.data(), sinf.size());
        memoryBufferSize = static_cast<uint32_t>(sinf.size());

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getTrackInformations(Array<TrackInformation>& trackInfos) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        const size_t totalSize   = mFileProperties.initTrackInfos.size();
        trackInfos               = Array<TrackInformation>(totalSize);
        uint32_t outTrackIdxBase = 0;
        size_t i                 = 0;

        uint32_t outTrackIdx = outTrackIdxBase;

        for (auto const& trackPropsKv : mFileProperties.initTrackInfos)
        {
            SequenceId trackId                                = trackPropsKv.first;
            const InitTrackInfo& initTrackInfo                = trackPropsKv.second;
            trackInfos.elements[outTrackIdx].trackId          = trackId;
            trackInfos.elements[outTrackIdx].alternateGroupId = initTrackInfo.alternateGroupId;
            trackInfos.elements[outTrackIdx].features         = initTrackInfo.trackFeature.getFeatureMask();
            trackInfos.elements[outTrackIdx].timeScale        = mFileProperties.initTrackInfos.at(trackId).timeScale;

            trackInfos.elements[outTrackIdx].alternateTrackIds = makeArray<SequenceId>(initTrackInfo.alternateTrackIds);

            trackInfos.elements[outTrackIdx].referenceTrackIds = mapToArray(initTrackInfo.referenceTrackIds);

            trackInfos.elements[outTrackIdx].trackGroupIds = Array<FourCCToIds>(initTrackInfo.trackGroupInfoMap.size());
            i                                              = 0;
            for (auto const& group : initTrackInfo.trackGroupInfoMap)
            {
                trackInfos.elements[outTrackIdx].trackGroupIds[i].type     = FourCC(group.first.getUInt32());
                trackInfos.elements[outTrackIdx].trackGroupIds[i].trackIds = makeArray<SequenceId>(group.second.ids);
                i++;
            }
            outTrackIdx++;
        }

        // rewind and loop again, but this time we are interested only in samples

        Map<uint32_t, size_t> trackSampleCounts;  // sum of the number of samples for each outTrackIdx

        Vector<SequenceId> initSegTrackIds;
        {
            uint32_t count = 0;
            for (auto const& initTrackInfosKv : mFileProperties.initTrackInfos)
            {
                initSegTrackIds.push_back(initTrackInfosKv.first);
                trackSampleCounts[count + outTrackIdxBase] = 0u;
                ++count;
            }
        }

        for (auto const& allSegmentProperties : mFileProperties.segmentPropertiesMap)
        {
            const auto& segTrackInfos = allSegmentProperties.second.trackInfos;
            outTrackIdx               = outTrackIdxBase;
            for (SequenceId trackId : initSegTrackIds)
            {
                auto trackInfo = segTrackInfos.find(trackId);
                if (trackInfo != segTrackInfos.end())
                {
                    trackSampleCounts[outTrackIdx] += trackInfo->second.samples.size();
                }
                ++outTrackIdx;
            }
        }

        for (auto const& trackSampleCount : trackSampleCounts)
        {
            auto counttrackid                                  = trackSampleCount.first;
            auto count                                         = trackSampleCount.second;
            trackInfos.elements[counttrackid].sampleProperties = Array<SampleInformation>(count);
            trackInfos.elements[counttrackid].maxSampleSize    = 0;
        }

        Map<uint32_t, size_t> sampleOffset;
        for (auto const& segment : segmentsBySequence())
        {
            outTrackIdx = outTrackIdxBase;
            for (SequenceId trackId : initSegTrackIds)
            {
                auto trackInfoIt = segment.trackInfos.find(trackId);
                if (trackInfoIt != segment.trackInfos.end())
                {
                    const auto& trackInfo = trackInfoIt->second;
                    i                     = sampleOffset[outTrackIdx];

                    if (trackInfo.hasTtyp)
                    {
                        const auto& tBox                                    = trackInfo.ttyp;
                        trackInfos.elements[outTrackIdx].hasTypeInformation = true;
                        trackInfos.elements[outTrackIdx].type.majorBrand    = tBox.getMajorBrand().getUInt32();
                        trackInfos.elements[outTrackIdx].type.minorVersion  = tBox.getMinorVersion();

                        Vector<FourCC> convertedCompatibleBrands;
                        for (const auto& compatibleBrand : tBox.getCompatibleBrands())
                        {
                            convertedCompatibleBrands.push_back(FourCC(compatibleBrand.getUInt32()));
                        }
                        trackInfos.elements[outTrackIdx].type.compatibleBrands =
                            makeArray<FourCC>(convertedCompatibleBrands);
                    }
                    else
                    {
                        trackInfos.elements[outTrackIdx].hasTypeInformation = false;
                    }

                    if (!trackInfo.samples.empty())
                    {
                        std::uint64_t sum = 0;
                        for (const auto& sample : trackInfo.samples)
                        {
                            sum += static_cast<std::uint64_t>(sample.sampleDurationTS);
                        }
                        auto timeScale = mFileProperties.initTrackInfos.at(trackId).timeScale;
                        trackInfos.elements[outTrackIdx].frameRate =
                            Rational{timeScale, sum / trackInfo.samples.size()};
                    }

                    for (auto const& sample : trackInfo.samples)
                    {
                        trackInfos.elements[outTrackIdx].sampleProperties[i].sampleId = sample.sampleId.get();
                        trackInfos.elements[outTrackIdx].sampleProperties[i].sampleEntryType =
                            sample.sampleEntryType.getUInt32();
                        trackInfos.elements[outTrackIdx].sampleProperties[i].sampleDescriptionIndex =
                            sample.sampleDescriptionIndex.get();
                        trackInfos.elements[outTrackIdx].sampleProperties[i].sampleType = sample.sampleType;
                        trackInfos.elements[outTrackIdx].sampleProperties[i].segmentId  = segment.segmentId.get();
                        if (!sample.compositionTimes.empty() != 0u)
                        {
                            // Edit list has been applied to these timestamps already, so negative times shouldn't be
                            // present.
                            trackInfos.elements[outTrackIdx].sampleProperties[i].earliestTimestamp =
                                static_cast<std::uint64_t>(sample.compositionTimes.at(0));
                            trackInfos.elements[outTrackIdx].sampleProperties[i].earliestTimestampTS =
                                sample.compositionTimesTS.at(0);
                        }
                        else
                        {
                            trackInfos.elements[outTrackIdx].sampleProperties[i].earliestTimestamp   = 0;
                            trackInfos.elements[outTrackIdx].sampleProperties[i].earliestTimestampTS = 0;
                        }
                        trackInfos.elements[outTrackIdx].sampleProperties[i].sampleFlags.flagsAsUInt =
                            sample.sampleFlags.flagsAsUInt;
                        trackInfos.elements[outTrackIdx].sampleProperties[i].sampleDurationTS = sample.sampleDurationTS;

                        unsigned int sampleSize = sample.dataLength;
                        if (sampleSize > trackInfos.elements[outTrackIdx].maxSampleSize)
                        {
                            trackInfos.elements[outTrackIdx].maxSampleSize = sampleSize;
                        }
                        i++;
                    }
                    sampleOffset[outTrackIdx] = i;
                }
                ++outTrackIdx;
            }

            // outTrackIdxBase += static_cast<uint32_t>(mFileProperties.initTrackInfos.size());
        }

        return ErrorCode::OK;
    }

}  // namespace HEIF
