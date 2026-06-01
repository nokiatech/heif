/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
#include <cstring>

#include "heiferrorinfo.h"

#include "accessibilitytext.hpp"
#include "auxiliarytypeproperty.hpp"
#include "cleanaperturebox.hpp"
#include "colourinformationbox.hpp"
#include "creationtimeinformation.hpp"
#include "heiffiledatatypesinternal.hpp"
#include "heifreaderimpl.hpp"
#include "imagemirror.hpp"
#include "imagerelativelocationproperty.hpp"
#include "imagerotation.hpp"
#include "imagescaling.hpp"
#include "layerselectorproperty.hpp"
#include "log.hpp"
#include "metabox.hpp"
#include "modificationtimeinformation.hpp"
#include "operatingpointsinformationproperty.hpp"
#include "pixelaspectratiobox.hpp"
#include "pixelinformationproperty.hpp"
#include "contentlightlevelbox.hpp"
#include "masteringdisplaycolourvolumebox.hpp"
#include "contentcolourvolumebox.hpp"
#include "ambientviewingenvironmentbox.hpp"
#include "referenceviewingenvironmentbox.hpp"
#include "nominaldiffusewhitebox.hpp"
#include "requiredreferencetypesproperty.hpp"
#include "subsampleinformationproperty.hpp"
#include "targetolsproperty.hpp"
#include "userdescriptionproperty.hpp"
#include "tonemapimage.hpp"
#include "bbox.hpp"
#include "customallocator.hpp"
#include "decoderconfigrecord.hpp"
#include "decoderconfigurationbox.hpp"
#include "extendedtypebox.hpp"
#include "filetypebox.hpp"
#include "fourccint.hpp"
#include "heifcommondatatypes.h"
#include "heifreaderdatatypes.h"
#include "heifreadersegment.hpp"
#include "heifstreaminternal.hpp"
#include "imagespatialextentsproperty.hpp"
#include "iteminfobox.hpp"
#include "itempropertiesbox.hpp"
#include "itemreferencebox.hpp"
#include "protectionschemeinfobox.hpp"
#include "tracktypebox.hpp"
#include "typecombinationbox.hpp"
#include "vvcdecoderconfigrecord.hpp"
#include "writeoncemap.hpp"
#include <cstdint>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

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
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getFileInformation: called before initialize()");
        }

        fileInfo = mFileInformation;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getFileInformation: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getMajorBrand(FourCC& majorBrand) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getMajorBrand: called before initialize()");
        }
        majorBrand = FourCC(mFtyp.getMajorBrand().getUInt32());
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getMajorBrand: exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getMinorVersion(uint32_t& minorVersion) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getMinorVersion: called before initialize()");
        }
        minorVersion = mFtyp.getMinorVersion();
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getMinorVersion: exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getCompatibleBrands(Array<FourCC>& compatibleBrands) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getCompatibleBrands: called before initialize()");
        }
        const auto& brands = mFtyp.getCompatibleBrands();

        compatibleBrands = Array<FourCC>(brands.size());

        for (uint32_t i = 0; i < brands.size(); ++i)
        {
            compatibleBrands[i] = FourCC(brands[i].getUInt32());
        }

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getCompatibleBrands: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getCompatibleBrandCombinations(Array<Array<FourCC>>& compatibleBrandCombinations) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getCompatibleBrandCombinations: called before initialize()");
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getCompatibleBrandCombinations: exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getDisplayWidth(const SequenceId& sequenceId, uint32_t& displayWidth) const
    {
        HEIF_ENTRY_TRY
        {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }
        displayWidth = getInitTrackInfo(sequenceId).width;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getDisplayWidth: exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getDisplayHeight(const SequenceId& sequenceId, uint32_t& displayHeight) const
    {
        HEIF_ENTRY_TRY
        {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }
        displayHeight = getInitTrackInfo(sequenceId).height;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getDisplayHeight: exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getWidth(const ImageId& itemId, uint32_t& width) const
    {
        HEIF_ENTRY_TRY
        {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }
        width = mMetaBoxInfo.itemInfoMap.at(itemId.get()).width;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getWidth: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getWidth(const SequenceId& sequenceId,
                                       const SequenceImageId& itemId,
                                       uint32_t& width) const
    {
        HEIF_ENTRY_TRY
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getWidth: exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getHeight(const ImageId& itemId, uint32_t& height) const
    {
        HEIF_ENTRY_TRY
        {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }
        height = mMetaBoxInfo.itemInfoMap.at(itemId.get()).height;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getHeight: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getHeight(const SequenceId& sequenceId,
                                        const SequenceImageId& itemId,
                                        uint32_t& height) const
    {
        HEIF_ENTRY_TRY
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getHeight: exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getMatrix(Array<std::int32_t>& matrix) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getMatrix: called before initialize()");
        }

        if (mFileProperties.moovProperties.mMatrix.empty())
        {
            return HEIF_FAIL(ErrorCode::NOT_APPLICABLE,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getMatrix: operation not valid for this file/state");
        }

        matrix = makeArray<int32_t>(mFileProperties.moovProperties.mMatrix);
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getMatrix: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getMatrix(const SequenceId& sequenceId, Array<int32_t>& matrix) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getMatrix: called before initialize()");
        }

        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }
        matrix = makeArray<int32_t>(mFileProperties.initTrackInfos.at(sequenceId.get()).matrix);
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getMatrix: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getPlaybackDurationInSecs(const SequenceId& sequenceId, double& durationInSecs) const
    {
        HEIF_ENTRY_TRY
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
                            maxTimeUs, static_cast<int64_t>((compositionTimesTS + sample.sampleDurationTS) * 1000000 / timescale));
                    }
                }
            }
        }
        durationInSecs = static_cast<double>(maxTimeUs) / 1000000.0;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getPlaybackDurationInSecs: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getMasterImages(Array<ImageId>& itemIds) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getMasterImages: called before initialize()");
        }

        Vector<ImageId> allItems;
        getCollectionItems(allItems);

        Vector<ImageId> masterItemIds;
        for (auto itemId : allItems)
        {
            const auto rawType = mMetaBox.getItemInfoBox().getItemById(itemId.get()).getItemType().getString();
            // L-HEVC enhancement items (ISO/IEC 14496-15 \u00a79.6) carry the
            // "lhv1" four-cc and are valid master images in their own right
            // even though they reference a base-layer hvc1 via an "sbas"
            // item reference; the layered pair is the user-visible image.
            if ((rawType == "avc1") || (rawType == "hvc1") || (rawType == "lhv1") || (rawType == "vvc1"))
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getMasterImages: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getMasterImages(const SequenceId& sequenceId, Array<SequenceImageId>& itemIds) const
    {
        HEIF_ENTRY_TRY
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getMasterImages: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getItemListByType(const FourCC& itemType, Array<ImageId>& itemIds) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItemListByType(Arra): called before initialize()");
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItemListByType(Arra): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getItemListByType(const SequenceId& sequenceId,
                                                const TrackSampleType& sampleType,
                                                Array<SequenceImageId>& sampleIdsApi) const
    {
        HEIF_ENTRY_TRY
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
                            matches.emplace_back(index + sampleBase.get());
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
                            matches.emplace_back(index + sampleBase.get());
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
                            matches.emplace_back(index + sampleBase.get());
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
                            sampleIds.emplace_back(index + sampleBase.get());
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
                            samplePresentationTimes.emplace_back(sampleId, sampleTime);
                        }
                    }

                    // Sort to display order using composition times
                    std::sort(samplePresentationTimes.begin(), samplePresentationTimes.end(),
                              [&](ItemIdTimestampPair a, ItemIdTimestampPair b) { return a.second < b.second; });

                    // Push sample ids to the result
                    for (auto pair : samplePresentationTimes)
                    {
                        matches.emplace_back(pair.first.get());
                    }
                }
            }
            else if (sampleType == TrackSampleType::samples)
            {
                matches = allSamples;
            }
            else
            {
                return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                                 ErrorOrigin::Reader_ItemValidation,
                                 "HeifReaderImpl::getItemListByType(Arra): invalid argument");
            }
        }

        sampleIdsApi = makeArray<SequenceImageId>(matches);

        if (foundSegment == false)
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItemListByType(Arra): sequence (track) id not found");
        }

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItemListByType(Arra): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getItemType(const ImageId& itemId, FourCC& type) const
    {
        HEIF_ENTRY_TRY
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

        return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                         ErrorOrigin::Reader_ItemValidation,
                         "HeifReaderImpl::getItemType(FourCC): item id not found");
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItemType(FourCC): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getItemType(const SequenceId& sequenceId,
                                          const SequenceImageId& sequenceImageId,
                                          FourCC& type) const
    {
        HEIF_ENTRY_TRY
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItemType(FourCC): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getReferencedFromItemListByType(const ImageId& fromItemId,
                                                              const FourCC& referenceType,
                                                              Array<ImageId>& itemIds) const
    {
        HEIF_ENTRY_TRY
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getReferencedFromItemListByType: exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getReferencedToItemListByType(const ImageId& toItemId,
                                                            const FourCC& referenceType,
                                                            Array<ImageId>& itemIds) const
    {
        HEIF_ENTRY_TRY
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
                    itemIdVector.emplace_back(reference.getFromItemID());
                }
            }
        }

        itemIds = makeArray<ImageId>(itemIdVector);
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getReferencedToItemListByType: exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getPrimaryItem(ImageId& itemId) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getPrimaryItem: called before initialize()");
        }
        if (!mIsPrimaryItemSet)
        {
            return HEIF_FAIL(ErrorCode::PRIMARY_ITEM_NOT_SET,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getPrimaryItem: no primary item has been set");
        }

        itemId = mPrimaryItemId;

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getPrimaryItem: exception escaped firewall")
    }

    /// @todo Avoid data copying.
    /// @todo add support for tmap item (contains metadata)
    ErrorCode HeifReaderImpl::getItemData(const ImageId& itemId,
                                          uint8_t* memoryBuffer,
                                          uint64_t& memoryBufferSize,
                                          bool bytestreamHeaders) const
    {
        HEIF_ENTRY_TRY
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
                return HEIF_FAIL(ErrorCode::FILE_HEADER_ERROR,
                                 ErrorOrigin::Reader_BoxParse,
                                 "HeifReaderImpl::getItemData: file header (ftyp) is malformed or missing");
            }
        }
        catch (...)
        {
            return HEIF_FAIL_FROM_UNKNOWN(ErrorCode::FILE_READ_ERROR,
                                          ErrorOrigin::Reader_FileRead,
                                          "getItemDataWithDecoderParameters: itemLength resolution threw");
        }

        ///@todo why is the size limited to UINT32T_MAX ??

        // Compute the post-transform buffer size for the size-probe contract.
        // VVC items are grown in place by processVvcItemData /
        // processCompactVvcItemData; the caller-allocated buffer must be
        // big enough for the expanded form, not just itemLength.
        uint64_t extraBytes = 0;
        {
            FourCCInt rawTypeForSize;
            ErrorCode rawErr = getRawItemType(mMetaBox, itemId, rawTypeForSize);
            if (rawErr == ErrorCode::OK && rawTypeForSize == "vvc1" &&
                mMetaBox.isFromMini())
            {
                if (!mMetaBox.isMultilayer())
                {
                    // Compact case: the item is bare NAL payload and
                    // processCompactVvcItemData synthesises a 4-byte length
                    // field + 2-byte NAL header (8/1/0).
                    extraBytes = 6;
                }
                else
                {
                    // Multi-layer case: the item is a chain of length-prefixed
                    // NAL units.  processVvcItemData replaces each lengthSize-
                    // byte length field with a 4-byte start code, growing the
                    // buffer by numNALs * (4 - lengthSize).  numNALs was
                    // recorded by parseMiniBox; lengthSize comes from vvcC.
                    const auto* configBox = mMetaBox.getConfigBox(itemId.get());
                    if (configBox != nullptr)
                    {
                        const uint8_t lengthSize = dynamic_cast<const VvcDecoderConfigurationRecord&>(
                            configBox->getConfiguration()).getLengthSize();
                        if (lengthSize < 4)
                        {
                            const uint32_t numNALs = mMetaBox.getNalUnitCount(itemId.get());
                            extraBytes = static_cast<uint64_t>(numNALs) *
                                         static_cast<uint64_t>(4u - lengthSize);
                        }
                    }
                }
            }
        }

        if (memoryBufferSize < itemLength + extraBytes)
        {
            memoryBufferSize = static_cast<uint32_t>(itemLength + extraBytes);
            return HEIF_FAIL(ErrorCode::BUFFER_SIZE_TOO_SMALL,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItemData: caller-supplied buffer is too small for the data");
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
            logError() << "Error: " << exc.what() << '\n';
            return HEIF_FAIL_FROM(ErrorCode::FILE_READ_ERROR,
                                  ErrorOrigin::Reader_BoxParse,
                                  "getItemDataWithDecoderParameters: readItem ISOBMFF parse threw",
                                  exc);
        }
        catch (const std::exception& e)
        {
            logError() << "Error: " << e.what() << '\n';
            return HEIF_FAIL_FROM(ErrorCode::FILE_READ_ERROR,
                                  ErrorOrigin::Reader_FileRead,
                                  "getItemDataWithDecoderParameters: readItem stream IO threw",
                                  e);
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

        //define processData for actual images (need bytestream processing) (no metadata items)
        if (!isProtected && ((rawType == "hvc1") || (rawType == "avc1") || (rawType == "vvc1")))
        {
            processData = true;
        }

        //If the item is formatted as compact VVC, it needs to be expanded first.
        //processCompactVvcItemData synthesises a single NAL with a 4-byte length
        //prefix + 2-byte NAL header (INSERT_SIZE = 6).  After this step the
        //buffer is in 4-byte-length-prefixed form regardless of the configured
        //LengthSizeMinusOne in the synthesised VvcDecoderConfigurationRecord,
        //so the subsequent processVvcItemData() pass MUST be invoked with
        //lengthSize == 4 to (a) avoid mis-parsing the synthesised length and
        //(b) avoid inserting extra bytes that the (NULL-buffer) size probe in
        //the branch above did not account for -- that would write past the
        //caller-allocated buffer end.  See BUG: compact-VVC bitstream() probe
        //size mismatch (Python binding heap-overrun warning).
        const bool isCompactVvc =
            (rawType == "vvc1") && mMetaBox.isFromMini() && !mMetaBox.isMultilayer();
        if (isCompactVvc){

            //Get and modify item data from VVC bitstream
            error = processCompactVvcItemData(memoryBuffer, memoryBufferSize);

            if (error != ErrorCode::OK){ return error; }
        }

        // Process itemdata with length field and convert it to startcode
        if (processData && bytestreamHeaders)
        {
            // Process bitstream by codec
            FourCC codeType;
            error = getDecoderCodeType(itemId, codeType);
            if (error != ErrorCode::OK){
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
            else if (codeType == FourCC("vvc1"))
            {
                ///@todo with VVC items and makeConfigFromSPS, this will be hard-set to 4
                ///@bug multilayer vvc with lengthsize != 4 will probably missread nalu size
                //
                // For compact-VVC items the upstream processCompactVvcItemData
                // already produced a 4-byte length prefix; force lengthSize=4
                // here so processVvcItemData only converts the length field
                // to a start-code without inserting any further bytes (which
                // would exceed the size returned by the NULL-buffer probe).
                uint8_t lengthSize = isCompactVvc
                    ? static_cast<uint8_t>(4)
                    : dynamic_cast<const VvcDecoderConfigurationRecord&>(
                          mMetaBox.getConfigBox(itemId.get())->getConfiguration())
                          .getLengthSize();

                //Get item data from VVC bitstream
                error = processVvcItemData(memoryBuffer, memoryBufferSize, lengthSize);
                if (error != ErrorCode::OK){
                    return error;
                }
            }else{
                // Code type not supported
                return HEIF_FAIL(ErrorCode::UNSUPPORTED_CODE_TYPE,
                                 ErrorOrigin::Reader_DecoderConfig,
                                 "HeifReaderImpl::getItemData: codec type not supported");
            }
        }
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItemData: exception escaped firewall")
    }

    /// @todo Avoid data copying.
    ErrorCode HeifReaderImpl::getItemData(const SequenceId& sequenceId,
                                          const SequenceImageId& itemId,
                                          uint8_t* memoryBuffer,
                                          uint64_t& memoryBufferSize,
                                          bool bytestreamHeaders)
    {
        HEIF_ENTRY_TRY
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
            else if (codeType == FourCC("vvc1") || codeType == FourCC("vvs1"))
            {
                ///@todo if this is possible (compact vvc in sequence), implement
                uint8_t lengthSize =  dynamic_cast<const VvcDecoderConfigurationRecord&>(mMetaBox.getConfigBox(itemId.get())->getConfiguration()).getLengthSize();
                if(mMetaBox.isFromMini()){
                    error = processCompactVvcItemData(memoryBuffer, memoryBufferSize);
                }else{
                    // Get item data from VVC bitstream
                    error = processVvcItemData(memoryBuffer, memoryBufferSize, lengthSize);
                }
                if (error != ErrorCode::OK){
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
                return HEIF_FAIL(ErrorCode::UNSUPPORTED_CODE_TYPE,
                                 ErrorOrigin::Reader_DecoderConfig,
                                 "HeifReaderImpl::getItemData: codec type not supported");
            }
        }
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItemData: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getTrackSampleData(const SequenceId& trackId,
                                                 const SequenceImageId& itemIdApi,
                                                 uint8_t* memoryBuffer,
                                                 uint64_t& memoryBufferSize)
    {
        HEIF_TRY
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
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getTrackSampleData: item id not found");
        }

        const uint32_t sampleLength = getTrackInfo(segTrackId).samples.at(itemId.get()).dataLength;
        if (memoryBufferSize < sampleLength)
        {
            memoryBufferSize = sampleLength;
            return HEIF_FAIL(ErrorCode::MEMORY_TOO_SMALL_BUFFER,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getTrackSampleData: internal buffer too small to hold the data");
        }

        seekInput(io, static_cast<std::int64_t>(getTrackInfo(segTrackId).samples.at(itemId.get()).dataOffset));
        io.stream->read(reinterpret_cast<char*>(memoryBuffer), sampleLength);
        memoryBufferSize = sampleLength;

        if (!io.stream->good())
        {
            return HEIF_FAIL(ErrorCode::FILE_READ_ERROR,
                             ErrorOrigin::Reader_FileRead,
                             "HeifReaderImpl::getTrackSampleData: underlying stream read failed");
        }

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getTrackSampleData: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::segmentIdOf(SequenceId sequenceId, SequenceImageId itemId, SegmentId& segmentId) const
    {
        HEIF_TRY
        {
        bool wentPast = false;

        if (mFileProperties.segmentPropertiesMap.empty())
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEGMENT,
                             ErrorOrigin::Reader_BoxParse,
                             "HeifReaderImpl::segmentIdOf: segment id not found");
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
                if (itemId.get() - track->second.itemIdBase.get() < static_cast<std::uint32_t>(track->second.samples.size()))
                {
                    ret = ErrorCode::OK;
                }
            }
        }
        return ret;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::segmentIdOf: exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getItem(const ImageId& itemId, Overlay& iovlItem) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItem(Overlay): called before initialize()");
        }

        bool isProtected = false;
        ErrorCode error  = getProtection(itemId, isProtected);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        if (isProtected)
        {
            return HEIF_FAIL(ErrorCode::PROTECTED_ITEM,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItem(Overlay): item is DRM-protected and cannot be accessed");
        }
        if (mMetaBoxInfo.iovlItems.count(itemId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItem(Overlay): item id not found");
        }
        iovlItem = mMetaBoxInfo.iovlItems.at(itemId);
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItem(Overlay): exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getItem(const ImageId& itemId, Grid& gridItem) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItem(Grid): called before initialize()");
        }

        bool isProtected = false;
        ErrorCode error  = getProtection(itemId, isProtected);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        if (isProtected)
        {
            return HEIF_FAIL(ErrorCode::PROTECTED_ITEM,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItem(Grid): item is DRM-protected and cannot be accessed");
        }
        if (mMetaBoxInfo.gridItems.count(itemId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItem(Grid): item id not found");
        }
        gridItem = mMetaBoxInfo.gridItems.at(itemId);
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItem(Grid): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getItem(const ImageId& itemId, Tonemap& tmapItem) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItem(Tonemap): called before initialize()");
        }

        bool isProtected = false;
        ErrorCode error  = getProtection(itemId, isProtected);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        if (isProtected)
        {
            return HEIF_FAIL(ErrorCode::PROTECTED_ITEM,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItem(Tonemap): item is DRM-protected and cannot be accessed");
        }
        if (mMetaBoxInfo.tmapItems.count(itemId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItem(Tonemap): item id not found");
        }

        const ToneMapImage&    toneMapImage = mMetaBoxInfo.tmapItems.at(itemId);
        const GainMapMetadata& md           = toneMapImage.getGainMapMetadata();

        GainMapData& gmap                   = tmapItem.gain_map_metadata;
        gmap.minimumVersion                 = md.getVersion().minimumVersion;
        gmap.writerVersion                  = md.getVersion().writerVersion;
        gmap.isMultichannel                 = md.isMultichannel();
        gmap.useBaseColourSpace             = md.useBaseColourSpace();
        gmap.baseHdrHeadroomNumerator       = md.getBaseHdrHeadroomNumerator();
        gmap.baseHdrHeadroomDenominator     = md.getBaseHdrHeadroomDenominator();
        gmap.alternateHdrHeadroomNumerator  = md.getAlternateHdrHeadroomNumerator();
        gmap.alternateHdrHeadroomDenominator= md.getAlternateHdrHeadroomDenominator();

        const auto& srcChannels = md.getChannels();
        gmap.channelCount       = static_cast<int>(srcChannels.size());
        gmap.channels           = Array<GainMapData::Channel>(srcChannels.size());
        for (size_t i = 0; i < srcChannels.size(); ++i)
        {
            const auto& s = srcChannels.at(i);
            GainMapData::Channel c;
            c.gainMapMinNumerator        = s.gainMapMinNumerator;
            c.gainMapMinDenominator      = s.gainMapMinDenominator;
            c.gainMapMaxNumerator        = s.gainMapMaxNumerator;
            c.gainMapMaxDenominator      = s.gainMapMaxDenominator;
            c.gammaNumerator             = s.gammaNumerator;
            c.gammaDenominator           = s.gammaDenominator;
            c.baseOffsetNumerator        = s.baseOffsetNumerator;
            c.baseOffsetDenominator      = s.baseOffsetDenominator;
            c.alternateOffsetNumerator   = s.alternateOffsetNumerator;
            c.alternateOffsetDenominator = s.alternateOffsetDenominator;
            gmap.channels[i] = c;
        }

        // For tmap derived items the dimg reference list is, by spec,
        // [base_image, gain_map_image] in that order.
        getReferencedFromItemListByType(itemId, "dimg", tmapItem.imageIds);

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItem(Tonemap): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, Scale& iscl) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(Scale): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "iscl"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(Scale): property at index is not 'iscl' (ImageScaling) - actual fourcc differs");
        }
        const auto isclPtr = static_cast<const ImageScaling*>(boxPtr);
        iscl.targetWidthD  = isclPtr->getWidth().denominator;
        iscl.targetWidthN  = isclPtr->getWidth().numerator;
        iscl.targetHeightD = isclPtr->getHeight().denominator;
        iscl.targetHeightN = isclPtr->getHeight().numerator;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(Scale): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, LayerSelector& lsel) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(LayerSelector): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "lsel"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(LayerSelector): property at index is not 'lsel' (LayerSelector) - actual fourcc differs");
        }
        const auto* lselPtr = static_cast<const LayerSelectorProperty*>(boxPtr);
        lsel.layerId        = lselPtr->getLayerId();
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(LayerSelector): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, TargetOls& tols) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(TargetOls): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "tols"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(TargetOls): property at index is not 'tols' (TargetOls) - actual fourcc differs");
        }
        const auto* tolsPtr   = static_cast<const TargetOlsProperty*>(boxPtr);
        tols.targetOlsIndex   = tolsPtr->getOlsIndex();
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(TargetOls): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, OperatingPointsInfo& oinf) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(OperatingPointsInfo): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "oinf"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(OperatingPointsInfo): property at index is not 'oinf' (OperatingPointsInformation) - actual fourcc differs");
        }
        const auto* oinfPtr = static_cast<const OperatingPointsInformationProperty*>(boxPtr);

        oinf.scalabilityMask = oinfPtr->getScalabilityMask();

        const auto& srcPtls = oinfPtr->getProfileTierLevels();
        oinf.profileTierLevels = Array<OperatingPointsInfo::ProfileTierLevel>(srcPtls.size());
        for (std::size_t i = 0; i < srcPtls.size(); ++i)
        {
            oinf.profileTierLevels[i].generalProfileSpace              = srcPtls[i].generalProfileSpace;
            oinf.profileTierLevels[i].generalTierFlag                  = srcPtls[i].generalTierFlag;
            oinf.profileTierLevels[i].generalProfileIdc                = srcPtls[i].generalProfileIdc;
            oinf.profileTierLevels[i].generalProfileCompatibilityFlags = srcPtls[i].generalProfileCompatibilityFlags;
            oinf.profileTierLevels[i].generalConstraintIndicatorFlags  = srcPtls[i].generalConstraintIndicatorFlags;
            oinf.profileTierLevels[i].generalLevelIdc                  = srcPtls[i].generalLevelIdc;
        }

        const auto& srcOps = oinfPtr->getOperatingPoints();
        oinf.operatingPoints = Array<OperatingPointsInfo::OperatingPoint>(srcOps.size());
        for (std::size_t i = 0; i < srcOps.size(); ++i)
        {
            const auto& src = srcOps[i];
            auto&       dst = oinf.operatingPoints[i];
            dst.outputLayerSetIdx = src.outputLayerSetIdx;
            dst.maxTemporalId     = src.maxTemporalId;
            dst.layers            = Array<OperatingPointsInfo::OperatingPointLayer>(src.layers.size());
            for (std::size_t j = 0; j < src.layers.size(); ++j)
            {
                dst.layers[j].ptlIdx                 = src.layers[j].ptlIdx;
                dst.layers[j].layerId                = src.layers[j].layerId;
                dst.layers[j].isOutputLayer          = src.layers[j].isOutputLayer ? 1u : 0u;
                dst.layers[j].isAlternateOutputLayer = src.layers[j].isAlternateOutputLayer ? 1u : 0u;
            }
            dst.minPicWidth        = src.minPicWidth;
            dst.minPicHeight       = src.minPicHeight;
            dst.maxPicWidth        = src.maxPicWidth;
            dst.maxPicHeight       = src.maxPicHeight;
            dst.maxChromaFormat    = src.maxChromaFormat;
            dst.maxBitDepthMinus8  = src.maxBitDepthMinus8;
            dst.frameRateInfoFlag  = src.frameRateInfoFlag ? 1u : 0u;
            dst.bitRateInfoFlag    = src.bitRateInfoFlag ? 1u : 0u;
            dst.avgFrameRate       = src.avgFrameRate;
            dst.constantFrameRate  = src.constantFrameRate;
            dst.maxBitRate         = src.maxBitRate;
            dst.avgBitRate         = src.avgBitRate;
        }

        const auto& srcLayers = oinfPtr->getLayers();
        oinf.layers = Array<OperatingPointsInfo::LayerInfo>(srcLayers.size());
        for (std::size_t i = 0; i < srcLayers.size(); ++i)
        {
            oinf.layers[i].dependentLayerId = srcLayers[i].dependentLayerId;
            oinf.layers[i].dependsOnLayerIds = Array<uint8_t>(srcLayers[i].dependsOnLayerIds.size());
            for (std::size_t j = 0; j < srcLayers[i].dependsOnLayerIds.size(); ++j)
            {
                oinf.layers[i].dependsOnLayerIds[j] = srcLayers[i].dependsOnLayerIds[j];
            }
            oinf.layers[i].dimensionIdentifiers = Array<uint8_t>(srcLayers[i].dimensionIdentifiers.size());
            for (std::size_t j = 0; j < srcLayers[i].dimensionIdentifiers.size(); ++j)
            {
                oinf.layers[i].dimensionIdentifiers[j] = srcLayers[i].dimensionIdentifiers[j];
            }
        }
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(OperatingPointsInfo): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, SubSampleInfo& subs) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(SubSampleInfo): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "subs"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(SubSampleInfo): property at index is not 'subs' - actual fourcc differs");
        }
        const auto* subsPtr = static_cast<const SubSampleInformationProperty*>(boxPtr);
        const auto& entries = subsPtr->getSubSamples();
        subs.subSamples = Array<SubSampleInfo::SubSampleEntry>(entries.size());
        for (std::size_t i = 0; i < entries.size(); ++i)
        {
            subs.subSamples[i].subsampleSize           = entries[i].subsampleSize;
            subs.subSamples[i].subsamplePriority       = entries[i].subsamplePriority;
            subs.subSamples[i].discardable             = entries[i].discardable;
            subs.subSamples[i].codecSpecificParameters = entries[i].codecSpecificParameters;
        }
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(SubSampleInfo): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, CreationTimeInformation& crtt) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(CreationTimeInformation): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "crtt"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(CreationTimeInformation): property at index is not 'crtt' (CreationTimeProperty) - actual fourcc differs");
        }
        const auto crttPtr = static_cast<const CreationTimeProperty*>(boxPtr);
        crtt.time          = crttPtr->getCreationTime();
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(CreationTimeInformation): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, ModificationTimeInformation& mdft) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(ModificationTimeInformation): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "mdft"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(ModificationTimeInformation): property at index is not 'mdft' (ModificationTimeProperty) - actual fourcc differs");
        }
        const auto mdftPtr = static_cast<const ModificationTimeProperty*>(boxPtr);
        mdft.time          = mdftPtr->getModificationTime();
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(ModificationTimeInformation): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, AccessibilityText& altt) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(AccessibilityText): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "altt"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(AccessibilityText): property at index is not 'altt' (AccessibilityTextProperty) - actual fourcc differs");
        }
        const auto alttPtr = static_cast<const AccessibilityTextProperty*>(boxPtr);
        altt.lang          = makeArray<char>(alttPtr->getLang());
        altt.text          = makeArray<char>(alttPtr->getText());
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(AccessibilityText): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, SpatialExtents& ispe) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(SpatialExtents): called before initialize()");
        }

        const ImageSpatialExtentsProperty* ispePtr = dynamic_cast<const ImageSpatialExtentsProperty*>(mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get()));
        if (ispePtr == nullptr)
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(SpatialExtents): property at index is not 'ispe' (ImageSpatialExtentsProperty)");
        }

        ispe.mImageHeight = ispePtr->getDisplayHeight();
        ispe.mImageWidth = ispePtr->getDisplayWidth();

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(SpatialExtents): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, ContentLightLevel& clli) const
    {
        HEIF_ENTRY_TRY
        {
        if( isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(ContentLightLevel): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "clli"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(ContentLightLevel): property at index is not 'clli' (ContentLightLevelBox) - actual fourcc differs");
        }
        const auto clliPtr = static_cast<const ContentLightLevelBox*>(boxPtr);

        clli.maxContentLightLevel = clliPtr->getMaxContentLightLevel();
        clli.maxPicAverageLightLevel = clliPtr->getMaxPicAverageLightLevel();

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(ContentLightLevel): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, MasteringDisplayColorVolume& mdcv) const
    {
        HEIF_ENTRY_TRY
        {
        if( isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(MasteringDisplayColorVolume): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "mdcv"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(MasteringDisplayColorVolume): property at index is not 'mdcv' (MasteringDisplayColourVolumeBox) - actual fourcc differs");
        }
        const auto mdcvPtr = static_cast<const MasteringDisplayColourVolumeBox*>(boxPtr);

        const auto& srcX = mdcvPtr->getDisplayPrimariesX();
        mdcv.displayPrimariesX = Array<uint16_t>(srcX.begin(), srcX.end());

        const auto& srcY = mdcvPtr->getDisplayPrimariesY();
        mdcv.displayPrimariesY = Array<uint16_t>(srcY.begin(), srcY.end());

        mdcv.whitePointX = mdcvPtr->getWhitePointX();
        mdcv.whitePointY = mdcvPtr->getWhitePointY();
        mdcv.maxDisplayMasteringLuminance = mdcvPtr->getMaxDisplayMasteringLuminance();
        mdcv.minDisplayMasteringLuminance = mdcvPtr->getMinDisplayMasteringLuminance();

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(MasteringDisplayColorVolume): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, ContentColorVolume& cclv) const
    {
        HEIF_ENTRY_TRY
        {
        if( isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(ContentColorVolume): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "cclv"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(ContentColorVolume): property at index is not 'cclv' (ContentColourVolumeBox) - actual fourcc differs");
        }
        const auto cclvPtr = static_cast<const ContentColourVolumeBox*>(boxPtr);
        cclv.ccvAvgLuminanceValuePresent = cclvPtr->isCcvAvgLuminanceValuePresent();
        cclv.ccvMinLuminanceValuePresent = cclvPtr->isCcvMinLuminanceValuePresent();
        cclv.ccvMaxLuminanceValuePresent = cclvPtr->isCcvMaxLuminanceValuePresent();
        cclv.ccvPrimariesPresent = cclvPtr->isCcvPrimariesPresent();
        
        const auto& srcX = cclvPtr->getCcvPrimariesX();
        cclv.ccvPrimariesX = Array<uint32_t>(srcX.begin(), srcX.end());

        const auto& srcY = cclvPtr->getCcvPrimariesY();
        cclv.ccvPrimariesY = Array<uint32_t>(srcY.begin(), srcY.end());

        cclv.ccvMinLuminanceValue = cclvPtr->getCcvMinLuminance();
        cclv.ccvMaxLuminanceValue = cclvPtr->getCcvMaxLuminance();
        cclv.ccvAvgLuminanceValue = cclvPtr->getCcvAvgLuminance();
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(ContentColorVolume): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, AmbientViewingEnvironment& amve) const
    {
        HEIF_ENTRY_TRY
        {
        if( isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(AmbientViewingEnvironment): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "amve"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(AmbientViewingEnvironment): property at index is not 'amve' (AmbientViewingEnvironmentBox) - actual fourcc differs");
        }
        const auto amvePtr = static_cast<const AmbientViewingEnvironmentBox*>(boxPtr);

        amve.ambientIlluminance = amvePtr->getAmbientIlluminance();
        amve.ambientLightX = amvePtr->getAmbientLightX();;
        amve.ambientLightY = amvePtr->getAmbientLightY();

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(AmbientViewingEnvironment): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, ReferenceViewingEnvironment& reve) const
    {
        HEIF_ENTRY_TRY
        {
        if( isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(ReferenceViewingEnvironment): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "reve"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(ReferenceViewingEnvironment): property at index is not 'reve' (ReferenceViewingEnvironmentBox) - actual fourcc differs");
        }
        const auto revePtr = static_cast<const ReferenceViewingEnvironmentBox*>(boxPtr);

        reve.peripheryLightX = revePtr->getPeripheryLightX();
        reve.peripheryLightY = revePtr->getPeripheryLightY();
        reve.peripheryLuminance = revePtr->getPeripheryLuminance();
        reve.surroundLightX = revePtr->getSurroundLightX();
        reve.surroundLightY = revePtr->getSurroundLightY();
        reve.surroundLuminance = revePtr->getSurroundLuminance();

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(ReferenceViewingEnvironment): exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, NominalDiffuseWhite& ndwt) const{
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(NominalDiffuseWhite): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "ndwt"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(NominalDiffuseWhite): property at index is not 'ndwt' (NominalDiffuseWhiteBox) - actual fourcc differs");
        }
        const auto ndwtPtr = static_cast<const NominalDiffuseWhiteBox*>(boxPtr);

        ndwt.diffuseWhiteLuminance = ndwtPtr->getDiffuseWhiteLuminance();

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, RequiredReferenceTypes& rref) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(RequiredReferenceTypes): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "rref"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(RequiredReferenceTypes): property at index is not 'rref' (RequiredReferenceTypesProperty) - actual fourcc differs");
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(RequiredReferenceTypes): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, UserDescription& udes) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(UserDescription): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "udes"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(UserDescription): property at index is not 'udes' (UserDescriptionProperty) - actual fourcc differs");
        }
        auto udesPtr = static_cast<const UserDescriptionProperty*>(boxPtr);

        udes.lang        = makeArray<char>(udesPtr->getLang());
        udes.name        = makeArray<char>(udesPtr->getName());
        udes.description = makeArray<char>(udesPtr->getDescription());
        udes.tags        = makeArray<char>(udesPtr->getTags());
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(UserDescription): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, AuxiliaryType& auxc) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(AuxiliaryType,item): called before initialize()");
        }
        const auto* boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "auxC"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(AuxiliaryType,item): property at index is not 'auxC' (AuxiliaryTypeProperty) - actual fourcc differs");
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(AuxiliaryType): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const SequenceId& sequenceId,
                                          const std::uint32_t index,
                                          AuxiliaryType& auxc) const
    {
        HEIF_ENTRY_TRY
        {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }
        if (mFileProperties.initTrackInfos.at(sequenceId).auxiProperties.count(index) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_SAMPLE_DESCRIPTION_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(AuxiliaryType,track): sample description index has no auxi property in the requested track");
        }

        auxc.auxType = mFileProperties.initTrackInfos.at(sequenceId).auxiProperties.at(index).auxType;
        auxc.subType = {};  /// @todo Should read also SEI messages from sample entry to here.
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(AuxiliaryType): exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, Mirror& imir) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(Mirror): called before initialize()");
        }

        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "imir"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(Mirror): property at index is not 'imir' (ImageMirror) - actual fourcc differs");
        }
        auto imirBox = static_cast<const ImageMirror*>(boxPtr);

        imir.horizontalAxis = imirBox->getHorizontalAxis();
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(Mirror): exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, Rotate& irot) const
    {
        HEIF_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(Rotate): called before initialize()");
        }

        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "irot"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(Rotate): property at index is not 'irot' (ImageRotation) - actual fourcc differs");
        }
        auto irotBox = static_cast<const ImageRotation*>(boxPtr);

        irot.angle = irotBox->getAngle();
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(Rotate): exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, RelativeLocation& rloc) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(RelativeLocation): called before initialize()");
        }

        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "rloc"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(RelativeLocation): property at index is not 'rloc' (ImageRelativeLocationProperty) - actual fourcc differs");
        }
        auto rlocPtr = static_cast<const ImageRelativeLocationProperty*>(boxPtr);

        rloc.horizontalOffset = rlocPtr->getHorizontalOffset();
        rloc.verticalOffset   = rlocPtr->getVerticalOffset();
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(RelativeLocation): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, PixelInformation& pixi) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(PixelInformation): called before initialize()");
        }
        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "pixi"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(PixelInformation): property at index is not 'pixi' (PixelInformationProperty) - actual fourcc differs");
        }
        auto pixiPtr = static_cast<const PixelInformationProperty*>(boxPtr);

        pixi.numChannels = pixiPtr->getNumChannels();
        const auto& bitsPerChannels = pixiPtr->getBitsPerChannels();
        pixi.bitsPerChannel         = makeArray<std::uint8_t>(bitsPerChannels);
        const auto& interimChanData = pixiPtr->getChannelData();
        pixi.ChannelData = Array<PixelInformation::ChannelEntry>(interimChanData.size());
        size_t entryIdx = 0;
        for(const auto& chanData : interimChanData){
            PixelInformation::ChannelInfo inputChannelInfo;
            inputChannelInfo.ComponentFormat = chanData.second.ComponentFormat;
            inputChannelInfo.SubSamplingFlag = chanData.second.SubSamplingFlag;
            inputChannelInfo.ChannelLabelFlag = chanData.second.ChannelLabelFlag;
            inputChannelInfo.SubSamplingType = chanData.second.SubSamplingType;
            inputChannelInfo.SubSamplingLocation = chanData.second.SubSamplingLocation;
            const auto& label = chanData.second.ChannelLabel;
            inputChannelInfo.ChannelLabel = Array<char>(label.begin(), label.end());

            pixi.ChannelData[entryIdx].channelIdc = chanData.first;
            pixi.ChannelData[entryIdx].info = inputChannelInfo;
            ++entryIdx;
        }

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(PixelInformation): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, ColourInformation& colr) const
    {
        HEIF_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(ColourInformation): called before initialize()");
        }

        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "colr"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(ColourInformation): property at index is not 'colr' (ColourInformationBox) - actual fourcc differs");
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(ColourInformation): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, PixelAspectRatio& pasp) const
    {
        HEIF_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(PixelAspectRatio): called before initialize()");
        }

        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "pasp"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(PixelAspectRatio): property at index is not 'pasp' (PixelAspectRatioBox) - actual fourcc differs");
        }
        auto paspPtr = static_cast<const PixelAspectRatioBox*>(boxPtr);

        pasp.relativeWidth  = paspPtr->getRelativeWidth();
        pasp.relativeHeight = paspPtr->getRelativeHeight();
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(PixelAspectRatio): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, CleanAperture& clap) const
    {
        HEIF_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(CleanAperture): called before initialize()");
        }

        auto boxPtr = mMetaBox.getItemPropertiesBox().getPropertyByIndex(index.get());
        if ((boxPtr == nullptr) || (boxPtr->getType() != "clap"))
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(CleanAperture): property at index is not 'clap' (CleanApertureBox) - actual fourcc differs");
        }
        auto clapPtr = static_cast<const CleanApertureBox*>(boxPtr);
        clap         = makeClap(clapPtr);
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(CleanAperture): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, RawProperty& property) const
    {
        HEIF_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(RawProperty): called before initialize()");
        }

        const auto data = mMetaBox.getItemPropertiesBox().getPropertyDataByIndex(index.get());
        if (data.size() < 8)
        {
            return HEIF_FAIL(ErrorCode::INVALID_PROPERTY_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(RawProperty): property data too small (need at least 8 bytes for box header)");
        }

        property.data = makeArray<std::uint8_t>(data);
        property.type = FourCC(reinterpret_cast<const char*>(&data[4]));

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(RawProperty): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getProperty(const SequenceId& sequenceId, std::uint32_t index, CleanAperture& clap) const
    {
        HEIF_ENTRY_TRY
        {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }
        if (mFileProperties.initTrackInfos.at(sequenceId).clapProperties.count(index) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_SAMPLE_DESCRIPTION_INDEX,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getProperty(CleanAperture): sample description index out of range");
        }

        clap = mFileProperties.initTrackInfos.at(sequenceId).clapProperties.at(index);

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getProperty(CleanAperture): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::isValidEntityGroup(const GroupId& groupId) const
    {
        HEIF_TRY
        {
        const auto& entityGroupings = mFileProperties.rootLevelMetaBoxProperties.entityGroupings;

        auto found = std::find_if(entityGroupings.cbegin(), entityGroupings.cend(),
                                  [&groupId](const EntityGrouping& x) { return x.groupId == groupId; });
        if (found == entityGroupings.cend())
        {
            return HEIF_FAIL(ErrorCode::INVALID_GROUP_ID,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::isValidEntityGroup: group id not found");
        }

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_Derivation,
                              "HeifReaderImpl::isValidEntityGroup: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getItemProperties(const GroupId& groupId, Array<ItemPropertyInfo>& propertyTypes) const
    {
        HEIF_ENTRY_TRY
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItemProperties: exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getItemProperties(const ImageId& itemId, Array<ItemPropertyInfo>& propertyTypes) const
    {
        HEIF_ENTRY_TRY
        {
        ErrorCode error;
        if ((error = isValidItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }

        if (mMetaBoxInfo.properties.count(itemId.get()) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItemProperties: item id not found");
        }

        propertyTypes = makeArray<ItemPropertyInfo>(mMetaBoxInfo.properties.at(itemId.get()));
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItemProperties: exception escaped firewall")
    }


    ErrorCode HeifReaderImpl::getItemDataWithDecoderParameters(const ImageId& itemId,
                                                               uint8_t* memoryBuffer,
                                                               uint64_t& memoryBufferSize) const
    {
        HEIF_ENTRY_TRY
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
            return HEIF_FAIL(ErrorCode::PROTECTED_ITEM,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItemDataWithDecoderParameters(uint64_t): item is DRM-protected and cannot be accessed");
        }

        FourCC codeType;
        error = getDecoderCodeType(itemId, codeType);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        ///@todo can this method actually support other types? mp4a as an example
        if ((codeType != FourCC("hvc1")) && (codeType != FourCC("avc1")) && (codeType != FourCC("vvc1")))
        {
            // No other code types supported
            return HEIF_FAIL(ErrorCode::UNSUPPORTED_CODE_TYPE,
                             ErrorOrigin::Reader_DecoderConfig,
                             "HeifReaderImpl::getItemDataWithDecoderParameters(uint64_t): codec type not supported");
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_DecoderConfig,
                              "HeifReaderImpl::getItemDataWithDecoderParameters(uint64_t): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getItemDataWithDecoderParameters(const SequenceId& sequenceId,
                                                               const SequenceImageId& itemId,
                                                               uint8_t* memoryBuffer,
                                                               uint64_t& memoryBufferSize)
    {
        HEIF_ENTRY_TRY
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
            return HEIF_FAIL(ErrorCode::UNSUPPORTED_CODE_TYPE,
                             ErrorOrigin::Reader_DecoderConfig,
                             "HeifReaderImpl::getItemDataWithDecoderParameters(uint64_t): codec type not supported");
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
            // The track-side getItemData (i.e. getTrackSampleData) historically
            // returns the alternate spelling MEMORY_TOO_SMALL_BUFFER on probe
            // while the item-side path returns BUFFER_SIZE_TOO_SMALL.  Accept
            // both so the required total (sample + parameter sets) is
            // propagated to the caller in either case.
            if (error == ErrorCode::BUFFER_SIZE_TOO_SMALL ||
                error == ErrorCode::MEMORY_TOO_SMALL_BUFFER)
            {
                memoryBufferSize = itemSize + static_cast<uint32_t>(parameterSize);
            }
            return error;
        }
        memoryBufferSize = itemSize + static_cast<uint32_t>(parameterSize);
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_DecoderConfig,
                              "HeifReaderImpl::getItemDataWithDecoderParameters(uint64_t): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getItemTimestamps(const SequenceId& sequenceId, Array<TimestampIDPair>& timestamps) const
    {
        HEIF_ENTRY_TRY
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItemTimestamps(Arra): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getTimestampsOfItem(const SequenceId& sequenceId,
                                                  const SequenceImageId& itemIdApi,
                                                  Array<int64_t>& timestamps) const
    {
        HEIF_ENTRY_TRY
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getTimestampsOfItem: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getItemsInDecodingOrder(const SequenceId& sequenceId,
                                                      Array<TimestampIDPair>& decodingOrder) const
    {
        HEIF_ENTRY_TRY
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
                decodingOrderVector.reserve(decodingOrderVector.size() + samples.size());
                // SampleProperties::sampleId is sample-table-entry order, which
                // ISO/IEC 14496-12 Section 8.6 defines as decoding order. Walk the
                // samples once per sample (independent of compositionTimes
                // multiplicity, which is inflated by an edit list) and use
                // the first composition time as the entry's timestamp. Empty
                // compositionTimes (NON_OUTPUT_REFERENCE_FRAME samples have
                // no presentation time but must still be decoded) take 0.
                for (const auto& sample : samples)
                {
                    const std::int64_t firstCompositionTime =
                        sample.compositionTimes.empty() ? 0 : sample.compositionTimes.front();
                    decodingOrderVector.push_back({firstCompositionTime, sample.sampleId});
                }
            }
        }

        // Add possible decoding dependencies
        decodingOrderVector = addDecodingDependencies(sequenceId, decodingOrderVector);

        decodingOrder = makeArray<TimestampIDPair>(decodingOrderVector);
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItemsInDecodingOrder: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getDecodeDependencies(const ImageId& imageId, Array<ImageId>& dependencies) const
    {
        HEIF_ENTRY_TRY
        {
        return getReferencedFromItemListByType(imageId, "pred", dependencies);
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getDecodeDependencies: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getDecodeDependencies(const SequenceId& sequenceId,
                                                    const SequenceImageId& itemId,
                                                    Array<SequenceImageId>& dependencies) const
    {
        HEIF_ENTRY_TRY
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getDecodeDependencies: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getDecoderCodeType(const ImageId& itemId, FourCC& type) const
    {
        HEIF_ENTRY_TRY
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

        return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                         ErrorOrigin::Reader_ItemValidation,
                         "HeifReaderImpl::getDecoderCodeType(FourCC): item id not found");
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getDecoderCodeType(FourCC): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getDecoderCodeType(const SequenceId& trackId,
                                                 const SequenceImageId& sampleId,
                                                 FourCC& decoderCodeType) const
    {
        HEIF_ENTRY_TRY
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
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_IMAGE_ID,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getDecoderCodeType(FourCC): sequence image id not found");
        }
        const auto& segmentProperties = segmentPropertiesIt->second;
        const auto trackInfoIt        = segmentProperties.trackInfos.find(segTrackId.second);
        if (trackInfoIt == segmentProperties.trackInfos.end())
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getDecoderCodeType(FourCC): sequence (track) id not found");
        }
        const TrackInfoInSegment& trackInfo = trackInfoIt->second;
        const auto& decoderCodeTypeMap      = trackInfo.decoderCodeTypeMap;

        auto iter = decoderCodeTypeMap.find(sampleId);
        if (iter != decoderCodeTypeMap.end())
        {
            decoderCodeType = iter->second.getUInt32();
            return ErrorCode::OK;
        }

        return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_IMAGE_ID,
                         ErrorOrigin::Reader_ItemValidation,
                         "HeifReaderImpl::getDecoderCodeType(FourCC): sequence image id not found");
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getDecoderCodeType(FourCC): exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getDecoderParameterSets(const ImageId& itemId, DecoderConfiguration& decoderInfos) const
    {
        HEIF_ENTRY_TRY
        {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }

        if (mImageToParameterSetMap.count(itemId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getDecoderParameterSets: item id not found");
        }
        DecoderConfigId parameterSetId = mImageToParameterSetMap.at(itemId);

        const auto iter = mImageItemParameterSetMap.find(parameterSetId);
        if (iter == mImageItemParameterSetMap.cend())
        {
            return HEIF_FAIL(ErrorCode::FILE_HEADER_ERROR,
                             ErrorOrigin::Reader_BoxParse,
                             "HeifReaderImpl::getDecoderParameterSets: file header (ftyp) is malformed or missing");
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_DecoderConfig,
                              "HeifReaderImpl::getDecoderParameterSets: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getDecoderParameterSetsForAllLayers(const ImageId& itemId,
                                                                  DecoderConfiguration& decoderInfos) const
    {
        HEIF_ENTRY_TRY
        {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }

        // Multi-layer VVC items get the cached all-layers bundle. For every
        // other item (non-VVC, single-layer VVC, or VVC with multi_layer_flag=0)
        // there is no separate bundle and the regular layer-specific accessor
        // is the right answer, so we just delegate.
        const auto allIter = mImageToAllLayersParameterSetMap.find(itemId);
        if (allIter == mImageToAllLayersParameterSetMap.cend())
        {
            return getDecoderParameterSets(itemId, decoderInfos);
        }

        const DecoderConfigId baseConfigId = allIter->second;
        const auto setIter = mImageItemAllLayersParameterSetMap.find(baseConfigId);
        if (setIter == mImageItemAllLayersParameterSetMap.cend())
        {
            return HEIF_FAIL(ErrorCode::FILE_HEADER_ERROR,
                             ErrorOrigin::Reader_BoxParse,
                             "HeifReaderImpl::getDecoderParameterSetsForAllLayers: file header (ftyp) is malformed or missing");
        }

        const auto& parameterSetMap      = setIter->second;
        decoderInfos.decoderConfigId     = baseConfigId;
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_DecoderConfig,
                              "HeifReaderImpl::getDecoderParameterSetsForAllLayers: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getDecoderParameterSets(const SequenceId& sequenceId,
                                                      const SequenceImageId& itemId,
                                                      DecoderConfiguration& decoderInfos) const
    {
        HEIF_ENTRY_TRY
        {
        ErrorCode error;
        if ((error = isValidSample(sequenceId, itemId)) != ErrorCode::OK)
        {
            return error;
        }

        if (auto* parameterSetMap = getParameterSetMap(sequenceId, itemId))
        {
            // Resolve the config ID from the sample-to-parameter-set map
            decoderInfos.decoderConfigId = [&]() -> DecoderConfigId {
                SegmentId segId;
                if (segmentIdOf(sequenceId, itemId, segId) != ErrorCode::OK)
                    return DecoderConfigId(0);
                const auto& segProps = mFileProperties.segmentPropertiesMap.at(segId);
                const unsigned int idx = itemId.get() - getTrackInfo({segId, sequenceId}).itemIdBase.get();
                const auto it = segProps.sampleToParameterSetMap.find({sequenceId, idx});
                if (it != segProps.sampleToParameterSetMap.end())
                    return DecoderConfigId(it->second.get());
                return DecoderConfigId(0);
            }();
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
        return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_IMAGE_ID,
                         ErrorOrigin::Reader_ItemValidation,
                         "HeifReaderImpl::getDecoderParameterSets: sequence image id not found");
        }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_DecoderConfig,
                              "HeifReaderImpl::getDecoderParameterSets: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getItemProtectionScheme(const ImageId& itemId,
                                                      uint8_t* memoryBuffer,
                                                      uint64_t& memoryBufferSize) const
    {
        HEIF_ENTRY_TRY
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
            return HEIF_FAIL(ErrorCode::UNPROTECTED_ITEM,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItemProtectionScheme: item expected to be protected but is not");
        }

        const auto& meta     = mMetaBox;
        const uint16_t index = static_cast<uint16_t>(
            meta.getItemInfoBox().getItemById(itemId.get()).getItemProtectionIndex() - 1);  // convert index to 0-based

        Vector<std::uint8_t> sinf = meta.getProtectionSchemeInfoBox(index).getData();

        if (sinf.size() > memoryBufferSize)
        {
            memoryBufferSize = static_cast<uint32_t>(sinf.size());
            return HEIF_FAIL(ErrorCode::BUFFER_SIZE_TOO_SMALL,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getItemProtectionScheme: caller-supplied buffer is too small for the data");
        }
        std::memcpy(memoryBuffer, sinf.data(), sinf.size());
        memoryBufferSize = static_cast<uint32_t>(sinf.size());

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getItemProtectionScheme: exception escaped firewall")
    }

    ErrorCode HeifReaderImpl::getTrackInformations(Array<TrackInformation>& trackInfos) const
    {
        HEIF_ENTRY_TRY
        {
        if (isInitialized() != ErrorCode::OK)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Reader_ItemValidation,
                             "HeifReaderImpl::getTrackInformations: called before initialize()");
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
                            convertedCompatibleBrands.emplace_back(compatibleBrand.getUInt32());
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Reader_BoxParse,
                              "HeifReaderImpl::getTrackInformations: exception escaped firewall")
    }

}  // namespace HEIF
