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

#include <CodedImageItem.h>
#include "heifcommondatatypes.h"
#include <cstdint>

namespace HEIFPP
{
    class JPEGCodedImageItem : public HEIFPP::CodedImageItem
    {
        // Construction is private; use Heif::create<T>() factory.
        friend class HEIFPP::Heif;

    public:
        ~JPEGCodedImageItem() override = default;

    protected:
        explicit JPEGCodedImageItem(Heif* aHeif);
        // serialization
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

        bool getBitstream(std::uint8_t*& aData, std::uint64_t& aSize) override;

    public:
        JPEGCodedImageItem& operator=(const JPEGCodedImageItem&) = delete;
        JPEGCodedImageItem& operator=(JPEGCodedImageItem&&) = delete;
        JPEGCodedImageItem(const JPEGCodedImageItem&)       = delete;
        JPEGCodedImageItem(JPEGCodedImageItem&&)            = delete;
        JPEGCodedImageItem()                                = delete;
    };
}  // namespace HEIFPP
