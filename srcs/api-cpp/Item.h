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
    /** @brief Item abstraction*/
    class Item
    {
        friend class Heif;
        friend class ImageItem;

    public:
        virtual ~Item();

        /** Sets the custom user data
         *  @param [in] aContext Pointer to the custom data */
        void setContext(const void* aContext);

        /** Returns a pointer to the custom user data. */
        const void* getContext() const;

        const HEIF::ImageId& getId() const;

        /** Returns the type of the item as a FourCC code. */
        const HEIF::FourCC& getType() const;


        bool isImageItem() const;
        bool isTileImageItem() const;  // not sure if this should be exposed only in imageItems.. Also not supported yet
        bool isMetadataItem() const;
        bool isExifItem() const;
        bool isMimeItem() const;
        bool isXMPItem() const;
        bool isMPEG7Item() const;
        bool isProtected() const;

        /** Adds a property for the item
         * @param [in] aProperty: Property to be added
         * @param [in] aEssential: If the property is an essential property */
        void addProperty(ItemProperty* aProperty, bool aEssential);

        /** Removes a property from the item
         * @param [in] aProperty: Property to be removed. */
        void removeProperty(ItemProperty* aProperty);

        /** Returns the property count for the item. */
        uint32_t propertyCount() const;

        /** Returns a property with the given index
         * @param [in] aIndex: Index of the property */
        ItemProperty* getProperty(uint32_t aIndex);
        const ItemProperty* getProperty(uint32_t aIndex) const;

        /** Returns if the property is essential
         * @param [in] aIndex: The index of the property */
        bool isEssential(uint32_t aId) const;

        /** Returns if the property is essential
         * @param [in] aProperty: The property */
        bool isEssential(ItemProperty* aProperty) const;

        /** Sets the essential flag for the property
         * @param [in] aIndex: Index of the property
         * @param [in] aEssential: If the property is essential */
        void setEssential(uint32_t aId, bool aEssential);

        /** Sets the essential flag for the property
         * @param [in] aProperty: The property
         * @param [in] aEssential: If the property is essential */
        void setEssential(ItemProperty* aProperty, bool aEssential);

        /** Returns the parent HEIF object of the item */
        Heif* getHeif();
        const Heif* getHeif() const;

    protected:
        // serialization methods.
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

        void setId(const HEIF::ImageId&);

        Item(Heif* aHeif, const HEIF::FourCC& aType, bool aIsImage);
        Heif* mHeif;
        HEIF::ImageId mId;
        HEIF::FourCC mType;
        bool mIsProtected;
        bool mIsImageItem;
        const void* mContext;

        uint32_t mFirstTransform;
        uint32_t mTransformCount;
        std::vector<std::pair<ItemProperty*, bool>> mProps;

    private:
        Item& operator=(const Item&) = delete;
        Item(const Item&)            = delete;
        Item(Item&&)                 = delete;
        Item()                       = delete;
    };
}  // namespace HEIFPP
