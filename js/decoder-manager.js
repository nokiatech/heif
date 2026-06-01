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

// Decoder manager: receives Annex-B encoded bitstreams, drives
// libde265 / vvdec decode → RGBA, and returns pixel data.
// No HEIF-binding coupling beyond receiving compressed bytes
// and colour metadata from image-provider.js.

/** Global HDR/SDR display configuration
 * Controls how HDR content and gainmaps are processed
 * @namespace */
var HEIFDisplayConfig = window.HEIFDisplayConfig || {
    /** HDR display preference mode
     * 'auto' - Detect display capability (defaults to SDR if detection fails)
     * 'hdr' - Always try to display HDR content
     * 'sdr' - Always display SDR content (apply gainmaps for HDR->SDR tone mapping)
     * @type {string} */
    preferHDR: 'sdr',  // Default to SDR for web compatibility
    
    /** Whether to apply gainmaps when available
     * true - Apply gainmaps for tone mapping when conversion is needed
     * false - Display base image without tone mapping
     * @type {boolean} */
    applyGainmap: true,
    
    /** Gainmap weight for tone mapping (0 to 1, per ISO 21496-1)
     * 0.0 = Output base image (no conversion)
     * 1.0 = Full conversion to alternate representation
     * 'auto' = Determine based on base image type and preferHDR setting
     * @type {number|string} */
    gainmapWeight: 'auto',
    
    /** Check if HDR display is supported by the environment
     * @returns {boolean} True if HDR display is likely supported */
    isHDRSupported: function() {
        // Check for HDR display capability
        // This is a basic check - more sophisticated detection would use
        // matchMedia for color-gamut and dynamic-range queries
        if (typeof window !== 'undefined' && window.matchMedia) {
            // Check for wide color gamut (P3 or rec2020)
            var wideGamut = window.matchMedia('(color-gamut: p3)').matches ||
                           window.matchMedia('(color-gamut: rec2020)').matches;
            // Check for HDR dynamic range
            var hdrCapable = window.matchMedia('(dynamic-range: high)').matches;
            return wideGamut && hdrCapable;
        }
        return false;
    },
    
};
// Note: getEffectiveGainmapWeight() was removed in the HDR cleanup.
// _postProcessGainmaps now computes the weight directly from
// preferHDR + isHDRSupported() + the base image type, so the helper
// had no remaining callers.

// Expose globally
window.HEIFDisplayConfig = HEIFDisplayConfig;

/** Utility for logging HEIF file information to console.
 *
 *  Off by default. When OFF the logger is a near-no-op: it emits a
 *  single coloured one-line summary per file (so casual users opening
 *  the console still see what happened) and stores nothing else, so
 *  there is no measurable overhead on mobile / headless / production
 *  visitors who never touch DevTools.
 *
 *  When ON it groups, dumps the full `heifInfo` blob, decoded-item
 *  table, colour info, gainmap relationships, etc.
 *
 *  Power-user toggle (DevTools console only — there is intentionally
 *  no URL parameter or UI control):
 *
 *      HEIFFileLogger.verbose = true   // turn on, persists across reloads
 *      HEIFFileLogger.verbose = false  // turn off
 *
 *  The flag is persisted in `localStorage` under
 *  `HEIFFileLogger.verbose`, so it survives reloads on the same
 *  origin. Remember to turn it back off when you're done — verbose
 *  mode serialises the full heifInfo to the console panel for every
 *  decoded image, which is a real performance hit on large galleries.
 *
 *  The `console.debug` lines inside the verbose body are themselves
 *  only displayed when the DevTools log level includes "Verbose"
 *  (Chrome/Firefox: Console → Log level dropdown → check "Verbose").
 *  The one-line summary is `console.log` and always shows.
 *  @namespace */
var HEIFFileLogger = (function () {
    var LS_KEY = 'HEIFFileLogger.verbose';
    var initial = false;
    try { initial = window.localStorage.getItem(LS_KEY) === '1'; }
    catch (e) { /* private mode / no storage */ }

    var ns = {
        /** Derive display label (basename) from a full URL / path. */
        _label: function (filename) {
            if (!filename) return 'unknown';
            var slash = filename.lastIndexOf('/');
            return slash === -1 ? filename : filename.substring(slash + 1);
        }
    };

    var _verbose = initial;
    Object.defineProperty(ns, 'verbose', {
        enumerable: true,
        get: function () { return _verbose; },
        set: function (v) {
            _verbose = !!v;
            try { window.localStorage.setItem(LS_KEY, _verbose ? '1' : '0'); }
            catch (e) { /* ignore */ }
            // Drop any heavy stash that was kept "just in case"; the
            // next decode will either re-stash (if turned on) or stay
            // lean (if turned off).
            if (!_verbose) ns._pending = {};
        }
    });

    /** Log HEIF file information to console
     * @param {Object} options Logging options
     * @param {string} options.filename The filename or URL (used verbatim as key — full URL avoids basename collisions)
     * @param {Object} options.heifInfo The heifInfo object from _buildHeifInfo
     * @param {Object} [options.decodedFrames] Optional map of decoded frame data (itemId -> frameData)
     * @param {Object} [options.propertiesMap] Optional map of item properties
     * @param {Object} [options.alphaMaskMap] Optional map of alpha masks
     * @param {Object} [options.gainmapMap] Optional map of gainmaps
     * @param {number} [options.displayFrameCount] Optional count of display frames (vs total decoded)
     */
    ns.logFileInfo = function (options) {
        var key = options.filename || 'unknown';
        ns._pending = ns._pending || {};
        if (!_verbose) {
            // Lean stash: enough to print the summary line at decode
            // completion, but no references to heifInfo / decodedFrames
            // / property tables. This is what mobile + production
            // visitors hit, and the cost must stay near zero.
            ns._pending[key] = {
                _lean: true,
                alphaMaskMap: options.alphaMaskMap,  // small object, used for counts only
                gainmapMap:   options.gainmapMap
            };
            return;
        }
        ns._pending[key] = options;
    };

    /** Log decode completion status — also flushes the stashed file info
     *  and color info into one consolidated group per file.
     * @param {Object} options Completion options
     * @param {string} options.filename The filename
     * @param {boolean} options.success Whether decode succeeded
     * @param {number} options.decodedFrameCount Number of frames successfully decoded
     * @param {number} options.expectedFrameCount Expected frame count
     * @param {Array<number>} [options.decodedItemIds] IDs of decoded items
     * @param {string} [options.error] Error message if failed
     */
    ns.logDecodeComplete = function (options) {
        var key = options.filename || 'unknown';
        var filename = ns._label(key);

        // Retrieve stashed file info
        var pending = (ns._pending || {})[key];
        var heifInfo = pending ? pending.heifInfo : null;
        var decodedFrames = pending ? (pending.decodedFrames || {}) : {};
        var propertiesMap = pending ? (pending.propertiesMap || {}) : {};
        var alphaMaskMap = pending ? (pending.alphaMaskMap || {}) : {};
        var gainmapMap = pending ? (pending.gainmapMap || {}) : {};
        var displayFrameCount = pending ? pending.displayFrameCount : undefined;

        // Build one-line summary
        var parts = [];
        if (options.success) {
            parts.push(options.decodedFrameCount + '/' + options.expectedFrameCount + ' frames');
        } else if (options.noDecoder) {
            parts.push('no decoder (metadata only)');
        } else {
            parts.push('FAILED ' + options.decodedFrameCount + '/' + options.expectedFrameCount);
        }
        var alphaCount = Object.keys(alphaMaskMap).length;
        var gainmapCount = Object.keys(gainmapMap).length;
        if (alphaCount > 0) parts.push(alphaCount + ' alpha');
        if (gainmapCount > 0) parts.push(gainmapCount + ' gainmap');

        // Color info stashed by logColorInfo (per-file when possible,
        // falls back to the legacy global slot for older callers).
        var colorStash = (pending && pending.colorInfo) ? pending.colorInfo : logColorInfo._last;
        logColorInfo._last = null;

        var label = '%c[HEIF] ' + filename + ' — ' + parts.join(', ');
        var style = options.success
            ? 'color:#4caf50; font-weight:bold'
            : options.noDecoder
            ? 'color:#ff9800; font-weight:bold'
            : 'color:#f44336; font-weight:bold';

        if (_verbose) {
            console.group(label, style);
        } else {
            // Compact mode: one-line summary only, no group body.
            // console.debug inside a collapsed group is still expensive —
            // Chrome serialises every object for the console panel even
            // when the group is never expanded.
            console.log(label, style);
        }

        if (!options.success && options.error) {
            console.warn(options.error);
        }

        if (!_verbose) {
            // Clean up stash
            if (ns._pending) delete ns._pending[key];
            return;
        }

        // --- verbose body below ---

        // Decoded items table
        var decodedCount = Object.keys(decodedFrames).length;
        if (decodedCount > 0) {
            var decodedTable = {};
            for (var frameItemId in decodedFrames) {
                if (!decodedFrames.hasOwnProperty(frameItemId)) continue;
                var decodedFrame = decodedFrames[frameItemId];
                var props = propertiesMap[frameItemId] || [];
                var numericItemId = parseInt(frameItemId);
                var itemType = 'Master';
                for (var baseId in alphaMaskMap) {
                    if (alphaMaskMap[baseId] === numericItemId) { itemType = 'Alpha'; break; }
                }
                if (itemType === 'Master') {
                    for (var gmBaseId in gainmapMap) {
                        if (gainmapMap[gmBaseId] && gainmapMap[gmBaseId].gainmapItemId === numericItemId) { itemType = 'Gainmap'; break; }
                    }
                }
                var codec = '-';
                var propList = [];
                var raw = (props && props.raw) ? props.raw : [];
                for (var i = 0; i < raw.length; i++) {
                    var prop = raw[i];
                    propList.push(prop.type + (prop.essential ? '*' : ''));
                    switch (prop.type) {
                        case 'hvcC': codec = 'HEVC'; break;
                        case 'vvcC': codec = 'VVC';  break;
                        case 'av1C': codec = 'AV1';  break;
                        case 'avcC': codec = 'AVC';  break;
                        case 'jpgC': codec = 'JPEG'; break;
                        case 'auxC': itemType = 'Aux'; break;
                    }
                }
                decodedTable['Item ' + frameItemId] = {
                    'Type': itemType,
                    'Size': decodedFrame.width + 'x' + decodedFrame.height,
                    'Bits': decodedFrame.bitDepth || '-',
                    'Codec': codec,
                    'Props': propList.join(', ') || '-'
                };
            }
            if (Object.keys(decodedTable).length > 0) {
                console.debug('Decoded items:', decodedTable);
            }
        }

        // Color info (if captured during this decode)
        if (colorStash) {
            console.debug('Color:', colorStash.info);
        }

        // File / items / metabox / tracks
        if (heifInfo) {
            if (heifInfo.file) {
                console.debug('File:', heifInfo.file.majorBrand, '| Brands:', (heifInfo.file.compatibleBrands || []).join(', '));
            }
            if (heifInfo.items && Object.keys(heifInfo.items).length > 0) {
                var allItemsTable = {};
                for (var itemId in heifInfo.items) {
                    if (!heifInfo.items.hasOwnProperty(itemId)) continue;
                    var item = heifInfo.items[itemId];
                    var flags = [];
                    if (item.features) {
                        if (item.features.isMasterImage) flags.push('Master');
                        if (item.features.isThumbnailImage) flags.push('Thumb');
                        if (item.features.isAuxiliaryImage) flags.push('Aux');
                        if (item.features.isDerivedImage) flags.push('Derived');
                    }
                    allItemsTable['Item ' + itemId] = {
                        'Type': item.type + (flags.length ? ' [' + flags.join(',') + ']' : ''),
                        'Size': item.width + 'x' + item.height,
                        'Codec': item.codec || '-',
                        'Bytes': item.size || '-'
                    };
                }
                console.debug('All Items (' + Object.keys(heifInfo.items).length + '):', allItemsTable);
            }
            if (heifInfo.metaBox) {
                console.debug('MetaBox: ' + heifInfo.metaBox.itemCount + ' items (' +
                    heifInfo.metaBox.masterImageCount + ' master, ' +
                    heifInfo.metaBox.thumbnailCount + ' thumb, ' +
                    heifInfo.metaBox.auxiliaryCount + ' aux, ' +
                    heifInfo.metaBox.derivedImageCount + ' derived)');
            }
            if (heifInfo.tracks && heifInfo.tracks.length > 0) {
                var trackSummary = heifInfo.tracks.map(function(t) {
                    var features = [];
                    if (t.trackFeature) {
                        if (t.trackFeature.isMasterImageSequence) features.push('master');
                        if (t.trackFeature.isThumbnailImageSequence) features.push('thumb');
                        if (t.trackFeature.isAuxiliaryImageSequence) features.push('aux');
                    }
                    return 'Track ' + t.trackId + ' [' + features.join(',') + '] ' + (t.sampleCount || 0) + ' samples';
                });
                console.debug('Tracks:', trackSummary.join('; '));
            }
            // Gainmap relationships
            if (gainmapCount > 0) {
                var gmSummary = [];
                for (var gmBase in gainmapMap) {
                    if (!gainmapMap.hasOwnProperty(gmBase)) continue;
                    var gm = gainmapMap[gmBase];
                    gmSummary.push('base ' + gmBase + ' → gainmap ' + gm.gainmapItemId);
                }
                console.debug('Gainmaps: ' + gmSummary.join('; '));
            }

            // Gainmap processing result
            var gmResult = pending ? pending.gainmapResult : null;
            if (gmResult) {
                if (gmResult.applied > 0) {
                    console.debug('Gainmap processing: applied (weight=' + gmResult.weight.toFixed(2) +
                               ', base=' + (gmResult.isBaseHDR ? 'HDR' : 'SDR') + ')');
                } else if (gmResult.skipped > 0) {
                    console.debug('Gainmap processing: skipped (' + (gmResult.reason || 'weight=0') +
                               ', base=' + (gmResult.isBaseHDR ? 'HDR' : 'SDR') + ')');
                }
            }

            console.debug('Full heifInfo:', heifInfo);
        }

        console.groupEnd();

        // Clean up stash
        if (ns._pending) delete ns._pending[key];
    };

    // One-time hint for power users opening DevTools. console.info
    // shows at default log level but is unobtrusive (small "i" icon
    // in Chrome). Casual visitors who never open DevTools never see
    // it, and mobile users never pay for it.
    if (typeof console !== 'undefined' && console.info) {
        console.info(
            '%c[HEIF]%c verbose decode logging available: %cHEIFFileLogger.verbose = true%c ' +
            '(persists across reloads; remember to set back to false — verbose mode ' +
            'serialises full heifInfo per file and noticeably slows large galleries).',
            'color:#4caf50; font-weight:bold',
            '',
            'font-family:monospace; background:#eee; color:#222; padding:0 4px',
            ''
        );
    }

    return ns;
})();

