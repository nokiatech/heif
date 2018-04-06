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
using namespace HEIFPP;

DecoderConfiguration::DecoderConfiguration(Heif* aHeif, const HEIF::MediaFormat& aFormat)
    : mHeif(aHeif)
    , mContext(nullptr)
    , mFormat(aFormat)
    , mConfig{Heif::InvalidDecoderConfig, {0}}
    , mBuffer(nullptr)
    , mBufferSize()
{
    mHeif->addDecoderConfig(this);
}
DecoderConfiguration::~DecoderConfiguration()
{
    for (; !mLinks.empty();)
    {
        const auto& p         = mLinks[0];
        CodedImageItem* image = p.first;
        if (image)
        {
            image->setDecoderConfiguration(nullptr);
        }
    }
    mHeif->removeDecoderConfig(this);
    delete[] mBuffer;
    mBuffer     = 0;
    mBufferSize = 0;
}

void DecoderConfiguration::setContext(const void* aContext)
{
    mContext = aContext;
}

const void* DecoderConfiguration::getContext() const
{
    return mContext;
}

void DecoderConfiguration::setId(const HEIF::DecoderConfigId& aId)
{
    mConfig.decoderConfigId = aId;
}
const HEIF::DecoderConfigId& DecoderConfiguration::getId() const
{
    return mConfig.decoderConfigId;
}

const HEIF::Array<HEIF::DecoderSpecificInfo>& DecoderConfiguration::getConfig() const
{
    return mConfig.decoderSpecificInfo;
}

HEIF::ErrorCode DecoderConfiguration::setConfig(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig)
{
    if (mBuffer)
    {
        delete[] mBuffer;
        mBuffer     = nullptr;
        mBufferSize = 0;
    }
    HEIF::ErrorCode error;
    error = convertToRawData(aConfig, mBuffer, mBufferSize);
    if (HEIF::ErrorCode::OK == error)
    {
        error = convertFromRawData(mBuffer, mBufferSize);
    }

    return error;
}
const HEIF::MediaFormat& DecoderConfiguration::getMediaFormat() const
{
    return mFormat;
}

HEIF::ErrorCode DecoderConfiguration::save(HEIF::Writer* aWriter)
{
    return aWriter->feedDecoderConfig(mConfig.decoderSpecificInfo, mConfig.decoderConfigId);
}

void DecoderConfiguration::link(CodedImageItem* aImage)
{
    mLinks.addLink(aImage);
}
void DecoderConfiguration::unlink(CodedImageItem* aImage)
{
    if (!mLinks.removeLink(aImage))
    {
        // Tried to remove a non-existant link.
        HEIF_ASSERT(false);
    }
}

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
    for (uint32_t i = 0; i < getBaseImageCount(); ++i)
    {
        setBaseImage(i, nullptr);
    }
    mBaseImages.clear();
}

DecoderConfiguration* CodedImageItem::getDecoderConfiguration()
{
    return mConfig;
}

const DecoderConfiguration* CodedImageItem::getDecoderConfiguration() const
{
    return mConfig;
}
void CodedImageItem::setDecoderConfiguration(DecoderConfiguration* aConfig)
{
    if (mConfig)
        mConfig->unlink(this);
    if (aConfig)
    {
        if (aConfig->getMediaFormat() != mFormat)
        {
            // invalid configuration.
            HEIF_ASSERT(false);
        }
    }
    mConfig = aConfig;
    if (mConfig)
        mConfig->link(this);
}
const HEIF::FourCC& CodedImageItem::getDecoderCodeType() const
{
    return mType;
}
const HEIF::MediaFormat& CodedImageItem::getMediaFormat() const
{
    return mFormat;
}

uint32_t CodedImageItem::getBaseImageCount() const
{
    return (uint32_t) mBaseImages.size();
}
ImageItem* CodedImageItem::getBaseImage(uint32_t aId)
{
    if (aId < mBaseImages.size())
    {
        return mBaseImages[aId];
    }
    return nullptr;
}
const ImageItem* CodedImageItem::getBaseImage(uint32_t aId) const
{
    if (aId < mBaseImages.size())
    {
        return mBaseImages[aId];
    }
    return nullptr;
}


