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
    // generic base for all 'mime' type items. (xmp, mpeg-7, possibly others later.)
    class MimeItem : public HEIFPP::MetaItem
    {
    public:
        MimeItem(Heif* aHeif);
        ~MimeItem();

        /** Gets the content type of the MimeItem */
        const std::string& getContentType() const;

        /** Sets the content type of the MimeItem
         * @param [in] aType: Content type */
        HEIFPP::Result setContentType(const std::string& aType);

        /** Gets the content encoding of the MimeItem */
        const std::string& getContentEncoding() const;

        /** Sets the content encoding of the MimeItem
         * @param [in] aType: Encoding type */
        HEIFPP::Result setContentEncoding(const std::string& aType);

        /** Returns the size of the data */
        std::uint64_t getDataSize() const;

        /** Returns a pointer to the data */
        const std::uint8_t* getData();

        /** Sets the data for the MimeItem
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
        MimeItem& operator=(const MimeItem&) = delete;
        MimeItem& operator=(MimeItem&&) = delete;
        MimeItem(const MimeItem&)       = delete;
        MimeItem(MimeItem&&)            = delete;
        MimeItem()                      = delete;
    };
}  // namespace HEIFPP
