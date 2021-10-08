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

#include "writerimpl.hpp"

#include <cassert>
#include <cstring>
#include <limits>

#include "buildinfo.hpp"
#include "customallocator.hpp"
#include "jpegparser.hpp"

using namespace std;

namespace HEIF
{
    namespace
    {
        std::vector<uint8_t> embedJPEGDecoderConfig(const HEIF::DecoderSpecificInfo& aDecoderSpecificInfo,
                                                    const Data& aData)
        {
            std::vector<uint8_t> completeImage;
            const Array<uint8_t>& decoderSpecInfoData = aDecoderSpecificInfo.decSpecInfoData;

            assert(aData.mediaFormat == MediaFormat::JPEG);
            assert(aDecoderSpecificInfo.decSpecInfoType == DecoderSpecInfoType::JPEG);

            completeImage.reserve(decoderSpecInfoData.size + aData.size);
            auto toImage = std::back_inserter(completeImage);
            std::copy(decoderSpecInfoData.begin(), decoderSpecInfoData.end(), toImage);
            std::copy(aData.data, aData.data + aData.size, toImage);

            return completeImage;
        }

        void writeBitstream(BitStream& input, OutputStreamInterface* output)
        {
            const Vector<uint8_t>& data = input.getStorage();
            output->write(data.data(), static_cast<uint64_t>(data.size()));
        }
    }  // namespace

    HEIF_DLL_PUBLIC ErrorCode Writer::SetCustomAllocator(CustomAllocator* customAllocator)
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

    HEIF_DLL_PUBLIC Writer* Writer::Create()
    {
        return CUSTOM_NEW(WriterImpl, ());
    }

    HEIF_DLL_PUBLIC void Writer::Destroy(Writer* writer)
    {
        CUSTOM_DELETE(writer, Writer);  // Extra semicolon prevents clang-format from wrapping this to one line.
    }

    HEIF_DLL_PUBLIC const char* Writer::GetVersion()
    {
        return BuildInfo::Version;
    }

    WriterImpl::WriterImpl()
        : mState(State::UNINITIALIZED)
        , mAllDecoderConfigs()
        , mMediaData()
        , mMediaDataHashes()
        , mImageSequences()
        , mImageCollection()
        , mEntityGroups()
        , mTrackGroups()
        , mMetadataItems()
        , mIspeIndexes()
        , mDecoderConfigs()
        , mDecoderConfigIndexToSize()
        , mJpegDimensions()
        , mMatrix({0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000})
        , mFileTypeBox()
        , mExtendedTypeBox()
        , mMetaBox()
        , mMovieBox()
        , mMediaDataBox()
    {
        mFile = nullptr;
    }

    WriterImpl::~WriterImpl()
    {
        clear();
        delete mFile;
    }

    void WriterImpl::clear()
    {
        Context::reset();
        Track::reset();

        mAllDecoderConfigs.clear();
        mMediaData.clear();
        mMediaDataHashes.clear();
        mImageSequences.clear();
        mImageCollection = {};
        mEntityGroups.clear();
        mTrackGroups.clear();
        mMetadataItems.clear();
        mIspeIndexes.clear();
        mDecoderConfigs.clear();
        mDecoderConfigIndexToSize.clear();
        mJpegDimensions.clear();
        mMatrix.clear();
        mMatrix = {0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000};

        mFileTypeBox     = {};
        mExtendedTypeBox = {};
        mMetaBox         = {};
        mMediaDataBox    = {};
        mMovieBox.clear();

        mMdatOffset     = 0;
        mInitialMdat    = false;
        mPrimaryItemSet = false;

        mPredRrefPropertyId = 0;

        if (mState == State::WRITING)
        {
            mFile->remove();
            delete mFile;
            mFile = nullptr;
        }

        mState = State::UNINITIALIZED;
    }

