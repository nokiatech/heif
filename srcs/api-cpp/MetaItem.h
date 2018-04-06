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

#include <Item.h>
namespace HEIFPP
{
    // generic base for all 'meta' type items.
    class MetaItem : public HEIFPP::Item
    {
    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId);

        MetaItem(Heif* aHeif, const HEIF::FourCC& aType);
        virtual ~MetaItem() = default;

    private:
        MetaItem& operator=(const MetaItem&) = delete;
        MetaItem(const MetaItem&)            = delete;
        MetaItem(MetaItem&&)                 = delete;
        MetaItem()                           = delete;
    };
}  // namespace HEIFPP
