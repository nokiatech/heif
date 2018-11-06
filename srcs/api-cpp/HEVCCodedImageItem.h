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

#include <CodedImageItem.h>

namespace HEIFPP
{
    class HEVCCodedImageItem : public HEIFPP::CodedImageItem
    {
    public:
        HEVCCodedImageItem(Heif* aHeif);
        ~HEVCCodedImageItem() = default;
        // serialization
    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

        bool getBitstream(std::uint8_t*& aData, std::uint64_t& aSize) override;

    private:
        HEVCCodedImageItem& operator=(const HEVCCodedImageItem&) = delete;
        HEVCCodedImageItem& operator=(HEVCCodedImageItem&&)      = delete;
        HEVCCodedImageItem(const HEVCCodedImageItem&)            = delete;
        HEVCCodedImageItem(HEVCCodedImageItem&&)                 = delete;
        HEVCCodedImageItem()                                     = delete;
    };
}  // namespace HEIFPP
