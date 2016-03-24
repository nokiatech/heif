/* Copyright (c) 2015, Nokia Technologies Ltd.
 * All rights reserved.
 *
 * Licensed under the Nokia High-Efficiency Image File Format (HEIF) License (the "License").
 *
 * You may not use the High-Efficiency Image File Format except in compliance with the License.
 * The License accompanies the software and can be found in the file "LICENSE.TXT".
 *
 * You may also obtain the License at:
 * https://nokiatech.github.io/heif/license.txt
 */

#include "hevcimagefilereader.hpp"

#include "auxiliarytypeproperty.hpp"
#include "cleanaperture.hpp"
#include "codingconstraintsbox.hpp"
#include "commondefs.hpp"
#include "hevcconfigurationbox.hpp"
#include "hevcsampleentry.hpp"
#include "idspace.hpp"
#include "imagegrid.hpp"
#include "imageoverlay.hpp"
#include "imagerelativelocationproperty.hpp"
#include "imagerotation.hpp"
#include "log.hpp"
#include "mediadatabox.hpp"
#include "metabox.hpp"
#include "moviebox.hpp"

#include <algorithm>
#include <bitset>
#include <fstream>

using namespace std;

HevcImageFileReader::HevcImageFileReader() :
    mInputStream(nullptr),
    mState(State::UNINITIALIZED)
{
}


void HevcImageFileReader::initialize(const std::string& fileName)
{
    mInputFileStream.open(fileName, ifstream::binary);

    if (!mInputFileStream.is_open())
    {
        throw FileReaderException(FileReaderException::StatusCode::FILE_OPEN_ERROR,
            "Unable to open input file '" + fileName + "'");
    }
    mInputStream = &mInputFileStream;

    readStream();
}


void HevcImageFileReader::initialize(std::istream& inputStream)
{
    mInputStream = &inputStream;
    readStream();
}


void HevcImageFileReader::close()
{
    if (mInputFileStream.is_open())
    {
        mInputFileStream.close();
    }
    mState = State::UNINITIALIZED;
}


std::string HevcImageFileReader::getMajorBrand() const
{
    isInitialized();
    return mFtyp.getMajorBrand();
}


std::uint32_t HevcImageFileReader::getMinorVersion() const
{
    isInitialized();
    return mFtyp.getMinorVersion();
}


std::vector<std::string> HevcImageFileReader::getCompatibleBrands() const
{
    isInitialized();
    return mFtyp.getCompatibleBrands();
}


uint32_t HevcImageFileReader::getDisplayWidth(const uint32_t contextId) const
{
    isInitialized();

    uint32_t width = 0;
    if (getContextType(contextId) == ContextType::TRACK)
    {
        width = mTrackInfo.at(contextId).width;
    }
    else
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    return width;
}


uint32_t HevcImageFileReader::getDisplayHeight(const uint32_t contextId) const
{
    isInitialized();

    uint32_t height = 0;
    if (getContextType(contextId) == ContextType::TRACK)
    {
        height = mTrackInfo.at(contextId).height;
    }
    else
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    return height;
}


uint32_t HevcImageFileReader::getWidth(const uint32_t contextId, const uint32_t itemId) const
{
    isInitialized();

    uint32_t width = 0;
    uint32_t height = 0;
    getImageDimensions(contextId, itemId, width, height);
    return width;
}


uint32_t HevcImageFileReader::getHeight(const uint32_t contextId, const uint32_t itemId) const
{
    isInitialized();

    uint32_t width = 0;
    uint32_t height = 0;
    getImageDimensions(contextId, itemId, width, height);
    return height;
}


std::vector<std::int32_t> HevcImageFileReader::getMatrix() const
{
    return mMatrix;
}

float HevcImageFileReader::getPlaybackDurationInSecs(const uint32_t contextId) const
{
    isInitialized();

    float duration = 0.0;
    switch (getContextType(contextId))
    {
        case ContextType::META:
            if (mMetaBoxInfo.at(contextId).isForcedFpsSet)
            {
                duration = mMetaBoxInfo.at(contextId).displayableMasterImages / mMetaBoxInfo.at(contextId).forcedFps;
            }
            else
            {
                logWarning() << "getPlaybackDurationInSecs() called for meta context, but forced FPS was not set" << endl;
            }
            break;

        case ContextType::TRACK:
            duration = mTrackInfo.at(contextId).duration;
            break;

        case ContextType::FILE:
            // Find maximum duration of contexts
            for (const auto& contextInfo : mContextInfoMap)
            {
                float contextDuration = getPlaybackDurationInSecs(contextInfo.first);
                if (contextDuration > duration)
                {
                    duration = contextDuration;
                }
            }
            break;

        default:
            throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    return duration;
}


void HevcImageFileReader::setForcedTimedPlayback(const uint32_t contextId, const float forcedFps)
{
    isInitialized();

    if (getContextType(contextId) == ContextType::META)
    {
        mMetaBoxInfo.at(contextId).isForcedFpsSet = true;
        mMetaBoxInfo.at(contextId).forcedFps = forcedFps;

        static const double SECONDS_TO_MILLISECONDS = 1000;
        const double millisecondsPerImage = SECONDS_TO_MILLISECONDS / forcedFps;
        double displayTime = 0;

        IdVector masterImages;
        getItemListByType(contextId, "master", masterImages);
        for (auto imageId : masterImages)
        {
            mMetaBoxInfo.at(contextId).imageInfoMap.at(imageId).displayTime = displayTime;

            IdVector thumbIds;
            getReferencedToItemListByType(contextId, imageId, "thmb", thumbIds);
            for (auto thumbId : thumbIds)
            {
                mMetaBoxInfo.at(contextId).imageInfoMap.at(thumbId).displayTime = displayTime;
            }

            displayTime += millisecondsPerImage;
        }
    }
    else
    {
        logWarning() << "setForcedTimedPlayback call for non-meta context ID" << endl;
    }
}


void HevcImageFileReader::setForcedLoopPlayback(const uint32_t contextId, const bool forceLoopPlayback)
{
    isInitialized();

    if (mContextInfoMap.count(contextId) == 0)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID, "setForcedLoopPlayback");
    }
    mContextInfoMap.at(contextId).isForcedLoopPlaybackEnabled = forceLoopPlayback;
}


void HevcImageFileReader::getItemListByType(const uint32_t contextId, const string& itemType, IdVector& itemIds) const
{
    isInitialized();

    itemIds.clear();
    IdVector allItems = getContextItems(contextId);
    if (getContextType(contextId) == ContextType::META)
    {
        for (auto itemId : allItems)
        {
            if (getItemType(contextId, itemId) == itemType)
            {
                itemIds.push_back(itemId);
            }
        }
    }
    else if (getContextType(contextId) == ContextType::TRACK)
    {
        if (itemType == "out_ref")
        {
            for (const auto& sampleProperty : mFileProperties.trackProperties.at(contextId).sampleProperties)
            {
                if (sampleProperty.second.sampleType == SampleType::OUTPUT_REFERENCE_FRAME)
                {
                    itemIds.push_back(sampleProperty.first);
                }
            }
        }
        else if (itemType == "non_out_ref")
        {
            for (const auto& sampleProperty : mFileProperties.trackProperties.at(contextId).sampleProperties)
            {
                if (sampleProperty.second.sampleType == SampleType::NON_OUTPUT_REFERENCE_FRAME)
                {
                    itemIds.push_back(sampleProperty.first);
                }
            }
        }
        else if (itemType == "out_non_ref")
        {
            for (const auto& sampleProperty : mFileProperties.trackProperties.at(contextId).sampleProperties)
            {
                if (sampleProperty.second.sampleType == SampleType::OUTPUT_NON_REFERENCE_FRAME)
                {
                    itemIds.push_back(sampleProperty.first);
                }
            }
        }
        else if (itemType == "display")
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
            std::vector<ItemIdTimestampPair> samplePresentationTimes;
            for (auto sampleId : sampleIds)
            {
                const std::vector<uint32_t> singleSamplePresentationTimes = mTrackInfo.at(contextId).samples.at(sampleId).compositionTimes;
                for (auto sampleTime : singleSamplePresentationTimes)
                {
                    samplePresentationTimes.push_back(std::make_pair(sampleId, sampleTime));
                }
            }

            // Sort to display order using composition times
            std::sort(samplePresentationTimes.begin(), samplePresentationTimes.end(), [&](ItemIdTimestampPair a, ItemIdTimestampPair b)
            {
                return a.second < b.second;
            });

            // Push sample ids to the result
            for (auto pair : samplePresentationTimes)
            {
                itemIds.push_back(pair.first);
            }
        }
        else if (itemType == "samples")
        {
            itemIds = allItems;
        }
        else
        {
            throw FileReaderException(FileReaderException::StatusCode::INVALID_FUNCTION_PARAMETER);
        }
    }
    else
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }
}


