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

#include "OverlayImageItem.h"
#include <heifreader.h>
#include <heifwriter.h>

using namespace HEIFPP;
Overlay::Overlay(Heif* aHeif)
    : DerivedImageItem(aHeif, HEIF::FourCC("iovl"))
{
    mR = 0;
    mG = 0;
    mB = 0;
    mA = 65535;
}
uint16_t Overlay::r() const
{
    return mR;
}
uint16_t Overlay::g() const
{
    return mG;
}
uint16_t Overlay::b() const
{
    return mB;
}
uint16_t Overlay::a() const
{
    return mA;
}

void Overlay::setR(uint16_t aR)
{
    mR = aR;
}
void Overlay::setG(uint16_t aG)
{
    mG = aG;
}
void Overlay::setB(uint16_t aB)
{
    mB = aB;
}
void Overlay::setA(uint16_t aA)
{
    mA = aA;
}

ImageItem* Overlay::getImage(uint32_t aId, HEIF::Overlay::Offset& aOffset)
{
    if (aId < getSourceImageCount())
    {
        aOffset = mOffsets[aId];
        return getSourceImage(aId);
    }
    return nullptr;
}
const ImageItem* Overlay::getImage(uint32_t aId, HEIF::Overlay::Offset& aOffset) const
{
    if (aId < getSourceImageCount())
    {
        aOffset = mOffsets[aId];
        return getSourceImage(aId);
    }
    return nullptr;
}
uint32_t Overlay::imageCount() const
{
    return getSourceImageCount();
}

Result Overlay::setImage(uint32_t aId, ImageItem* aImage, const HEIF::Overlay::Offset& aOffset)
{
    if (aId >= getSourceImageCount())
    {
        return Result::INDEX_OUT_OF_BOUNDS;
    }
    setSourceImage(aId, aImage);
    mOffsets[aId] = aOffset;
    return Result::OK;
}
void Overlay::addImage(ImageItem* aImage, const HEIF::Overlay::Offset& aOffset)
{
    addSourceImage(aImage);
    mOffsets.push_back(aOffset);
}

Result Overlay::removeImage(uint32_t aId)
{
    if (aId >= getSourceImageCount())
    {
        return Result::INDEX_OUT_OF_BOUNDS;
    }
    removeSourceImage(aId);
    mOffsets.erase(mOffsets.begin() + (int32_t) aId);
    return Result::OK;
}
Result Overlay::removeImage(ImageItem* aImage)
{
    bool found = false;
    for (;;)
    {
        const auto& it = FindItemIn(mSourceImages, aImage);
        if (it == mSourceImages.end())
            break;
        intptr_t id = it - mSourceImages.begin();
        removeSourceImage((uint32_t) id);
        mOffsets.erase(mOffsets.begin() + id);
        found = true;
    }
    if (!found)
    {
        return Result::INVALID_HANDLE;
    }
    return Result::OK;
}

HEIF::ErrorCode Overlay::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error;
    error = DerivedImageItem::load(aReader, aId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    HEIF::Overlay overlay;
    error = aReader->getItem(aId, overlay);
    if (HEIF::ErrorCode::OK != error)
        return error;

    mR = overlay.r;
    mG = overlay.g;
    mB = overlay.b;
    mA = overlay.a;
    if (getSourceImageCount() != overlay.offsets.size)
    {
        return HEIF::ErrorCode::FILE_READ_ERROR;
    }
    HEIF_ASSERT(overlay.outputWidth == width());
    HEIF_ASSERT(overlay.outputHeight == height());
    if ((overlay.outputWidth != width()) || (overlay.outputHeight != height()))
    {
        return HEIF::ErrorCode::MEDIA_PARSING_ERROR;
    }
    mOffsets.reserve(overlay.offsets.size);
    for (auto off : overlay.offsets)
    {
        mOffsets.push_back(off);
    }
    return HEIF::ErrorCode::OK;
}


HEIF::ErrorCode Overlay::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error;
    HEIF::Overlay overlay;
    overlay.outputWidth  = width();
    overlay.outputHeight = height();
    overlay.r            = mR;
    overlay.g            = mG;
    overlay.b            = mB;
    overlay.a            = mA;
    HEIF::Array<HEIF::ImageId> ids(imageCount());
    HEIF::Array<HEIF::Overlay::Offset> offsets(imageCount());
    for (uint32_t i = 0; i < imageCount(); ++i)
    {
        ImageItem* image = getImage(i, offsets[i]);
        if (image == nullptr)
        {
            return HEIF::ErrorCode::INVALID_ITEM_ID;
        }
        if (image->getId() == Heif::InvalidItem)
        {
            error = image->save(aWriter);
            if (HEIF::ErrorCode::OK != error)
                return error;
        }
        ids[i] = image->getId();
    }
    overlay.imageIds = ids;
    overlay.offsets  = offsets;
    error            = aWriter->addDerivedImageItem(overlay, mId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    return DerivedImageItem::save(aWriter);
}
