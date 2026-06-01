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
    class HEVCCodedImageItem : public HEIFPP::CodedImageItem
    {
        // Construction is private; use Heif::create<T>() factory.
        friend class HEIFPP::Heif;

    public:
        ~HEVCCodedImageItem() override = default;
        // serialization
    protected:
        explicit HEVCCodedImageItem(Heif* aHeif);
        HEVCCodedImageItem(Heif* aHeif, const HEIF::FourCC& aType, HEIF::MediaFormat aFormat);
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

        /** Get the encoded HEVC bitstream. The HEIFPP layer pre-caches the
         *  bytes during `Heif::load`; the user never sees the underlying
         *  probe-and-fetch protocol on `HEIF::Reader`. See
         *  the probe-and-fetch protocol (Layer 2) for the full contract
         *  and the trade-offs vs. calling `Reader::getItem*` directly. */
        bool getBitstream(std::uint8_t*& aData, std::uint64_t& aSize) override;

    public:
        HEVCCodedImageItem& operator=(const HEVCCodedImageItem&) = delete;
        HEVCCodedImageItem& operator=(HEVCCodedImageItem&&) = delete;
        HEVCCodedImageItem(const HEVCCodedImageItem&)       = delete;
        HEVCCodedImageItem(HEVCCodedImageItem&&)            = delete;
        HEVCCodedImageItem()                                = delete;
    };
}  // namespace HEIFPP
