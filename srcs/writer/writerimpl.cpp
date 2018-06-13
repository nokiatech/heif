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

#include "writerimpl.hpp"
#include <cstring>
#include <limits>
#include "buildinfo.hpp"
#include "customallocator.hpp"
#include "jpegparser.hpp"

using namespace std;

namespace HEIF
{
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
        CUSTOM_DELETE(writer, Writer);
    }

    HEIF_DLL_PUBLIC const char* Writer::GetVersion()
    {
        return BuildInfo::Version;
    }

    WriterImpl::WriterImpl()
        : mState(State::UNINITIALIZED)
        , mAllDecoderConfigs()
        , mMediaData()
        , mImageSequences()
        , mImageCollection()
        , mEntityGroups()
        , mMetadataItems()
        , mIspeIndexes()
        , mDecoderConfigs()
        , mDecoderConfigIndexToSize()
        , mJpegDimensions()
        , mMatrix({0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000})
        , mFileTypeBox()
        , mMetaBox()
        , mMovieBox()
        , mMediaDataBox()
    {
    }

    WriterImpl::~WriterImpl()
    {
        clear();
    }

    void WriterImpl::clear()
    {
        Context::reset();
        Track::reset();

        mAllDecoderConfigs.clear();
        mMediaData.clear();
        mImageSequences.clear();
        mImageCollection = {};
        mEntityGroups.clear();
        mMetadataItems.clear();
        mIspeIndexes.clear();
        mDecoderConfigs.clear();
        mDecoderConfigIndexToSize.clear();
        mJpegDimensions.clear();
        mMatrix.clear();
        mMatrix = {0x00010000, 0, 0, 0, 0x00010000, 0, 0, 0, 0x40000000};

        mFileTypeBox  = {};
        mMetaBox      = {};
        mMediaDataBox = {};
        mMovieBox.clear();

        mMdatOffset     = 0;
        mInitialMdat    = false;
        mPrimaryItemSet = false;

        if (mState == State::WRITING)
        {
            mFile.close();
            std::remove(mFilename.c_str());
            mFilename.clear();
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

        mFilename = outputConfig.fileName;
        mFile.open(mFilename.c_str(), std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
        if (!mFile.is_open())
        {
            return ErrorCode::FILE_OPEN_ERROR;
        }

        for (auto brand : outputConfig.compatibleBrands)
        {
            mFileTypeBox.addCompatibleBrand(brand.value);
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
            writeBitstream(output, mFile);

            // Write Media Data Box 'mdat' header. We can not know input data size, so use 64-bit large size field for
            // the box.
            mMdatOffset = static_cast<uint64_t>(mFile.tellp());
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

        if (((aData.mediaFormat == MediaFormat::AVC) || (aData.mediaFormat == MediaFormat::HEVC) ||
             (aData.mediaFormat == MediaFormat::AAC)) &&
            !mAllDecoderConfigs.count(aData.decoderConfigId))
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
            // todo: was possible to not have decoder config?
        }

        MediaData mediaData       = {};
        mediaData.id              = Context::getValue();
        mediaData.mediaFormat     = aData.mediaFormat;
        mediaData.decoderConfigId = aData.decoderConfigId;
        mediaData.size            = aData.size;

        mMediaDataSize += mediaData.size;

        if (aData.mediaFormat == MediaFormat::JPEG)
        {
            JpegParser parser;
            const JpegParser::JpegInfo info = parser.parse(aData.data, static_cast<unsigned int>(aData.size));
            if (!info.parsingOk)
            {
                return ErrorCode::MEDIA_PARSING_ERROR;
            }
            mJpegDimensions[mediaData.id] = {info.imageWidth, info.imageHeight};
        }

        if (mInitialMdat)
        {
            mediaData.offset = static_cast<uint64_t>(mFile.tellp());
            mFile.write(reinterpret_cast<char*>(aData.data), static_cast<streamsize>(aData.size));
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
            mMediaDataBox.writeBox(mFile);
        }
        mFile.close();

        mState = State::UNINITIALIZED;

        return ErrorCode::OK;
    }

    void WriterImpl::finalizeMdatBox()
    {
        BitStream output;
        const int64_t position = mFile.tellp();
        output.write64Bits(static_cast<uint64_t>(position) - mMdatOffset);
        const int64_t LARGESIZE_OFFSET = 8;
        mFile.seekp(static_cast<int64_t>(mMdatOffset) + LARGESIZE_OFFSET);
        writeBitstream(output, mFile);
        mFile.seekp(position);
    }

    void writeBitstream(BitStream& input, std::ofstream& output)
    {
        const Vector<uint8_t>& data = input.getStorage();
        output.write(reinterpret_cast<const char*>(data.data()), static_cast<streamsize>(data.size()));
    }
}  // namespace HEIF