    ErrorCode WriterImpl::WriterImpl::initialize(const OutputConfig& outputConfig)
    {
        if (mState != State::UNINITIALIZED)
        {
            return ErrorCode::ALREADY_INITIALIZED;
        }

        clear();
        Context::reset();
        Track::reset();

        if (outputConfig.progressiveFile)
        {
            mInitialMdat = false;
        }
        else
        {
            if (outputConfig.majorBrand == FourCC())
            {
                return ErrorCode::BRANDS_NOT_SET;
            }
            mInitialMdat = true;
        }

        mWriteItemCreationTimes = outputConfig.itemCreationTimes;

        mFile = nullptr;
        if (outputConfig.outputStream)
        {
            mFile             = outputConfig.outputStream;
            mOwnsOutputHandle = false;
        }
        else if ((outputConfig.fileName) && (outputConfig.fileName[0] != 0))
        {
            mFile             = ConstructFileStream(outputConfig.fileName);
            mOwnsOutputHandle = true;
        }
        if (mFile == nullptr)
        {
            return ErrorCode::FILE_OPEN_ERROR;
        }

        for (const auto& brand : outputConfig.compatibleBrands)
        {
            mFileTypeBox.addCompatibleBrand(brand.value);
        }

        for (const auto& combination : outputConfig.compatibleCombinations)
        {
            TypeCombinationBox tyco;
            for (const auto& brand : combination)
            {
                tyco.addCompatibleBrand(brand.value);
            }
            mExtendedTypeBox.addTypeCombinationBox(tyco);
        }

        if (outputConfig.majorBrand != FourCC())
        {
            mFileTypeBox.setMajorBrand(outputConfig.majorBrand.value);
            mFileTypeBox.addCompatibleBrand(outputConfig.majorBrand.value);
        }

        if (mInitialMdat)
        {
            BitStream output;
            mFileTypeBox.writeBox(output);
            if (!mExtendedTypeBox.getTypeCombinationBoxes().empty())
            {
                mExtendedTypeBox.writeBox(output);
            }
            writeBitstream(output, mFile);

            // Write Media Data Box 'mdat' header. We can not know input data size, so use 64-bit large size field for
            // the box.
            mMdatOffset = static_cast<uint64_t>(mFile->tellp());
            output.clear();
            output.write32Bits(1);  // size field, value 1 implies using largesize field instead.
            output.write32Bits(FourCCInt("mdat").getUInt32());  // boxtype field
            output.write64Bits(0);                              // largesize field
            writeBitstream(output, mFile);
        }

        mState = State::WRITING;
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::feedDecoderConfig(const Array<DecoderSpecificInfo>& config, DecoderConfigId& decoderConfigId)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        /// @todo Check parameter set integrity?
        decoderConfigId                     = Context::getValue();
        mAllDecoderConfigs[decoderConfigId] = config;
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::feedMediaData(const Data& aData, MediaDataId& aMediaDataId)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        ErrorCode error = validateFedMediaData(aData);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        return storeFedMediaData(aData, aMediaDataId);
    }

