/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

"use strict";

/** Bridges HEIFReader with DecoderManager for HEIF file decoding.
 * @constructor
 * @param {HEIFReader} heifReader Modern heif.js reader (HEIF.open() result).
 * @param {string=}    url        Source URL — modern reader is bytes-in,
 *                                so the caller threads the URL separately
 *                                for logging/diagnostics.
 * @param {Object=}    fileInfo   Per-file snapshot built by
 *                                file-info.js#buildFileInfo. When provided
 *                                (the normal path through heif-object.js),
 *                                every helper below reads its closed-reader-
 *                                invariant data from this object instead of
 *                                hitting the wasm reader — turning the
 *                                ~50-round-trip per-decode chatter back into
 *                                pure JS field reads (§15 active item 2).
 *                                If omitted, helpers fall back to live reader
 *                                queries for back-compat with callers that
 *                                still construct an ImageProvider directly. */
function ImageProvider(heifReader, url, fileInfo) {
    var self = this;
    this._reader = heifReader;
    this._url    = url || "";
    this._fileInfo = fileInfo || null;
    this._managerCache = {}; // Cache decoder managers by type

    // Modern reader has no "context" concept for the metabox: items are
    // addressed by itemId, tracks by trackId. We map the legacy contextId
    // arg used throughout this file to: 0 == root meta box, >0 == trackId.
    this._metaContextId = 0;
    var trackList;
    if (this._fileInfo && this._fileInfo.trackIds) {
        trackList = this._fileInfo.trackIds.slice();
    } else {
        trackList = [];
        try { trackList = heifReader.tracks() || []; } catch (e) { /* still-image */ }
    }
    this._trackIds = trackList;
    this._trackIdSet = {};
    for (var _t = 0; _t < trackList.length; _t++) { this._trackIdSet[trackList[_t]] = true; }

    /** Resolve which snapshot to read from. Prefer the explicit arg (some
     *  callers thread their own copy), fall back to the one captured at
     *  construction. Returns null if neither is available — helpers must
     *  still tolerate that and fall back to the reader. */
    this._fi = function (fileInfoArg) {
        return fileInfoArg || self._fileInfo || null;
    };

    /** Look up a metabox image item entry from the snapshot. Returns null
     *  if the snapshot is missing or the item isn't in imageFeaturesMap
     *  (e.g. caller passed a bare itemId for a non-image item). */
    this._itemEntry = function (fileInfoArg, itemId) {
        var fi = self._fi(fileInfoArg);
        if (!fi || !fi.rootLevelMetaBoxProperties) return null;
        return fi.rootLevelMetaBoxProperties.imageFeaturesMap[itemId] || null;
    };

    /** Distinguish track context (positive trackId) from the metabox (0). */
    this._isTrackContext = function (contextId) {
        return contextId > 0 && self._trackIdSet[contextId] === true;
    };

    /** Bitstream fetch routed by context: track samples vs metabox items. */
    this._bitstream = function (contextId, itemId) {
        if (self._isTrackContext(contextId)) {
            return self._reader.trackSampleBitstream(contextId, itemId);
        }
        return self._reader.itemBitstream(itemId);
    };

    /** Decode dependencies routed by context. Modern reader exposes
     *  trackSampleDecodeDependencies for tracks; for metabox items there is no
     *  per-item dep accessor yet (§15) — still-image items typically carry no
     *  inter-item dependencies, so we return [itemId] which the caller's
     *  duplicate-suppression logic treats as "no extra deps".
     *
     *  Special case: `iden` (identity-derived) items carry no coded data of
     *  their own — the actual bitstream lives in the source item referenced
     *  via dimg.  Resolve through that reference so the caller never tries to
     *  fetch a bitstream for the `iden` wrapper itself. */
    this._decodeDependencies = function (contextId, itemId) {
        if (self._isTrackContext(contextId)) {
            try { return self._reader.trackSampleDecodeDependencies(contextId, itemId); } catch (e) {}
        }
        // Resolve iden items to their coded source (follow chains)
        try {
            var cur = itemId;
            var seen = {};
            while (self._reader.itemType(cur) === 'iden') {
                if (seen[cur]) break;  // cycle guard
                seen[cur] = true;
                var sources = self._reader.referencedFrom(cur, 'dimg');
                if (sources && sources.length > 0) {
                    cur = sources[0];
                } else {
                    break;
                }
            }
            if (cur !== itemId) {
                return [cur];
            }
        } catch (e) {}
        // Predictively coded items: `pred` references point to items
        // whose decoded picture the decoder needs as a reference frame.
        // Prepend them so they're decoded first in the combined stream.
        try {
            var predRefs = self._reader.referencedFrom(itemId, 'pred');
            if (predRefs && predRefs.length > 0) {
                var deps = [];
                for (var _p = 0; _p < predRefs.length; _p++) {
                    deps.push(predRefs[_p]);
                }
                deps.push(itemId);
                return deps;
            }
        } catch (e) {}
        return [itemId];
    };

    /** Concatenate two ArrayBuffers */
    function _concatenateBuffers(first, second) {
        var combined = new Uint8Array(first.byteLength + second.byteLength);
        combined.set(new Uint8Array(first), 0);
        combined.set(new Uint8Array(second), first.byteLength);
        return combined.buffer;
    }

    /** Result structure for decode operations */
    function DecodeResult(isSuccessful, imageFrames, maxWidth, maxHeight) {
        this.success = isSuccessful;
        this.frames = imageFrames || [];
        this.displayWidth = maxWidth || 0;
        this.displayHeight = maxHeight || 0;
        
        // Raw HEIF file information - populated during decode
        this.heifInfo = null;
        
        // Add helper method to get actual display dimensions considering rotation
        this.getDisplayDimensions = function() {
            if (this.frames.length > 0) {
                var frame = this.frames[0];
                return {
                    width: frame.width,
                    height: frame.height
                };
            }
            return {
                width: this.displayWidth,
                height: this.displayHeight
            };
        };
    }

    /** Context for queued decode requests */
    function DecodeRequest(completionHandler, itemIdentifiers, dataStream, dependencyMap, resultObject) {
        this.callback = completionHandler;
        this.itemIds = itemIdentifiers;
        this.currentFrameIndex = 0;
        this.stream = dataStream;
        this.dependencies = dependencyMap;
        this.payload = resultObject;
    }

    /** Determine decoder type based on file/item properties
     * @param {number} contextId Context identifier
     * @param {number} itemId Item identifier
     * @returns {string} Decoder type ('libde265', 'vvdec') or null if unsupported */
    this._determineDecoderType = function(contextId, itemId) {
        var majorBrand = self._reader.majorBrand;
        var compatibleBrands = self._reader.compatibleBrands;
        
        // Use HEIFUtils.analyzeBrands for authoritative codec detection
        var brandAnalysis = HEIFUtils.analyzeBrands(majorBrand, compatibleBrands);
        
        // Store brand analysis for later use (e.g., in deriveItemType)
        self._lastBrandAnalysis = brandAnalysis;
        
        // VVC - use vvdec decoder
        if (brandAnalysis.hasVVC) {
            return 'vvdec';
        }
        
        // HEVC - use libde265 decoder
        if (brandAnalysis.hasHEVC) {
            return 'libde265';
        }
        
        // AV1 - no decoder available
        if (brandAnalysis.hasAV1) {
            console.warn('AV1 decoder is not available. Only metadata parsing is supported for:', majorBrand);
            return null;
        }
        
        // AVC (H.264) - no decoder available yet
        if (brandAnalysis.hasAVC) {
            console.warn('AVC decoder is not available. Only metadata parsing is supported for:', majorBrand);
            return null;
        }
        
        // JPEG 2000 - no decoder available
        if (brandAnalysis.hasJ2K) {
            console.warn('JPEG 2000 decoder is not available. Only metadata parsing is supported for:', majorBrand);
            return null;
        }
        
        // Default to libde265 for unrecognized brands (legacy behavior)
        console.debug('Defaulting to libde265 decoder for brand:', majorBrand);
        return 'libde265';
    };

    /** Get or create decoder manager for specific type
     * @param {string} decoderType Type of decoder needed
     * @returns {DecoderManager} Manager instance */
    this._getDecoderManager = function(decoderType) {
        if (!self._managerCache[decoderType]) {
            self._managerCache[decoderType] = new DecoderManager(decoderType);
        }
        return self._managerCache[decoderType];
    };

    /** Check if item has alpha mask auxiliary image */
    this._getAlphaMaskItemId = function(contextId, itemId) {
        // Track samples don't carry per-item alpha auxiliaries.
        if (self._isTrackContext(contextId)) {
            return null;
        }
        // Snapshot path (no wasm round-trip).
        var entry = self._itemEntry(null, itemId);
        if (entry) {
            return (entry.auxlRefs && entry.auxlRefs.length > 0) ? entry.auxlRefs[0] : null;
        }
        // Fallback when the snapshot isn't available (legacy direct construction).
        var auxRefs;
        try { auxRefs = self._reader.referencedTo(itemId, "auxl"); }
        catch (e) { return null; }
        if (!auxRefs || auxRefs.length === 0) {
            return null;
        }
        // First aux ref is the alpha mask in the common case. The legacy
        // path additionally consulted imageFeaturesMap.isAuxiliaryImage and
        // an AUXC property type — both require entry points the modern
        // binding does not yet expose (item features, auxc property).
        // Returning the first ref is right for the still-image baseline;
        // tighter validation is a follow-up §15 candidate.
        return auxRefs[0];
    };

    /** Get the item type for a given item ID via the modern reader.
     * @param {number} itemId Item identifier
     * @returns {string|null} Item type ('tmap', 'grid', 'iovl', 'hvc1', etc.) or null */
    this._getItemType = function(itemId) {
        try { return self._reader.itemType(itemId) || null; }
        catch (e) { return null; }
    };

    this._isTmapItem = function(itemId) { return self._getItemType(itemId) === 'tmap'; };
    this._isGridItem = function(itemId) { return self._getItemType(itemId) === 'grid'; };
    this._isIovlItem = function(itemId) { return self._getItemType(itemId) === 'iovl'; };

    // ========================================================================
    // TMAP (Tone Map / Gainmap) HANDLING
    // tmap items use 'dimg' references to base image and gainmap image
    // ========================================================================

    /** Lazy-built map of {baseItemId -> {tmapItemId, gainmapItemId, metadata}}.
     *  Populated on first call to _getGainmapInfo by walking
     *  fileInfo.tmapIds and asking the reader for per-tmap header +
     *  channel records + dimg image-id list. Reader-invariant — kept
     *  for the lifetime of this ImageProvider. */
    var _tmapByBaseId = null;

    var _ensureTmapMap = function () {
        if (_tmapByBaseId !== null) return _tmapByBaseId;
        _tmapByBaseId = {};
        var fi = self._fileInfo;
        var tmapIds = (fi && fi.tmapIds) ? fi.tmapIds : [];
        for (var i = 0; i < tmapIds.length; i++) {
            var tmapId = tmapIds[i];
            try {
                var images = self._reader.tonemapImages(tmapId);
                if (!images || images.length < 2) continue;  // need base + gainmap
                var hdr = self._reader.tonemapGainmap(tmapId);
                var metadata = self._convertGainMapMetadata(hdr);
                if (metadata === null) continue;
                // Apple-style HDR gainmap: dimg[0] = base, dimg[1] = gainmap.
                _tmapByBaseId[images[0]] = {
                    tmapItemId:    tmapId,
                    gainmapItemId: images[1],
                    metadata:      metadata,
                };
            } catch (e) {
                console.debug('tmap ' + tmapId + ' rejected: ' + (e && e.message ? e.message : e));
            }
        }
        return _tmapByBaseId;
    };

    /** Get tmap components for a given tmap item id.
     *  Returns the GainMapData header (channels + headroom rationals)
     *  plus the resolved {baseItemId, gainmapItemId} pair, or null if
     *  the tmap is malformed (missing dimg refs, channel-count mismatch).
     *
     *  @param {number} contextId Context identifier (unused for metabox tmaps)
     *  @param {number} tmapItemId The tmap item ID
     *  @returns {Object|null} {baseItemId, gainmapItemId, metadata} or null. */
    this._getTmapComponents = function(contextId, tmapItemId) {
        try {
            var images = self._reader.tonemapImages(tmapItemId);
            if (!images || images.length < 2) return null;
            var hdr = self._reader.tonemapGainmap(tmapItemId);
            var metadata = self._convertGainMapMetadata(hdr);
            if (metadata === null) return null;
            return {
                baseItemId:    images[0],
                gainmapItemId: images[1],
                metadata:      metadata,
            };
        } catch (e) {
            return null;
        }
    };

    /** Convert GainMapData from HEIF API format (per-channel rationals)
     * to the float-valued shape consumed by decoder-manager.applyGainmap.
     * 
     * @param {Object} apiMetadata GainMapData structure from HEIF API
     * @returns {Object} Internal gainmap metadata format */
    this._convertGainMapMetadata = function(apiMetadata) {
        if (!apiMetadata) return null;

        // ISO 21496-1: when channel_count is 0 or the channels array is
        // empty, all per-channel values take their specification defaults.
        // Default: gainMapMin=0, gainMapMax=1, gamma=1, offsets=1/64.
        var channels = apiMetadata.channels;
        if (!channels || channels.length === 0) {
            channels = [{
                gainMapMinNumerator: 0, gainMapMinDenominator: 1,
                gainMapMaxNumerator: 1, gainMapMaxDenominator: 1,
                gammaNumerator: 1,      gammaDenominator: 1,
                baseOffsetNumerator: 1,  baseOffsetDenominator: 64,
                alternateOffsetNumerator: 1, alternateOffsetDenominator: 64
            }];
        }
        
        var isMultichannel = apiMetadata.isMultichannel && channels.length >= 3;
        
        // Helper to convert rational to float
        var toFloat = function(num, denom) {
            return denom !== 0 ? num / denom : 0;
        };
        
        // Build per-channel arrays if multichannel, otherwise use first channel for all
        var gainMapMin = [];
        var gainMapMax = [];
        var gamma = [];
        var baseOffset = [];
        var alternateOffset = [];
        
        var numChannels = isMultichannel ? Math.min(channels.length, 3) : 1;
        
        for (var i = 0; i < numChannels; i++) {
            var ch = channels[i];
            gainMapMin.push(toFloat(ch.gainMapMinNumerator, ch.gainMapMinDenominator));
            gainMapMax.push(toFloat(ch.gainMapMaxNumerator, ch.gainMapMaxDenominator));
            gamma.push(toFloat(ch.gammaNumerator, ch.gammaDenominator));
            baseOffset.push(toFloat(ch.baseOffsetNumerator, ch.baseOffsetDenominator));
            alternateOffset.push(toFloat(ch.alternateOffsetNumerator, ch.alternateOffsetDenominator));
        }
        
        // If single channel, return scalar values
        if (!isMultichannel) {
            return {
                gainMapMin: gainMapMin[0],
                gainMapMax: gainMapMax[0],
                gamma: gamma[0],
                baseOffset: baseOffset[0],
                alternateOffset: alternateOffset[0],
                baseHdrHeadroom: toFloat(apiMetadata.baseHdrHeadroomNumerator, apiMetadata.baseHdrHeadroomDenominator),
                alternateHdrHeadroom: toFloat(apiMetadata.alternateHdrHeadroomNumerator, apiMetadata.alternateHdrHeadroomDenominator),
                useBaseColourSpace: apiMetadata.useBaseColourSpace
            };
        }
        
        return {
            gainMapMin: gainMapMin,
            gainMapMax: gainMapMax,
            gamma: gamma,
            baseOffset: baseOffset,
            alternateOffset: alternateOffset,
            baseHdrHeadroom: toFloat(apiMetadata.baseHdrHeadroomNumerator, apiMetadata.baseHdrHeadroomDenominator),
            alternateHdrHeadroom: toFloat(apiMetadata.alternateHdrHeadroomNumerator, apiMetadata.alternateHdrHeadroomDenominator),
            useBaseColourSpace: apiMetadata.useBaseColourSpace,
            isMultichannel: true
        };
    };

    /** Check if item has a gainmap for HDR/SDR tone mapping.
     *
     *  Walks the per-file tmap snapshot (built lazily on first call)
     *  and returns the record if `itemId` is the base layer
     *  of some `tmap` derivation. Reader-invariant — the map is built
     *  once per ImageProvider lifetime.
     *
     *  @param {number} contextId Context identifier (unused — gainmap
     *      lookups are file-level, not track-scoped)
     *  @param {number} itemId Item identifier
     *  @returns {Object|null} {gainmapItemId, metadata} if `itemId` is
     *      the base image of a tmap derivation, else null. */
    this._getGainmapInfo = function(contextId, itemId) {
        var map = _ensureTmapMap();
        var entry = map[itemId];
        if (!entry) return null;
        return {
            gainmapItemId: entry.gainmapItemId,
            metadata:      entry.metadata,
        };
    };

    /** Build the per-decode "heifInfo" view consumed by HEIFFileLogger and
     *  attached to every DecodeResult.heifInfo.
     *
     *  Pre-Cut-C this method walked the reader on every call (8× itemsByType
     *  + per-item itemType/itemSize/itemPropertiesResolved + 3 ref walks —
     *  ~50 wasm round-trips per request on a 10-item file). After Cut C the
     *  closed-reader-invariant data lives on self._fileInfo (the snapshot
     *  built once at open time by file-info.js#buildFileInfo). All this
     *  method does now is project the snapshot into the legacy logger shape
     *  and layer the per-call bits (which items were requested, the alpha /
     *  gainmap resolution for THIS request) on top.
     *
     *  §15 fields the snapshot does not (yet) have — entityGroups, per-item
     *  feature flags, typed-property accessors — stay null/empty rather than
     *  faked, exactly as before. */
    this._buildHeifInfo = function(contextId, requestedItemIds, decodeList, propertiesMap, alphaMaskMap, gainmapMap) {
        var snapshot = self._fileInfo;
        var majorBrand = snapshot ? snapshot.majorBrand : null;
        var compatibleBrands = snapshot ? snapshot.compatibleBrands : [];
        var brandAnalysis = null;
        try { brandAnalysis = HEIFUtils.analyzeBrands(majorBrand, compatibleBrands); } catch (e) {}

        // Track summary projection (one entry per track). Pass everything
        // the snapshot already carries so the diagnostic logger and any
        // consumer downstream can show real sample counts, edit-list /
        // feature flags, frame-rate, etc., instead of synthesising zeros.
        var tracks = null;
        if (snapshot && snapshot.trackProperties && snapshot.trackProperties.length > 0) {
            tracks = [];
            for (var ti = 0; ti < snapshot.trackProperties.length; ti++) {
                var tp = snapshot.trackProperties[ti];
                tracks.push({
                    trackId:       tp.trackId,
                    displayWidth:  tp.displayWidth,
                    displayHeight: tp.displayHeight,
                    majorBrand:    tp.majorBrand,
                    timeScale:     tp.timeScale,
                    frameRate:     tp.frameRate,
                    sampleCount:   tp.sampleCount,
                    sampleIds:     tp.sampleIds,
                    trackFeature:  tp.trackFeature,
                });
            }
        }

        // Per-item map projection: keep snapshot entries verbatim so consumers
        // can read .properties / .thmbRefs / .auxlRefs / .dimgRefs / .isMaster
        // from the same shape file-info.js produces. Decode-list items not in
        // the metabox bucket (e.g. track samples) get a thin synthetic entry
        // so the logger doesn't see undefined holes.
        var items = {};
        if (snapshot && snapshot.rootLevelMetaBoxProperties) {
            var snapItems = snapshot.rootLevelMetaBoxProperties.imageFeaturesMap;
            for (var key in snapItems) {
                if (snapItems.hasOwnProperty(key)) items[key] = snapItems[key];
            }
        }
        for (var d = 0; d < decodeList.length; d++) {
            var did = decodeList[d];
            if (!items[did]) {
                // Synthetic entry for items not in the snapshot's image
                // bucket (track samples, opaque items). properties may have
                // been resolved per-call into propertiesMap.
                var resolved = propertiesMap ? propertiesMap[did] : null;
                items[did] = {
                    itemId: did,
                    type: 'unknown',
                    size: 0,
                    properties: resolved || null,
                    width:  (resolved && resolved.ispe) ? resolved.ispe.width  : 0,
                    height: (resolved && resolved.ispe) ? resolved.ispe.height : 0
                };
            }
        }

        // Reference summaries for the logger — same data already encoded on
        // each item entry, but the legacy logger consumes them as id→id maps
        // at the relationships.* level. Project from the snapshot.
        var thumbnailsRel = {};
        var auxiliariesRel = {};
        var derivedImagesRel = {};
        if (snapshot && snapshot.rootLevelMetaBoxProperties) {
            var ifMap = snapshot.rootLevelMetaBoxProperties.imageFeaturesMap;
            for (var iid in ifMap) {
                if (!ifMap.hasOwnProperty(iid)) continue;
                var e = ifMap[iid];
                if (e.thmbRefs && e.thmbRefs.length > 0) thumbnailsRel[iid] = e.thmbRefs[0];
                if (e.auxlRefs && e.auxlRefs.length > 0) auxiliariesRel[iid] = e.auxlRefs.slice();
                if (e.dimgRefs && e.dimgRefs.length > 0) derivedImagesRel[iid] = e.dimgRefs.slice();
            }
        }

        return {
            file: {
                url: self._url,
                majorBrand: majorBrand,
                compatibleBrands: compatibleBrands,
                brandAnalysis: brandAnalysis,
                fileFeatures: snapshot ? snapshot.fileFeature : null
            },
            context: {
                contextId: contextId,
                contextType: self._isTrackContext(contextId) ? 'track' : 'meta'
            },
            items: items,
            relationships: {
                alphaMasks:    alphaMaskMap || {},
                gainmaps:      gainmapMap   || {},
                thumbnails:    thumbnailsRel,
                derivedImages: derivedImagesRel,
                auxiliaries:   auxiliariesRel
            },
            decodedItems: {
                requested: requestedItemIds.slice(),
                all: decodeList.slice()
            },
            tracks: tracks,
            metaBox: snapshot ? snapshot.rootLevelMetaBoxProperties : null
        };
    };

    /** Request image decode for specified items */
    this.requestImageData = function(contextId, itemIds, callback) {
        if (!Array.isArray(itemIds)) {
            itemIds = [itemIds];
        }

        if (itemIds.length === 0) {
            return;
        }

        // Resolve tmap items: a tmap derived image has no coded data of its
        // own — it references a base image + gainmap via dimg. Replace the
        // tmap item in the request with its base, and wire up the gainmap
        // relationship so _postProcessGainmaps applies the tone map.
        for (var ti = 0; ti < itemIds.length; ti++) {
            if (self._isTmapItem(itemIds[ti])) {
                var tmapComp = self._getTmapComponents(contextId, itemIds[ti]);
                if (tmapComp) {
                    // Replace tmap with its base image
                    itemIds[ti] = tmapComp.baseItemId;
                }
            }
        }

        var decoderType = self._determineDecoderType(contextId, itemIds[0]);

        // Bitstreams are pushed onto an array and joined once at the end
        var streamChunks = [];
        var streamBytes  = 0;
        var decodeList = [];
        var depMap = {};
        var displayMap = {};
        var alphaMaskMap = {};
        var gainmapMap = {};

        var gatherError = null;
        var pushBitstream = function (cid, iid) {
            if (decoderType === null) return;
            var buf = self._bitstream(cid, iid);
            var view = new Uint8Array(buf);
            streamChunks.push(view);
            streamBytes += view.byteLength;
        };

        try {
            // Identify alpha masks for main items
            for (var i = 0; i < itemIds.length; i++) {
                var alphaMaskId = self._getAlphaMaskItemId(contextId, itemIds[i]);
                if (alphaMaskId !== null) {
                    alphaMaskMap[itemIds[i]] = alphaMaskId;
                }
            }

            // Identify gainmaps for main items (HDR/SDR tone mapping)
            for (var i = 0; i < itemIds.length; i++) {
                var gainmapInfo = self._getGainmapInfo(contextId, itemIds[i]);
                if (gainmapInfo !== null) {
                    gainmapMap[itemIds[i]] = gainmapInfo;
                }
            }

            // Process dependencies and build decode list
            for (var i = 0; i < itemIds.length; i++) {
                var dependencies = self._decodeDependencies(contextId, itemIds[i]);

                // If the item is iden, _decodeDependencies resolved through
                // dimg to the coded source.  That source IS the display item
                // (not a decode-only dependency), so mark it in displayMap.
                var idenResolved = false;
                if (!self._isTrackContext(contextId)) {
                    try { idenResolved = (self._reader.itemType(itemIds[i]) === 'iden'); } catch (e) {}
                }

                for (var j = 0; j < dependencies.length; j++) {
                    if (dependencies[j] !== itemIds[i]) {
                        if (idenResolved) {
                            // Treat the coded source as a display item
                            if (!(dependencies[j] in depMap) && !(dependencies[j] in displayMap)) {
                                displayMap[dependencies[j]] = true;
                                pushBitstream(contextId, dependencies[j]);
                                decodeList.push(dependencies[j]);
                            }
                        } else if (!(dependencies[j] in depMap) && !(dependencies[j] in displayMap)) {
                            depMap[dependencies[j]] = true;
                            pushBitstream(contextId, dependencies[j]);
                            decodeList.push(dependencies[j]);
                        }
                    } else {
                        displayMap[dependencies[j]] = true;
                    }
                }
            }

            // Add alpha masks to decode list
            for (var baseItemId in alphaMaskMap) {
                var maskItemId = alphaMaskMap[baseItemId];
                baseItemId = parseInt(baseItemId);

                if (!(maskItemId in depMap) && !(maskItemId in displayMap)) {
                    depMap[maskItemId] = true;
                    pushBitstream(contextId, maskItemId);
                    decodeList.push(maskItemId);
                }
            }

            // Add gainmap items to decode list
            for (var baseItemId in gainmapMap) {
                var gainmapInfo = gainmapMap[baseItemId];
                var gainmapItemId = gainmapInfo.gainmapItemId;
                baseItemId = parseInt(baseItemId);

                // Skip entries with no resolved gainmap item id
                if (gainmapItemId === null || gainmapItemId === undefined) {
                    continue;
                }

                if (!(gainmapItemId in depMap) && !(gainmapItemId in displayMap)) {
                    depMap[gainmapItemId] = true;
                    pushBitstream(contextId, gainmapItemId);
                    decodeList.push(gainmapItemId);
                }
            }

            // Add main items to decode list.  `iden` items have no coded
            // data — their source was already queued via _decodeDependencies,
            // so skip entirely (the decoded frame from the source IS this item's frame).
            for (i = 0; i < itemIds.length; i++) {
                if (!self._isTrackContext(contextId)) {
                    try {
                        if (self._reader.itemType(itemIds[i]) === 'iden') {
                            continue;
                        }
                    } catch (e) {}
                }
                pushBitstream(contextId, itemIds[i]);
                decodeList.push(itemIds[i]);
            }
        } catch (gatherErr) {
            gatherError = gatherErr;
            console.error('HEIF bitstream gather failed for', self._url, gatherErr);
        }

        // Single-pass concat — once at the end.
        var combinedStream;
        if (streamChunks.length === 0) {
            combinedStream = new Uint8Array(0);
        } else {
            combinedStream = new Uint8Array(streamBytes);
            var off = 0;
            for (var c = 0; c < streamChunks.length; c++) {
                combinedStream.set(streamChunks[c], off);
                off += streamChunks[c].byteLength;
            }
        }
        streamChunks = null;

        // Build properties map for decode list items. Track samples have no
        // per-sample property accessor yet (§15) — only metabox items carry
        // ispe/irot/imir/pixi/colr through itemPropertiesResolved.
        var propertiesMap = {};
        if (!self._isTrackContext(contextId)) {
            for (var i = 0; i < decodeList.length; i++) {
                var itemId = decodeList[i];
                try {
                    var props = self._reader.itemPropertiesResolved(itemId);
                    if (props) propertiesMap[itemId] = props;
                } catch (e) {}
            }
        }

        // Build heifInfo with all raw HEIF file data (always available, even without decoder)
        var heifInfo = self._buildHeifInfo(contextId, itemIds, decodeList, propertiesMap, alphaMaskMap, gainmapMap);

        // =====================================================================
        // LOG FILE INFO IMMEDIATELY - before any decode attempts
        // This ensures file metadata is always logged regardless of:
        // - Decoder availability
        // - Decode success/failure
        // - Partial decode (some frames fail)
        // =====================================================================
        if (window.HEIFFileLogger) {
            window.HEIFFileLogger.logFileInfo({
                filename: self._url,
                heifInfo: heifInfo,
                propertiesMap: propertiesMap,
                alphaMaskMap: alphaMaskMap,
                gainmapMap: gainmapMap,
                decoderType: decoderType,
                itemCount: decodeList.length
            });
        }

        // If no decoder available, return metadata-only result with error
        if (decoderType === null) {
            // Flush the stashed file info now — no decode will follow to
            // trigger logDecodeComplete, so emit a warning-styled group here.
            if (window.HEIFFileLogger && window.HEIFFileLogger.logDecodeComplete) {
                window.HEIFFileLogger.logDecodeComplete({
                    filename: self._url,
                    success: false,
                    noDecoder: true,
                    decodedFrameCount: 0,
                    expectedFrameCount: decodeList.length,
                    error: 'No decoder available for this codec (metadata only)'
                });
            }

            var errorResult = new DecodeResult(false, [], 0, 0);
            errorResult.heifInfo = heifInfo;
            errorResult.error = 'No decoder available for this codec. Metadata is still available.';
            errorResult.noDecoder = true;
            
            // Call callback asynchronously to maintain consistent behavior
            setTimeout(function() {
                callback(errorResult);
            }, 0);
            return;
        }

        // Bitstream gather threw earlier — surface it asynchronously so the
        // caller's then-chain shape stays uniform with success / no-decoder.
        if (gatherError !== null) {
            var gatherErrorResult = new DecodeResult(false, [], 0, 0);
            gatherErrorResult.heifInfo = heifInfo;
            gatherErrorResult.error = 'Bitstream gather failed: ' + (gatherError && gatherError.message || gatherError);
            setTimeout(function () { callback(gatherErrorResult); }, 0);
            return;
        }

        var manager = self._getDecoderManager(decoderType);
        var result = new DecodeResult(true, [], 0, 0);
        result.heifInfo = heifInfo;

        var request = new DecodeRequest(callback, decodeList, combinedStream, depMap, result);
        request.propertiesMap = propertiesMap;
        request.alphaMaskMap = alphaMaskMap;
        request.gainmapMap = gainmapMap;
        request.filename = self._url;
        
        manager.enqueueRequest(request);
        manager.processNext();
    };

    /** Get display items for context
     * @param {Object} fileInfo File information object
     * @param {number} contextId Context identifier
     * @returns {Array.<number>} Array of item IDs */
    /** Items considered "display" for a context (master images for metabox,
     * sync samples for tracks). The fileInfo arg is ignored — the modern
     * reader has the truth. */
    this._getDisplayItems = function(fileInfo, contextId) {
        if (self._isTrackContext(contextId)) {
            try { return self._reader.trackMasterImages(contextId) || []; }
            catch (e) { return []; }
        }
        var fi = self._fi(fileInfo);
        if (fi && fi.masterIds) return fi.masterIds.slice();
        try { return self._reader.masterImages() || []; }
        catch (e) { return []; }
    };

    /** Get maximum display height. */
    this.getDisplayHeight = function(fileInfo, contextId) {
        if (self._isTrackContext(contextId)) {
            var fi0 = self._fi(fileInfo);
            if (fi0 && fi0.trackProperties) {
                for (var ti = 0; ti < fi0.trackProperties.length; ti++) {
                    if (fi0.trackProperties[ti].trackId === contextId) return fi0.trackProperties[ti].displayHeight || 0;
                }
            }
            try { return self._reader.trackDisplayHeight(contextId) || 0; }
            catch (e) { return 0; }
        }
        var items = self._getDisplayItems(fileInfo, contextId);
        var fi = self._fi(fileInfo);
        var maximum = 0;
        for (var i = 0; i < items.length; i++) {
            var h = 0;
            var entry = self._itemEntry(fi, items[i]);
            if (entry) { h = entry.height || 0; }
            else { try { h = self._reader.itemHeight(items[i]) || 0; } catch (e) {} }
            if (h > maximum) maximum = h;
        }
        return maximum;
    };

    /** Get maximum display width. */
    this.getDisplayWidth = function(fileInfo, contextId) {
        if (self._isTrackContext(contextId)) {
            var fi0 = self._fi(fileInfo);
            if (fi0 && fi0.trackProperties) {
                for (var ti = 0; ti < fi0.trackProperties.length; ti++) {
                    if (fi0.trackProperties[ti].trackId === contextId) return fi0.trackProperties[ti].displayWidth || 0;
                }
            }
            try { return self._reader.trackDisplayWidth(contextId) || 0; }
            catch (e) { return 0; }
        }
        var items = self._getDisplayItems(fileInfo, contextId);
        var fi = self._fi(fileInfo);
        var maximum = 0;
        for (var i = 0; i < items.length; i++) {
            var w = 0;
            var entry = self._itemEntry(fi, items[i]);
            if (entry) { w = entry.width || 0; }
            else { try { w = self._reader.itemWidth(items[i]) || 0; } catch (e) {} }
            if (w > maximum) maximum = w;
        }
        return maximum;
    };

    /** Master-image context. Returns the first track with master samples,
     * or 0 (metabox sentinel) if there's a metabox with master items. */
    this.getMasterContextId = function(fileInfo) {
        var fi = self._fi(fileInfo);
        if (fi) {
            // Sequence wins: any track is by definition a master sample stream.
            if (fi.trackIds && fi.trackIds.length > 0) return fi.trackIds[0];
            if (fi.masterIds && fi.masterIds.length > 0) return self._metaContextId;
            return null;
        }
        for (var i = 0; i < self._trackIds.length; i++) {
            var tid = self._trackIds[i];
            try {
                if ((self._reader.trackMasterImages(tid) || []).length > 0) {
                    return tid;
                }
            } catch (e) { /* skip */ }
        }
        try {
            if ((self._reader.masterImages() || []).length > 0) {
                return self._metaContextId;
            }
        } catch (e) {}
        return null;
    };

    /** Thumbnail context. Track-level thumbnail sequences first, then
     * metabox if it contains thumbnail items. */
    this.getThumbnailContextId = function(fileInfo) {
        // No track-flavoured "is-thumbnail-sequence" probe in the modern
        // reader yet (§15 candidate). Skip tracks for now and look in the
        // metabox.
        var fi = self._fi(fileInfo);
        if (fi) {
            return (fi.thmbIds && fi.thmbIds.length > 0) ? self._metaContextId : null;
        }
        try {
            if ((self._reader.itemsByType("thmb") || []).length > 0) {
                return self._metaContextId;
            }
        } catch (e) {}
        return null;
    };

    /** Request only the thumbnail for a master image
        * @param {Object} fileInfo File information object
        * @param {number} contextId Context identifier
        * @param {number} masterItemId Master image item ID
        * @param {function} callback Called with decoded thumbnail data */
        this.requestThumbnail = function(fileInfo, contextId, masterItemId, callback) {
           var thumbId = self.getThumbnailIdByItemId(fileInfo, contextId, masterItemId);
           
           if (thumbId === null) {
              callback({ success: false, error: 'No embedded thumbnail found' });
              return;
           }
           
           self.requestImageData(contextId, [thumbId], function(result) {
              if (result.success && result.frames.length > 0) {
                 callback({
                    success: true,
                    frame: result.frames[0],
                    isThumbnail: true
                 });
              } else {
                 callback({
                    success: false,
                    error: result.error || 'Thumbnail decode failed'
                 });
              }
           });
        };

    /** Master image IDs (item ids for metabox, sample ids for tracks). */
    this.getMasterIds = function(fileInfo) {
        var masterCtx = self.getMasterContextId(fileInfo);
        if (masterCtx === null) return null;
        if (self._isTrackContext(masterCtx)) {
            // Track sample ids stay lazy on the reader — snapshot doesn't
            // pre-walk the per-sample table (§0 principle: snapshot only
            // holds closed-reader-invariant scalars / type buckets).
            var samples;
            try { samples = self._reader.trackSamples(masterCtx) || []; }
            catch (e) { return []; }
            var ids = [];
            for (var i = 0; i < samples.length; i++) {
                var s = samples[i];
                ids.push(typeof s === 'object' ? s.sampleId : s);
            }
            return ids;
        }
        var fi = self._fi(fileInfo);
        if (fi && fi.masterIds) return fi.masterIds.slice();
        try { return self._reader.masterImages() || []; }
        catch (e) { return []; }
    };

    /** Thumbnail image IDs (metabox only for now — no track-thumbnail probe). */
    this.getThumbnailIds = function(fileInfo) {
        var thumbCtx = self.getThumbnailContextId(fileInfo);
        if (thumbCtx === null) return null;
        if (self._isTrackContext(thumbCtx)) {
            // §15 candidate: track-thumbnail sequence enumeration.
            return [];
        }
        var fi = self._fi(fileInfo);
        if (fi && fi.thmbIds) return fi.thmbIds.slice();
        try { return self._reader.itemsByType("thmb") || []; }
        catch (e) { return []; }
    };

    /** Master item id for a given thumbnail. Track context is identity. */
    this.getItemIdByThumbnailId = function(fileInfo, contextId, thumbnailId) {
        if (self._isTrackContext(contextId)) return thumbnailId;
        var references;
        try { references = self._reader.referencedFrom(thumbnailId, "thmb"); }
        catch (e) { return null; }
        if (references && references.length) return references[0];
        return null;
    };

    /** Get thumbnail ID for a given master item.
     *
     * Walk order:
     *   1. Direct `thmb` iref on the item.
     *   2. Grid/iovl fallback: some encoders (notably iOS) write the
     *      `thmb` iref on the FIRST TILE rather than on the composite
     *      derived item. Walk dimg-referenced tiles and pick the first
     *      one that carries its own `thmb`.
     *   3. Last-resort fallback: scan `itemsByType('thmb')` and pick the
     *      smallest standalone thumbnail strictly under half the master's
     *      area, on the assumption that real thumbnails are always
     *      <<5% of the master and a near-master-sized hit indicates we
     *      have the wrong heuristic. This catches Apple square 1:1
     *      captures that ship a thumbnail item but write no `thmb` iref
     *      on the grid or any tile.
     */
    this.getThumbnailIdByItemId = function(fileInfo, contextId, itemId) {
        if (self._isTrackContext(contextId)) return itemId;
        var fi = self._fi(fileInfo);
        var entry = self._itemEntry(fi, itemId);

        if (entry) {
            if (entry.thmbRefs && entry.thmbRefs.length) return entry.thmbRefs[0];
        } else {
            var references;
            try { references = self._reader.referencedTo(itemId, "thmb"); } catch (e) {}
            if (references && references.length) return references[0];
        }

        // Grid/iovl tile fallback: thmb iref may be on a tile rather than
        // the composite. Snapshot's dimgRefs already lists the tiles.
        var itype = entry ? entry.type : self._getItemType(itemId);
        if (itype === 'grid' || itype === 'iovl') {
            var tiles = (entry && entry.dimgRefs) ? entry.dimgRefs : null;
            if (!tiles) {
                try { tiles = self._reader.referencedFrom(itemId, "dimg") || []; } catch (e) { tiles = []; }
            }
            for (var di = 0; di < tiles.length; di++) {
                var tileId = tiles[di];
                var tileEntry = self._itemEntry(fi, tileId);
                if (tileEntry && tileEntry.thmbRefs && tileEntry.thmbRefs.length) {
                    return tileEntry.thmbRefs[0];
                }
                if (!tileEntry) {
                    try {
                        var tileThmb = self._reader.referencedTo(tileId, "thmb");
                        if (tileThmb && tileThmb.length) return tileThmb[0];
                    } catch (e) {}
                }
            }
        }

        // Last resort: scan all thumbnail items, pick smallest under
        // half-area ceiling. Without itemFeatures (\u00a715) we use itemsByType.
        var primaryW = 0, primaryH = 0;
        try {
            primaryW = self._reader.itemWidth(itemId)  || 0;
            primaryH = self._reader.itemHeight(itemId) || 0;
        } catch (e) {}
        var areaCeiling = (primaryW && primaryH)
            ? (primaryW * primaryH * 0.5)
            : Infinity;

        var thumbs = [];
        try { thumbs = self._reader.itemsByType("thmb") || []; } catch (e) {}

        var bestId = null, bestArea = Infinity;
        for (var ti = 0; ti < thumbs.length; ti++) {
            var nid = thumbs[ti];
            if (nid === itemId) continue;
            var w = 0, h = 0;
            try {
                w = self._reader.itemWidth(nid)  || 0;
                h = self._reader.itemHeight(nid) || 0;
            } catch (e) {}
            if (!w || !h) continue;
            var area = w * h;
            if (area >= areaCeiling) continue;
            if (area < bestArea) { bestArea = area; bestId = nid; }
        }
        if (bestId !== null) return bestId;
        return null;
    };

    /** Get meta box thumbnail IDs (snapshot first, reader fallback). */
    this.getMetaBoxThumbnailIds = function(fileInfo) {
        var fi = self._fi(fileInfo);
        if (fi && fi.thmbIds) return fi.thmbIds.slice();
        try { return self._reader.itemsByType('thmb') || []; } catch (e) { return []; }
    };

    /** Get meta box master IDs (snapshot first, reader fallback). */
    this.getMetaBoxMasterIds = function(fileInfo) {
        var fi = self._fi(fileInfo);
        if (fi && fi.masterIds) return fi.masterIds.slice();
        try { return self._reader.masterImages() || []; } catch (e) { return []; }
    };

    // =========================================================================
    // GALLERY HELPER METHODS - Simplified API for UI layers
    // =========================================================================

    /**
     * Get primary displayable item ID using the modern reader.
     * Priority: track-sequence first sample → grid/iovl derived → primaryItemId
     * (cover) → first 'master'-type item.
     * @param {Object} fileInfo - retained for signature compat; ignored.
     * @returns {Object} { contextId, primaryId, thumbId, isSequence, isDerived }
     */
    this.getPrimaryItemInfo = function(fileInfo) {
        var reader = self._reader;
        var fi = self._fi(fileInfo);
        var result = {
            contextId: null,
            primaryId: null,
            thumbId: null,
            isSequence: false,
            isDerived: false
        };

        // Sequences first — pick the first track and its first sample.
        // Snapshot tells us whether tracks exist; sample ids stay lazy.
        var tids = (fi && fi.trackIds) ? fi.trackIds : self._trackIds;
        if (tids && tids.length > 0) {
            var tid = tids[0];
            try {
                var samples = reader.trackSamples(tid);
                if (samples && samples.length > 0) {
                    result.isSequence = true;
                    result.contextId = tid;
                    result.primaryId = samples[0].sampleId;
                    return result;
                }
            } catch (e) {}
        }

        result.contextId = self._metaContextId;

        // Priority 1: grid/iovl derived master (the "composed" image).
        var grids = (fi && fi.gridIds) ? fi.gridIds
                    : (function () { try { return reader.itemsByType('grid') || []; } catch (e) { return []; } })();
        var iovls = (fi && fi.iovlIds) ? fi.iovlIds
                    : (function () { try { return reader.itemsByType('iovl') || []; } catch (e) { return []; } })();
        if (grids.length > 0) {
            result.primaryId = grids[0];
            result.isDerived = true;
            result.thumbId = self.getThumbnailIdByItemId(fi, result.contextId, result.primaryId);
            return result;
        }
        if (iovls.length > 0) {
            result.primaryId = iovls[0];
            result.isDerived = true;
            result.thumbId = self.getThumbnailIdByItemId(fi, result.contextId, result.primaryId);
            return result;
        }

        // Priority 2: primaryItemId (cover image).
        var pid = fi ? fi.primaryItemId : 0;
        if (!pid) { try { pid = reader.primaryItemId; } catch (e) {} }
        if (pid && pid > 0) {
            var t = null;
            var pe = self._itemEntry(fi, pid);
            if (pe) t = pe.type;
            else { try { t = reader.itemType(pid); } catch (e) {} }
            if (t !== 'Exif' && t !== 'mime') {
                result.primaryId = pid;
                if (t === 'grid' || t === 'iovl') result.isDerived = true;
            }
        }

        // Priority 3: first 'master'-type item.
        if (result.primaryId === null) {
            var masters = (fi && fi.masterIds) ? fi.masterIds
                          : (function () { try { return reader.masterImages() || []; } catch (e) { return []; } })();
            if (masters.length > 0) {
                result.primaryId = masters[0];
            }
        }

        if (result.primaryId !== null) {
            result.thumbId = self.getThumbnailIdByItemId(fi, result.contextId, result.primaryId);
        }

        return result;
    };

    /**
     * Derive item type from the file. Modern reader walk only — no fileFeature
     * dependency. Brand analysis still drives the still/sequence split.
     *
     * Entity-group classification (stereo / bothie / brackets / panorama /
     * slideshow / progressive / audio-image / album / favorites / burst)
     * is driven by `fileInfo.rootLevelMetaBoxProperties.entityGroups` —
     * the GroupsListBox payload now surfaced on the snapshot.
     *
     * @returns {string} One of: 'animation', 'burst', 'tmap', 'grid',
     *   'overlay', 'alpha', 'stereo', 'bothie', 'panorama', 'slideshow',
     *   'progressive', 'audio-image',
     *   'bracket-exposure'/'bracket-wb'/'bracket-focus'/'bracket-flash'/'bracket-dof',
     *   'album', 'favorites', 'collection', 'still'
     */
    this.deriveItemType = function(fileInfo) {
        var reader = self._reader;
        var fi = self._fi(fileInfo);

        var brandAnalysis = self._lastBrandAnalysis;
        if (!brandAnalysis) {
            try {
                brandAnalysis = HEIFUtils.analyzeBrands(
                    fi ? fi.majorBrand : reader.majorBrand,
                    fi ? fi.compatibleBrands : reader.compatibleBrands);
            } catch (e) {
                brandAnalysis = { isSequence: false };
            }
        }

        // Entity-group lookup. Build a map { type -> first matching group }
        // so the rest of the function reads as flat checks.
        var entityGroups = (fi && fi.rootLevelMetaBoxProperties)
            ? fi.rootLevelMetaBoxProperties.entityGroups : null;
        var hasGroup = function (type) {
            if (!entityGroups) return false;
            for (var i = 0; i < entityGroups.length; i++) {
                if (entityGroups[i].type === type
                    && entityGroups[i].entityIds
                    && entityGroups[i].entityIds.length > 0) return true;
            }
            return false;
        };

        var tids = (fi && fi.trackIds) ? fi.trackIds : self._trackIds;
        var hasTrack = !!(tids && tids.length > 0);

        // Burst vs animation discrimination
        // ---------------------------------
        // ISO/IEC 23008-12 §6.8.2 says a burst capture is marked with the
        // 'brst' entity group. That's the spec-defined signal and always wins.
        //
        // Legacy fallback for pre-2024 captures that pre-date 'brst' but
        // ship the same structural shape (image-sequence track AND multiple
        // metabox master items, e.g. Apple bursts): treat as burst. A pure
        // animation has a 'pict' track but typically no full-resolution
        // metabox masters (the samples ARE the frames).
        if (hasGroup('brst')) return 'burst';
        var masterCount = (fi && fi.masterIds) ? fi.masterIds.length : 0;
        if (hasTrack && masterCount > 1) return 'burst';

        // Single-purpose entity groups (each implies a specific modal).
        // These MUST be checked before the generic isSequence+hasTrack
        // animation test — a file can have both a track AND an entity
        // group (e.g. C048 has tsyn + two tracks for the bothie pair).
        // The entity group carries spec-defined semantics that wins.
        if (hasGroup('ster'))                  return 'stereo';
        if (hasGroup('tsyn'))                  return 'bothie';
        if (hasGroup('pano'))                  return 'panorama';
        if (hasGroup('slid'))                  return 'slideshow';
        if (hasGroup('prgr'))                  return 'progressive';
        // 'iaug' is the real Image+Audio group. 'altr' is handled
        // after tmap/grid/overlay/alpha (see below) so those types
        // aren't hijacked — altr is generic and used by many pairs.
        if (hasGroup('iaug'))                  return 'audio-image';
        if (hasGroup('aebr'))                  return 'bracket-exposure';
        if (hasGroup('wbbr'))                  return 'bracket-wb';
        if (hasGroup('fobr') || hasGroup('afbr')) return 'bracket-focus';
        if (hasGroup('flbr'))                  return 'bracket-flash';
        if (hasGroup('dobr'))                  return 'bracket-dof';
        if (hasGroup('albc'))                  return 'album';
        if (hasGroup('favc'))                  return 'favorites';

        if (brandAnalysis.isSequence && hasTrack) {
            return 'animation';
        }

        if (fi) {
            if (fi.tmapIds && fi.tmapIds.length > 0) return 'tmap';
            if (fi.gridIds && fi.gridIds.length > 0) return 'grid';
            if (fi.iovlIds && fi.iovlIds.length > 0) return 'overlay';
            // Alpha auxiliary check: any auxl item present implies a master
            // has a linked aux (no AUXC type split exposed today, but the
            // auxl bucket itself is enough for the website's classifier).
            if (fi.auxlIds && fi.auxlIds.length > 0) return 'alpha';
            // Alternate items: 'altr' means "these are alternatives of
            // each other" (different resolutions, codecs, etc.). Only
            // classify as 'altr' when no higher-priority type (tmap,
            // grid, overlay, alpha) already claimed the file — those
            // types use altr internally for their own purposes (e.g.
            // tmap+gainmap pair).
            if (hasGroup('altr'))               return 'altr';
            // Identity-derived primary: the primary item is `iden` (no coded
            // data, references a source item via dimg and applies properties
            // like irot/imir).  Show in a dedicated modal.
            try {
                if (reader.itemType(fi.primaryItemId) === 'iden') return 'iden';
            } catch (e) {}
            // Filter hidden items from the collection count — hidden
            // masters are not intended for display.
            var visibleMasters = fi.masterIds;
            if (fi.hiddenIds && fi.hiddenIds.length > 0) {
                visibleMasters = fi.masterIds.filter(function (id) {
                    return fi.hiddenIds.indexOf(id) === -1;
                });
            }
            if (visibleMasters.length > 1) return 'collection';
            return 'still';
        }

        try {
            if ((reader.itemsByType('tmap') || []).length > 0) return 'tmap';
            if ((reader.itemsByType('grid') || []).length > 0) return 'grid';
            if ((reader.itemsByType('iovl') || []).length > 0) return 'overlay';
        } catch (e) {}

        try {
            var masters = reader.masterImages() || [];
            for (var i = 0; i < masters.length; i++) {
                var auxRefs = reader.referencedTo(masters[i], 'auxl');
                if (auxRefs && auxRefs.length > 0) return 'alpha';
            }
        } catch (e) {}

        try {
            if ((reader.masterImages() || []).length > 1) return 'collection';
        } catch (e) {}

        return 'still';
    };

    /**
     * Get bit depth from item properties using itemPropertiesResolved.pixi.
     */
    this.getBitDepth = function(fileInfo, itemId) {
        if (!itemId) return 8;
        var entry = self._itemEntry(self._fi(fileInfo), itemId);
        if (entry && entry.properties && entry.properties.pixi && entry.properties.pixi.length > 0) {
            return entry.properties.pixi[0];
        }
        try {
            var resolved = self._reader.itemPropertiesResolved(itemId);
            if (resolved && resolved.pixi && resolved.pixi.length > 0) {
                return resolved.pixi[0];
            }
        } catch (e) {}
        return 8;
    };

    /**
     * Get playback FPS for the first image-sequence track via trackInfo.frameRate.
     */
    this.getPlaybackFPS = function(fileInfo) {
        var fi = self._fi(fileInfo);
        if (fi && fi.trackProperties && fi.trackProperties.length > 0) {
            var fr = fi.trackProperties[0].frameRate;
            if (fr && fr.den > 0) return fr.num / fr.den;
            return 0;
        }
        if (!self._trackIds || self._trackIds.length === 0) return 0;
        try {
            var info = self._reader.trackInfo(self._trackIds[0]);
            if (info && info.frameRate && info.frameRate.den > 0) {
                return info.frameRate.num / info.frameRate.den;
            }
        } catch (e) {}
        return 0;
    };

    /**
     * Extract comprehensive metadata for an item using the modern reader.
     * @param {Object} fileInfo - retained for signature compat; ignored.
     * @param {number} itemId - item id (metabox) or sample id (sequence)
     * @param {Object} primaryInfo - optional primary info from getPrimaryItemInfo()
     * @returns {Object} Metadata object
     */
    this.extractMetadata = function(fileInfo, itemId, primaryInfo) {
        var reader = self._reader;
        var meta = {
            width: 0,
            height: 0,
            bitDepth: 8,
            codec: 'Unknown',
            format: 'Unknown',
            colorSpace: 'Unknown',
            color: null,
            hasAlpha: false,
            hasThumbnail: false,
            thumbWidth: 0,
            thumbHeight: 0,
            rotation: 0,
            mirror: null,
            cleanAperture: null,
            scale: null,
            layerSelection: null,
            targetOls: null,
            relativeLocation: null,
            operatingPointsInfo: null,
            frameCount: 1,
            brands: [],
            itemId: itemId || null
        };

        var isSequence = !!(primaryInfo && primaryInfo.isSequence);
        var trackContextId = isSequence ? primaryInfo.contextId : null;
        var contextId = isSequence ? trackContextId : self._metaContextId;

        // Dimensions
        if (isSequence && trackContextId) {
            try {
                meta.width  = reader.trackDisplayWidth(trackContextId);
                meta.height = reader.trackDisplayHeight(trackContextId);
            } catch (e) {}
        } else if (itemId) {
            try {
                meta.width  = reader.itemWidth(itemId);
                meta.height = reader.itemHeight(itemId);
            } catch (e) {}
        }

        // Brands → format/codec
        try {
            meta.brands = reader.compatibleBrands || [];
            var brandAnalysis = HEIFUtils.analyzeBrands(reader.majorBrand, meta.brands);
            meta.format = brandAnalysis.format;
            meta.codec  = brandAnalysis.codecString;
            meta.brandAnalysis = brandAnalysis;
        } catch (e) {}

        // Item-level resolved properties (ispe/irot/imir/clap/iscl/lsel/pixi/colr).
        if (itemId && !isSequence) {
            try {
                var resolved = reader.itemPropertiesResolved(itemId);
                if (resolved) {
                    if (resolved.ispe) {
                        meta.width  = resolved.ispe.width  || meta.width;
                        meta.height = resolved.ispe.height || meta.height;
                    }
                    if (typeof resolved.irot === 'number') meta.rotation = resolved.irot;
                    if (resolved.imir) meta.mirror = resolved.imir;
                    if (resolved.clap) meta.cleanAperture = resolved.clap;
                    if (resolved.iscl) meta.scale = resolved.iscl;
                    if (resolved.lsel) meta.layerSelection = resolved.lsel;
                    if (resolved.tols) meta.targetOls = resolved.tols;
                    if (resolved.rloc) meta.relativeLocation = resolved.rloc;
                    if (resolved.oinf) meta.operatingPointsInfo = resolved.oinf;
                    if (resolved.pixi && resolved.pixi.length > 0) meta.bitDepth = resolved.pixi[0];
                    if (resolved.colr) {
                        meta.colorSpace = self._getColorSpaceName(resolved.colr);
                        meta.color = resolved.colr;
                    }
                }
            } catch (e) {}
        }

        // Thumbnail
        if (itemId) {
            var thumbId = self.getThumbnailIdByItemId(fileInfo, contextId, itemId);
            if (thumbId) {
                meta.hasThumbnail = true;
                try {
                    meta.thumbWidth  = reader.itemWidth(thumbId);
                    meta.thumbHeight = reader.itemHeight(thumbId);
                } catch (e) {}
            }
        }

        // Alpha auxiliary — metabox only.
        if (itemId && !isSequence) {
            try {
                var auxRefs = reader.referencedTo(itemId, 'auxl');
                if (auxRefs && auxRefs.length > 0) meta.hasAlpha = true;
            } catch (e) {}
        }

        // Sequence frame count / fps / codec
        if (isSequence && trackContextId) {
            try {
                var samples = reader.trackSamples(trackContextId);
                meta.frameCount = (samples && samples.length) || 1;
                if (samples && samples.length > 0) {
                    var entryType = (samples[0].sampleEntryType || '').toLowerCase();
                    if (entryType === 'hvc1' || entryType === 'hev1' || entryType === 'lhv1') meta.codec = 'HEVC';
                    else if (entryType === 'vvc1' || entryType === 'vvi1') meta.codec = 'VVC';
                    else if (entryType === 'av01') meta.codec = 'AV1';
                    else if (entryType) meta.codec = samples[0].sampleEntryType;
                }
            } catch (e) {}
            try {
                var info = reader.trackInfo(trackContextId);
                if (info) {
                    if (info.frameRate && info.frameRate.den > 0) {
                        meta.fps = info.frameRate.num / info.frameRate.den;
                    }
                    if (info.timeScale) meta.timeScale = info.timeScale;
                }
            } catch (e) {}
            // §15: hasInfiniteLoopPlayback flag not exposed by trackInfo bitmask
            // accessor yet — default to false.
            meta.isLooping = false;
        }

        return meta;
    };

    /**
     * Get human-readable color space name from COLR property.
     *
     * Thin delegate to HEIFUtils.getColorSpaceName so that this module and
     * the modal info panel always agree on the colour-space string and
     * pick up the BT.2020 matrix override (matrix coefficients 9/10) for
     * free. Previously a private copy here drifted out of sync and missed
     * that override.
     *
     * @param {Object} colrProp - colr property from itemPropertiesResolved()
     *                             (modern shape: {type, primaries, transfer,
     *                             matrix, fullRange})
     * @returns {string} Human-readable color space name
     */
    this._getColorSpaceName = function(colrProp) {
        if (!colrProp) return 'Unknown';
        if (typeof HEIFUtils === 'undefined' || !HEIFUtils.getColorSpaceName) {
            // Should never happen at runtime — heif-utils.js is loaded
            // before image-provider.js by build/wrap-head.js.
            return 'Unknown';
        }
        // Bridge the modern colr shape into the legacy one consumed by
        // HEIFUtils.getColorSpaceName. Done locally rather than in
        // heif-utils so the utility's surface stays stable.
        var legacy = {
            colourType:              colrProp.type,
            colourPrimaries:         colrProp.primaries,
            transferCharacteristics: colrProp.transfer,
            matrixCoefficients:      colrProp.matrix,
            fullRangeFlag:           colrProp.fullRange,
        };
        return HEIFUtils.getColorSpaceName(legacy);
    };

    /**
     * Filter item IDs to exclude metadata items (Exif/XMP/MPEG7).
     * Modern reader walk: itemType(id) returns the 4cc; metadata 4ccs are
     * 'Exif' and 'mime' (XMP/MPEG7 carried in mime).
     * @param {Object} fileInfo - retained for signature compat; ignored.
     * @param {Array<number>|number} itemIds - Item IDs to filter
     * @returns {Array<number>|number|null} Filtered item IDs
     */
    this.filterImageItems = function(fileInfo, itemIds) {
        var isSingleId = !Array.isArray(itemIds);
        if (isSingleId) itemIds = [itemIds];

        var fi = self._fi(fileInfo);
        var info = (fi && fi.rootLevelMetaBoxProperties) ? fi.rootLevelMetaBoxProperties.itemInformationMap : null;

        var filtered = itemIds.filter(function(id) {
            var t;
            if (info && info[id]) {
                t = info[id].type;
            } else {
                try { t = self._reader.itemType(id); } catch (e) { return false; }
            }
            if (t === 'Exif' || t === 'mime') return false;
            return true;
        });

        return isSingleId ? (filtered.length > 0 ? filtered[0] : null) : filtered;
    };

    /**
     * Get grid item definition from fileInfo
     * @param {Object} fileInfo - FileInfo from HEIFReader.requestFileInfo()
     * @param {number} gridItemId - Grid item ID
     * @returns {Object|null} Grid definition { outputWidth, outputHeight, columns, rows, tileIds }
     */
    this.getGridItemInfo = function(fileInfo, gridItemId) {
        if (!gridItemId) return null;
        try {
            return self._reader.grid(gridItemId);
        } catch (e) {
            console.error('Failed to get grid info for item', gridItemId, e);
            return null;
        }
    };

    /**
     * Get overlay item definition from fileInfo
     * @param {Object} fileInfo - FileInfo from HEIFReader.requestFileInfo()
     * @param {number} iovlItemId - Overlay item ID
     * @returns {Object|null} Overlay definition { outputWidth, outputHeight, fillRgba, images: [{imageId, hOffset, vOffset}] }
     */
    this.getOverlayItemInfo = function(fileInfo, iovlItemId) {
        if (!iovlItemId) return null;
        try {
            return self._reader.overlay(iovlItemId);
        } catch (e) {
            console.error('Failed to get overlay info for item', iovlItemId, e);
            return null;
        }
    };

    /** Clean up decoder managers. The HEIFReader is owned by heif-object.js
     *  (which calls reader.close() on its own dispose) — we just drop the
     *  reference here so we can't accidentally use a closed handle. */
    this.dispose = function() {
        for (var key in self._managerCache) {
            if (self._managerCache.hasOwnProperty(key)) {
                var mgr = self._managerCache[key];
                if (mgr) {
                    mgr._contextQueue = [];
                    mgr._decodedFrames = {};
                    mgr._currentContext = null;
                }
            }
        }
        self._managerCache = {};
        self._reader = null;
    };
}