string HevcImageFileReader::getItemType(const uint32_t contextId, const uint32_t itemId) const
{
    isInitialized();

    switch (getContextType(contextId))
    {
        case ContextType::META:
        {
            if (mMetaBoxInfo.at(contextId).imageInfoMap.count(itemId))
            {
                return mMetaBoxInfo.at(contextId).imageInfoMap.at(itemId).type;
            }
            else if (mMetaBoxInfo.at(contextId).itemInfoMap.count(itemId))
            {
                return mMetaBoxInfo.at(contextId).itemInfoMap.at(itemId).type;
            }
            break;
        }

        case ContextType::TRACK:
            return mFileProperties.trackProperties.at(contextId).sampleProperties.at(itemId).sampleEntryType;

        default:
            throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    throw FileReaderException(FileReaderException::StatusCode::INVALID_ITEM_ID);
}


void HevcImageFileReader::getReferencedFromItemListByType(const uint32_t contextId, const uint32_t fromItemId,
    const string& referenceType, IdVector& itemIds) const
{
    isInitialized();

    if (getContextType(contextId) != ContextType::META)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    const ItemReferenceBox& itemReferenceBox = mMetaBoxMap.at(contextId).getItemReferenceBox();
    const vector<SingleItemTypeReferenceBox> references = itemReferenceBox.getReferencesOfType(referenceType);

    itemIds.clear();
    for (const auto& reference : references)
    {
        if (reference.getFromItemID() == fromItemId)
        {
            const vector<uint32_t> toIds = reference.getToItemIds();
            itemIds.insert(itemIds.end(), toIds.begin(), toIds.end());
        }
    }
}


void HevcImageFileReader::getReferencedToItemListByType(const uint32_t contextId, const uint32_t toItemId,
    const string& referenceType, IdVector& itemIds) const
{
    isInitialized();

    if (getContextType(contextId) != ContextType::META)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    const ItemReferenceBox& itemReferenceBox = mMetaBoxMap.at(contextId).getItemReferenceBox();
    const vector<SingleItemTypeReferenceBox> references = itemReferenceBox.getReferencesOfType(referenceType);

    itemIds.clear();
    for (const auto& reference : references)
    {
        vector<uint32_t> toIds = reference.getToItemIds();
        for (const auto id : toIds)
        {
            if (id == toItemId)
            {
                itemIds.push_back(reference.getFromItemID());
            }
        }
    }
}


uint32_t HevcImageFileReader::getCoverImageItemId(const uint32_t contextId) const
{
    isInitialized();

    const auto it = mContextInfoMap.find(contextId);
    if (it == mContextInfoMap.end())
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    if (it->second.isCoverImageSet == false)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    return it->second.coverImageId;
}


void HevcImageFileReader::getItemData(const uint32_t contextId, const uint32_t itemId, DataVector& itemData)
{
    isInitialized();
    itemData.clear();

    // read NAL data to bitstream object
    DataVector rawItemData;
    bool processData = false;
    const ContextType contextType = getContextType(contextId);
    switch (contextType)
    {
        case ContextType::META:
        {
            readItem(mMetaBoxMap.at(contextId), itemId, rawItemData);
            const std::string type = getItemType(contextId, itemId);
            if ((not isProtected(contextId, itemId)) && getRawItemType(mMetaBoxMap.at(contextId), itemId) == "hvc1")
            {
                processData = true;
            }
            break;
        }
        case ContextType::TRACK:
        {
            rawItemData = getTrackFrameData(itemId, mTrackInfo.at(contextId));
            processData = true;
            break;
        }
        default:
            throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    if (processData)
    {
        BitStream bitstream(rawItemData);
        while (bitstream.numBytesLeft() > 0)
        {
            const unsigned int nalLength = bitstream.read32Bits();
            const uint8_t firstByte = bitstream.read8Bits();
            HevcNalUnitType naluType = HevcNalUnitType((firstByte >> 1) & 0x3f);

            // Add start code before each NAL unit
            if (itemData.size() == 0 ||
                naluType == HevcNalUnitType::VPS ||
                naluType == HevcNalUnitType::SPS ||
                naluType == HevcNalUnitType::PPS)
            {
                itemData.push_back(0); // Additional zero_byte required before parameter sets and the first NAL unit of the frame
            }
            itemData.push_back(0);
            itemData.push_back(0);
            itemData.push_back(1);

            itemData.push_back(firstByte);
            bitstream.read8BitsArray(itemData, nalLength - 1);
        }
    }
    else
    {
        itemData = rawItemData;
    }
}


HevcImageFileReader::IovlItem HevcImageFileReader::getItemIovl(const std::uint32_t contextId, const std::uint32_t itemId) const
{
    isInitialized();
    if (mMetaBoxInfo.count(contextId) == 0)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }
    if (isProtected(contextId, itemId))
    {
        throw FileReaderException(FileReaderException::StatusCode::PROTECTED_ITEM);
    }
    if (mMetaBoxInfo.at(contextId).iovlItems.count(itemId) == 0)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_ITEM_ID);
    }
    return mMetaBoxInfo.at(contextId).iovlItems.at(itemId);
}


HevcImageFileReader::GridItem HevcImageFileReader::getItemGrid(const std::uint32_t contextId, const std::uint32_t itemId) const
{
    isInitialized();
    if (mMetaBoxInfo.count(contextId) == 0)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }
    if (isProtected(contextId, itemId))
    {
        throw FileReaderException(FileReaderException::StatusCode::PROTECTED_ITEM);
    }
    if (mMetaBoxInfo.at(contextId).gridItems.count(itemId) == 0)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_ITEM_ID);
    }
    return mMetaBoxInfo.at(contextId).gridItems.at(itemId);
}


HevcImageFileReader::AuxProperty HevcImageFileReader::getPropertyAuxc(const std::uint32_t contextId, const std::uint32_t index) const
{
    isInitialized();
    if (mMetaBoxInfo.count(contextId) == 0)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    const auto auxcPtr = mMetaBoxMap.at(contextId).getItemPropertiesBox().getPropertyByIndex<AuxiliaryTypeProperty>(index);
    if (not auxcPtr)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_PROPERTY_INDEX);
    }

    AuxProperty auxc;
    auxc.auxType = auxcPtr->getAuxType();
    auxc.subType = auxcPtr->getAuxSubType();
    return auxc;
}


HevcImageFileReader::IrotProperty HevcImageFileReader::getPropertyIrot(const std::uint32_t contextId, const std::uint32_t index) const
{
    isInitialized();
    if (mMetaBoxInfo.count(contextId) == 0)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    const auto irotBox = mMetaBoxMap.at(contextId).getItemPropertiesBox().getPropertyByIndex<ImageRotation>(index);
    if (not irotBox)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_PROPERTY_INDEX);
    }

    IrotProperty irot;
    irot.rotation = irotBox->getAngle();
    return irot;
}


HevcImageFileReader::RlocProperty HevcImageFileReader::getPropertyRloc(const std::uint32_t contextId, const std::uint32_t index) const
{
    isInitialized();
    if (mMetaBoxInfo.count(contextId) == 0)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    const auto rlocPtr = mMetaBoxMap.at(contextId).getItemPropertiesBox().getPropertyByIndex<ImageRelativeLocationProperty>(index);
    if (not rlocPtr)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_PROPERTY_INDEX);
    }

    RlocProperty rloc;
    rloc.horizontalOffset = rlocPtr->getHorizontalOffset();
    rloc.verticalOffset = rlocPtr->getVerticalOffset();
    return rloc;
}


ImageFileReaderInterface::ClapProperty HevcImageFileReader::getPropertyClap(const std::uint32_t contextId, const std::uint32_t index) const
{
    isInitialized();

    if (getContextType(contextId) ==  ContextType::META)
    {
        const auto clapPtr = mMetaBoxMap.at(contextId).getItemPropertiesBox().getPropertyByIndex<CleanAperture>(index);
        if (not clapPtr)
        {
            throw FileReaderException(FileReaderException::StatusCode::INVALID_PROPERTY_INDEX);
        }

        return makeClap(clapPtr);
    }
    else if (getContextType(contextId) ==  ContextType::TRACK)
    {
        if (mTrackInfo.at(contextId).clapProperties.count(index) == 0)
        {
            throw FileReaderException(FileReaderException::StatusCode::INVALID_SAMPLE_DESCRIPTION_INDEX);
        }

        return mTrackInfo.at(contextId).clapProperties.at(index);
    }

    throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
}


ImageFileReaderInterface::PropertyTypeVector HevcImageFileReader::getItemProperties(const std::uint32_t contextId, const std::uint32_t itemId) const
{
    isInitialized();
    if (mMetaBoxInfo.count(contextId) == 0)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }
    if (mMetaBoxInfo.at(contextId).properties.count(itemId) == 0)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_ITEM_ID);
    }

    return mMetaBoxInfo.at(contextId).properties.at(itemId);
}


void HevcImageFileReader::getItemDataWithDecoderParameters(const uint32_t contextId, const uint32_t itemId,
    DataVector& itemDataWithDecoderParameters)
{
    isInitialized();

    if (isProtected(contextId, itemId))
    {
        throw FileReaderException(FileReaderException::StatusCode::PROTECTED_ITEM);
    }

    itemDataWithDecoderParameters.clear();

    ImageFileReaderInterface::ParameterSetMap parameterSet;
    getDecoderParameterSets(contextId, itemId, parameterSet);
    itemDataWithDecoderParameters.insert(itemDataWithDecoderParameters.end(), parameterSet.at("VPS").begin(), parameterSet.at("VPS").end());
    itemDataWithDecoderParameters.insert(itemDataWithDecoderParameters.end(), parameterSet.at("SPS").begin(), parameterSet.at("SPS").end());
    itemDataWithDecoderParameters.insert(itemDataWithDecoderParameters.end(), parameterSet.at("PPS").begin(), parameterSet.at("PPS").end());

    DataVector itemData;
    getItemData(contextId, itemId, itemData);
    // +1 comes from skipping first zero after decoder parameters
    itemDataWithDecoderParameters.insert(itemDataWithDecoderParameters.end(), itemData.begin() + 1, itemData.end());
}


void HevcImageFileReader::getItemTimestamps(const uint32_t contextId, TimestampMap& timestamps) const
{
    isInitialized();
    timestamps.clear();

    switch (getContextType(contextId))
    {
        case ContextType::TRACK:
        {
            for (const auto& sampleInfo : mTrackInfo.at(contextId).samples)
            {
                for (auto compositionTime : sampleInfo.compositionTimes)
                {
                    timestamps[compositionTime] = sampleInfo.decodingOrder;
                }
            }
            break;
        }

        case ContextType::META:
        {
            if (mMetaBoxInfo.at(contextId).isForcedFpsSet == true)
            {
                for (const auto& imageInfo : mMetaBoxInfo.at(contextId).imageInfoMap)
                {
                    if (imageInfo.second.type == "master")
                    {
                        timestamps[imageInfo.second.displayTime] = imageInfo.first;
                    }
                }
            }
            else
            {
                throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID,
                    "getItemTimestamps: Forced FPS not set for meta context");
            }
            break;
        }

        default:
            throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }
}


