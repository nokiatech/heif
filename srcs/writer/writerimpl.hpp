/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
 * subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
 * written consent of Nokia.
 */

#ifndef WRITERIMPL_HPP
#define WRITERIMPL_HPP

#include "OutputStreamInterface.h"
#include "extendedtypebox.hpp"
#include "filetypebox.hpp"
#include "heifcommondatatypes.h"
#include "heifwriter.h"
#include "idgenerators.hpp"
#include "mediadatabox.hpp"
#include "metabox.hpp"
#include "moviebox.hpp"
#include "writerdatatypesinternal.hpp"

namespace HEIF
{
    class WriterImpl : public Writer
    {
    public:
        WriterImpl();
        ~WriterImpl() override;

        ErrorCode initialize(const OutputConfig& outputConfig) override;
        ErrorCode setMajorBrand(const FourCC& brand) override;
        ErrorCode addCompatibleBrand(const FourCC& brand) override;
        ErrorCode addCompatibleBrandCombination(const Array<FourCC>& compatibleBrandCombination) override;

        ErrorCode finalize() override;

        ErrorCode feedDecoderConfig(const Array<DecoderSpecificInfo>& config,
                                    DecoderConfigId& decoderConfigId) override;
        ErrorCode feedMediaData(const Data& data, MediaDataId& mediaDataId) override;

        ErrorCode addImage(const MediaDataId& mediaDataId, ImageId& imageId) override;
        ErrorCode addImage(const MediaDataId& mediaDataId,
                           const Array<ImageId>& referenceImageIds,
                           ImageId& imageId) override;
        ErrorCode setPrimaryItem(const ImageId& imageId) override;
        ErrorCode setItemDescription(const ImageId& imageId, const ItemDescription& itemDescription) override;
        ErrorCode addMetadata(const MediaDataId& mediaDataId, MetadataItemId& metadataIemId) override;
        ErrorCode addThumbnail(const ImageId& thumbImageId, const ImageId& masterImageId) override;
        ErrorCode addProperty(const CleanAperture& clap, PropertyId& propertyId) override;
        ErrorCode addProperty(const Mirror& imir, PropertyId& propertyId) override;
        ErrorCode addProperty(const Rotate& irot, PropertyId& propertyId) override;
        ErrorCode addProperty(const Scale& iscl, PropertyId& propertyId) override;
        ErrorCode addProperty(const RelativeLocation& rloc, PropertyId& propertyId) override;
        ErrorCode addProperty(const PixelAspectRatio& pasp, PropertyId& propertyId) override;
        ErrorCode addProperty(const PixelInformation& pixi, PropertyId& propertyId) override;
        ErrorCode addProperty(const ColourInformation& colr, PropertyId& propertyId) override;
        ErrorCode addProperty(const AuxiliaryType& auxC, PropertyId& propertyId) override;
        ErrorCode addProperty(const RequiredReferenceTypes& rref, PropertyId& propertyId) override;
        ErrorCode addProperty(const UserDescription& udes, PropertyId& propertyId) override;
        ErrorCode addProperty(const CreationTimeInformation& crtt, PropertyId& propertyId) override;
        ErrorCode addProperty(const ModificationTimeInformation& mdft, PropertyId& propertyId) override;
        ErrorCode addProperty(const AccessibilityText& altt, PropertyId& propertyId) override;

        ErrorCode addProperty(const RawProperty& property,
                              const bool isTransformative,
                              PropertyId& propertyId) override;
        ErrorCode associateProperty(const ImageId& imageId,
                                    const PropertyId& propertyId,
                                    const bool isEssential = false) override;
        ErrorCode associateProperty(const GroupId& imageId,
                                    const PropertyId& propertyId,
                                    const bool isEssential) override;
        ErrorCode addDerivedImage(const ImageId& imageId, ImageId& derivedImageId) override;
        ErrorCode addDerivedImageItem(const Grid& grid, ImageId& gridId) override;
        ErrorCode addDerivedImageItem(const Overlay& iovl, ImageId& overlayId) override;

        ErrorCode addMetadataItemReference(const MetadataItemId& metadataItemId, const ImageId& toImageId) override;
        ErrorCode addTbasItemReference(const ImageId& fromImageId, const ImageId& toImageId) override;
        ErrorCode addBaseItemReference(const ImageId& fromImageId, const Array<ImageId>& toImageIds) override;
        ErrorCode addAuxiliaryReference(const ImageId& fromImageId, const ImageId& toImageId) override;
        ErrorCode setImageHidden(const ImageId& imageId, const bool hidden) override;

