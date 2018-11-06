/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#include "CodedImageItem.h"
#include <heifreader.h>
#include <heifwriter.h>
#include <cstring>
#include "DecoderConfiguration.h"
#include "H26xTools.h"

using namespace HEIFPP;

CodedImageItem::CodedImageItem(Heif* aHeif, const HEIF::FourCC& aType, const HEIF::MediaFormat& aFormat)
    : ImageItem(aHeif, aType, false, true)
    , mFormat(aFormat)
    , mConfig(nullptr)
    , mBufferSize(0)
    , mBuffer(nullptr)
    , mMandatoryConfiguration(true)
{
}

CodedImageItem::~CodedImageItem()
{
    setDecoderConfiguration(nullptr);
    delete[] mBuffer;
    for (std::uint32_t i = 0; i < getBaseImageCount(); ++i)
    {
        setBaseImage(i, nullptr);
    }
    mBaseImages.clear();
}

DecoderConfig* CodedImageItem::getDecoderConfiguration()
{
    return mConfig;
}

const DecoderConfig* CodedImageItem::getDecoderConfiguration() const
{
    return mConfig;
}
Result CodedImageItem::setDecoderConfiguration(DecoderConfig* aConfig)
{
    if (mConfig)
        mConfig->unlink(this);
    if (aConfig)
    {
        if (aConfig->getMediaFormat() != mFormat)
        {
            // invalid configuration.
            return Result::INVALID_CONFIG;
        }
    }
    mConfig = aConfig;
    if (mConfig)
    {
        mConfig->link(this);
    }
    return Result::OK;
}
const HEIF::FourCC& CodedImageItem::getDecoderCodeType() const
{
    if (mConfig)
    {
        return mConfig->getMediaType();
    }
    return getType();
}
HEIF::MediaFormat CodedImageItem::getMediaFormat() const
{
    return Heif::mediaFormatFromFourCC(getDecoderCodeType());
}

std::uint32_t CodedImageItem::getBaseImageCount() const
{
    return (std::uint32_t) mBaseImages.size();
}
ImageItem* CodedImageItem::getBaseImage(std::uint32_t aId)
{
    if (aId < mBaseImages.size())
    {
        return mBaseImages[aId];
    }
    return nullptr;
}
const ImageItem* CodedImageItem::getBaseImage(std::uint32_t aId) const
{
    if (aId < mBaseImages.size())
    {
        return mBaseImages[aId];
    }
    return nullptr;
}


void CodedImageItem::setBaseImage(std::uint32_t aId, ImageItem* aImage)
{
    if (aId < mBaseImages.size())
    {
        if (mBaseImages[aId])
        {
            if (!removeBaseLink(mBaseImages[aId], this))
            {
                HEIF_ASSERT(false);
            }
        }
        mBaseImages[aId] = aImage;
        if (aImage)
        {
            addBaseLink(aImage, this);
        }
    }
}

void CodedImageItem::setBaseImage(ImageItem* aOldImage, ImageItem* aNewImage)
{
    for (auto it = mBaseImages.begin(); it != mBaseImages.end(); ++it)
    {
        if (aOldImage == (*it))
        {
            if (aOldImage)
            {
                if (!removeBaseLink(aOldImage, this))
                {
                    HEIF_ASSERT(false);
                }
            }
            *it = aNewImage;
            if (aNewImage)
            {
                addBaseLink(aNewImage, this);
            }
        }
    }
}

void CodedImageItem::addBaseImage(ImageItem* aImage)
{
    if (aImage)
    {
        addBaseLink(aImage, this);
        mBaseImages.push_back(aImage);
    }
}

