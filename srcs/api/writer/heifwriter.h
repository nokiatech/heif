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

#ifndef HEIFWRITER_H
#define HEIFWRITER_H

#include "heifallocator.h"
#include "heifexport.h"
#include "heifwriterdatatypes.h"

namespace HEIF
{
    /**
     * HEIF (High Efficiency Image File Format) writer.
     * Standard specification:
     * http://standards.iso.org/ittf/PubliclyAvailableStandards/c066067_ISO_IEC_23008-12_2017.zip
     */
    class HEIF_DLL_PUBLIC Writer
    {
    public:
        /** Make an instance of Writer
         *
         *  If a custom memory allocator has not been set with SetCustomAllocator prior to
         *  calling this, the default allocator will be set into use.
         */
        static Writer* Create();

        /** Destroy the instance returned by Create */
        static void Destroy(Writer* instance);

        /** Set an optional custom memory allocator. Call this before calling Create for the
            first time, unless your new allocator is able to release blocks allocated with the
            previous allocator. The allocator is shared by all instances of
            HeifWriterInterface.

            If you wish to change the allocator after once setting it, you must first set it to
            nullptr, which succeeds always. After this you can change it to the desired value.

            @param [in] customAllocator the allocator to use
            @return ErrorCode: OK or ALLOCATOR_ALREADY_SET if the custom allocator has already been set
                    (possibly automatically upon use); in this case the allocator remains the same
                    as before this call.
        */
        static ErrorCode SetCustomAllocator(CustomAllocator* customAllocator);

        /**
         * Get library version string.
         * @return Version string. */
        static const char* GetVersion();

        /** Open file for writing.
         *  @param outputConfig  OutputConfig struct containing file name, brands and other output config information.
         *  @return ErrorCode: OK, ALREADY_INITIALIZED, BRANDS_NOT_SET or FILE_OPEN_ERROR
         */
        virtual ErrorCode initialize(const OutputConfig& outputConfig) = 0;

        /**
         * Set the major brand.
         * Calling this is possible only if the file type box was not already written (OutputConfig.progressiveFile =
         * true).
         * @param brand FourCC of the new major brand to be set.
         * @return ErrorCode: OK, UNINITIALIZED or FTYP_ALREADY_WRITTEN
         */
        virtual ErrorCode setMajorBrand(const FourCC& brand) = 0;

        /**
         * Add a new compatible brand.
         * Calling this is possible only if the file type box was not already written (OutputConfig.progressiveFile =
         * true).
         * @param brand FourCC of new compatible brand to append.
         * @return ErrorCode: OK, UNINITIALIZED or FTYP_ALREADY_WRITTEN
         */
        virtual ErrorCode addCompatibleBrand(const FourCC& brand) = 0;

        /**
         * Finalize the file writing.
         * @return ErrorCode: OK, UNINITIALIZED or BRANDS_NOT_SET
         */
        virtual ErrorCode finalize() = 0;

        ////////////////////////
        // Data input methods //
        ////////////////////////

        /**
         * Add a new decoder configuration for file content.
         * Needed only for encoded image MediaFormats (AVC/H.264, HEVC/H.265) and encoded AAC-LC audio.
         * @param decoderConfig    [in]  Decoder configuration data.
         * @param decoderConfigId  [out] DecoderConfigId of the added decoder configuration.
         *                               The Id must be provided along media data Data when feeding encoded data to
         * inform what decoder configuration should be used to decode that media data.
         * @return ErrorCode: OK or UNINITIALIZED
         */
        virtual ErrorCode feedDecoderConfig(const Array<DecoderSpecificInfo>& decoderConfig,
                                            DecoderConfigId& decoderConfigId) = 0;

        /**
         * Add new encoded image/video/audio data or external metadata (EXIF, XMP, MPEG-7) bytearray to MediaDataBox
         * ('mdat') of the file.
         * @param data        [in]  Data struct. Ownership of the data will not be transferred. It must be freed by the
         * caller. This can be done immediately after the call.
         * @param mediaDataId [out] MediaDataId for the added data. This can then be for example referred by addImage()
         * when creating images from the added data.
         * @return ErrorCode: OK, UNINITIALIZED, INVALID_DECODER_CONFIG_ID or INVALID_MEDIA_FORMAT
         */
        virtual ErrorCode feedMediaData(const Data& data, MediaDataId& mediaDataId) = 0;

