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

#include "heifreaderimpl.hpp"

#include "auxiliarytypeproperty.hpp"
#include "avcconfigurationbox.hpp"
#include "avcdecoderconfigrecord.hpp"
#include "avcsampleentry.hpp"
#include "buildinfo.hpp"
#include "cleanaperturebox.hpp"
#include "codingconstraintsbox.hpp"
#include "colourinformationbox.hpp"
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
#include "pixelaspectratiobox.hpp"
#include "pixelinformationproperty.hpp"
#include "rawpropertybox.hpp"

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>

using namespace std;

namespace HEIF
{
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


    ErrorCode HeifReaderImpl::getDisplayWidth(const SequenceId& sequenceId, uint32_t& displayWidth) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }
        displayWidth = mTrackInfo.at(sequenceId.get()).width;
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getDisplayHeight(const SequenceId& sequenceId, uint32_t& displayHeight) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }
        displayHeight = mTrackInfo.at(sequenceId.get()).height;
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getWidth(const ImageId& itemId, uint32_t& width) const
    {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }
        const auto contextId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        width                = mMetaBoxInfo.at(contextId).itemInfoMap.at(itemId.get()).width;
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getWidth(const SequenceId& sequenceId,
                                       const SequenceImageId& itemId,
                                       uint32_t& width) const
    {
        ErrorCode error;
        if ((error = isValidSample(sequenceId, itemId)) != ErrorCode::OK)
        {
            return error;
        }
        width = mTrackInfo.at(sequenceId).samples.at(itemId.get()).width;
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getHeight(const ImageId& itemId, uint32_t& height) const
    {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }
        const auto contextId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        height               = mMetaBoxInfo.at(contextId).itemInfoMap.at(itemId.get()).height;
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getHeight(const SequenceId& sequenceId,
                                        const SequenceImageId& itemId,
                                        uint32_t& height) const
    {
        ErrorCode error;
        if ((error = isValidSample(sequenceId, itemId)) != ErrorCode::OK)
        {
            return error;
        }

        height = mTrackInfo.at(sequenceId.get()).samples.at(itemId.get()).height;
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getMatrix(Array<std::int32_t>& matrix) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (mMatrix.size() == 0)
        {
            return ErrorCode::NOT_APPLICABLE;
        }

        matrix = makeArray<int32_t>(mMatrix);
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
        matrix = makeArray<int32_t>(mTrackInfo.at(sequenceId.get()).matrix);
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getPlaybackDurationInSecs(const SequenceId& sequenceId, double& durationInSecs) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }
        durationInSecs = mTrackInfo.at(sequenceId).duration;
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getMasterImages(Array<ImageId>& itemIds) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        const auto contextId = mFileProperties.rootLevelMetaBoxProperties.contextId;

        IdVector allItems;
        getCollectionItems(allItems);

        IdVector masterItemIds;
        for (auto itemId : allItems)
        {
            const auto rawType =
                mMetaBoxMap.at(contextId).getItemInfoBox().getItemById(itemId).getItemType().getString();
            if ((rawType == "avc1") || (rawType == "hvc1"))
            {
                // A master image is an image that is not an auxiliary image or a thumbnail image.
                if (!doReferencesFromItemIdExist(mMetaBoxMap.at(contextId), itemId, "auxl") &&
                    !doReferencesFromItemIdExist(mMetaBoxMap.at(contextId), itemId, "thmb"))
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

        IdVector allItems;
        getSequenceItems(sequenceId, allItems);
        if (mFileProperties.trackProperties.at(sequenceId.get())
                .trackFeature.hasFeature(TrackFeatureEnum::IsMasterImageSequence))
        {
            itemIds = makeArray<SequenceImageId>(allItems);
        }

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getItemListByType(const FourCC& itemType, Array<ImageId>& itemIds) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        Vector<uint32_t> itemIdVector;
        IdVector allItems;
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
                                                const TrackSampleType& itemType,
                                                Array<SequenceImageId>& itemIds) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }

        const auto contextId = sequenceId.get();
        Vector<uint32_t> itemIdVector;
        IdVector allItems;
        if (getSequenceItems(sequenceId, allItems) != ErrorCode::OK)
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }

        if (itemType == TrackSampleType::out_ref)
        {
            for (const auto& sampleProperty : mFileProperties.trackProperties.at(contextId).sampleProperties)
            {
                if (sampleProperty.second.sampleType == SampleType::OUTPUT_REFERENCE_FRAME)
                {
                    itemIdVector.push_back(sampleProperty.first);
                }
            }
        }
        else if (itemType == TrackSampleType::non_out_ref)
        {
            for (const auto& sampleProperty : mFileProperties.trackProperties.at(contextId).sampleProperties)
            {
                if (sampleProperty.second.sampleType == SampleType::NON_OUTPUT_REFERENCE_FRAME)
                {
                    itemIdVector.push_back(sampleProperty.first);
                }
            }
        }
        else if (itemType == TrackSampleType::out_non_ref)
        {
            for (const auto& sampleProperty : mFileProperties.trackProperties.at(contextId).sampleProperties)
            {
                if (sampleProperty.second.sampleType == SampleType::OUTPUT_NON_REFERENCE_FRAME)
                {
                    itemIdVector.push_back(sampleProperty.first);
                }
            }
        }
        else if (itemType == TrackSampleType::display)
        {
            IdVector sampleIds;
            // Collect frames to display
            for (const auto& sampleProperty : mFileProperties.trackProperties.at(contextId).sampleProperties)
            {
                if (sampleProperty.second.sampleType == SampleType::OUTPUT_NON_REFERENCE_FRAME ||
                    sampleProperty.second.sampleType == SampleType::OUTPUT_REFERENCE_FRAME)
                {
                    sampleIds.push_back(sampleProperty.first);
                }
            }

            // Collect every presentation time stamp of every sample
            Vector<ItemIdTimestampPair> samplePresentationTimes;
            for (auto sampleId : sampleIds)
            {
                const Vector<int64_t>& singleSamplePresentationTimes =
                    mTrackInfo.at(contextId).samples.at(sampleId).compositionTimes;
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
                itemIdVector.push_back(pair.first);
            }
        }
        else if (itemType == TrackSampleType::samples)
        {
            itemIdVector = allItems;
        }
        else
        {
            return ErrorCode::INVALID_FUNCTION_PARAMETER;
        }

        itemIds = makeArray<SequenceImageId>(itemIdVector);

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getItemType(const ImageId& itemId, FourCC& type) const
    {
        ErrorCode error;
        if ((error = isInitialized()) != ErrorCode::OK)
        {
            return error;
        }

        const auto rootMetaId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        if (mMetaBoxInfo.at(rootMetaId).itemInfoMap.count(itemId.get()))
        {
            type = mMetaBoxInfo.at(rootMetaId).itemInfoMap.at(itemId.get()).type.getUInt32();
            return ErrorCode::OK;
        }

        return ErrorCode::INVALID_ITEM_ID;
    }

    ErrorCode HeifReaderImpl::getItemType(const SequenceId& sequenceId,
                                          const SequenceImageId& sequenceImageId,
                                          FourCC& type) const
    {
        ErrorCode error;
        if ((error = isValidSample(sequenceId, sequenceImageId)) != ErrorCode::OK)
        {
            return error;
        }

        type = mFileProperties.trackProperties.at(sequenceId)
                   .sampleProperties.at(sequenceImageId.get())
                   .sampleEntryType.value;
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

        const ItemReferenceBox& itemReferenceBox =
            mMetaBoxMap.at(mFileProperties.rootLevelMetaBoxProperties.contextId).getItemReferenceBox();
        const Vector<SingleItemTypeReferenceBox>& references =
            itemReferenceBox.getReferencesOfType(FourCCInt(referenceType.value));

        IdVector itemIdVector;
        for (const auto& reference : references)
        {
            if (reference.getFromItemID() == fromItemId.get())
            {
                const Vector<uint32_t>& toIds = reference.getToItemIds();
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

        const ItemReferenceBox& itemReferenceBox =
            mMetaBoxMap.at(mFileProperties.rootLevelMetaBoxProperties.contextId).getItemReferenceBox();
        const Vector<SingleItemTypeReferenceBox>& references =
            itemReferenceBox.getReferencesOfType(FourCCInt(referenceType.value));

        IdVector itemIdVector;
        for (const auto& reference : references)
        {
            const auto toIds = reference.getToItemIds();
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
        if (mIsPrimaryItemSet == false)
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
        const auto contextId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        std::uint64_t itemLength(0);

        try
        {
            List<ItemId> pastReferences;
            error = getItemLength(mMetaBoxMap.at(contextId), itemId.get(), itemLength, pastReferences);
            if (error != ErrorCode::OK)
            {
                return error;
            }
            else if (static_cast<int64_t>(itemLength) > mIo.size)
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
            error = readItem(mMetaBoxMap.at(contextId), itemId.get(), memoryBuffer, memoryBufferSize);
            if (error != ErrorCode::OK)
            {
                return error;
            }
        }
        catch (const Exception& exc)
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
        error = getRawItemType(mMetaBoxMap.at(contextId), itemId.get(), rawType);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        bool isProtected;
        error = getProtection(itemId.get(), isProtected);
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
            error = getDecoderCodeType(itemId.get(), codeType);
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
                                          bool bytestreamHeaders) const
    {
        ErrorCode error;
        if ((error = isValidSample(sequenceId, itemId)) != ErrorCode::OK)
        {
            return error;
        }

        // read NAL data to bitstream object
        error = getTrackFrameData(itemId.get(), mTrackInfo.at(sequenceId.get()), memoryBuffer, memoryBufferSize);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        // Process bitstream by codec
        FourCC codeType;
        error = getDecoderCodeType(sequenceId, itemId.get(), codeType);
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
            else
            {
                // Code type not supported
                return ErrorCode::UNSUPPORTED_CODE_TYPE;
            }
        }
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getItem(const ImageId& itemId, Overlay& iovlItem) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        const auto contextId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        bool isProtected;
        ErrorCode error = getProtection(itemId.get(), isProtected);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        if (isProtected)
        {
            return ErrorCode::PROTECTED_ITEM;
        }
        if (mMetaBoxInfo.at(contextId).iovlItems.count(itemId.get()) == 0)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }
        iovlItem = mMetaBoxInfo.at(contextId).iovlItems.at(itemId.get());
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getItem(const ImageId& itemId, Grid& gridItem) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        const auto contextId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        bool isProtected;
        ErrorCode error = getProtection(itemId.get(), isProtected);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        if (isProtected)
        {
            return ErrorCode::PROTECTED_ITEM;
        }
        if (mMetaBoxInfo.at(contextId).gridItems.count(itemId.get()) == 0)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }
        gridItem = mMetaBoxInfo.at(contextId).gridItems.at(itemId.get());
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, AuxiliaryType& auxc) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }
        const auto metaBoxId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        const auto* boxPtr   = mMetaBoxMap.at(metaBoxId).getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "auxC")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        auto auxcPtr = static_cast<const AuxiliaryTypeProperty*>(boxPtr);

        const String auxType = auxcPtr->getAuxType();
        auxc.auxType         = makeArray<char>(auxType);

        const auto subType = auxcPtr->getAuxSubType();
        auxc.subType       = Array<uint8_t>(subType.size());
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
        if (mTrackInfo.at(sequenceId).auxiProperties.count(index) == 0)
        {
            return ErrorCode::INVALID_SAMPLE_DESCRIPTION_INDEX;
        }

        auxc.auxType = mTrackInfo.at(sequenceId).auxiProperties.at(index).auxType;
        auxc.subType = {};  /// @todo Should read also SEI messages from sample entry to here.
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, Mirror& imir) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        const auto metaBoxId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        auto boxPtr          = mMetaBoxMap.at(metaBoxId).getItemPropertiesBox().getPropertyByIndex(index.get());
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

        const auto metaBoxId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        auto boxPtr          = mMetaBoxMap.at(metaBoxId).getItemPropertiesBox().getPropertyByIndex(index.get());
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

        const auto metaBoxId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        auto boxPtr          = mMetaBoxMap.at(metaBoxId).getItemPropertiesBox().getPropertyByIndex(index.get());
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
        const auto metaBoxId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        auto boxPtr          = mMetaBoxMap.at(metaBoxId).getItemPropertiesBox().getPropertyByIndex(index.get());
        if (boxPtr->getType() != "pixi")
        {
            return ErrorCode::INVALID_PROPERTY_INDEX;
        }
        auto pixiPtr = static_cast<const PixelInformationProperty*>(boxPtr);

        const auto bitsPerChannels = pixiPtr->getBitsPerChannels();
        pixi.bitsPerChannel        = makeArray<uint8_t>(bitsPerChannels);
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProperty(const PropertyId& index, ColourInformation& colr) const
    {
        if (isInitialized() != ErrorCode::OK)
        {
            return ErrorCode::UNINITIALIZED;
        }

        const auto metaBoxId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        auto boxPtr          = mMetaBoxMap.at(metaBoxId).getItemPropertiesBox().getPropertyByIndex(index.get());
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

        const auto metaBoxId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        auto boxPtr          = mMetaBoxMap.at(metaBoxId).getItemPropertiesBox().getPropertyByIndex(index.get());
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

        const auto metaBoxId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        auto boxPtr          = mMetaBoxMap.at(metaBoxId).getItemPropertiesBox().getPropertyByIndex(index.get());
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

        const auto metaBoxId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        const auto data      = mMetaBoxMap.at(metaBoxId).getItemPropertiesBox().getPropertyDataByIndex(index.get());
        if (data.size() == 0)
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
        if (mTrackInfo.at(sequenceId.get()).clapProperties.count(index) == 0)
        {
            return ErrorCode::INVALID_SAMPLE_DESCRIPTION_INDEX;
        }

        clap = mTrackInfo.at(sequenceId.get()).clapProperties.at(index);

        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getItemProperties(const ImageId& itemId, Array<ItemPropertyInfo>& propertyTypes) const
    {
        ErrorCode error;
        if ((error = isValidItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }

        const auto contextId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        if (mMetaBoxInfo.at(contextId).properties.count(itemId.get()) == 0)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }

        propertyTypes = makeArray<ItemPropertyInfo>(mMetaBoxInfo.at(contextId).properties.at(itemId.get()));
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

        bool isProtected;
        error = getProtection(itemId.get(), isProtected);
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
        error = getItemData(itemId.get(), &memoryBuffer[parameterSize], itemSize);
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
                                                               uint64_t& memoryBufferSize) const
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

        Vector<TimestampIDPair> timestampVector;
        for (size_t i = 0; i < mTrackInfo.at(sequenceId.get()).samples.size(); i++)
        {
            if (mFileProperties.trackProperties.at(sequenceId)
                    .sampleProperties.at(static_cast<uint32_t>(i))
                    .sampleType != SampleType::NON_OUTPUT_REFERENCE_FRAME)
            {
                const auto& sampleInfo = mTrackInfo.at(sequenceId).samples.at(i);
                for (auto compositionTime : sampleInfo.compositionTimes)
                {
                    timestampVector.push_back(TimestampIDPair{compositionTime, sampleInfo.decodingOrder});
                }
            }
        }

        // Sort using composition times
        std::sort(timestampVector.begin(), timestampVector.end(),
                  [&](TimestampIDPair a, TimestampIDPair b) { return a.timeStamp < b.timeStamp; });

        timestamps = makeArray<TimestampIDPair>(timestampVector);
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getTimestampsOfItem(const SequenceId& sequenceId,
                                                  const SequenceImageId& itemId,
                                                  Array<int64_t>& timestamps) const
    {
        ErrorCode error;
        if ((error = isValidSample(sequenceId, itemId)) != ErrorCode::OK)
        {
            return error;
        }

        Vector<std::int64_t> timestampVector;

        if (mFileProperties.trackProperties.at(sequenceId).sampleProperties.count(itemId.get()))
        {
            if (mFileProperties.trackProperties.at(sequenceId).sampleProperties.at(itemId.get()).sampleType !=
                SampleType::NON_OUTPUT_REFERENCE_FRAME)
            {
                const Vector<std::int64_t>& displayTimes =
                    mTrackInfo.at(sequenceId).samples.at(itemId.get()).compositionTimes;
                timestampVector.insert(timestampVector.begin(), displayTimes.begin(), displayTimes.end());
            }
        }
        else
        {
            return ErrorCode::INVALID_ITEM_ID;
        }

        timestamps = makeArray<int64_t>(timestampVector);
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

        Vector<TimestampIDPair> decodingOrderVector;
        for (const auto& sample : mTrackInfo.at(sequenceId.get()).samples)
        {
            for (const auto compositionTime : sample.compositionTimes)
            {
                decodingOrderVector.push_back(TimestampIDPair{compositionTime, sample.decodingOrder});
            }
        }
        // Sort using composition times
        std::sort(decodingOrderVector.begin(), decodingOrderVector.end(),
                  [&](TimestampIDPair a, TimestampIDPair b) { return a.timeStamp < b.timeStamp; });

        // Add possible decoding dependencies
        decodingOrderVector = addDecodingDependencies(sequenceId.get(), decodingOrderVector);

        decodingOrder = makeArray<TimestampIDPair>(decodingOrderVector);
        return ErrorCode::OK;
    }


    ErrorCode HeifReaderImpl::getDecodeDependencies(const SequenceId& sequenceId,
                                                    const SequenceImageId& itemId,
                                                    Array<SequenceImageId>& dependencies) const
    {
        ErrorCode error;
        if ((error = isValidSample(sequenceId, itemId)) != ErrorCode::OK)
        {
            return error;
        }

        IdVector dependencyVector;
        const IdVector& decodeDependencies =
            mTrackInfo.at(sequenceId.get()).samples.at(itemId.get()).decodeDependencies;
        dependencyVector.insert(dependencyVector.begin(), decodeDependencies.cbegin(), decodeDependencies.cend());

        // For I-frames return item id itself.
        if (dependencyVector.size() == 0)
        {
            dependencyVector.push_back(itemId.get());
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

        const auto contextId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        const auto iter      = mDecoderCodeTypeMap.find(Id(contextId, itemId.get()));
        if (iter != mDecoderCodeTypeMap.end())
        {
            type = FourCC(iter->second.getUInt32());
            return ErrorCode::OK;
        }

        return ErrorCode::INVALID_ITEM_ID;
    }

    ErrorCode HeifReaderImpl::getDecoderCodeType(const SequenceId& sequenceId,
                                                 const SequenceImageId& itemId,
                                                 FourCC& type) const
    {
        ErrorCode error;
        if ((error = isValidSample(sequenceId, itemId)) != ErrorCode::OK)
        {
            return error;
        }

        const auto iter = mDecoderCodeTypeMap.find(Id(sequenceId.get(), itemId.get()));
        if (iter != mDecoderCodeTypeMap.end())
        {
            type = FourCC(iter->second.getUInt32());
            return ErrorCode::OK;
        }

        return ErrorCode::INVALID_ITEM_ID;
    }

    ErrorCode HeifReaderImpl::getDecoderParameterSets(const ImageId& itemId, DecoderConfiguration& decoderInfos) const
    {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }

        const Id imageFullId = Id(mFileProperties.rootLevelMetaBoxProperties.contextId, itemId.get());
        if (mImageToParameterSetMap.count(imageFullId) == 0)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }
        Id parameterSetId = mImageToParameterSetMap.at(imageFullId);

        const auto iter = mParameterSetMap.find(parameterSetId);
        if (iter == mParameterSetMap.cend())
        {
            return ErrorCode::FILE_HEADER_ERROR;
        }

        const auto& parameterSetMap      = iter->second;
        decoderInfos.decoderConfigId     = parameterSetId.second;
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

        const Id imageFullId = Id(sequenceId.get(), itemId.get());
        if (mImageToParameterSetMap.count(imageFullId) == 0)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }
        Id parameterSetId = mImageToParameterSetMap.at(imageFullId);

        const auto iter = mParameterSetMap.find(parameterSetId);
        if (iter == mParameterSetMap.cend())
        {
            return ErrorCode::INVALID_ITEM_ID;
        }
        assert(iter != mParameterSetMap.cend());

        const auto& parameterSetMap      = iter->second;
        decoderInfos.decoderConfigId     = parameterSetId.second;
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

    ErrorCode HeifReaderImpl::getItemProtectionScheme(const ImageId& itemId,
                                                      uint8_t* memoryBuffer,
                                                      uint64_t& memoryBufferSize) const
    {
        ErrorCode error;
        if ((error = isValidImageItem(itemId)) != ErrorCode::OK)
        {
            return error;
        }

        const auto contextId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        bool isProtected;
        error = getProtection(itemId.get(), isProtected);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        if (!isProtected)
        {
            return ErrorCode::UNPROTECTED_ITEM;
        }

        const auto& meta     = mMetaBoxMap.at(contextId);
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
}  // namespace HEIF
