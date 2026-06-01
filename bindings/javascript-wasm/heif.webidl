/*
 * heif.webidl -- WebIDL description of the Nokia HEIF JavaScript binding.
 *
 * This file formally specifies the public API surface exposed by heif.js
 * (the cwrap-based JavaScript binding of the HEIF C facade).  It serves
 * three purposes:
 *
 *   1. Machine-readable contract that tooling (linters, doc generators,
 *      codegen pipelines) can consume.
 *   2. Reference for binding authors porting to other wasm glue
 *      strategies (Embind, wasm-bindgen, webidl_binder.py).
 *   3. Spec-grade documentation of the JS-visible types and methods.
 *
 * The canonical TypeScript declarations live in heif.d.ts; this file
 * mirrors that surface in W3C WebIDL syntax (https://webidl.spec.whatwg.org/).
 *
 * Caveats:
 *   - The binding is hand-rolled via Emscripten cwrap, not generated from
 *     this IDL.  If heif.js and this file diverge, heif.js wins.
 *   - WebIDL has no notion of Emscripten module lifecycle; the static
 *     HEIF.load() factory is expressed as a namespace operation.
 *   - bigint-typed properties (crtt, mdft) use the WebIDL "bigint" type
 *     introduced in the 2024 spec update.
 */


/*  Load options                                                      */

dictionary HEIFLoadOptions {
    any      factory;
    object   moduleArgs;
};


/*  Top-level namespace                                               */

[Exposed=Window]
namespace HEIF {
    Promise<HEIF> load(optional HEIFLoadOptions opts = {});
};

[Exposed=Window]
interface HEIF {
    /* Version / identity */
    DOMString  runtimeVersion();
    VersionTriple runtimeVersionTriple();
    unsigned long runtimeVersionNumber();
    AbiHandshake  abiHandshake();

    /* File opening */
    HEIFReader open(Uint8Array bytes);

    /* Static helpers (no reader needed) */
    DOMString  errorCodeName(long code);
    DOMString  errorOriginName(long origin);
    boolean    hasLastError();
    DOMString  lastErrorCause();
    DOMString  lastErrorFile();
};


/*  Reader (one open file / segment stream)                           */

[Exposed=Window]
interface HEIFReader {
    /* File-level identity (readonly attributes set at open time) */
    readonly attribute DOMString        majorBrand;
    readonly attribute unsigned long    minorVersion;
    readonly attribute sequence<DOMString> compatibleBrands;
    readonly attribute unsigned long    primaryItemId;

    undefined close();

    /* File / metabox features */
    unsigned long      fileFeatures();
    unsigned long      metaboxFeatures();
    Int32Array         matrix();
    sequence<sequence<DOMString>> compatibleBrandCombos();

    /* Item enumeration */
    Uint32Array        masterImages();
    Uint32Array        itemsByType(DOMString fourCC);
    Uint32Array        referencedFrom(unsigned long itemId, DOMString refType);
    Uint32Array        referencedTo(unsigned long itemId, DOMString refType);

    /* Per-item accessors */
    DOMString          itemType(unsigned long itemId);
    unsigned long      itemFeatures(unsigned long itemId);
    boolean            itemIsHidden(unsigned long itemId);
    unsigned long      itemWidth(unsigned long itemId);
    unsigned long      itemHeight(unsigned long itemId);
    ItemSize           itemSize(unsigned long itemId);

    /* Bytestream extraction */
    Uint8Array         itemBitstream(unsigned long itemId);
    Uint8Array         itemRawData(unsigned long itemId);
    DOMString          decoderCodeType(unsigned long itemId);
    Uint32Array        decodeDependencies(unsigned long itemId);
    Uint8Array         itemProtectionScheme(unsigned long itemId);

    /* Decoder parameter sets */
    sequence<ParamSetEntry> itemDecoderParamSets(unsigned long itemId);
    sequence<ParamSetEntry> itemDecoderParamSetsAllLayers(unsigned long itemId);

    /* Item properties */
    sequence<ItemProperty>  itemProperties(unsigned long itemId);
    ResolvedProperties      itemPropertiesResolved(unsigned long itemId);