    ErrorCode WriterImpl::validateFedMediaData(const Data& aData)
    {
        if ((((aData.mediaFormat == MediaFormat::AVC) || (aData.mediaFormat == MediaFormat::HEVC) ||
              (aData.mediaFormat == MediaFormat::AAC)) &&
             !mAllDecoderConfigs.count(aData.decoderConfigId)) &&
            !(aData.mediaFormat == MediaFormat::JPEG))
        {
            return ErrorCode::INVALID_DECODER_CONFIG_ID;
        }

        if (aData.mediaFormat == MediaFormat::INVALID)
        {
            return ErrorCode::INVALID_MEDIA_FORMAT;
        }
        else if (aData.mediaFormat == MediaFormat::AVC)
        {
            Array<DecoderSpecificInfo>& decoderSpecInfo = mAllDecoderConfigs.at(aData.decoderConfigId);
            if (decoderSpecInfo.size >= 2)
            {
                DecoderSpecInfoType type = decoderSpecInfo.elements[0].decSpecInfoType;
                if ((type != DecoderSpecInfoType::AVC_SPS) && (type != DecoderSpecInfoType::AVC_PPS))
                {
                    return ErrorCode::INVALID_DECODER_CONFIG_ID;
                }
            }
            else
            {
                return ErrorCode::INVALID_DECODER_CONFIG_ID;
            }
        }
        else if (aData.mediaFormat == MediaFormat::HEVC)
        {
            Array<DecoderSpecificInfo>& decoderSpecInfo = mAllDecoderConfigs.at(aData.decoderConfigId);
            if (decoderSpecInfo.size >= 3)
            {
                DecoderSpecInfoType type = decoderSpecInfo.elements[0].decSpecInfoType;
                if ((type != DecoderSpecInfoType::HEVC_SPS) && (type != DecoderSpecInfoType::HEVC_PPS) &&
                    (type != DecoderSpecInfoType::HEVC_VPS))
                {
                    return ErrorCode::INVALID_DECODER_CONFIG_ID;
                }
            }
            else
            {
                return ErrorCode::INVALID_DECODER_CONFIG_ID;
            }
        }
        else if (aData.mediaFormat == MediaFormat::AAC)
        {
            Array<DecoderSpecificInfo>& decoderSpecInfo = mAllDecoderConfigs.at(aData.decoderConfigId);
            if (decoderSpecInfo.size == 1)
            {
                DecoderSpecInfoType type = decoderSpecInfo.elements[0].decSpecInfoType;
                if (type != DecoderSpecInfoType::AudioSpecificConfig)
                {
                    return ErrorCode::INVALID_DECODER_CONFIG_ID;
                }
            }
            else
            {
                return ErrorCode::INVALID_DECODER_CONFIG_ID;
            }
        }
        else if (aData.mediaFormat == MediaFormat::JPEG)
        {
            if (aData.decoderConfigId.get())
            {
                Array<DecoderSpecificInfo>& decoderSpecInfo = mAllDecoderConfigs.at(aData.decoderConfigId);
                if (decoderSpecInfo.size == 1)
                {
                    DecoderSpecInfoType type = decoderSpecInfo.elements[0].decSpecInfoType;
                    if (type != DecoderSpecInfoType::JPEG)
                    {
                        return ErrorCode::INVALID_DECODER_CONFIG_ID;
                    }
                }
                else
                {
                    // If there is decoderConfigId, we expect at least some decoder config data for it
                    return ErrorCode::INVALID_DECODER_CONFIG_ID;
                }
            }
            else
            {
                return ErrorCode::OK;
            }
        }
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::storeFedMediaData(const Data& aData, MediaDataId& aMediaDataId)
    {
        uint64_t hash = FNVHash::generate(aData.data, aData.size);
        if (mMediaDataHashes.count(hash))
        {
            aMediaDataId = mMediaDataHashes.at(hash);
        }
        else
        {
            MediaData mediaData       = {};
            mediaData.id              = Context::getValue();
            mediaData.mediaFormat     = aData.mediaFormat;
            mediaData.decoderConfigId = aData.decoderConfigId;
            mediaData.size            = aData.size;

            mMediaDataSize += mediaData.size;

            if (aData.mediaFormat == MediaFormat::JPEG)
            {
                JpegParser parser;
                JpegParser::JpegInfo info;

                const Array<DecoderSpecificInfo>* decoderSpecInfo = mAllDecoderConfigs.count(aData.decoderConfigId)
                                                                        ? &mAllDecoderConfigs.at(aData.decoderConfigId)
                                                                        : nullptr;
                // Compose a complete image out of decoder specific info and the data
                if (decoderSpecInfo && decoderSpecInfo->size == 1)
                {
                    // TODO: JpegParser::parse should work completely as a state machine, so the input can
                    // be provided in arbitrary units, ie. provide bytes until its state is "header
                    // parsed". Now we need to construct an intermediate data that includes the decoder
                    // configuration to do the verification.

                    auto completeImage = embedJPEGDecoderConfig(decoderSpecInfo->elements[0], aData);

                    info = parser.parse(completeImage.data(), completeImage.size());
                }
                else if (decoderSpecInfo && decoderSpecInfo->size > 1)
                {
                    // we don't really expect this, the error should have occurred earlier
                    return ErrorCode::INVALID_DECODER_CONFIG_ID;
                }
                else
                {
                    // But if there's no decoder specific info, just work on the original data
                    info = parser.parse(aData.data, static_cast<unsigned int>(aData.size));
                }

                if (!info.parsingOk)
                {
                    return ErrorCode::MEDIA_PARSING_ERROR;
                }
                mJpegDimensions[mediaData.id] = {info.imageWidth, info.imageHeight};
            }

            if (mInitialMdat)
            {
                mediaData.offset = mFile->tellp();
                mFile->write(aData.data, static_cast<uint64_t>(aData.size));
            }
            else
            {
                mediaData.offset = mMediaDataBox.addData(aData.data, aData.size);
                if (mMediaDataSize > std::numeric_limits<std::uint32_t>::max())
                {
                    mMediaDataBox.setLargeSize();
                }
            }

            mMediaData[mediaData.id] = mediaData;
            aMediaDataId             = mediaData.id;
            mMediaDataHashes[hash]   = mediaData.id;
        }
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::createEntityGroup(const FourCC& type, GroupId& id)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        EntityGroup group;
        group.type = type;
        group.id   = Context::getValue();

        mEntityGroups[group.id] = group;

        id = group.id;
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::createAlternativesGroup(GroupId& id)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        return createEntityGroup("altr", id);
    }

    ErrorCode WriterImpl::createEquivalenceGroup(GroupId& id)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        return createEntityGroup("eqiv", id);
    }