// Expose globally
window.HEIFFileLogger = HEIFFileLogger;

/** Manages decoding requests and canvas operations independently of decoder implementation
 *  @constructor
 *  @param {string} decoderType Type of decoder to use ('libde265', 'vvdec', etc.) */
function DecoderManager(decoderType) {
    var self = this;
    
    this._activeRequest = false;
    this._currentContext = null;
    this._contextQueue = [];
    this._imageData = null;
    this._decodedFrames = {};
    this.canvas = null;
    this.ctx = null;
    this.decoderType = decoderType || 'libde265'; //defaulting to libde265 in case of no decodertype

    /** Add a request to the processing queue */
    this.enqueueRequest = function(context) {
        self._contextQueue.push(context);
    };

    /** Process next request if available */
    this.processNext = function() {
        if(self._activeRequest || self._contextQueue.length === 0){
            return;
        }
        self._currentContext = self._contextQueue.shift();
        self._activeRequest = self._currentContext;
        self._processStream(self._currentContext.stream);
    };

    /** Initialize canvas for decoding */
    this._setupCanvas = function(parent) {
        // Check if decoder canvas already exists
        var existingCanvas = document.getElementById('decoder-canvas');
        if(existingCanvas){
            self.canvas = existingCanvas;
            // Recreate context for reused canvas
            self.ctx = self.canvas.getContext("2d");
            return;
        }
        self.canvas = document.createElement("canvas");
        self.canvas.id = "decoder-canvas";
        self.canvas.style.display = "none";
        parent.appendChild(self.canvas);
        self.ctx = self.canvas.getContext("2d");
    };

    /** Handle decoded image data.
     *
     * @param {number} width
     * @param {number} height
     * @param {Uint8ClampedArray|Uint8Array} pixelData - 8-bit RGBA. Note: this
     *   is ALWAYS 8-bit per channel regardless of the source bit depth -
     *   the libde265/vvdec extract paths read 10/12-bit YUV via HEAPU16
     *   and reduce to 8-bit (round-half-up) immediately, because canvas-2D
     *   ImageData only supports Uint8ClampedArray. There is no 10/12-bit
     *   pipeline anywhere downstream of this entry point.
     * @param {number} bitDepth - INFORMATIONAL ONLY. Reflects the SOURCE
     *   stream bit depth (8/10/12). Does NOT describe `pixelData`. Used
     *   to populate the 'Bits' badge in the modal and to drive HDR
     *   detection heuristics. Do not derive buffer math from this value.
     */
    this.handleDecodedImage = function(width, height, pixelData, bitDepth) {
        if (!self._currentContext){
            return;
        }

        var context = self._currentContext;
        var itemId = context.itemIds[context.currentFrameIndex];
        var result = context.payload;

        // Store RGBA data
        var rgba = (pixelData instanceof Uint8ClampedArray) ? pixelData : new Uint8ClampedArray(pixelData);

        // Store decoded frame data
        self._decodedFrames[itemId] = {
            width: width,
            height: height,
            data: rgba,
            bitDepth: bitDepth
        };

        result.displayHeight = Math.max(result.displayHeight || 0, height);
        result.displayWidth = Math.max(result.displayWidth || 0, width);

        // Only add to display frames if NOT a dependency
        if (!(itemId in context.dependencies)) {

            // Extract all transformative properties from the item
            // Order matters per HEIF spec: LSEL -> CLAP -> ISCL -> IROT -> IMIR
            var transformations = {
                layerSelection: null,  // LSEL - layer selection (placeholder)
                cleanAperture: null,   // CLAP - clean aperture (crop)
                scale: null,           // ISCL - scaling
                rotation: 0,           // IROT - rotation (0, 90, 180, 270)
                mirror: null           // IMIR - mirroring
            };
            
            if (context.propertiesMap && context.propertiesMap[itemId]) {
                var p = context.propertiesMap[itemId];

                // Image rotation — typed accessor returns degrees directly
                // (0/90/180/270, counter-clockwise per ISO/IEC 23008-12).
                if (typeof p.irot === 'number') {
                    transformations.rotation = p.irot;
                }

                // Image mirror — typed accessor returns {axis: 'horizontal'
                // | 'vertical'}. The renderer below consumes a
                // `horizontalAxis` boolean: true means horizontal-axis flip
                // (top↔bottom, i.e. axis=0 in the on-wire encoding).
                if (p.imir) {
                    transformations.mirror = {
                        horizontalAxis: (p.imir.axis === 'horizontal')
                    };
                }

                // §15: clap / iscl / lsel typed accessors
                // (heif_reader_get_property_clap / _iscl / _lsel) are not
                // yet bound on this binding. Their presence is visible via
                // p.raw[].type but the payload (crop rect, scale ratios,
                // layer id) is not reachable. Leave the corresponding
                // transformations.* fields at their initial null defaults
                // — the renderer treats null as "no transform".
            }
            
            result.frames.push({
                canvasFrameData: rgba,
                itemId: itemId,
                width: width,
                height: height,
                // Legacy fields for backward compatibility
                rotation: transformations.rotation,
                mirror: transformations.mirror,
                // New comprehensive transformations object
                transformations: transformations,
                bitDepth: bitDepth
            });
        }

        context.currentFrameIndex++;

        if (context.currentFrameIndex >= context.itemIds.length) {
            // Post-process frames with alpha masks
            self._postProcessAlphaMasks(context);
            
            // Post-process frames with gainmaps (HDR/SDR tone mapping)
            self._postProcessGainmaps(context);

            // Log decode completion status (file info already logged in image-provider)
            // This logs supplementary decode results - frame count, which frames decoded
            if (window.HEIFFileLogger && window.HEIFFileLogger.logDecodeComplete) {
                window.HEIFFileLogger.logDecodeComplete({
                    filename: context.filename,
                    success: true,
                    decodedFrameCount: result.frames.length,
                    expectedFrameCount: context.itemIds.length,
                    decodedItemIds: Object.keys(self._decodedFrames).map(Number)
                });
            }
            
            context.callback(result);
            
            // Clear cache. _currentContext is nulled so a subsequent
            // _finalizePartialDecode (defensive call from the adapter)
            // becomes a no-op.
            self._currentContext = null;
            self._decodedFrames = {};
            self._activeRequest = false;
            self.processNext();
        }
    };


    /** Apply alpha mask to base image RGBA data (in-place)
     * @note Alpha layers are either YUV400 monochrome or YUV420, 
     *       but the opacity value is always the Y channel value (0-255 or limited) per pixel.
     *       Mutates baseRGBA directly — the caller replaces its reference
     *       so the old buffer is never retained.
     * @param {Uint8ClampedArray} baseRGBA Base image RGBA data (modified in place)
     * @param {Uint8ClampedArray} maskRGBA Alpha mask RGBA data (R channel used as alpha)
     * @param {number} width Image width
     * @param {number} height Image height
     * @returns {Uint8ClampedArray} baseRGBA with alpha channel stamped */
    this.applyAlphaMask = function(baseRGBA, maskRGBA, width, height) {
        var pixelCount = width * height;

        // Stamp alpha from mask's R channel into the base buffer.
        // Use Uint32Array view to write 4 bytes at a time — we read the
        // existing RGB from the base and the alpha from the mask's R
        // channel, compositing via bitwise ops.  On little-endian (all
        // wasm targets) the layout is [R, G, B, A] → 0xAABBGGRR.
        var base32 = new Uint32Array(baseRGBA.buffer, baseRGBA.byteOffset, pixelCount);
        var mask32 = new Uint32Array(maskRGBA.buffer, maskRGBA.byteOffset, pixelCount);
        for (var i = 0; i < pixelCount; i++) {
            // Keep base RGB, replace alpha with mask's R (low byte)
            base32[i] = (base32[i] & 0x00FFFFFF) | ((mask32[i] & 0xFF) << 24);
        }

        return baseRGBA;
    };

    /** Post-process frames to apply alpha masks */
    this._postProcessAlphaMasks = function(context) {

        //function called for possible non-alpha items, so filter them out
        if (!context.alphaMaskMap || Object.keys(context.alphaMaskMap).length === 0) {
            return;
        }

        var result = context.payload;

        for (var i = 0; i < result.frames.length; i++) {
            var frame = result.frames[i];
            var maskItemId = context.alphaMaskMap[frame.itemId];

            if (maskItemId && self._decodedFrames[maskItemId]) {
                var maskFrame = self._decodedFrames[maskItemId];

                // NOTE: dimension mismatch is possible when the base has CLAP
                // cropping and the mask doesn't (or vice versa). The guard
                // silently skips the mask in that case — acceptable for now.
                if (maskFrame.width === frame.width && maskFrame.height === frame.height) {
                    frame.canvasFrameData = self.applyAlphaMask(
                        frame.canvasFrameData,
                        maskFrame.data,
                        frame.width,
                        frame.height
                    );
                }

                // Release mask buffer as soon as it's applied
                delete self._decodedFrames[maskItemId];
            }
        }
    };

    /** Apply gainmap tone mapping for HDR/SDR conversion (ISO 21496-1 compliant)
     * 
     * The gain map represents the ratio between HDR and SDR representations.
     * Formula per ISO 21496-1:
     *   output = (base + baseOffset) * 2^(logGain * w) - alternateOffset
     * 
     * Where:
     * - base: Base image in LINEAR light space
     * - logGain: log2 gain value mapped from stored gainmap
     * - w: weight factor (0 = base output, 1 = alternate output)
     * 
     * Critical: Gain map math MUST be performed in LINEAR light space!
     * 
     * IMPORTANT: This function REQUIRES proper metadata from the HEIF API.
     * Do NOT call this function without valid gainMapMin/gainMapMax values.
     * Conforms to ISO 21496-1 gainmap application formula.
     * Defaults: gamma=1.0, baseOffset=1/64, alternateOffset=1/64,
     * baseTransfer=16 (PQ), outputTransfer=13 (sRGB).
     * @param {Uint8ClampedArray} baseRGBA Base image RGBA data (gamma-encoded)
     * @param {Uint8ClampedArray} gainmapRGBA Gainmap RGBA data
     * @param {number} width Image width
     * @param {number} height Image height
     * @param {number} weight Tone mapping weight (0-1): 0=base, 1=alternate (SDR)
     * @param {Object} metadata Gainmap metadata (REQUIRED) with:
     *   - gainMapMin: min log2 gain values (number or [R,G,B] array)
     *   - gainMapMax: max log2 gain values (number or [R,G,B] array)
     *   - gamma: (optional) gainmap encoding gamma, default 1.0
     *   - baseOffset: (optional) offset for base, default 1/64
     *   - alternateOffset: (optional) offset for alternate, default 1/64
     *   - baseTransfer: (optional) ITU-T H.273 transfer code for base, default 16 (PQ)
     *   - outputTransfer: (optional) ITU-T H.273 transfer code for output, default 13 (sRGB)
     * @returns {Uint8ClampedArray} Tone-mapped RGBA data */
    this.applyGainmap = function(baseRGBA, gainmapRGBA, width, height, weight, metadata) {
        // Validate required metadata
        if (!metadata || metadata.gainMapMin === undefined || metadata.gainMapMax === undefined) {
            console.error('applyGainmap: Required metadata (gainMapMin/gainMapMax) is missing');
            return baseRGBA; // Return unchanged base image
        }
        
        var result = new Uint8ClampedArray(width * height * 4);
        
        // Extract per-channel values from metadata (required)
        var gainMapMinR = Array.isArray(metadata.gainMapMin) ? metadata.gainMapMin[0] : metadata.gainMapMin;
        var gainMapMinG = Array.isArray(metadata.gainMapMin) && metadata.gainMapMin.length > 1 ? 
            metadata.gainMapMin[1] : gainMapMinR;
        var gainMapMinB = Array.isArray(metadata.gainMapMin) && metadata.gainMapMin.length > 2 ? 
            metadata.gainMapMin[2] : gainMapMinR;
            
        var gainMapMaxR = Array.isArray(metadata.gainMapMax) ? metadata.gainMapMax[0] : metadata.gainMapMax;
        var gainMapMaxG = Array.isArray(metadata.gainMapMax) && metadata.gainMapMax.length > 1 ? 
            metadata.gainMapMax[1] : gainMapMaxR;
        var gainMapMaxB = Array.isArray(metadata.gainMapMax) && metadata.gainMapMax.length > 2 ? 
            metadata.gainMapMax[2] : gainMapMaxR;
        
        // Optional parameters with ISO 21496-1 defaults
        var gammaR = metadata.gamma !== undefined ? 
            (Array.isArray(metadata.gamma) ? metadata.gamma[0] : metadata.gamma) : 1.0;
        var gammaG = metadata.gamma !== undefined && Array.isArray(metadata.gamma) && metadata.gamma.length > 1 ? 
            metadata.gamma[1] : gammaR;
        var gammaB = metadata.gamma !== undefined && Array.isArray(metadata.gamma) && metadata.gamma.length > 2 ? 
            metadata.gamma[2] : gammaR;
        
        var baseOffsetR = metadata.baseOffset !== undefined ? 
            (Array.isArray(metadata.baseOffset) ? metadata.baseOffset[0] : metadata.baseOffset) : 1/64;
        var baseOffsetG = metadata.baseOffset !== undefined && Array.isArray(metadata.baseOffset) && metadata.baseOffset.length > 1 ? 
            metadata.baseOffset[1] : baseOffsetR;
        var baseOffsetB = metadata.baseOffset !== undefined && Array.isArray(metadata.baseOffset) && metadata.baseOffset.length > 2 ? 
            metadata.baseOffset[2] : baseOffsetR;
            
        var altOffsetR = metadata.alternateOffset !== undefined ? 
            (Array.isArray(metadata.alternateOffset) ? metadata.alternateOffset[0] : metadata.alternateOffset) : 1/64;
        var altOffsetG = metadata.alternateOffset !== undefined && Array.isArray(metadata.alternateOffset) && metadata.alternateOffset.length > 1 ? 
            metadata.alternateOffset[1] : altOffsetR;
        var altOffsetB = metadata.alternateOffset !== undefined && Array.isArray(metadata.alternateOffset) && metadata.alternateOffset.length > 2 ? 
            metadata.alternateOffset[2] : altOffsetR;
        
        // Get transfer characteristics for proper linearization
        var baseTransfer = metadata.baseTransfer !== undefined ? metadata.baseTransfer : 16; // Default PQ for HDR
        var outputTransfer = metadata.outputTransfer !== undefined ? metadata.outputTransfer : 13; // Default sRGB for SDR
        
        // Clamp weight to valid range [0, 1] per ISO 21496-1
        // weight=0 means output equals base, weight=1 means output equals alternate (SDR)
        weight = Math.max(0.0, Math.min(1.0, weight));
        
        // Check if gainmap is single-channel (grayscale) or multi-channel
        var isGrayscale = true;
        for (var i = 0; i < Math.min(100, width * height); i++) {
            var idx = i * 4;
            if (gainmapRGBA[idx] !== gainmapRGBA[idx + 1] || 
                gainmapRGBA[idx + 1] !== gainmapRGBA[idx + 2]) {
                isGrayscale = false;
                break;
            }
        }
        
        // Hoist transfer-function dispatch outside the inner loop. The
        // previous version did `switch(transferCharacteristics){...}`
        // 6 times per pixel (3 channels into linear, 3 channels back
        // out), which on a 24 MP image is ~144 million switch dispatches
        // and a measurable share of the total runtime. Resolving each
        // direction to a direct function reference once here lets the
        // loop call it without the dispatch overhead and lets V8 inline
        // the small (<10-line) PQ/HLG/sRGB helpers more reliably.
        // Behaviour is unchanged: every fallback case in transferToLinear
        // /linearToTransfer is preserved via the wrapper below.
        var resolveToLinear = function(t) {
            switch (t) {
                case 16: return pqToLinear;
                case 18: return hlgToLinear;
                case 13: case 1: case 6: return srgbToLinear;
                default: return function(v) { return Math.pow(v, 2.2); };
            }
        };
        var resolveFromLinear = function(t) {
            switch (t) {
                case 16: return linearToPQ;
                case 18: return linearToHLG;
                case 13: case 1: case 6: return linearToSRGB;
                default: return function(v) { return Math.pow(v < 0 ? 0 : v, 1/2.2); };
            }
        };
        var toLinear = resolveToLinear(baseTransfer);
        var fromLinear = resolveFromLinear(outputTransfer);

        for (var i = 0; i < width * height; i++) {
            var idx = i * 4;
            
            // Get base RGB values normalized to 0-1
            var baseR = baseRGBA[idx + 0] / 255.0;
            var baseG = baseRGBA[idx + 1] / 255.0;
            var baseB = baseRGBA[idx + 2] / 255.0;
            
            // Convert base image from gamma-encoded to LINEAR light
            var baseLinR = toLinear(baseR);
            var baseLinG = toLinear(baseG);
            var baseLinB = toLinear(baseB);
            
            // Get gainmap values normalized to 0-1
            var gainR = gainmapRGBA[idx + 0] / 255.0;
            var gainG = isGrayscale ? gainR : gainmapRGBA[idx + 1] / 255.0;
            var gainB = isGrayscale ? gainR : gainmapRGBA[idx + 2] / 255.0;
            
            // Apply gamma to gainmap values (undo gainmap encoding gamma)
            if (gammaR !== 1.0) gainR = Math.pow(gainR, gammaR);
            if (gammaG !== 1.0) gainG = Math.pow(gainG, gammaG);
            if (gammaB !== 1.0) gainB = Math.pow(gainB, gammaB);
            
            // Map gainmap values from [0,1] to [gainMapMin, gainMapMax] (log2 domain)
            var logGainR = gainMapMinR + gainR * (gainMapMaxR - gainMapMinR);
            var logGainG = gainMapMinG + gainG * (gainMapMaxG - gainMapMinG);
            var logGainB = gainMapMinB + gainB * (gainMapMaxB - gainMapMinB);
            
            // Calculate the multiplier: 2^(logGain * weight)
            // Per ISO 21496-1: output = (base + baseOffset) * 2^(logGain * w) - altOffset
            var multR = Math.pow(2, logGainR * weight);
            var multG = Math.pow(2, logGainG * weight);
            var multB = Math.pow(2, logGainB * weight);
            
            // Apply gain map formula in LINEAR space
            var outLinR = (baseLinR + baseOffsetR) * multR - altOffsetR;
            var outLinG = (baseLinG + baseOffsetG) * multG - altOffsetG;
            var outLinB = (baseLinB + baseOffsetB) * multB - altOffsetB;
            
            // Clamp negative values (can occur with offsets)
            outLinR = Math.max(0, outLinR);
            outLinG = Math.max(0, outLinG);
            outLinB = Math.max(0, outLinB);
            
            // Convert from LINEAR back to output transfer function
            // (transfer fn resolved once before the loop - see hoisting above)
            var outR = fromLinear(outLinR);
            var outG = fromLinear(outLinG);
            var outB = fromLinear(outLinB);
            
            // Clamp and convert back to 0-255
            result[idx + 0] = Math.max(0, Math.min(255, Math.round(outR * 255)));
            result[idx + 1] = Math.max(0, Math.min(255, Math.round(outG * 255)));
            result[idx + 2] = Math.max(0, Math.min(255, Math.round(outB * 255)));
            result[idx + 3] = baseRGBA[idx + 3]; // Preserve alpha from base
        }
        
        return result;
    };

    /** Post-process frames to apply gainmap tone mapping */
    this._postProcessGainmaps = function(context) {
        // Filter out contexts without gainmaps
        if (!context.gainmapMap || Object.keys(context.gainmapMap).length === 0) {
            return;
        }
        
        // Check global configuration
        if (!window.HEIFDisplayConfig || !window.HEIFDisplayConfig.applyGainmap) {
            return;
        }

        var result = context.payload;

        for (var i = 0; i < result.frames.length; i++) {
            var frame = result.frames[i];
            var gainmapInfo = context.gainmapMap[frame.itemId];

            if (gainmapInfo && gainmapInfo.gainmapItemId && self._decodedFrames[gainmapInfo.gainmapItemId]) {
                var gainmapFrame = self._decodedFrames[gainmapInfo.gainmapItemId];

                // Get gainmap metadata - MUST come from gainmapInfo (via HEIF API)
                // Do NOT use hardcoded defaults - if metadata is missing, warn and skip
                var gainmapMetadata = gainmapInfo.metadata || {};
                
                // Check if we have the required gainmap parameters
                var hasRequiredMetadata = (
                    gainmapMetadata.gainMapMin !== undefined &&
                    gainmapMetadata.gainMapMax !== undefined
                );
                
                if (!hasRequiredMetadata) {
                    console.warn('Gainmap detected for item ' + frame.itemId + 
                               ' but required metadata (gainMapMin/Max) is missing. ' +
                               'Cannot apply gainmap without proper parameters. ' +
                               'Displaying base image as-is.');
                    frame.hasGainmap = true;
                    frame.gainmapApplied = 0;
                    frame.gainmapSkippedReason = 'missingMetadata';
                    delete self._decodedFrames[gainmapInfo.gainmapItemId];
                    continue;
                }
                
                // Determine if base image is HDR or SDR based on COLR properties
                var isBaseHDR = self._isImageHDR(frame.itemId, context.propertiesMap);
                frame.isHDR = isBaseHDR;
                
                // Get transfer characteristics for the base image
                var baseTransfer = getTransferCharacteristics(frame.itemId, context.propertiesMap);
                gainmapMetadata.baseTransfer = baseTransfer;
                
                // Determine output transfer based on display preference
                // If we want SDR output, use sRGB (13); for HDR, keep the base transfer
                var wantSDR = window.HEIFDisplayConfig.preferHDR === 'sdr' || 
                             (window.HEIFDisplayConfig.preferHDR === 'auto' && !window.HEIFDisplayConfig.isHDRSupported());
                gainmapMetadata.outputTransfer = wantSDR ? 13 : baseTransfer; // 13 = sRGB
                
                // Check dimensions - gainmap may be lower resolution
                var gainmapWidth = gainmapFrame.width;
                var gainmapHeight = gainmapFrame.height;
                var baseWidth = frame.width;
                var baseHeight = frame.height;
                
                // If gainmap is different resolution, we need to scale it
                var gainmapData = gainmapFrame.data;
                if (gainmapWidth !== baseWidth || gainmapHeight !== baseHeight) {
                    gainmapData = self._scaleGainmap(gainmapFrame.data, gainmapWidth, gainmapHeight, baseWidth, baseHeight);
                }

                // Calculate weight based on configuration
                // Per ISO 21496-1: weight=0 means base image, weight=1 means alternate (SDR)
                var weight;
                if (isBaseHDR && wantSDR) {
                    // HDR base, want SDR output: apply full gainmap (weight=1)
                    weight = 1.0;
                } else if (isBaseHDR && !wantSDR) {
                    // HDR base, want HDR output: no conversion needed (weight=0)
                    weight = 0.0;
                } else if (!isBaseHDR && wantSDR) {
                    // SDR base, want SDR: no conversion (weight=0)
                    weight = 0.0;
                } else {
                    // SDR base, want HDR: would need inverse gainmap (not typically available)
                    // Just output base for now
                    weight = 0.0;
                }
                
                // Override with explicit config if set
                if (typeof window.HEIFDisplayConfig.gainmapWeight === 'number') {
                    weight = Math.max(0, Math.min(1, window.HEIFDisplayConfig.gainmapWeight));
                }
                
                // Skip if weight is 0 (no conversion needed)
                if (weight === 0) {
                    frame.hasGainmap = true;
                    frame.gainmapApplied = 0;
                    delete self._decodedFrames[gainmapInfo.gainmapItemId];
                    continue;
                }
                
                // Mark frame as having gainmap applied
                frame.hasGainmap = true;
                frame.gainmapApplied = weight;
                
                frame.canvasFrameData = self.applyGainmap(
                    frame.canvasFrameData,
                    gainmapData,
                    frame.width,
                    frame.height,
                    weight,
                    gainmapMetadata
                );

                // Release gainmap buffer after it's applied
                delete self._decodedFrames[gainmapInfo.gainmapItemId];
            }
        }

        // Stash processing summary for the file logger. Key by the
        // full URL (matches the verbose stash); skip when the logger
        // is in compact mode (lean stash has no gainmapResult slot
        // anyway, and verbose=false won't print it).
        var key = context.filename || 'unknown';
        var pending = (HEIFFileLogger._pending || {})[key];
        if (pending && !pending._lean) {
            var applied = result.frames.filter(function(f) { return f.gainmapApplied > 0; });
            var skipped = result.frames.filter(function(f) { return f.hasGainmap && !f.gainmapApplied; });
            pending.gainmapResult = {
                applied: applied.length,
                skipped: skipped.length,
                weight: applied.length > 0 ? applied[0].gainmapApplied : 0,
                isBaseHDR: applied.length > 0 ? applied[0].isHDR : (skipped.length > 0 ? skipped[0].isHDR : null),
                reason: skipped.length > 0 ? (skipped[0].gainmapSkippedReason || 'weight=0') : null
            };
        }
    };

    /** Scale gainmap to match base image dimensions using GPU-accelerated
     * canvas drawImage instead of manual per-pixel bilinear interpolation.
     * @param {Uint8ClampedArray} srcData Source gainmap RGBA data
     * @param {number} srcWidth Source width
     * @param {number} srcHeight Source height
     * @param {number} dstWidth Target width
     * @param {number} dstHeight Target height
     * @returns {Uint8ClampedArray} Scaled gainmap RGBA data */
    this._scaleGainmap = function(srcData, srcWidth, srcHeight, dstWidth, dstHeight) {
        // Use an off-screen canvas pair: paint source pixels onto a
        // srcWidth×srcHeight canvas, then drawImage onto a dstWidth×dstHeight
        // canvas — the browser's built-in scaler is GPU-accelerated.
        var srcCanvas = document.createElement('canvas');
        srcCanvas.width = srcWidth;
        srcCanvas.height = srcHeight;
        var srcCtx = srcCanvas.getContext('2d');
        var srcImg = new ImageData(new Uint8ClampedArray(srcData), srcWidth, srcHeight);
        srcCtx.putImageData(srcImg, 0, 0);

        var dstCanvas = document.createElement('canvas');
        dstCanvas.width = dstWidth;
        dstCanvas.height = dstHeight;
        var dstCtx = dstCanvas.getContext('2d');
        dstCtx.imageSmoothingEnabled = true;
        dstCtx.imageSmoothingQuality = 'high';
        dstCtx.drawImage(srcCanvas, 0, 0, dstWidth, dstHeight);

        return dstCtx.getImageData(0, 0, dstWidth, dstHeight).data;
    };

    /** Determine if an image is HDR based on its COLR + PIXI properties.
     * Thin adapter over HEIFUtils.isHDRContent (the single source of truth
     * for HDR detection across the codebase). The previous in-line
     * implementation here was self-flagged as guesswork; the shared
     * helper applies the same rules consistently across gallery.js and
     * the decode pipeline.
     * @param {number} itemId The item ID
     * @param {Object} propertiesMap Map of item properties
     * @returns {boolean} True if image appears to be HDR */
    this._isImageHDR = function(itemId, propertiesMap) {
        if (!propertiesMap || !propertiesMap[itemId]) return false;
        if (typeof HEIFUtils === 'undefined' || !HEIFUtils.isHDRContent) return false;

        var p = propertiesMap[itemId];
        if (!p.colr) return false;
        var bitDepth = (p.pixi && p.pixi.length > 0) ? p.pixi[0] : null;

        // Bridge the modern reader's colr shape (type/primaries/transfer
        // /matrix/fullRange) into the legacy colorInfo shape that
        // HEIFUtils.isHDRContent consumes. The ICC payload bytes are
        // not exposed by propertyColr yet (§15: a propertyColrIcc
        // accessor is needed); ICC-based HDR detection is therefore
        // unavailable until that lands and degrades to the NCLX path.
        var colorInfo = {
            colourType:              p.colr.type,
            colourPrimaries:         p.colr.colourPrimaries,
            transferCharacteristics: p.colr.transferCharacteristics,
            iccProfile:              null,
        };
        return HEIFUtils.isHDRContent(colorInfo, bitDepth);
    };

    /** Signal decode error to current context callback */
    this._signalDecodeError = function(errorMessage) {
        if (self._currentContext && self._currentContext.callback) {
            var errorResult = {
                success: false,
                error: errorMessage,
                frames: []
            };
            
            // IMPORTANT: Attach heifInfo from the payload so UI can still show file metadata
            if (self._currentContext.payload && self._currentContext.payload.heifInfo) {
                errorResult.heifInfo = self._currentContext.payload.heifInfo;
            }
            
            // Log decode failure
            if (window.HEIFFileLogger && window.HEIFFileLogger.logDecodeComplete) {
                window.HEIFFileLogger.logDecodeComplete({
                    filename: self._currentContext.filename || 'unknown',
                    success: false,
                    decodedFrameCount: 0,
                    expectedFrameCount: self._currentContext.itemIds ? self._currentContext.itemIds.length : 0,
                    error: errorMessage
                });
            }
            
            self._currentContext.callback(errorResult);
        }
        
        // Clear state and process next request
        self._decodedFrames = {};
        self._activeRequest = false;
        self.processNext();
    };

    /** Complete the current decode request - call this AFTER flush completes */
    this.completeRequest = function() {
        if (self._currentContext) {
            self._currentContext.callback(self._currentContext.payload);
            self._currentContext = null;
        }
        self._activeRequest = false;
        self.processNext();
    };

    /** Finalize after partial decode.
     *
     * `handleDecodedImage` only fires the request callback when
     * `currentFrameIndex >= itemIds.length` — the natural "all frames in"
     * threshold. When a decoder yields fewer frames than expected (libde265
     * skipping faulty pictures, vvdec rejecting a sample, etc.) the callback
     * would otherwise never fire and the request queue would stall forever.
     *
     * Adapter `_processStream` paths call this after exhausting the input
     * stream. It is a no-op when the natural threshold already fired
     * (`_currentContext` is null at that point). */
    this._finalizePartialDecode = function() {
        var ctx = self._currentContext;
        if (!ctx) return;
        if (ctx.currentFrameIndex >= ctx.itemIds.length) return;
        var expected = ctx.itemIds.length;
        ctx.itemIds = ctx.itemIds.slice(0, ctx.currentFrameIndex);
        // Mark the result as failed when no display frames were produced.
        // Without this, callers see {success: true, frames: []} and treat
        // the decode as successful — no error message is ever shown.
        if (ctx.payload.frames.length === 0) {
            ctx.payload.success = false;
        }
        try { self._postProcessAlphaMasks(ctx); } catch (e) {}
        try { self._postProcessGainmaps(ctx); } catch (e) {}
        if (window.HEIFFileLogger && window.HEIFFileLogger.logDecodeComplete) {
            window.HEIFFileLogger.logDecodeComplete({
                filename: ctx.filename,
                success: ctx.payload.frames.length > 0,
                decodedFrameCount: ctx.payload.frames.length,
                expectedFrameCount: expected,
                decodedItemIds: Object.keys(self._decodedFrames).map(Number),
                partial: true
            });
        }
        ctx.callback(ctx.payload);
        self._currentContext = null;
        self._decodedFrames = {};
        self._activeRequest = false;
        self.processNext();
    };

    /** Reset internal state */
    this._clearState = function() {
        self._imageData = null;
    };

    /** Process stream - implemented by decoder adapter */
    this._processStream = function(buffer) {
        self._clearState();
        throw new Error("_processStream must be implemented by decoder adapter");
    };

    /** Initialize appropriate decoder adapter */
    this._initializeAdapter = function() {
        switch(self.decoderType.toLowerCase()) {
            case 'libde265':
                new Libde265Adapter(self);
                break;
            case 'vvdec':
                new VvdecAdapter(self);
                break;
            default:
                throw new Error("Unknown decoder type: " + self.decoderType);
        }
    };

    this._setupCanvas(document.documentElement);
    this._clearState();
    this._initializeAdapter();
}