void HevcImageFileReader::getTimestampsOfItem(const uint32_t contextId, const uint32_t itemId,
    TimestampVector& timestamps) const
{
    isInitialized();
    timestamps.clear();

    switch (getContextType(contextId))
    {
        case ContextType::TRACK:
        {
            const std::vector<std::uint32_t>& displayTimes = mTrackInfo.at(contextId).samples.at(itemId).compositionTimes;
            timestamps.insert(timestamps.begin(), displayTimes.begin(), displayTimes.end());
            break;
        }

        case ContextType::META:
        {
            if (mMetaBoxInfo.at(contextId).isForcedFpsSet == true)
            {
                timestamps.push_back(mMetaBoxInfo.at(contextId).imageInfoMap.at(itemId).displayTime);
            }
            else
            {
                throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID,
                    "getTimestampsOfItem: Forced FPS not set for meta context");
            }
            break;
        }

        default:
        {
            throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
        }
    }
}


void HevcImageFileReader::getItemsInDecodingOrder(const uint32_t contextId,
    DecodingOrderVector& itemDecodingOrder) const
{
    isInitialized();
    itemDecodingOrder.clear();

    switch (getContextType(contextId))
    {
        case ContextType::TRACK:
            for (const auto& sample : mTrackInfo.at(contextId).samples)
            {
                for (const auto compositionTime : sample.compositionTimes)
                {
                    itemDecodingOrder.push_back(std::make_pair(sample.decodingOrder, compositionTime));
                }
            }
            // Sort using composition times
            std::sort(itemDecodingOrder.begin(), itemDecodingOrder.end(), [&](ItemIdTimestampPair a, ItemIdTimestampPair b)
            {
                return a.second < b.second;
            });

            // Add possible decoding dependencies
            itemDecodingOrder = addDecodingDependencies(contextId, itemDecodingOrder);
            break;

        case ContextType::META:
            if (mMetaBoxInfo.at(contextId).isForcedFpsSet == true)
            {
                itemDecodingOrder.reserve(mMetaBoxInfo.at(contextId).imageInfoMap.size());
                for (const auto& image : mMetaBoxInfo.at(contextId).imageInfoMap)
                {
                    itemDecodingOrder.push_back(std::pair<std::uint32_t, Timestamp>(image.first, image.second.displayTime));
                }
            }
            else
            {
                throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID,
                    "getItemsInDecodingOrder: Forced FPS not set for meta context");
            }
            break;

        default:
            throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }
}


void HevcImageFileReader::getItemDecodeDependencies(const uint32_t contextId, const uint32_t itemId,
    IdVector& dependencies) const
{
    isInitialized();
    dependencies.clear();
    ContextType contextType = getContextType(contextId);
    if (contextType == ContextType::META)
    {
        dependencies.push_back(itemId);
    }
    else if (contextType == ContextType::TRACK)
    {
        const IdVector& decodeDependencies = mTrackInfo.at(contextId).samples.at(itemId).decodeDependencies;
        dependencies.insert(dependencies.begin(), decodeDependencies.begin(), decodeDependencies.end());

        // For I-frames return item id itself.
        if (dependencies.size() == 0)
        {
            dependencies.push_back(itemId);
        }
    }
    else
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }
}


void HevcImageFileReader::getDecoderParameterSets(const uint32_t contextId, const uint32_t itemId,
    ParameterSetMap& parameterSets) const
{
    isInitialized();
    auto iter = mParameterSetMap.find(Id(contextId, itemId));
    if (iter != mParameterSetMap.end())
    {
        parameterSets = iter->second;
        return;
    }

    // Was it an image/sample?
    const Id parameterSetId = mImageToParameterSetMap.at(Id(contextId, itemId));
    iter = mParameterSetMap.find(parameterSetId);
    if (iter != mParameterSetMap.end())
    {
        parameterSets = iter->second;
        return;
    }
    throw FileReaderException(FileReaderException::StatusCode::INVALID_ITEM_ID); // or invalid context...?
}

HevcImageFileReader::DataVector HevcImageFileReader::getItemProtectionScheme(const std::uint32_t contextId, const std::uint32_t itemId) const
{
    if (getContextType(contextId) != ContextType::META)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }
    if (not isProtected(contextId, itemId))
    {
        throw FileReaderException(FileReaderException::StatusCode::UNPROTECTED_ITEM);
    }
    const auto& meta = mMetaBoxMap.at(contextId);
    const auto index = meta.getItemInfoBox().getItemById(itemId).getItemProtectionIndex() - 1; // convert index to 0-based

    return meta.getProtectionSchemeInfoBox(index).getData();
}

/* ********************************************************************** */
/* *********************** Common private methods *********************** */
/* ********************************************************************** */

void HevcImageFileReader::isInitialized() const
{
    if (not (mState == State::INITIALIZING ||
        mState == State::READY))
    {
        throw FileReaderException(FileReaderException::StatusCode::UNINITIALIZED);
    }
}

HevcImageFileReader::ContextType HevcImageFileReader::getContextType(const ContextId id) const
{
    const auto contextInfo = mContextInfoMap.find(id);
    if (contextInfo == mContextInfoMap.end())
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    return contextInfo->second.contextType;
}


void HevcImageFileReader::readStream()
{
    mState = State::INITIALIZING;

    bool ftypFound = false;
    bool metaFound = false;
    bool moovFound = false;

    while (mInputStream->peek() != EOF)
    {
        string boxType;
        uint64_t boxSize = 0;
        BitStream bitstream;
        readBox(bitstream, boxType, boxSize);
        if (boxType == "ftyp")
        {
            bool supportedBrandsFound = false;
            if (ftypFound == true)
            {
                throw FileReaderException(FileReaderException::StatusCode::FILE_READ_ERROR, "Multiple ftyp boxes");
            }
            ftypFound = true;
            FileTypeBox ftyp;
            ftyp.parseBox(bitstream);

            if (ftyp.checkCompatibleBrand("msf1") && ftyp.checkCompatibleBrand("hevc"))
            {
                logInfo() << "This is a track based file." << std::endl;
                supportedBrandsFound = true;
            }
            if (ftyp.checkCompatibleBrand("mif1") && ftyp.checkCompatibleBrand("heic"))
            {
                logInfo() << "This is a meta based file." << std::endl;
                supportedBrandsFound = true;
            }
            if (not supportedBrandsFound)
            {
                throw FileReaderException(FileReaderException::StatusCode::FILE_READ_ERROR, "No compatible brands in file");
            }

            mFtyp = ftyp;
        }
        else if (boxType == "meta")
        {
            if (metaFound == true)
            {
                throw FileReaderException(FileReaderException::StatusCode::FILE_READ_ERROR, "Multiple meta boxes");
            }
            metaFound = true;
            /** @todo The following approach of setting the contextId should be refactored. **/
            const ContextId contextId = IdSpace::getValue(1000);

            MetaBox& metaBox = mMetaBoxMap[contextId]; // Create new MetaBox to map, and take a reference
            metaBox.parseBox(bitstream);

            mFileProperties.rootLevelMetaBoxProperties = extractMetaBoxProperties(metaBox);
            mFileProperties.rootLevelMetaBoxProperties.contextId = contextId;
            mContextInfoMap[contextId] = createContextInfo(mFileProperties.rootLevelMetaBoxProperties);
            mMetaBoxInfo[contextId] = extractItems(metaBox, contextId);
            processHvccProperties(contextId);
            fillImageInfoMap(contextId);
        }
        else if (boxType == "moov")
        {
            if (moovFound == true)
            {
                throw FileReaderException(FileReaderException::StatusCode::FILE_READ_ERROR, "Multiple moov boxes");
            }
            moovFound = true;
            MovieBox moov;
            moov.parseBox(bitstream);
            mFileProperties.moovFeatures = extractMoovProperties(moov);
            mFileProperties.trackProperties = fillTrackProperties(moov);
            mMatrix = moov.getMovieHeaderBox().getMatrix();
        }
        else if (boxType == "mdat")
        {
            // Do nothing, 'mdat' content is handled elsewhere as needed
        }
        else
        {
            logWarning() << "Skipping root level box of unknown type '" << boxType << "'" << endl;
        }
    }

    // peek() sets eof bit for the stream. Clear stream to make sure it is still accessible. seekg() in C++11 should
    // clear stream after eof, but this does not seem to be always happening.
    if ((not mInputStream->good()) && (not mInputStream->eof()))
    {
        throw FileReaderException(FileReaderException::StatusCode::FILE_READ_ERROR);
    }
    mInputStream->clear();

    mFileProperties.fileFeature = getFileFeatures();

    ImageFileReaderInterface::setFileProperties(mFileProperties);
    mState = State::READY;
}


HevcImageFileReader::ContextInfo HevcImageFileReader::createContextInfo(const ImageFileReaderInterface::MetaBoxProperties& metaBoxProperties) const
{
    ContextInfo contextInfo;
    contextInfo.contextType = ContextType::META;

    for (const auto& image : metaBoxProperties.imageFeaturesMap)
    {
        if (image.second.hasFeature(ImageFeature::IsCoverImage) ||
            image.second.hasFeature(ImageFeature::IsPrimaryImage))
        {
            contextInfo.isCoverImageSet = true;
            contextInfo.coverImageId = image.first;
            break;
        }
    }

    return contextInfo;
}


