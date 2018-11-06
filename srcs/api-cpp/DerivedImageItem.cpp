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

#include "DerivedImageItem.h"
#include <heifreader.h>
#include <heifwriter.h>

using namespace HEIFPP;

DerivedImageItem::DerivedImageItem(Heif* aHeif, const HEIF::FourCC& aType)
    : ImageItem(aHeif, aType, true, false)
{
}
DerivedImageItem::~DerivedImageItem()
{
    for (auto it = mSourceImages.begin(); it != mSourceImages.end();)
    {
        if (*it)
        {
            if (!removeSourceLink(*it, this))
            {
                HEIF_ASSERT(false);
            }
        }
        it = mSourceImages.erase(it);
    }
}
HEIF::ErrorCode DerivedImageItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error;
    error = ImageItem::load(aReader, aId);
    if (HEIF::ErrorCode::OK == error)
    {
        HEIF::Array<HEIF::ImageId> dimgIds;
        // fetch dimg's.
        error = aReader->getReferencedFromItemListByType(aId, "dimg", dimgIds);
        if (HEIF::ErrorCode::OK == error)
        {
            mSourceImages.reserve((std::uint32_t) dimgIds.size);
            for (const auto& dimgId : dimgIds)
            {
                ImageItem* image = getHeif()->constructImageItem(aReader, dimgId, error);
                if (HEIF::ErrorCode::OK != error)
                {
                    return error;
                }
                addSourceImage(image);
            }
        }
    }
    return error;
}
std::uint32_t DerivedImageItem::getSourceImageCount() const
{
    return (std::uint32_t) mSourceImages.size();
}
ImageItem* DerivedImageItem::getSourceImage(std::uint32_t aId)
{
    if (aId < mSourceImages.size())
    {
        return mSourceImages[aId];
    }
    return nullptr;
}
const ImageItem* DerivedImageItem::getSourceImage(std::uint32_t aId) const
{
    if (aId < mSourceImages.size())
    {
        return mSourceImages[aId];
    }
    return nullptr;
}
void DerivedImageItem::setSourceImage(std::uint32_t aId, ImageItem* aImage)
{
    if (aId < mSourceImages.size())
    {
        auto it = mSourceImages.begin() + (std::int32_t) aId;
        if (*it)
        {
            if (!removeSourceLink(*it, this))
            {
                HEIF_ASSERT(false);
            }
        }
        *it = aImage;
        if (aImage)
        {
            addSourceLink(aImage, this);
        }
    }
}
bool DerivedImageItem::setSourceImage(ImageItem* aOldImage, ImageItem* aNewImage)
{
    bool found = false;
    for (auto it = mSourceImages.begin(); it != mSourceImages.end(); ++it)
    {
        if (aOldImage == (*it))
        {
            found = true;
            if (aOldImage)
            {
                if (!removeSourceLink(aOldImage, this))
                {
                    HEIF_ASSERT(false);
                }
            }
            *it = aNewImage;
            if (aNewImage)
            {
                addSourceLink(aNewImage, this);
            }
        }
    }
    return found;
}

void DerivedImageItem::removeSourceImage(std::uint32_t aId)
{
    if (aId < mSourceImages.size())
    {
        auto it = mSourceImages.begin() + (std::int32_t) aId;
        if (*it)
        {
            if (!removeSourceLink(*it, this))
            {
                HEIF_ASSERT(false);
            }
        }
        mSourceImages.erase(it);
    }
}

void DerivedImageItem::addSourceImage(ImageItem* aImage)
{
    if (aImage)
    {
        addSourceLink(aImage, this);
        mSourceImages.push_back(aImage);
    }
}

bool DerivedImageItem::removeSourceImage(ImageItem* aImage)
{
    bool found = false;
    for (auto it = mSourceImages.begin(); it != mSourceImages.end();)
    {
        if (aImage == (*it))
        {
            found = true;
            if (aImage)
            {
                if (!removeSourceLink(aImage, this))
                {
                    HEIF_ASSERT(false);
                }
            }
            it = mSourceImages.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return found;
}

void DerivedImageItem::reserveSourceImages(std::uint32_t aCount)
{
    if (aCount < mSourceImages.size())
    {
        for (auto it = mSourceImages.begin() + (std::int32_t) aCount; it != mSourceImages.end(); ++it)
        {
            if (!removeSourceLink(*it, this))
            {
                HEIF_ASSERT(false);
            }
            *it = nullptr;
        }
    }
    mSourceImages.resize(aCount);
}