        ///////////////////////////////////
        // HEIF Image Collection Methods //
        ///////////////////////////////////

        /**
         * Add an image to a HEIF Image Collection. Image bitstream must have been already fed by using feedMediaData().
         * @param mediaDataId  [in]  MediaDataId of the data from feedMediaData().
         * @param imageId      [out] ImageId of the added image.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_MEDIADATA_ID
         */
        virtual ErrorCode addImage(const MediaDataId& mediaDataId, ImageId& imageId) = 0;

        /**
         * Set primary item of the file.
         * Typically cover image / main thumbnail for the file.
         * The primary item of the file can not be hidden.
         * @param imageId    [in]  ImageId of the image item.
         * @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID or HIDDEN_PRIMARY_ITEM
         */
        virtual ErrorCode setPrimaryItem(const ImageId& imageId) = 0;

        /**
         * Set item description.
         * @param itemDescription    [in]  ItemDescription of the item.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_ITEM_ID
         */
        virtual ErrorCode setItemDescription(const ImageId& imageId, const ItemDescription& itemDescription) = 0;

        /**
         * Add external metadata (EXIF, XMP or MPEG-7) for an image in Image Collection
         * @param mediaDataId     [in]  MediaDataId of metadata that was previously fed to writer using feedMediaData().
         * @param metadataItemId  [out] MetadataItemId of the created metadata item.
         * @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID or INVALID_MEDIADATA_ID
         */
        virtual ErrorCode addMetadata(const MediaDataId& mediaDataId, MetadataItemId& metadataItemId) = 0;

        /**
         * Add a thumbnail image for an image.
         * This creates a 'thmb' reference from the thumbnail image to the
         * master image. A thumbnail can not be marked as a thumbnail of another thumbnail.
         * @param thumbImageId  [in] ImageId of the thumbnail image.
         * @param masterImageId [in] ImageId of the master image for which the thumbnail is added.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_ITEM_ID
         */
        virtual ErrorCode addThumbnail(const ImageId& thumbImageId, const ImageId& masterImageId) = 0;

        /**
         * Add a clean aperture 'clap' transformative property (cropping).
         * It can be then associated to one or several images by using method associateProperty().
         * @param clap       [in]  Parameters for the clean aperture property.
         * @param propertyId [out] PropertyId of the added property, to be used as a parameter of associateProperty().
         * @return ErrorCode: OK or UNINITIALIZED
         */
        virtual ErrorCode addProperty(const CleanAperture& clap, PropertyId& propertyId) = 0;

        /**
         * Add an image mirroring 'imir' transformative property.
         * It can be then associated to one or several images by using method associateProperty().
         * @param imir       [in]  Parameters for the the image mirroring property.
         * @param propertyId [out] PropertyId of the added property, to be used as a parameter of associateProperty().
         * @return ErrorCode: OK or UNINITIALIZED
         */
        virtual ErrorCode addProperty(const Mirror& imir, PropertyId& propertyId) = 0;

        /**
         * Add an image rotation 'irot' transformative property.
         * It can be then associated to one or several images by using method associateProperty().
         * @param irot       [in]  Parameters for the image rotation property.
         * @param propertyId [out] PropertyId of the added property, to be used as a parameter of associateProperty().
         * @return ErrorCode: OK or UNINITIALIZED
         */
        virtual ErrorCode addProperty(const Rotate& irot, PropertyId& propertyId) = 0;

        /**
         * Add a relative location 'rloc' descriptive property.
         * It can be then associated to one or several images by using method associateProperty().
         * If an image has an associated Relative location property, a 'tbas' reference from
         * it must be also present. The reference is created by using method addTbasItemReference().
         * @param rloc       [in]  Parameters for the relative location property.
         * @param propertyId [out] PropertyId of the added property, to be used as a parameter of associateProperty().
         * @return ErrorCode: OK or UNINITIALIZED
         */
        virtual ErrorCode addProperty(const RelativeLocation& rloc, PropertyId& propertyId) = 0;

