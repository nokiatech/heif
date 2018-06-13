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

#ifndef HEIFREADER_H
#define HEIFREADER_H

#include <cstdint>
#include "heifallocator.h"
#include "heifexport.h"
#include "heifreaderdatatypes.h"

namespace HEIF
{
    class StreamInterface;

    /** Interface for reading an High Efficiency Image File Format (HEIF) file. */
    class HEIF_DLL_PUBLIC Reader
    {
    public:
        /** Make an instance of HeifReader
         *
         *  If a custom memory allocator has not been set with SetCustomAllocator prior to
         *  calling this, the default allocator will be set into use. */
        static Reader* Create();

        /** Destroy the instance returned by Create */
        static void Destroy(Reader* imageFileInterface);

        /** Set an optional custom memory allocator. Call this before calling Create for the
         *  first time, unless your new allocator is able to release blocks allocated with the
         *   previous allocator. The allocator is shared by all instances of
         *   Reader.
         *
         *  If you wish to change the allocator after once setting it, you must first set it to
         *  nullptr, which succeeds always. After this you can change it to the desired value.
         *
         *  @param [in] customAllocator  The allocator to use.
         *  @return ErrorCode: OK, ALLOCATOR_ALREADY_SET if the custom allocator has already been set
         *          (possibly automatically upon use); in this case the allocator remains the same
         *          as before this call. */
        static ErrorCode SetCustomAllocator(CustomAllocator* customAllocator);

        /**
         * Get library version string.
         * @return Version string. */
        static const char* GetVersion();

        /*---------- Interface methods are defined as follows:--------------------- */

        /** Open a file for reading and read the file header information.
         *  @param [in] fileName File to open.
         *  @return ErrorCode: OK, FILE_OPEN_ERROR, FILE_READ_ERROR, FILE_HEADER_ERROR */
        virtual ErrorCode initialize(const char* fileName) = 0;

        /** Open an input stream for reading and read the header information.
         *  @param input Stream to open.
         *  @return ErrorCode: OK, FILE_HEADER_ERROR, FILE_READ_ERROR */
        virtual ErrorCode initialize(StreamInterface* input) = 0;

        /** Reset reader internal state. */
        virtual void close() = 0;

        /** @param [out] majorBrand Major brand from the File Type Box
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED */
        virtual ErrorCode getMajorBrand(FourCC& majorBrand) const = 0;

        /** @param [out] minorVersion Minor version from the File Type Box
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED */
        virtual ErrorCode getMinorVersion(uint32_t& minorVersion) const = 0;

        /** @param [out] compatibleBrands Compatible brands list from the File Type Box
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED */
        virtual ErrorCode getCompatibleBrands(Array<FourCC>& compatibleBrands) const = 0;

        /** Get file information.
         *  This information can be used to further initialize the presentation of the data in the file.
         *  Information also give hints about the way and means to request data from the file.
         *  @pre initialize() has been called successfully.
         *  @param [out] fileinfo FileInformation struct that hold file information.
         *  @return ErrorCode: OK or UNINITIALIZED */
        virtual ErrorCode getFileInformation(FileInformation& fileinfo) const = 0;

        /** Get maximum display width from track headers.
         *  @param [in]  sequenceId    Image sequence ID (track ID).
         *  @param [out] displayWidth  Maximum display width in pixels.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID */
        virtual ErrorCode getDisplayWidth(const SequenceId& sequenceId, uint32_t& displayWidth) const = 0;

        /** Get maximum display height from track headers.
         *  @param [in]  sequenceId     Image sequence ID (track ID).
         *  @param [out] displayHeight  Maximum display height in pixels.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID */
        virtual ErrorCode getDisplayHeight(const SequenceId& sequenceId, uint32_t& displayHeight) const = 0;

        /** Get width of an image item.
         *  @param [in]   imageId An image item in the metabox
         *  @param [out]  width  Width in pixels.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID */
        virtual ErrorCode getWidth(const ImageId& imageId, uint32_t& width) const = 0;

        /** Get width of an image/sample.
         *  @param [in]  sequenceId Image sequence ID (track ID).
         *  @param [in]  imageId    Identifier of an image in the sequence (a sample).
         *  @param [out] width      Width in pixels.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_SEQUENCE_IMAGE_ID */
        virtual ErrorCode getWidth(const SequenceId& sequenceId,
                                   const SequenceImageId& imageId,
                                   uint32_t& width) const = 0;

