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

#include "XMPItem.h"
#include <heifreader.h>
#include <heifwriter.h>

using namespace HEIFPP;

XMPItem::XMPItem(Heif* aHeif)
    : MimeItem(aHeif)
{
    setContentType("application/rdf+xml");
}
HEIF::ErrorCode XMPItem::load(HEIF::Reader* aReader, const HEIF::ImageId& aId)
{
    HEIF::ErrorCode error;
    error = MimeItem::load(aReader, aId);
    return error;
}
HEIF::ErrorCode XMPItem::save(HEIF::Writer* aWriter)
{
    HEIF::ErrorCode error;
    error = MimeItem::save(aWriter);
    return error;
}
