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
    : ItemProperty(aHeif, HEIF::ItemPropertyType::RAW, HEIF::FourCC((std::uint32_t)0), false)
{
}
RawProperty::RawProperty(Heif* aHeif, const HEIF::FourCC& aType, bool aIsTransform)
    : ItemProperty(aHeif, HEIF::ItemPropertyType::RAW, aType,aIsTransform)
{
}

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
    const int offset = 8;//offset to payload (size of box type and size)
    mRaw.data    = HEIF::Array<uint8_t>((aLength + offset));
    mRaw.data[0] = ((aLength + offset) >> 24) & 0xFF;
    mRaw.data[1] = ((aLength + offset) >> 16) & 0xFF;
    mRaw.data[2] = ((aLength + offset) >> 8) & 0xFF;
    mRaw.data[3] = ((aLength + offset)) &0xFF;    
    mRaw.data[4] = static_cast<std::uint8_t>(rawType().value[0]);
    mRaw.data[5] = static_cast<std::uint8_t>(rawType().value[1]);
    mRaw.data[6] = static_cast<std::uint8_t>(rawType().value[2]);
    mRaw.data[7] = static_cast<std::uint8_t>(rawType().value[3]);
    std::memcpy(mRaw.data.elements + offset, aData, aLength);
}
HEIFPP::Result RawProperty::setRawType(const HEIF::FourCC& aType, bool aIsTransform)
{
    HEIFPP::Result res=ItemProperty::setRawType(aType);
    if (res == HEIFPP::Result::OK)
    {
        setIsTransformative(aIsTransform);
        // TODO: if mIsTransform changes AFTER the property is associated with an item,
        // we should move it to correct place in the props list. (declarative properties first, then transformative).
    }
    return res;
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
        HEIF::FourCC fc;
        fc.value[0] = static_cast<char>(mRaw.data[4]);
        fc.value[1] = static_cast<char>(mRaw.data[5]);
        fc.value[2] = static_cast<char>(mRaw.data[6]);
        fc.value[3] = static_cast<char>(mRaw.data[7]);
        fc.value[4] = 0;
        setRawType(fc,false);
        if (size != mRaw.data.size)
        {
            return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
        }        
        if (rawType() != mRaw.type)
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
