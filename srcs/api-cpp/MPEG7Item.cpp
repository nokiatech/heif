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

#include "MPEG7Item.h"
#include <heifreader.h>
#include <heifwriter.h>

using namespace HEIFPP;

MPEG7Item::MPEG7Item(Heif* aHeif)
    : MimeItem(aHeif)
{
    setContentType("text/xml");
}
HEIF::ErrorCode MPEG7Item::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error;
    error = MimeItem::load(aReader, aId);
    return error;
}
HEIF::ErrorCode MPEG7Item::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error;
    error = MimeItem::save(aWriter);
    return error;
}
