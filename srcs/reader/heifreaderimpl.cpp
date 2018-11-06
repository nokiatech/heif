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
#include "mp4audiosampleentrybox.hpp"
#include "sampletometadataitementry.hpp"
#include "visualequivalenceentry.hpp"

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <fstream>

using namespace std;

namespace HEIF
{
    namespace
    {
        Array<FourCCToIds> mapToArray(const TypeToIdsMap& typeToIdsMap)
        {
            Array<FourCCToIds> array(typeToIdsMap.size());
            unsigned int i = 0;
            for (auto iter = typeToIdsMap.cbegin(); iter != typeToIdsMap.cend(); ++iter)
            {
                array[i].type     = iter->first;
                array[i].trackIds = makeArray<SequenceId>(iter->second);
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
        , mIo()
        , mFileProperties()
        , mDecoderCodeTypeMap()
        , mParameterSetMap()
        , mImageToParameterSetMap()
        , mIsPrimaryItemSet(false)
        , mPrimaryItemId(0)
        , mFtyp()
        , mFileInformation()
        , mMetaBoxMap()
        , mMetaBoxInfo()
        , mMatrix()
        , mTrackInfo()
    {
    }

    ErrorCode HeifReaderImpl::initialize(const char* fileName)
    {
        ErrorCode rc;
        mIo.fileStream.reset(openFile(fileName));
        rc = initialize(&*mIo.fileStream);
        if (rc != ErrorCode::OK)
        {
            mIo.fileStream.reset();
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

        auto& io  = mIo;
        io.stream = std::move(internalStream);
        io.size   = io.stream->size();

        reset();

        try
        {
            ErrorCode error = readStream();
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

        mFileInformation.rootMetaBoxInformation =
            convertRootMetaBoxInformation(mFileProperties.rootLevelMetaBoxProperties);
        mFileInformation.trackInformation = convertTrackInformation(mFileProperties.trackProperties);
        mFileInformation.features         = mFileProperties.fileFeature.getFeatureMask();
        mFileInformation.movieTimescale   = mFileProperties.movieTimescale;

        return ErrorCode::OK;
    }

    void HeifReaderImpl::close()
    {
        reset();
    }

    HEIF_DLL_PUBLIC ErrorCode Reader::SetCustomAllocator(CustomAllocator* customAllocator)
    {
        if (!setCustomAllocator(customAllocator))
        {
            return ErrorCode::ALLOCATOR_ALREADY_SET;
        }
        else
        {
            return ErrorCode::OK;
        }
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

        mFileProperties = {};
        mDecoderCodeTypeMap.clear();
        mParameterSetMap.clear();
        mImageToParameterSetMap.clear();
        mIsPrimaryItemSet = false;
        mPrimaryItemId    = 0;
        mFtyp             = {};
        mFileInformation  = {};
        mMetaBoxMap.clear();
        mMetaBoxInfo.clear();
        mMatrix.clear();
        mTrackInfo.clear();
    }

    MetaBoxInformation HeifReaderImpl::convertRootMetaBoxInformation(const MetaBoxProperties& metaboxProperties) const
    {
        MetaBoxInformation metaBoxInformation;
        metaBoxInformation.features = metaboxProperties.metaBoxFeature.getFeatureMask();

        Array<ItemInformation> itemInformation(metaboxProperties.itemFeaturesMap.size());
        size_t i = 0;
        for (const auto& item : metaboxProperties.itemFeaturesMap)
        {
            const ItemInfo& iteminfo =
                mMetaBoxInfo.at(mFileProperties.rootLevelMetaBoxProperties.contextId).itemInfoMap.at(item.first.get());

            itemInformation[i].itemId                      = item.first.get();
            itemInformation[i].type                        = iteminfo.type.getUInt32();
            itemInformation[i].description.name            = makeArray<char>(iteminfo.name);
            itemInformation[i].description.contentType     = makeArray<char>(iteminfo.contentType);
            itemInformation[i].description.contentEncoding = makeArray<char>(iteminfo.contentEncoding);
            itemInformation[i].features                    = item.second.getFeatureMask();

            List<ItemId> pastReferences;
            if ((getItemLength(mMetaBoxMap.at(mFileProperties.rootLevelMetaBoxProperties.contextId), item.first.get(),
                               itemInformation[i].size, pastReferences) != ErrorCode::OK) ||
                static_cast<int64_t>(itemInformation[i].size) > mIo.size)
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

    Array<TrackInformation> HeifReaderImpl::convertTrackInformation(const TrackPropertiesMap& trackPropertiesMap) const
    {
        Array<TrackInformation> trackInformation(trackPropertiesMap.size());

        unsigned int i = 0;
        for (const auto& trackInfo : trackPropertiesMap)
        {
            const TrackProperties& trackProperties = trackInfo.second;
            trackInformation[i].trackId            = trackInfo.first;
            trackInformation[i].alternateGroupId   = trackProperties.alternateGroupId;
            trackInformation[i].features           = trackProperties.trackFeature.getFeatureMask();
            trackInformation[i].alternateTrackIds  = makeArray<SequenceId>(trackProperties.alternateTrackIds);
            trackInformation[i].referenceTrackIds  = mapToArray(trackProperties.referenceTrackIds);
            trackInformation[i].sampleGroups       = trackProperties.groupedSamples;
            trackInformation[i].equivalences       = trackProperties.equivalences;
            trackInformation[i].metadatas          = trackProperties.metadatas;
            trackInformation[i].referenceSamples   = trackProperties.referenceSamples;
            trackInformation[i].maxSampleSize      = trackProperties.maxSampleSize;
            trackInformation[i].timeScale          = trackProperties.timeScale;
            trackInformation[i].editList           = trackProperties.editList;

            Array<SampleInformation> sampleInfo(trackProperties.sampleProperties.size());
            unsigned int j = 0;
            for (const auto& entry : trackProperties.sampleProperties)
            {
                const SampleProperties& sampleProp      = entry.second;
                sampleInfo[j].sampleId                  = sampleProp.sampleId;
                sampleInfo[j].sampleEntryType           = sampleProp.sampleEntryType;
                sampleInfo[j].sampleDescriptionIndex    = sampleProp.sampleDescriptionIndex;
                sampleInfo[j].sampleType                = sampleProp.sampleType;
                sampleInfo[j].sampleDurationTS          = sampleProp.sampleDurationTS;
                sampleInfo[j].sampleCompositionOffsetTs = sampleProp.sampleCompositionOffsetTs;
                sampleInfo[j].hasClap                   = sampleProp.hasClap;
                sampleInfo[j].hasAuxi                   = sampleProp.hasAuxi;
                sampleInfo[j].codingConstraints         = sampleProp.codingConstraints;
                sampleInfo[j].size                      = sampleProp.size;
                ++j;
            }
            trackInformation[i].sampleProperties = sampleInfo;
            ++i;
        }

        return trackInformation;
    }

    ErrorCode HeifReaderImpl::readStream()
    {
        State prevState = mState;
        mState          = State::INITIALIZING;
        if (mIo.stream->peekEof())
        {
            mState = prevState;
            mIo.stream.reset();
            return ErrorCode::FILE_READ_ERROR;
        }
        mIo.size = mIo.stream->size();

        bool ftypFound = false;
        bool moovFound = false;
        bool metaFound = false;

        ErrorCode error = ErrorCode::OK;
        if (mIo.stream->peekEof())
        {
            error = ErrorCode::FILE_HEADER_ERROR;
        }

        try
        {
            while ((error == ErrorCode::OK) && !mIo.stream->peekEof())
            {
                String boxType;
                std::int64_t boxSize = 0;
                BitStream bitstream;
                error = readBoxParameters(boxType, boxSize);
                if (error == ErrorCode::OK)
                {
                    if (boxType == "ftyp")
                    {
                        if (ftypFound == true)
                        {
                            error = ErrorCode::FILE_READ_ERROR;
                            break;
                        }
                        ftypFound = true;

                        error = readBox(bitstream);
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
                                logInfo()
                                    << "No supported brands found. Trying to continue parsing anyway." << std::endl;
                            }
                            else
                            {
                                logInfo() << "Compatible brands found:" << std::endl;
                                for (auto brand : supportedBrands)
                                {
                                    logInfo() << " " << brand << std::endl;
                                }
                            }
                            mFtyp = ftyp;
                        }
                    }
                    else if (boxType == "meta")
                    {
                        if (metaFound == true)
                        {
                            return ErrorCode::FILE_READ_ERROR;  // Multiple root-level meta boxes.
                        }
                        metaFound = true;

                        error = readBox(bitstream);
                        if (error != ErrorCode::OK)
                        {
                            break;
                        }
                        const ContextId contextId = 0;  // Always use id 0 for root-level meta box.
                        MetaBox& metaBox          = mMetaBoxMap[contextId];
                        metaBox.parseBox(bitstream);

                        mFileProperties.rootLevelMetaBoxProperties           = extractMetaBoxProperties(metaBox);
                        mFileProperties.rootLevelMetaBoxProperties.contextId = contextId;
                        mMetaBoxInfo[contextId]                              = extractItems(metaBox, contextId);
                        processDecoderConfigProperties(contextId);

                        Array<ImageId> masterImages;
                        getMasterImages(masterImages);
                        mMetaBoxInfo.at(contextId).displayableMasterImages = static_cast<uint32_t>(masterImages.size);

                        for (const auto& itemEntry : mFileProperties.rootLevelMetaBoxProperties.itemFeaturesMap)
                        {
                            if (itemEntry.second.hasFeature(ItemFeatureEnum::IsPrimaryImage))
                            {
                                mIsPrimaryItemSet = true;
                                mPrimaryItemId    = itemEntry.first;
                            }
                        }
                    }
                    else if (boxType == "moov")
                    {
                        if (moovFound == true)
                        {
                            error = ErrorCode::FILE_READ_ERROR;
                            break;
                        }
                        moovFound = true;

                        error = readBox(bitstream);
                        if (error != ErrorCode::OK)
                        {
                            break;
                        }

                        MovieBox moov;
                        moov.parseBox(bitstream);
                        mFileProperties.trackProperties = fillTrackProperties(moov);
                        mFileProperties.movieTimescale  = moov.getMovieHeaderBox().getTimeScale();
                        mMatrix                         = moov.getMovieHeaderBox().getMatrix();
                    }
                    else if (boxType == "mdat" || boxType == "free" || boxType == "skip")
                    {
                        // skip 'mdat' as it is handled elsewhere, 'free' can be skipped
                        error = skipBox();
                    }
                    else
                    {
                        logWarning() << "Skipping root level box of unknown type '" << boxType << "'" << std::endl;
                        error = skipBox();
                    }
                }
            }
        }
        catch (const Exception& exc)
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
            // peek() sets eof bit for the stream. Clear stream to make sure it is still accessible. seekg() in C++11
            // should clear stream after eof, but this does not seem to be always happening.
            if ((!mIo.stream->good()) && (!mIo.stream->eof()))
            {
                return ErrorCode::FILE_READ_ERROR;
            }
            mIo.stream->clear();
            mFileProperties.fileFeature = getFileFeatures();
            mState                      = State::READY;
        }

        return error;
    }

    HeifReaderImpl::ItemInfoMap HeifReaderImpl::extractItemInfoMap(const MetaBox& metaBox) const
    {
        ItemInfoMap itemInfoMap;
        const auto& itemIds = metaBox.getItemInfoBox().getItemIds();
        for (const auto itemId : itemIds)
        {
            const ItemInfoEntry& item = metaBox.getItemInfoBox().getItemById(itemId);
            ItemInfo itemInfo;
            itemInfo.type            = item.getItemType();
            itemInfo.name            = item.getItemName();
            itemInfo.contentType     = item.getContentType();
            itemInfo.contentEncoding = item.getContentEncoding();

            if (isImageItemType(itemInfo.type))
            {
                const ItemPropertiesBox& iprp = metaBox.getItemPropertiesBox();
                const std::uint32_t ispeIndex = iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::ISPE, itemId);
                if (ispeIndex)
                {
                    const auto imageSpatialExtentsProperties =
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

        if (mMetaBoxInfo.count(mFileProperties.rootLevelMetaBoxProperties.contextId) == 1)
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
        for (const auto& trackProperties : mFileProperties.trackProperties)
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

    ErrorCode HeifReaderImpl::readBytes(const unsigned int count, std::int64_t& result)
    {
        std::int64_t value = 0;
        for (unsigned int i = 0; i < count; ++i)
        {
            value = (value << 8) | static_cast<int64_t>(mIo.stream->get());
            if (!mIo.stream->good())
            {
                return ErrorCode::FILE_READ_ERROR;
            }
        }

        result = value;
        return ErrorCode::OK;
    }

    void HeifReaderImpl::seekInput(const std::int64_t pos) const
    {
        if (mIo.stream->tell() != pos)
        {
            mIo.stream->seek(pos);
        }
    }

    ErrorCode HeifReaderImpl::skipBox()
    {
        const std::int64_t startLocation = mIo.stream->tell();

        String boxType;
        std::int64_t boxSize = 0;
        ErrorCode error      = readBoxParameters(boxType, boxSize);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        seekInput(startLocation + boxSize);
        if (!mIo.stream->good())
        {
            return ErrorCode::FILE_READ_ERROR;
        }
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::readBox(BitStream& bitstream)
    {
        String boxType;
        std::int64_t boxSize = 0;

        ErrorCode error = readBoxParameters(boxType, boxSize);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        Vector<uint8_t> data((std::uint64_t) boxSize);
        mIo.stream->read(reinterpret_cast<char*>(data.data()), boxSize);
        if (!mIo.stream->good())
        {
            return ErrorCode::FILE_READ_ERROR;
        }
        bitstream.clear();
        bitstream.reset();
        bitstream.write8BitsArray(data, std::uint64_t(boxSize));
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::readBoxParameters(String& boxType, std::int64_t& boxSize)
    {
        const std::int64_t startLocation = mIo.stream->tell();

        // Read the 32-bit length field of the box
        ErrorCode error = readBytes(4, boxSize);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        // Read the four character string for boxType
        static const size_t TYPE_LENGTH = 4;
        boxType.resize(TYPE_LENGTH);
        mIo.stream->read(&boxType[0], TYPE_LENGTH);
        if (!mIo.stream->good())
        {
            return ErrorCode::FILE_READ_ERROR;
        }

        // Check if 64-bit largesize field is used
        if (boxSize == 1)
        {
            error = readBytes(8, boxSize);
            if (error != ErrorCode::OK)
            {
                return error;
            }
        }

        int64_t boxEndOffset = startLocation + boxSize;
        if (boxSize < 8 || (boxEndOffset < 8) || ((mIo.size > 0) && (boxEndOffset > mIo.size)))
        {
            return ErrorCode::FILE_READ_ERROR;
        }

        // Seek to box beginning
        seekInput(startLocation);
        if (!mIo.stream->good())
        {
            return ErrorCode::FILE_READ_ERROR;
        }
        return ErrorCode::OK;
    }

    ParameterSetMap HeifReaderImpl::makeDecoderParameterSetMap(const DecoderConfigurationRecord& record) const
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
            default:
            {
                type = (DecoderSpecInfoType) t.first;
                break;
            }
            }
            auto& e = pm[type];
            e.insert(e.begin(), t.second.begin(), t.second.end());
        }
        return pm;
    }

    void HeifReaderImpl::getCollectionItems(IdVector& items) const
    {
        const auto contextId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        items.clear();
        for (const auto& imageInfo : mMetaBoxInfo.at(contextId).itemInfoMap)
        {
            if (isImageItemType(imageInfo.second.type))
            {
                items.push_back(imageInfo.first);
            }
        }
    }

    ErrorCode HeifReaderImpl::getSequenceItems(SequenceId sequenceId, IdVector& items) const
    {
        ErrorCode error;
        if ((error = isValidTrack(sequenceId)) != ErrorCode::OK)
        {
            return error;
        }

        items.clear();
        for (const auto& sampleInfo : mTrackInfo.at(sequenceId).samples)
        {
            items.push_back(sampleInfo.decodingOrder);
        }

        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::getProtection(const std::uint32_t itemId, bool& isProtected) const
    {
        ItemInfoEntry entry;
        try
        {
            const auto contextId = mFileProperties.rootLevelMetaBoxProperties.contextId;
            entry                = mMetaBoxMap.at(contextId).getItemInfoBox().getItemById(itemId);
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

    ErrorCode HeifReaderImpl::processAvcItemData(uint8_t* memoryBuffer, uint64_t& memoryBufferSize) const
    {
        uint32_t outputOffset = 0;
        uint32_t byteOffset   = 0;

        while (outputOffset < memoryBufferSize)
        {
            uint32_t nalLength                      = (uint8_t) memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 0;
            byteOffset++;
            nalLength = (nalLength << 8) | (uint8_t) memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 0;
            byteOffset++;
            nalLength = (nalLength << 8) | (uint8_t) memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 0;
            byteOffset++;
            nalLength = (nalLength << 8) | (uint8_t) memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 1;
            // byteOffset++;
            // AvcNalUnitType naluType = AvcNalUnitType((uint8_t)memoryBuffer[outputOffset + byteOffset] & 0x1f);
            outputOffset += nalLength + 4;  // 4 bytes of nal length information
            byteOffset = 0;
        }
        return ErrorCode::OK;
    }

    ErrorCode HeifReaderImpl::processHevcItemData(uint8_t* memoryBuffer, uint64_t& memoryBufferSize) const
    {
        uint32_t outputOffset = 0;
        uint32_t byteOffset   = 0;

        while (outputOffset < memoryBufferSize)
        {
            uint32_t nalLength                      = (uint8_t) memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 0;
            byteOffset++;
            nalLength = (nalLength << 8) | (uint8_t) memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 0;
            byteOffset++;
            nalLength = (nalLength << 8) | (uint8_t) memoryBuffer[outputOffset + byteOffset];
            memoryBuffer[outputOffset + byteOffset] = 0;
            byteOffset++;
            nalLength = (nalLength << 8) | (uint8_t) memoryBuffer[outputOffset + byteOffset];
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
        const auto rootMetaId = mFileProperties.rootLevelMetaBoxProperties.contextId;

        FourCCInt type;
        try
        {
            type = mMetaBoxMap.at(rootMetaId).getItemInfoBox().getItemById(imageId.get()).getItemType();
        }
        catch (...)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }

        if (isImageItemType(type))
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
        const auto rootMetaId = mFileProperties.rootLevelMetaBoxProperties.contextId;
        try
        {
            mMetaBoxMap.at(rootMetaId).getItemInfoBox().getItemById(imageId.get()).getItemType();
        }
        catch (...)
        {
            return ErrorCode::INVALID_ITEM_ID;
        }

        return ErrorCode::OK;
    }

    MetaBoxProperties HeifReaderImpl::extractMetaBoxProperties(const MetaBox& metaBox) const
    {
        MetaBoxProperties metaBoxProperties;
        metaBoxProperties.itemFeaturesMap = extractMetaBoxItemPropertiesMap(metaBox);
        metaBoxProperties.entityGroupings = extractMetaBoxEntityToGroupMaps(metaBox);
        metaBoxProperties.metaBoxFeature =
            extractMetaBoxFeatures(metaBoxProperties.itemFeaturesMap, metaBoxProperties.entityGroupings);

        return metaBoxProperties;
    }

    MetaBoxFeature HeifReaderImpl::extractMetaBoxFeatures(const ItemFeaturesMap& imageFeatures,
                                                          const Groupings& groupings) const
    {
        MetaBoxFeature metaBoxFeature;

        if (groupings.size() > 0)
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

    Groupings HeifReaderImpl::extractMetaBoxEntityToGroupMaps(const MetaBox& metaBox) const
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

    ItemFeaturesMap HeifReaderImpl::extractMetaBoxItemPropertiesMap(const MetaBox& metaBox) const
    {
        ItemFeaturesMap itemFeaturesMap;
        const auto& itemIds = metaBox.getItemInfoBox().getItemIds();

        for (const auto itemId : itemIds)
        {
            const ItemInfoEntry& item = metaBox.getItemInfoBox().getItemById(itemId);
            const auto type           = item.getItemType();

            ItemFeature itemFeatures;
            if (isImageItemType(type))
            {
                if (item.getItemProtectionIndex() > 0)
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsProtected);
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
                if (itemFeatures.hasFeature(ItemFeatureEnum::IsThumbnailImage) == false &&
                    itemFeatures.hasFeature(ItemFeatureEnum::IsAuxiliaryImage) == false)
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
                if (item.getFlags() & HIDDEN_IMAGE_MASK)
                {
                    itemFeatures.setFeature(ItemFeatureEnum::IsHiddenImage);
                }
            }
            else
            {
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
            itemFeaturesMap[itemId] = itemFeatures;
        }

        return itemFeaturesMap;
    }  // namespace HEIF

    static ItemPropertyType itemPropertyTypeFromPropertyType(const ItemPropertiesBox::PropertyType& aType)
    {
        if (aType == ItemPropertiesBox::PropertyType::RAW)
        {
            return ItemPropertyType::RAW;
        }
        if (aType == ItemPropertiesBox::PropertyType::AUXC)
        {
            return ItemPropertyType::AUXC;
        }
        if (aType == ItemPropertiesBox::PropertyType::AVCC)
        {
            return ItemPropertyType::AVCC;
        }
        if (aType == ItemPropertiesBox::PropertyType::CLAP)
        {
            return ItemPropertyType::CLAP;
        }
        if (aType == ItemPropertiesBox::PropertyType::COLR)
        {
            return ItemPropertyType::COLR;
        }
        if (aType == ItemPropertiesBox::PropertyType::HVCC)
        {
            return ItemPropertyType::HVCC;
        }
        if (aType == ItemPropertiesBox::PropertyType::IMIR)
        {
            return ItemPropertyType::IMIR;
        }
        if (aType == ItemPropertiesBox::PropertyType::IROT)
        {
            return ItemPropertyType::IROT;
        }
        if (aType == ItemPropertiesBox::PropertyType::ISPE)
        {
            return ItemPropertyType::ISPE;
        }
        if (aType == ItemPropertiesBox::PropertyType::JPGC)
        {
            return ItemPropertyType::JPGC;
        }
        if (aType == ItemPropertiesBox::PropertyType::PASP)
        {
            return ItemPropertyType::PASP;
        }
        if (aType == ItemPropertiesBox::PropertyType::PIXI)
        {
            return ItemPropertyType::PIXI;
        }
        if (aType == ItemPropertiesBox::PropertyType::RLOC)
        {
            return ItemPropertyType::RLOC;
        }
        return ItemPropertyType::RAW;
    }

    HeifReaderImpl::Properties HeifReaderImpl::processItemProperties(const ContextId contextId) const
    {
        Properties propertyMap;

        const ItemPropertiesBox& iprp = mMetaBoxMap.at(contextId).getItemPropertiesBox();
        const auto& itemIds           = mMetaBoxMap.at(contextId).getItemInfoBox().getItemIds();
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

        return propertyMap;
    }

    void HeifReaderImpl::processDecoderConfigProperties(const ContextId contextId)
    {
        // Decoder configuration gets special handling because that information is accessed by the reader
        // implementation itself.

        const ItemPropertiesBox& iprp = mMetaBoxMap.at(contextId).getItemPropertiesBox();

        for (const auto& imageProperties : mFileProperties.rootLevelMetaBoxProperties.itemFeaturesMap)
        {
            const ImageId imageId = imageProperties.first;
            const Id id(contextId, imageId.get());
            const std::uint32_t hvccIndex =
                iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::HVCC, imageId.get());
            const std::uint32_t avccIndex =
                iprp.findPropertyIndex(ItemPropertiesBox::PropertyType::AVCC, imageId.get());

            FourCCInt type;
            Id configIndex(0, 0);
            const DecoderConfigurationBox* record = nullptr;
            if (hvccIndex)
            {
                configIndex = Id(contextId, hvccIndex);
                type        = "hvc1";
            }
            else if (avccIndex)
            {
                configIndex = Id(contextId, avccIndex);
                type        = "avc1";
            }
            else
            {
                continue;
            }
            record = static_cast<const DecoderConfigurationBox*>(iprp.getPropertyByIndex(configIndex.second - 1));
            if (!mParameterSetMap.count(configIndex))
            {
                mParameterSetMap[configIndex] = makeDecoderParameterSetMap(record->getConfiguration());
            }
            mImageToParameterSetMap[id] = configIndex;
            mDecoderCodeTypeMap[id]     = type;
        }
    }

    HeifReaderImpl::MetaBoxInfo HeifReaderImpl::extractItems(const MetaBox& metaBox,
                                                             const std::uint32_t contextId) const
    {
        MetaBoxInfo metaBoxInfo;
        const auto& itemIds = metaBox.getItemInfoBox().getItemIds();
        for (const auto itemId : itemIds)
        {
            const ItemInfoEntry& item = metaBox.getItemInfoBox().getItemById(itemId);
            const auto type           = item.getItemType();
            if (type == "grid" || type == "iovl")
            {
                bool iProtected;
                ErrorCode error = getProtection(itemId, iProtected);
                if (error != ErrorCode::OK || iProtected)
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
                    getReferencedFromItemListByType(ImageId(itemId), "dimg", grid.imageIds);
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
                    for (std::uint16_t i = 0; i < imageOverlay.offsets.size(); ++i)
                    {
                        iovl.offsets[i] = Overlay::Offset{imageOverlay.offsets[i].horizontalOffset,
                                                          imageOverlay.offsets[i].verticalOffset};
                    }

                    getReferencedFromItemListByType(itemId, "dimg", iovl.imageIds);
                    metaBoxInfo.iovlItems.insert({itemId, iovl});
                }
            }
        }

        metaBoxInfo.properties  = processItemProperties(contextId);
        metaBoxInfo.itemInfoMap = extractItemInfoMap(metaBox);

        return metaBoxInfo;
    }

    ErrorCode HeifReaderImpl::loadItemData(const MetaBox& metaBox, const ItemId itemId, DataVector& data) const
    {
        const streampos oldPosition = mIo.stream->tell();

        uint64_t itemLength(0);
        List<ItemId> pastReferences;
        ErrorCode error = getItemLength(metaBox, itemId, itemLength, pastReferences);
        if (error != ErrorCode::OK)
        {
            return error;
        }
        else if (static_cast<int64_t>(itemLength) > mIo.size)
        {
            return ErrorCode::FILE_HEADER_ERROR;
        }
        data.resize(itemLength);

        uint8_t* dataPtr = data.data();
        error            = readItem(metaBox, itemId, dataPtr, itemLength);
        mIo.stream->seek(oldPosition);
        return error;
    }

    ErrorCode HeifReaderImpl::getItemLength(const MetaBox& metaBox,
                                            const ItemId& itemId,
                                            std::uint64_t& itemLength,
                                            List<ItemId>& pastReferences) const
    {
        ErrorCode error = isValidItem(itemId);
        if (error != ErrorCode::OK)
        {
            return error;
        }

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
        if (!iloc.hasItemIdEntry(itemId))
        {
            itemLength = 0;
            return ErrorCode::INVALID_ITEM_ID;
        }
        const ItemLocation& itemLocation                          = iloc.getItemLocationForID(itemId);
        const ItemLocation::ConstructionMethod constructionMethod = itemLocation.getConstructionMethod();
        const ExtentList& extentList                              = itemLocation.getExtentList();

        if (extentList.size() == 0)
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
                const ItemId& subItemId = toItemIds.at(extentSourceItemIndex - 1);
                if (itemId == subItemId)
                {
                    return ErrorCode::FILE_HEADER_ERROR;  // avoid looping references.
                }

                error = getItemLength(metaBox, subItemId, subItemLength, pastReferences);
                if (error != ErrorCode::OK)
                {
                    return error;
                }
                else if (static_cast<int64_t>(subItemLength) > mIo.size)
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

    ErrorCode
    HeifReaderImpl::readItem(const MetaBox& metaBox, const ItemId itemId, uint8_t* memoryBuffer, uint64_t maxSize) const
    {
        ErrorCode error = isValidItem(itemId);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        const ItemLocationBox& iloc = metaBox.getItemLocationBox();
        const unsigned int version  = iloc.getVersion();
        if (!iloc.hasItemIdEntry(itemId))
        {
            return ErrorCode::INVALID_ITEM_ID;
        }
        const ItemLocation& itemLocation                          = iloc.getItemLocationForID(itemId);
        const ItemLocation::ConstructionMethod constructionMethod = itemLocation.getConstructionMethod();
        const ExtentList& extentList                              = itemLocation.getExtentList();
        const std::uint64_t baseOffset                            = itemLocation.getBaseOffset();

        if (extentList.size() == 0)
        {
            return ErrorCode::FILE_READ_ERROR;  // No extents given for an item.
        }

        uint64_t totalLenght(0);
        if (version == 0 || ((version >= 1) && constructionMethod == ItemLocation::ConstructionMethod::FILE_OFFSET))
        {
            for (const auto& extent : extentList)
            {
                const std::int64_t offset = static_cast<std::int64_t>(baseOffset + extent.mExtentOffset);
                mIo.stream->seek(offset);
                if (!mIo.stream->good())
                {
                    return ErrorCode::FILE_READ_ERROR;
                }
                if (totalLenght + extent.mExtentLength > maxSize)
                {
                    return ErrorCode::FILE_READ_ERROR;
                }
                mIo.stream->read(reinterpret_cast<char*>(memoryBuffer), std::streamsize(extent.mExtentLength));
                if (!mIo.stream->good())
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
                if (metaBox.getItemDataBox().read(memoryBuffer, offset, extent.mExtentLength) == false)
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

                const ItemId subItemId = toItemIds.at(extentSourceItemIndex - 1);
                if (itemId == subItemId)
                {
                    return ErrorCode::FILE_HEADER_ERROR;
                }
                uint64_t subItemLength(0);
                List<ItemId> pastReferences;
                pastReferences.push_back(itemId);
                error = getItemLength(metaBox, subItemId, subItemLength, pastReferences);
                if (error != ErrorCode::OK)
                {
                    return error;
                }
                else if (static_cast<int64_t>(subItemLength) > mIo.size)
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

    ErrorCode HeifReaderImpl::isValidTrack(const SequenceId& sequenceId) const
    {
        ErrorCode error;
        if ((error = isInitialized()) != ErrorCode::OK)
        {
            return error;
        }
        if (mTrackInfo.count(sequenceId) != 0)
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
        if (mTrackInfo.at(sequenceId).samples.size() > sequenceImageId.get())
        {
            return ErrorCode::OK;
        }
        return ErrorCode::INVALID_SEQUENCE_IMAGE_ID;
    }

    TrackPropertiesMap HeifReaderImpl::fillTrackProperties(const MovieBox& moovBox)
    {
        TrackPropertiesMap trackPropertiesMap;

        const Vector<UniquePtr<TrackBox>>& trackBoxes = moovBox.getTrackBoxes();
        for (const auto& trackBoxP : trackBoxes)
        {
            const TrackBox* trackBox = trackBoxP.get();
            TrackProperties trackProperties;
            TrackInfo trackInfo = extractTrackInfo(trackBox, moovBox);

            trackProperties.trackId = trackBox->getTrackHeaderBox().getTrackID();

            trackProperties.sampleProperties    = makeSamplePropertiesMap(trackBox);
            std::uint64_t maxSampleSize         = 0;
            trackInfo.samples                   = makeSampleInfoVector(trackBox, trackInfo.pMap, maxSampleSize);
            mTrackInfo[trackProperties.trackId] = trackInfo;

            fillSampleEntryMap(trackBox);

            trackProperties.trackFeature      = getTrackFeatures(trackBox);
            trackProperties.referenceTrackIds = getReferenceTrackIds(trackBox);
            trackProperties.groupedSamples    = getSampleGroupings(trackBox);
            trackProperties.equivalences      = getEquivalenceGroups(trackBox);
            trackProperties.metadatas         = getSampleToMetadataItemGroups(trackBox);
            trackProperties.referenceSamples  = getDirectReferenceSamplesGroups(trackBox);
            trackProperties.alternateTrackIds = getAlternateTrackIds(trackBox, moovBox);
            trackProperties.alternateGroupId  = trackBox->getTrackHeaderBox().getAlternateGroup();
            trackProperties.maxSampleSize     = maxSampleSize;
            trackProperties.timeScale         = trackBox->getMediaBox().getMediaHeaderBox().getTimeScale();
            trackProperties.editList          = getEditList(trackBox, trackInfo.repetitions);

            trackPropertiesMap[trackProperties.trackId] = trackProperties;
        }

        // Some TrackFeatures are easiest to set after a part of properties have already been filled.
        for (auto& mapEntry : trackPropertiesMap)
        {
            const SequenceId trackId   = mapEntry.first;
            TrackFeature& trackFeature = mapEntry.second.trackFeature;
            if (isAnyLinkedToWithType(trackPropertiesMap, trackId, "thmb") == true)
            {
                trackFeature.setFeature(TrackFeatureEnum::HasLinkedThumbnailImageSequence);
            }
            if (isAnyLinkedToWithType(trackPropertiesMap, trackId, "auxl") == true)
            {
                trackFeature.setFeature(TrackFeatureEnum::HasLinkedAuxiliaryImageSequence);
            }

            if (trackFeature.hasFeature(TrackFeatureEnum::HasEditList) && (mTrackInfo[trackId].pMap.size() <= 1))
            {
                trackFeature.setFeature(TrackFeatureEnum::DisplayAllSamples);
            }
        }

        return trackPropertiesMap;
    }

    IdVector HeifReaderImpl::getAlternateTrackIds(const TrackBox* trackBox, const MovieBox& moovBox) const
    {
        IdVector trackIds;
        const std::uint16_t alternateGroupId = trackBox->getTrackHeaderBox().getAlternateGroup();

        if (alternateGroupId == 0)
        {
            return trackIds;
        }

        const std::uint32_t trackId                   = trackBox->getTrackHeaderBox().getTrackID();
        const Vector<UniquePtr<TrackBox>>& trackBoxes = moovBox.getTrackBoxes();
        for (const auto& trackboxP : trackBoxes)
        {
            const TrackBox* trackbox        = trackboxP.get();
            const uint32_t alternateTrackId = trackbox->getTrackHeaderBox().getTrackID();
            if ((trackId != alternateTrackId) &&
                (alternateGroupId == trackbox->getTrackHeaderBox().getAlternateGroup()))
            {
                trackIds.push_back(alternateTrackId);
            }
        }

        return trackIds;
    }

    bool HeifReaderImpl::isAnyLinkedToWithType(const TrackPropertiesMap& trackPropertiesMap,
                                               const SequenceId trackId,
                                               const FourCCInt referenceType) const
    {
        for (const auto& trackProperties : trackPropertiesMap)
        {
            for (const auto& reference : trackProperties.second.referenceTrackIds)
            {
                if ((reference.first == FourCC(referenceType.getUInt32())) &&
                    (std::find(reference.second.begin(), reference.second.end(), trackId.get()) !=
                     reference.second.end()))
                {
                    return true;
                }
            }
        }

        return false;
    }

    TrackFeature HeifReaderImpl::getTrackFeatures(const TrackBox* trackBox) const
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

            if (trackBox->getHasTrackReferences() == false && handlerBox.getHandlerType() == "pict")
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
                    const VisualSampleEntryBox* visualSampleEntry =
                        static_cast<const VisualSampleEntryBox*>(sampleEntry.get());
                    if ((visualSampleEntry) && (visualSampleEntry->isCodingConstraintsBoxPresent() == true))
                    {
                        trackFeature.setFeature(TrackFeatureEnum::HasCodingConstraints);
                        break;
                    }
                }
            }

            // Sample groupings
            if (stblBox.getSampleToGroupBoxes().size() != 0)
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
                if (editListBox)
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

    TypeToIdsMap HeifReaderImpl::getReferenceTrackIds(const TrackBox* trackBox) const
    {
        TypeToIdsMap trackReferenceMap;

        const auto& trackReferenceTypeBoxes = trackBox->getTrackReferenceBox().getTrefTypeBoxes();
        for (const auto& trackReferenceTypeBox : trackReferenceTypeBoxes)
        {
            trackReferenceMap[FourCC(trackReferenceTypeBox.getType().getUInt32())] =
                trackReferenceTypeBox.getTrackIds();
        }

        return trackReferenceMap;
    }

    EditList HeifReaderImpl::getEditList(const TrackBox* trackBox, const double repetitions) const
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
                if ((trackBox->getTrackHeaderBox().getVersion() == 0) &&
                    (duration == std::numeric_limits<uint32_t>::max()))
                {
                    editlist.repetitions = 0.0;
                }
                else if (duration == std::numeric_limits<uint64_t>::max())
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
                    const EditListBox::EntryVersion0& mEntryVersion0 =
                        editListBox->getEntry<EditListBox::EntryVersion0>(i);
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
                }
                else
                {
                    const EditListBox::EntryVersion1& mEntryVersion1 =
                        editListBox->getEntry<EditListBox::EntryVersion1>(i);
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
                }
                editUnits.push_back(editUnit);
            }
            editlist.editUnits = makeArray<EditUnit>(editUnits);
        }
        return editlist;
    }

    Array<SampleGrouping> HeifReaderImpl::getSampleGroupings(const TrackBox* trackBox) const
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

    Array<SampleVisualEquivalence> HeifReaderImpl::getEquivalenceGroups(const TrackBox* trackBox) const
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
            const VisualEquivalenceEntry* eqiv = static_cast<const VisualEquivalenceEntry*>(sgpd->getEntry(groupIndex));

            eqivInfos[groupIndex - 1].sampleGroupDescriptionIndex = groupIndex;
            eqivInfos[groupIndex - 1].timeOffset                  = eqiv->getTimeOffset();
            eqivInfos[groupIndex - 1].timescaleMultiplier         = eqiv->getTimescaleMultiplier();
        }

        return eqivInfos;
    }

    Array<SampleToMetadataItem> HeifReaderImpl::getSampleToMetadataItemGroups(const TrackBox* trackBox) const
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
            const SampleToMetadataItemEntry* stmi =
                static_cast<const SampleToMetadataItemEntry*>(sgpd->getEntry(groupIndex));
            if (stmi->getMetaBoxHandlerType() == "pict")
            {
                stmiInfos[groupIndex - 1].sampleGroupDescriptionIndex = groupIndex;
                stmiInfos[groupIndex - 1].metadataItemIds             = makeArray<ImageId>(stmi->getItemIds());
            }
        }
        return stmiInfos;
    }

    Array<DirectReferenceSamples> HeifReaderImpl::getDirectReferenceSamplesGroups(const TrackBox* trackBox) const
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
            const DirectReferenceSamplesList* refs =
                static_cast<const DirectReferenceSamplesList*>(sgpd->getEntry(groupIndex));

            refsInfos[groupIndex - 1].sampleGroupDescriptionIndex = groupIndex;
            refsInfos[groupIndex - 1].sampleId                    = refs->getSampleId();
            refsInfos[groupIndex - 1].referenceItemIds =
                makeArray<SequenceImageId>(refs->getDirectReferenceSampleIds());
        }
        return refsInfos;
    }

