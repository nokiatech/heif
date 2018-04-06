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

#pragma once

#include <MimeItem.h>
namespace HEIFPP
{
    // generic base for all 'mime' type items. (xmp, mpeg-7, possibly others later.)
    class MPEG7Item : public HEIFPP::MimeItem
    {
    public:
        MPEG7Item(Heif* aHeif);
        virtual ~MPEG7Item() = default;

    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

    private:
        MPEG7Item& operator=(const MPEG7Item&) = delete;
        MPEG7Item(const MPEG7Item&)            = delete;
        MPEG7Item(MPEG7Item&&)                 = delete;
        MPEG7Item()                            = delete;
    };
}  // namespace HEIFPP
