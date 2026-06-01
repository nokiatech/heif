/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#include "DecoderConfiguration.h"

#include "Heif.h"

#include <heifwriter.h>

#include "CodedImageItem.h"
#include "Sample.h"
#include "heifcommondatatypes.h"
#include "heifreaderdatatypes.h"
#include "helpers.h"
#include <utility>

using namespace HEIFPP;

DecoderConfig::DecoderConfig(Heif* aHeif, const HEIF::FourCC& aType)
    : mHeif(aHeif)
    , mContext(nullptr)
    , mType(aType)
    , mConfig{Heif::InvalidDecoderConfig, HEIF::Array<HEIF::DecoderSpecificInfo>(0)}
    , mLinks()
    , mSampleLinks()
    , mBuffer(nullptr)
    , mBufferSize()
{
    mHeif->addDecoderConfig(this);
}

DecoderConfig::~DecoderConfig()
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
    mBuffer     = nullptr;
    mBufferSize = 0;
}

void DecoderConfig::setContext(const void* aContext)
{
    mContext = aContext;
}

const void* DecoderConfig::getContext() const
{
    return mContext;
}

void DecoderConfig::setId(const HEIF::DecoderConfigId& aId)
{
    mConfig.decoderConfigId = aId;
}
const HEIF::DecoderConfigId& DecoderConfig::getId() const noexcept
{
    return mConfig.decoderConfigId;
}

const HEIF::Array<HEIF::DecoderSpecificInfo>& DecoderConfig::getConfig() const
{
    return mConfig.decoderSpecificInfo;
}

HEIF::ErrorCode DecoderConfig::setConfig(const HEIF::Array<HEIF::DecoderSpecificInfo>& aConfig)
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
const HEIF::FourCC& DecoderConfig::getMediaType() const
{
    return mType;
}
HEIF::MediaFormat DecoderConfig::getMediaFormat() const
{
    return Heif::mediaFormatFromFourCC(mType);
}

HEIF::ErrorCode DecoderConfig::save(HEIF::Writer* aWriter)
{
    return aWriter->feedDecoderConfig(mConfig.decoderSpecificInfo, mConfig.decoderConfigId);
}

void DecoderConfig::link(CodedImageItem* aImage)
{
    mLinks.addLink(aImage);
}
void DecoderConfig::unlink(CodedImageItem* aImage)
{
    HEIF_LOG_INVARIANT(mLinks.removeLink(aImage),
                       "DecoderConfig::unlink(CodedImageItem): link not found");
}

void DecoderConfig::link(Sample* aImage)
{
    mSampleLinks.addLink(aImage);
}
void DecoderConfig::unlink(Sample* aImage)
{
    HEIF_LOG_INVARIANT(mSampleLinks.removeLink(aImage),
                       "DecoderConfig::unlink(Sample): link not found");
}