void HevcImageFileReader::fillImageInfoMap(const ContextId contextId)
{
    const ItemInfoBox& itemInfoBox = mMetaBoxMap.at(contextId).getItemInfoBox();

    for (const auto& image : mFileProperties.rootLevelMetaBoxProperties.imageFeaturesMap)
    {
        const ItemId itemId = image.first;
        ImageInfo imageInfo;

        const string rawType = itemInfoBox.getItemById(itemId).getItemType();
        imageInfo.type = rawType;
        if (rawType == "hvc1")
        {
            // Override raw image type in some cases
            if (!doReferencesFromItemIdExist(mMetaBoxMap.at(contextId), itemId, "auxl") &&
                !doReferencesFromItemIdExist(mMetaBoxMap.at(contextId), itemId, "thmb"))
            {
                imageInfo.type = "master";
            }
            else if (image.second.hasFeature(ImageFeature::IsHiddenImage))
            {
                imageInfo.type = "hidden";
            }
            else if (image.second.hasFeature(ImageFeature::IsPreComputedDerivedImage))
            {
                imageInfo.type = "pre-computed";
            }
        }

        // Set dimensions
        const ItemPropertiesBox& iprp = mMetaBoxMap.at(contextId).getItemPropertiesBox();
        const std::uint32_t ispeIndex = iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::ISPE, itemId);
        if (ispeIndex != 0)
        {
            const auto imageSpatialExtentsProperties = iprp.getPropertyByIndex<ImageSpatialExtentsProperty>(ispeIndex - 1);
            imageInfo.height = imageSpatialExtentsProperties->getDisplayHeight();
            imageInfo.width = imageSpatialExtentsProperties->getDisplayWidth();
        }
        else
        {
            logWarning() << "No ImageSpatialExtentsPropertyIndex found for image item id " << itemId << std::endl;
        }

        mMetaBoxInfo.at(contextId).imageInfoMap[image.first] = imageInfo;
    }

    IdVector masterImages;
    getItemListByType(contextId, "master", masterImages);
    mMetaBoxInfo.at(contextId).displayableMasterImages = masterImages.size();
}


HevcImageFileReader::ItemInfoMap HevcImageFileReader::extractItemInfoMap(const MetaBox& metaBox) const
{
    ItemInfoMap itemInfoMap;
    const std::vector<uint32_t> itemIds = metaBox.getItemInfoBox().getItemIds();
    for (const auto itemId : itemIds)
    {
        const ItemInfoEntry& item = metaBox.getItemInfoBox().getItemById(itemId);
        const string type = item.getItemType();
        if (!isImageItemType(type))
        {
            ItemInfo itemInfo;
            itemInfo.type = type;
            itemInfoMap.insert( { itemId, itemInfo } );
        }
    }

    return itemInfoMap;
}

HevcImageFileReader::FileFeature HevcImageFileReader::getFileFeatures() const
{
    FileFeature fileFeature;
    fileFeature.setContextId(IdSpace::getValue());

    if (mMetaBoxInfo.count(mFileProperties.rootLevelMetaBoxProperties.contextId) == 1)
    {
        fileFeature.setFeature(FileFeature::HasRootLevelMetaBox);
        if (mFileProperties.rootLevelMetaBoxProperties.metaBoxFeature.hasFeature(MetaBoxFeature::HasCoverImage))
        {
            fileFeature.setFeature(FileFeature::HasCoverImage);
        }
        IdVector masterImages;
        getItemListByType(mFileProperties.rootLevelMetaBoxProperties.contextId, "master", masterImages);

        if (masterImages.size() == 1)
        {
            fileFeature.setFeature(FileFeature::HasSingleImage);
        }
        else if (masterImages.size() > 1)
        {
            fileFeature.setFeature(FileFeature::HasImageCollection);
        }
    }
    for (const auto& trackProperties : mFileProperties.trackProperties)
    {
        if (trackProperties.second.trackFeature.hasFeature(TrackFeature::IsMasterImageSequence))
        {
            fileFeature.setFeature(FileFeature::HasImageSequence);
        }
        if (trackProperties.second.trackFeature.hasFeature(TrackFeature::HasAlternatives))
        {
            fileFeature.setFeature(FileFeature::HasAlternateTracks);
        }
    }

    /** @todo Add support for:
    hasOtherTimedMedia,
    hasMoovLevelMetaBox */

    return fileFeature;
}

uint64_t HevcImageFileReader::readBytes(std::istream* stream, const unsigned int count)
{
    uint64_t value = 0;
    for (unsigned int i = 0; i < count; ++i)
    {
        value = (value << 8) | stream->get();
        if (!mInputStream->good())
        {
            throw FileReaderException(FileReaderException::StatusCode::FILE_READ_ERROR);
        }
    }

    return value;
}

void HevcImageFileReader::readBox(BitStream& bitstream, std::string& boxType, uint64_t& boxSize)
{
    int startLocation = mInputStream->tellg();

    // Read the 32-bit length field of the box
    boxSize = readBytes(mInputStream, 4);

    // Read the four character string for boxType
    static const size_t TYPE_LENGTH = 4;
    boxType.resize(TYPE_LENGTH);
    mInputStream->read(&boxType[0], TYPE_LENGTH);
    if (not mInputStream->good())
    {
        throw FileReaderException(FileReaderException::StatusCode::FILE_READ_ERROR);
    }

    // Check if 64-bit largesize field is used
    if (boxSize == 1)
    {
        boxSize = readBytes(mInputStream, 8);
    }

    // Seek to box beginning and dump data to bitstream
    std::vector<uint8_t> data(boxSize);
    mInputStream->seekg(startLocation);
    mInputStream->read(reinterpret_cast<char*>(data.data()), boxSize);
    if (not mInputStream->good())
    {
       throw FileReaderException(FileReaderException::StatusCode::FILE_READ_ERROR);
    }
    bitstream.clear();
    bitstream.reset();
    bitstream.write8BitsArray(data, boxSize);
}


void HevcImageFileReader::getImageDimensions(const uint32_t contextId, const uint32_t itemId, uint32_t& width,
    uint32_t& height) const
{
    switch (getContextType(contextId))
    {
        case ContextType::META:
            height = mMetaBoxInfo.at(contextId).imageInfoMap.at(itemId).height;
            width = mMetaBoxInfo.at(contextId).imageInfoMap.at(itemId).width;
            break;

        case ContextType::TRACK:
            height = mTrackInfo.at(contextId).samples.at(itemId).height;
            width = mTrackInfo.at(contextId).samples.at(itemId).width;
            break;

        default:
            throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }
}


HevcImageFileReader::ParameterSetMap HevcImageFileReader::makeDecoderParameterSetMap(const HevcDecoderConfigurationRecord& record) const
{
    vector<uint8_t> sps;
    vector<uint8_t> pps;
    vector<uint8_t> vps;
    record.getOneParameterSet(sps, HevcNalUnitType::SPS);
    record.getOneParameterSet(pps, HevcNalUnitType::PPS);
    record.getOneParameterSet(vps, HevcNalUnitType::VPS);

    ParameterSetMap parameterSetMap;
    parameterSetMap.insert(pair<string, DataVector>("SPS", move(sps)));
    parameterSetMap.insert(pair<string, DataVector>("PPS", move(pps)));
    parameterSetMap.insert(pair<string, DataVector>("VPS", move(vps)));

    return parameterSetMap;
}


HevcImageFileReader::IdVector HevcImageFileReader::getContextItems(ContextId contextId) const
{
    IdVector items;
    if (getContextType(contextId) == ContextType::META)
    {
        for (const auto& imageInfo : mMetaBoxInfo.at(contextId).imageInfoMap)
        {
            items.push_back(imageInfo.first);
        }
    }
    else if (getContextType(contextId) == ContextType::TRACK)
    {
        for (const auto& sampleInfo : mTrackInfo.at(contextId).samples)
        {
            items.push_back(sampleInfo.decodingOrder);
        }
    }
    else
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_CONTEXT_ID);
    }

    return items;
}

bool HevcImageFileReader::isProtected(const std::uint32_t contextId, const std::uint32_t itemId) const
{
    ItemInfoEntry entry;

    if (getContextType(contextId) != ContextType::META)
    {
        return false;
    }

    try
    {
        entry = mMetaBoxMap.at(contextId).getItemInfoBox().getItemById(itemId);
    }
    catch (...)
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_ITEM_ID);
    }

    if (entry.getItemProtectionIndex() > 0)
    {
        return true;
    }
    return false;
}

/* ********************************************************************** */
/* *********************** Meta-specific methods  *********************** */
/* ********************************************************************** */

ImageFileReaderInterface::MetaBoxProperties HevcImageFileReader::extractMetaBoxProperties(const MetaBox& metaBox) const
{
    MetaBoxProperties metaBoxProperties;
    metaBoxProperties.imageFeaturesMap = extractMetaBoxImagePropertiesMap(metaBox);
    metaBoxProperties.itemFeaturesMap = extractMetaBoxItemPropertiesMap(metaBox);
    metaBoxProperties.entityToGroupMaps = extractMetaBoxEntityToGroupMaps(metaBox);
    metaBoxProperties.metaBoxFeature = extractMetaBoxFeatures(metaBoxProperties.imageFeaturesMap, metaBoxProperties.entityToGroupMaps);

    return metaBoxProperties;
}


ImageFileReaderInterface::MetaBoxFeature HevcImageFileReader::extractMetaBoxFeatures(const ImageFeaturesMap& imageFeatures,
    const GroupingMap& groupings) const
{
    /** @todo Add support for features:
     * hasDataInMetaBox ///< i.e. contains 'idat' box. */

    MetaBoxFeature metaBoxFeature;

    if (groupings.size() > 0)
    {
        metaBoxFeature.setFeature(MetaBoxFeature::HasGroupLists);
    }

    if (imageFeatures.size() == 1)
    {
        metaBoxFeature.setFeature(MetaBoxFeature::IsSingleImage);
    }
    else if (imageFeatures.size() > 1)
    {
        metaBoxFeature.setFeature(MetaBoxFeature::IsImageCollection);
    }

    for (const auto& i : imageFeatures)
    {
        const ImageFeature imageFeatures = i.second;

        if (imageFeatures.hasFeature(ImageFeature::IsMasterImage))
        {
            metaBoxFeature.setFeature(MetaBoxFeature::HasMasterImages);
        }
        if (imageFeatures.hasFeature(ImageFeature::IsThumbnailImage))
        {
            metaBoxFeature.setFeature(MetaBoxFeature::HasThumbnails);
        }
        if (imageFeatures.hasFeature(ImageFeature::IsCoverImage))
        {
            metaBoxFeature.setFeature(MetaBoxFeature::HasCoverImage);
        }
        if (imageFeatures.hasFeature(ImageFeature::IsAuxiliaryImage))
        {
            metaBoxFeature.setFeature(MetaBoxFeature::HasAuxiliaryImages);
        }
        if (imageFeatures.hasFeature(ImageFeature::IsDerivedImage))
        {
            metaBoxFeature.setFeature(MetaBoxFeature::HasDerivedImages);
        }
        if (imageFeatures.hasFeature(ImageFeature::IsPreComputedDerivedImage))
        {
            metaBoxFeature.setFeature(MetaBoxFeature::HasPreComputedDerivedImages);
        }
        if (imageFeatures.hasFeature(ImageFeature::IsHiddenImage))
        {
            metaBoxFeature.setFeature(MetaBoxFeature::HasHiddenImages);
        }
    }

    return metaBoxFeature;
}


