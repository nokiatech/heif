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

#ifndef HEIF_H
#define HEIF_H

#include <heifcommondatatypes.h>
#include <heifreaderdatatypes.h>
#include <heifwriterdatatypes.h>

#include <helpers.h>
#include <string>
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
    class Track;
    class Sample;
    class AudioTrack;
    class VideoTrack;
    class ImageSequence;
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
    class AuxiliaryProperty;
    class RawProperty;
    class Heif;
    class DecoderConfig;
    class EntityGroup;
    class AlternativeTrackGroup;
    /** @brief HEIF file abstraction*/
    class Heif
    {
        friend class EntityGroup;
        friend class Track;
        friend class Sample;
        friend class AudioTrack;
        friend class VideoTrack;
        friend class ImageSequence;
        friend class ExifItem;
        friend class ItemProperty;
        friend class Item;
        friend class CodedImageItem;
        friend class DerivedImageItem;
        friend class ImageItem;
        friend class DecoderConfig;
        friend class MimeItem;
        friend class AlternativeTrackGroup;

    public:
        static const HEIF::GroupId InvalidGroup;
        static const HEIF::ImageId InvalidItem;
        static const HEIF::SequenceId InvalidSequence;
        static const HEIF::SequenceImageId InvalidSequenceImage;
        static const HEIF::PropertyId InvalidProperty;
        static const HEIF::DecoderConfigId InvalidDecoderConfig;
        static const HEIF::MediaDataId InvalidMediaData;

        enum PreloadMode
        {
            LOAD_ALL_DATA = 0,  // Loads all item data to memory.
            LOAD_PREVIEW_DATA,  // Load preview data to memory (thumbnail/meta). Fast to preview file, but actual item
                                // data loaded on demand.
            LOAD_ON_DEMAND      // Preload none of the sample/image/metadata to memory. Fastest to load.
        };

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
        Result load(const char* aFilename, PreloadMode loadMode = LOAD_ALL_DATA);

        /** Load content from a stream.
         *  @param [in] aStream Stream to read the file from.
         *  @return Result: Possible error code */
        Result load(HEIF::StreamInterface* aStream, PreloadMode loadMode = LOAD_ALL_DATA);

        /** Save content to file.
         *  @param [in] fileName File to open.
         *  @return Result: Possible error code*/
        Result save(const char* aFileName);

        /** Save content to file.
         *  @param [in] fileName File to open.
         *  @return Result: Possible error code*/
        Result save(HEIF::OutputStreamInterface* aStream);

        /** Clears the container to initial state. */
        void reset();

        /** Returns if the file is a single image file
         * @return bool: Single file */
        bool hasSingleImage();

        /** Returns if the file has an image collection
         * @return bool: Is a collection */
        bool hasImageCollection();


        // Track / TrackCollection
        /** Returns the total amount tracks in the file
         *  @return std::uint32_t: The track count */
        std::uint32_t getTrackCount() const;

        /** Returns the track with the given index
         * @param [in] aTrack: Index of the track
         * @return Track*: The Track with the requested index */
        Track* getTrack(std::uint32_t aTrack);

        /** Returns the track with the given index
         * @param [in] aTrack: Index of the track
         * @return const Track*: The track with the requested index */
        const Track* getTrack(std::uint32_t aTrack) const;

        /** Remove track from file
         * @param [in] aTrack: Track to be removed */
        void removeTrack(Track* aTrack);
        void removeSample(Sample* aSample);


        // Image / Item collection
        /** Returns the total amount items in the file
         *  @return std::uint32_t: The item count */
        std::uint32_t getItemCount() const;

        /** Returns the item with the given index
         * @param [in] aItem: Index of the item
         * @return Item*: The item with the requested index */
        Item* getItem(std::uint32_t aItem);

        /** Returns the item with the given index
         * @param [in] aItem: Index of the item
         * @return Item*: The item with the requested index */
        const Item* getItem(std::uint32_t aItem) const;

        /** Remove item/image from file
         * @param [in] aItem: Item to be removed */
        void removeItem(Item* aItem);

        /** Returns the amount of images in the file*
         * @return std::uint32_t: The amount of images */
        std::uint32_t getImageCount() const;

        /** Returns the image with the given index
         * @param [in] aIndex: Index of the image
         * @return ImageItem*: ImageItem with the given index*/
        ImageItem* getImage(std::uint32_t aIndex);

        /** Returns the image with the given index
         * @param [in] aIndex: Index of the image
         * @return ImageItem*: ImageItem with the given index*/
        const ImageItem* getImage(std::uint32_t aIndex) const;

        /** Returns the amount of master images
         *  @return std::uint32_t: Master image count */
        std::uint32_t getMasterImageCount() const;

        /** Returns the master image with the given index
         * @param [in] aIndex: The index of the master image
         * @return ImageItem*: The master image with the given index */
        ImageItem* getMasterImage(std::uint32_t aIndex);

        /** Returns the master image with the given index
         * @param [in] aIndex: The index of the master image
         * @return ImageItem*: The master image with the given index */
        const ImageItem* getMasterImage(std::uint32_t aIndex) const;

        /** Returns the amount of items with the given type
         * @param [in] aType: The type as a FourCC code
         * @return std::uint32_t: The item count */
        std::uint32_t getItemsOfTypeCount(const HEIF::FourCC& aType) const;

        /** Returns the item with the given type and index
         * @param [in] aType: The type as a FourCC code
         * @param [in] aIndex: The index
         * @return Item: The requested Item*/
        Item* getItemOfType(const HEIF::FourCC& aType, std::uint32_t aIndex);

        /** Returns the item with the given type and index
         * @param [in] aType: The type as a FourCC code
         * @param [in] aIndex: The index
         * @return Item: The requested Item */
        const Item* getItemOfType(const HEIF::FourCC& aType, std::uint32_t aIndex) const;

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
         *  @return std::uint32_t: Minor version from the File Type Box */
        std::uint32_t getMinorVersion() const;

        /** Sets the minor version of the file
         * @param [in]: aVersion: Minor version of the file
         */
        void setMinorVersion(std::uint32_t aVersion);

        /** Returns the amount of compatible brands set the for the file
         *  @return std::uint32_t: The compatible brand count. */
        std::uint32_t compatibleBrands() const;

        /** Returns the compatible brand with the given index
         *  @param [in] aIndex: Index to the compatible brands list from the File Type Box
         *  @return FourCC: The brand for the given index. */
        const HEIF::FourCC& getCompatibleBrand(std::uint32_t aIndex) const;

        /** Adds a compatible brand to the file
         * @param [in] aBrand: The brand as FourCC. */
        void addCompatibleBrand(const HEIF::FourCC& aBrand);

        /** Removes a compatible brand from the file
         * @param [in] aIndex: The brand with the given index from the file. */
        void removeCompatibleBrand(std::uint32_t aIndex);

        /** Removes a compatible brand from the file
         * @param [in] aBrand: The brand as a FourCC. */
        void removeCompatibleBrand(const HEIF::FourCC& aBrand);

        /** Gets the amount of decoder configurations in the file
         * @return std::uint32_t: The amount of decoder configs. */
        std::uint32_t getDecoderConfigCount() const;

        /** Remove decoder config from the file
         * @param [in] aDecoderConfig: The config to be removed. */
        void removeDecoderConfig(DecoderConfig* aDecoderConfig);

        /** Returns a decoder config with the given index
         * @param [in] aIndex: The index of the decoder config
         * @return DecoderConfig: The config. */
        DecoderConfig* getDecoderConfig(std::uint32_t aIndex);

        /** Returns a decoder config with the given index
         * @param [in] aIndex: The index of the decoder config
         * @return DecoderConfig: The config. */
        const DecoderConfig* getDecoderConfig(std::uint32_t aIndex) const;

        /** Returns the amount of ItemProperties in the file
         *  @return std::uint32_t: The property count. */
        std::uint32_t getPropertyCount() const;

        /** Gets an ItemProperty with the given index
         * @param [in] aIndex: The index of the ItemProperty
         * @return ItemProperty: The ItemProperty */
        ItemProperty* getProperty(std::uint32_t aIndex);

        /** Gets an ItemProperty with the given index
         * @param [in] aIndex: The index of the ItemProperty
         * @return ItemProperty: The ItemProperty */
        const ItemProperty* getProperty(std::uint32_t aIndex) const;

        /** Remove an ItemProperty from the file
         * @param [in] aProperty: The ItemProperty to be removed */
        void removeProperty(ItemProperty* aProperty);


        // groups
        std::uint32_t getGroupCount() const;
        EntityGroup* getGroup(std::uint32_t aId);
        const EntityGroup* getGroup(std::uint32_t aId) const;
        void removeGroup(EntityGroup* aGroup);

        std::uint32_t getGroupByTypeCount(const HEIF::FourCC& aType) const;
        EntityGroup* getGroupByType(const HEIF::FourCC& aType, std::uint32_t aId);
        EntityGroup* getGroupById(const HEIF::GroupId& aId);
        const EntityGroup* getGroupByType(const HEIF::FourCC& aType, std::uint32_t aId) const;
        const EntityGroup* getGroupById(const HEIF::GroupId& aId) const;


        std::uint32_t getAlternativeTrackGroupCount() const;
        AlternativeTrackGroup* getAlternativeTrackGroup(std::uint32_t aId);
        const AlternativeTrackGroup* getAlternativeTrackGroup(std::uint32_t aId) const;
        void removeAlternativeTrackGroup(AlternativeTrackGroup* aGroup);

        static HEIF::MediaFormat mediaFormatFromFourCC(const HEIF::FourCC& aType);

    protected:
        Track* constructTrack(HEIF::Reader* aReader, const HEIF::SequenceId& aItemId, HEIF::ErrorCode& aErrorCode);
        Sample* constructSample(HEIF::Reader* aReader,
                                const HEIF::SequenceId& aTrack,
                                const HEIF::SampleInformation& aId,
                                HEIF::ErrorCode& aErrorCode);
        ImageItem* constructImageItem(HEIF::Reader* aReader, const HEIF::ImageId& aItemId, HEIF::ErrorCode& aErrorCode);
        MetaItem* constructMetaItem(HEIF::Reader* aReader, const HEIF::ImageId& aItemId, HEIF::ErrorCode& aErrorCode);
        ItemProperty* constructItemProperty(HEIF::Reader* aReader,
                                            const HEIF::ItemPropertyInfo& aItemInfo,
                                            HEIF::ErrorCode& aErrorCode);

        DecoderConfig* constructDecoderConfig(HEIF::Reader* aReader,
                                              const HEIF::SequenceId& aTrackId,
                                              const HEIF::SequenceImageId& aTrackImageId,
                                              HEIF::ErrorCode& aErrorCode);
        DecoderConfig* constructDecoderConfig(HEIF::Reader* aReader,
                                              const HEIF::ImageId& aItemId,
                                              HEIF::ErrorCode& aErrorCode);
        DecoderConfig* constructDecoderConfig(HEIF::Reader* aReader,
                                              const HEIF::ImageId& aItemId,
                                              const HEIF::SequenceId& aTrackId,
                                              const HEIF::SequenceImageId& aTrackImageId,
                                              HEIF::ErrorCode& aErrorCode);

        EntityGroup* constructGroup(const HEIF::FourCC& aType);
        const HEIF::FileInformation* getFileInformation() const;
        const HEIF::ItemInformation* getItemInformation(const HEIF::ImageId& aItemId) const;
        const HEIF::TrackInformation* getTrackInformation(const HEIF::SequenceId& aItemId) const;


        // the following should never be called by users.
        void addItem(Item* aItem);
        void addSample(Sample* aItem);
        void addTrack(Track* aItem);
        void addDecoderConfig(DecoderConfig* aDecoderConfig);
        void addProperty(ItemProperty* aProperty);
        void addGroup(EntityGroup* aGroup);
        void addAlternativeTrackGroup(AlternativeTrackGroup* aGroup);
        HEIF::Reader* getReaderInstance();

    protected:
        HEIF::FileInformation mFileinfo;
        HEIF::FourCC mMajorBrand;
        std::uint32_t mMinorVersion;
        std::vector<HEIF::FourCC> mCompatibleBrands;
        // NOTE: ItemIds and SequenceIds SHOULD be in the same namespace.
        std::vector<Item*> mItems;
        std::vector<Track*> mTracks;
        std::vector<Sample*> mSamples;
        std::map<HEIF::FourCC, std::vector<Item*>> mItemsOfType;
        std::vector<ItemProperty*> mProperties;
        std::vector<DecoderConfig*> mDecoderConfigs;
        std::vector<EntityGroup*> mGroups;
        std::vector<AlternativeTrackGroup*> mAltGroups;
        std::map<HEIF::FourCC, std::vector<EntityGroup*>> mGroupsOfType;
        ImageItem* mPrimaryItem;
        std::int32_t mMatrix[9];  // movie matrix.

        PreloadMode mPreLoadMode;

        // temporary objects, part of serialization.
        std::map<HEIF::ImageId, Item*> mItemsLoad;
        std::map<HEIF::SequenceId, Track*> mTracksLoad;
        std::map<HEIF::PropertyId, ItemProperty*> mPropertiesLoad;
        std::map<std::pair<HEIF::SequenceId, HEIF::DecoderConfigId>, DecoderConfig*> mDecoderConfigsLoad;
        std::map<std::pair<HEIF::SequenceId, HEIF::SequenceImageId>, Sample*> mSamplesLoad;
        std::map<HEIF::GroupId, EntityGroup*> mGroupsLoad;
        std::map<std::uint32_t, AlternativeTrackGroup*> mAltGroupsLoad;

    private:
        Result load(const char* aFilename, HEIF::StreamInterface* aStream, PreloadMode loadMode);
        Result save(const char* aFilename, HEIF::OutputStreamInterface* aStream);
        HEIF::ErrorCode load(HEIF::Reader* aReader);
        const void* mContext;
        HEIF::Reader* mReader;

    private:
        Heif& operator=(const Heif&) = delete;
        Heif& operator=(Heif&&) = delete;
        Heif(const Heif&)       = delete;
        Heif(Heif&&)            = delete;
    };
}  // namespace HEIFPP

#endif //HEIF_H