/** ========================================================
 *  =============== HDR STUFF ==============================
 *  ========================================================
 * 
 */

// Transfer Function Utilities for HDR/SDR
// Verified 2026-05-07 against SMPTE ST 2084 (PQ) and BT.2100 (HLG).

/** PQ (SMPTE ST 2084) constants */
var PQ = {
    m1: 0.1593017578125,    // 2610/16384
    m2: 78.84375,           // 2523/32 * 128
    c1: 0.8359375,          // 3424/4096
    c2: 18.8515625,         // 2413/128
    c3: 18.6875             // 2392/128
};

/** Convert PQ-encoded value to linear light (0-1 normalized, peak ~10000 nits)
 * @param {number} pq PQ value (0-1)
 * @returns {number} Linear light value */
var pqToLinear = function(pq) {
    if (pq <= 0) return 0;
    var pqPow = Math.pow(pq, 1 / PQ.m2);
    var num = Math.max(pqPow - PQ.c1, 0);
    var den = PQ.c2 - PQ.c3 * pqPow;
    return Math.pow(num / den, 1 / PQ.m1);
};

/** Convert linear light to PQ-encoded value
 * @param {number} linear Linear light value (0-1 normalized)
 * @returns {number} PQ value (0-1) */
var linearToPQ = function(linear) {
    if (linear <= 0) return 0;
    var linPow = Math.pow(linear, PQ.m1);
    var num = PQ.c1 + PQ.c2 * linPow;
    var den = 1 + PQ.c3 * linPow;
    return Math.pow(num / den, PQ.m2);
};