    IspeResult         propertyIspe(unsigned long propertyId);
    unsigned long      propertyIrot(unsigned long propertyId);
    ImirResult         propertyImir(unsigned long propertyId);
    ClapProperty       propertyClap(unsigned long propertyId);
    IsclProperty       propertyIscl(unsigned long propertyId);
    LselResult         propertyLsel(unsigned long propertyId);
    TolsResult         propertyTols(unsigned long propertyId);
    RlocResult         propertyRloc(unsigned long propertyId);
    OinfProperty       propertyOinf(unsigned long propertyId);
    sequence<unsigned short> propertyPixi(unsigned long propertyId);
    sequence<PixiChannel>    propertyPixiChannels(unsigned long propertyId);
    ColrProperty       propertyColr(unsigned long propertyId);
    DOMString          propertyAuxcType(unsigned long propertyId);
    Uint8Array         propertyAuxcSubtype(unsigned long propertyId);
    PaspResult         propertyPasp(unsigned long propertyId);
    CliProperty        propertyClli(unsigned long propertyId);
    MdcvProperty       propertyMdcv(unsigned long propertyId);
    CclvProperty       propertyCclv(unsigned long propertyId);
    AmveProperty       propertyAmve(unsigned long propertyId);
    ReveProperty       propertyReve(unsigned long propertyId);
    NdwtResult         propertyNdwt(unsigned long propertyId);
    bigint             propertyCrtt(unsigned long propertyId);
    bigint             propertyMdft(unsigned long propertyId);
    UdesProperty       propertyUdes(unsigned long propertyId);
    AlttResult         propertyAltt(unsigned long propertyId);
    sequence<DOMString> propertyRref(unsigned long propertyId);
    RawProperty        propertyRaw(unsigned long propertyId);
    sequence<SubsEntry> propertySubs(unsigned long propertyId);

    /* Entity-group properties */
    sequence<ItemProperty> groupProperties(unsigned long groupId);

    /* Composition */
    GridInfo               grid(unsigned long itemId);
    OverlayInfo            overlay(unsigned long itemId);
    TonemapGainmapInfo     tonemapGainmap(unsigned long itemId);
    sequence<TonemapImage> tonemapImages(unsigned long itemId);

    /* Entity groups */
    sequence<EntityGroup>  entityGroups();

    /* Tracks (image sequences) */
    Uint32Array            tracks();
    TrackInfo              trackInfo(unsigned long trackId);
    unsigned long          trackDisplayWidth(unsigned long trackId);
    unsigned long          trackDisplayHeight(unsigned long trackId);
    unsigned long          trackSampleCount(unsigned long trackId);
    double                 trackPlaybackDurationSecs(unsigned long trackId);
    sequence<TrackSample>  trackSamples(unsigned long trackId);
    sequence<TrackTimestamp> trackTimestamps(unsigned long trackId);
    sequence<TrackReference> trackReferences(unsigned long trackId);
    Uint8Array             trackSampleBitstream(unsigned long trackId, unsigned long sampleId);
    Uint8Array             trackSampleRawData(unsigned long trackId, unsigned long sampleId);
    EditListMeta           trackEditListMeta(unsigned long trackId);
    sequence<EditUnit>     trackEditUnits(unsigned long trackId);
    Uint32Array            trackAlternates(unsigned long trackId);
    sequence<TrackGroup>   trackGroups(unsigned long trackId);
    sequence<DOMString>    trackCompatibleBrands(unsigned long trackId);
    sequence<ItemTimestamp> itemTimestamps(unsigned long trackId, unsigned long sampleId);
    sequence<SegmentEntry> segments();
    unsigned long          trackSampleWidth(unsigned long trackId, unsigned long sampleId);
    unsigned long          trackSampleHeight(unsigned long trackId, unsigned long sampleId);
    ItemSize               trackSampleSize(unsigned long trackId, unsigned long sampleId);
    Int32Array             trackMatrix(unsigned long trackId);
    Uint32Array            trackMasterImages(unsigned long trackId);
    Uint32Array            trackSamplesByType(unsigned long trackId, unsigned long sampleType);
    ClapProperty           trackSampleClap(unsigned long trackId, unsigned long sampleDescIdx);
    TrackSampleAuxi        trackSampleAuxi(unsigned long trackId, unsigned long sampleDescIdx);
    DOMString              trackSampleDecoderCodeType(unsigned long trackId, unsigned long sampleId);
    sequence<ParamSetEntry> trackSampleDecoderParamSets(unsigned long trackId, unsigned long sampleId);
    sequence<DecodingOrderEntry> trackDecodingOrder(unsigned long trackId);
    Uint32Array            trackSampleDecodeDependencies(unsigned long trackId, unsigned long sampleId);

