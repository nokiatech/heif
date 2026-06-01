/*
 * This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved. Copying, including reproducing, storing, adapting or translating, any or all
 * of this material requires the prior written consent of Nokia.
 */

#pragma once

#include <Item.h>

#include <helpers.h>
#include "heifcommondatatypes.h"
#include "helpers.h"

namespace HEIFPP
{
    class ImageItem;
    class Sample;
    // generic base for all 'meta' type items.
    class MetaItem : public HEIFPP::Item
    {
        friend class Sample;
        friend class ImageItem;

    public:
        ~MetaItem() override;

    protected:
        void link(ImageItem* aImage);
        void unlink(ImageItem* aImage);
        void link(Sample* aSample);
        void unlink(Sample* aSample);

        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId) override;

        MetaItem(Heif* aHeif, const HEIF::FourCC& aType);


        LinkArray<ImageItem*> mIsMetaToItem;
        LinkArray<Sample*> mIsMetaToSample;

    public:
        MetaItem& operator=(const MetaItem&) = delete;
        MetaItem& operator=(MetaItem&&) = delete;
        MetaItem(const MetaItem&)       = delete;
        MetaItem(MetaItem&&)            = delete;
        MetaItem()                      = delete;
    };
}  // namespace HEIFPP