ImageFileReaderInterface::GroupingMap HevcImageFileReader::extractMetaBoxEntityToGroupMaps(const MetaBox& metaBox) const
{
    GroupingMap groupingMap;

    const std::vector<EntityToGroupBox>& entityToGroupBoxes = metaBox.getGroupsListBox().getEntityToGroupsBoxes();
    for (const auto& box : entityToGroupBoxes)
    {
        const std::string type = box.getType();
        const IdVector ids = box.getEntityIds();
        groupingMap[type].push_back(ids);
    }

    return groupingMap;
}


ImageFileReaderInterface::ImageFeaturesMap HevcImageFileReader::extractMetaBoxImagePropertiesMap(const MetaBox& metaBox) const
{
    ImageFeaturesMap imagePropertiesMap;
    const std::vector<uint32_t> itemIds = metaBox.getItemInfoBox().getItemIds();

    for (const auto itemId : itemIds)
    {
        const ItemInfoEntry& item = metaBox.getItemInfoBox().getItemById(itemId);
        const string type = item.getItemType();

        if (isImageItemType(type))
        {
            ImageFeature imageFeatures;

            if (item.getItemProtectionIndex() > 0)
            {
                imageFeatures.setFeature(ImageFeature::IsProtected);
            }

            if (doReferencesFromItemIdExist(metaBox, itemId, "thmb"))
            {
                imageFeatures.setFeature(ImageFeature::IsThumbnailImage);
            }
            if (doReferencesFromItemIdExist(metaBox, itemId, "auxl"))
            {
                imageFeatures.setFeature(ImageFeature::IsAuxiliaryImage);
            }
            if (doReferencesFromItemIdExist(metaBox, itemId, "base"))
            {
                imageFeatures.setFeature(ImageFeature::IsPreComputedDerivedImage);
            }
            if (doReferencesFromItemIdExist(metaBox, itemId, "dimg"))
            {
                imageFeatures.setFeature(ImageFeature::IsDerivedImage);
            }
            // Is this master image (<=> not a thumb and not an auxiliary image)
            if (imageFeatures.hasFeature(ImageFeature::IsThumbnailImage) == false
                && imageFeatures.hasFeature(ImageFeature::IsAuxiliaryImage) == false)
            {
                imageFeatures.setFeature(ImageFeature::IsMasterImage);
            }

            if (doReferencesToItemIdExist(metaBox, itemId, "thmb"))
            {
                imageFeatures.setFeature(ImageFeature::HasLinkedThumbnails);
            }
            if (doReferencesToItemIdExist(metaBox, itemId, "auxl"))
            {
                imageFeatures.setFeature(ImageFeature::HasLinkedAuxiliaryImage);
            }
            if (doReferencesToItemIdExist(metaBox, itemId, "cdsc"))
            {
                imageFeatures.setFeature(ImageFeature::HasLinkedMetadata);
            }
            if (doReferencesToItemIdExist(metaBox, itemId, "base"))
            {
                imageFeatures.setFeature(ImageFeature::HasLinkedPreComputedDerivedImage);
            }
            if (doReferencesToItemIdExist(metaBox, itemId, "tbas"))
            {
                imageFeatures.setFeature(ImageFeature::HasLinkedTiles);
            }
            if (doReferencesToItemIdExist(metaBox, itemId, "dimg"))
            {
                imageFeatures.setFeature(ImageFeature::HasLinkedDerivedImage);
            }

            if (metaBox.getPrimaryItemBox().getItemId() == itemId)
            {
                imageFeatures.setFeature(ImageFeature::IsPrimaryImage);
                imageFeatures.setFeature(ImageFeature::IsCoverImage);
            }

            static const uint32_t HIDDEN_IMAGE_MASK = 0x1;
            if (item.getFlags() & HIDDEN_IMAGE_MASK)
            {
                imageFeatures.setFeature(ImageFeature::IsHiddenImage);
            }

            imagePropertiesMap[itemId] = imageFeatures;
        }
    }

    return imagePropertiesMap;
}


ImageFileReaderInterface::ItemFeaturesMap HevcImageFileReader::extractMetaBoxItemPropertiesMap(const MetaBox& metaBox) const
{
    ItemFeaturesMap itemFeaturesMap;
    const std::vector<uint32_t> itemIds = metaBox.getItemInfoBox().getItemIds();

    for (const auto itemId : itemIds)
    {
        const ItemInfoEntry& item = metaBox.getItemInfoBox().getItemById(itemId);
        const string type = item.getItemType();
        if (type != "hvc1")
        {
            ItemFeature itemFeature;

            if (item.getItemProtectionIndex() > 0)
            {
                itemFeature.setFeature(ItemFeature::IsProtected);
            }

            if (doReferencesFromItemIdExist(metaBox, itemId, "cdsc"))
            {
                itemFeature.setFeature(ItemFeature::IsMetadataItem);
            }

            if (type == "Exif")
            {
                itemFeature.setFeature(ItemFeature::IsExifItem);
            }
            else if (type == "mime")
            {
                itemFeature.setFeature(ItemFeature::IsMimeItem);
            }
            else if (type == "hvt1")
            {
                itemFeature.setFeature(ItemFeature::IsTileImageItem);
            }

            itemFeaturesMap[itemId] = itemFeature;
        }
    }

    return itemFeaturesMap;
}


HevcImageFileReader::Properties HevcImageFileReader::processItemProperties(const ContextId contextId) const
{
    Properties propertyMap;

    const ItemPropertiesBox& iprp = mMetaBoxMap.at(contextId).getItemPropertiesBox();
    const std::vector<uint32_t> itemIds = mMetaBoxMap.at(contextId).getItemInfoBox().getItemIds();
    for (const auto itemId : itemIds)
    {
        const ItemInfoEntry& item = mMetaBoxMap.at(contextId).getItemInfoBox().getItemById(itemId);

        ItemPropertiesBox::PropertyInfos propertyVector = iprp.getItemProperties(itemId);

        // The following loop copies item property information to interface. Data structures are essentially identical
        // in ItemPropertiesBox and the reader API, but it is not desirable to expose ItemPropertiesBox in the API,
        // or include reader interface as part of ItemPropertiesBox.
        PropertyTypeVector propertyTypeVector;
        for (const auto& property : propertyVector)
        {
            static const std::map<ItemPropertiesBox::PropertyType, ImageFileReaderInterface::ItemPropertyType> ENUM_MAPPING =
            {
                { ItemPropertiesBox::PropertyType::UNKNOWN, ImageFileReaderInterface::ItemPropertyType::UNKNOWN },
                { ItemPropertiesBox::PropertyType::AUXC, ImageFileReaderInterface::ItemPropertyType::AUXC },
                { ItemPropertiesBox::PropertyType::CLAP, ImageFileReaderInterface::ItemPropertyType::CLAP },
                { ItemPropertiesBox::PropertyType::HVCC, ImageFileReaderInterface::ItemPropertyType::HVCC },
                { ItemPropertiesBox::PropertyType::IROT, ImageFileReaderInterface::ItemPropertyType::IROT },
                { ItemPropertiesBox::PropertyType::ISPE, ImageFileReaderInterface::ItemPropertyType::ISPE },
                { ItemPropertiesBox::PropertyType::RLOC, ImageFileReaderInterface::ItemPropertyType::RLOC }
            };

            ItemPropertyInfo info;
            info.essential = property.essential;
            info.index = property.index;
            info.type = ENUM_MAPPING.at(property.type);
            propertyTypeVector.push_back(info);
        }
        propertyMap.insert(std::make_pair(itemId, propertyTypeVector));
    }

    return propertyMap;
}

void HevcImageFileReader::processHvccProperties(const ContextId contextId)
{
    // Decoder configuration gets special handling because that information is accessed by the reader
    // implementation itself.

    const ItemPropertiesBox& iprp = mMetaBoxMap.at(contextId).getItemPropertiesBox();

    for (const auto& imageProperties : mFileProperties.rootLevelMetaBoxProperties.imageFeaturesMap)
    {
        const ItemId imageId = imageProperties.first;
        const std::uint32_t hvccIndex = iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::HVCC, imageId);
        if (hvccIndex != 0)
        {
            mImageToParameterSetMap[Id(contextId, imageId)] = Id(contextId, hvccIndex);
            const HevcDecoderConfigurationRecord record =
                iprp.getPropertyByIndex<HevcConfigurationBox>(hvccIndex - 1)->getConfiguration();
            const ParameterSetMap parameterSetMap = makeDecoderParameterSetMap(record);
            mParameterSetMap[Id(contextId, hvccIndex)] = parameterSetMap;
        }
    }
}