    /* Track sample sub-structures */
    DOMString              trackSampleItemType(unsigned long trackId, unsigned long sampleId);
    sequence<SampleEqiv>   trackSampleEqiv(unsigned long trackId);
    sequence<SampleStmi>   trackSampleStmi(unsigned long trackId);
    sequence<SampleRefs>   trackSampleRefs(unsigned long trackId);
    sequence<SampleTrif>   trackSampleTrif(unsigned long trackId);
    sequence<SampleSpor>   trackSampleSpor(unsigned long trackId);
    sequence<SampleSulm>   trackSampleSulm(unsigned long trackId);
    sequence<SampleMinp>   trackSampleMinp(unsigned long trackId);

    /* DASH / segmented parsing */
    undefined              openFile(DOMString path);
    undefined              parseInitSegment(Uint8Array bytes);
    undefined              parseSegment(Uint8Array bytes, unsigned long segmentId, unsigned long long earliestPts);
    undefined              invalidateSegment(unsigned long segmentId);
    sequence<SegmentInfo>  parseSegmentIndex(Uint8Array bytes);
};


/*  Error                                                             */

[Exposed=Window]
interface HEIFError {
    readonly attribute long      code;
    readonly attribute DOMString codeName;
    readonly attribute DOMString origin;
    readonly attribute long      line;
    readonly attribute DOMString detail;
};


/*  Sample flags (pure JS bit-extractors)                             */

[Exposed=Window]
namespace sampleFlags {
    unsigned long isLeading(unsigned long flags);
    unsigned long sampleDependsOn(unsigned long flags);
    unsigned long sampleIsDependedOn(unsigned long flags);
    unsigned long sampleHasRedundancy(unsigned long flags);
    unsigned long samplePaddingValue(unsigned long flags);
    unsigned long sampleIsNonSyncSample(unsigned long flags);
    unsigned long sampleDegradationPriority(unsigned long flags);
};


/*  Supporting dictionaries                                           */

dictionary VersionTriple {
    unsigned long major;
    unsigned long minor;
    unsigned long patch;
};

dictionary AbiHandshake {
    unsigned long major;
    unsigned long minor;
    unsigned long patch;
    unsigned long flags;
};

dictionary ItemSize {
    unsigned long width;
    unsigned long height;
};

dictionary IspeResult {
    unsigned long width;
    unsigned long height;
};

dictionary ImirResult {
    DOMString axis;    /* "horizontal" | "vertical" */
};

dictionary LselResult {
    unsigned long layerId;
};

dictionary TolsResult {
    unsigned long targetOlsIndex;
};

dictionary RlocResult {
    unsigned long horizontalOffset;
    unsigned long verticalOffset;
};

dictionary PaspResult {
    unsigned long relativeWidth;
    unsigned long relativeHeight;
};

dictionary NdwtResult {
    unsigned long diffuseWhiteLuminance;
};

dictionary AlttResult {
    DOMString text;
    DOMString lang;
};

dictionary ItemProperty {
    unsigned long id;
    DOMString     type;
    boolean       essential;
};

dictionary ClapProperty {
    unsigned long cleanApertureWidthN;
    unsigned long cleanApertureWidthD;
    unsigned long cleanApertureHeightN;
    unsigned long cleanApertureHeightD;
    long          horizOffN;
    unsigned long horizOffD;
    long          vertOffN;
    unsigned long vertOffD;
};

dictionary IsclProperty {
    unsigned long targetWidthN;
    unsigned long targetWidthD;
    unsigned long targetHeightN;
    unsigned long targetHeightD;
};

dictionary CliProperty {
    unsigned long maxContentLightLevel;
    unsigned long maxPicAverageLightLevel;
};

dictionary MdcvProperty {
    sequence<unsigned long> displayPrimariesX;   /* 3 elements */
    sequence<unsigned long> displayPrimariesY;   /* 3 elements */
    unsigned long whitePointX;
    unsigned long whitePointY;
    unsigned long maxDisplayMasteringLuminance;
    unsigned long minDisplayMasteringLuminance;
};

dictionary CclvProperty {
    unsigned long primariesPresent;
    unsigned long minLuminancePresent;
    unsigned long maxLuminancePresent;
    unsigned long avgLuminancePresent;
    sequence<unsigned long> primariesX;          /* 3 elements */
    sequence<unsigned long> primariesY;          /* 3 elements */
    unsigned long minLuminanceValue;
    unsigned long maxLuminanceValue;
    unsigned long avgLuminanceValue;
};

