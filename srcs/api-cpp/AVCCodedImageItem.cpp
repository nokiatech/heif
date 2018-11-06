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

#include "AVCCodedImageItem.h"
#include <heifreader.h>
#include <heifwriter.h>
#include <cstring>
#include "H26xTools.h"

using namespace HEIFPP;

AVCCodedImageItem::AVCCodedImageItem(Heif* aHeif)
    : CodedImageItem(aHeif, HEIF::FourCC("avc1"), HEIF::MediaFormat::AVC)
{
}
HEIF::ErrorCode AVCCodedImageItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    return CodedImageItem::load(aReader, aId);
}
HEIF::ErrorCode AVCCodedImageItem::save(HEIF::Writer* aWriter)
{
    return CodedImageItem::save(aWriter);
}
bool AVCCodedImageItem::getBitstream(uint8_t*& aData, std::uint64_t& aSize)
{
    // convert nal bytestream to nal unit stream (ie. change start code prefixes to lengths)
    return NAL_State::convertFromByteStream(mBuffer, mBufferSize, aData, aSize);
}
