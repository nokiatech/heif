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

#include "ExifItem.h"
#include <heifreader.h>
#include <heifwriter.h>
using namespace HEIFPP;
ExifItem::ExifItem(Heif* aHeif)
    : MetaItem(aHeif, HEIF::FourCC("Exif"))
    , mBufferSize(0)
    , mBuffer(nullptr)
{
}
ExifItem::~ExifItem()
{
    delete[] mBuffer;
}

HEIF::ErrorCode ExifItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error;
    error = MetaItem::load(aReader, aId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    const HEIF::ItemInformation* info = mHeif->getItemInformation(aId);
    mBufferSize                       = info->size;
    mBuffer                           = new uint8_t[mBufferSize];
    error                             = aReader->getItemData(aId, mBuffer, mBufferSize);
    if (HEIF::ErrorCode::OK != error)
        return error;
    return HEIF::ErrorCode::OK;
}
HEIF::ErrorCode ExifItem::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error;
    HEIF::MediaDataId mediaDataId;
    HEIF::Data fr;
    fr.mediaFormat     = HEIF::MediaFormat::EXIF;
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


const uint8_t* ExifItem::getData() const
{
    return mBuffer;
}
uint64_t ExifItem::getDataSize() const
{
    return mBufferSize;
}

void ExifItem::setData(const uint8_t* aData, uint64_t aDataSize)
{
    delete[] mBuffer;
    mBuffer = nullptr;
    mBuffer = new uint8_t[aDataSize];
    memcpy(mBuffer, aData, aDataSize);
    mBufferSize = aDataSize;
}
