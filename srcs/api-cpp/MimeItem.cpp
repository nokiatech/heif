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

#include "MimeItem.h"
#include <heifreader.h>
#include <heifwriter.h>
#include <cstring>
#include "ErrorCodes.h"

using namespace HEIFPP;

MimeItem::MimeItem(Heif* aHeif)
    : MetaItem(aHeif, HEIF::FourCC("mime"))
    , mBufferSize(0)
    , mBuffer(nullptr)
{
}
MimeItem::~MimeItem()
{
    delete[] mBuffer;
}
const std::string& MimeItem::getContentType() const
{
    return Item::getContentType();
}
HEIFPP::Result MimeItem::setContentType(const std::string& aType)
{
    return Item::setContentType(aType);
}
const std::string& MimeItem::getContentEncoding() const
{
    return Item::getContentEncoding();
}
HEIFPP::Result MimeItem::setContentEncoding(const std::string& aType)
{
    return Item::setContentEncoding(aType);
}

HEIF::ErrorCode MimeItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error;
    error = MetaItem::load(aReader, aId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    const HEIF::ItemInformation* info = getHeif()->getItemInformation(aId);

    mBufferSize = info->size;
    if (getHeif()->mPreLoadMode == Heif::PreloadMode::LOAD_ALL_DATA ||
        getHeif()->mPreLoadMode == Heif::PreloadMode::LOAD_PREVIEW_DATA)
    {
        error = loadData();
    }
    return HEIF::ErrorCode::OK;
}
HEIF::ErrorCode MimeItem::save(HEIF::Writer* aWriter)
{
    if (mBuffer == nullptr)
    {
        // TODO: actual error is NO_MEDIA
        return HEIF::ErrorCode::BUFFER_SIZE_TOO_SMALL;
    }
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;

    HEIF::MediaDataId mediaDataId;
    HEIF::Data fr;
    if (Item::getContentType() == "")
    {
        // TODO: "content type not set"
        return HEIF::ErrorCode::INVALID_MEDIA_FORMAT;
    }
    // TODO: this should not be needed. actual contenttype is written Item::save
    fr.mediaFormat     = HEIF::MediaFormat::MPEG7;
    fr.size            = mBufferSize;
    fr.data            = mBuffer;
    fr.decoderConfigId = 0;

    // TODO: re-use of data?
    error = aWriter->feedMediaData(fr, mediaDataId);
    if (HEIF::ErrorCode::OK != error)
        return error;

    HEIF::MetadataItemId metadataItemId;
    error = aWriter->addMetadata(mediaDataId, metadataItemId);

    setId(metadataItemId.get());
    return MetaItem::save(aWriter);
}


const std::uint8_t* MimeItem::getData()
{
    if (mBuffer == nullptr)
    {
        loadData();
    }
    return mBuffer;
}
std::uint64_t MimeItem::getDataSize() const
{
    return mBufferSize;
}

void MimeItem::setData(const std::uint8_t* aData, std::uint64_t aDataSize)
{
    delete[] mBuffer;
    mBuffer = nullptr;
    mBuffer = new std::uint8_t[aDataSize];
    std::memcpy(mBuffer, aData, aDataSize);
    mBufferSize = aDataSize;
}

HEIF::ErrorCode MimeItem::loadData()
{
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;
    if (mBufferSize == 0)
    {
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
                return error;
            }
        }
        else
        {
            HEIF_ASSERT(false);
        }
    }
    return error;
}