dictionary AmveProperty {
    unsigned long ambientIlluminance;
    unsigned long ambientLightX;
    unsigned long ambientLightY;
};

dictionary ReveProperty {
    unsigned long surroundLuminance;
    unsigned long surroundLightX;
    unsigned long surroundLightY;
    unsigned long peripheryLuminance;
    unsigned long peripheryLightX;
    unsigned long peripheryLightY;
};

dictionary UdesProperty {
    DOMString lang;
    DOMString name;
    DOMString description;
    DOMString tags;
};

dictionary SubsEntry {
    unsigned long subsampleSize;
    unsigned long subsamplePriority;
    boolean       discardable;
};

dictionary PixiChannel {
    unsigned long channelIdc;
    unsigned long componentFormat;
    boolean       subsamplingFlag;
    boolean       channelLabelFlag;
    unsigned long subsamplingType;
    unsigned long subsamplingLocation;
    DOMString?    label;
};

/*
 * ColrProperty is a union type in the TS declarations.  WebIDL does not
 * support tagged unions directly; we use a single dictionary whose
 * members are conditionally present depending on "type".
 *
 *   type == "nclx"          -> colourPrimaries, transferCharacteristics,
 *                              matrixCoefficients, fullRangeFlag populated.
 *   type == "rICC" | "prof" -> iccProfile populated.
 *   other                   -> only type is populated.
 */
dictionary ColrProperty {
    DOMString     type;
    unsigned long colourPrimaries;
    unsigned long transferCharacteristics;
    unsigned long matrixCoefficients;
    boolean       fullRangeFlag;
    Uint8Array?   iccProfile;
};

dictionary ParamSetEntry {
    unsigned long type;
    Uint8Array    data;
};

dictionary OinfProperty {
    unsigned long          scalabilityMask;
    sequence<OinfPtl>      ptl;
    sequence<OinfOperatingPoint> operatingPoints;
    sequence<OinfLayer>    layers;
};

dictionary OinfPtl {
    unsigned long ptlIdx;
};

dictionary OinfOperatingPoint {
    unsigned long              opIdx;
    sequence<unsigned long>    layers;
};

dictionary OinfLayer {
    unsigned long              layerIdx;
    sequence<unsigned long>    dependencies;
    sequence<unsigned long>    dimensions;
};

dictionary RawProperty {
    DOMString  type;
    Uint8Array data;
};

dictionary GridInfo {
    unsigned long outputWidth;
    unsigned long outputHeight;
    unsigned long columns;
    unsigned long rows;
    Uint32Array   tileIds;
};

dictionary OverlayInfo {
    unsigned long          outputWidth;
    unsigned long          outputHeight;
    unsigned long          r;
    unsigned long          g;
    unsigned long          b;
    unsigned long          a;
    sequence<OverlayImage> images;
};

dictionary OverlayImage {
    unsigned long itemId;
    long          horizontalOffset;
    long          verticalOffset;
};

dictionary TonemapGainmapInfo {
    unsigned long            baseImageItemId;
    unsigned long            gainmapImageItemId;
    unsigned long            version;
    unsigned long            flags;
    sequence<TonemapChannel> channels;
};

dictionary TonemapChannel {
    unsigned long gainMapMin;
    unsigned long gainMapMax;
    unsigned long gamma;
    unsigned long baseOffset;
    unsigned long alternateOffset;
};

dictionary TonemapImage {
    unsigned long itemId;
    unsigned long role;
};

dictionary EntityGroup {
    unsigned long          groupId;
    DOMString              type;
    sequence<unsigned long> entityIds;
};

dictionary TrackInfo {
    unsigned long              features;
    unsigned long              alternateGroupId;
    sequence<unsigned long>    alternateTrackIds;
};

dictionary TrackSample {
    unsigned long sampleId;
    unsigned long size;
    unsigned long sampleFlags;
    unsigned long segmentId;
    unsigned long sampleType;
    boolean       hasClap;
    boolean       hasAuxi;
    unsigned long codingConstraints;
    DOMString     sampleEntryType;
};

dictionary TrackTimestamp {
    unsigned long sampleId;
    double        timestampMs;
};