void CodedImageItem::setBaseImage(uint32_t aId, ImageItem* aImage)
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
    }
    mBaseImages.push_back(aImage);
}


void CodedImageItem::removeBaseImage(uint32_t aId)
{
    if (aId < mBaseImages.size())
    {
        auto it = mBaseImages.begin() + (int32_t) aId;
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

void CodedImageItem::reserveBaseImages(uint32_t aCount)
{
    if (aCount < mBaseImages.size())
    {
        for (auto it = mBaseImages.begin() + (int32_t) aCount; it != mBaseImages.end(); ++it)
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

uint64_t CodedImageItem::getItemDataSize() const
{
    return mBufferSize;
}
const uint8_t* CodedImageItem::getItemData() const
{
    return mBuffer;
}

void CodedImageItem::setItemData(const uint8_t* aData, uint64_t aSize)
{
    mBufferSize = aSize;
    delete[] mBuffer;
    mBuffer = new uint8_t[aSize];
    memcpy(mBuffer, aData, mBufferSize);
}

HEIF::ErrorCode CodedImageItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error;
    error = ImageItem::load(aReader, aId);
    if (HEIF::ErrorCode::OK != error)
    {
        return error;
    }

    const auto* info = mHeif->getImageInformation(aId);
    HEIF_ASSERT(info);  // Images MUST have image information
    if (info->features & HEIF::ImageFeatureEnum::IsPreComputedDerivedImage)
    {
        HEIF::Array<HEIF::ImageId> baseIds;
        // base images for pre-derived images..
        error = aReader->getReferencedToItemListByType(aId, "base", baseIds);
        if (HEIF::ErrorCode::OK != error)
            return error;
        // construct base images.
        mBaseImages.reserve((uint32_t) baseIds.size);
        for (const auto& baseId : baseIds)
        {
            ImageItem* tmp = static_cast<ImageItem*>(mHeif->constructItem(aReader, baseId, error));
            if (HEIF::ErrorCode::OK != error)
            {
                return error;
            }
            addBaseImage(tmp);
        }
    }

    HEIF::FourCC codeType;
    error = aReader->getDecoderCodeType(aId, codeType);
    if (HEIF::ErrorCode::OK == error)
    {
        if (codeType != mType)
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
        setDecoderConfiguration(mHeif->constructDecoderConfig(aReader, mFormat, aId, error));
        if (HEIF::ErrorCode::OK != error)
        {
            return error;
        }
    }
    else
    {
        // Corrupted image? OR a jpeg? (jpegs have optional decoder config...)
        if (mMandatoryConfiguration)
        {
            return error;
        }
    }

    mBufferSize = info->size;
    mBuffer     = new uint8_t[mBufferSize];
    error       = aReader->getItemData(aId, mBuffer, mBufferSize);
    return error;
}

HEIF::ErrorCode CodedImageItem::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error;
    if (mBuffer == nullptr)
    {
        // TODO: actual error is NO_MEDIA
        return HEIF::ErrorCode::BUFFER_SIZE_TOO_SMALL;
    }
    // save all base images first... if we have any
    if (!mBaseImages.empty())
    {
        HEIF::Array<HEIF::ImageId> toImageIds(mBaseImages.size());
        uint32_t count = 0;
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
        error = aWriter->addBaseItemReference(mId, toImageIds);
        if (HEIF::ErrorCode::OK != error)
            return error;
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


    uint64_t size = 0;
    uint8_t* data = nullptr;

    getBitstream(data, size);
    fr.size        = size;
    fr.data        = data;
    fr.mediaFormat = mFormat;

    if (fr.data == nullptr)
    {
        // mediadata not set, or corrupted.
        return HEIF::ErrorCode::INVALID_MEDIA_FORMAT;
    }

    // TOODO: should mediadata reuse be possible (technically yes, but do it later?)
    error = aWriter->feedMediaData(fr, mediaDataId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    error = aWriter->addImage(mediaDataId, mId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    delete[] fr.data;
    return ImageItem::save(aWriter);
}
