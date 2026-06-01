/**
 * TypeScript declarations for heif.js -- the JavaScript binding of the
 * Nokia HEIF library C facade.
 *
 * Usage:
 *   const heif = await HEIF.load();
 *   const reader = heif.open(bytes);
 *   console.log(reader.majorBrand, reader.primaryItemId);
 *   reader.close();
 */

export interface HEIFLoadOptions {
    /** Emscripten module factory (defaults to globalThis.HEIFModule). */
    factory?: (...args: any[]) => Promise<any>;
    /** Arguments forwarded to the module factory. */
    moduleArgs?: Record<string, any>;
}

// Top-level namespace

export function load(opts?: HEIFLoadOptions): Promise<HEIF>;

export class HEIF {
    /** Library version as a human-readable string (e.g. "4.0.0"). */
    runtimeVersion(): string;
    /** Library version as {major, minor, patch}. */
    runtimeVersionTriple(): { major: number; minor: number; patch: number };
    /** Library version as a single number (major*10000 + minor*100 + patch). */
    runtimeVersionNumber(): number;
    /** ABI handshake: returns the library's runtime identity. */
    abiHandshake(): { major: number; minor: number; patch: number; flags: number };

    /** Open a HEIF file from in-memory bytes. */
    open(bytes: Uint8Array): HEIFReader;

    /** Static lookup: numeric error code -> name string. */
    errorCodeName(code: number): string;
    /** Static lookup: numeric error origin -> name string. */
    errorOriginName(origin: number): string;
    /** True if the per-thread last-error slot is populated. */
    hasLastError(): boolean;
    /** Chained cause message from the last error (if any). */
    lastErrorCause(): string;
    /** Source file path from the last error (library internal). */
    lastErrorFile(): string;
}

// Reader (one open file)

export class HEIFReader {
    // File-level identity

    /** ISO base media file major brand FourCC (e.g. "heic", "mif1"). */
    readonly majorBrand: string;
    /** Minor version from the file type box. */
    readonly minorVersion: number;
    /** Compatible brands array. */
    readonly compatibleBrands: string[];
    /** Primary item id. */
    readonly primaryItemId: number;

    /** Release native resources. Must be called when done. */
    close(): void;

    // File / metabox features

    /** File-level feature bitmask (HEIF::FileFeatureEnum::Feature). */
    fileFeatures(): number;
    /** Metabox-level feature bitmask (HEIF::MetaBoxFeatureEnum::Feature). */
    metaboxFeatures(): number;
    /** File-level 3x3 transformation matrix (9 int32 values). */
    matrix(): Int32Array;
    /** Compatible brand combinations -- array of FourCC arrays. */
    compatibleBrandCombos(): string[][];

    // Item enumeration

    /** All master image item ids. */
    masterImages(): Uint32Array;
    /** All item ids matching a FourCC item type. */
    itemsByType(fourCC: string): Uint32Array;
    /** Items that itemId references via a given reference type. */
    referencedFrom(itemId: number, refType: string): Uint32Array;
    /** Items that reference itemId via a given reference type. */
    referencedTo(itemId: number, refType: string): Uint32Array;

    // Per-item accessors

    /** FourCC item type (e.g. "hvc1", "grid"). */
    itemType(itemId: number): string;
    /** Item feature bitmask. */
    itemFeatures(itemId: number): number;
    /** True if the IsHiddenImage flag is set. */
    itemIsHidden(itemId: number): boolean;
    /** Coded width. */
    itemWidth(itemId: number): number;
    /** Coded height. */
    itemHeight(itemId: number): number;
    /** {width, height} convenience. */
    itemSize(itemId: number): { width: number; height: number };

    // Bytestream extraction

    /** Decoder-ready bitstream (with decoder parameters). */
    itemBitstream(itemId: number): Uint8Array;
    /** Raw item bytes (on-wire payload). */
    itemRawData(itemId: number): Uint8Array;
    /** Decoder code type FourCC (e.g. "hvc1", "vvc1"). */
    decoderCodeType(itemId: number): string;
    /** Item ids that must be decoded before this item. */
    decodeDependencies(itemId: number): Uint32Array;
    /** Item protection scheme bytes. */
    itemProtectionScheme(itemId: number): Uint8Array;

    // Item decoder parameter sets