        /**
         * Add a pixel information 'pixi' descriptive property.
         * It can be then associated to one or several images by using method associateProperty().
         * @param pixi       [in]  Parameters for the pixel information property.
         * @param propertyId [out] PropertyId of the added property, to be used as a parameter of associateProperty().
         * @return ErrorCode: OK or UNINITIALIZED
         */
        virtual ErrorCode addProperty(const PixelInformation& pixi, PropertyId& propertyId) = 0;

        /**
         * Add a colour information 'colr' descriptive property.
         * It can be then associated to one or several images by using method associateProperty().
         * @param colr       [in]  Parameters for the colour information property.
         * @param propertyId [out] PropertyId of the added property, to be used as a parameter of associateProperty().
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_FUNCTION_PARAMETER
         */
        virtual ErrorCode addProperty(const ColourInformation& colr, PropertyId& propertyId) = 0;

        /**
         * Add a pixel aspect ratio 'pasp' descriptive property.
         * It can be then associated to one or several images by using method associateProperty().
         * @param pasp       [in]  Parameters for the pixel aspect ratio property.
         * @param propertyId [out] PropertyId of the added property, to be used as a parameter of associateProperty().
         * @return ErrorCode: OK or UNINITIALIZED
         */
        virtual ErrorCode addProperty(const PixelAspectRatio& pasp, PropertyId& propertyId) = 0;

        /**
         * Add a auxiliary type 'auxC' descriptive property.
         * It can be then associated to one or several images by using method associateProperty().
         * If an image has an associated auxiliary property, a 'auxl' reference from
         * from the auxiliary image to the master image must be also present. The reference is created by using method
         * addAuxiliaryReference().
         * @param auxC       [in]  Parameters for the pixel aspect ratio property.
         * @param propertyId [out] PropertyId of the added property, to be used as a parameter of associateProperty().
         * @return ErrorCode: OK or UNINITIALIZED
         */
        virtual ErrorCode addProperty(const AuxiliaryType& auxC, PropertyId& propertyId) = 0;

        /**
         * Add a new property from binary data.
         * Note that complete property data including box or full box header must be supplied.
         * This can be used to add properties which are not supported by the writer, or for other
         * purposes when full control of property data is wanted.
         * The added property can then be associated to one or several images by using method associateProperty().
         * @param property         [in]  Full data of the property.
         * @param isTransformative [in]  True if this is a transformative property, false if it is a descriptive
         * property.
         * @param propertyId       [out] PropertyId of the added property, to be used as a parameter of
         * associateProperty().
         * @return ErrorCode: OK or UNINITIALIZED
         */
        virtual ErrorCode addProperty(const RawProperty& property,
                                      const bool isTransformative,
                                      PropertyId& propertyId) = 0;

        /**
         * Associate a property to an image.
         * The writer will arrange all descriptive property associations before transformative property associations.
         * Otherwise association order will be same as fed by using this method.
         * Note that it is possible to associate same property to several images.
         * @param imageId     [in]  ImageId of the existing image item.
         * @param propertyId  [in]  PropertyId of the existing property.
         * @param isEssential [out] True if this property is defined 'essential' for this image. A reader should
         *                     not present an image if it is not capable of processing an essential property.
         * @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID or INVALID_PROPERTY_INDEX
         */
        virtual ErrorCode associateProperty(const ImageId& imageId,
                                            const PropertyId& propertyId,
                                            const bool isEssential = false) = 0;

        /**
         * Create a new identity-derived image from an existing image.
         * This implies creation of an item of type 'iden' (identity transformation) and
         * 'dimg' item reference from the 'iden' image to the existing source item.
         * @param imageId        [in]  ImageId of the existing image item.
         * @param derivedImageId [out] ImageId of the created 'iden' image item.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_ITEM_ID
         */
        virtual ErrorCode addDerivedImage(const ImageId& imageId, ImageId& derivedImageId) = 0;