HevcImageFileReader::MetaBoxInfo HevcImageFileReader::extractItems(const MetaBox& metaBox, const std::uint32_t contextId) const
{
    MetaBoxInfo metaBoxInfo;
    const std::vector<uint32_t> itemIds = metaBox.getItemInfoBox().getItemIds();
    for (const auto itemId : itemIds)
    {
        const ItemInfoEntry& item = metaBox.getItemInfoBox().getItemById(itemId);
        const std::string type = item.getItemType();
        if (type == "grid" || type == "iovl")
        {
            if (isProtected(contextId, itemId))
            {
                continue;
            }
            std::vector<std::uint8_t> data = loadItemData(metaBox, itemId);
            BitStream bitstream(data);

            if (type == "grid")
            {
                const ImageGrid imageGrid = parseImageGrid(bitstream);
                GridItem grid;
                grid.columnsMinusOne = imageGrid.columnsMinusOne;
                grid.rowsMinusOne = imageGrid.rowsMinusOne;
                grid.outputWidth = imageGrid.outputWidth;
                grid.outputHeight = imageGrid.outputHeight;
                getReferencedFromItemListByType(contextId, itemId, "dimg", grid.itemIds);
                metaBoxInfo.gridItems.insert( { itemId, grid} );
            }
            if (type == "iovl")
            {
                ImageOverlay imageOverlay = parseImageOverlay(bitstream);
                IovlItem iovl;
                iovl.canvasFillValue.push_back(imageOverlay.canvasFillValueR);
                iovl.canvasFillValue.push_back(imageOverlay.canvasFillValueG);
                iovl.canvasFillValue.push_back(imageOverlay.canvasFillValueB);
                iovl.canvasFillValue.push_back(imageOverlay.canvasFillValueA);
                iovl.outputWidth = imageOverlay.outputWidth;
                iovl.outputHeight = imageOverlay.outputHeight;
                const std::vector<ImageOverlay::Offset>& offsets = imageOverlay.offsets;
                for (const auto& offset : offsets)
                {
                    iovl.offsets.push_back( { offset.horizontalOffset, offset.verticalOffset } );
                }
                getReferencedFromItemListByType(contextId, itemId, "dimg", iovl.itemIds);
                metaBoxInfo.iovlItems.insert( { itemId, iovl} );
            }
        }
    }

    metaBoxInfo.properties = processItemProperties(contextId);
    metaBoxInfo.itemInfoMap = extractItemInfoMap(metaBox);

    return metaBoxInfo;
}

std::vector<std::uint8_t> HevcImageFileReader::loadItemData(const MetaBox& metaBox, const ItemId itemId) const
{
    const streampos oldPosition = mInputStream->tellg();
    std::vector<uint8_t> data;
    readItem(metaBox, itemId, data);
    mInputStream->seekg(oldPosition);
    return std::move(data);
}

/** @details Method can not handle items located in different files. Therefore also zero length items are not supported.
 *  @todo Add support for item_offset construction method. */
void HevcImageFileReader::readItem(const MetaBox& metaBox, const ItemId itemId, std::vector<std::uint8_t>& data) const
{
    const ItemLocationBox& iloc = metaBox.getItemLocationBox();
    const ItemLocation& itemLocation = iloc.getItemLocationForID(itemId);
    const ItemLocation::ConstructionMethod constructionMethod = itemLocation.getConstructionMethod();
    const ExtentList& extentList = itemLocation.getExtentList();
    const unsigned int baseOffset = itemLocation.getBaseOffset();
    const unsigned int version = iloc.getVersion();
    unsigned int itemLength = 0;

    if (extentList.size() == 0)
    {
        throw FileReaderException(FileReaderException::StatusCode::FILE_READ_ERROR, "No extents given for an item.");
    }

    for (const auto& extent : extentList)
    {
        itemLength += extent.mExtentLength;
    }

    if (version == 0 ||
        (version == 1 && constructionMethod == ItemLocation::ConstructionMethod::FILE_OFFSET))
    {
        data.resize(itemLength);
        char* dataPtr = reinterpret_cast<char*>(data.data());
        for (const auto& extent : extentList)
        {
            const unsigned int offset = baseOffset + extent.mExtentOffset;
            mInputStream->seekg(offset);
            if (!mInputStream->good())
            {
                throw FileReaderException(FileReaderException::StatusCode::FILE_READ_ERROR,
                    "Error while reading item data. Faulty offset in iloc, or damaged file?");
            }
            mInputStream->read(dataPtr, extent.mExtentLength);
            dataPtr += extent.mExtentLength;
        }
    }
    else if (version == 1 && (constructionMethod == ItemLocation::ConstructionMethod::IDAT_OFFSET))
    {
        data.clear();
        for (const auto& extent : extentList)
        {
            const size_t offset = baseOffset + extent.mExtentOffset;
            metaBox.getItemDataBox().read(data, offset, extent.mExtentLength);
        }
    }
    else if (constructionMethod == ItemLocation::ConstructionMethod::ITEM_OFFSET)
    {
        throw FileReaderException(FileReaderException::StatusCode::NOT_APPLICABLE,
            "Item construction_method item_offset is not currently supported.");
    }
    else
    {
        throw FileReaderException(FileReaderException::StatusCode::FILE_READ_ERROR,
            "Unknown ItemLocationBox version and/or construction method.");
    }

    if (not mInputStream->good())
    {
        throw FileReaderException(FileReaderException::StatusCode::FILE_READ_ERROR, "Error while reading item data.");
    }
}


/* *********************************************************************** */
/* *********************** Track-specific methods  *********************** */
/* *********************************************************************** */

HevcImageFileReader::TrackPropertiesMap HevcImageFileReader::fillTrackProperties(MovieBox& moovBox)
{
    TrackPropertiesMap trackPropertiesMap;

    vector<TrackBox*> trackBoxes = moovBox.getTrackBoxes();
    for (auto trackBox : trackBoxes)
    {
        TrackProperties trackProperties;
        TrackInfo trackInfo = extractTrackInfo(trackBox, moovBox);

        trackProperties.trackId = trackBox->getTrackHeaderBox().getTrackID();

        /** @todo Fill MetaBoxProperties trackLevelMetaBoxProperties when track level meta box is supported. **/
        /** @todo Fill TypeToIdsMap trackGroupIds; ///< <group_type, track IDs> ... coming from Track Group Box 'trgr'
         *        when Track Group Box is supported. **/

        trackProperties.sampleProperties = makeSamplePropertiesMap(trackBox);
        trackInfo.samples = makeSampleInfoVector(trackBox, trackInfo.pMap);
        mTrackInfo[trackProperties.trackId] = trackInfo;

        fillHevcSampleEntryMap(trackBox);

        trackProperties.trackFeature = getTrackFeatures(trackBox);
        trackProperties.referenceTrackIds = getReferenceTrackIds(trackBox);
        trackProperties.sampleGroupIds = getSampleGroupIds(trackBox);
        trackProperties.alternateTrackIds = getAlternateTrackIds(trackBox, moovBox);
        trackProperties.alternateGroupId = trackBox->getTrackHeaderBox().getAlternateGroup();

        trackPropertiesMap[trackProperties.trackId] = trackProperties;

        ContextInfo contextInfo;
        contextInfo.contextType = ContextType::TRACK;
        mContextInfoMap[trackProperties.trackId] = contextInfo;
    }

    // Some TrackFeatures are easiest to set after first round of properties have already been filled.
    for (auto& trackProperties : trackPropertiesMap)
    {
        const uint32_t trackId = trackProperties.second.trackId;
        if (isAnyLinkedToWithType(trackPropertiesMap, trackId, "thmb") == true)
        {
            trackProperties.second.trackFeature.setFeature(TrackFeature::HasLinkedThumbnailImageSequence);
        }
        if (isAnyLinkedToWithType(trackPropertiesMap, trackId, "auxl") == true)
        {
            trackProperties.second.trackFeature.setFeature(TrackFeature::HasLinkedAuxiliaryImageSequence);
        }
    }

    return trackPropertiesMap;
}


HevcImageFileReader::IdVector HevcImageFileReader::getAlternateTrackIds(TrackBox* trackBox, MovieBox& moovBox) const
{
    IdVector trackIds;
    const std::uint16_t alternateGroupId = trackBox->getTrackHeaderBox().getAlternateGroup();

    if (alternateGroupId == 0)
    {
        return trackIds;
    }

    const std::uint32_t trackId = trackBox->getTrackHeaderBox().getTrackID();
    std::vector<TrackBox*> trackBoxes = moovBox.getTrackBoxes();
    for (auto trackbox : trackBoxes)
    {
        const uint32_t alternateTrackId = trackbox->getTrackHeaderBox().getTrackID();
        if ((trackId != alternateTrackId) &&
            (alternateGroupId == trackbox->getTrackHeaderBox().getAlternateGroup()))
        {

            trackIds.push_back(alternateTrackId);
        }
    }

    return trackIds;
}


bool HevcImageFileReader::isAnyLinkedToWithType(const TrackPropertiesMap& trackPropertiesMap,
    const std::uint32_t trackId, const std::string& referenceType) const
{
    for (const auto& trackProperties : trackPropertiesMap)
    {
        for (const auto& reference : trackProperties.second.referenceTrackIds)
        {
            if (reference.first == referenceType)
            {
                if (std::find(reference.second.begin(), reference.second.end(), trackId) != reference.second.end())
                {
                    return true;
                }
            }
        }
    }

    return false;
}


/**
hasTrackLevelMetaBox,     ///< @todo Implement
isTimedMetadataTrack,     ///< @todo Not implemented yet (subtitles for example)
hasCoverImage,            ///< @todo Implement later
hasLinkedTimedMetaDataTrack, ///< @todo Implement later
hasSampletoItemGrouping,     ///< @todo SampleToItemGroupingBox
hasLinkedContentDescription, ///< @todo Track reference of type 'cdsc' to reference image sequence's metadata as a timed metadata track
hasExifSampleEntry, ///< @todo From SampleEntryBox, not implemented yet
hasXmlSampleEntry,  ///< @todo From SampleEntryBox, not implemented yet
 */
