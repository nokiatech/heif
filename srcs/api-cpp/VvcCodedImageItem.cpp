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
#include "VvcCodedImageItem.h"



#include "H26xTools.h"
#include "CodedImageItem.h"
#include "heifwriterdatatypes.h"

using namespace HEIFPP;

VVCCodedImageItem::VVCCodedImageItem(Heif* aHeif)
    : CodedImageItem(aHeif, HEIF::FourCC("vvc1"), HEIF::MediaFormat::VVC)
{
}
HEIF::ErrorCode VVCCodedImageItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    return CodedImageItem::load(aReader, aId);
}
HEIF::ErrorCode VVCCodedImageItem::save(HEIF::Writer* aWriter)
{
    return CodedImageItem::save(aWriter);
}

bool VVCCodedImageItem::getBitstream(uint8_t*& aData, std::uint64_t& aSize)
{
    // convert nal bytestream to nal unit stream (ie. change start code prefixes to lengths)
    return NAL_State::convertFromByteStream(mBuffer, mBufferSize, aData, aSize);
}