    /** Decoder parameter sets (VPS/SPS/PPS) for an item. */
    itemDecoderParamSets(itemId: number): ParamSetEntry[];
    /** Decoder parameter sets across all layers for an item. */
    itemDecoderParamSetsAllLayers(itemId: number): ParamSetEntry[];

    // Item properties

    /** All properties associated with an item. */
    itemProperties(itemId: number): ItemProperty[];
    /** Convenience: all typed properties resolved into one object. */
    itemPropertiesResolved(itemId: number): ResolvedProperties;

    /** ispe -- encoded extents. */
    propertyIspe(propertyId: number): { width: number; height: number };
    /** irot -- rotation in degrees (0/90/180/270). */
    propertyIrot(propertyId: number): number;
    /** imir -- mirror axis. */
    propertyImir(propertyId: number): { axis: 'horizontal' | 'vertical' };
    /** clap -- clean aperture. */
    propertyClap(propertyId: number): ClapProperty;
    /** iscl -- image scaling. */
    propertyIscl(propertyId: number): IsclProperty;
    /** lsel -- layer selector. */
    propertyLsel(propertyId: number): { layerId: number };
    /** tols -- target output layer set. */
    propertyTols(propertyId: number): { targetOlsIndex: number };
    /** rloc -- relative location. */
    propertyRloc(propertyId: number): { horizontalOffset: number; verticalOffset: number };
    /** oinf -- operating-points-info (multi-layer). */
    propertyOinf(propertyId: number): OinfProperty;
    /** pixi -- bits per channel (simple form). */
    propertyPixi(propertyId: number): number[];
    /** pixi -- full channel info (rich v3 form). */
    propertyPixiChannels(propertyId: number): PixiChannel[];
    /** colr -- colour information (NCLX or ICC). */
    propertyColr(propertyId: number): ColrProperty;
    /** auxC -- auxiliary type URN string. */
    propertyAuxcType(propertyId: number): string;
    /** auxC -- raw subtype bytes. */
    propertyAuxcSubtype(propertyId: number): Uint8Array;
    /** pasp -- pixel aspect ratio. */
    propertyPasp(propertyId: number): { relativeWidth: number; relativeHeight: number };
    /** clli -- content light level info. */
    propertyClli(propertyId: number): CliProperty;
    /** mdcv -- mastering display colour volume. */
    propertyMdcv(propertyId: number): MdcvProperty;
    /** cclv -- content colour volume. */
    propertyCclv(propertyId: number): CclvProperty;
    /** amve -- ambient viewing environment. */
    propertyAmve(propertyId: number): AmveProperty;
    /** reve -- reference viewing environment. */
    propertyReve(propertyId: number): ReveProperty;
    /** ndwt -- nominal diffuse white luminance. */
    propertyNdwt(propertyId: number): { diffuseWhiteLuminance: number };
    /** crtt -- creation time (microseconds since 1904-01-01 UTC). */
    propertyCrtt(propertyId: number): bigint;
    /** mdft -- modification time (microseconds since 1904-01-01 UTC). */
    propertyMdft(propertyId: number): bigint;
    /** udes -- user description. */
    propertyUdes(propertyId: number): UdesProperty;
    /** altt -- accessibility text. */
    propertyAltt(propertyId: number): { text: string; lang: string };
    /** rref -- required reference types (FourCC array). */
    propertyRref(propertyId: number): string[];
    /** raw -- unrecognised property (FourCC type + payload bytes). */
    propertyRaw(propertyId: number): { type: string; data: Uint8Array };
    /** subs -- sub-sample information. */
    propertySubs(propertyId: number): SubsEntry[];

    // Entity-group properties

    /** Properties associated with an entity group (by group_id). */
    groupProperties(groupId: number): ItemProperty[];

    // Composition

    /** Grid derivation. */
    grid(itemId: number): GridInfo;
    /** Overlay derivation. */
    overlay(itemId: number): OverlayInfo;
    /** Tonemap/gainmap metadata. */
    tonemapGainmap(itemId: number): TonemapGainmapInfo;
    /** Tonemap image list. */
    tonemapImages(itemId: number): TonemapImage[];

    // Entity groups

    /** All entity groups in the file. */
    entityGroups(): EntityGroup[];

    // Tracks (image sequences)