HevcImageFileReader::TrackFeature HevcImageFileReader::getTrackFeatures(TrackBox* trackBox) const
{
    TrackFeature trackFeature;

    TrackHeaderBox tkhdBox = trackBox->getTrackHeaderBox();
    HandlerBox& handlerBox = trackBox->getMediaBox().getHandlerBox();
    SampleTableBox& stblBox = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
    SampleDescriptionBox& stsdBox = stblBox.getSampleDescriptionBox();

    if (handlerBox.getHandlerType() == "pict")
    {
        std::bitset<24> flags(tkhdBox.getFlags());
        if (flags.test(0))
        {
            trackFeature.setFeature(TrackFeature::IsEnabled);
        }
        if (flags.test(1))
        {
            trackFeature.setFeature(TrackFeature::IsInMovie);
        }
        if (flags.test(2))
        {
            trackFeature.setFeature(TrackFeature::IsInPreview);
        }

        if (tkhdBox.getAlternateGroup() != 0)
        {
            trackFeature.setFeature(TrackFeature::HasAlternatives);
        }

        if (trackBox->getHasTrackReferences() == false)
        {
            // isMasterImageSequence,    ///< sample entry box has 'pict', and is not referencing any another track ('thmb')
            trackFeature.setFeature(TrackFeature::IsMasterImageSequence);
        }
        else
        {
            if (trackBox->getTrackReferenceBox().isReferenceTypePresent("thmb"))
            {
                // isThumbnailImageSequence, ///< is referencing an another track 'thmb' tref
                trackFeature.setFeature(TrackFeature::IsThumbnailImageSequence);
            }

            if (trackBox->getTrackReferenceBox().isReferenceTypePresent("auxl"))
            {
                // isAuxiliaryImageSequence, ///< is referencing an another track 'auxl' tref
                trackFeature.setFeature(TrackFeature::IsAuxiliaryImageSequence);
            }
        }

        // hasCodingConstraints - from Coding Constraints Box in HevcSampleEntry
        const std::vector<HevcSampleEntry*> sampleEntries = stsdBox.getSampleEntries<HevcSampleEntry>();
        for (const auto& sampleEntry : sampleEntries)
        {
            if (sampleEntry->isCodingConstraintsBoxPresent() == true)
            {
                trackFeature.setFeature(TrackFeature::HasCodingConstraints);
                break;
            }
        }

        // hasSampleGroups
        if (stblBox.getSampleToGroupBoxes().size() != 0)
        {
            trackFeature.setFeature(TrackFeature::HasSampleGroups);
        }

        std::shared_ptr<const EditBox> editBox = stblBox.getEditBox();
        if (editBox)
        {
            const EditListBox* editListBox = editBox->getEditListBox();
            if (editListBox)
            {
                trackFeature.setFeature(TrackFeature::HasEditList);
                // Edit list box flag == 1 determines infinite looping
                if (editListBox->getFlags() == 1 && tkhdBox.getDuration() == 0xffffffff)
                {
                    trackFeature.setFeature(TrackFeature::HasInfiniteLoopPlayback);
                }
            }
        }
    }

    return trackFeature;
}


HevcImageFileReader::TypeToIdsMap HevcImageFileReader::getReferenceTrackIds(TrackBox* trackBox) const
{
    std::map<std::string, IdVector> trackReferenceMap;

    const std::vector<TrackReferenceTypeBox>& trackReferenceTypeBoxes = trackBox->getTrackReferenceBox().getTrefTypeBoxes();
    for (const auto& trackReferenceTypeBox : trackReferenceTypeBoxes)
    {
        trackReferenceMap[trackReferenceTypeBox.getType()] = trackReferenceTypeBox.getTrackIds();
    }

    return trackReferenceMap;
}


HevcImageFileReader::TypeToIdsMap HevcImageFileReader::getSampleGroupIds(TrackBox* trackBox) const
{
    std::map<std::string, IdVector> sampleGroupIdsMap;

    SampleTableBox& stblBox = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
    const std::vector<SampleToGroupBox> sampleToGroupBoxes = stblBox.getSampleToGroupBoxes();
    for (const auto& sampleToGroupBox : sampleToGroupBoxes)
    {
        const unsigned int numberOfSamples = sampleToGroupBox.getNumberOfSamples();
        IdVector sampleIds(numberOfSamples);
        for (unsigned int i = 0; i < numberOfSamples; ++i)
        {
            if (sampleToGroupBox.getSampleGroupDescriptionIndex(i) != 0)
            {
                sampleIds.push_back(i);
            }
        }
        sampleGroupIdsMap[sampleToGroupBox.getGroupingType()] = sampleIds;
    }

    return sampleGroupIdsMap;
}


HevcImageFileReader::TrackInfo HevcImageFileReader::extractTrackInfo(TrackBox* trackBox, MovieBox& moovBox) const
{
    TrackInfo trackInfo;
    SampleTableBox& stblBox = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
    TrackHeaderBox& trackHeaderBox = trackBox->getTrackHeaderBox();
    const TimeToSampleBox& timeToSampleBox = stblBox.getTimeToSampleBox();
    std::shared_ptr<const CompositionOffsetBox> compositionOffsetBox = stblBox.getCompositionOffsetBox();

    trackInfo.width = trackHeaderBox.getWidth() >> 16;
    trackInfo.height = trackHeaderBox.getHeight() >> 16;

    const uint32_t timescale = moovBox.getMovieHeaderBox().getTimeScale(); // Number of time units that pass in a second
    const uint32_t duration = trackHeaderBox.getDuration(); // Duration is in timescale units

    std::shared_ptr<const EditBox> editBox = stblBox.getEditBox();
    DecodePts decodePts;
    decodePts.loadBox(&timeToSampleBox);
    decodePts.loadBox(compositionOffsetBox.get());
    if (editBox)
    {
        const EditListBox* editListBox = editBox->getEditListBox();
        decodePts.loadBox(editListBox);
    }
    decodePts.unravel();

    static const uint32_t DURATION_FROM_EDIT_LIST = 0xffffffff;
    if (duration == DURATION_FROM_EDIT_LIST)
    {
        trackInfo.pMap = decodePts.getTime(timescale);

        const double maxTimestampInSeconds = trackInfo.pMap.rbegin()->first / 1000.0;
        const uint32_t numberOfSamples = trackInfo.pMap.size();
        double secondsPerSample = 1; // Default in case there is only 1 sample

        // Last sample length is not presented by timestamps
        if (numberOfSamples > 1)
        {
            secondsPerSample = maxTimestampInSeconds / (numberOfSamples - 1);
        }

        // Duration is just an approximation, as we can't really know the length of the last sample.
        trackInfo.duration = maxTimestampInSeconds + secondsPerSample;
    }
    else
    {
        trackInfo.duration = (duration / static_cast<double>(timescale));
        trackInfo.pMap = decodePts.getTime(timescale, trackInfo.duration * 1000);
    }
    return trackInfo;
}