    HeifReaderImpl::TrackInfo HeifReaderImpl::extractTrackInfo(const TrackBox* trackBox, const MovieBox& moovBox) const
    {
        TrackInfo trackInfo;
        const SampleTableBox& stblBox          = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
        const TrackHeaderBox& trackHeaderBox   = trackBox->getTrackHeaderBox();
        const TimeToSampleBox& timeToSampleBox = stblBox.getTimeToSampleBox();
        std::shared_ptr<const CompositionOffsetBox> compositionOffsetBox = stblBox.getCompositionOffsetBox();

        trackInfo.width  = trackHeaderBox.getWidth() >> 16;
        trackInfo.height = trackHeaderBox.getHeight() >> 16;
        trackInfo.matrix = trackHeaderBox.getMatrix();

        const uint32_t mediaTimeScale = trackBox->getMediaBox().getMediaHeaderBox().getTimeScale();
        const uint32_t movieTimeScale =
            moovBox.getMovieHeaderBox().getTimeScale();              // Number of time units that pass in a second
        const uint64_t tkhdDuration = trackHeaderBox.getDuration();  // Duration is in timescale units

        std::shared_ptr<const EditBox> editBox = trackBox->getEditBox();
        DecodePts decodePts;
        decodePts.loadBox(&timeToSampleBox);
        decodePts.loadBox(compositionOffsetBox.get());
        if (editBox != nullptr)
        {
            const EditListBox* editListBox = editBox->getEditListBox();
            decodePts.loadBox(editListBox, movieTimeScale, mediaTimeScale);
        }
        if (!decodePts.unravel())
        {
            throw FileReaderException(ErrorCode::FILE_HEADER_ERROR);
        }

        trackInfo.pMap = decodePts.getTime(mediaTimeScale);

        static const uint32_t DURATION_FROM_EDIT_LIST = 0xffffffff;
        if (tkhdDuration == DURATION_FROM_EDIT_LIST)
        {
            trackInfo.duration = static_cast<double>(decodePts.getSpan()) / mediaTimeScale;
        }
        else
        {
            trackInfo.duration = tkhdDuration / static_cast<double>(movieTimeScale);
        }

        if (editBox != nullptr)
        {
            // HEIF defines that when (flags & 1) is equal to 1, the entire edit list is repeated a sufficient
            // number of times to equal the track duration.
            if ((editBox->getEditListBox()->getFlags() & 1) == 1)
            {
                DecodePts::PMap repeatingPMap;
                const int64_t trackDuration    = static_cast<int64_t>(trackInfo.duration * 1000u);
                const int64_t editListDuration = static_cast<int64_t>(decodePts.getSpan() * 1000u / mediaTimeScale);
                trackInfo.repetitions          = double(trackDuration) / double(editListDuration);
                auto iter                      = trackInfo.pMap.cbegin();
                int64_t nextSampleTimestamp    = iter->first;
                int64_t offset                 = 0;

                while (nextSampleTimestamp < trackDuration)
                {
                    repeatingPMap.insert(nextSampleTimestamp, iter->second);
                    ++iter;

                    // Increase timestamp offset and skip to begin if the end was reached.
                    if (iter == trackInfo.pMap.cend())
                    {
                        iter = trackInfo.pMap.cbegin();
                        offset += editListDuration;
                    }
                    nextSampleTimestamp = iter->first + offset;
                }

                trackInfo.pMap = repeatingPMap;
            }
        }

        return trackInfo;
    }