    /** All track ids. */
    tracks(): Uint32Array;
    /** Track info struct. */
    trackInfo(trackId: number): TrackInfo;
    /** Track display width. */
    trackDisplayWidth(trackId: number): number;
    /** Track display height. */
    trackDisplayHeight(trackId: number): number;
    /** Number of samples in a track. */
    trackSampleCount(trackId: number): number;
    /** Playback duration in seconds. */
    trackPlaybackDurationSecs(trackId: number): number;
    /** All samples in presentation order. */
    trackSamples(trackId: number): TrackSample[];
    /** All timestamps for a track. */
    trackTimestamps(trackId: number): TrackTimestamp[];
    /** Track references (reference type + target track ids). */
    trackReferences(trackId: number): TrackReference[];
    /** Decoder-ready bitstream for one track sample. */
    trackSampleBitstream(trackId: number, sampleId: number): Uint8Array;
    /** Raw bytes for one track sample. */
    trackSampleRawData(trackId: number, sampleId: number): Uint8Array;
    /** Edit list metadata (flags, repetitions, duration). */
    trackEditListMeta(trackId: number): EditListMeta;
    /** Edit list units. */
    trackEditUnits(trackId: number): EditUnit[];
    /** Alternate track ids (same alternateGroupId). */
    trackAlternates(trackId: number): Uint32Array;
    /** Track groups. */
    trackGroups(trackId: number): TrackGroup[];
    /** Compatible brands for a track. */
    trackCompatibleBrands(trackId: number): string[];
    /** Per-sample timestamps (under edit lists, one sample -> N display times). */
    itemTimestamps(trackId: number, sampleId: number): ItemTimestamp[];
    /** Segment index entries. */
    segments(): SegmentEntry[];
    /** Per-sample coded width. */
    trackSampleWidth(trackId: number, sampleId: number): number;
    /** Per-sample coded height. */
    trackSampleHeight(trackId: number, sampleId: number): number;
    /** Per-sample {width, height}. */
    trackSampleSize(trackId: number, sampleId: number): { width: number; height: number };
    /** Per-track 3x3 display matrix (9 int32 values). */
    trackMatrix(trackId: number): Int32Array;
    /** Master (sync/random-access) sample ids in a track. */
    trackMasterImages(trackId: number): Uint32Array;
    /** Sample ids filtered by TrackSampleType. */
    trackSamplesByType(trackId: number, sampleType: number): Uint32Array;
    /** Per-sample-description-entry clean aperture. */
    trackSampleClap(trackId: number, sampleDescriptionIndex: number): ClapProperty;
    /** Per-sample-description-entry auxiliary type info. */
    trackSampleAuxi(trackId: number, sampleDescriptionIndex: number): TrackSampleAuxi;
    /** Decoder code type FourCC for a track sample. */
    trackSampleDecoderCodeType(trackId: number, sampleId: number): string;
    /** Decoder parameter sets for a track sample. */
    trackSampleDecoderParamSets(trackId: number, sampleId: number): ParamSetEntry[];
    /** Decoding-order list (for B-frames / composition offsets). */
    trackDecodingOrder(trackId: number): DecodingOrderEntry[];
    /** Decode dependencies for one track sample. */
    trackSampleDecodeDependencies(trackId: number, sampleId: number): Uint32Array;

    // Track sample sub-structures

    /** FourCC item type for a track sample. */
    trackSampleItemType(trackId: number, sampleId: number): string;
    /** Visual equivalence sample grouping. */
    trackSampleEqiv(trackId: number): SampleEqiv[];
    /** Sample-to-metadata-item grouping. */
    trackSampleStmi(trackId: number): SampleStmi[];
    /** Reference sample grouping. */
    trackSampleRefs(trackId: number): SampleRefs[];
    /** Rectangular region (tile) sample grouping. */
    trackSampleTrif(trackId: number): SampleTrif[];
    /** Sub-picture order sample grouping. */
    trackSampleSpor(trackId: number): SampleSpor[];
    /** Sub-layer multiview sample grouping. */
    trackSampleSulm(trackId: number): SampleSulm[];
    /** Mixed NALU-unit-type picture sample grouping. */
    trackSampleMinp(trackId: number): SampleMinp[];

    // DASH / segmented parsing

    /** Open from a filesystem path (wasm sandboxed FS only). */
    openFile(path: string): void;
    /** Parse a DASH initialization segment. */
    parseInitSegment(bytes: Uint8Array): void;
    /** Parse a DASH media segment. */
    parseSegment(bytes: Uint8Array, segmentId: number, earliestPts: number): void;
    /** Invalidate a previously-parsed media segment. */
    invalidateSegment(segmentId: number): void;
    /** Parse a segment index (sidx) and return segment info. */
    parseSegmentIndex(bytes: Uint8Array): SegmentInfo[];
}