/** HLG (Hybrid Log-Gamma) constants */
var HLG = {
    a: 0.17883277,
    b: 0.28466892,  // 1 - 4*a
    c: 0.55991073   // 0.5 - a * ln(4*a)
};

/** Convert HLG-encoded value to linear light
 * @param {number} hlg HLG value (0-1)
 * @returns {number} Linear light value */
var hlgToLinear = function(hlg) {
    if (hlg <= 0) return 0;
    if (hlg <= 0.5) {
        return (hlg * hlg) / 3;
    } else {
        return (Math.exp((hlg - HLG.c) / HLG.a) + HLG.b) / 12;
    }
};

/** Convert linear light to HLG-encoded value
 * @param {number} linear Linear light value
 * @returns {number} HLG value (0-1) */
var linearToHLG = function(linear) {
    if (linear <= 0) return 0;
    if (linear <= 1/12) {
        return Math.sqrt(3 * linear);
    } else {
        return HLG.a * Math.log(12 * linear - HLG.b) + HLG.c;
    }
};

/** Convert sRGB gamma-encoded value to linear light
 * @param {number} srgb sRGB value (0-1)
 * @returns {number} Linear light value */
var srgbToLinear = function(srgb) {
    if (srgb <= 0.04045) {
        return srgb / 12.92;
    } else {
        return Math.pow((srgb + 0.055) / 1.055, 2.4);
    }
};

