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

#include <heifcommondatatypes.h>
#include <heifreaderdatatypes.h>
#include <heifwriterdatatypes.h>
#include <helpers.h>
#include "ErrorCodes.h"

namespace HEIF
{
    class StreamInterface;
    class Reader;
    class Writer;
}  // namespace HEIF


namespace HEIFPP
{
    // Forward declare types
    class Item;
    class ImageItem;
    class CodedImageItem;
    class DerivedImageItem;
    class MetaItem;
    class ExifItem;
    class ItemProperty;
    class TransformativeProperty;
    class DescriptiveProperty;
    class PixelAspectRatioProperty;
    class ColourInformationProperty;
    class PixelInformationProperty;
    class RelativeLocationProperty;
    class AuxProperty;
    class RawProperty;
    class Heif;
    class DecoderConfiguration;

    /** @brief HEIF file abstraction*/
    class Heif
    {
        friend class ExifItem;
        friend class ItemProperty;
        friend class Item;
        friend class CodedImageItem;
        friend class DerivedImageItem;
        friend class ImageItem;
        friend class DecoderConfiguration;
        friend class MimeItem;

    public:
        static const HEIF::ImageId InvalidItem;
        static const HEIF::PropertyId InvalidProperty;
        static const HEIF::DecoderConfigId InvalidDecoderConfig;
        static const HEIF::MediaDataId InvalidMediaData;

        /** Create an empty instance
         */
        Heif();
        ~Heif();


        /** Sets the custom user data
         *  @param [in] aContext Pointer to the custom data */
        void setContext(const void* aContext);

        /** Gets the custom user data
         *  @return void* Pointer to the custom user data */
        const void* getContext() const;

        /** Load content from file.
         *  @param [in] fileName File to open.
         *  @return Result: Possible error code */
        Result load(const char* aFileName);

        /** Load content from a stream.
         *  @param [in] aStream Stream to read the file from.
         *  @return Result: Possible error code */
        Result load(HEIF::StreamInterface* aStream);

        /** Save content to file.
         *  @param [in] fileName File to open.
         *  @return Result: Possible error code*/
        Result save(const char* aFileName);


        /** Clears the container to initial state. */
        void reset();

        /** Returns if the file is a single image file
         * @return bool: Single file */
        bool hasSingleImage();

        /** Returns if the file has an image collection
         * @return bool: Is a collection */
        bool hasImageCollection();

        /** Returns the total amount items in the file
         *  @return uint32_t: The item count */
        uint32_t getItemCount() const;

        /** Returns the item with the given index
         * @param [in] aItem: Index of the item
         * @return Item*: The item with the requested index */
        Item* getItem(uint32_t aItem);

        /** Returns the item with the given index
         * @param [in] aItem: Index of the item
         * @return Item*: The item with the requested index */
        const Item* getItem(uint32_t aItem) const;

        /** Remove item/image from file
         * @param [in] aItem: Item to be removed */
        void removeItem(Item* aItem);

        /** Returns the amount of images in the file*
         * @return uint32_t: The amount of images */
        uint32_t getImageCount() const;

        /** Returns the image with the given index
         * @param [in] aIndex: Index of the image
         * @return ImageItem*: ImageItem with the given index*/
        ImageItem* getImage(uint32_t aIndex);

        /** Returns the image with the given index
         * @param [in] aIndex: Index of the image
         * @return ImageItem*: ImageItem with the given index*/
        const ImageItem* getImage(uint32_t aIndex) const;

        /** Returns the amount of master images
         *  @return uint32_t: Master image count */
        uint32_t getMasterImageCount() const;

        /** Returns the master image with the given index
         * @param [in] aIndex: The index of the master image
         * @return ImageItem*: The master image with the given index */
        ImageItem* getMasterImage(uint32_t aIndex);

        /** Returns the master image with the given index
         * @param [in] aIndex: The index of the master image
         * @return ImageItem*: The master image with the given index */
        const ImageItem* getMasterImage(uint32_t aIndex) const;

        /** Returns the amount of items with the given type
         * @param [in] aType: The type as a FourCC code
         * @return uint32_t: The item count */
        uint32_t getItemsOfTypeCount(const HEIF::FourCC& aType) const;

        /** Returns the item with the given type and index
         * @param [in] aType: The type as a FourCC code
         * @param [in] aIndex: The index
         * @return Item: The requested Item*/
        Item* getItemOfType(const HEIF::FourCC& aType, uint32_t aIndex);

        /** Returns the item with the given type and index
         * @param [in] aType: The type as a FourCC code
         * @param [in] aIndex: The index
         * @return Item: The requested Item */
        const Item* getItemOfType(const HEIF::FourCC& aType, uint32_t aIndex) const;

        /** Returns the primary item of the file or NULL (cover image)
         * @return ImageItem or nullptr if not set.*/
        ImageItem* getPrimaryItem();

        /** Returns the primary item of the file or NULL (cover image)
         *  @return ImageItem or nullptr if not set.*/
        const ImageItem* getPrimaryItem() const;

        /** Sets the primary item of the file (cover image)
         * @param [in] aItem: The new primary item */
        void setPrimaryItem(ImageItem* aItem);