dictionary TrackReference {
    DOMString              type;
    sequence<unsigned long> trackIds;
};

dictionary EditListMeta {
    unsigned long flags;
    long          nRepetitions;
    double        durationMs;
};

dictionary EditUnit {
    unsigned long editType;
    double        mediaTimeMs;
    double        durationMs;
};

dictionary TrackGroup {
    DOMString     type;
    unsigned long groupId;
};

dictionary ItemTimestamp {
    double timestampMs;
};

dictionary SegmentEntry {
    unsigned long      segmentId;
    unsigned long      referenceId;
    unsigned long      timescale;
    unsigned long      earliestPresentationTimestamp;
    unsigned long long startDataOffset;
    unsigned long long dataSize;
    unsigned long      durationInTs;
    unsigned long      referenceType;
};

dictionary TrackSampleAuxi {
    DOMString auxType;
    DOMString auxSubType;
};

dictionary DecodingOrderEntry {
    double        timestampMs;
    unsigned long sampleId;
};

dictionary SampleEqiv {
    unsigned long sampleGroupDescriptionIndex;
    long          timeOffset;
    unsigned long timescaleMultiplier;
};

dictionary SampleStmi {
    unsigned long              sampleGroupDescriptionIndex;
    sequence<unsigned long>    metadataItemIds;
};

dictionary SampleRefs {
    unsigned long              sampleGroupDescriptionIndex;
    unsigned long              sampleId;
    sequence<unsigned long>    referenceItemIds;
};

dictionary SampleTrif {
    unsigned long              sampleGroupDescriptionIndex;
    unsigned long              groupId;
    unsigned long              independentIdc;
    unsigned long              horizontalOffset;
    unsigned long              verticalOffset;
    unsigned long              regionWidth;
    unsigned long              regionHeight;
    boolean                    rectRegionFlag;
    boolean                    fullPicture;
    boolean                    filteringDisabled;
    sequence<unsigned long>    dependencies;
};

dictionary SampleSpor {
    unsigned long              sampleGroupDescriptionIndex;
    unsigned long              subpicIdBitPos;
    boolean                    subpicIdInfoFlag;
    unsigned long              subpicdLenMinus1;
    boolean                    startCodeEmulFlag;
    boolean                    ppsSpsSubpicIdFlag;
    unsigned long              ppsId;
    unsigned long              spsId;
    sequence<unsigned long>    subpTrackRefIdxs;
};

dictionary SampleSulm {
    unsigned long              sampleGroupDescriptionIndex;
    DOMString                  groupIdInfo4cc;
    sequence<unsigned long>    groupIds;
};

dictionary SampleMinp {
    unsigned long              sampleGroupDescriptionIndex;
    unsigned long              ppsMixNaluTypesInPicBitPos;
    unsigned long              ppsId;
    sequence<unsigned long>    mixSubpTrackIdx1;
    sequence<unsigned long>    mixSubpTrackIdx2;
};

dictionary SegmentInfo {
    unsigned long      segmentId;
    unsigned long long earliestPtsInTs;
    unsigned long long startDataOffset;
    unsigned long      referenceId;
    unsigned long      timescale;
    unsigned long      durationInTs;
    unsigned long long dataSize;
    unsigned long      referenceType;
};

/*
 * ResolvedProperties -- convenience aggregate returned by
 * HEIFReader.itemPropertiesResolved().  All members except "raw" are
 * optional (present only when the corresponding box exists on the item).
 */
dictionary ResolvedProperties {
    required sequence<ItemProperty> raw;
    IspeResult?    ispe;
    unsigned long? irot;
    ImirResult?    imir;
    sequence<unsigned short>? pixi;
    ColrProperty?  colr;
    DOMString?     auxC;
    ClapProperty?  clap;
    IsclProperty?  iscl;
    LselResult?    lsel;
    TolsResult?    tols;
    RlocResult?    rloc;
    OinfProperty?  oinf;
    CliProperty?   clli;
    MdcvProperty?  mdcv;
    CclvProperty?  cclv;
    AmveProperty?  amve;
    ReveProperty?  reve;
    NdwtResult?    ndwt;
    PaspResult?    pasp;
    bigint?        crtt;
    bigint?        mdft;
    UdesProperty?  udes;
    AlttResult?    altt;
    sequence<DOMString>? rref;
    sequence<SubsEntry>? subs;
    sequence<RawProperty>? unknown;
};
