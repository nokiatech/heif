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

#include "ItemProperty.h"
#include <heifreader.h>
#include <heifwriter.h>
#include "ImageItem.h"

using namespace HEIFPP;

ItemProperty::ItemProperty(Heif* aHeif, const HEIF::ItemPropertyType& aType, bool aIsTransform)
    : mHeif(aHeif)
    , mId(Heif::InvalidProperty)
    , mType(aType)
    , mContext(nullptr)
    , mIsTransform(aIsTransform)
{
    mHeif->addProperty(this);
}
ItemProperty::~ItemProperty()
{
    for (; !mLinks.empty();)
    {
        Item* linkedto = mLinks[0];
        if (linkedto)
        {
            linkedto->removeProperty(this);
        }
    }
    mHeif->removeProperty(this);
}
void ItemProperty::setContext(const void* aContext)
{
    mContext = aContext;
}
const void* ItemProperty::getContext() const
{
    return mContext;
}

void ItemProperty::setId(const HEIF::PropertyId& aId)
{
    mId = aId;
}
const HEIF::PropertyId& ItemProperty::getId() const
{
    return mId;
}

HEIF::ErrorCode ItemProperty::load(HEIF::Reader* /*aReader*/, const HEIF::PropertyId& aId)
{
    mId = aId;
    return HEIF::ErrorCode::OK;
}
// void ItemProperty::save(HEIF::Writer* /*aWriter*/)
//{
//}
const HEIF::ItemPropertyType& ItemProperty::getType() const
{
    return mType;
}
bool ItemProperty::isTransformative() const
{
    return mIsTransform;
}


void ItemProperty::link(Item* aItem)
{
    HEIF_ASSERT(aItem);
    if (!AddItemTo(mLinks, aItem))
    {
        // Tried to link to an already linked item.
        HEIF_ASSERT(false);
    }
}
void ItemProperty::unlink(Item* aItem)
{
    HEIF_ASSERT(aItem);
    if (!RemoveItemFrom(mLinks, aItem))
    {
        // Tried to unlink a non linked item.
        HEIF_ASSERT(false);
    }
}
