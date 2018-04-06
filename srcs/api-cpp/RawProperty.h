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

#include <ItemProperty.h>
namespace HEIFPP
{
    class RawProperty : public HEIFPP::ItemProperty
    {
    public:
        RawProperty(Heif* aHeif);
        virtual ~RawProperty();
        const HEIF::FourCC& rawType() const;
        /*
            Currently rawtype / istransform should not be changed if the property is attached to an item.
        */
        void setRawType(const HEIF::FourCC& aType, bool aIsTransform);

        /* returns and sets the box payload. */
        void getData(const uint8_t*& aData, uint64_t& aLength) const;
        void setData(const uint8_t* aData, uint64_t aLength);

    protected:
        virtual HEIF::ErrorCode load(HEIF::Reader*, const HEIF::PropertyId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

        HEIF::FourCC mRawType;
        uint8_t* mRawData;
        uint64_t mRawDataSize;

    private:
        RawProperty& operator=(const RawProperty&) = delete;
        RawProperty(const RawProperty&)            = delete;
        RawProperty(RawProperty&&)                 = delete;
        RawProperty()                              = delete;
    };

}  // namespace HEIFPP