        /** Get height of an image item.
         *  @param [in]  imageId An image item in the metabox
         *  @param [out] height  Height in pixels.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID */
        virtual ErrorCode getHeight(const ImageId& imageId, uint32_t& height) const = 0;

        /** Get height of an image/sample.
         *  @param [in]  sequenceId Image sequence ID (track ID).
         *  @param [in]  imageId    Identifier of an image in the sequence (a sample).
         *  @param [out] height     Height in pixels.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_SEQUENCE_IMAGE_ID */
        virtual ErrorCode getHeight(const SequenceId& sequenceId,
                                    const SequenceImageId& imageId,
                                    uint32_t& height) const = 0;

        /** Get transformation matrix for the video, from the Movie Header Box of the file (if present).
         *  @param [out] matrix The transformation matrix, consisting of 9 32-bit integers.
         *                      Returns error code NOT_APPLICABLE if a Movie Header Box is not present in
         *                      the file.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED */
        virtual ErrorCode getMatrix(Array<int32_t>& matrix) const = 0;

        /** Get transformation matrix for the video track, from a Track Header Box of the file.
         *  @param [in]  sequenceId Image sequence ID (track ID).
         *  @param [out] matrix     The transformation matrix, consisting of 9 32-bit integers.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, INVALID_SEQUENCE_ID, UNINITIALIZED */
        virtual ErrorCode getMatrix(const SequenceId& sequenceId, Array<int32_t>& matrix) const = 0;

        /** Get playback duration of image sequence or media track in seconds.
         *  This considers also edit lists.
         *  @param [in]  sequenceId Image sequence ID (track ID).
         *  @param [out] duration   The playback duration of track in seconds.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID */
        virtual ErrorCode getPlaybackDurationInSecs(const SequenceId& sequenceId, double& duration) const = 0;

        /** Get an array of items in MetaBox having the requested type.
         *  @param [in]  itemType  Four-character code of the item type (e.g. 'hvc1', 'iovl', 'grid', 'Exif', 'mime',
         * 'hvt1', 'iden')
         *  @param [out] imageIds  Found items.
         *                         An empty Array if no items are found.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED */
        virtual ErrorCode getItemListByType(const FourCC& itemType, Array<ImageId>& imageIds) const = 0;

        /** Get an array of items in the container with the ID sequenceId having the requested itemType.
         *  @param [in]  sequenceId Image sequence ID (track ID).
         *  @param [in]  itemType   Type of samples to request.
         *  @param [out] imageIds   Array of found items.
         *                          An empty Array if no items are found.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_FUNCTION_PARAMETER, INVALID_SEQUENCE_ID */
        virtual ErrorCode getItemListByType(const SequenceId& sequenceId,
                                            const TrackSampleType& itemType,
                                            Array<SequenceImageId>& imageIds) const = 0;

        /** Get an array of master image items of the image collection.
         *  @param [out] imageIds  Found items, if any. The order of the item ids are as present in the file.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED */
        virtual ErrorCode getMasterImages(Array<ImageId>& imageIds) const = 0;

        /** Get list of master image items of an image sequence.
         *  @param [in]  sequenceId Image sequence ID (track ID).
         *  @param [out] imageIds   Found items, if any. The order of the item ids are as present in the file.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID */
        virtual ErrorCode getMasterImages(const SequenceId& sequenceId, Array<SequenceImageId>& imageIds) const = 0;

        /** Get type of an item.
         *  @param [in]  itemId Id of an item in the image collection.
         *  @param [out] type   Four-character code of the item type (e.g. 'hvc1', 'iovl', 'grid', 'Exif', 'mime',
         *                      'hvt1', 'iden')
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID */
        virtual ErrorCode getItemType(const ImageId& itemId, FourCC& type) const = 0;

        /** Get sample description entry type of a sample / sequence image.
         *  @param [in]  sequenceId Image sequence ID (track ID).
         *  @param [in]  imageId    Identifier of an image in the sequence (a sample).
         *  @param [out] type       Sample description entry type is returned.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_SEQUENCE_IMAGE_ID */
        virtual ErrorCode getItemType(const SequenceId& sequenceId,
                                      const SequenceImageId& imageId,
                                      FourCC& type) const = 0;

