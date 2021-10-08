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

#include "JPEGCodedImageItem.h"

#include <heifreader.h>
#include <heifwriter.h>

#include <cstring>

using namespace HEIFPP;

JPEGCodedImageItem::JPEGCodedImageItem(Heif* aHeif)
    : CodedImageItem(aHeif, HEIF::FourCC("jpeg"), HEIF::MediaFormat::JPEG)
{
    mMandatoryConfiguration = false;
}
HEIF::ErrorCode JPEGCodedImageItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    return CodedImageItem::load(aReader, aId);
}
HEIF::ErrorCode JPEGCodedImageItem::save(HEIF::Writer* aWriter)
{
    return CodedImageItem::save(aWriter);
}

bool JPEGCodedImageItem::getBitstream(uint8_t*& aData, std::uint64_t& aSize)
{
    // TODO: nothing to do?
    aSize = mBufferSize;
    aData = new std::uint8_t[mBufferSize];
    std::memcpy(aData, mBuffer, aSize);
    return true;
}
