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
    class AVCCodedImageItem : public HEIFPP::CodedImageItem
    {
    public:
        AVCCodedImageItem(Heif* aHeif);
        ~AVCCodedImageItem() = default;

    protected:
        // serialization
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

        bool getBitstream(std::uint8_t*& aData, std::uint64_t& aSize) override;

    private:
        AVCCodedImageItem& operator=(const AVCCodedImageItem&) = delete;
        AVCCodedImageItem& operator=(AVCCodedImageItem&&)      = delete;
        AVCCodedImageItem(const AVCCodedImageItem&)            = delete;
        AVCCodedImageItem(AVCCodedImageItem&&)                 = delete;
        AVCCodedImageItem()                                    = delete;
    };
}  // namespace HEIFPP
