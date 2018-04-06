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

#include "RawProperty.h"
#include <heifreader.h>
#include <heifwriter.h>

using namespace HEIFPP;
RawProperty::RawProperty(Heif* aHeif)
    : ItemProperty(aHeif, HEIF::ItemPropertyType::RAW, false)
    , mRawData(nullptr)
    , mRawDataSize(0){};
RawProperty::~RawProperty()
{
    delete[] mRawData;
}
void RawProperty::getData(const uint8_t*& aData, uint64_t& aLength) const
{
    aData   = mRawData;
    aLength = mRawDataSize;
}
void RawProperty::setData(const uint8_t* aData, uint64_t aLength)
{
    mRawDataSize = aLength;
    delete[] mRawData;
    mRawData = new uint8_t[mRawDataSize];
    memcpy(mRawData, aData, mRawDataSize);
}
const HEIF::FourCC& RawProperty::rawType() const
{
    return mRawType;
}
void RawProperty::setRawType(const HEIF::FourCC& aType, bool aIsTransform)
{
    mIsTransform = aIsTransform;
    mRawType     = aType;
    // TODO: if mIsTransform changes AFTER the property is associated with an item,
    // we should move it to correct place in the props list. (declarative properties first, then transformative).
}
HEIF::ErrorCode RawProperty::load(HEIF::Reader* aReader, const HEIF::PropertyId& aId)
{
    HEIF::ErrorCode error;
    error = ItemProperty::load(aReader, aId);
    if (HEIF::ErrorCode::OK == error)
    {
        HEIF::RawProperty raw;
        aReader->getProperty(aId, raw);
        if (raw.data.size < 8)  // needs atleast 8 bytes
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
        uint32_t size;
        size = ((uint32_t) raw.data[0]) << 24;
        size |= ((uint32_t) raw.data[1]) << 16;
        size |= ((uint32_t) raw.data[2]) << 8;
        size |= ((uint32_t) raw.data[3]);
        uint8_t fcc[5] = {0};
        fcc[0]         = raw.data[4];
        fcc[1]         = raw.data[5];
        fcc[2]         = raw.data[6];
        fcc[3]         = raw.data[7];
        mRawType       = HEIF::FourCC((char*) fcc);
        if (size != raw.data.size)
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
        if (mRawType != raw.type)
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
        setData(raw.data.elements + 8, (uint32_t) raw.data.size - 8);
    }
    return error;
};
HEIF::ErrorCode RawProperty::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error;
    HEIF::RawProperty raw;
    raw.data.elements = mRawData;
    raw.data.size     = mRawDataSize;
    error             = aWriter->addProperty(raw, mIsTransform, mId);
    raw.data.elements = nullptr;
    raw.data.size     = 0;
    return error;
}