        /** Get items which are referenced by the given item with a certain type of reference.
         *  @param [in]  fromItemId    Item id (reference from).
         *  @param [in]  referenceType Reference type (e.g. 'thmb', 'cdcs', 'auxl', 'dimg', 'base').
         *  @param [out] itemIds       Referenced item ids.
         *                             The order of the item ids is as present in the file.
         *                             An empty Array if no items are found.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID */
        virtual ErrorCode getReferencedFromItemListByType(const ImageId& fromItemId,
                                                          const FourCC& referenceType,
                                                          Array<ImageId>& itemIds) const = 0;

        /** Get items which are referencing the given item with a certain type of reference.
         *  @param [in]  toItemId      Item id (reference to).
         *  @param [in]  referenceType Reference type (e.g. 'thmb', 'cdcs', 'auxl', 'dimg', 'base').
         *  @param [out] itemIds       Referencing item ids.
         *                             The order of the item ids is as present in the file.
         *                             An empty Array if no items are found.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID */
        virtual ErrorCode getReferencedToItemListByType(const ImageId& toItemId,
                                                        const FourCC& referenceType,
                                                        Array<ImageId>& itemIds) const = 0;

        /** Get the ID of the primary item of the metabox/file.
         *  @param [out] imageId Item ID of the primary item.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED or PRIMARY_ITEM_NOT_SET */
        virtual ErrorCode getPrimaryItem(ImageId& imageId) const = 0;

        /** Get item data.
         *  Item data does not contain initialization or configuration data (i.e. decoder configuration records).
         *  By default nal-length values of 'hvc1'/'avc1' type encoded image data is substituted with bytestream
         * headers. For protected items pure data is returned always. Information how to handle such data is available
         * from getItemProtectionScheme() which returns related 'sinf' box as whole. Note that getItemData() is the only
         * reader API method which can be used for requesting data of such items.
         *  @param [in]      imageId           Item id of the image.
         *  @param [in,out]  memoryBuffer      Memory buffer where  data is to be written to.
         *  @param [in,out]  memoryBufferSize  Memory buffer size.
         *  @param [in]      bytestreamHeaders Optional - by default true. Whether to substitute H.264/H.265 nal-lenght
         * values with bytestream header (0001).
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID, FILE_READ_ERROR */
        virtual ErrorCode getItemData(const ImageId& imageId,
                                      uint8_t* memoryBuffer,
                                      uint64_t& memoryBufferSize,
                                      bool bytestreamHeaders = true) const = 0;

        /** Get item data for a sample/image in an image sequence.
         *  Item data does not contain initialization or configuration data (i.e. decoder configuration records).
         *  By default nal-length values of 'hvc1'/'avc1' type encoded image data is substituted with bytestream
         * headers. For protected items pure data is returned always. Information how to handle such data is available
         * from getItemProtectionScheme() which returns related 'sinf' box as whole. Note that getItemData() is the only
         * reader API method which can be used for requesting data of such items.
         *  @param [in]  sequenceId            Image sequence ID (track ID).
         *  @param [in]  imageId               Identifier of an image in the sequence (a sample).
         *  @param [in,out] memoryBuffer       Memory buffer where  data is to be written to.
         *  @param [in,out] memoryBufferSize   Memory buffer size.
         *  @param [in]  bytestreamHeaders     Optional - by default true. Whether to substitute H.264/H.265 nal-lenght
         * values with bytestream header (0001).
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_ITEM_ID */
        virtual ErrorCode getItemData(const SequenceId& sequenceId,
                                      const SequenceImageId& imageId,
                                      uint8_t* memoryBuffer,
                                      uint64_t& memoryBufferSize,
                                      bool bytestreamHeaders = true) const = 0;

        /** Get data of an image overlay item (item type 'iovl').
         *  @param [in]  imageId   Id of Image overlay item
         *  @param [out] iovlItem  Overlay derived item struct with requested data.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID, PROTECTED_ITEM */
        virtual ErrorCode getItem(const ImageId& imageId, Overlay& iovlItem) const = 0;

        /** Get data of an image grid item (item type 'grid').
         *  @param [in]  imageId   Id of Image overlay item
         *  @param [out] gridItem  Grid derived item struct with requested data.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID, PROTECTED_ITEM */
        virtual ErrorCode getItem(const ImageId& imageId, Grid& gridItem) const = 0;