// HEIFError

export class HEIFError extends Error {
    code: number;
    codeName: string;
    origin: string;
    line: number;
    detail: string;
}

// Sample flags (pure JS bit-extractors)

export const sampleFlags: {
    isLeading(flags: number): number;
    sampleDependsOn(flags: number): number;
    sampleIsDependedOn(flags: number): number;
    sampleHasRedundancy(flags: number): number;
    samplePaddingValue(flags: number): number;
    sampleIsNonSyncSample(flags: number): number;
    sampleDegradationPriority(flags: number): number;
};

// Supporting types

export interface ItemProperty {
    id: number;
    type: string;
    essential: boolean;
}

export interface ClapProperty {
    cleanApertureWidthN: number;
    cleanApertureWidthD: number;
    cleanApertureHeightN: number;
    cleanApertureHeightD: number;
    horizOffN: number;
    horizOffD: number;
    vertOffN: number;
    vertOffD: number;
}

export interface IsclProperty {
    targetWidthN: number;
    targetWidthD: number;
    targetHeightN: number;
    targetHeightD: number;
}

export interface CliProperty {
    maxContentLightLevel: number;
    maxPicAverageLightLevel: number;
}

export interface MdcvProperty {
    displayPrimariesX: [number, number, number];
    displayPrimariesY: [number, number, number];
    whitePointX: number;
    whitePointY: number;
    maxDisplayMasteringLuminance: number;
    minDisplayMasteringLuminance: number;
}

export interface CclvProperty {
    primariesPresent: number;
    minLuminancePresent: number;
    maxLuminancePresent: number;
    avgLuminancePresent: number;
    primariesX: [number, number, number];
    primariesY: [number, number, number];
    minLuminanceValue: number;
    maxLuminanceValue: number;
    avgLuminanceValue: number;
}

export interface AmveProperty {
    ambientIlluminance: number;
    ambientLightX: number;
    ambientLightY: number;
}

export interface ReveProperty {
    surroundLuminance: number;
    surroundLightX: number;
    surroundLightY: number;
    peripheryLuminance: number;
    peripheryLightX: number;
    peripheryLightY: number;
}

export interface UdesProperty {
    lang: string;
    name: string;
    description: string;
    tags: string;
}

export interface SubsEntry {
    subsampleSize: number;
    subsamplePriority: number;
    discardable: boolean;
}

export interface PixiChannel {
    channelIdc: number;
    componentFormat: number;
    subsamplingFlag: boolean;
    channelLabelFlag: boolean;
    subsamplingType: number;
    subsamplingLocation: number;
    label?: string;
}

export type ColrProperty =
    | { type: 'nclx'; colourPrimaries: number; transferCharacteristics: number; matrixCoefficients: number; fullRangeFlag: boolean }
    | { type: 'rICC' | 'prof'; iccProfile: Uint8Array }
    | { type: string };

export interface ParamSetEntry {
    type: number;
    data: Uint8Array;
}

export interface OinfProperty {
    scalabilityMask: number;
    ptl: OinfPtl[];
    operatingPoints: OinfOperatingPoint[];
    layers: OinfLayer[];
}

export interface OinfPtl {
    ptlIdx: number;
}

export interface OinfOperatingPoint {
    opIdx: number;
    layers: number[];
}

export interface OinfLayer {
    layerIdx: number;
    dependencies: number[];
    dimensions: number[];
}

export interface GridInfo {
    outputWidth: number;
    outputHeight: number;
    columns: number;
    rows: number;
    tileIds: Uint32Array;
}

export interface OverlayInfo {
    outputWidth: number;
    outputHeight: number;
    r: number;
    g: number;
    b: number;
    a: number;
    images: OverlayImage[];
}

export interface OverlayImage {
    itemId: number;
    horizontalOffset: number;
    verticalOffset: number;
}

export interface TonemapGainmapInfo {
    baseImageItemId: number;
    gainmapImageItemId: number;
    version: number;
    flags: number;
    channels: TonemapChannel[];
}

export interface TonemapChannel {
    gainMapMin: number;
    gainMapMax: number;
    gamma: number;
    baseOffset: number;
    alternateOffset: number;
}