        /**
         * Returns the major brand of the file
         *  @return FourCC: Major brand from the File Type Box*/
        const HEIF::FourCC& getMajorBrand() const;

        /** Sets the major brand
         * @param [in] aBrand: The brand as a FourCC */
        void setMajorBrand(const HEIF::FourCC& aBrand);

        /** Returns the minor version of the file
         *  @return uint32_t: Minor version from the File Type Box */
        uint32_t getMinorVersion() const;

        /** Sets the minor version of the file
         * @param [in]: aVersion: Minor version of the file
         */
        void setMinorVersion(uint32_t aVersion);

        /** Returns the amount of compatible brands set the for the file
         *  @return uint32_t: The compatible brand count. */
        uint32_t compatibleBrands() const;

        /** Returns the compatible brand with the given index
         *  @param [in] aIndex: Index to the compatible brands list from the File Type Box
         *  @return FourCC: The brand for the given index. */
        const HEIF::FourCC& getCompatibleBrand(uint32_t aIndex) const;

        /** Adds a compatible brand to the file
         * @param [in] aBrand: The brand as FourCC. */
        void addCompatibleBrand(const HEIF::FourCC& aBrand);

        /** Removes a compatible brand from the file
         * @param [in] aIndex: The brand with the given index from the file. */
        void removeCompatibleBrand(uint32_t aIndex);

        /** Removes a compatible brand from the file
         * @param [in] aBrand: The brand as a FourCC. */
        void removeCompatibleBrand(const HEIF::FourCC& aBrand);

        /** Gets the amount of decoder configurations in the file
         * @return uint32_t: The amount of decoder configs. */
        uint32_t getDecoderConfigCount() const;

        /** Remove decoder config from the file
         * @param [in] aDecoderConfig: The config to be removed. */
        void removeDecoderConfig(DecoderConfiguration* aDecoderConfig);

        /** Returns a decoder config with the given index
         * @param [in] aIndex: The index of the decoder config
         * @return DecoderConfig: The config. */
        DecoderConfiguration* getDecoderConfig(uint32_t aIndex);

        /** Returns a decoder config with the given index
         * @param [in] aIndex: The index of the decoder config
         * @return DecoderConfig: The config. */
        const DecoderConfiguration* getDecoderConfig(uint32_t aIndex) const;

        /** Returns the amount of ItemProperties in the file
         *  @return uint32_t: The property count. */
        uint32_t getPropertyCount() const;

        /** Gets an ItemProperty with the given index
         * @param [in] aIndex: The index of the ItemProperty
         * @return ItemProperty: The ItemProperty */
        ItemProperty* getProperty(uint32_t aIndex);

        /** Gets an ItemProperty with the given index
         * @param [in] aIndex: The index of the ItemProperty
         * @return ItemProperty: The ItemProperty */
        const ItemProperty* getProperty(uint32_t aIndex) const;

        /** Remove an ItemProperty from the file
         * @param [in] aProperty: The ItemProperty to be removed */
        void removeProperty(ItemProperty* aProperty);


    protected:
        Item* constructItem(HEIF::Reader* aReader, const HEIF::ImageId& aItemId, HEIF::ErrorCode& aErrorCode);
        ItemProperty* constructItemProperty(HEIF::Reader* aReader,
                                            const HEIF::ItemPropertyInfo& aItemInfo,
                                            HEIF::ErrorCode& aErrorCode);
        DecoderConfiguration* constructDecoderConfig(HEIF::Reader* aReader,
                                                     const HEIF::MediaFormat& aFormat,
                                                     const HEIF::ImageId& aItemId,
                                                     HEIF::ErrorCode& aErrorCode);
        const HEIF::ItemInformation* getItemInformation(const HEIF::ImageId& aItemId) const;
        const HEIF::ImageInformation* getImageInformation(const HEIF::ImageId& aItemId) const;


        // the following should never be called by users.
        void addItem(Item* aItem);
        void addDecoderConfig(DecoderConfiguration* aDecoderConfig);
        void addProperty(ItemProperty* aProperty);

    protected:
        HEIF::FileInformation mFileinfo;
        HEIF::FourCC mMajorBrand;
        uint32_t mMinorVersion;
        std::vector<HEIF::FourCC> mCompatibleBrands;
        std::vector<Item*> mItems;
        std::map<HEIF::FourCC, std::vector<Item*>> mItemsOfType;
        std::vector<ItemProperty*> mProperties;
        std::vector<DecoderConfiguration*> mDecoderConfigs;
        ImageItem* mPrimaryItem;

        // temporary objects, part of serialization.
        std::map<HEIF::ImageId, Item*> mItemsLoad;
        std::map<HEIF::PropertyId, ItemProperty*> mPropertiesLoad;
        std::map<HEIF::DecoderConfigId, DecoderConfiguration*> mDecoderConfigsLoad;

    private:
        HEIF::ErrorCode load(HEIF::Reader* aReader);
        const void* mContext;

    private:
        Heif& operator=(const Heif&) = delete;
        Heif(const Heif&)            = delete;
        Heif(Heif&&)                 = delete;
    };
}  // namespace HEIFPP