HevcImageFileReader::SampleInfoVector HevcImageFileReader::makeSampleInfoVector(TrackBox* trackBox,
    const DecodePts::PMap& pMap) const
{
    SampleInfoVector sampleInfoVector;

    SampleTableBox& stblBox = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
    SampleDescriptionBox& stsdBox = stblBox.getSampleDescriptionBox();
    SampleToChunkBox& stscBox = stblBox.getSampleToChunkBox();
    ChunkOffsetBox& stcoBox = stblBox.getChunkOffsetBox();
    SampleSizeBox& stszBox = stblBox.getSampleSizeBox();

    const vector<uint32_t> sampleSizeEntries = stszBox.getEntrySize();
    const vector<uint64_t> chunkOffsets = stcoBox.getChunkOffsets();
    const std::vector<SampleToGroupBox> sampleToGroupBoxes = stblBox.getSampleToGroupBoxes();

    const unsigned int sampleCount = stszBox.getSampleCount();

    std::uint32_t previousChunkIndex = 0; // Index is 1-based so 0 will not be used.
    for (uint32_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
    {
        SampleInfo sampleInfo;

        // Set basic sample information
        sampleInfo.decodingOrder = sampleIndex;
        sampleInfo.dataLength = sampleSizeEntries.at(sampleIndex);

        const std::uint32_t chunkIndex = stscBox.getSampleChunkIndex(sampleIndex);
        if (chunkIndex != previousChunkIndex)
        {
            sampleInfo.dataOffset = chunkOffsets.at(chunkIndex - 1);
            previousChunkIndex = chunkIndex;
        }
        else
        {
            sampleInfo.dataOffset = sampleInfoVector.back().dataOffset + sampleInfoVector.back().dataLength;
        }

        // Set dimensions
        const uint32_t sampleDescriptionIndex = stscBox.getSampleDescriptionIndex(sampleIndex);
        const HevcSampleEntry* hevcSampleEntry = stsdBox.getSampleEntry<HevcSampleEntry>(sampleDescriptionIndex);
        sampleInfo.width = hevcSampleEntry->getWidth();
        sampleInfo.height = hevcSampleEntry->getHeight();

        // Figure out decode dependencies
        for (const auto& sampleToGroupBox : sampleToGroupBoxes)
        {
            const uint32_t sampleGroupDescriptionIndex = sampleToGroupBox.getSampleGroupDescriptionIndex(sampleIndex);
            if (sampleGroupDescriptionIndex != 0)
            {
                const string groupingType =  sampleToGroupBox.getGroupingType();

                if (groupingType == "refs")
                {
                    const SampleGroupDescriptionBox* sgdb = stblBox.getSampleGroupDescriptionBox(groupingType);
                    sampleInfo.decodeDependencies = getSampleDirectDependencies(sampleIndex, sgdb, sampleToGroupBox);
                }
            }
        }

        sampleInfoVector.push_back(sampleInfo);
    }

    // Set composition times from Pmap, which considers also edit lists
    for (const auto& pair : pMap)
    {
        sampleInfoVector.at(pair.second).compositionTimes.push_back(pair.first);
    }

    return sampleInfoVector;
}


HevcImageFileReader::SamplePropertiesMap HevcImageFileReader::makeSamplePropertiesMap(TrackBox* trackBox)
{
    SamplePropertiesMap samplePropertiesMap;

    SampleTableBox& stblBox = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
    SampleDescriptionBox& stsdBox = stblBox.getSampleDescriptionBox();
    SampleToChunkBox& stscBox = stblBox.getSampleToChunkBox();
    SampleSizeBox& stszBox = stblBox.getSampleSizeBox();

    const std::vector<SampleToGroupBox> sampleToGroupBoxes = stblBox.getSampleToGroupBoxes();

    const unsigned int sampleCount = stszBox.getSampleCount();
    for (uint32_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
    {
        SampleProperties sampleProperties;
        sampleProperties.sampleId = sampleIndex;
        sampleProperties.sampleDescriptionIndex = stscBox.getSampleDescriptionIndex(sampleIndex);
        VisualSampleEntryBox* sampleEntry = stsdBox.getSampleEntry<VisualSampleEntryBox>(sampleProperties.sampleDescriptionIndex);
        sampleProperties.sampleEntryType = sampleEntry->getType();

        // Get CodingConstraintsBox from HEVC SampleEntryType boxes
        CodingConstraintsBox* ccst = sampleEntry->getCodingConstraintsBox();

        if (!ccst)
        {
            throw FileReaderException(FileReaderException::StatusCode::UNINITIALIZED, "Failed to read coding constraints from '" + sampleProperties.sampleEntryType + "' sample entry type box.");
        }

        // Store values from CodingConstraintsBox
        if (ccst->getAllRefPicsIntra() == true)
        {
            sampleProperties.codingConstraints.setFeature(CodingConstraints::IsAllReferencePicturesIntra);
        }
        if (ccst->getIntraPredUsed() == true)
        {
            sampleProperties.codingConstraints.setFeature(CodingConstraints::IsIntraPredictionUsed);
        }

        sampleProperties.hasClap = sampleEntry->getClap();

        // By default, set as output reference frame (groupings can change this later)
        // By definition, an output reference frame MAY be used as a reference for other samples.
        sampleProperties.sampleType = OUTPUT_REFERENCE_FRAME;

        for (const auto& sampleToGroupBox : sampleToGroupBoxes)
        {
            const uint32_t sampleGroupDescriptionIndex = sampleToGroupBox.getSampleGroupDescriptionIndex(sampleIndex);
            if (sampleGroupDescriptionIndex != 0)
            {
                const string groupingType = sampleToGroupBox.getGroupingType();
                sampleProperties.sampleGroupsMap[groupingType] = sampleGroupDescriptionIndex;

                if (groupingType == "refs")
                {
                    // If there is a "refs" sample grouping, then there is a chance that the sample is an output non-reference frame
                    // If there is no "refs" sample group, then all samples are potentially output reference frames.
                    const DirectReferenceSampleListEntry* entry =
                        dynamic_cast<const DirectReferenceSampleListEntry*>(stblBox.getSampleGroupDescriptionBox(groupingType)->getEntry(sampleGroupDescriptionIndex));

                    if (entry->getSampleId() == 0)
                    {
                        // This means that there is at least one reference sample of this sample, hence it is a non-reference frame.
                        sampleProperties.sampleType = OUTPUT_NON_REFERENCE_FRAME;
                    }
                }
            }
        }
        /** @todo Set the rest of sampleProperties.sampleTypes when needed:
         *        NON_OUTPUT_REFERENCE_FRAME from ctts box (composition time negative, meaning -2^31) **/

        samplePropertiesMap[sampleIndex] = sampleProperties;

        const uint32_t trackId = trackBox->getTrackHeaderBox().getTrackID();
        mImageToParameterSetMap[Id(trackId, sampleIndex)] = Id(trackId, sampleProperties.sampleDescriptionIndex);
    }

    return samplePropertiesMap;
}


std::vector<HevcImageFileReader::ItemId> HevcImageFileReader::getSampleDirectDependencies(const ItemId itemId,
    const SampleGroupDescriptionBox* sgpd, const SampleToGroupBox& sampleToGroupBox) const
{
    const uint32_t index = sampleToGroupBox.getSampleGroupDescriptionIndex(itemId);
    const DirectReferenceSampleListEntry* entry = dynamic_cast<const DirectReferenceSampleListEntry*>(sgpd->getEntry(index));

    const std::vector<ItemId> sampleIds = entry->getDirectReferenceSampleIds();

    // IDs from entry are not sample IDs (in item decoding order), they have be mapped to sample ids
    std::vector<ItemId> ids;
    for (auto entryId : sampleIds)
    {
        const uint32_t entryIndex = sgpd->getEntryIndexOfSampleId(entryId);
        ids.push_back(sampleToGroupBox.getSampleId(entryIndex));
    }

    return ids;
}


ImageFileReaderInterface::MoovProperties HevcImageFileReader::extractMoovProperties(const MovieBox& moovBox) const
{
    MoovProperties moovProperties;
    /** @todo Set MoovFeatures: hasCoverImage as well **/

    if (moovBox.isMetaBoxPresent())
    {
        moovProperties.moovFeature.setFeature(MoovFeature::HasMoovLevelMetaBox);
    }

    return moovProperties;
}


HevcImageFileReader::DataVector HevcImageFileReader::getTrackFrameData(const unsigned int frameIndex, const TrackInfo& trackInfo) const
{
    DataVector frameData;

    // The requested frame should be one that is available
    if (frameIndex >= trackInfo.samples.size())
    {
        throw FileReaderException(FileReaderException::StatusCode::INVALID_FUNCTION_PARAMETER , "Requested frame out of index");
    }

    const unsigned int sampleLength = trackInfo.samples.at(frameIndex).dataLength;
    frameData.resize(sampleLength);
    mInputStream->seekg(trackInfo.samples.at(frameIndex).dataOffset, ios::beg);
    mInputStream->read(reinterpret_cast<char*>(&frameData[0]), sampleLength);
    if (!mInputStream->good())
    {
        throw FileReaderException(FileReaderException::StatusCode::FILE_READ_ERROR);
    }

    return frameData;
}


void HevcImageFileReader::fillHevcSampleEntryMap(TrackBox* trackBox)
{
    const uint32_t trackId = trackBox->getTrackHeaderBox().getTrackID();
    SampleDescriptionBox& stsdBox = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox().getSampleDescriptionBox();

    const std::vector<HevcSampleEntry*> sampleEntries = stsdBox.getSampleEntries<HevcSampleEntry>();
    unsigned int index = 1;
    for (auto& entry : sampleEntries)
    {
        ParameterSetMap parameterSetMap =
            makeDecoderParameterSetMap(entry->getHevcConfigurationBox().getConfiguration());
        mParameterSetMap[Id(trackId, index)] = parameterSetMap;

        const CleanAperture* clapBox = entry->getClap();
        if (clapBox != nullptr)
        {
            mTrackInfo.at(trackId).clapProperties.insert(std::make_pair(index, makeClap(clapBox)));
        }
        ++index;
    }
}

HevcImageFileReader::DecodingOrderVector HevcImageFileReader::addDecodingDependencies(const uint32_t contextId,
    const DecodingOrderVector& itemDecodingOrder) const
{
    DecodingOrderVector output;

    // Add dependencies for each sample
    for (const auto& entry : itemDecodingOrder)
    {
        IdVector dependencies;
        getItemDecodeDependencies(contextId, entry.first, dependencies);

        // If only one dependency is given, it is the sample itself, so it is not added.
        if (not (dependencies.size() == 1 && dependencies.at(0) == entry.first))
        {
            for (const auto& sampleId : dependencies)
            {
                output.push_back(std::pair<ItemId, Timestamp>(sampleId, 0xffffffff));
            }
        }
        output.push_back(entry);
    }

    return output;
}

/* *********************************************************************** */
/* ************************* Helper functions **************************** */
/* *********************************************************************** */

std::string getRawItemType(const MetaBox& metaBox, const uint32_t itemId)
{
    return metaBox.getItemInfoBox().getItemById(itemId).getItemType();
}

ImageFileReaderInterface::ClapProperty makeClap(const CleanAperture* clapBox)
{
    if (clapBox == nullptr)
    {
        throw ImageFileReaderInterface::FileReaderException(ImageFileReaderInterface::FileReaderException::StatusCode::NOT_APPLICABLE);
    }

    ImageFileReaderInterface::ClapProperty clap;
    clap.widthN = clapBox->getWidth().numerator;
    clap.widthD = clapBox->getWidth().denominator;
    clap.heightN = clapBox->getHeight().numerator;
    clap.heightD = clapBox->getHeight().denominator;
    clap.horizontalOffsetN = clapBox->getHorizOffset().numerator;
    clap.horizontalOffsetD = clapBox->getHorizOffset().denominator;
    clap.verticalOffsetN = clapBox->getVertOffset().numerator;
    clap.verticalOffsetD = clapBox->getVertOffset().denominator;

    return clap;
}


bool isImageItemType(const std::string& type)
{
    static const std::set<std::string> IMAGE_TYPES =
    {
        "hvc1", "grid", "iovl", "iden"
    };

    return (IMAGE_TYPES.find(type) != IMAGE_TYPES.end());
}



bool doReferencesFromItemIdExist(const MetaBox& metaBox, const uint32_t itemId, const string& referenceType)
{
    const vector<SingleItemTypeReferenceBox> references = metaBox.getItemReferenceBox().getReferencesOfType(referenceType);
    for (const auto& singleItemTypeReferenceBox : references)
    {
        if (singleItemTypeReferenceBox.getFromItemID() == itemId)
        {
            return true;
        }
    }
    return false;
}


bool doReferencesToItemIdExist(const MetaBox& metaBox, const uint32_t itemId, const string& referenceType)
{
    const vector<SingleItemTypeReferenceBox> references = metaBox.getItemReferenceBox().getReferencesOfType(referenceType);
    for (const auto& singleItemTypeReferenceBox : references)
    {
        const vector<uint32_t> toIds = singleItemTypeReferenceBox.getToItemIds();
        const auto id = find(toIds.cbegin(), toIds.cend(), itemId);
        if (id != toIds.cend())
        {
            return true;
        }
    }
    return false;
}