    ErrorCode WriterImpl::addToGroup(const GroupId& groupId, const ImageId& id)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (!checkImageIds({id}))
        {
            return ErrorCode::INVALID_ITEM_ID;
        }

        if (mEntityGroups.count(groupId) == 0)
        {
            return ErrorCode::INVALID_GROUP_ID;
        }

        EntityGroup::Entity entry;
        entry.type = EntityGroup::Entity::Type::ITEM;
        entry.id   = id.get();
        mEntityGroups[groupId].entities.push_back(entry);
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::createTrackGroup(const FourCC& type, TrackGroupId& id)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        TrackGroup group;
        group.type = type;
        group.id   = Context::getValue();

        mTrackGroups[group.id] = group;

        id = group.id;
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::addToGroup(const TrackGroupId& trackGroupId, const SequenceId& id)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (mImageSequences.count(id) == 0)
        {
            return ErrorCode::INVALID_SEQUENCE_ID;
        }

        if (mTrackGroups.count(trackGroupId) == 0)
        {
            return ErrorCode::INVALID_GROUP_ID;
        }

        mTrackGroups[trackGroupId].trackIds.insert(id);

        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::setMajorBrand(const FourCC& brand)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (mInitialMdat)
        {
            return ErrorCode::FTYP_ALREADY_WRITTEN;
        }

        mFileTypeBox.setMajorBrand(brand.value);
        mFileTypeBox.addCompatibleBrand(brand.value);
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::addCompatibleBrand(const FourCC& brand)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        if (mInitialMdat)
        {
            return ErrorCode::FTYP_ALREADY_WRITTEN;
        }

        mFileTypeBox.addCompatibleBrand(brand.value);
        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::addCompatibleBrandCombination(const Array<FourCC>& compatibleBrandCombination)
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        // Check if file type box has already been written. If so, return error in case a new type combination box would
        // be needed.
        if (mInitialMdat)
        {
            Vector<FourCCInt> brandVector;
            for (const auto& brand : compatibleBrandCombination)
            {
                brandVector.push_back(brand.value);
            }
            if (mExtendedTypeBox.checkCompatibility(brandVector) == false)
            {
                return ErrorCode::FTYP_ALREADY_WRITTEN;
            }
        }