/** Convert linear light to sRGB gamma-encoded value
 * @param {number} linear Linear light value
 * @returns {number} sRGB value (0-1) */
var linearToSRGB = function(linear) {
    if (linear <= 0.0031308) {
        return linear * 12.92;
    } else {
        return 1.055 * Math.pow(linear, 1/2.4) - 0.055;
    }
};

/** Convert value from transfer function to linear light
 * @param {number} value Encoded value (0-1)
 * @param {number} transferCharacteristics ITU-T H.273 transfer characteristic code
 * @returns {number} Linear light value */
var transferToLinear = function(value, transferCharacteristics) {
    switch (transferCharacteristics) {
        case 16: // PQ (SMPTE ST 2084)
            return pqToLinear(value);
        case 18: // HLG (ARIB STD-B67)
            return hlgToLinear(value);
        case 13: // sRGB
            return srgbToLinear(value);
        case 1:  // BT.709
        case 6:  // BT.601
            // BT.709/601 gamma ~2.2 approximation using sRGB formula
            return srgbToLinear(value);
        default:
            // Unknown transfer - assume gamma 2.2
            return Math.pow(value, 2.2);
    }
};

/** Convert value from linear light to transfer function
 * @param {number} value Linear light value
 * @param {number} transferCharacteristics ITU-T H.273 transfer characteristic code
 * @returns {number} Encoded value (0-1) */
var linearToTransfer = function(value, transferCharacteristics) {
    if (value < 0) value = 0;
    switch (transferCharacteristics) {
        case 16: // PQ
            return linearToPQ(value);
        case 18: // HLG
            return linearToHLG(value);
        case 13: // sRGB
            return linearToSRGB(value);
        case 1:  // BT.709
        case 6:  // BT.601
            return linearToSRGB(value);
        default:
            // Unknown transfer - assume gamma 2.2
            return Math.pow(value, 1/2.2);
    }
};

/** Get transfer characteristics for an item from properties map
 * @param {number} itemId Item ID
 * @param {Object} propertiesMap Properties map
 * @returns {number} Transfer characteristics code (default 13 = sRGB) */
var getTransferCharacteristics = function(itemId, propertiesMap) {
    if (!propertiesMap || !propertiesMap[itemId]) {
        return 13; // Default to sRGB
    }
    var p = propertiesMap[itemId];
    if (p.colr && p.colr.type === 'nclx' && typeof p.colr.transferCharacteristics === 'number') {
        return p.colr.transferCharacteristics;
    }
    return 13; // Default to sRGB
};





/** =======================================================
 * ============== COLOUR SPACE STUFF ======================
 * ========================================================
 */


/** Color space conversion coefficients */
// YUV→RGB coefficients verified 2026-05-07 against ITU-R BT.601,
// BT.709, BT.2020. Limited-range values derived by scaling full-range
// chroma by 255/224 and luma by 255/219 per ISO/IEC 23091-2.
var ColorMatrix = {
    // BT.601 (SD video, older cameras)
    BT601_LIMITED: {
        name: 'BT.601 Limited',
        yOffset: 16,
        yScale: 1.164383,    // 255/219
        uOffset: 128,
        vOffset: 128,
        vToR: 1.596027,
        uToG: -0.391762,
        vToG: -0.812968,
        uToB: 2.017232
    },
    BT601_FULL: {
        name: 'BT.601 Full',
        yOffset: 0,
        yScale: 1.0,
        uOffset: 128,
        vOffset: 128,
        vToR: 1.402,
        uToG: -0.344136,
        vToG: -0.714136,
        uToB: 1.772
    },
    
    // BT.709 (HD video, most modern content)
    BT709_LIMITED: {
        name: 'BT.709 Limited',
        yOffset: 16,
        yScale: 1.164383,    // 255/219
        uOffset: 128,
        vOffset: 128,
        vToR: 1.792741,
        uToG: -0.213249,
        vToG: -0.532909,
        uToB: 2.112402
    },
    BT709_FULL: {
        name: 'BT.709 Full',
        yOffset: 0,
        yScale: 1.0,
        uOffset: 128,
        vOffset: 128,
        vToR: 1.5748,
        uToG: -0.187324,
        vToG: -0.468124,
        uToB: 1.8556
    },
    
    // BT.2020 (UHD/HDR content)
    BT2020_LIMITED: {
        name: 'BT.2020 Limited',
        yOffset: 16,
        yScale: 1.164383,    // 255/219
        uOffset: 128,
        vOffset: 128,
        vToR: 1.67867,
        uToG: -0.187326,
        vToG: -0.650424,
        uToB: 2.14177
    },
    BT2020_FULL: {
        name: 'BT.2020 Full',
        yOffset: 0,
        yScale: 1.0,
        uOffset: 128,
        vOffset: 128,
        vToR: 1.4746,
        uToG: -0.164553,
        vToG: -0.571353,
        uToB: 1.8814
    }
};