export interface TonemapImage {
    itemId: number;
    role: number;
}

export interface EntityGroup {
    groupId: number;
    type: string;
    entityIds: number[];
}

export interface TrackInfo {
    features: number;
    alternateGroupId: number;
    alternateTrackIds: number[];
}

export interface TrackSample {
    sampleId: number;
    size: number;
    sampleFlags: number;
    segmentId: number;
    sampleType: number;
    hasClap: boolean;
    hasAuxi: boolean;
    codingConstraints: number;
    sampleEntryType: string;
}

export interface TrackTimestamp {
    sampleId: number;
    timestampMs: number;
}

export interface TrackReference {
    type: string;
    trackIds: number[];
}

export interface EditListMeta {
    flags: number;
    nRepetitions: number;
    durationMs: number;
}

export interface EditUnit {
    editType: number;
    mediaTimeMs: number;
    durationMs: number;
}

export interface TrackGroup {
    type: string;
    groupId: number;
}

export interface ItemTimestamp {
    timestampMs: number;
}

export interface SegmentEntry {
    segmentId: number;
    referenceId: number;
    timescale: number;
    earliestPresentationTimestamp: number;
    startDataOffset: number;
    dataSize: number;
    durationInTs: number;
    referenceType: number;
}

export interface TrackSampleAuxi {
    auxType: string;
    auxSubType: string;
}

export interface DecodingOrderEntry {
    timestampMs: number;
    sampleId: number;
}

export interface SampleEqiv {
    sampleGroupDescriptionIndex: number;
    timeOffset: number;
    timescaleMultiplier: number;
}

export interface SampleStmi {
    sampleGroupDescriptionIndex: number;
    metadataItemIds: number[];
}

export interface SampleRefs {
    sampleGroupDescriptionIndex: number;
    sampleId: number;
    referenceItemIds: number[];
}

export interface SampleTrif {
    sampleGroupDescriptionIndex: number;
    groupId: number;
    independentIdc: number;
    horizontalOffset: number;
    verticalOffset: number;
    regionWidth: number;
    regionHeight: number;
    rectRegionFlag: boolean;
    fullPicture: boolean;
    filteringDisabled: boolean;
    dependencies: number[];
}

export interface SampleSpor {
    sampleGroupDescriptionIndex: number;
    subpicIdBitPos: number;
    subpicIdInfoFlag: boolean;
    subpicdLenMinus1: number;
    startCodeEmulFlag: boolean;
    ppsSpsSubpicIdFlag: boolean;
    ppsId: number;
    spsId: number;
    subpTrackRefIdxs: number[];
}

export interface SampleSulm {
    sampleGroupDescriptionIndex: number;
    groupIdInfo4cc: string;
    groupIds: number[];
}

export interface SampleMinp {
    sampleGroupDescriptionIndex: number;
    ppsMixNaluTypesInPicBitPos: number;
    ppsId: number;
    mixSubpTrackIdx1: number[];
    mixSubpTrackIdx2: number[];
}

export interface SegmentInfo {
    segmentId: number;
    earliestPtsInTs: number;
    startDataOffset: number;
    referenceId: number;
    timescale: number;
    durationInTs: number;
    dataSize: number;
    referenceType: number;
}

export interface ResolvedProperties {
    raw: ItemProperty[];
    ispe?: { width: number; height: number };
    irot?: number;
    imir?: { axis: 'horizontal' | 'vertical' };
    pixi?: number[];
    colr?: ColrProperty;
    auxC?: string;
    clap?: ClapProperty;
    iscl?: IsclProperty;
    lsel?: { layerId: number };
    tols?: { targetOlsIndex: number };
    rloc?: { horizontalOffset: number; verticalOffset: number };
    oinf?: OinfProperty;
    clli?: CliProperty;
    mdcv?: MdcvProperty;
    cclv?: CclvProperty;
    amve?: AmveProperty;
    reve?: ReveProperty;
    ndwt?: { diffuseWhiteLuminance: number };
    pasp?: { relativeWidth: number; relativeHeight: number };
    crtt?: bigint;
    mdft?: bigint;
    udes?: UdesProperty;
    altt?: { text: string; lang: string };
    rref?: string[];
    subs?: SubsEntry[];
    unknown?: Array<{ type: string; data: Uint8Array | null }>;
}