void CodedImageItem::removeBaseImage(std::uint32_t aId)
{
    if (aId < mBaseImages.size())
    {
        auto it = mBaseImages.begin() + (std::int32_t) aId;
        if (*it)
        {
            if (!removeBaseLink(*it, this))
            {
                HEIF_ASSERT(false);
            }
        }
        mBaseImages.erase(it);
    }
}
void CodedImageItem::removeBaseImage(ImageItem* aImage)
{
    for (auto it = mBaseImages.begin(); it != mBaseImages.end();)
    {
        if (aImage == (*it))
        {
            if (aImage)
            {
                if (!removeBaseLink(aImage, this))
                {
                    HEIF_ASSERT(false);
                }
            }
            it = mBaseImages.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void CodedImageItem::reserveBaseImages(std::uint32_t aCount)
{
    if (aCount < mBaseImages.size())
    {
        for (auto it = mBaseImages.begin() + (std::int32_t) aCount; it != mBaseImages.end(); ++it)
        {
            if (!removeBaseLink(*it, this))
            {
                HEIF_ASSERT(false);
            }
            *it = nullptr;
        }
    }
    mBaseImages.resize(aCount);
}

std::uint64_t CodedImageItem::getItemDataSize() const
{
    return mBufferSize;
}

HEIF::ErrorCode CodedImageItem::loadItemData()
{
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;
    if (mBufferSize == 0)
    {
        // Umm.. no actual data in file?
        // TODO: add warnings for user.
        mBuffer = nullptr;
    }
    else
    {
        if ((getHeif() != nullptr) && (getHeif()->getReaderInstance() != nullptr))
        {
            delete[] mBuffer;
            mBuffer = nullptr;
            mBuffer = new std::uint8_t[mBufferSize];
            error   = getHeif()->getReaderInstance()->getItemData(getId(), mBuffer, mBufferSize, false);
            if (HEIF::ErrorCode::OK != error)
            {
                // Could not get the data. fail.
                mBufferSize = 0;
                delete[] mBuffer;
                mBuffer = nullptr;
            }
            else
            {
                switch (mConfig->getMediaFormat())
                {
                case HEIF::MediaFormat::AVC:
                case HEIF::MediaFormat::HEVC:
                {
                    error = NAL_State::convertToByteStream(mBuffer, mBufferSize) ? HEIF::ErrorCode::OK
                                                                                 : HEIF::ErrorCode::MEDIA_PARSING_ERROR;
                    break;
                }
                default:
                {
                    break;
                }
                }
            }
        }
        else
        {
            HEIF_ASSERT(false);
        }
    }
    return error;
}

const std::uint8_t* CodedImageItem::getItemData()
{
    if (mBuffer == nullptr)
    {
        loadItemData();
    }
    return mBuffer;
}

void CodedImageItem::setItemData(const std::uint8_t* aData, std::uint64_t aSize)
{
    mBufferSize = aSize;
    delete[] mBuffer;
    mBuffer = nullptr;
    mBuffer = new std::uint8_t[aSize];
    std::memcpy(mBuffer, aData, mBufferSize);
}

HEIF::ErrorCode CodedImageItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error = ImageItem::load(aReader, aId);
    if (HEIF::ErrorCode::OK != error)
    {
        return error;
    }

    const auto* info = getHeif()->getItemInformation(aId);
    HEIF_ASSERT(info);  // Images MUST have image information
    if (info->features & HEIF::ItemFeatureEnum::IsPreComputedDerivedImage)
    {
        HEIF::Array<HEIF::ImageId> baseIds;
        // base images for pre-derived images..
        error = aReader->getReferencedFromItemListByType(aId, "base", baseIds);
        if (HEIF::ErrorCode::OK != error)
            return error;
        // construct base images.
        mBaseImages.reserve((std::uint32_t) baseIds.size);
        for (const auto& baseId : baseIds)
        {
            ImageItem* tmp = getHeif()->constructImageItem(aReader, baseId, error);
            if (HEIF::ErrorCode::OK != error)
            {
                return error;
            }
            addBaseImage(tmp);
        }
    }

    DecoderConfig* config = getHeif()->constructDecoderConfig(aReader, aId, error);
    if (HEIF::ErrorCode::OK != error)
    {
        // Corrupted image? OR a jpeg? (jpegs have optional decoder config...)
        if (mMandatoryConfiguration)
        {
            return error;
        }
        // Ignore the error since DecoderConfiguration is not mandatory for this item.
        error = HEIF::ErrorCode::OK;
    }
    if (config)
    {
        if (setDecoderConfiguration(config) != Result::OK)
        {
            return HEIF::ErrorCode::DECODER_CONFIGURATION_ERROR;
        }
    }

    mBufferSize = info->size;
    if (getHeif()->mPreLoadMode == Heif::PreloadMode::LOAD_ALL_DATA)
    {
        error = loadItemData();
    }
    return error;
}

HEIF::ErrorCode CodedImageItem::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;
    if (mBuffer == nullptr)
    {
        // TODO: actual error is NO_MEDIA
        return HEIF::ErrorCode::BUFFER_SIZE_TOO_SMALL;
    }
    HEIF::Data fr;
    HEIF::MediaDataId mediaDataId;
    if (mConfig)
    {
        if (mConfig->getId() == Heif::InvalidDecoderConfig)
        {
            error = mConfig->save(aWriter);
            if (HEIF::ErrorCode::OK != error)
                return error;
        }
        fr.decoderConfigId = mConfig->getId();
    }
    else
    {
        if (mMandatoryConfiguration)
        {
            // no configuration and it's mandatory
            return HEIF::ErrorCode::INVALID_DECODER_CONFIG_ID;
        }
        fr.decoderConfigId = 0;
    }


    std::uint64_t size = 0;
    std::uint8_t* data = nullptr;

    if (!getBitstream(data, size))
    {
        return HEIF::ErrorCode::INVALID_MEDIA_FORMAT;
    }
    fr.size        = size;
    fr.data        = data;
    fr.mediaFormat = mFormat;

    if (fr.data == nullptr)
    {
        // mediadata not set, or corrupted.
        return HEIF::ErrorCode::INVALID_MEDIA_FORMAT;
    }

    error = aWriter->feedMediaData(fr, mediaDataId);

    // free temporary data.
    delete[] fr.data;

    if (HEIF::ErrorCode::OK != error)
        return error;
    HEIF::ImageId newId;
    error = aWriter->addImage(mediaDataId, newId);
    setId(newId);
    if (HEIF::ErrorCode::OK != error)
        return error;

    // save base images.. needs to be done here since we only get the correct mId AFTER aWriter->addImage.
    if (!mBaseImages.empty())
    {
        HEIF::Array<HEIF::ImageId> toImageIds(mBaseImages.size());
        std::uint32_t count = 0;
        for (ImageItem* image : mBaseImages)
        {
            if (image == nullptr)
            {
                return HEIF::ErrorCode::INVALID_ITEM_ID;
            }
            if (image->getId() == Heif::InvalidItem)
            {
                error = image->save(aWriter);
                if (HEIF::ErrorCode::OK != error)
                    return error;
            }
            toImageIds[count] = image->getId();
            ++count;
        }
        error = aWriter->addBaseItemReference(getId(), toImageIds);
        if (HEIF::ErrorCode::OK != error)
            return error;
    }

    return ImageItem::save(aWriter);
}
