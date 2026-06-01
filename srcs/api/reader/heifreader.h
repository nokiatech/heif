/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
        /**
         * Thread-safety contract (normative).
         *
         * | Surface | Concurrency rule |
         * |---|---|
         * | `Reader::Create()` / `Destroy()` | Thread-safe. May be called concurrently from any thread. The library does not retain any reference to the returned instance. |
         * | A live `Reader*` instance | **Not** safe for concurrent use from multiple threads. Each consumer thread should hold its own instance. Multiple instances may exist concurrently. This is an *architectural* contract, not a missing feature: a `Reader` carries mutable parser state (current box position, partially-resolved item references, lazy `FileInformation` cache, internal allocator scratch) and that state is intentionally lock-free. Concurrent calls on a single instance are undefined behavior and not detected by the library; if your binding needs N readers it must construct N `Reader` instances rather than synchronise around one. The skipped test `concurrency_test.cpp::SingleReaderManyThreads_NotPartOfContract` exists to make this restriction discoverable from the test corpus. |
         * | `Reader::SetCustomAllocator()` / `GetVersion()` | Process-wide. Call `SetCustomAllocator` once at process start, before the first `Create()` call. Calling it from one thread while another thread is inside any `Reader` method is undefined. `GetVersion()` is always safe. |
         * | `HEIF::ErrorInfo` slot | `thread_local`. A failure on thread A is invisible to thread B. Drains via `HEIF::getErrorInfo()` / `heif_get_last_error_*` on the *same* thread that observed the non-OK return. |
         * | `HEIF::setErrorSink()` / sink callback | The setter is thread-safe; the sink + user pair is published atomically (no torn pair). The sink itself is invoked **on whichever thread produced the failure**, possibly concurrently from many threads. If your sink writes to a shared logger, ensure the logger is thread-safe. |
         * | `StreamInterface` (the `initialize(StreamInterface*)` overload) | The library calls the stream callbacks **only from the thread that called the `Reader` method**, never concurrently on a single instance. The same `StreamInterface*` MAY be shared across multiple `Reader` instances if and only if the implementation is itself thread-safe. |
         *
         * **Embedder pattern for multi-threaded consumers:** keep a
         * thread-local `Reader*` (or a small per-thread pool) and
         * never share an instance across threads. Cross-thread
         * coordination is then limited to whatever the embedder
         * already does at the application level; no library-side
         * synchronisation is required.
         */
        /** Make an instance of HeifReader
         *
         *  If a custom memory allocator has not been set with SetCustomAllocator prior to
         *  calling this, the default allocator will be set into use.
         *
         *  @throws std::bad_alloc only. All other failures are deferred to
         *          initialize() and surfaced through the ErrorCode return
         *          channel; Create() itself never throws any other exception
         *          and never returns nullptr (allocation failure throws).
         *
         *  @note  The returned pointer **must** be released via Reader::Destroy().
         *         Marked [[nodiscard]] so an embedder cannot silently leak the
         *         instance by ignoring the return value (e.g. accidental
         *         `Reader::Create();` instead of `auto* r = Reader::Create();`).
         *         The library does not retain any reference to the instance; lifetime
         *         is entirely the caller's responsibility. */
        [[nodiscard]] static Reader* Create();

        /** Destroy the instance returned by Create.
         *  Guaranteed to never throw - safe to call from destructors,
         *  catch handlers, and JNI/COM bindings that assume no-throw. */
        static void Destroy(Reader* imageFileInterface) noexcept;

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

        /** @param [out] compatibleBrandCombinations Compatible brand combinations from the
         *                                           (optional)root-level Extended Type Box.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED */
        virtual ErrorCode getCompatibleBrandCombinations(Array<Array<FourCC>>& compatibleBrandCombinations) const = 0;

        /** Get file information.
         *  This information can be used to further initialize the presentation of the data in the file.
         *  Information also give hints about the way and means to request data from the file.
         *
         *  Snapshot semantics. The returned struct reflects whatever
         *  the reader has parsed so far:
         *  - Non-fragmented input (single `initialize()` call against a
         *    file or a complete stream): all sample-table data is
         *    present and `TrackInformation::sampleProperties` is fully
         *    populated.
         *  - Fragmented streaming flow (`parseInitializationSegment` +
         *    `parseSegment`): only init-segment-derived fields are
         *    populated until media segments are fed.  Per-sample arrays
         *    grow as `parseSegment()` is called and stale entries
         *    disappear when `invalidateSegment()` runs.  Per-segment
         *    indexing (`SampleInformation::segmentId`, `earliestTimestamp[TS]`,
         *    `sampleFlags`) only carries useful data for samples that
         *    came from a parsed media segment.
         *
         *  @pre initialize() has been called successfully.
         *  @param [out] fileinfo FileInformation struct that hold file information.
         *  @return ErrorCode: OK or UNINITIALIZED */
        virtual ErrorCode getFileInformation(FileInformation& fileinfo) const = 0;

        /** Get track information.
         *  These properties can be used to further initialize the presentation of the data in the track.
         *  Properties also give hints about the way and means to request data from the track.
         *
         *  Track-handler coverage. The reader recognises exactly four
         *  handler types ('pict', 'auxv', 'vide', 'soun'). Tracks with
         *  any other handler (subtitle, hint, timed metadata, ...) are
         *  silently dropped at parse time and will NOT appear in the
         *  returned array. Recognised sample entries are limited to
         *  hvc1/hev1/avc1/avc3/vvc1/vvs1 (video) and mp4a (audio); other
         *  codecs surface as a track with no usable sample data.
         *
         *  Fragmented MP4 (moof/mvex), edit lists, alternate groups,
         *  track references, and track groups are all read; DRM
         *  (sinf/tenc/senc/pssh) is parsed only as opaque blobs and
         *  data access on a protected item returns PROTECTED_ITEM.
         *
         *  See the MP4 support documentation for the complete capability
         *  matrix and known limitations.
         *
         *  @pre initialize() has been called successfully.
         *  @param [out] trackInfos Array of TrackInformation struct that hold track information.
         *  @return ErrorCode: OK or UNINITIALIZED */
        virtual ErrorCode getTrackInformations(Array<TrackInformation>& trackInfos) const = 0;

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
         *  @param [in]  referenceType Reference type (e.g. 'thmb', 'cdsc', 'auxl', 'dimg', 'base').
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
         *  @param [in]  referenceType Reference type (e.g. 'thmb', 'cdsc', 'auxl', 'dimg', 'base').
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

        /**
         * @anchor probe_protocol
         * @par Probe-and-fetch convention (applies to every getItem* / getItemDataWithDecoderParameters /
         *      getItemProtectionScheme overload below)
         *
         * The byte-fetch methods do not allocate. They use a two-call "size probe" protocol:
         *
         *   1. **Probe.** Call with `memoryBuffer = nullptr` and `memoryBufferSize = 0`.
         *      The library writes the *exact* number of bytes it will need into `memoryBufferSize`
         *      and returns @ref ErrorCode::BUFFER_SIZE_TOO_SMALL.
         *   2. **Allocate.** Allocate `memoryBufferSize` bytes with the allocator of your choice.
         *   3. **Fetch.** Call again with the allocated buffer and the size returned in step 1.
         *      On success the library writes the bytes and **narrows** `memoryBufferSize` to the
         *      number of bytes actually written (always `<=` the value passed in).
         *
         * The required size is **not** the same as `ItemInformation::size`. It depends on:
         *   * Whether bytestream-header insertion is requested (`bytestreamHeaders = true`).
         *   * The codec's payload framing -- e.g. compact-VVC items unpack to N+6 bytes because
         *     the library reinserts the `vvc1` NAL header that was stripped at write time;
         *     multi-layer items rewrite NAL length fields; decoder parameter sets are inlined by
         *     `getItemDataWithDecoderParameters`. **The library is the only place that knows
         *     the unpacked size -- there is no precompute formula the embedder can use.**
         *
         * The two-call protocol exists because the library cannot know your allocator (you may
         * be writing into a JNI direct buffer, a wasm heap slab, an arena, etc.). For C++
         * callers who just want a vector of bytes, prefer the non-virtual convenience wrappers
         * declared at the bottom of this header (`HEIF::readItemData`, `HEIF::readItemDataWithDecoderParameters`,
         * `HEIF::readItemProtectionScheme`); they perform the probe + allocation + fetch and
         * return an ABI-stable `Array<uint8_t>`.
         */

        /** Get item data.
         *  Item data does not contain initialization or configuration data (i.e. decoder configuration records).
         *  By default nal-length values of 'hvc1'/'avc1' type encoded image data is substituted with bytestream
         * headers. For protected items pure data is returned always. Information how to handle such data is available
         * from getItemProtectionScheme() which returns related 'sinf' box as whole. Note that getItemData() is the only
         * reader API method which can be used for requesting data of such items.
         *  @param [in]      imageId           Item id of the image.
         *  @param [in,out]  memoryBuffer      Memory buffer where data is to be written to. Pass `nullptr` to probe.
         *  @param [in,out]  memoryBufferSize  Memory buffer size in bytes. **Probe-and-fetch:** see @ref probe_protocol
         *                                     above; pass 0 to probe, then the value returned by the probe to fetch.
         *                                     On `OK`, narrowed to bytes actually written.
         *  @param [in]      bytestreamHeaders Optional - by default true. Whether to substitute H.264/H.265 nal-lenght
         * values with bytestream header (0001).
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID, FILE_READ_ERROR, BUFFER_SIZE_TOO_SMALL */
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
         *  @param [in,out] memoryBuffer       Memory buffer where data is to be written to. Pass `nullptr` to probe.
         *  @param [in,out] memoryBufferSize   Memory buffer size in bytes. **Probe-and-fetch:** see @ref probe_protocol
         *                                     above. On `OK`, narrowed to bytes actually written.
         *  @param [in]  bytestreamHeaders     Optional - by default true. Whether to substitute H.264/H.265 nal-lenght
         * values with bytestream header (0001).
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_ITEM_ID, BUFFER_SIZE_TOO_SMALL */
        virtual ErrorCode getItemData(const SequenceId& sequenceId,
                                      const SequenceImageId& imageId,
                                      uint8_t* memoryBuffer,
                                      uint64_t& memoryBufferSize,
                                      bool bytestreamHeaders = true) = 0;

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

        /** Get tone-map / gain-map metadata for a 'tmap' derived item.
         *  @param [in]  tmapItemId   Item id of the 'tmap' derived item.
         *  @param [out] tonemap      Filled with imageIds (from dimg refs)
         *                            and gain_map_metadata (parsed from
         *                            the tmap item payload).
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK,
         *                    UNINITIALIZED,
         *                    INVALID_ITEM_ID            (item does not exist),
         *                    NOT_APPLICABLE             (item exists but is not 'tmap'),
         *                    UNSUPPORTED_GAINMAP_VERSION (minimumVersion != 0). */
        virtual ErrorCode getItem(const ImageId& tmapItemId, Tonemap& tonemap) const = 0;

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

        /** Get property Image scaling ('iscl')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] iscl   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, Scale& iscl) const = 0;

        /** Get item property Relative Location ('rloc')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] rloc   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, RelativeLocation& rloc) const = 0;

        /** Get item property Layer Selector ('lsel')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] lsel   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, LayerSelector& lsel) const = 0;

        /** Get item property Target Output Layer Set ('tols')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] tols   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, TargetOls& tols) const = 0;

        /** Get item property Operating Points Information ('oinf')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] oinf   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, OperatingPointsInfo& oinf) const = 0;

        /** Get item property Sub-sample information ('subs')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] subs   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, SubSampleInfo& subs) const = 0;

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

        /** Get property Required reference types ('rref')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] rref   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, RequiredReferenceTypes& rref) const = 0;

        /** Get property User description property ('udes')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] udes   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, UserDescription& udes) const = 0;

        /** Get property Accessibility text property ('altt')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] altt   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, AccessibilityText& altt) const = 0;

        /** Get property Creation time information property ('crtt')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] crtt   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, CreationTimeInformation& crtt) const = 0;

        /** Get property Modification time information property ('mdft')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] mdft   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX */
        virtual ErrorCode getProperty(const PropertyId& index, ModificationTimeInformation& mdft) const = 0;

        /**
         * @brief Get Image Spatial Extents Property ('ispe')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] ispe   Data of the property.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX
         */
        virtual ErrorCode getProperty(const PropertyId& index, SpatialExtents& ispe) const = 0;

        /**
         * @brief Get Content Light Level Information Property ('clli')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] clli   Data of the property.
         *  @pre initialize() has been called successfully. Property exists.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX
         */
        virtual ErrorCode getProperty(const PropertyId& index, ContentLightLevel& clli) const = 0;

        /**
         * @brief Get Mastering Display Color Volume Property ('mdcv')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] mdcv   Data of the property.
         *  @pre initialize() has been called successfully. Property exists.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX
         */
        virtual ErrorCode getProperty(const PropertyId& index, MasteringDisplayColorVolume& mdcv) const = 0;

        /**
         * @brief Get Content Color Volume Property ('cclv')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] cclv   Data of the property.
         *  @pre initialize() has been called successfully. Property exists.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX
         */
        virtual ErrorCode getProperty(const PropertyId& index, ContentColorVolume& cclv) const = 0;

        /**
         * @brief Get Ambient Viewing Environment Property ('amve')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] amve   Data of the property.
         *  @pre initialize() has been called successfully.  Property exists.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX
         */
        virtual ErrorCode getProperty(const PropertyId& index, AmbientViewingEnvironment& amve) const = 0;

        /**
         * @brief Get Reference Viewing Environment Property ('reve')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] reve   Data of the property.
         *  @pre initialize() has been called successfully. Property exists.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX
         */
        virtual ErrorCode getProperty(const PropertyId& index, ReferenceViewingEnvironment& reve) const = 0;

        /**
         * @brief Get Nominal Diffuse White Point Property ('ndwt')
         *  @param [in]  index  Id of the property. @see getItemProperties()
         *  @param [out] ndwt   Data of the property.
         *  @pre initialize() has been called successfully. Property exists.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_PROPERTY_INDEX
         */
        virtual ErrorCode getProperty(const PropertyId& index, NominalDiffuseWhite& ndwt) const = 0;

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

        /** Get properties of an entity group.
         *  @param [in]  groupId       Group ID which properties to get.
         *  @param [out] propertyTypes Associated properties of the entity groups.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_GROUP_ID */
        virtual ErrorCode getItemProperties(const GroupId& groupId, Array<ItemPropertyInfo>& propertyTypes) const = 0;

        /** Get data of an encoded image item.
         *  This method shall not be used if the item is not of 'hvc1', 'avc1' or 'master' type. @see getItemData()
         *  @param [in] imageId               Item id.
         *  @param [in,out] memoryBuffer      Memory buffer where data is to be written to. Pass `nullptr` to probe.
         *  @param [in,out] memoryBufferSize  Memory buffer size in bytes. **Probe-and-fetch:** see @ref probe_protocol;
         *                                    pass 0 to probe, then the returned value to fetch. On `OK`, narrowed to
         *                                    bytes actually written. The unpacked size includes inlined decoder
         *                                    parameter sets and is **larger** than `ItemInformation::size`.
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
         *  @param [in,out] memoryBuffer      Memory buffer where data is to be written to. Pass `nullptr` to probe.
         *  @param [in,out] memoryBufferSize  Memory buffer size in bytes. **Probe-and-fetch:** see @ref probe_protocol.
         *                                    On `OK`, narrowed to bytes actually written.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_SEQUENCE_ID, INVALID_SEQUENCE_IMAGE_ID, UNSUPPORTED_CODE_TYPE,
         *                     BUFFER_SIZE_TOO_SMALL */
        virtual ErrorCode getItemDataWithDecoderParameters(const SequenceId& sequenceId,
                                                           const SequenceImageId& imageId,
                                                           uint8_t* memoryBuffer,
                                                           uint64_t& memoryBufferSize) = 0;

        /** Get Protection Scheme Information Box for a protected item.
         *  @param [in] imageId               Item id.
         *  @param [in,out] memoryBuffer      Memory buffer where 'sinf' data is to be written to. Pass `nullptr` to
         *                                    probe.
         *  @param [in,out] memoryBufferSize  Memory buffer size in bytes. **Probe-and-fetch:** see @ref probe_protocol.
         *                                    On `OK`, narrowed to bytes actually written.
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

        /** Retrieve decoding dependencies for given image id, in decoding order.
         *  Information here comes from "pred" item references referenced from itemId.
         *  @param [in]  imageId       Identifier of an image.
         *  @param [out] dependencies  Array including direct and indirect decoding dependencies.
         *                             Parameter imageId itself is not included in the array.
         *                             An empty array indicates imageId in independently decodable.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_IMAGE_ID */
        virtual ErrorCode getDecodeDependencies(const ImageId& imageId, Array<ImageId>& dependencies) const = 0;

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

        /** Get decoder configuration record parameter sets covering ALL layers
         *  of a multi-layer VVC image item.
         *
         *  For VVC items whose vvcC has multi_layer_flag = 1 this returns the
         *  full per-layer SPS/PPS bundle (plus the single CVS-global VPS and
         *  any APSs) needed to decode the multi-layer access unit assembled
         *  from all sibling items sharing the same vvcC. The returned
         *  decoderConfigId is the base configuration index (i.e. the iprp
         *  property index, with no per-layer encoding in the upper bits) so
         *  that all sibling items in the multi-layer stream report the same
         *  id and can be grouped on it.
         *
         *  For non-VVC items, single-layer VVC items, or items whose vvcC has
         *  multi_layer_flag = 0, this method behaves exactly like the
         *  layer-specific getDecoderParameterSets() above.
         *
         *  @param [in]  imageId       Identifier of an image item.
         *  @param [out] decoderInfos  DecoderConfiguration with the all-layers
         *                             parameter set bundle.
         *  @pre initialize() has been called successfully.
         *  @return ErrorCode: OK, UNINITIALIZED, INVALID_ITEM_ID */
        virtual ErrorCode getDecoderParameterSetsForAllLayers(const ImageId& imageId,
                                                              DecoderConfiguration& decoderInfos) const = 0;

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

    public:  // segment parsing methods (streaming)
        /** Parse Initialization Segment
         *
         *  @param [in]  streamInterface   StreamInterface*  Interface to read initialization segment from.
         *  @return ErrorCode: OK */
        virtual ErrorCode parseInitializationSegment(StreamInterface* streamInterface) = 0;

        /** Parse Segment
         *
         *  Note that the segment id must be globally unique per an instance of
         *  reader and must be different from any init segment id as well.
         *
         *  @pre Initialization Segment has been parsed before feeding in segment.
         *  @param [in]  streamInterface   StreamInterface*  Interface to read segment from.
         *  @param [in]  segmentId       uint32_t Segment Id of the segment being fed to method through segmentData
         *                                        pointer
         *  @param [in]  earliestPTSinTS uint64_t Optional - in case of feeding partial segment without 'sidx' box this
         *                                        can be used to give earliest presentation time in timescale for
         * samples.
         *  @return ErrorCode: OK, FILE_READ_ERROR  */
        virtual ErrorCode parseSegment(StreamInterface* streamInterface,
                                       SegmentId segmentId,
                                       uint64_t earliestPTSinTS = UINT64_MAX) = 0;

        /** Invalidate Segment
         *  Invalidates the data buffer pointer to given media segment id - data from this segment can no longer be
         * read.
         *
         *  Note! Must be called for all fed media segments if client is seeking.
         *
         *  @pre Segment with segmentId has been parsed with reader using parseSegment.
         *  @param [in]  segmentId     uint32_t Segment Id of the Initialize Segment
         *  @return ErrorCode: OK, INVALID_SEGMENT */
        virtual ErrorCode invalidateSegment(SegmentId segmentId) = 0;

        /** Get Segment Index
         *  Provides segment index contents for use with DASH ISO Base Media File Format On-Demand profile.
         *
         *  Returned array will contain segmentId for each of the byte ranges, that must be used to feed
         *  those byte ranges through parseInitializationSegment() to parse the file.
         *
         *  @pre Byte range which contains ISO BMFF On-Demand profile initialization byte range has been parsed
         *       with reader using parseInitializeSegment.
         *  @param [out] segmentIndex    Array of SegmentInformation struct that hold segment index information.
         *  @return ErrorCode: OK, INVALID_SEGMENT */
        virtual ErrorCode getSegmentIndex(Array<SegmentInformation>& segmentIndex) = 0;

        /** Parse Segment Index ('sidx' box) from given stream.
         *
         *  @param [in]  streamInterface   StreamInterface*  Interface to read segment from.
         *  @param [out] segmentIndex    Array of SegmentInformation struct that hold segment index information.
         *  @return ErrorCode: OK, INVALID_SEGMENT */
        virtual ErrorCode parseSegmentIndex(StreamInterface* streamInterface,
                                            Array<SegmentInformation>& segmentIndex) = 0;

    protected:
        virtual ~Reader() = default;
    };

    // Inline convenience wrappers -- probe + allocate + fetch in one call.
    //
    // These hide the two-call probe-and-fetch protocol (see @ref probe_protocol
    // in the Reader class above) for C++ callers who just want a vector of
    // bytes. They are header-only, non-virtual, and do not change the ABI:
    //
    //   * `Array<uint8_t>` is the library's ABI-stable container; its ctor
    //     and dtor are out-of-line in libheif, so the allocation crosses the
    //     DLL boundary correctly (same allocator on both sides).
    //   * They are defined inline here so they compile into the caller --
    //     no vtable entry, no symbol export, no version bump.
    //   * On error the returned Array is empty (`size == 0`) and `*outError`
    //     (if non-null) carries the ErrorCode. Pass `outError = nullptr` to
    //     ignore failures (suitable only for examples / scratch code).
    //
    // For the raw probe-and-fetch protocol -- needed when allocating into a
    // JNI direct buffer, a wasm heap slab, or any non-C++ allocator -- call
    // the virtual `Reader::getItem*` overloads directly.

    /** Read raw bytes of an image item in one call. Substitutes bytestream headers by
     *  default (toggle with `bytestreamHeaders`). Returns an empty Array on failure;
     *  set `outError` to recover the ErrorCode.
     *  @see Reader::getItemData @see @ref probe_protocol */
    inline Array<uint8_t> readItemData(const Reader* reader,
                                       const ImageId& imageId,
                                       bool bytestreamHeaders = true,
                                       ErrorCode* outError    = nullptr)
    {
        uint64_t needed = 0;
        ErrorCode probe = reader->getItemData(imageId, nullptr, needed, bytestreamHeaders);
        if (probe != ErrorCode::OK && probe != ErrorCode::BUFFER_SIZE_TOO_SMALL)
        {
            if (outError) *outError = probe;
            return {};
        }
        Array<uint8_t> buf(needed);
        if (needed > 0)
        {
            ErrorCode fetch = reader->getItemData(imageId, buf.elements, needed, bytestreamHeaders);
            if (fetch != ErrorCode::OK)
            {
                if (outError) *outError = fetch;
                return {};
            }
        }
        if (outError) *outError = ErrorCode::OK;
        return buf;  // size already narrowed to bytes-written by getItemData on OK
    }

    /** Read raw bytes of one sample in an image-sequence track. @see Reader::getItemData
     *  @see @ref probe_protocol */
    inline Array<uint8_t> readItemData(Reader* reader,
                                       const SequenceId& sequenceId,
                                       const SequenceImageId& sampleId,
                                       bool bytestreamHeaders = true,
                                       ErrorCode* outError    = nullptr)
    {
        uint64_t needed = 0;
        ErrorCode probe = reader->getItemData(sequenceId, sampleId, nullptr, needed, bytestreamHeaders);
        if (probe != ErrorCode::OK && probe != ErrorCode::BUFFER_SIZE_TOO_SMALL)
        {
            if (outError) *outError = probe;
            return {};
        }
        Array<uint8_t> buf(needed);
        if (needed > 0)
        {
            ErrorCode fetch = reader->getItemData(sequenceId, sampleId, buf.elements, needed, bytestreamHeaders);
            if (fetch != ErrorCode::OK)
            {
                if (outError) *outError = fetch;
                return {};
            }
        }
        if (outError) *outError = ErrorCode::OK;
        return buf;
    }

    /** Read encoded bytes of an image item with decoder parameter sets inlined,
     *  ready to feed straight to a decoder. @see Reader::getItemDataWithDecoderParameters
     *  @see @ref probe_protocol */
    inline Array<uint8_t> readItemDataWithDecoderParameters(const Reader* reader,
                                                            const ImageId& imageId,
                                                            ErrorCode* outError = nullptr)
    {
        uint64_t needed = 0;
        ErrorCode probe = reader->getItemDataWithDecoderParameters(imageId, nullptr, needed);
        if (probe != ErrorCode::OK && probe != ErrorCode::BUFFER_SIZE_TOO_SMALL)
        {
            if (outError) *outError = probe;
            return {};
        }
        Array<uint8_t> buf(needed);
        if (needed > 0)
        {
            ErrorCode fetch = reader->getItemDataWithDecoderParameters(imageId, buf.elements, needed);
            if (fetch != ErrorCode::OK)
            {
                if (outError) *outError = fetch;
                return {};
            }
        }
        if (outError) *outError = ErrorCode::OK;
        return buf;
    }

    /** Read encoded bytes of one sample in an image-sequence track with decoder
     *  parameter sets inlined. @see Reader::getItemDataWithDecoderParameters
     *  @see @ref probe_protocol */
    inline Array<uint8_t> readItemDataWithDecoderParameters(Reader* reader,
                                                            const SequenceId& sequenceId,
                                                            const SequenceImageId& sampleId,
                                                            ErrorCode* outError = nullptr)
    {
        uint64_t needed = 0;
        ErrorCode probe = reader->getItemDataWithDecoderParameters(sequenceId, sampleId, nullptr, needed);
        if (probe != ErrorCode::OK && probe != ErrorCode::BUFFER_SIZE_TOO_SMALL)
        {
            if (outError) *outError = probe;
            return {};
        }
        Array<uint8_t> buf(needed);
        if (needed > 0)
        {
            ErrorCode fetch =
                reader->getItemDataWithDecoderParameters(sequenceId, sampleId, buf.elements, needed);
            if (fetch != ErrorCode::OK)
            {
                if (outError) *outError = fetch;
                return {};
            }
        }
        if (outError) *outError = ErrorCode::OK;
        return buf;
    }

    /** Read the Protection Scheme Information Box ('sinf') of a protected item.
     *  @see Reader::getItemProtectionScheme @see @ref probe_protocol */
    inline Array<uint8_t> readItemProtectionScheme(const Reader* reader,
                                                   const ImageId& imageId,
                                                   ErrorCode* outError = nullptr)
    {
        uint64_t needed = 0;
        ErrorCode probe = reader->getItemProtectionScheme(imageId, nullptr, needed);
        if (probe != ErrorCode::OK && probe != ErrorCode::BUFFER_SIZE_TOO_SMALL)
        {
            if (outError) *outError = probe;
            return {};
        }
        Array<uint8_t> buf(needed);
        if (needed > 0)
        {
            ErrorCode fetch = reader->getItemProtectionScheme(imageId, buf.elements, needed);
            if (fetch != ErrorCode::OK)
            {
                if (outError) *outError = fetch;
                return {};
            }
        }
        if (outError) *outError = ErrorCode::OK;
        return buf;
    }

}  // namespace HEIF

#endif /* HEIFREADER_H */
