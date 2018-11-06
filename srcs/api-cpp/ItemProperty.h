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

#include <Heif.h>

namespace HEIFPP
{
    class ItemProperty
    {
        friend class Item;
        friend class Heif;

    public:
        virtual ~ItemProperty();

        /** Returns the type of the property */
        const HEIF::ItemPropertyType& getType() const;

        /** Returns the type of the property */
        const HEIF::FourCC& rawType() const;

        /** Sets the custom user data
         *  @param [in] aContext Pointer to the custom data */
        void setContext(const void* aContext);

        /** Gets the custom user data
         *  @return void* Pointer to the custom user data */
        const void* getContext() const;

        const HEIF::PropertyId& getId() const;

        /** Returns if the property is a transformative property. */
        bool isTransformative() const;

    protected:
        void setId(const HEIF::PropertyId& id);
        void link(Item* aItem);
        void unlink(Item* aItem);
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::PropertyId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter) = 0;

        ItemProperty(Heif* aHeif, const HEIF::ItemPropertyType& aType, const HEIF::FourCC& aRawType, bool aIsTransform);
        void setIsTransformative(bool aIsTransformative);
        HEIFPP::Result setRawType(const HEIF::FourCC&);
    private:
        Heif* mHeif;
        HEIF::PropertyId mId;

        HEIF::FourCC mRawType;
        HEIF::ItemPropertyType mType;

        LinkArray<Item*> mLinks;
        const void* mContext;
        bool mIsTransform;

        ItemProperty& operator=(const ItemProperty&) = delete;
        ItemProperty& operator=(ItemProperty&&) = delete;
        ItemProperty(const ItemProperty&)       = delete;
        ItemProperty(ItemProperty&&)            = delete;
        ItemProperty()                          = delete;
    };
}  // namespace HEIFPP
