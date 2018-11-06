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

#include "MetaItem.h"
#include <heifreader.h>
#include <heifwriter.h>
#include "ImageItem.h"
#include "Sample.h"

using namespace HEIFPP;

MetaItem::MetaItem(Heif* aHeif, const HEIF::FourCC& aType)
    : Item(aHeif, aType, false)
    , mIsMetaToItem()
    , mIsMetaToSample()
{
}

MetaItem::~MetaItem()
{
    for (; !mIsMetaToItem.empty();)
    {
        const auto& p    = mIsMetaToItem[0];
        ImageItem* image = p.first;
        if (image)
        {
            image->removeMetadata(this);
        }
    }
    for (; !mIsMetaToSample.empty();)
    {
        const auto& p  = mIsMetaToSample[0];
        Sample* sample = p.first;
        if (sample)
        {
            sample->removeMetadata(this);
        }
    }
}
HEIF::ErrorCode MetaItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    return Item::load(aReader, aId);
}

void MetaItem::link(ImageItem* aImage)
{
    mIsMetaToItem.addLink(aImage);
}
void MetaItem::unlink(ImageItem* aImage)
{
    mIsMetaToItem.removeLink(aImage);
}
void MetaItem::link(Sample* aSample)
{
    mIsMetaToSample.addLink(aSample);
}
void MetaItem::unlink(Sample* aSample)
{
    mIsMetaToSample.removeLink(aSample);
}