// Default color matrix
var activeColorMatrix = ColorMatrix.BT709_LIMITED;

/** Set the color matrix for YUV to RGB conversion
 * @param {string} matrixName - One of: 'BT601_LIMITED', 'BT601_FULL', 'BT709_LIMITED', 'BT709_FULL', 'BT2020_LIMITED', 'BT2020_FULL'
 */
function setColorMatrix(matrixName) {
    if (ColorMatrix[matrixName]) {
        activeColorMatrix = ColorMatrix[matrixName];
        console.debug('Color matrix set to:', activeColorMatrix.name);
    } else {
        console.warn('Unknown color matrix:', matrixName, '- using BT.709 Limited');
        activeColorMatrix = ColorMatrix.BT709_LIMITED;
    }
}

/** Determine the appropriate color matrix from COLR property values
 * @param {Object} colrInfo - COLR property data with matrixCoefficients, fullRangeFlag, colourPrimaries, or iccProfile
 * @returns {Object} Color matrix object
 */
function getColorMatrixFromColr(colrInfo) {
    if (!colrInfo) {
        return ColorMatrix.BT709_LIMITED;
    }
    
    var matrixCoeff = colrInfo.matrixCoefficients;
    var fullRange = colrInfo.fullRangeFlag;
    var colourType = colrInfo.colourType;
    
    // For ICC profiles (rICC or prof), try to derive matrix from profile
    if ((colourType === 'rICC' || colourType === 'prof') && colrInfo.iccProfile) {
        return getColorMatrixFromICCProfile(colrInfo.iccProfile, fullRange);
    }
    
    // Matrix coefficients defined in ITU-T H.273 / ISO/IEC 23091-2
    // 1 = BT.709
    // 5,6 = BT.601 (5=625 line, 6=525 line)
    // 9 = BT.2020 non-constant luminance
    // 10 = BT.2020 constant luminance
    
    if (matrixCoeff === 9 || matrixCoeff === 10) {
        // BT.2020
        return fullRange ? ColorMatrix.BT2020_FULL : ColorMatrix.BT2020_LIMITED;
    } else if (matrixCoeff === 5 || matrixCoeff === 6) {
        // BT.601
        return fullRange ? ColorMatrix.BT601_FULL : ColorMatrix.BT601_LIMITED;
    } else if (matrixCoeff === 1) {
        // BT.709
        return fullRange ? ColorMatrix.BT709_FULL : ColorMatrix.BT709_LIMITED;
    } else if (matrixCoeff === 0) {
        // Identity/GBR - may indicate RGB content
        // For YUV content, default to BT.709
        return fullRange ? ColorMatrix.BT709_FULL : ColorMatrix.BT709_LIMITED;
    }
    
    // Default: use colour primaries as hint if matrix is unspecified (2) or reserved
    var primaries = colrInfo.colourPrimaries;
    if (primaries === 9) {
        // BT.2020 primaries
        return fullRange ? ColorMatrix.BT2020_FULL : ColorMatrix.BT2020_LIMITED;
    } else if (primaries === 5 || primaries === 6) {
        // BT.601 primaries
        return fullRange ? ColorMatrix.BT601_FULL : ColorMatrix.BT601_LIMITED;
    }
    
    // Final fallback: BT.709
    return fullRange ? ColorMatrix.BT709_FULL : ColorMatrix.BT709_LIMITED;
}

/** Determine color matrix from ICC profile
 * Uses HEIFUtils.parseICCProfile to identify the profile and select appropriate YUV matrix
 * @param {Array|Uint8Array} iccProfile - ICC profile byte array
 * @param {boolean} fullRange - Whether full range (if known from other source)
 * @returns {Object} Color matrix object
 */
function getColorMatrixFromICCProfile(iccProfile, fullRange) {
    if (!iccProfile || iccProfile.length < 128) {
        return ColorMatrix.BT709_LIMITED;
    }
    
    // Convert to Uint8Array if needed
    var bytes = iccProfile instanceof Uint8Array ? iccProfile : new Uint8Array(iccProfile);
    
    // Use HEIFUtils parser if available - it already does comprehensive profile identification
    if (typeof HEIFUtils !== 'undefined' && HEIFUtils.parseICCProfile) {
        var parsed = HEIFUtils.parseICCProfile(bytes);
        if (parsed && parsed.valid && parsed.identified) {
            var identified = parsed.identified;
            
            // Use the yuvMatrix hint if available from profile identification
            if (identified.yuvMatrix === 'BT2020') {
                return fullRange ? ColorMatrix.BT2020_FULL : ColorMatrix.BT2020_LIMITED;
            } else if (identified.yuvMatrix === 'BT601') {
                return fullRange ? ColorMatrix.BT601_FULL : ColorMatrix.BT601_LIMITED;
            }
            // Default to BT.709 for most RGB profiles (sRGB, P3, Adobe RGB all use BT.709-derived matrices)
            return fullRange ? ColorMatrix.BT709_FULL : ColorMatrix.BT709_LIMITED;
        }
    }
    
    // Fallback if HEIFUtils not available: basic heuristics
    // This is a simplified version - HEIFUtils provides better identification
    return fullRange ? ColorMatrix.BT709_FULL : ColorMatrix.BT709_LIMITED;
}

/**
 * Log color information in a consolidated table format
 * @param {number|string} itemId - Item ID being processed
 * @param {Object} colrProp - COLR property data
 * @param {Object} matrix - Selected color matrix
 * @param {Object} [iccInfo] - Parsed ICC profile info (optional)
 * @param {string} [filename] - File URL/key to stash under (avoids cross-file leak when decodes ever pipeline)
 */
function logColorInfo(itemId, colrProp, matrix, iccInfo, filename) {
    if (!colrProp) return;
    
    var colorType = colrProp.colourType || 'unknown';
    var isICC = colorType === 'rICC' || colorType === 'prof';
    
    var info = {
        'Item ID': itemId,
        'Colour Type': colorType,
        'YUV Matrix': matrix ? matrix.name : 'N/A'
    };
    
    if (isICC && colrProp.iccProfile) {
        info['ICC Profile Size'] = colrProp.iccProfile.length + ' bytes';
        if (iccInfo && iccInfo.valid) {
            info['ICC Description'] = iccInfo.description || 'N/A';
            info['ICC Color Space'] = iccInfo.colorSpace || 'N/A';
            if (iccInfo.identified) {
                info['Identified As'] = iccInfo.identified.name || 'Unknown';
                info['Canvas Color Space'] = iccInfo.identified.canvasColorSpace || 'srgb';
                if (iccInfo.identified.note) {
                    info['Note'] = iccInfo.identified.note;
                }
            }
        }
    } else if (colorType === 'nclx') {
        info['Colour Primaries'] = colrProp.colourPrimaries !== undefined ? colrProp.colourPrimaries : 'N/A';
        info['Transfer Chars'] = colrProp.transferCharacteristics !== undefined ? colrProp.transferCharacteristics : 'N/A';
        info['Matrix Coeffs'] = colrProp.matrixCoefficients !== undefined ? colrProp.matrixCoefficients : 'N/A';
        info['Full Range'] = colrProp.fullRangeFlag !== undefined ? colrProp.fullRangeFlag : 'N/A';
    }
    
    // Stash for inclusion in the file-info group log; don't emit
    // standalone. Prefer per-file slot so concurrent decodes can't
    // attach colour info to the wrong group; keep _last as a
    // fallback for callers that don't yet pass a filename.
    var stash = { itemId: itemId, info: info };
    if (filename && window.HEIFFileLogger && window.HEIFFileLogger._pending) {
        var pending = window.HEIFFileLogger._pending[filename];
        if (pending) pending.colorInfo = stash;
    }
    logColorInfo._last = stash;
}


/** Get the color matrix for the current decode context (uses first item with COLR)
 * @returns {Object} Color matrix to use for YUV to RGB conversion
 */
var getColorMatrixForContext = function(manager) {
    var context = manager._currentContext;
    if (!context || !context.propertiesMap) {
        return activeColorMatrix;
    }

    // Look through all items in the context for a colr property and
    // pick the matrix from the first one that has one.
    for (var itemId in context.propertiesMap) {
        if (!context.propertiesMap.hasOwnProperty(itemId)) continue;

        var p = context.propertiesMap[itemId];
        if (!p.colr) continue;

        // Bridge the modern reader's colr shape to the legacy colrProp
        // shape consumed by getColorMatrixFromColr / logColorInfo. ICC
        // payload bytes are not exposed by propertyColr yet (§15), so
        // ICC-based matrix derivation degrades to NCLX-only.
        var colrProp = {
            colourType:              p.colr.type,
            colourPrimaries:         p.colr.primaries,
            transferCharacteristics: p.colr.transfer,
            matrixCoefficients:      p.colr.matrix,
            fullRangeFlag:           p.colr.fullRange,
            iccProfile:              null,
        };
        var iccInfo = null;
        var matrix = getColorMatrixFromColr(colrProp);
        logColorInfo(itemId, colrProp, matrix, iccInfo, context.filename);
        return matrix;
    }

    return activeColorMatrix;
};



/** ===========================================
 *  =========== LIBDE 265 ADAPTER =============
 *  ===========================================
 */

/**
 * Worker-pool adapter for libde265 (asm.js).
 *
 * Decoding happens off the main thread in classic Web Workers.
 * A shared pool of N workers is lazily spawned (default 2).
 * Requests carry a priority ('modal' > 'thumbnail' > 'animation')
 * so the most user-visible decode always runs first.
 */

/** Shared worker pool — singleton across all DecoderManager instances */
var _libde265Pool = null;

/** Priority values — lower number = higher priority */
var DECODE_PRIORITY = { modal: 0, thumbnail: 1, animation: 2 };

function Libde265WorkerPool(size) {
    this.size = size || 2;
    this.workers = [];
    this.idle = [];
    // 3-bucket priority queue: [modal[], thumbnail[], animation[]]
    this._buckets = [[], [], []];
    this.pending = {};       // id -> {onComplete, onError}
    this._nextId = 1;
    this._readyCount = 0;
    this._readyCallbacks = [];

    var self = this;

    for (var i = 0; i < this.size; i++) {
        (function(index) {
            var w = new Worker('js/decode-worker.js');

            w.onmessage = function(e) {
                var msg = e.data;
                if (msg.type === 'ready') {
                    self._readyCount++;
                    self.idle.push(index);
                    self._flushReadyCallbacks();
                    self._dispatch();
                    return;
                }
                if (msg.type === 'decode-complete') {
                    var cb = self.pending[msg.id];
                    delete self.pending[msg.id];
                    self.idle.push(index);
                    if (cb) cb.onComplete(msg.frames);
                    self._dispatch();
                    return;
                }
                if (msg.type === 'error') {
                    var ecb = self.pending[msg.id];
                    delete self.pending[msg.id];
                    self.idle.push(index);
                    if (ecb) ecb.onError(msg.error);
                    self._dispatch();
                }
            };

            w.onerror = function(err) {
                console.error('decode-worker ' + index + ' error:', err);
            };

            self.workers.push(w);
        })(i);
    }
}

