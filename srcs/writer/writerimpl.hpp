/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2018 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
        ~WriterImpl();

        virtual ErrorCode initialize(const OutputConfig& outputConfig);
        virtual ErrorCode setMajorBrand(const FourCC& brand);
        virtual ErrorCode addCompatibleBrand(const FourCC& brand);
        virtual ErrorCode finalize();

        virtual ErrorCode feedDecoderConfig(const Array<DecoderSpecificInfo>& config, DecoderConfigId& decoderConfigId);
        virtual ErrorCode feedMediaData(const Data& data, MediaDataId& mediaDataId);

        virtual ErrorCode addImage(const MediaDataId& mediaDataId, ImageId& imageId);
        virtual ErrorCode setPrimaryItem(const ImageId& imageId);
        virtual ErrorCode setItemDescription(const ImageId& imageId, const ItemDescription& itemDescription);
        virtual ErrorCode addMetadata(const MediaDataId& mediaDataId, MetadataItemId& metadataIemId);
        virtual ErrorCode addThumbnail(const ImageId& thumbImageId, const ImageId& masterImageId);
        virtual ErrorCode addProperty(const CleanAperture& clap, PropertyId& propertyId);
        virtual ErrorCode addProperty(const Mirror& imir, PropertyId& propertyId);
        virtual ErrorCode addProperty(const Rotate& irot, PropertyId& propertyId);
        virtual ErrorCode addProperty(const RelativeLocation& rloc, PropertyId& propertyId);
        virtual ErrorCode addProperty(const PixelAspectRatio& pasp, PropertyId& propertyId);
        virtual ErrorCode addProperty(const PixelInformation& pixi, PropertyId& propertyId);
        virtual ErrorCode addProperty(const ColourInformation& colr, PropertyId& propertyId);
        virtual ErrorCode addProperty(const AuxiliaryType& auxC, PropertyId& propertyId);
        virtual ErrorCode addProperty(const RawProperty& property, const bool isTransformative, PropertyId& propertyId);
        virtual ErrorCode associateProperty(const ImageId& imageId,
                                            const PropertyId& propertyId,
                                            const bool isEssential = false);
        virtual ErrorCode addDerivedImage(const ImageId& imageId, ImageId& derivedImageId);
        virtual ErrorCode addDerivedImageItem(const Grid& grid, ImageId& gridId);
        virtual ErrorCode addDerivedImageItem(const Overlay& iovl, ImageId& overlayId);

        virtual ErrorCode addMetadataItemReference(const MetadataItemId& metadataItemId, const ImageId& toImageId);
        virtual ErrorCode addTbasItemReference(const ImageId& fromImageId, const ImageId& toImageId);
        virtual ErrorCode addBaseItemReference(const ImageId& fromImageId, const Array<ImageId>& toImageIds);
        virtual ErrorCode addAuxiliaryReference(const ImageId& fromImageId, const ImageId& toImageId);
        virtual ErrorCode setImageHidden(const ImageId& imageId, const bool hidden);

        virtual ErrorCode addImageSequence(const Rational& timeBase,
                                           const CodingConstraints& aCodingConstraints,
                                           SequenceId& id);
        virtual ErrorCode addImage(const SequenceId& sequenceId,
                                   const MediaDataId& mediaDataId,
                                   const SampleInfo& sampleInfo,
                                   SequenceImageId& imageId);
        virtual ErrorCode addMetadataItemReference(const MetadataItemId& metadataItemId,
                                                   const SequenceId& sequenceId,
                                                   const SequenceImageId& imageId);
        virtual ErrorCode addThumbnails(const SequenceId& thumbSequenceId, const SequenceId& sequenceId);
        virtual ErrorCode setImageHidden(const SequenceImageId& sequenceImageId, const bool hidden);
        virtual ErrorCode addProperty(const CleanAperture& clap, const SequenceId& sequenceId);
        virtual ErrorCode addAuxiliaryReference(const AuxiliaryType& auxC,
                                                const SequenceId& auxiliarySequenceId,
                                                const SequenceId& sequenceId);
        virtual ErrorCode setEditList(const SequenceId& sequenceId, const EditList& editList);
        virtual ErrorCode setMatrix(const Array<int32_t>& matrix);
        virtual ErrorCode setMatrix(const SequenceId& sequenceId, const Array<int32_t>& matrix);

        virtual ErrorCode createEntityGroup(const FourCC& type, GroupId& id);
        virtual ErrorCode createAlternativesGroup(GroupId& id);
        virtual ErrorCode createEquivalenceGroup(GroupId& id);
        virtual ErrorCode addToGroup(const GroupId& groupId, const ImageId& id);
        virtual ErrorCode addToGroup(const GroupId& groupId, const SequenceId& id);
        virtual ErrorCode addToEquivalenceGroup(const GroupId& equivalenceGroupId,
                                                const SequenceId& sequenceId,
                                                const SequenceImageId& id,
                                                const EquivalenceTimeOffset& offset = {0, 1 << 8});

        virtual ErrorCode setAlternateGrouping(const SequenceId& sequenceId1, const SequenceId& sequenceId2);

        virtual ErrorCode addVideoTrack(const Rational& timeBase, SequenceId& id);
        virtual ErrorCode addVideo(const SequenceId& sequenceId,
                                   const MediaDataId& mediaDataId,
                                   const SampleInfo& sampleInfo,
                                   SequenceImageId& sampleid);
        virtual ErrorCode addAudioTrack(const Rational& timeBase, const AudioParams& config, SequenceId& id);
        virtual ErrorCode addAudio(const SequenceId& sequenceId,
                                   const MediaDataId& mediaDataId,
                                   const SampleInfo& sampleInfo,
                                   SequenceImageId& sampleid);

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
         */
        void addEmptyEdit(EditListBox* editListBox, const EditUnit& editUnit) const;

        /**
         * @brief addDwellEdit   Append a new dwell edit to an edit list box.
         * @param editListBox    Edit List Box where the new entry will be appended.
         * @param editUnit       Edit List entry where edit type is DWELL.
         */
        void addDwellEdit(EditListBox* editListBox, const EditUnit& editUnit) const;

        /**
         * @brief addShiftEdit   Append a new shift edit to an edit list box.
         * @param editListBox    Edit List Box where the new entry will be appended.
         * @param editUnit       Edit List entry where edit type is SHIFT.
         */
        void addShiftEdit(EditListBox* editListBox, const EditUnit& editUnit) const;

        /**
         * @brief Initialize internal data structures and ID generators of the writer.
         */
        void clear();

        /**
         * @brief Get track duration, in movie header time scale.
         * @return Track duration, in movie header time scale.
         */
        uint64_t getTrackDuration(TrackBox* track, const ImageSequence& sequence) const;

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
