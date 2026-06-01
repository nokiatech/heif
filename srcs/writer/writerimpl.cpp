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

#include "writerimpl.hpp"

#include "heiferrorinfo.h"

#include <limits>

#include "buildinfo.hpp"
#include "customallocator.hpp"
#include "jpegparser.hpp"
#include "OutputStreamInterface.h"
#include "bitstream.hpp"
#include "fourccint.hpp"
#include "heifexport.h"
#include "idgenerators.hpp"
#include "itemlocationbox.hpp"
#include "typecombinationbox.hpp"
#include <algorithm>
#include <iterator>
#include <list>
#include <map>
#include <new>
#include <stdexcept>
#include <utility>
#include <vector>

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

            if (aData.mediaFormat != MediaFormat::JPEG)
            {
                throw std::invalid_argument("embedJPEGDecoderConfig: expected JPEG media format");
            }
            if (aDecoderSpecificInfo.decSpecInfoType != DecoderSpecInfoType::JPEG)
            {
                throw std::invalid_argument("embedJPEGDecoderConfig: expected JPEG decoder spec info type");
            }

            completeImage.reserve(decoderSpecInfoData.size + aData.size);
            auto toImage = std::back_inserter(completeImage);
            std::copy(decoderSpecInfoData.begin(), decoderSpecInfoData.end(), toImage);
            std::copy(aData.data, aData.data + aData.size, toImage);

            return completeImage;
        }

        void writeBitstream(BitStream& input, OutputStreamInterface* output)
        {
            const Vector<uint8_t>& data = input.getStorage();
            output->write(data.data(), data.size());
        }
    }  // namespace

    HEIF_DLL_PUBLIC ErrorCode Writer::SetCustomAllocator(CustomAllocator* customAllocator)
    {
        if (!setCustomAllocator(customAllocator))
        {
            return HEIF_FAIL(ErrorCode::ALLOCATOR_ALREADY_SET,
                             ErrorOrigin::Writer_State,
                             "writerimpl.cpp: custom allocator has already been installed");
        }
        else
        {
            return ErrorCode::OK;
        }
    }

    HEIF_DLL_PUBLIC Writer* Writer::Create()
    {
        // OOM-safe construction. Mirror of Reader::Create:
        // placement-new on the result of customAllocate();
        // customAllocate now throws std::bad_alloc when
        // the embedder's CustomAllocator returns nullptr, and the ctor
        // body itself can throw during member init under continuing
        // allocator pressure. Translate every escape into a null return so
        // the embedder sees the documented failure mode (Writer::Create
        // returning nullptr) instead of the host process unwinding through
        // a noexcept boundary.
        try
        {
            void* mem = customAllocate(sizeof(WriterImpl));
            try
            {
                return new (mem) WriterImpl();
            }
            catch (...)
            {
                customDeallocate(mem);
                return nullptr;
            }
        }
        catch (...)
        {
            return nullptr;
        }
    }

    HEIF_DLL_PUBLIC void Writer::Destroy(Writer* writer) noexcept
    {
        // Swallow any unexpected throw from a destructor / allocator hook;
        // Destroy() is contractually noexcept (JNI / COM bindings assume
        // no-throw and would call std::terminate otherwise).
        try
        {
            writer->~Writer();
            customDeallocate(writer);
        }
        catch (...)
        {
            if (::HEIF::hasErrorSink())
            {
                ::HEIF::ErrorInfo info;
                info.setCode     (::HEIF::ErrorCode::UNKNOWN_ERROR);
                info.setOrigin   (::HEIF::ErrorOrigin::Writer_BoxBuild);
                info.setMessage  ("Writer::Destroy: destructor threw (swallowed for noexcept contract)");
                info.setCauseWhat("(unknown C++ exception)");
                info.setFile     (__FILE__);
                info.setLine     (__LINE__);
                info.setValid    (true);
                ::HEIF::setErrorInfo(info);
            }
        }
    }

    HEIF_DLL_PUBLIC const char* Writer::GetVersion()
    {
        return BuildInfo::Version;
    }

    WriterImpl::WriterImpl()
        : 
         mAllDecoderConfigs()
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
        // Only delete the output handle if WE constructed it.
        // If the user supplied OutputConfig::outputStream the pointer is
        // theirs to free; deleting it here corrupts their heap (FFI bindings
        // hold the same pointer and call delete/free on their side).
        // clear() already nulls mFile when it owned the handle and was in
        // WRITING state; finalize() does the same on its happy path. The
        // remaining case is an init-fail/early-teardown where mFile may
        // still hold a library-owned FileStream we constructed.
        if (mOwnsOutputHandle)
        {
            delete mFile;
        }
        mFile = nullptr;
    }

    void WriterImpl::clear()
    {
        Context::reset();
        Track::reset();

        mAllDecoderConfigs.clear();
        mVvcParsedConfigs.clear();
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
        mNextSubpicId       = 0;

        if (mState == State::WRITING)
        {
            // remove() asks the underlying sink to discard its work; this
            // is on the user's stream (if supplied) or on our internal
            // FileStream, and is safe in both cases per the
            // OutputStreamInterface contract.
            mFile->remove();
            // Only delete the underlying handle if WE built it.
            // When the embedder supplied OutputConfig::outputStream the
            // pointer is theirs to free.
            if (mOwnsOutputHandle)
            {
                delete mFile;
            }
            mFile = nullptr;
        }

        mState = State::UNINITIALIZED;
    }

    ErrorCode WriterImpl::WriterImpl::initialize(const OutputConfig& outputConfig)
    {
        HEIF_TRY
        {
        if (mState != State::UNINITIALIZED)
        {
            return HEIF_FAIL(ErrorCode::ALREADY_INITIALIZED,
                             ErrorOrigin::Writer_State,
                             "writerimpl.cpp: initialize() called twice");
        }

        clear();
        Context::reset();
        Track::reset();

        if (outputConfig.progressiveFile || outputConfig.writeMini)
        {
            mInitialMdat = false;
        }
        else
        {
            if (outputConfig.majorBrand == FourCC())
            {
                return HEIF_FAIL(ErrorCode::BRANDS_NOT_SET,
                                 ErrorOrigin::Writer_State,
                                 "writerimpl.cpp: major brand has not been set (call setMajorBrand first)");
            }
            mInitialMdat = true;
        }

        if(outputConfig.writeMini && outputConfig.itemCreationTimes){
            return HEIF_FAIL(ErrorCode::NOT_APPLICABLE,
                             ErrorOrigin::Writer_State,
                             "writerimpl.cpp: operation not valid for this file/state");
        }else{
            mWriteItemCreationTimes = outputConfig.itemCreationTimes;
        }

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
            return HEIF_FAIL(ErrorCode::FILE_OPEN_ERROR,
                             ErrorOrigin::Writer_FileIO,
                             "writerimpl.cpp: could not open the underlying file/stream");
        }

        mWriteMini = outputConfig.writeMini;

        //Disable writing of compatible brands and etyp for mini
        if(!mWriteMini){
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
        }

        if (outputConfig.majorBrand != FourCC())
        {
            mFileTypeBox.setMajorBrand(outputConfig.majorBrand.value);
            if(!mWriteMini){
                mFileTypeBox.addCompatibleBrand(outputConfig.majorBrand.value);
            }
        }

        //Add minor version
        if(outputConfig.minorVersion != 0){
            mFileTypeBox.setMinorVersion(outputConfig.minorVersion);
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
            mMdatOffset = mFile->tellp();
            output.clear();
            output.write32Bits(1);  // size field, value 1 implies using largesize field instead.
            output.write32Bits(FourCCInt("mdat").getUInt32());  // boxtype field
            output.write64Bits(0);                              // largesize field
            writeBitstream(output, mFile);
        }

        mState = State::WRITING;
        return ErrorCode::OK;
        }
        HEIF_CATCH_AND_RETURN(ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::initialize: exception escaped from box-build path")
    }

    ErrorCode WriterImpl::feedDecoderConfig(const Array<DecoderSpecificInfo>& config, DecoderConfigId& decoderConfigId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::feedDecoderConfig: called before initialize()");
        }

        /// @todo Check parameter set integrity?
        decoderConfigId                     = Context::getValue();
        mAllDecoderConfigs[decoderConfigId] = config;

        // One-shot parse of SPS/PPS for VVC configs so finalize-time consumers
        // (writeVvcSubpicOrderEntry) can read sps/pps id and subpic mapping
        // info without invoking VvcParser::parseSps / parsePps a second time.
        // Failures are non-fatal; the entry simply stays unset and the legacy
        // code path handles it.
        for (const auto& nalUnit : config)
        {
            const auto bytes = vectorize(nalUnit.decSpecInfoData);
            if (bytes.empty())
            {
                continue;
            }
            try
            {
                if (nalUnit.decSpecInfoType == DecoderSpecInfoType::VVC_SPS)
                {
                    auto& entry  = mVvcParsedConfigs[decoderConfigId];
                    entry.sps    = VvcParser::parseSps(bytes);
                    entry.hasSps = true;
                }
                else if (nalUnit.decSpecInfoType == DecoderSpecInfoType::VVC_PPS)
                {
                    auto& entry  = mVvcParsedConfigs[decoderConfigId];
                    entry.pps    = VvcParser::parsePps(bytes);
                    entry.hasPps = true;
                }
            }
            catch (...)
            {
                // Defensive: leave the entry unset and let downstream code
                // re-attempt at finalize time if needed.
            }
        }

        return ErrorCode::OK;
        }
        HEIF_CATCH_AND_RETURN(ErrorOrigin::Writer_DecoderConfig,
                              "WriterImpl::feedDecoderConfig: exception escaped")
    }

    ErrorCode WriterImpl::feedMediaData(const Data& aData, MediaDataId& aMediaDataId)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::feedMediaData(Data): called before initialize()");
        }

        ErrorCode error = validateFedMediaData(aData);
        if (error != ErrorCode::OK)
        {
            return error;
        }

        return storeFedMediaData(aData, aMediaDataId);
        }
        HEIF_CATCH_AND_RETURN(ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::feedMediaData(Data): exception escaped")
    }

    ErrorCode WriterImpl::validateFedMediaData(const Data& aData)
    {
        HEIF_TRY
        {
        if ((((aData.mediaFormat == MediaFormat::AVC) || (aData.mediaFormat == MediaFormat::HEVC) ||
              (aData.mediaFormat == MediaFormat::AAC) || (aData.mediaFormat == MediaFormat::VVC) ||
              (aData.mediaFormat == MediaFormat::VVC_SUBPIC)) &&
             !mAllDecoderConfigs.count(aData.decoderConfigId)) &&
            !(aData.mediaFormat == MediaFormat::JPEG))
        {
            return HEIF_FAIL(ErrorCode::INVALID_DECODER_CONFIG_ID,
                             ErrorOrigin::Writer_DecoderConfig,
                             "WriterImpl::validateFedMediaData: decoder config id not registered (feedMediaData called before feedDecoderConfig for that id)");
        }

        if (aData.mediaFormat == MediaFormat::INVALID)
        {
            return HEIF_FAIL(ErrorCode::INVALID_MEDIA_FORMAT,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::validateFedMediaData: unrecognised or unsupported media format");
        }
        else if (aData.mediaFormat == MediaFormat::AVC)
        {
            Array<DecoderSpecificInfo>& decoderSpecInfo = mAllDecoderConfigs.at(aData.decoderConfigId);
            if (decoderSpecInfo.size >= 2)
            {
                DecoderSpecInfoType type = decoderSpecInfo.elements[0].decSpecInfoType;
                if ((type != DecoderSpecInfoType::AVC_SPS) && (type != DecoderSpecInfoType::AVC_PPS))
                {
                    return HEIF_FAIL(ErrorCode::INVALID_DECODER_CONFIG_ID,
                                     ErrorOrigin::Writer_DecoderConfig,
                                     "WriterImpl::validateFedMediaData: AVC media data references a decoder config whose first NAL is not SPS or PPS");
                }
            }
            else
            {
                return HEIF_FAIL(ErrorCode::INVALID_DECODER_CONFIG_ID,
                                 ErrorOrigin::Writer_DecoderConfig,
                                 "WriterImpl::validateFedMediaData: AVC decoder config has fewer than 2 specific-info entries (need at least SPS+PPS)");
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
                    return HEIF_FAIL(ErrorCode::INVALID_DECODER_CONFIG_ID,
                                     ErrorOrigin::Writer_DecoderConfig,
                                     "WriterImpl::validateFedMediaData: HEVC media data references a decoder config whose first NAL is not SPS, PPS or VPS");
                }
            }
            else
            {
                return HEIF_FAIL(ErrorCode::INVALID_DECODER_CONFIG_ID,
                                 ErrorOrigin::Writer_DecoderConfig,
                                 "WriterImpl::validateFedMediaData: HEVC decoder config has fewer than 3 specific-info entries (need at least VPS+SPS+PPS)");
            }
        }
        else if ((aData.mediaFormat == MediaFormat::VVC) || (aData.mediaFormat == MediaFormat::VVC_SUBPIC))
        {
            Array<DecoderSpecificInfo>& decoderSpecInfo = mAllDecoderConfigs.at(aData.decoderConfigId);
            if (decoderSpecInfo.size >= 2)
            {
                DecoderSpecInfoType type = decoderSpecInfo.elements[0].decSpecInfoType;
                if ((type != DecoderSpecInfoType::VVC_PPS) && (type != DecoderSpecInfoType::VVC_SPS) &&
                    (type != DecoderSpecInfoType::VVC_VPS))
                {
                    return HEIF_FAIL(ErrorCode::INVALID_DECODER_CONFIG_ID,
                                     ErrorOrigin::Writer_DecoderConfig,
                                     "WriterImpl::validateFedMediaData: VVC media data references a decoder config whose first NAL is not SPS, PPS or VPS");
                }
            }
            else
            {
                return HEIF_FAIL(ErrorCode::INVALID_DECODER_CONFIG_ID,
                                 ErrorOrigin::Writer_DecoderConfig,
                                 "WriterImpl::validateFedMediaData: VVC decoder config has fewer than 2 specific-info entries (need at least SPS+PPS)");
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
                    return HEIF_FAIL(ErrorCode::INVALID_DECODER_CONFIG_ID,
                                     ErrorOrigin::Writer_DecoderConfig,
                                     "WriterImpl::validateFedMediaData: AAC decoder config first entry is not AudioSpecificConfig");
                }
            }
            else
            {
                return HEIF_FAIL(ErrorCode::INVALID_DECODER_CONFIG_ID,
                                 ErrorOrigin::Writer_DecoderConfig,
                                 "WriterImpl::validateFedMediaData: AAC decoder config must contain exactly one AudioSpecificConfig entry");
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
                        return HEIF_FAIL(ErrorCode::INVALID_DECODER_CONFIG_ID,
                                         ErrorOrigin::Writer_DecoderConfig,
                                         "WriterImpl::validateFedMediaData: JPEG decoder config first entry is not JPEG type");
                    }
                }
                else
                {
                    // If there is decoderConfigId, we expect at least some decoder config data for it
                    return HEIF_FAIL(ErrorCode::INVALID_DECODER_CONFIG_ID,
                                     ErrorOrigin::Writer_DecoderConfig,
                                     "WriterImpl::validateFedMediaData: JPEG decoder config must contain exactly one entry");
                }
            }
            else
            {
                return ErrorCode::OK;
            }
        }
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::validateFedMediaData: exception escaped firewall")
    }

    ErrorCode WriterImpl::storeFedMediaData(const Data& aData, MediaDataId& aMediaDataId)
    {
        HEIF_TRY
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

                    info = parser.parse(completeImage.data(), static_cast<unsigned int>(completeImage.size()));
                }
                else if (decoderSpecInfo && decoderSpecInfo->size > 1)
                {
                    // we don't really expect this, the error should have occurred earlier
                    return HEIF_FAIL(ErrorCode::INVALID_DECODER_CONFIG_ID,
                                     ErrorOrigin::Writer_DecoderConfig,
                                     "WriterImpl::storeFedMediaData: decoder config id not found");
                }
                else
                {
                    // But if there's no decoder specific info, just work on the original data
                    info = parser.parse(aData.data, static_cast<unsigned int>(aData.size));
                }

                if (!info.parsingOk)
                {
                    return HEIF_FAIL(ErrorCode::MEDIA_PARSING_ERROR,
                                     ErrorOrigin::Writer_BoxBuild,
                                     "WriterImpl::storeFedMediaData: media bitstream is malformed");
                }
                mJpegDimensions[mediaData.id] = {info.imageWidth, info.imageHeight};
            }

            if (mInitialMdat)
            {
                mediaData.offset = mFile->tellp();
                mFile->write(aData.data, aData.size);
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
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::storeFedMediaData: exception escaped firewall")
    }

    ErrorCode WriterImpl::createEntityGroup(const FourCC& type, GroupId& id)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::createEntityGroup: called before initialize()");
        }

        EntityGroup group;
        group.type = type;
        group.id   = Context::getValue();

        mEntityGroups[group.id] = group;

        id = group.id;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::createEntityGroup: exception escaped firewall")
    }

    ErrorCode WriterImpl::createAlternativesGroup(GroupId& id)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::createAlternativesGroup: called before initialize()");
        }

        return createEntityGroup("altr", id);
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::createAlternativesGroup: exception escaped firewall")
    }

    ErrorCode WriterImpl::createEquivalenceGroup(GroupId& id)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::createEquivalenceGroup: called before initialize()");
        }

        return createEntityGroup("eqiv", id);
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::createEquivalenceGroup: exception escaped firewall")
    }

    ErrorCode WriterImpl::addToGroup(const GroupId& groupId, const ImageId& id)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addToGroup: called before initialize()");
        }

        if (!checkImageIds({id}))
        {
            return HEIF_FAIL(ErrorCode::INVALID_ITEM_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addToGroup: item id not found");
        }

        if (mEntityGroups.count(groupId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_GROUP_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addToGroup: group id not found");
        }

        EntityGroup::Entity entry;
        entry.type = EntityGroup::Entity::Type::ITEM;
        entry.id   = id.get();
        mEntityGroups[groupId].entities.push_back(entry);
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addToGroup: exception escaped firewall")
    }

    ErrorCode WriterImpl::createTrackGroup(const FourCC& type, TrackGroupId& id)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::createTrackGroup: called before initialize()");
        }

        TrackGroup group;
        group.type = type;
        group.id   = Context::getValue();

        mTrackGroups[group.id] = group;

        id = group.id;
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::createTrackGroup: exception escaped firewall")
    }

    ErrorCode WriterImpl::addToGroup(const TrackGroupId& trackGroupId, const SequenceId& id)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addToGroup: called before initialize()");
        }

        if (mImageSequences.count(id) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_SEQUENCE_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addToGroup: sequence (track) id not found");
        }

        if (mTrackGroups.count(trackGroupId) == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_GROUP_ID,
                             ErrorOrigin::Writer_BoxBuild,
                             "WriterImpl::addToGroup: group id not found");
        }

        mTrackGroups[trackGroupId].trackIds.insert(id);

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addToGroup: exception escaped firewall")
    }

    ErrorCode WriterImpl::setMajorBrand(const FourCC& brand)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::setMajorBrand(FourCC): called before initialize()");
        }

        if (mInitialMdat)
        {
            return HEIF_FAIL(ErrorCode::FTYP_ALREADY_WRITTEN,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::setMajorBrand(FourCC): operation invalid after the ftyp box has been written");
        }

        mFileTypeBox.setMajorBrand(brand.value);
        mFileTypeBox.addCompatibleBrand(brand.value);
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::setMajorBrand(FourCC): exception escaped firewall")
    }

    ErrorCode WriterImpl::addCompatibleBrand(const FourCC& brand)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addCompatibleBrand(FourCC): called before initialize()");
        }

        if (mInitialMdat)
        {
            return HEIF_FAIL(ErrorCode::FTYP_ALREADY_WRITTEN,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addCompatibleBrand(FourCC): operation invalid after the ftyp box has been written");
        }

        mFileTypeBox.addCompatibleBrand(brand.value);
        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addCompatibleBrand(FourCC): exception escaped firewall")
    }

    ErrorCode WriterImpl::addCompatibleBrandCombination(const Array<FourCC>& compatibleBrandCombination)
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::addCompatibleBrandCombination(Arra): called before initialize()");
        }

        // Check if file type box has already been written. If so, return error in case a new type combination box would
        // be needed.
        if (mInitialMdat)
        {
            Vector<FourCCInt> brandVector;
            for (const auto& brand : compatibleBrandCombination)
            {
                brandVector.emplace_back(brand.value);
            }
            if (mExtendedTypeBox.checkCompatibility(brandVector) == false)
            {
                return HEIF_FAIL(ErrorCode::FTYP_ALREADY_WRITTEN,
                                 ErrorOrigin::Writer_State,
                                 "WriterImpl::addCompatibleBrandCombination(Arra): operation invalid after the ftyp box has been written");
            }
        }

        if (compatibleBrandCombination.size == 0)
        {
            return HEIF_FAIL(ErrorCode::INVALID_FUNCTION_PARAMETER,
                             ErrorOrigin::Writer_Validation,
                             "WriterImpl::addCompatibleBrandCombination(Arra): invalid argument");
        }

        TypeCombinationBox tyco;
        for (auto const& brand : compatibleBrandCombination)
        {
            tyco.addCompatibleBrand(brand.value);
        }
        mExtendedTypeBox.addTypeCombinationBox(tyco);

        return ErrorCode::OK;
            }
        HEIF_CATCH_AND_RETURN(::HEIF::ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::addCompatibleBrandCombination(Arra): exception escaped firewall")
    }

    ErrorCode WriterImpl::finalize()
    {
        HEIF_ENTRY_TRY
        {
        if (mState != State::WRITING)
        {
            return HEIF_FAIL(ErrorCode::UNINITIALIZED,
                             ErrorOrigin::Writer_State,
                             "WriterImpl::finalize: called before initialize()");
        }

        BitStream output;

        //Separate writing sequence for mini as
        //mini writing is vastly different from normal heif
        if(mWriteMini){
            //Write the only two boxes in a Mini (ftyp, mini)
            mFileTypeBox.writeBox(output);

            // Compact VVC item data is single-layer only (ISO/IEC 14496-15).
            // Gate on brand AND on the parsed SPS multilayer-enabled flag.
            bool goCompact = mFileTypeBox.getMinorVersion() == FourCCInt("vvi3").getUInt32();
            if (goCompact) {
                for (const auto& entry : mVvcParsedConfigs) {
                    if (entry.second.hasSps &&
                        entry.second.sps.mNativePtl.mPtlMultilayerEnabledFlag != 0) {
                        goCompact = false;
                        break;
                    }
                }
            }
            mMetaBox.setWriteCompact(goCompact);

            ErrorCode miniWriteError = finalizeMetaBox();
            if(miniWriteError != ErrorCode::OK){ return miniWriteError; }

            miniWriteError = mMetaBox.writeMiniBox(output);
            if(miniWriteError != ErrorCode::OK){ return miniWriteError; }

            //Write writer buffer to file and clear it
            writeBitstream(output, mFile);
            output.clear();

            //Write raw item payloads to buffer
            const List<Vector<uint8_t>> data = mMediaDataBox.getRawItemData();
            for (const auto& dataBlock : data){
                if(goCompact){
                    const uint8_t TRIM_SIZE = mMetaBox.getItemLocationBox().getLengthSize() + 2;
                    //Skip the writing the bytes from itemdata beginning, as they are not supported in compact format
                    mFile->write(dataBlock.data() + TRIM_SIZE, dataBlock.size() - TRIM_SIZE);
                }else{
                    mFile->write(dataBlock.data(), dataBlock.size());
                }
            }

            //Write item payloads from buffer to file
            writeBitstream(output, mFile);
            output.clear();
        }else{

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
                    return HEIF_FAIL(ErrorCode::BRANDS_NOT_SET,
                                     ErrorOrigin::Writer_State,
                                     "WriterImpl::finalize: major brand has not been set (call setMajorBrand first)");
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
                    mFile->write(dataBlock.data(), dataBlock.size());
                }
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
        HEIF_CATCH_AND_RETURN(ErrorOrigin::Writer_BoxBuild,
                              "WriterImpl::finalize: exception escaped from box-write path")
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
