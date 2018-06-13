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
#include <cstring>

using namespace HEIFPP;

RawProperty::RawProperty(Heif* aHeif)
    : ItemProperty(aHeif, HEIF::ItemPropertyType::RAW, false){};
RawProperty::~RawProperty()
{
}
void RawProperty::getData(const std::uint8_t*& aData, std::uint64_t& aLength) const
{
    aData   = mRaw.data.elements + 8;
    aLength = mRaw.data.size - 8;
}
void RawProperty::setData(const std::uint8_t* aData, std::uint64_t aLength)
{
    mRaw.data    = HEIF::Array<uint8_t>(aLength + 8);
    mRaw.data[0] = (aLength >> 24) & 0xFF;
    mRaw.data[1] = (aLength >> 16) & 0xFF;
    mRaw.data[2] = (aLength >> 8) & 0xFF;
    mRaw.data[3] = (aLength) &0xFF;
    mRaw.data[4] = static_cast<std::uint8_t>(mRawType.value[0]);
    mRaw.data[5] = static_cast<std::uint8_t>(mRawType.value[1]);
    mRaw.data[6] = static_cast<std::uint8_t>(mRawType.value[2]);
    mRaw.data[7] = static_cast<std::uint8_t>(mRawType.value[3]);
    std::memcpy(mRaw.data.elements + 8, aData, aLength);
}
const HEIF::FourCC& RawProperty::rawType() const
{
    return mRawType;
}
void RawProperty::setRawType(const HEIF::FourCC& aType, bool aIsTransform)
{
    setIsTransformative(aIsTransform);
    mRawType = aType;
    // TODO: if mIsTransform changes AFTER the property is associated with an item,
    // we should move it to correct place in the props list. (declarative properties first, then transformative).
}
HEIF::ErrorCode RawProperty::load(HEIF::Reader* aReader, const HEIF::PropertyId& aId)
{
    HEIF::ErrorCode error;
    error = ItemProperty::load(aReader, aId);
    if (HEIF::ErrorCode::OK == error)
    {
        aReader->getProperty(aId, mRaw);
        if (mRaw.data.size < 8)  // needs atleast 8 bytes
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
        std::uint32_t size;
        size = static_cast<uint32_t>(mRaw.data[0]) << 24;
        size |= static_cast<uint32_t>(mRaw.data[1]) << 16;
        size |= static_cast<uint32_t>(mRaw.data[2]) << 8;
        size |= static_cast<uint32_t>(mRaw.data[3]);
        mRawType.value[0] = static_cast<char>(mRaw.data[4]);
        mRawType.value[1] = static_cast<char>(mRaw.data[5]);
        mRawType.value[2] = static_cast<char>(mRaw.data[6]);
        mRawType.value[3] = static_cast<char>(mRaw.data[7]);
        mRawType.value[4] = 0;
        if (size != mRaw.data.size)
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
        if (mRawType != mRaw.type)
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }
    }
    return error;
};
HEIF::ErrorCode RawProperty::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;
    HEIF::PropertyId newId;
    error = aWriter->addProperty(mRaw, isTransformative(), newId);
    if (HEIF::ErrorCode::OK == error)
        setId(newId);
    return error;
}