        /**
         * Create an image grid derived item.
         * @param grid     [in]  Parameters for the image grid item.
         * @param gridId   [out] Id of the created image grid.
         * @return ErrorCode: OK, UNINITIALIZED, INVALID_REFERENCE_COUNT, INVALID_FUNCTION_PARAMETER or INVALID_ITEM_ID
         */
        virtual ErrorCode addDerivedImageItem(const Grid& grid, ImageId& gridId) = 0;

        /**
         * Create an image overlay derived item.
         * @param iovl      [in]  Parameters for the image overlay item.
         * @param overlayId [out] Id of the created image overlay.
         * @return ErrorCode: OK, UNINITIALIZED, INVALID_REFERENCE_COUNT, INVALID_FUNCTION_PARAMETER or INVALID_ITEM_ID
         */
        virtual ErrorCode addDerivedImageItem(const Overlay& iovl, ImageId& overlayId) = 0;


        /** Add metadata reference from metadata item to Image that metadata describes.
         * @param fromMetadata [in]  MetadataItemId of linked metadata
         * @param toImageId    [in]  ImageId of image that the metadata is linked to
         */
        virtual ErrorCode addMetadataItemReference(const MetadataItemId& fromMetadata, const ImageId& toImageId) = 0;

        /**
         * Set 'tbas' item reference from an image associated with a Relative location ('rloc') property
         * to a reconstructed image.
         * If an image has an associated Relative location property, a 'tbas' reference from it must be also
         * present.
         * @param fromImageId Id of the item associated with Relative location ('rloc') property.
         * @param toImageId   Id of the reconstructed image, a part of which fromImageId is.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_ITEM_ID
         */
        virtual ErrorCode addTbasItemReference(const ImageId& fromImageId, const ImageId& toImageId) = 0;

        /**
         * Set 'base' item references for a pre-derived coded image.
         * @param fromImageId ImageId of the pre-derived coded image.
         * @param toImageIds  ImageId's of images the pre-derived coded image derives from.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_ITEM_ID
         */
        virtual ErrorCode addBaseItemReference(const ImageId& fromImageId, const Array<ImageId>& toImageIds) = 0;

        /**
         * Set an image as an auxiliary image of another image.
         * Adds item reference of type 'auxl' from the auxiliary image to the master image.
         * @param fromImageId ImageId of the auxiliary image.
         * @param toImageId   ImageId of the image for which the auxiliary image is added.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_ITEM_ID
         */
        virtual ErrorCode addAuxiliaryReference(const ImageId& fromImageId, const ImageId& toImageId) = 0;

        /**
         * Set an image hidden.
         * @param imageId Id of the image to hide or set visible.
         * @param hidden  True if image is hidden, false if not.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_ITEM_ID
         */
        virtual ErrorCode setImageHidden(const ImageId& imageId, const bool hidden) = 0;

        /////////////////////////////////
        // HEIF Image Sequence Methods //
        /////////////////////////////////

        /**
         * Add a new HEIF Image Sequence (track with 'pict' handler type) to the file.
         * @param timeBase      [in] Rational value of number of time units per second used for this image sequence
         * (usually 1 / timescale)
         * @param constraints   [in] CodingConstraints for this image sequence. See CodingConstraints struct definition
         * for more info.
         * @param id            [out] SequenceId identifier of the added sequence. This is not necessarily the same as
         * the corresponding track ID in the file.
         * @return ErrorCode: OK or UNINITIALIZED
         */
        virtual ErrorCode addImageSequence(const Rational& timeBase,
                                           const CodingConstraints& constraints,
                                           SequenceId& id) = 0;

        /**
         * Add an image to a image sequence. Image bitstream must have been already fed by using feedMediaData().
         * Note: Images must be fed in media data / decoding order!
         * @param sequenceId   [in]  SequenceId of the image sequence, returned by addImageSequence().
         * @param mediaDataId  [in]  MediaDataId of the data from feedMediaData().
         * @param sampleInfo   [in]  SampleInfo struct information of given mediaDataId to generate playable sequence.
         * @param imageId      [out] Identifier of the added image.
         * @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_MEDIADATA_ID, INVALID_FUNCTION_PARAMETER
         * or INVALID_MEDIA_FORMAT
         */
        virtual ErrorCode addImage(const SequenceId& sequenceId,
                                   const MediaDataId& mediaDataId,
                                   const SampleInfo& sampleInfo,
                                   SequenceImageId& imageId) = 0;