        /** Get item property Image Mirror ('imir')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] imir   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, Mirror& imir) const = 0;

        /** Get item property Image Rotation ('irot')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] irot   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, Rotate& irot) const = 0;

        /** Get item property Relative Location ('rloc')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] rloc   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, RelativeLocation& rloc) const = 0;

        /** Get item property Pixel information ('pixi')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] pixi   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, PixelInformation& pixi) const = 0;

        /** Get item property Colour information ('colr')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] colr   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, ColourInformation& colr) const = 0;

        /** Get item property Pixel aspect ratio ('pasp')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] pasp   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, PixelAspectRatio& pasp) const = 0;

        /** Get property Clean aperture ('clap')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] clap   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, CleanAperture& clap) const = 0;

        /** Get image properties for auxiliary images ('auxC')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] auxC   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, AuxiliaryType& auxC) const = 0;

        /** Get raw data of an item property.
         *  @param [in]  index     Id of the property. @see getItemProperties()
         *  @param [out] property  Property data.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, RawProperty& property) const = 0;

        /** Get Clean aperture ('clap') information from a sample description entry of a track.
         *  @param [in]  sequenceId Image sequence ID (track ID).
         *  @param [in]  index      Sample description index.
         *  @param [out] clap       Requested clean aperture data.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_SAMPLE_DESCRIPTION_INDEX */
        virtual ErrorCode getProperty(const SequenceId& sequenceId,
                                      const uint32_t index,
                                      CleanAperture& clap) const = 0;

        /** Get Auxiliary type info box ('auxi') information from a sample description entry of a track.
         *  @param [in]  sequenceId Image sequence ID (track ID).
         *  @param [in]  index      Sample description index.
         *  @param [out] auxi       Requested auxiliary data.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_SAMPLE_DESCRIPTION_INDEX */
        virtual ErrorCode getProperty(const SequenceId& sequenceId,
                                      const uint32_t index,
                                      AuxiliaryType& auxi) const = 0;

        /** Get properties of an item.
         *  @param [in]  imageId       Item ID which properties to get.
         *  @param [out] propertyTypes Properties of an item.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID */
        virtual ErrorCode getItemProperties(const ImageId& imageId, Array<ItemPropertyInfo>& propertyTypes) const = 0;

        /** Get data of an encoded image item.
         *  This method shall not be used if the item is not of 'hvc1', 'avc1' or 'master' type. @see getItemData()
         *  @param [in] imageId               Item id.
         *  @param [in,out] memoryBuffer      Memory buffer where data is to be written to.
         *  @param [in,out] memoryBufferSize  Memory buffer size.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID, PROTECTED_ITEM, UNSUPPORTED_CODE_TYPE,
         *                     BUFFER_SIZE_TOO_SMALL */
        virtual ErrorCode getItemDataWithDecoderParameters(const ImageId& imageId,
                                                           uint8_t* memoryBuffer,
                                                           uint64_t& memoryBufferSize) const = 0;

        /** Get data of an image sequence image.
         *  This method shall be used only for 'hvc1' or 'avc1' type images.
         *  @param [in] sequenceId            Image sequence ID (track ID).
         *  @param [in] imageId               Identifier of an image in the sequence (a sample).
         *  @param [in,out] memoryBuffer      Memory buffer where data is to be written to.
         *  @param [in,out] memoryBufferSize  Memory buffer size.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_SEQUENCE_IMAGE_ID, UNSUPPORTED_CODE_TYPE,
         *                     BUFFER_SIZE_TOO_SMALL */
        virtual ErrorCode getItemDataWithDecoderParameters(const SequenceId& sequenceId,
                                                           const SequenceImageId& imageId,
                                                           uint8_t* memoryBuffer,
                                                           uint64_t& memoryBufferSize) const = 0;

        /** Get Protection Scheme Information Box for a protected item.
         *  @param [in] imageId               Item id.
         *  @param [in,out] memoryBuffer      Memory buffer where 'sinf' data is to be written to.
         *  @param [in,out] memoryBufferSize  Memory buffer size.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID, UNPROTECTED_ITEM, BUFFER_SIZE_TOO_SMALL]
         */
        virtual ErrorCode getItemProtectionScheme(const ImageId& imageId,
                                                  uint8_t* memoryBuffer,
                                                  uint64_t& memoryBufferSize) const = 0;

        /** Get display timestamp for each item of a track/sequence.
         *  @param [in]  sequenceId Image sequence ID (track ID).
         *  @param [out] timestamps Array of timestamps in milliseconds. Timestamp are truncated integer from float.
         *                          For non-output samples, an empty Array.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID */
        virtual ErrorCode getItemTimestamps(const SequenceId& sequenceId, Array<TimestampIDPair>& timestamps) const = 0;

