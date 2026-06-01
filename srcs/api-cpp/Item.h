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

#include <heifcommondatatypes.h>

#include "ErrorCodes.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>
#include "heifcommondatatypes.h"

namespace HEIF
{
    class Reader;
    class Writer;
}  // namespace HEIF

namespace HEIFPP
{
    class EntityGroup;
    class Heif;
    class ItemProperty;

    /** @brief Item abstraction*/
    class Item
    {
        friend class EntityGroup;
        friend class Heif;
        friend class ImageItem;

    public:
        /**
         * Destructor. Removes this Item from its owning `Heif` container's
         * indexing and from any groups it belongs to.
         *
         * @warning **Do not call `delete` on a pointer returned by
         *          `Heif::getItem()` / `getImage()` / `getMasterImage()` /
         *          `getItemOfType()` etc.** Those pointers are owned by
         *          the parent `Heif` instance -- see the ownership contract
         *          on @ref HEIFPP::Heif.
         *
         * @warning **Do not destroy individual Items after the parent
         *          `Heif` has already been destroyed.** This destructor
         *          back-references `mHeif` to deregister itself; calling it
         *          on an Item whose parent has already gone away is
         *          undefined behavior (use-after-free of the parent).
         *          The safe pattern is always to let `~Heif()` cascade the
         *          deletion of every owned Item, or to call
         *          `Heif::removeItem(item)` while the parent is still live.
         */
        virtual ~Item();

        /** Sets the custom user data
         *  @param [in] aContext Pointer to the custom data */
        void setContext(const void* aContext);

        /** Returns a pointer to the custom user data. */
        const void* getContext() const;

        const HEIF::ImageId& getId() const noexcept;

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
        std::uint32_t propertyCount() const;

        /** Returns a property with the given index
         * @param [in] aIndex: Index of the property */
        ItemProperty* getProperty(uint32_t aIndex);
        const ItemProperty* getProperty(uint32_t aIndex) const;

        /** Returns if the property is essential
         * @param [in] aId: The index of the property */
        bool isEssential(uint32_t aId) const;

        /** Returns if the property is essential
         * @param [in] aProperty: The property */
        bool isEssential(const ItemProperty* aProperty) const;

        /** Sets the essential flag for the property
         * @param [in] aId: Index of the property
         * @param [in] aEssential: If the property is essential */
        void setEssential(uint32_t aId, bool aEssential);

        /** Sets the essential flag for the property
         * @param [in] aProperty: The property
         * @param [in] aEssential: If the property is essential */
        void setEssential(const ItemProperty* aProperty, bool aEssential);

        /** Returns the parent HEIF object of the item */
        Heif* getHeif();
        const Heif* getHeif() const;

        const std::string& getName() const;
        void setName(const std::string& aName);

        // groups
        std::uint32_t getGroupCount() const;
        EntityGroup* getGroup(uint32_t aId);

        std::uint32_t getGroupByTypeCount(const HEIF::FourCC& aType);
        EntityGroup* getGroupByType(const HEIF::FourCC& aType, uint32_t aId);
        EntityGroup* getGroupById(const HEIF::GroupId& aId);

    protected:
        void addToGroup(EntityGroup* aGroup);
        void removeFromGroup(EntityGroup* aGroup);
        // serialization methods.
        virtual HEIF::ErrorCode load(HEIF::Reader* aReader, const HEIF::ImageId& aId);
        virtual HEIF::ErrorCode save(HEIF::Writer* aWriter);

        void setId(const HEIF::ImageId&);

        Item(Heif* aHeif, const HEIF::FourCC& aType, bool aIsImage);

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


    private:
        Heif* mHeif;
        HEIF::ImageId mId;
        HEIF::FourCC mType;
        bool mIsProtected;
        bool mIsImageItem;
        const void* mContext;
        std::uint32_t mTransformCount;
        std::vector<std::pair<ItemProperty*, bool>> mProps;
        std::vector<EntityGroup*> mGroups;
        std::string mName;
        std::string mContentType;
        std::string mContentEncoding;

    public:
        Item& operator=(const Item&) = delete;
        Item& operator=(Item&&) = delete;
        Item(const Item&)       = delete;
        Item(Item&&)            = delete;
        Item()                  = delete;
    };
}  // namespace HEIFPP