        /**
         * Add external metadata for an image in Image Sequence
         * @param metadataItemId  [in]  MetadataItemId of metadata.
         * @param sequenceId   [in]  SequenceId of the image sequence.
         * @param imageId      [in] SequenceImageId of the image to link metadata to.
         * @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID or INVALID_SEQUENCE_IMAGE_ID
         */
        virtual ErrorCode addMetadataItemReference(const MetadataItemId& metadataItemId,
                                                   const SequenceId& sequenceId,
                                                   const SequenceImageId& imageId) = 0;

        /** Add a thumbnail sequence for a track. This results in:
         *   - Creating 'thmb' track reference from thumbnail track to master image track.
         *   - Enabling track_in_preview flag of the TrackHeaderBox 'tkhd' of the thumbnail track
         *   - Setting the same integer value for alternate_group in both track headers.
         * @param thumbSequenceId  [in]  Id of the thumbnail image sequence.
         * @param sequenceId       [in]  Id of the image sequence for which the thumbnail sequence is added.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_SEQUENCE_ID
         */
        virtual ErrorCode addThumbnails(const SequenceId& thumbSequenceId, const SequenceId& sequenceId) = 0;

        /**
         * Set an image from Image Sequence to be hidden.
         * @param sequenceImageId  [in]  Id of the image to hide or set visible.
         * @param hidden           [in]  True if image is hidden, false if not.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_SEQUENCE_IMAGE_ID
         */
        virtual ErrorCode setImageHidden(const SequenceImageId& sequenceImageId, const bool hidden) = 0;

        /**
         * Add a clean aperture 'clap' (cropping) property to an image sequence.
         * @param clap        [in]  Parameters for the CleanApertureBox.
         * @param sequenceId  [in]  Id of the image sequence for which the CleanApertureBox is added.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_SEQUENCE_ID
         */
        virtual ErrorCode addProperty(const CleanAperture& clap, const SequenceId& sequenceId) = 0;

        /**
         * Set an Image Sequence as an auxiliary Image Sequence for another Image Sequence.
         *   - Changes auxiliary Image Sequence track handler type to 'auxv'.
         *   - Adds 'auxl' track reference from auxiliary Image Sequence to master Image Sequence.
         * @param auxC                  [in]   Type of the auxiliary image as AuxiliaryType struct
         *                                     AuxiliaryType.auxType : "urn:mpeg:hevc:2015:auxid:1" for alpha plane
         *                                                             "urn:mpeg:hevc:2015:auxid:2" for depth map
         * @param auxiliarySequenceId   [in]  SequenceId of the Image Sequence that is marked as auxiliary.
         * @param sequenceId            [in]  SequenceId of the Image Sequence for which the auxiliary Image Sequence is
         * added.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_SEQUENCE_ID
         */
        virtual ErrorCode addAuxiliaryReference(const AuxiliaryType& auxC,
                                                const SequenceId& auxiliarySequenceId,
                                                const SequenceId& sequenceId) = 0;

        /**
         * Set edit list for an image sequence.
         * @param sequenceId  [in]  SequenceId of the image sequence.
         * @param editList    [in]  EditList struct containing edit entries.
         * @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID or INVALID_FUNCTION_PARAMETER
         */
        virtual ErrorCode setEditList(const SequenceId& sequenceId, const EditList& editList) = 0;

        /**
         * Set global transformation matrix (in MovieHeaderBox).
         * Matrix values as defined in 6.2.2 of ISO/IEC 14496-12:2015(E) and 7.2.1 of ISO/IEC 23008-12:2017(E)
         * Default values {0x00010000,0,0,0,0x00010000,0,0,0,0x40000000} used if not set.
         * @param matrix  [in]  3x3 matrix as 9 values in Array of integers {a,b,u, c,d,v, x,y,w}.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_FUNCTION_PARAMETER
         */
        virtual ErrorCode setMatrix(const Array<int32_t>& matrix) = 0;

