/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#include "DecoderConfiguration.h"
#include <heifreader.h>
#include <heifwriter.h>
#include "CodedImageItem.h"
#include "Sample.h"

using namespace HEIFPP;

DecoderConfiguration::DecoderConfiguration(Heif* aHeif, const HEIF::FourCC& aType)
    : mHeif(aHeif)
    , mContext(nullptr)
    , mType(aType)
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
    for (; !mSampleLinks.empty();)
    {
        const auto& p  = mSampleLinks[0];
        Sample* sample = p.first;
        if (sample)
        {
            sample->setDecoderConfiguration(nullptr);
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
    HEIF::ErrorCode error;
    std::uint8_t* aData;
    std::uint32_t aDataSize;
    error = convertToRawData(aConfig, aData, aDataSize);
    if (HEIF::ErrorCode::OK == error)
    {
        error = setConfig(aData, aDataSize);
        delete[] aData;
    }
    return error;
}
const HEIF::FourCC& DecoderConfiguration::getMediaType() const
{
    return mType;
}
HEIF::MediaFormat DecoderConfiguration::getMediaFormat() const
{
    return Heif::mediaFormatFromFourCC(mType);
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

void DecoderConfiguration::link(Sample* aImage)
{
    mSampleLinks.addLink(aImage);
}
void DecoderConfiguration::unlink(Sample* aImage)
{
    if (!mSampleLinks.removeLink(aImage))
    {
        // Tried to remove a non-existant link.
        HEIF_ASSERT(false);
    }
}
