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
        RawProperty(Heif* aHeif, const HEIF::FourCC& aType, bool aIsTransform);
        ~RawProperty();
        /*
            Currently rawtype / istransform should not be changed if the property is attached to an item.
        */
        HEIFPP::Result setRawType(const HEIF::FourCC& aType, bool aIsTransform);

        /* returns and sets the box payload. */
        void getData(const std::uint8_t*& aData, std::uint64_t& aLength) const;
        void setData(const std::uint8_t* aData, std::uint64_t aLength);

    protected:
        HEIF::ErrorCode load(HEIF::Reader*, const HEIF::PropertyId& aId) override;
        HEIF::ErrorCode save(HEIF::Writer* aWriter) override;
        
        HEIF::RawProperty mRaw;

    private:
        RawProperty& operator=(const RawProperty&) = delete;
        RawProperty& operator=(RawProperty&&)      = delete;
        RawProperty(const RawProperty&)            = delete;
        RawProperty(RawProperty&&)                 = delete;
        RawProperty()                              = delete;
    };

}  // namespace HEIFPP