        /** Get display timestamps of an image. An image may be displayed many times based on the edit list.
         *  @param [in]  sequenceId Image sequence ID (track ID).
         *  @param [in]  imageId    Identifier of an image in the sequence (a sample).
         *  @param [out] timestamps Array of timestamps. For non-output samples, an empty Array.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_SEQUENCE_IMAGE_ID */
        virtual ErrorCode getTimestampsOfItem(const SequenceId& sequenceId,
                                              const SequenceImageId& imageId,
                                              Array<int64_t>& timestamps) const = 0;

        /** Get items in decoding order.
         *  @param [in]  sequenceId        Image sequence ID (track ID).
         *  @param [out] decodingOrder     TimestampIDPair struct of <display timestamp in milliseconds, sample id>
         * pairs. Also complete decoding dependencies are listed here. If an sample ID is present as a decoding
         * dependency for a succeeding frame, its timestamp is set to 0xffffffff. Array is sorted by composition time.
         *                                 Timestamps in milliseconds. Timestamp truncated integer from float.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID */
        virtual ErrorCode getItemsInDecodingOrder(const SequenceId& sequenceId,
                                                  Array<TimestampIDPair>& decodingOrder) const = 0;

        /** Retrieve decoding dependencies for given imageId, in decoding order.
         *  This method should be used to retrieve referenced samples of a sample in a track.
         *  @param [in]  sequenceId    Image sequence ID (track ID).
         *  @param [in]  imageId       Identifier of an image in the sequence (a sample).
         *  @param [out] dependencies  Array including dependent item IDs:
         *                             For images that has no dependencies, this method returns imageId itself.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_SEQUENCE_IMAGE_ID */
        virtual ErrorCode getDecodeDependencies(const SequenceId& sequenceId,
                                                const SequenceImageId& imageId,
                                                Array<SequenceImageId>& dependencies) const = 0;

        /** Get coding type for image collection image.
         *  @param [in]  imageId Identifier of an image item.
         *  @param [out] type    Decoder code type, e.g. "hvc1" or "avc1".
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID. INVALID_ITEM_ID is also returned in case the item ID
         * exists, but it is a grid or overlay image item without own encoded data. */
        virtual ErrorCode getDecoderCodeType(const ImageId& imageId, FourCC& type) const = 0;

        /** Get decoder code type for an image/sample.
         *  @param [in]  sequenceId Image sequence ID (track ID).
         *  @param [in]  imageId    Identifier of an image in the sequence (a sample).
         *  @param [out] type       Decoder code type, e.g. "hvc1" or "avc1".
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_SEQUENCE_IMAGE_ID. */
        virtual ErrorCode getDecoderCodeType(const SequenceId& sequenceId,
                                             const SequenceImageId& imageId,
                                             FourCC& type) const = 0;

        /** Get decoder configuration record parameter sets.
         *  The item must be a decodable image item, e.g. 'hvc1', 'avc1'.
         *  This method should not be called for items which have no own encoded data, like identity derived images
         *  (type 'iden'), image overlay and grid derived images. Calling the method for such item id will return error
         * code INVALID_ITEM_ID.
         *  @param [in]  imageId       Identifier of an image item.
         *  @param [out] decoderInfos  DecoderConfiguration struct containing:
         *                             DecoderConfigId             Property Id for decoder configs
         *                             Array<DecoderSpecificInfo>  Array of decoder configs with type
         * DecoderSpecInfoType and actual payload for each.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID */
        virtual ErrorCode getDecoderParameterSets(const ImageId& imageId, DecoderConfiguration& decoderInfos) const = 0;

        /** Get decoder configuration record parameter sets.
         *  @param [in]  sequenceId    Image sequence ID (track ID).
         *  @param [in]  imageId       Identifier of an image in the sequence (a sample).
         *  @param [out] decoderInfos  DecoderConfiguration struct containing:
         *                             DecoderConfigId             Property Id for decoder configs (only unique within
         *                                                         same SequenceId)
         *                             Array<DecoderSpecificInfo>  Array of decoder configs with type
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_SEQUENCE_IMAGE_ID */
        virtual ErrorCode getDecoderParameterSets(const SequenceId& sequenceId,
                                                  const SequenceImageId& imageId,
                                                  DecoderConfiguration& decoderInfos) const = 0;

    protected:
        virtual ~Reader() = default;
    };

}  // namespace HEIF

#endif /* HEIFREADER_H */
