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
    return mContentType;
}
void MimeItem::setContentType(const std::string& aType)
{
    if (mContentType.empty())
    {
        mContentType = aType;
    }
}

HEIF::ErrorCode MimeItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error;
    error = MetaItem::load(aReader, aId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    const HEIF::ItemInformation* info = mHeif->getItemInformation(aId);

    // TODO: when reader exposes the item content-type, use that.
    // contentType(info->contentType);

    mBufferSize = info->size;
    mBuffer     = new uint8_t[mBufferSize];
    error       = aReader->getItemData(aId, mBuffer, mBufferSize);
    if (HEIF::ErrorCode::OK != error)
        return error;
    return HEIF::ErrorCode::OK;
}
HEIF::ErrorCode MimeItem::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error;
    HEIF::MediaDataId mediaDataId;
    HEIF::Data fr;
    if (mContentType == "application/rdf+xml")
    {
        fr.mediaFormat = HEIF::MediaFormat::XMP;
    }
    else if (mContentType == "text/xml")
    {
        fr.mediaFormat = HEIF::MediaFormat::MPEG7;
    }
    fr.size            = mBufferSize;
    fr.data            = mBuffer;
    fr.decoderConfigId = 0;

    // TODO: re-use of data?
    error = aWriter->feedMediaData(fr, mediaDataId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    mId = mediaDataId.get();
    return MetaItem::save(aWriter);
}


const uint8_t* MimeItem::getData() const
{
    return mBuffer;
}
uint64_t MimeItem::getDataSize() const
{
    return mBufferSize;
}

void MimeItem::setData(const uint8_t* aData, uint64_t aDataSize)
{
    delete[] mBuffer;
    mBuffer = nullptr;
    mBuffer = new uint8_t[aDataSize];
    memcpy(mBuffer, aData, aDataSize);
    mBufferSize = aDataSize;
}