        /**
         * Set track level transformation matrix (in TrackHeaderBox).
         * Matrix values as defined in 6.2.2 of ISO/IEC 14496-12:2015(E) and 7.2.1 of ISO/IEC 23008-12:2017(E)
         * Default values {0x00010000,0,0,0,0x00010000,0,0,0,0x40000000} used if not set.
         * @param matrix      [in]  3x3 matrix as 9 values in Array of integers {a,b,u, c,d,v, x,y,w}.
         * @param sequenceId  [in]  Id of the image sequence.
         * @return ErrorCode: OK, UNINITIALIZED or INVALID_FUNCTION_PARAMETER
         */
        virtual ErrorCode setMatrix(const SequenceId& sequenceId, const Array<int32_t>& matrix) = 0;

        //////////////////////
        // Grouping Methods //
        //////////////////////

        /**
         * Set image sequences (or video tracks) to the same alternate group. This signals that these sequences contain
         * alternate data for one another.
         *
         * This alternate group is specified in ISOBMFF TrackHeaderBox ('thkd') by field alternate_group.
         * A single image sequence can be a member of only one alternate group.
         * When a thumbnail image sequence is associated to a master image sequence, both sequences will get assigned
         * to the same alternate group automatically.
         *
         * If neither of given sequences has an alternate group already assigned, a new alternate group will be
         * created and both sequences are assigned to it.
         * If either of sequences given as parameter already has an alternate group assigned, the other sequence
         * will be assigned to the same alternate group.
         * If both sequences have an alternate group already set, the alternate group of sequenceId2 will be overridden
         * by the alternate group of the sequenceId1.
         *
         * Note that this grouping is different from 'altr' type entity grouping which is handled by using
         * methods createAlternativesGroup and addToGroup. Those methods can not to be used to handle
         * this TrackHeaderBox alternate grouping. However, both groupings can co-exist as needed.
         * @param sequenceId1  [in]  Image or video sequence ID.
         * @param sequenceId2  [in]  Image or video sequence ID.
         * @return ErrorCode: OK, UNITIALIZED, INVALID_SEQUENCE_ID
         */
        virtual ErrorCode setAlternateGrouping(const SequenceId& sequenceId1, const SequenceId& sequenceId2) = 0;

        /**
         * Create a new entity grouping. Generic grouping for any FourCC identifier.
         * @param type [in]  Type of the entity grouping to add.
         * @param id   [out] Id of the created entity grouping.
         * @return ErrorCode: OK or UNINITIALIZED
         */
        virtual ErrorCode createEntityGroup(const FourCC& type, GroupId& id) = 0;

        /**
         * Create a new Alternative ('altr') entity grouping. Helper method that internally wraps createEntityGroup().
         * @param id [out] Id of the created 'altr' entity grouping.
         * @return ErrorCode: OK or UNINITIALIZED
         */
        virtual ErrorCode createAlternativesGroup(GroupId& id) = 0;

        /**
         * Create a new Equivalence ('eqiv') entity grouping. Helper method that internally wraps createEntityGroup().
         * Note: If any SequenceId are added to the group then further calls to addToEquivalenceGroup() are needed
         *       as image (=sample) specific information is needed (else none of the samples of sequece will be marked
         * as equivalence).
         * @param id     [out]  Id of the created 'eqiv' entity grouping.
         * @return ErrorCode: OK or UNINITIALIZED
         */
        virtual ErrorCode createEquivalenceGroup(GroupId& id) = 0;

        /**
         * Add a image item to an entity group.
         * @param groupId   [in]  GroupId of the entity group.
         * @param id        [in]  ImageId of the image item to add.
         * @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID or INVALID_GROUP_ID
         */
        virtual ErrorCode addToGroup(const GroupId& groupId, const ImageId& id) = 0;

        /**
         * Add a seqeunce image to an existing entity group.
         * @param groupId   [in]  GroupId of the entity group.
         * @param id        [in]  SequenceId of the image sequence added to the group.
         * @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID or INVALID_GROUP_ID
         */
        virtual ErrorCode addToGroup(const GroupId& groupId, const SequenceId& id) = 0;

