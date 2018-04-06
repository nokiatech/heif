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

using namespace HEIFPP;
MetaItem::MetaItem(Heif* aHeif, const HEIF::FourCC& aType)
    : Item(aHeif, aType, false)
{
}
HEIF::ErrorCode MetaItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    return Item::load(aReader, aId);
}