Libde265WorkerPool.prototype._flushReadyCallbacks = function() {
    if (this._readyCount >= this.size) {
        var cbs = this._readyCallbacks;
        this._readyCallbacks = [];
        for (var i = 0; i < cbs.length; i++) cbs[i]();
    }
};

Libde265WorkerPool.prototype._dispatch = function() {
    while (this.idle.length > 0) {
        // Drain from highest-priority non-empty bucket
        var job = null;
        for (var b = 0; b < this._buckets.length; b++) {
            if (this._buckets[b].length > 0) {
                job = this._buckets[b].shift();
                break;
            }
        }
        if (!job) break;
        var workerIdx = this.idle.shift();
        this.pending[job.id] = { onComplete: job.onComplete, onError: job.onError };
        this.workers[workerIdx].postMessage(
            { type: 'decode', id: job.id, buffer: job.buffer },
            [job.buffer]
        );
    }
};

/**
 * Enqueue a decode job.
 * @param {ArrayBuffer} buffer   - Annex-B bitstream (transferred, do not reuse)
 * @param {string}      priority - 'modal' | 'animation' | 'thumbnail'
 * @param {function}    onComplete - called with frames array
 * @param {function}    onError    - called with error string
 */
Libde265WorkerPool.prototype.enqueue = function(buffer, priority, onComplete, onError) {
    var id = this._nextId++;
    var pval = DECODE_PRIORITY[priority] !== undefined ? DECODE_PRIORITY[priority] : 2;
    var job = { id: id, buffer: buffer, priority: pval, onComplete: onComplete, onError: onError };

    // O(1) bucket insertion instead of O(n) sorted splice
    this._buckets[pval].push(job);

    this._dispatch();
};

/** Configurable pool size — set before first decode triggers pool creation */
Libde265WorkerPool.POOL_SIZE = 2;

function _getLibde265Pool() {
    if (!_libde265Pool) {
        _libde265Pool = new Libde265WorkerPool(Libde265WorkerPool.POOL_SIZE);
    }
    return _libde265Pool;
}


/** Adapter that dispatches to the shared worker pool */
function Libde265Adapter(manager) {

    manager._processStream = function(buffer) {
        var pool = _getLibde265Pool();

        // Normalise to an owned ArrayBuffer for transfer
        var ab;
        if (buffer instanceof ArrayBuffer) {
            ab = buffer;
        } else if (buffer instanceof Uint8Array) {
            ab = buffer.buffer.slice(buffer.byteOffset, buffer.byteOffset + buffer.byteLength);
        } else {
            ab = buffer;
        }

        // Determine priority from the active request context
        // Order: modal (0) > thumbnail (1) > animation (2)
        var priority = 'thumbnail';
        if (manager._activeRequest) {
            var req = manager._activeRequest;
            if (req.priority) {
                priority = req.priority;
            } else if (req.itemIds && req.itemIds.length > 1) {
                priority = 'animation';
            }
        }

        pool.enqueue(
            ab,
            priority,
            function onComplete(frames) {
                if (!frames || frames.length === 0) {
                    console.warn('libde265 worker returned 0 frames');
                    manager._finalizePartialDecode();
                    return;
                }
                for (var i = 0; i < frames.length; i++) {
                    var f = frames[i];
                    manager.handleDecodedImage(f.width, f.height, f.rgba, f.bitDepth);
                }
            },
            function onError(err) {
                console.error('libde265 worker decode error:', err);
                manager._finalizePartialDecode();
            }
        );
    };
}





/** ===========================================
 *  =========== VVDEC ADAPTER =================
 *  ===========================================
 */

