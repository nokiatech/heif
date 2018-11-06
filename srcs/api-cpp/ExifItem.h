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

#include <MetaItem.h>

namespace HEIFPP
{
    class ExifItem : public HEIFPP::MetaItem
    {
    public:
        ExifItem(Heif* aHeif);
        ~ExifItem();

        /** Returns the size of the data */
        std::uint64_t getDataSize() const;

        /** Returns the data. */
        const std::uint8_t* getData();

        /** Sets the data for the item
         * @param [in] aData: Pointer to the data
         * @param [in] aSize: Size of the data */
        void setData(const std::uint8_t* aData, std::uint64_t aSize);

    protected:
        HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;

        std::uint64_t mBufferSize;
        std::uint8_t* mBuffer;

    private:
        HEIF::ErrorCode loadData();

    private:
        ExifItem& operator=(const ExifItem&) = delete;
        ExifItem& operator=(ExifItem&&) = delete;
        ExifItem(const ExifItem&)       = delete;
        ExifItem(ExifItem&&)            = delete;
        ExifItem()                      = delete;
    };
}  // namespace HEIFPP
