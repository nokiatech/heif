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

#include "IdentityImageItem.h"
#include <heifreader.h>
#include <heifwriter.h>

using namespace HEIFPP;

IdentityImageItem::IdentityImageItem(Heif* aHeif)
    : DerivedImageItem(aHeif, HEIF::FourCC("iden"))
{
}
ImageItem* IdentityImageItem::getImage()
{
    if (getSourceImageCount() == 0)
        return nullptr;
    return getSourceImage(0);
}
const ImageItem* IdentityImageItem::getImage() const
{
    if (getSourceImageCount() == 0)
        return nullptr;
    return getSourceImage(0);
}
void IdentityImageItem::setImage(ImageItem* aImage)
{
    if (getSourceImageCount() == 0)
    {
        addSourceImage(aImage);
        return;
    }
    setSourceImage((uint32_t) 0, aImage);
}

Result IdentityImageItem::removeImage(ImageItem* aImage)
{
    if (getImage() != aImage)
    {
        // Error, more than one source image or invalid handle.
        // Try to find it so cleanup is done.
        std::uint32_t index = 0;
        for (const auto& image : mSourceImages)
        {
            if (image == aImage)
            {  // found it, -> remove it
                setSourceImage(index, nullptr);
                return Result::OK;
            }
            else
            {
                index++;
            }
        }
        return Result::INVALID_HANDLE;
    }
    setSourceImage((uint32_t) 0, nullptr);
    return Result::OK;
}

HEIF::ErrorCode IdentityImageItem::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error = HEIF::ErrorCode::OK;

    ImageItem* image = getImage();
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
    HEIF::ImageId newId;
    error = aWriter->addDerivedImage(image->getId(), newId);
    setId(newId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    error = DerivedImageItem::save(aWriter);
    return error;
}

HEIF::ErrorCode IdentityImageItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode retval = DerivedImageItem::load(aReader, aId);
    if (retval == HEIF::ErrorCode::OK)
    {
        if (getSourceImageCount() != 1)
        {
            return HEIF::ErrorCode::FILE_READ_ERROR;
        }
    }
    return retval;
}
