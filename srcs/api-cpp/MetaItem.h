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
    class Sample;
    // generic base for all 'meta' type items.
    class MetaItem : public HEIFPP::Item
    {
        friend class Sample;
        friend class ImageItem;

    public:
        ~MetaItem();

    protected:
        void link(ImageItem* aImage);
        void unlink(ImageItem* aImage);
        void link(Sample* aImage);
        void unlink(Sample* aImage);

        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId) override;

        MetaItem(Heif* aHeif, const HEIF::FourCC& aType);


        LinkArray<ImageItem*> mIsMetaToItem;
        LinkArray<Sample*> mIsMetaToSample;

    private:
        MetaItem & operator=(const MetaItem&) = delete;
        MetaItem& operator=(MetaItem&&)      = delete;
        MetaItem(const MetaItem&)            = delete;
        MetaItem(MetaItem&&)                 = delete;
        MetaItem()                           = delete;
    };
}  // namespace HEIFPP