    HeifReaderImpl::SampleInfoVector HeifReaderImpl::makeSampleInfoVector(const TrackBox* trackBox,
                                                                          const DecodePts::PMap& pMap,
                                                                          std::uint64_t& maxSampleSize) const
    {
        SampleInfoVector sampleInfoVector;

        const SampleTableBox& stblBox       = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
        const SampleDescriptionBox& stsdBox = stblBox.getSampleDescriptionBox();
        const SampleToChunkBox& stscBox     = stblBox.getSampleToChunkBox();
        const ChunkOffsetBox& stcoBox       = stblBox.getChunkOffsetBox();
        const SampleSizeBox& stszBox        = stblBox.getSampleSizeBox();
        const FourCCInt handlerType         = trackBox->getMediaBox().getHandlerBox().getHandlerType();

        const Vector<uint32_t>& sampleSizeEntries          = stszBox.getEntrySize();
        const Vector<uint64_t>& chunkOffsets               = stcoBox.getChunkOffsets();
        const Vector<SampleToGroupBox>& sampleToGroupBoxes = stblBox.getSampleToGroupBoxes();

        const unsigned int sampleCount = stszBox.getSampleCount();

        if (sampleCount > sampleSizeEntries.size())
        {
            throw FileReaderException(ErrorCode::FILE_HEADER_ERROR);
        }

        std::uint32_t previousChunkIndex = 0;  // Index is 1-based so 0 will not be used.
        std::uint64_t maxSize            = 0;
        for (uint32_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        {
            SampleInfo sampleInfo;

            // Set basic sample information
            sampleInfo.decodingOrder = sampleIndex;
            sampleInfo.dataLength    = sampleSizeEntries.at(sampleIndex);

            if (sampleInfo.dataLength > maxSize)
            {
                maxSize = sampleInfo.dataLength;
            }

            std::uint32_t chunkIndex;
            if (!stscBox.getSampleChunkIndex(sampleIndex, chunkIndex))
            {
                throw FileReaderException(ErrorCode::FILE_HEADER_ERROR);
            }

            if (chunkIndex == previousChunkIndex)
            {
                sampleInfo.dataOffset = sampleInfoVector.back().dataOffset + sampleInfoVector.back().dataLength;
            }
            else
            {
                sampleInfo.dataOffset = chunkOffsets.at(chunkIndex - 1);
                previousChunkIndex    = chunkIndex;
            }

            // Set dimensions
            if (handlerType == "pict" || handlerType == "vide" || handlerType == "auxv")
            {
                uint32_t sampleDescriptionIndex;
                if (stscBox.getSampleDescriptionIndex(sampleIndex, sampleDescriptionIndex) == false)
                {
                    throw FileReaderException(ErrorCode::FILE_HEADER_ERROR);
                }
                const VisualSampleEntryBox* sampleEntry =
                    static_cast<const VisualSampleEntryBox*>(stsdBox.getSampleEntry(sampleDescriptionIndex));
                if (sampleEntry != nullptr)
                {
                    sampleInfo.width  = sampleEntry->getWidth();
                    sampleInfo.height = sampleEntry->getHeight();
                }
                else
                {
                    // unknown sample entry, set to zero:
                    sampleInfo.width  = 0;
                    sampleInfo.height = 0;
                }
            }
            else  // non-visual track
            {
                sampleInfo.width  = 0;
                sampleInfo.height = 0;
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
                        sampleInfo.decodeDependencies =
                            getSampleDirectDependencies(sampleIndex, sgdb, sampleToGroupBox);
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

        maxSampleSize = maxSize;
        return sampleInfoVector;
    }

    SamplePropertiesMap HeifReaderImpl::makeSamplePropertiesMap(const TrackBox* trackBox)
    {
        SamplePropertiesMap samplePropertiesMap;

        const SampleTableBox& stblBox       = trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox();
        const SampleDescriptionBox& stsdBox = stblBox.getSampleDescriptionBox();
        const SampleToChunkBox& stscBox     = stblBox.getSampleToChunkBox();
        const SampleSizeBox& stszBox        = stblBox.getSampleSizeBox();
        const TimeToSampleBox& sttsBox      = stblBox.getTimeToSampleBox();
        const FourCCInt handlerType         = trackBox->getMediaBox().getHandlerBox().getHandlerType();

        const Vector<SampleToGroupBox>& sampleToGroupBoxes = stblBox.getSampleToGroupBoxes();
        const Vector<std::uint32_t>& sampleDeltas          = sttsBox.getSampleDeltas();
        const Vector<uint32_t>& sampleSizeEntries          = stszBox.getEntrySize();

        const unsigned int sampleCount = stszBox.getSampleCount();
        for (uint32_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        {
            SampleProperties sampleProperties{};
            sampleProperties.sampleId         = sampleIndex;
            sampleProperties.sampleDurationTS = sampleDeltas.at(sampleIndex);
            sampleProperties.size             = sampleSizeEntries.at(sampleIndex);
            if (stscBox.getSampleDescriptionIndex(sampleIndex, sampleProperties.sampleDescriptionIndex) == false)
            {
                throw FileReaderException(ErrorCode::FILE_HEADER_ERROR);
            }

            if (handlerType == "pict" || handlerType == "vide" || handlerType == "auxv")
            {
                const VisualSampleEntryBox* sampleEntry = static_cast<const VisualSampleEntryBox*>(
                    stsdBox.getSampleEntry(sampleProperties.sampleDescriptionIndex));
                if (sampleEntry != nullptr)
                {
                    sampleProperties.sampleEntryType = FourCC(sampleEntry->getType().getUInt32());
                    auto ccst                        = sampleEntry->getCodingConstraintsBox();
                    if (ccst)
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
                    sampleProperties.sampleEntryType = FourCC();
                    sampleProperties.hasClap         = false;
                    sampleProperties.hasAuxi         = false;
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
                            const DirectReferenceSamplesList* entry = static_cast<const DirectReferenceSamplesList*>(
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
                const SampleEntryBox* sampleEntry =
                    static_cast<const SampleEntryBox*>(stsdBox.getSampleEntry(sampleProperties.sampleDescriptionIndex));
                if (sampleEntry)
                {
                    sampleProperties.sampleEntryType = FourCC(sampleEntry->getType().getUInt32());
                }
                // all samples are reference frames for audio:
                sampleProperties.sampleType = OUTPUT_REFERENCE_FRAME;
            }

            samplePropertiesMap[sampleIndex] = sampleProperties;

            const uint32_t trackId = trackBox->getTrackHeaderBox().getTrackID();
            mDecoderCodeTypeMap[Id(trackId, sampleIndex)] =
                FourCCInt(sampleProperties.sampleEntryType.value);  // Store decoder type for track data decoding
            mImageToParameterSetMap[Id(trackId, sampleIndex)] = Id(trackId, sampleProperties.sampleDescriptionIndex);
        }

        if (stblBox.hasSyncSampleBox())
        {
            const Vector<std::uint32_t>& syncSamples = stblBox.getSyncSampleBox().get()->getSyncSampleIds();
            for (unsigned int i = 0; i < syncSamples.size(); ++i)
            {
                std::uint32_t syncSample    = syncSamples.at(i) - 1;
                auto& sampleProperties      = samplePropertiesMap.at(syncSample);
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
                if (samplePropertiesMap.count(static_cast<uint32_t>(i)))
                {
                    samplePropertiesMap.at(static_cast<uint32_t>(i)).sampleCompositionOffsetTs = offsets.at(i);
                    if (offsets.at(i) == min)
                    {
                        samplePropertiesMap.at(static_cast<uint32_t>(i)).sampleType =
                            SampleType::NON_OUTPUT_REFERENCE_FRAME;
                    }
                }
            }
        }

        return samplePropertiesMap;
    }

    Vector<ItemId> HeifReaderImpl::getSampleDirectDependencies(const ItemId itemId,
                                                               const SampleGroupDescriptionBox* sgpd,
                                                               const SampleToGroupBox& sampleToGroupBox) const
    {
        const uint32_t index                    = sampleToGroupBox.getSampleGroupDescriptionIndex(itemId);
        const DirectReferenceSamplesList* entry = static_cast<const DirectReferenceSamplesList*>(sgpd->getEntry(index));

        const Vector<ItemId>& sampleIds = entry->getDirectReferenceSampleIds();

        // IDs from entry are not sample IDs (in item decoding order), they have be mapped to sample ids
        Vector<ItemId> ids;
        for (auto entryId : sampleIds)
        {
            const uint32_t entryIndex = sgpd->getEntryIndexOfSampleId(entryId);
            ids.push_back(sampleToGroupBox.getSampleId(entryIndex));
        }

        return ids;
    }

    ErrorCode HeifReaderImpl::getTrackFrameData(const unsigned int frameIndex,
                                                const TrackInfo& trackInfo,
                                                uint8_t* memorybuffer,
                                                uint64_t& memorybuffersize) const
    {
        // The requested frame should be one that is available
        if (frameIndex >= trackInfo.samples.size())
        {
            return ErrorCode::INVALID_ITEM_ID;  // Requested frame out of index
        }

        const uint32_t sampleLength = trackInfo.samples.at(frameIndex).dataLength;
        if (memorybuffersize < sampleLength)
        {
            memorybuffersize = sampleLength;
            return ErrorCode::BUFFER_SIZE_TOO_SMALL;
        }
        memorybuffersize = sampleLength;

        mIo.stream->seek(static_cast<int64_t>(trackInfo.samples.at(frameIndex).dataOffset));
        if (!mIo.stream->good())
        {
            return ErrorCode::FILE_READ_ERROR;
        }
        mIo.stream->read(reinterpret_cast<char*>(memorybuffer), sampleLength);
        if (!mIo.stream->good())
        {
            return ErrorCode::FILE_READ_ERROR;
        }

        return ErrorCode::OK;
    }

    void HeifReaderImpl::fillSampleEntryMap(const TrackBox* trackBox)
    {
        const uint32_t trackId = trackBox->getTrackHeaderBox().getTrackID();
        const SampleDescriptionBox& stsdBox =
            trackBox->getMediaBox().getMediaInformationBox().getSampleTableBox().getSampleDescriptionBox();
        const auto& sampleEntries = stsdBox.getSampleEntries();
        unsigned int index        = 1;
        for (auto& entryBox : sampleEntries)
        {
            const SampleEntryBox* entry = entryBox.get();
            // FourCCInt type = entry->getType();
            if (entry)
            {
                mParameterSetMap[Id(trackId, index)] = makeDecoderParameterSetMap(*entry->getConfigurationRecord());

                if (entry->isVisual())
                {
                    const VisualSampleEntryBox* visual = static_cast<const VisualSampleEntryBox*>(entry);
                    const CleanApertureBox* clapBox    = visual->getClap();
                    if (clapBox != nullptr)
                    {
                        mTrackInfo.at(trackId).clapProperties.insert(std::make_pair(index, makeClap(clapBox)));
                    }
                    const AuxiliaryTypeInfoBox* auxiBox = visual->getAuxi();
                    if (auxiBox != nullptr)
                    {
                        mTrackInfo.at(trackId).auxiProperties.insert(std::make_pair(index, makeAuxi(auxiBox)));
                    }
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
                    output.push_back(TimestampIDPair{0xffffffff, sampleId.get()});
                }
            }
            output.push_back(entry);
        }

        return output;
    }

    /* *********************************************************************** */
    /* ************************* Helper functions **************************** */
    /* *********************************************************************** */

    ErrorCode getRawItemType(const MetaBox& metaBox, const uint32_t itemId, FourCCInt& type)
    {
        try
        {
            type = metaBox.getItemInfoBox().getItemById(itemId).getItemType();
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

        CleanAperture clap;
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

    bool isImageItemType(const FourCCInt& type)
    {
        static const std::set<FourCCInt> IMAGE_TYPES = {"avc1", "hvc1", "grid", "iovl", "iden", "jpeg"};

        return IMAGE_TYPES.find(type) != IMAGE_TYPES.end();
    }

    bool doReferencesFromItemIdExist(const MetaBox& metaBox, const uint32_t itemId, const FourCCInt& referenceType)
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

    bool doReferencesToItemIdExist(const MetaBox& metaBox, const uint32_t itemId, const FourCCInt& referenceType)
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
        for (typename Container::const_iterator it = container.begin(); it != container.end(); ++it)
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
    template Array<SequenceImageId> makeArray(const IdVector& container);
    template Array<TimestampIDPair> makeArray(const Vector<TimestampIDPair>& container);
    template Array<char> makeArray(const String& container);
    template Array<std::int32_t> makeArray(const Vector<std::int32_t>& container);
    template Array<std::uint32_t> makeArray(const Vector<std::uint32_t>& container);
    template Array<std::int64_t> makeArray(const Vector<std::int64_t>& container);
    template Array<std::uint64_t> makeArray(const Vector<std::uint64_t>& container);
    template Array<std::uint8_t> makeArray(const Vector<std::uint8_t>& container);
    template Array<EditUnit> makeArray(const Vector<EditUnit>& container);

}  // namespace HEIF
