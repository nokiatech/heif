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
Identity::Identity(Heif* aHeif)
    : DerivedImageItem(aHeif, HEIF::FourCC("iden"))
{
}
ImageItem* Identity::getImage()
{
    if (getSourceImageCount() == 0)
        return nullptr;
    HEIF_ASSERT(getSourceImageCount() == 1);
    return getSourceImage(0);
}
const ImageItem* Identity::getImage() const
{
    if (getSourceImageCount() == 0)
        return nullptr;
    HEIF_ASSERT(getSourceImageCount() == 1);
    return getSourceImage(0);
}
void Identity::setImage(ImageItem* aImage)
{
    if (getSourceImageCount() == 0)
    {
        addSourceImage(aImage);
        return;
    }
    HEIF_ASSERT(getSourceImageCount() == 1);
    setSourceImage((uint32_t) 0, aImage);
}

Result Identity::removeImage(ImageItem* aImage)
{
    ImageItem* curImage = getImage();
    if (curImage != aImage)
    {
        return Result::INVALID_HANDLE;
    }
    setSourceImage((uint32_t) 0, nullptr);
    return Result::OK;
}

HEIF::ErrorCode Identity::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error;
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
    error = aWriter->addDerivedImage(image->getId(), mId);
    if (HEIF::ErrorCode::OK != error)
        return error;
    error = DerivedImageItem::save(aWriter);
    return error;
}

HEIF::ErrorCode Identity::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
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