        if (compatibleBrandCombination.size == 0)
        {
            return ErrorCode::INVALID_FUNCTION_PARAMETER;
        }

        TypeCombinationBox tyco;
        for (auto const& brand : compatibleBrandCombination)
        {
            tyco.addCompatibleBrand(brand.value);
        }
        mExtendedTypeBox.addTypeCombinationBox(tyco);

        return ErrorCode::OK;
    }

    ErrorCode WriterImpl::finalize()
    {
        if (mState != State::WRITING)
        {
            return ErrorCode::UNINITIALIZED;
        }

        BitStream output;
        if (mInitialMdat)
        {
            finalizeMdatBox();
            ErrorCode error = finalizeMetaBox();
            if (error != ErrorCode::OK)
            {
                return error;
            }
            error = generateMoovBox();
            if (error != ErrorCode::OK)
            {
                return error;
            }

            mMetaBox.writeBox(output);
            writeBitstream(output, mFile);
            output.clear();
            if (mMovieBox.getTrackBoxes().size() > 0)
            {
                mMovieBox.writeBox(output);
                writeBitstream(output, mFile);
            }
        }
        else
        {
            if ((mFileTypeBox.getMajorBrand() == 0) || (mFileTypeBox.getCompatibleBrands().size() == 0))
            {
                return ErrorCode::BRANDS_NOT_SET;
            }

            uint64_t mdatOffset = 0;
            ErrorCode error     = finalizeMetaBox();
            if (error != ErrorCode::OK)
            {
                return error;
            }
            error = generateMoovBox();
            if (error != ErrorCode::OK)
            {
                return error;
            }

            mFileTypeBox.writeBox(output);
            if (!mExtendedTypeBox.getTypeCombinationBoxes().empty())
            {
                mExtendedTypeBox.writeBox(output);
            }

            writeBitstream(output, mFile);
            mdatOffset = output.getSize();
            output.clear();
            // Calculate meta box size.
            mMetaBox.writeBox(output);
            mdatOffset += output.getSize();
            output.clear();
            // Calculate optional moov box size.
            if (mMovieBox.getTrackBoxes().size() > 0)
            {
                mMovieBox.writeBox(output);
                mdatOffset += output.getSize();
                output.clear();
            }
            mMetaBox.setItemFileOffsetBase(mdatOffset);
            updateMoovBox(mdatOffset);

            // Serialize meta box again, now with correct mdat offset, and write it.
            mMetaBox.writeBox(output);
            writeBitstream(output, mFile);
            output.clear();
            // Write optional moov box.
            if (mMovieBox.getTrackBoxes().size() > 0)
            {
                mMovieBox.writeBox(output);
                writeBitstream(output, mFile);
                output.clear();
            }
            // Finally write mdat.

            const std::pair<const ISOBMFF::BitStream&, const List<Vector<uint8_t>>&>& data =
                mMediaDataBox.getSerializedData();
            mFile->write(data.first.getStorage().data(), data.first.getStorage().size());
            for (const auto& dataBlock : data.second)
            {
                mFile->write(dataBlock.data(), static_cast<uint64_t>(dataBlock.size()));
            }
        }
        if (mOwnsOutputHandle)
        {
            delete mFile;
        }

        mFile = nullptr;

        mState = State::UNINITIALIZED;

        return ErrorCode::OK;
    }

    void WriterImpl::finalizeMdatBox()
    {
        BitStream output;
        const uint64_t position = mFile->tellp();
        output.write64Bits(position - mMdatOffset);
        const int64_t LARGESIZE_OFFSET = 8;
        mFile->seekp(mMdatOffset + LARGESIZE_OFFSET);
        writeBitstream(output, mFile);
        mFile->seekp(position);
    }

}  // namespace HEIF