        ErrorCode addImageSequence(const Rational& timeBase,
                                   const CodingConstraints& aCodingConstraints,
                                   SequenceId& id) override;
        ErrorCode addImage(const SequenceId& sequenceId,
                           const MediaDataId& mediaDataId,
                           const SampleInfo& sampleInfo,
                           SequenceImageId& imageId) override;
        ErrorCode addMetadataItemReference(const MetadataItemId& metadataItemId,
                                           const SequenceId& sequenceId,
                                           const SequenceImageId& imageId) override;
        ErrorCode addThumbnails(const SequenceId& thumbSequenceId, const SequenceId& sequenceId) override;
        ErrorCode setImageHidden(const SequenceImageId& sequenceImageId, const bool hidden) override;
        ErrorCode addProperty(const CleanAperture& clap, const SequenceId& sequenceId) override;
        ErrorCode addAuxiliaryReference(const AuxiliaryType& auxC,
                                        const SequenceId& auxiliarySequenceId,
                                        const SequenceId& sequenceId) override;
        ErrorCode setEditList(const SequenceId& sequenceId, const EditList& editList) override;
        ErrorCode setMatrix(const Array<int32_t>& matrix) override;
        ErrorCode setMatrix(const SequenceId& sequenceId, const Array<int32_t>& matrix) override;

        ErrorCode createEntityGroup(const FourCC& type, GroupId& id) override;
        ErrorCode createAlternativesGroup(GroupId& id) override;
        ErrorCode createEquivalenceGroup(GroupId& id) override;
        ErrorCode addToGroup(const GroupId& groupId, const ImageId& id) override;
        ErrorCode addToGroup(const GroupId& groupId, const SequenceId& id) override;
        ErrorCode addToEquivalenceGroup(const GroupId& equivalenceGroupId,
                                        const SequenceId& sequenceId,
                                        const SequenceImageId& id,
                                        const EquivalenceTimeOffset& offset = {0, 1 << 8}) override;

        ErrorCode setAlternateGrouping(const SequenceId& sequenceId1, const SequenceId& sequenceId2) override;

        ErrorCode createTrackGroup(const FourCC& type, TrackGroupId& id) override;
        ErrorCode addToGroup(const TrackGroupId& trackGroupId, const SequenceId& id) override;

        ErrorCode addVideoTrack(const Rational& timeBase, SequenceId& id) override;
        ErrorCode addVideo(const SequenceId& sequenceId,
                           const MediaDataId& mediaDataId,
                           const SampleInfo& sampleInfo,
                           SequenceImageId& sampleid) override;
        ErrorCode addAudioTrack(const Rational& timeBase, const AudioParams& config, SequenceId& id) override;
        ErrorCode addAudio(const SequenceId& sequenceId,
                           const MediaDataId& mediaDataId,
                           const SampleInfo& sampleInfo,
                           SequenceImageId& sampleid) override;

    private:
        ErrorCode isValidSequenceImage(const SequenceId& sequenceId, const SequenceImageId& sequenceImageId) const;

        void finalizeMdatBox();                        // Set media data box size.
        ErrorCode generateMoovBox();                   // Fill movie box from intermediate HeifWriterImpl structures.
        ErrorCode updateMoovBox(uint64_t mdatOffset);  // Update moov box internal offset values to mdat data
        ErrorCode finalizeMetaBox();                   // Fill metabox from intermediate HeifWriterImpl structures.

        // writermoovimpl defines for moov writer helpers
        void writeMoovHiddenSamples(ImageSequence& sequence);
        ErrorCode writeMoovSampleTable(ImageSequence& sequence);
        void writeEquivalenceSampleGroup(ImageSequence& sequence);
        void writeRefSampleList(ImageSequence& sequence);
        void writeMetadataItemGroups(ImageSequence& sequence);
        void writeTrackGroups(ImageSequence& imageSequence);

        // helpers for handling fed mediaData
        ErrorCode validateFedMediaData(const Data& aData);
        ErrorCode storeFedMediaData(const Data& aData, MediaDataId& aMediaDataId);

        /**
         * Creates new metadataitem & id for given mediaDataId
         */
        ErrorCode createMetadataItem(const MediaDataId& mediaDataId, MetadataItemId& metadataItemId);

        /**
         * @param metadataItemIds Array of metadata ids in the writer.
         * @return True if metadataItemIds has one or more metadataIds, and all of them are valid (found from the
         * writer).
         */
        bool checkMetadataIds(const MetadataItemId& metadataItemId) const;

        /**
         * @param imageIds Array of image ids in the image collection.
         * @return True if imageIds has one or more ImageIds, and all of them are valid (found from the collection).
         */
        bool checkImageIds(const Array<ImageId>& imageIds) const;

        /**
         * @brief getIspe Get index of an 'ispe' property with given dimensions. If matching one does not already exist,
         * a new property is created and added to the metabox.
         * @param width  Width of the image in pixels.
         * @param height Height of the image in pixels.
         * @return 1-based 'ispe' property index in the Item Property Container Box.
         */
        std::uint16_t getIspeIndex(std::uint32_t width, std::uint32_t height);

        /**
         * @brief getConfigIndex Get index of an decoder configuration property for given decoder configuration in
         * Media. If matching one does not already exist, a new property is created and added to the metabox.
         * @param [in]  configIndex
         * @param [out] propertyIndex 1-based decoder configuration property index in the Item Property Container Box.
         * @return ErrorCode
         */
        ErrorCode getConfigIndex(DecoderConfigId configIndex, uint16_t& propertyIndex);