/** Adapter for VVdeC decoder - Simple per-request pattern */
function VvdecAdapter(manager) {

    // Buffer pool for reuse across frames
    var bufferPool = {
        yuv: null,
        rgba: null
    };

    /** Copy plane data with stride removal and optional bit-depth conversion
     * @param {Object} plane - Plane object with ptr, width, height, stride, bytesPerSample
     * @param {Uint8Array} dest - Destination buffer
     * @param {number} destOffset - Offset in destination buffer
     */
    var copyPlaneWithConversion = function(plane, dest, destOffset) {
        var width = plane.width;
        var height = plane.height;
        var strideInSamples = plane.stride / plane.bytesPerSample;
        var bytesPerSample = plane.bytesPerSample;
        var ptr = plane.ptr;

        if (bytesPerSample === 1) {
            // 8-bit: copy row by row to remove stride padding
            for (var row = 0; row < height; row++) {
                var srcOffset = row * strideInSamples;
                var dstOffset = destOffset + row * width;
                dest.set(ptr.subarray(srcOffset, srcOffset + width), dstOffset);
            }
        } else {
            // 10-bit or higher: convert to 8-bit while removing stride.
            // Round-half-up bias matches the libde265 extract path - see
            // the comment block there for rationale, including the >255
            // clamp that prevents max-value overflow from wrapping to 0
            // in the Uint8Array destination.
            var shift = bytesPerSample === 2 ? 2 : 0; // 10-bit uses 16-bit storage, shift by 2
            var roundBias = shift > 0 ? (1 << (shift - 1)) : 0;
            for (var row = 0; row < height; row++) {
                var srcBase = row * strideInSamples;
                var dstBase = destOffset + row * width;
                for (var col = 0; col < width; col++) {
                    var v = (ptr[srcBase + col] + roundBias) >> shift;
                    dest[dstBase + col] = v > 255 ? 255 : v;
                }
            }
        }
    };

    /** Extract YUV with buffer reuse
     * @param {Object} frame - Decoded frame with planes array
     * @returns {Object} YUV data object
     */
    var extractYUV = function(frame) {
        var planeY = frame.planes[0];
        var planeU = frame.planes[1];
        var planeV = frame.planes[2];

        var yWidth = planeY.width;
        var yHeight = planeY.height;
        var ySize = yWidth * yHeight;

        var uvWidth = planeU.width;
        var uvHeight = planeU.height;
        var uvSize = uvWidth * uvHeight;
        var isMonochrome = !uvSize;

        var totalSize = ySize + (isMonochrome ? 0 : uvSize * 2);

        // Reuse or allocate YUV buffer
        if (!bufferPool.yuv || bufferPool.yuv.length !== totalSize) {
            bufferPool.yuv = new Uint8Array(totalSize);
        }

        var yuvData = bufferPool.yuv;

        // Fast path check for Y plane: 8-bit with no stride padding
        var yStrideInSamples = planeY.stride / planeY.bytesPerSample;
        var noYPadding = yStrideInSamples === yWidth;

        if (planeY.bytesPerSample === 1 && noYPadding) {
            // Direct copy - fastest path
            yuvData.set(planeY.ptr.subarray(0, ySize), 0);
        } else {
            // Handle stride and/or bit-depth conversion
            copyPlaneWithConversion(planeY, yuvData, 0);
        }

        // Handle U/V planes if not monochrome
        var chromaSubsampling = '420'; // default
        if (!isMonochrome) {
            // Determine chroma subsampling from UV dimensions
            if (uvWidth === yWidth && uvHeight === yHeight) {
                chromaSubsampling = '444';
            } else if (uvWidth === (yWidth >> 1) && uvHeight === yHeight) {
                chromaSubsampling = '422';
            } else {
                chromaSubsampling = '420';
            }
            
            var uStrideInSamples = planeU.stride / planeU.bytesPerSample;
            var vStrideInSamples = planeV.stride / planeV.bytesPerSample;
            var noUVPadding = uStrideInSamples === uvWidth && vStrideInSamples === uvWidth;
            
            if (planeU.bytesPerSample === 1 && noUVPadding) {
                // Direct copy for U and V
                yuvData.set(planeU.ptr.subarray(0, uvSize), ySize);
                yuvData.set(planeV.ptr.subarray(0, uvSize), ySize + uvSize);
            } else {
                // Handle stride and/or bit-depth conversion
                copyPlaneWithConversion(planeU, yuvData, ySize);
                copyPlaneWithConversion(planeV, yuvData, ySize + uvSize);
            }
        }
        
        // VVdeC YUV extraction (suppress per-frame log)

        return {
            data: yuvData,
            width: frame.width,
            height: frame.height,
            isMonochrome: isMonochrome,
            chromaSubsampling: chromaSubsampling,
            uvWidth: uvWidth,
            uvHeight: uvHeight
        };
    };
    
    /** Convert YUV to RGBA (supports 420, 422, 444)
     * Uses color matrix coefficients from ITU-R standards.
     */
    var convertYUVToRGBA = function(yuvData, width, height, isMonochrome, colorMatrix, chromaSubsampling, uvWidth, uvHeight) {
        var matrix = colorMatrix || activeColorMatrix;
        var pixelCount = width * height;
        var rgbaSize = pixelCount * 4;
        
        chromaSubsampling = chromaSubsampling || '420';

        // Reuse or allocate RGBA buffer
        if (!bufferPool.rgba || bufferPool.rgba.length !== rgbaSize) {
            bufferPool.rgba = new Uint8ClampedArray(rgbaSize);
        }

        var rgbaData = bufferPool.rgba;

        if (isMonochrome) {
            // Monochrome - apply Y scaling for limited range
            var yOffset = matrix.yOffset;
            var yScale = matrix.yScale;
            
            for (var i = 0; i < pixelCount; i++) {
                var idx = i * 4;
                var y = (yuvData[i] - yOffset) * yScale;
                y = y < 0 ? 0 : (y > 255 ? 255 : y);
                
                rgbaData[idx] = y;     // R
                rgbaData[idx + 1] = y; // G
                rgbaData[idx + 2] = y; // B
                rgbaData[idx + 3] = 255; // A
            }
        } else {
            // Standard YUV to RGB conversion
            var ySize = pixelCount;
            
            // Calculate UV dimensions if not provided
            if (!uvWidth || !uvHeight) {
                if (chromaSubsampling === '444') {
                    uvWidth = width;
                    uvHeight = height;
                } else if (chromaSubsampling === '422') {
                    uvWidth = width >> 1;
                    uvHeight = height;
                } else {
                    uvWidth = width >> 1;
                    uvHeight = height >> 1;
                }
            }
            var uvSize = uvWidth * uvHeight;
            
            // Extract matrix coefficients for performance
            var yOffset = matrix.yOffset;
            var yScale = matrix.yScale;
            var vToR = matrix.vToR;
            var uToG = matrix.uToG;
            var vToG = matrix.vToG;
            var uToB = matrix.uToB;
            
            if (chromaSubsampling === '444') {
                // YUV444: 1:1 mapping
                for (var row = 0; row < height; row++) {
                    var rowOffset = row * width;
                    var uvRowOffset = ySize + row * uvWidth;

                    for (var col = 0; col < width; col++) {
                        var yIndex = rowOffset + col;
                        var uvIndex = uvRowOffset + col;

                        var yVal = yuvData[yIndex];
                        var uVal = yuvData[uvIndex] - 128;
                        var vVal = yuvData[uvIndex + uvSize] - 128;

                        var y = (yVal - yOffset) * yScale;
                        var r = y + vToR * vVal;
                        var g = y + uToG * uVal + vToG * vVal;
                        var b = y + uToB * uVal;
                        
                        var rgbaIndex = yIndex * 4;
                        rgbaData[rgbaIndex] = r < 0 ? 0 : (r > 255 ? 255 : r);
                        rgbaData[rgbaIndex + 1] = g < 0 ? 0 : (g > 255 ? 255 : g);
                        rgbaData[rgbaIndex + 2] = b < 0 ? 0 : (b > 255 ? 255 : b);
                        rgbaData[rgbaIndex + 3] = 255;
                    }
                }
            } else if (chromaSubsampling === '422') {
                // YUV422: horizontal 2:1 subsampling
                for (var row = 0; row < height; row++) {
                    var rowOffset = row * width;
                    var uvRowOffset = ySize + row * uvWidth;

                    for (var col = 0; col < width; col++) {
                        var yIndex = rowOffset + col;
                        var uvCol = col >> 1;
                        var uvIndex = uvRowOffset + uvCol;

                        var yVal = yuvData[yIndex];
                        var uVal = yuvData[uvIndex] - 128;
                        var vVal = yuvData[uvIndex + uvSize] - 128;

                        var y = (yVal - yOffset) * yScale;
                        var r = y + vToR * vVal;
                        var g = y + uToG * uVal + vToG * vVal;
                        var b = y + uToB * uVal;
                        
                        var rgbaIndex = yIndex * 4;
                        rgbaData[rgbaIndex] = r < 0 ? 0 : (r > 255 ? 255 : r);
                        rgbaData[rgbaIndex + 1] = g < 0 ? 0 : (g > 255 ? 255 : g);
                        rgbaData[rgbaIndex + 2] = b < 0 ? 0 : (b > 255 ? 255 : b);
                        rgbaData[rgbaIndex + 3] = 255;
                    }
                }
            } else {
                // YUV420: horizontal and vertical 2:1 subsampling
                for (var row = 0; row < height; row++) {
                    var uvRow = row >> 1;
                    var rowOffset = row * width;
                    var uvRowOffset = ySize + uvRow * uvWidth;

                    for (var col = 0; col < width; col++) {
                        var yIndex = rowOffset + col;
                        var uvCol = col >> 1;
                        var uvIndex = uvRowOffset + uvCol;

                        var yVal = yuvData[yIndex];
                        var uVal = yuvData[uvIndex] - 128;
                        var vVal = yuvData[uvIndex + uvSize] - 128;

                        var y = (yVal - yOffset) * yScale;
                        var r = y + vToR * vVal;
                        var g = y + uToG * uVal + vToG * vVal;
                        var b = y + uToB * uVal;
                        
                        var rgbaIndex = yIndex * 4;
                        rgbaData[rgbaIndex] = r < 0 ? 0 : (r > 255 ? 255 : r);
                        rgbaData[rgbaIndex + 1] = g < 0 ? 0 : (g > 255 ? 255 : g);
                        rgbaData[rgbaIndex + 2] = b < 0 ? 0 : (b > 255 ? 255 : b);
                        rgbaData[rgbaIndex + 3] = 255;
                    }
                }
            }
        }
        // Return the pool buffer directly — the caller owns it.
        // Reset the pool slot so the next call allocates fresh.
        var result = rgbaData;
        bufferPool.rgba = null;
        return result;
    };

    /** Process frame and send to manager */
    var processFrame = function(frame, decInstance, vvdecModule) {
        var yuvResult = extractYUV(frame);
        var contextColorMatrix = getColorMatrixForContext(manager);
        var rgbaData = convertYUVToRGBA(
            yuvResult.data,
            yuvResult.width,
            yuvResult.height,
            yuvResult.isMonochrome,
            contextColorMatrix,
            yuvResult.chromaSubsampling,
            yuvResult.uvWidth,
            yuvResult.uvHeight
        );
        
        manager.handleDecodedImage(yuvResult.width, yuvResult.height, rgbaData, frame.bitDepth || 0);
        decInstance.frame_unref(frame);
    };

    this._processStream = function(buffer) {
        // Create new module instance for THIS request only.
        // vvdec ships as three sibling files (vvdecapp.js + .wasm + .worker.js)
        // under website/js/vvdec/ — locate the <script> tag for vvdecapp.js so
        // the .wasm and .worker.js sidecars resolve to the same subfolder.
        var scriptPath = './js/vvdec/';
        var scripts = document.getElementsByTagName('script');
        for (var i = 0; i < scripts.length; i++) {
            if (scripts[i].src.indexOf('vvdecapp.js') !== -1) {
                scriptPath = scripts[i].src.substring(0, scripts[i].src.lastIndexOf('/') + 1);
                break;
            }
        }

        CreateVVdeC({
            locateFile: function(path) {
                if (path.endsWith('.wasm') || path.endsWith('.worker.js')) {
                    return scriptPath + path;
                }
                return path;
            }
        }).then(function(vvdecModule) {
            if (!VvdecAdapter._versionLogged && vvdecModule._vvdec_get_version) {
                var getVer = vvdecModule.cwrap('vvdec_get_version', 'string', []);
                console.info('vvdec ' + getVer() + ' ready');
                VvdecAdapter._versionLogged = true;
            }
            var decInstance = null;
            var au = null;
            var decodedAnyFrame = false;
            
            try {
                var data = new Uint8Array(buffer);

                // Create decoder instance
                var params = new vvdecModule.Params();
                params.threads = 0;
                decInstance = new vvdecModule.Decoder(params);
                params.delete();

                // Create access unit, which serves as the unit of measure for decoding
                //parametersets+image (=bunch of nalus)
                au = new vvdecModule.AccessUnit();
                au.alloc_payload(Math.max(100000, data.byteLength + 4));

                // Decode all NALUs
                var start = 0;
                while (start < data.byteLength) {
                    var end = data.byteLength;

                    // Find next start code
                    for (var i = start + 3; i + 3 < data.byteLength; i++) {
                        if ((data[i] === 0 && data[i+1] === 0 && data[i+2] === 1) ||
                            (data[i] === 0 && data[i+1] === 0 && data[i+2] === 0 && data[i+3] === 1)) {
                            end = i;
                            break;
                        }
                    }

                    var nalu = data.subarray(start, end);

                    // Decode NALU
                    if (au.payloadSize < nalu.length + 4) {
                        au.free_payload();
                        au.alloc_payload(Math.floor((nalu.length + 4) * 1.2));
                    }

                    au.payload.set(nalu);
                    au.payloadUsedSize = nalu.byteLength;
                    au.ctsValid = false;
                    au.dtsValid = false;

                    var frameHandle = new vvdecModule.FrameHandle();
                    // vvdec returns 0 on success, -40 = VVDEC_TRY_AGAIN (need more data, normal
                    // during tune-in), other negatives = real errors. See vvdec/vvdec.h.
                    var ret = decInstance.decode(au, frameHandle);
                    if (ret !== 0 && ret !== -40) {
                        var le  = (typeof decInstance.get_last_error            === 'function') ? decInstance.get_last_error()            : '';
                        var lae = (typeof decInstance.get_last_additional_error === 'function') ? decInstance.get_last_additional_error() : '';
                        console.warn('VVdec decode() ret=' + ret + ': ' + le + (lae ? ' — ' + lae : ''));
                    }

                    if (frameHandle.frame) {
                        decodedAnyFrame = true;
                        processFrame(frameHandle.frame, decInstance, vvdecModule);
                    }

                    frameHandle.delete();
                    start = end;

                    if (end >= data.byteLength) break;
                }

                // Flush remaining frames
                var flushing = true;
                while (flushing) {
                    var flushHandle = new vvdecModule.FrameHandle();
                    var ret = decInstance.flush(flushHandle);
                    if (flushHandle.frame) {
                        decodedAnyFrame = true;
                        processFrame(flushHandle.frame, decInstance, vvdecModule);
                    }
                    flushHandle.delete();
                    flushing = (ret === 0);
                }
                
                // If no frames were decoded, signal error
                if (!decodedAnyFrame) {
                    manager._signalDecodeError('No frames decoded from stream');
                } else {
                    // Partial decode safety net — see _finalizePartialDecode.
                    manager._finalizePartialDecode();
                }
                
            } catch (e) {
                // Unhandled C++ exception escaped from vvdec into JS. With Emscripten
                // exception-handling these arrive as a raw heap pointer (number).
                // The vvdec embind wrapper exposes get_last_error / get_last_additional_error
                // as methods on the Decoder instance (see vvdecWebPlayer/decoderWorker.js).
                // For multilayer VVC the additional error reads:
                //   "Multilayer profiles not yet supported"
                // — vvdec deliberately doesn't decode it (issue fraunhoferhhi/vvdec#197).
                var parts = [];
                try {
                    if (decInstance && typeof decInstance.get_last_error === 'function') {
                        var le = decInstance.get_last_error();
                        if (le) parts.push(le);
                    }
                    if (decInstance && typeof decInstance.get_last_additional_error === 'function') {
                        var lae = decInstance.get_last_additional_error();
                        if (lae) parts.push(lae);
                    }
                } catch (_) { /* decoder may already be in a torn-down state */ }
                try {
                    if (!parts.length && typeof e === 'number' &&
                        vvdecModule && typeof vvdecModule.getExceptionMessage === 'function') {
                        var emMsg = vvdecModule.getExceptionMessage(e);
                        if (Array.isArray(emMsg)) emMsg = emMsg.filter(Boolean).join(' | ');
                        if (emMsg) parts.push(emMsg);
                    }
                } catch (_) { /* getExceptionMessage may not be exported */ }
                if (!parts.length) {
                    parts.push(e && e.message ? e.message : String(e));
                }
                var msg = parts.join(' — ');
                console.error('VVdec decode error:', msg, '(raw:', e, ')');
                manager._signalDecodeError(msg || 'Decode error');
            } finally {
                // Cleanup
                if (au) au.delete();
                if (decInstance) decInstance.delete();
            }
        }).catch(function(error) {
            console.error('VVdec module creation failed:', error);
            manager._signalDecodeError(error.message || 'Module creation failed');
        });
    };
    manager._processStream = this._processStream;
}

// Export to global scope for non-module scripts that depend on these
if (typeof window !== 'undefined') {
    window.DecoderManager = DecoderManager;
    window.Libde265Adapter = Libde265Adapter;
    window.Libde265WorkerPool = Libde265WorkerPool;
    window.VvdecAdapter = VvdecAdapter;
    window.ColorMatrix = ColorMatrix;
    window.setColorMatrix = setColorMatrix;
    window.getColorMatrixFromColr = getColorMatrixFromColr;
}