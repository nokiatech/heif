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
        virtual ~MimeItem();

        /** Gets the content type of the MimeItem */
        const std::string& getContentType() const;

        /** Sets the content type of the MimeItem
         * @param [in] aType: Content type */
        void setContentType(const std::string& aType);

        /** Returns the size of the data */
        uint64_t getDataSize() const;

        /** Returns a pointer to the data */
        const uint8_t* getData() const;

        /** Sets the data for the MimeItem
         * @param [in] aData: Pointer to the data
         * @param [in] aSize: Size of the data */
        void setData(const uint8_t* aData, uint64_t aSize);

    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

        std::string mContentType;
        uint64_t mBufferSize;
        uint8_t* mBuffer;

    private:
        MimeItem& operator=(const MimeItem&) = delete;
        MimeItem(const MimeItem&)            = delete;
        MimeItem(MimeItem&&)                 = delete;
        MimeItem()                           = delete;
    };
}  // namespace HEIFPP