        /**
         * @brief createEditBox   Create EditBox based on the edit list and given media timescale.
         * @param editList        Input edit list entries.
         * @param mediaTimescale  Timescale of the media on this track.
         * @return New EditBox with EditListBox.
         */
        EditBox createEditBox(const EditList& editList) const;

        /**
         * @brief addEmptyEdit Append a new empty edit to an edit list box.
         * @param editListBox  Edit List Box where the new entry will be appended.
         * @param editUnit     Edit List entry where edit type is EMPTY.
         *                     Media rate fields are ignored.
         */
        void addEmptyEdit(EditListBox* editListBox, const EditUnit& editUnit) const;

        /**
         * @brief addDwellEdit   Append a new dwell edit to an edit list box.
         * @param editListBox    Edit List Box where the new entry will be appended.
         * @param editUnit       Edit List entry where edit type is DWELL.
         *                       Media rate fields are ignored.
         */
        void addDwellEdit(EditListBox* editListBox, const EditUnit& editUnit) const;

        /**
         * @brief addShiftEdit   Append a new shift edit to an edit list box.
         * @param editListBox    Edit List Box where the new entry will be appended.
         * @param editUnit       Edit List entry where edit type is SHIFT.
         *                       Media rate fields are ignored.
         */
        void addShiftEdit(EditListBox* editListBox, const EditUnit& editUnit) const;

        /**
         * @brief addRawEdit     Append a new edit  unitto an edit list box.
         * @param editListBox    Edit List Box where the new entry will be appended.
         * @param editUnit       Edit List entry. Media rate fields are written as set in fields.
         */
        void addRawEdit(EditListBox* editListBox, const EditUnit& editUnit) const;

        /**
         * @brief Initialize internal data structures and ID generators of the writer.
         */
        void clear();

        /**
         * @brief Get track duration, in movie header time scale.
         * @return Track duration, in movie header time scale.
         */
        uint64_t getTrackDuration(TrackBox* track, const ImageSequence& sequence) const;

        /**
         * @brief addCreationTimeInformation Add and associate CreationTimeProperty.
         * @param imageId Id of the image item to associated property to.
         */
        void addCreationTimeInformation(const ImageId& imageId);

    private:
        enum class State
        {
            UNINITIALIZED,  ///< State before starting to write the file and after closing it
            WRITING         ///< State during writing media data and metadata to the file
        };

    private:
        State mState;  ///< Running state of the reader API implementation

        Map<DecoderConfigId, Array<DecoderSpecificInfo>> mAllDecoderConfigs;
        Map<MediaDataId, MediaData> mMediaData;
        Map<std::uint64_t, MediaDataId> mMediaDataHashes;

        Map<SequenceId, ImageSequence> mImageSequences;
        ImageCollection mImageCollection;
        Map<GroupId, EntityGroup> mEntityGroups;
        Map<TrackGroupId, TrackGroup> mTrackGroups;
        Map<MediaDataId, MetadataItemId> mMetadataItems;
        Map<ImageSize, PropertyIndex>
            mIspeIndexes;  ///< Map of ispe property indexes in the item property container box.
        Map<DecoderConfigId, PropertyIndex>
            mDecoderConfigs;  ///< Map of decoder configuration property indexes in the item property container box.
        Map<PropertyIndex, ImageSize>
            mDecoderConfigIndexToSize;  ///< Mapping from decoder configuration property index to image size.
        Map<MediaDataId, ImageSize> mJpegDimensions;  ///< Image dimensions extracted from JPEG file bitstreams.
        Vector<int32_t> mMatrix;
        Map<PropertyId, PropertyInformation> mProperties;  ///< Manually added properties in the metabox.

        FileTypeBox mFileTypeBox;
        ExtendedTypeBox mExtendedTypeBox;
        MetaBox mMetaBox;
        MovieBox mMovieBox;
        MediaDataBox mMediaDataBox;

        OutputStreamInterface* mFile;

        std::uint64_t mMdatOffset    = 0;  ///< 'mdat' offset in the stream
        std::uint64_t mMediaDataSize = 8;  ///< Data size in 'mdat' box in bytes.
                                           ///< Used to check whether 32- or 64-bit size field is used.

        bool mInitialMdat = false;  ///< True if mdat is written to the file beginning after ftyp. False if it written
                                    ///< after meta and moov boxes.
        bool mPrimaryItemSet = false;  ///< True after a primary item has been set.

        bool mOwnsOutputHandle = false;  ///< True if the writer owns the output handle

        bool mWriteItemCreationTimes = false;  ///< Create and associate CreationTimeProperty to added image items.

        PropertyId mPredRrefPropertyId = 0;  ///< ID of 'pred' Required reference types property. 0 if not created.
    };

    namespace
    {
        // Given a reference type, return its value version
        // Useful only in templates (in conjunction with decltype
        template <typename T>
        T valuefy(const T& reference);

        // Given a container with .begin(), return a copy of it put inside a
        // vector
        template <typename T>
        auto vectorize(const T& container) -> Vector<decltype(valuefy(*container.begin()))>
        {
            return Vector<decltype(valuefy(*container.begin()))>(container.begin(), container.end());
        }
    }  // namespace
}  // namespace HEIF

#endif  // WRITERIMPL_HPP