        /**
         * Add a Image Seqeunce image to an equivalence ('eqiv') entity group. Image Collection images can use generic
         * addToGroup() instead.
         * @param equivalenceGroupId  [in]  GroupId of the Equivalence entity group.
         * @param id                  [in]  SequenceImageId of the image sequence image which is added to the group.
         * @param offset              [in]  Optional: In case equivalence image item doesn't match give SequenceImageId
         * timing on image sequence timeline this offset value can be added (see HEIF specification "6.8	Relating an
         * untimed item to a timed sequence" for details) to indicate this.
         * @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_IMAGE_ID or INVALID_GROUP_ID
         */
        virtual ErrorCode addToEquivalenceGroup(const GroupId& equivalenceGroupId,
                                                const SequenceId& sequenceId,
                                                const SequenceImageId& sequenceImageId,
                                                const EquivalenceTimeOffset& offset = {0, 1 << 8}) = 0;

        /////////////////////////////////////////////////////////////////////
        // Support for optional MP4 video and audio tracks writing methods //
        /////////////////////////////////////////////////////////////////////

        /**
         * Add a new video (track with 'vide' handler type) to the file. For HEIF Image sequences use addImageSequence()
         * instead.
         * @param timeBase      [in] Rational value of number of time units per second used for this video track
         * (usually 1 / timescale)
         * @param id            [out] Identifier of the added track. This is not necessary same the corresponding track
         * ID in the file.
         * @return ErrorCode: OK, UNINITIALIZED
         */
        virtual ErrorCode addVideoTrack(const Rational& timeBase, SequenceId& id) = 0;

        /**
         * Add an video sample to a video track. Video bitstream must have been already fed by using feedMediaData().
         * Note: Video must be fed in media data / decoding order!
         * For HEIF Image Sequences use addImage() instead.
         * @param id           [in]  SequenceId of the video track, generated by addVideoTrack().
         * @param mediaDataId  [in]  MediaDataId of the data from feedMediaData().
         * @param sampleInfo   [in]  Information of given mediaDataId to generate playable sequence.
         * @param sampleid     [out] Id for the sample that was added to the track
         * @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_MEDIADATA_ID, INVALID_FUNCTION_PARAMETER
         * or INVALID_MEDIA_FORMAT
         */
        virtual ErrorCode addVideo(const SequenceId& id,
                                   const MediaDataId& mediaDataId,
                                   const SampleInfo& sampleInfo,
                                   SequenceImageId& sampleid) = 0;

        /**
         * Add a new audio (track with 'soun' handler type) to the file.
         * @param timeBase [in] Rational value of number of time units per second used for this audio track (usually 1 /
         * samplerate)
         * @param config   [in] AudioParams for this audio track. See AudioParams struct definition for more info.
         * @param id       [out] SequenceId identifier of the added track. This is not necessary same the corresponding
         * track ID in the file.
         * @return ErrorCode: OK, UNINITIALIZED
         */
        virtual ErrorCode addAudioTrack(const Rational& timeBase, const AudioParams& config, SequenceId& id) = 0;

        /**
         * Add an audio sample to a audio track. Audio bitstream must have been already fed by using feedMediaData().
         * Note: Audio must be fed in media data / decoding order!
         * @param id           [in]  Id of the audio track, generated by addAudioTrack().
         * @param mediaDataId  [in]  Id of the data from feedMediaData().
         * @param sampleInfo   [in]  Information of given mediaDataId to generate playable sequence.
         * @param sampleid     [out] Id for the sample that was added to the track
         * @return ErrorCode:  OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_MEDIADATA_ID, INVALID_FUNCTION_PARAMETER
         * or INVALID_MEDIA_FORMAT
         */
        virtual ErrorCode addAudio(const SequenceId& id,
                                   const MediaDataId& mediaDataId,
                                   const SampleInfo& sampleInfo,
                                   SequenceImageId& sampleid) = 0;

    protected:
        virtual ~Writer() = default;
    };
}  // namespace HEIF

#endif  // HEIFWRITER_H
