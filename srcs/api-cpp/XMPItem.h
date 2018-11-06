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
    class XMPItem : public HEIFPP::MimeItem
    {
    public:
        XMPItem(Heif* aHeif);
        ~XMPItem() = default;

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

    private:
        XMPItem& operator=(const XMPItem&) = delete;
        XMPItem& operator=(XMPItem&&)      = delete;
        XMPItem(const XMPItem&)            = delete;
        XMPItem(XMPItem&&)                 = delete;
        XMPItem()                          = delete;
    };
}  // namespace HEIFPP
