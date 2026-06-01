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

/**
 * Shared utility functions for HEIF website
 * @namespace HEIFUtils
 */
var HEIFUtils = (function() {
    
    // ========================================
    // CONFIGURATION
    // ========================================
    
    /** Default timeout for async operations in milliseconds */
    var DEFAULT_TIMEOUT = 30000; // 30 seconds
    
    /** Default polling interval for readiness checks */
    var DEFAULT_POLL_INTERVAL = 100;
    
    // ========================================
    // FILE SIZE FORMATTING
    // ========================================
    
    /**
     * Format file size in human-readable form
     * @param {number} bytes - File size in bytes
     * @returns {string} Formatted file size (e.g., "1.5 MiB")
     */
    function formatFileSize(bytes) {
        if (bytes === null || bytes === undefined || isNaN(bytes)) {
            return 'Unknown';
        }
        if (bytes < 1024) return bytes + ' B';
        if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KiB';
        return (bytes / (1024 * 1024)).toFixed(2) + ' MiB';
    }
    
    // ========================================
    // DOM HELPERS
    // ========================================
    
    /**
     * Create a separator element for info displays
     * @returns {HTMLSpanElement} Separator span element
     */
    function createSeparator() {
        var sep = document.createElement('span');
        sep.className = 'info-sep';
        sep.textContent = '•';
        return sep;
    }
    
    // ========================================
    // BRAND DEFINITIONS (ISO/IEC 23008-12 HEIF)
    // ========================================
    
    /**
     * Brand classification constants
     * Based on ISO/IEC 23008-12 (HEIF) and related specifications
     */
    var BRANDS = {
        // === STRUCTURAL BRANDS (file structure) ===
        STILL_IMAGE: ['mif1', 'mif2', 'mif3'],  // Meta Image File (stills)
        SEQUENCE:    ['msf1'],                   // Meta Sequence File (animation/sequence)
        
        // === HEVC BRANDS (H.265) ===
        HEVC_STILL:    ['heic', 'heix'],         // HEVC still images
        HEVC_SEQUENCE: ['hevc', 'hevx'],         // HEVC image sequences
        HEVC_LHEVC_STILL:    ['heim', 'heis'],   // L-HEVC (layered) stills
        HEVC_LHEVC_SEQUENCE: ['hevm', 'hevs'],   // L-HEVC (layered) sequences
        
        // === VVC BRANDS (H.266) ===
        VVC_STILL:    ['vvic'],       // VVC still images
        VVC_SEQUENCE: ['vvis'],       // VVC image sequences
        
        // === AV1 BRANDS ===
        AV1_STILL:    ['avif', 'avio', 'MA1A', 'MA1B'],  // AV1 still images
        AV1_SEQUENCE: ['avis'],                          // AV1 image sequences
        
        // === AVC BRANDS (H.264) ===
        AVC_STILL:    ['avci'],                  // AVC still images
        AVC_SEQUENCE: ['avcs'],                  // AVC image sequences

        // === JPEG BRANDS ===
        JPEG_STILL:         ['jpeg'],  // Generic JPEG (baseline or extended)
        JPEG_SEQUENCE:      ['jpgs'],  // JPEG image sequences
        
        // === JPEG 2000 BRANDS ===
        J2K_STILL:    ['j2ki'],                  // JPEG 2000 still images
        J2K_SEQUENCE: ['j2is'],                  // JPEG 2000 image sequences

        EVC_BASE_STILL:    ['evbi'],             // EVC baseline
        EVC_MAIN_STILL:    ['evmi'],             // EVC main
        EVC_BASE_SEQUENCE: ['evbs'],             // EVC baseline sequence
        EVC_MAIN_SEQUENCE: ['evms'],             // EVC main sequence

        
        // === SPECIAL BRANDS ===
        MIAF:         ['miaf'],                  // MIAF compliant (interoperability)
        PRED:         ['pred'],                  // Predictively coded
        SLIM:         ['mif3'],                  // SlimHEIF / low overhead
        ONEPIC:       ['1pic'],                  // Single still image in items
        ISOEIGHT:     ['iso8'],                  // ISO Base Media File Format v8 (generic)
    };
    
    /**
     * Analyze brands to determine file characteristics
     * This is the AUTHORITATIVE source for brand-based decisions
     * 
     * @param {string} majorBrand - Major brand from ftyp box
     * @param {Array.<string>} compatibleBrands - Compatible brands array
     * @returns {Object} Brand analysis result
     */
    function analyzeBrands(majorBrand, compatibleBrands) {
        var result = {
            // Primary classification
            isStillImage: false,      // Major brand indicates still image
            isSequence: false,        // Major brand indicates sequence/animation
            
            // Codec detection
            codec: null,              // Primary codec: 'HEVC', 'VVC', 'AV1', 'AVC', 'EVC', 'JPEG', 'J2K'
            hasHEVC: false,
            hasVVC: false,
            hasAV1: false,
            hasAVC: false,
            hasEVC: false,
            hasJPEG: false,
            hasJ2K: false,
            
            // Special features
            isLayered: false,         // L-HEVC or similar layered codec
            isSlimHEIF: false,        // mif3 SlimHEIF
            isMIAF: false,            // MIAF compliant
            hasPredictive: false,     // Has predictively coded content (pred brand)
            isSingleImage: false,     // 1pic brand - single still image
            
            // Format string for display
            format: 'HEIF',
            codecString: 'Unknown',
            
            // Raw data
            majorBrand: majorBrand,
            compatibleBrands: compatibleBrands || []
        };
        
        if (!majorBrand && (!compatibleBrands || compatibleBrands.length === 0)) {
            return result;
        }
        
        var major = (majorBrand || '').toLowerCase();
        var allBrands = [major].concat((compatibleBrands || []).map(function(b) { 
            return b.toLowerCase(); 
        }));
        
        // Helper to check if any brand matches
        var hasBrand = function(brandList) {
            for (var i = 0; i < brandList.length; i++) {
                if (allBrands.indexOf(brandList[i].toLowerCase()) !== -1) {
                    return true;
                }
            }
            return false;
        };
        
        // === DETERMINE PRIMARY TYPE FROM MAJOR BRAND ===
        // The major brand is the AUTHORITATIVE indicator of file type
        
        // Build sequence brand list from all sequence types
        var sequenceMajorBrands = BRANDS.SEQUENCE
            .concat(BRANDS.HEVC_SEQUENCE)
            .concat(BRANDS.HEVC_LHEVC_SEQUENCE)
            .concat(BRANDS.VVC_SEQUENCE)
            .concat(BRANDS.AV1_SEQUENCE)
            .concat(BRANDS.AVC_SEQUENCE)
            .concat(BRANDS.EVC_BASE_SEQUENCE)
            .concat(BRANDS.EVC_MAIN_SEQUENCE)
            .concat(BRANDS.JPEG_SEQUENCE)
            .concat(BRANDS.J2K_SEQUENCE);
        
        // Build still brand list from all still types
        var stillMajorBrands = BRANDS.STILL_IMAGE
            .concat(BRANDS.HEVC_STILL)
            .concat(BRANDS.HEVC_LHEVC_STILL)
            .concat(BRANDS.VVC_STILL)
            .concat(BRANDS.AV1_STILL)
            .concat(BRANDS.AVC_STILL)
            .concat(BRANDS.EVC_BASE_STILL)
            .concat(BRANDS.EVC_MAIN_STILL)
            .concat(BRANDS.JPEG_STILL)
            .concat(BRANDS.J2K_STILL)
            .concat(BRANDS.ONEPIC);
        
        // Major brand determines primary classification
        if (sequenceMajorBrands.indexOf(major) !== -1) {
            result.isSequence = true;
        } else if (stillMajorBrands.indexOf(major) !== -1) {
            result.isStillImage = true;
        } else {
            // Unknown major brand - check compatible brands for hints
            if (hasBrand(BRANDS.SEQUENCE) || hasBrand(BRANDS.HEVC_SEQUENCE) || 
                hasBrand(BRANDS.AV1_SEQUENCE) || hasBrand(BRANDS.VVC_SEQUENCE) ||
                hasBrand(BRANDS.EVC_BASE_SEQUENCE) || hasBrand(BRANDS.EVC_MAIN_SEQUENCE) ||
                hasBrand(BRANDS.JPEG_SEQUENCE) || hasBrand(BRANDS.J2K_SEQUENCE)) {
                result.isSequence = true;
            } else {
                result.isStillImage = true; // Default to still
            }
        }
        
        // === DETECT CODECS ===
        var codecs = [];
        
        // HEVC detection (H.265)
        if (hasBrand(BRANDS.HEVC_STILL) || hasBrand(BRANDS.HEVC_SEQUENCE) ||
            hasBrand(BRANDS.HEVC_LHEVC_STILL) || hasBrand(BRANDS.HEVC_LHEVC_SEQUENCE)) {
            result.hasHEVC = true;
            codecs.push('HEVC');
            if (hasBrand(BRANDS.HEVC_LHEVC_STILL) || hasBrand(BRANDS.HEVC_LHEVC_SEQUENCE)) {
                result.isLayered = true;
            }
        }
        
        // VVC detection (H.266)
        if (hasBrand(BRANDS.VVC_STILL) || hasBrand(BRANDS.VVC_SEQUENCE)) {
            result.hasVVC = true;
            codecs.push('VVC');
        }
        
        // AV1 detection
        if (hasBrand(BRANDS.AV1_STILL) || hasBrand(BRANDS.AV1_SEQUENCE)) {
            result.hasAV1 = true;
            codecs.push('AV1');
        }
        
        // AVC detection (H.264)
        if (hasBrand(BRANDS.AVC_STILL) || hasBrand(BRANDS.AVC_SEQUENCE)) {
            result.hasAVC = true;
            codecs.push('AVC');
        }
        
        // EVC detection (MPEG-5 Part 1)
        if (hasBrand(BRANDS.EVC_BASE_STILL) || hasBrand(BRANDS.EVC_MAIN_STILL) ||
            hasBrand(BRANDS.EVC_BASE_SEQUENCE) || hasBrand(BRANDS.EVC_MAIN_SEQUENCE)) {
            result.hasEVC = true;
            codecs.push('EVC');
        }
        
        // JPEG detection
        if (hasBrand(BRANDS.JPEG_STILL) || hasBrand(BRANDS.JPEG_SEQUENCE)) {
            result.hasJPEG = true;
            codecs.push('JPEG');
        }
        
        // JPEG 2000 detection
        if (hasBrand(BRANDS.J2K_STILL) || hasBrand(BRANDS.J2K_SEQUENCE)) {
            result.hasJ2K = true;
            codecs.push('JPEG 2000');
        }
        
        // Set primary codec (first detected)
        result.codec = codecs.length > 0 ? codecs[0] : null;
        result.codecString = codecs.length > 0 ? codecs.join(' / ') : 'Unknown';
        
        // === DETECT SPECIAL FEATURES ===
        result.isSlimHEIF = hasBrand(BRANDS.SLIM);
        result.isMIAF = hasBrand(BRANDS.MIAF);
        result.hasPredictive = hasBrand(BRANDS.PRED);
        result.isSingleImage = hasBrand(BRANDS.ONEPIC);
        
        // === BUILD FORMAT STRING ===
        var formats = [];
        if (result.hasAV1) formats.push('AVIF');
        else if (result.hasHEVC) formats.push('HEIC');
        else if (result.hasEVC) formats.push('EVIF');
        else if (result.hasJPEG) formats.push('JPEG');
        else if (result.hasJ2K) formats.push('JPEG 2000');
        else formats.push('HEIF');
        
        if (result.isSlimHEIF) formats.push('SlimHEIF');
        if (result.isSequence) formats.push('Sequence');
        
        result.format = formats.join(' / ');
        
        return result;
    }
    
    // ========================================
    // FORMAT/CODEC DETECTION (LEGACY WRAPPER)
    // ========================================
    
    /**
     * Derive format and codec from compatible brands
     * @param {Array.<string>} brands - Array of brand strings from HEIF file
     * @returns {{format: string, codec: string}} Object with format and codec
     * @deprecated Use analyzeBrands() for comprehensive brand analysis
     */
    function deriveFormatAndCodec(brands) {
        // Use new analyzeBrands with null majorBrand for backward compatibility
        var analysis = analyzeBrands(brands && brands[0], brands);
        return {
            format: analysis.format,
            codec: analysis.codecString
        };
    }
    
    /**
     * Derive format from file extension for non-HEIF files
     * @param {string} url - File URL
     * @returns {{format: string, codec: string|null}} Object with format and codec
     */
    function deriveFormatFromExtension(url) {
        if (!url) return { format: 'Unknown', codec: null };
        
        var ext = url.toLowerCase().split('.').pop();
        
        switch (ext) {
            case 'jpg':
            case 'jpeg':
                return { format: 'JPEG', codec: null };
            case 'png':
                return { format: 'PNG', codec: null };
            case 'gif':
                return { format: 'GIF', codec: null };
            case 'webp':
                return { format: 'WebP', codec: null };
            case 'bmp':
                return { format: 'BMP', codec: null };
            case 'heic':
                return { format: 'HEIF', codec: 'HEVC' };
            case 'heif':
                // Note: .heif extension is commonly HEVC, though could be other codecs
                return { format: 'HEIF', codec: 'HEVC' };
            case 'hmg':
                return { format: 'SlimHEIF', codec: null }; // Determined by brands
            case 'avif':
                return { format: 'AVIF', codec: 'AV1' };
            default:
                return { format: 'Unknown', codec: null };
        }
    }
    
    /**
     * Check if a URL points to a HEIF-based format
     * @param {string} url - File URL
     * @returns {boolean} True if HEIF-based format
     */
    function isHeifFile(url) {
        if (!url) return false;
        var ext = url.toLowerCase().split('.').pop();
        return ['heic', 'heif', 'hmg', 'avif'].indexOf(ext) >= 0;
    }
    
    // ========================================
    // HTTP UTILITIES
    // ========================================
    
    /**
     * Get file size via HTTP HEAD request
     * @param {string} url - File URL
     * @param {function} callback - Called with file size in bytes (0 on error)
     * @param {number} [timeout] - Optional timeout in milliseconds
     */
    function getFileSize(url, callback, timeout) {
        var xhr = new XMLHttpRequest();
        var timeoutId = null;
        var completed = false;
        
        var complete = function(size) {
            if (completed) return;
            completed = true;
            if (timeoutId) clearTimeout(timeoutId);
            callback(size);
        };
        
        xhr.open('HEAD', url, true);
        
        xhr.onload = function() {
            var size = parseInt(xhr.getResponseHeader('Content-Length')) || 0;
            complete(size);
        };
        
        xhr.onerror = function() {
            complete(0);
        };
        
        xhr.ontimeout = function() {
            complete(0);
        };
        
        // Set timeout
        var effectiveTimeout = timeout || DEFAULT_TIMEOUT;
        xhr.timeout = effectiveTimeout;
        
        // Fallback timeout (in case xhr.timeout isn't supported)
        timeoutId = setTimeout(function() {
            if (!completed) {
                xhr.abort();
                complete(0);
            }
        }, effectiveTimeout + 1000);
        
        xhr.send();
    }
    
    /**
     * Get file size via HTTP HEAD request (Promise version)
     * @param {string} url - File URL
     * @param {number} [timeout] - Optional timeout in milliseconds
     * @returns {Promise<number>} Promise resolving to file size in bytes
     */
    function getFileSizeAsync(url, timeout) {
        return new Promise(function(resolve) {
            getFileSize(url, resolve, timeout);
        });
    }
    
    // ========================================
    // ASYNC UTILITIES WITH TIMEOUT
    // ========================================
    
    /**
     * Wrap an async callback-based operation with a timeout
     * @param {function} operation - Function that takes a callback as last argument
     * @param {Array} args - Arguments to pass to operation (excluding callback)
     * @param {number} [timeout] - Timeout in milliseconds
     * @param {*} [timeoutResult] - Result to return on timeout
     * @returns {Promise} Promise that resolves with operation result or rejects on timeout
     */
    function withTimeout(operation, args, timeout, timeoutResult) {
        var effectiveTimeout = timeout || DEFAULT_TIMEOUT;
        
        return new Promise(function(resolve, reject) {
            var completed = false;
            
            var timeoutId = setTimeout(function() {
                if (!completed) {
                    completed = true;
                    if (timeoutResult !== undefined) {
                        resolve(timeoutResult);
                    } else {
                        reject(new Error('Operation timed out after ' + effectiveTimeout + 'ms'));
                    }
                }
            }, effectiveTimeout);
            
            var callback = function(result) {
                if (!completed) {
                    completed = true;
                    clearTimeout(timeoutId);
                    resolve(result);
                }
            };
            
            var allArgs = args.concat([callback]);
            operation.apply(null, allArgs);
        });
    }
    
    /**
     * Create a timeout wrapper for HEIFReader.requestFileInfo
     * @param {HEIFReader} reader - HEIF reader instance
     * @param {number} [timeout] - Timeout in milliseconds
     * @returns {Promise<Object>} Promise resolving to file info
     */
    function requestFileInfoWithTimeout(reader, timeout) {
        return withTimeout(
            function(cb) { reader.requestFileInfo(cb); },
            [],
            timeout,
            { success: false, error: 'Request timed out' }
        );
    }
    
    /**
     * Create a timeout wrapper for ImageProvider.requestImageData
     * @param {ImageProvider} provider - Image provider instance
     * @param {number} contextId - Context identifier
     * @param {Array.<number>} itemIds - Item IDs to decode
     * @param {number} [timeout] - Timeout in milliseconds
     * @returns {Promise<Object>} Promise resolving to decode result
     */
    function requestImageDataWithTimeout(provider, contextId, itemIds, timeout) {
        return withTimeout(
            function(cb) { provider.requestImageData(contextId, itemIds, cb); },
            [],
            timeout,
            { success: false, frames: [], error: 'Decode timed out' }
        );
    }
    
    // ========================================
    // ELEMENT READINESS UTILITIES
    // ========================================
    
    /**
     * Wait for an element to be ready (have dimensions)
     * Uses MutationObserver with polling fallback
     * @param {HTMLElement} element - Element to watch
     * @param {function} callback - Called when element is ready
     * @param {number} [timeout] - Timeout in milliseconds
     */
    function waitForElementReady(element, callback, timeout) {
        var effectiveTimeout = timeout || DEFAULT_TIMEOUT;
        var startTime = Date.now();
        var completed = false;
        var observer = null;
        
        var complete = function(success) {
            if (completed) return;
            completed = true;
            if (observer) {
                observer.disconnect();
            }
            callback(success);
        };
        
        var checkReady = function() {
            if (completed) return false;
            
            // Check for timeout
            if (Date.now() - startTime > effectiveTimeout) {
                complete(false);
                return false;
            }
            
            // Check if element has dimensions
            var isCanvas = element.tagName === 'CANVAS';
            var isImg = element.tagName === 'IMG';
            
            if (isCanvas && element.width > 0 && element.height > 0) {
                complete(true);
                return true;
            }
            
            if (isImg && (element.naturalWidth > 0 || element.width > 0)) {
                complete(true);
                return true;
            }
            
            // Generic element check
            if (!isCanvas && !isImg && element.offsetWidth > 0) {
                complete(true);
                return true;
            }
            
            return false;
        };
        
        // Immediate check
        if (checkReady()) return;
        
        // Try MutationObserver for canvas/img
        if (typeof MutationObserver !== 'undefined') {
            observer = new MutationObserver(function() {
                checkReady();
            });
            
            observer.observe(element, {
                attributes: true,
                attributeFilter: ['width', 'height', 'src']
            });
            
            // Also observe parent for child additions
            if (element.parentElement) {
                observer.observe(element.parentElement, {
                    childList: true,
                    subtree: true
                });
            }
        }
        
        // Polling fallback
        var poll = function() {
            if (!completed && !checkReady()) {
                setTimeout(poll, DEFAULT_POLL_INTERVAL);
            }
        };
        setTimeout(poll, DEFAULT_POLL_INTERVAL);
    }
    
    /**
     * Wait for an element to be ready (Promise version)
     * @param {HTMLElement} element - Element to watch
     * @param {number} [timeout] - Timeout in milliseconds
     * @returns {Promise<boolean>} Promise resolving to true if ready, false if timeout
     */
    function waitForElementReadyAsync(element, timeout) {
        return new Promise(function(resolve) {
            waitForElementReady(element, resolve, timeout);
        });
    }
    
    // ========================================
    // ERROR HANDLING UTILITIES
    // ========================================
    
    /**
     * Create an error result object with consistent structure
     * @param {string} message - Error message
     * @param {Error} [originalError] - Original error object
     * @returns {{success: boolean, error: string, originalError: Error|null}}
     */
    function createErrorResult(message, originalError) {
        return {
            success: false,
            error: message,
            originalError: originalError || null
        };
    }
    
    /**
     * Log error with context and return error result
     * @param {string} context - Context description (e.g., function name)
     * @param {string} message - Error message
     * @param {Error} [error] - Original error
     * @returns {{success: boolean, error: string}}
     */
    function logAndCreateError(context, message, error) {
        var fullMessage = '[' + context + '] ' + message;
        if (error) {
            console.error(fullMessage, error);
        } else {
            console.error(fullMessage);
        }
        return createErrorResult(message, error);
    }
    
    // ========================================
    // COLOR SPACE UTILITIES
    // ========================================
    
    /**
     * Get human-readable color space name from COLR property
     * Based on ITU-T H.273 / ISO/IEC 23091-2
     * @param {Object} colrProp - COLR property object
     * @returns {string} Human-readable color space name
     */
    function getColorSpaceName(colrProp) {
        if (!colrProp) return 'Unknown';
        
        var primaries = colrProp.colourPrimaries;
        var matrix = colrProp.matrixCoefficients;
        var fullRange = colrProp.fullRangeFlag;
        
        var colorSpace = '';
        
        // Determine color primaries name
        switch (primaries) {
            case 1: colorSpace = 'BT.709'; break;
            case 4: colorSpace = 'BT.470M'; break;
            case 5: colorSpace = 'BT.601 (625)'; break;
            case 6: colorSpace = 'BT.601 (525)'; break;
            case 9: colorSpace = 'BT.2020'; break;
            case 10: colorSpace = 'XYZ'; break;
            case 11: colorSpace = 'DCI-P3'; break;
            case 12: colorSpace = 'Display P3'; break;
            default: colorSpace = 'Custom (' + primaries + ')'; break;
        }
        
        // Add matrix coefficients info if different from primaries default
        if (matrix === 9 || matrix === 10) {
            if (colorSpace.indexOf('BT.2020') === -1) {
                colorSpace += ' (Matrix: BT.2020)';
            }
        }
        
        // Add range info
        colorSpace += fullRange ? ' Full' : ' Limited';
        
        return colorSpace;
    }
    
    // ========================================
    // ICC PROFILE PARSING
    // ========================================
    
    /**
     * ICC Profile header offsets and tag signatures
     * Based on ICC.1:2022 specification
     */
    var ICC_CONSTANTS = {
        HEADER_SIZE: 128,
        TAG_TABLE_OFFSET: 128,
        
        // Header field offsets
        PROFILE_SIZE: 0,
        CMM_TYPE: 4,
        VERSION: 8,
        DEVICE_CLASS: 12,
        COLOR_SPACE: 16,
        PCS: 20,
        DATE_TIME: 24,
        SIGNATURE: 36,  // 'acsp'
        PRIMARY_PLATFORM: 40,
        FLAGS: 44,
        DEVICE_MANUFACTURER: 48,
        DEVICE_MODEL: 52,
        DEVICE_ATTRIBUTES: 56,
        RENDERING_INTENT: 64,
        PCS_ILLUMINANT: 68,
        CREATOR: 80,
        PROFILE_ID: 84,
        
        // Common tag signatures
        TAG_DESC: 'desc',      // Profile description
        TAG_CPRT: 'cprt',      // Copyright
        TAG_WTPT: 'wtpt',      // White point
        TAG_CHAD: 'chad',      // Chromatic adaptation
        TAG_RXYY: 'rXYZ',      // Red colorant XYZ
        TAG_GXYY: 'gXYZ',      // Green colorant XYZ  
        TAG_BXYY: 'bXYZ',      // Blue colorant XYZ
        TAG_RTRC: 'rTRC',      // Red tone reproduction curve
        TAG_GTRC: 'gTRC',      // Green tone reproduction curve
        TAG_BTRC: 'bTRC'       // Blue tone reproduction curve
    };
    
    /**
     * Parse ICC profile header and identify the color space
     * @param {Array|Uint8Array} iccBytes - ICC profile byte array
     * @returns {Object} Parsed ICC profile info
     */
    function parseICCProfile(iccBytes) {
        if (!iccBytes || iccBytes.length < ICC_CONSTANTS.HEADER_SIZE) {
            return { valid: false, error: 'Invalid or too small ICC profile' };
        }
        
        // Convert to Uint8Array if needed
        var bytes = (iccBytes instanceof Uint8Array) ? iccBytes : new Uint8Array(iccBytes);
        var view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
        
        // Verify 'acsp' signature at offset 36
        var signature = String.fromCharCode(bytes[36], bytes[37], bytes[38], bytes[39]);
        if (signature !== 'acsp') {
            return { valid: false, error: 'Invalid ICC signature: ' + signature };
        }
        
        // Read profile size
        var profileSize = view.getUint32(ICC_CONSTANTS.PROFILE_SIZE, false);
        
        // Read color space (4 chars at offset 16)
        var colorSpaceSig = String.fromCharCode(
            bytes[16], bytes[17], bytes[18], bytes[19]
        ).trim();
        
        // Read device class (4 chars at offset 12)
        var deviceClass = String.fromCharCode(
            bytes[12], bytes[13], bytes[14], bytes[15]
        ).trim();
        
        // Read version
        var majorVersion = bytes[8];
        var minorVersion = (bytes[9] >> 4) & 0xF;
        
        // Read profile description tag if present
        var description = _extractICCDescription(bytes);
        
        // Identify known profiles
        var identified = _identifyICCProfile(bytes, description, colorSpaceSig);
        
        return {
            valid: true,
            profileSize: profileSize,
            colorSpace: colorSpaceSig,
            deviceClass: deviceClass,
            version: majorVersion + '.' + minorVersion,
            description: description,
            identified: identified
        };
    }
    
    /**
     * Extract description string from ICC profile
     * @private
     */
    function _extractICCDescription(bytes) {
        if (bytes.length < ICC_CONSTANTS.HEADER_SIZE + 4) return null;
        
        var view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
        var tagCount = view.getUint32(ICC_CONSTANTS.TAG_TABLE_OFFSET, false);
        
        // Search for 'desc' tag
        for (var i = 0; i < tagCount && i < 100; i++) {
            var tagOffset = ICC_CONSTANTS.TAG_TABLE_OFFSET + 4 + i * 12;
            if (tagOffset + 12 > bytes.length) break;
            
            var tagSig = String.fromCharCode(
                bytes[tagOffset], bytes[tagOffset + 1], 
                bytes[tagOffset + 2], bytes[tagOffset + 3]
            );
            
            if (tagSig === 'desc') {
                var dataOffset = view.getUint32(tagOffset + 4, false);
                var dataSize = view.getUint32(tagOffset + 8, false);
                
                if (dataOffset + dataSize <= bytes.length && dataSize > 12) {
                    // Read description type signature
                    var typeSig = String.fromCharCode(
                        bytes[dataOffset], bytes[dataOffset + 1],
                        bytes[dataOffset + 2], bytes[dataOffset + 3]
                    );
                    
                    if (typeSig === 'desc') {
                        // TextDescriptionType (ICC v2)
                        var strLen = view.getUint32(dataOffset + 8, false);
                        if (strLen > 0 && dataOffset + 12 + strLen <= bytes.length) {
                            var chars = [];
                            for (var j = 0; j < strLen - 1 && j < 256; j++) {
                                var c = bytes[dataOffset + 12 + j];
                                if (c === 0) break;
                                chars.push(String.fromCharCode(c));
                            }
                            return chars.join('');
                        }
                    } else if (typeSig === 'mluc') {
                        // MultiLocalizedUnicodeType (ICC v4)
                        var recordCount = view.getUint32(dataOffset + 8, false);
                        if (recordCount > 0 && dataOffset + 16 + 12 <= bytes.length) {
                            var strOffset = view.getUint32(dataOffset + 20, false);
                            var strLength = view.getUint32(dataOffset + 16, false);
                            
                            if (dataOffset + strOffset + strLength <= bytes.length) {
                                var chars = [];
                                // Read as UTF-16BE
                                for (var j = 0; j < strLength && j < 512; j += 2) {
                                    var c = view.getUint16(dataOffset + strOffset + j, false);
                                    if (c === 0) break;
                                    chars.push(String.fromCharCode(c));
                                }
                                return chars.join('');
                            }
                        }
                    }
                }
                break;
            }
        }
        return null;
    }
    
    /**
     * Identify known ICC profiles by description or MD5
     * @private
     */
    function _identifyICCProfile(bytes, description, colorSpaceSig) {
        // Only handle RGB profiles for canvas color space mapping
        if (colorSpaceSig !== 'RGB') {
            return { 
                canvasColorSpace: 'srgb',
                name: 'Non-RGB (' + colorSpaceSig + ')',
                isWideGamut: false,
                yuvMatrix: 'BT709'
            };
        }
        
        // Check description for known profiles
        var descLower = (description || '').toLowerCase();
        
        // Display P3 variants
        if (descLower.indexOf('display p3') !== -1 || 
            descLower.indexOf('p3 d65') !== -1 ||
            descLower === 'p3') {
            return { 
                canvasColorSpace: 'display-p3', 
                name: 'Display P3',
                isWideGamut: true,
                yuvMatrix: 'BT709'  // P3 uses BT.709-derived matrix
            };
        }
        
        // DCI-P3 (theater standard, different white point)
        if (descLower.indexOf('dci-p3') !== -1 || descLower.indexOf('dci p3') !== -1) {
            return { 
                canvasColorSpace: 'display-p3',  // Best wide-gamut canvas approximation
                name: 'DCI-P3',
                isWideGamut: true,
                yuvMatrix: 'BT709',
                note: 'DCI-P3 white point differs from D65; rendered into the display-p3 canvas as the closest available wide-gamut surface'
            };
        }
        
        // sRGB variants
        if (descLower.indexOf('srgb') !== -1 || 
            descLower.indexOf('iec61966-2.1') !== -1 ||
            descLower.indexOf('iec 61966-2.1') !== -1) {
            return { 
                canvasColorSpace: 'srgb', 
                name: 'sRGB',
                isWideGamut: false,
                yuvMatrix: 'BT709'
            };
        }
        
        // Adobe RGB
        if (descLower.indexOf('adobe rgb') !== -1 || 
            descLower.indexOf('adobergb') !== -1) {
            return { 
                canvasColorSpace: 'srgb', 
                name: 'Adobe RGB (1998)',
                isWideGamut: true,
                yuvMatrix: 'BT709',
                note: 'Adobe RGB not supported in Canvas API, using sRGB'
            };
        }
        
        // ProPhoto RGB / ROMM RGB
        if (descLower.indexOf('prophoto') !== -1 || 
            descLower.indexOf('romm rgb') !== -1) {
            return { 
                canvasColorSpace: 'srgb', 
                name: 'ProPhoto RGB',
                isWideGamut: true,
                yuvMatrix: 'BT709',
                note: 'ProPhoto RGB not supported in Canvas API, using sRGB'
            };
        }
        
        // Rec.2020 / BT.2020 - check broadly for '2020' to catch various naming conventions
        if (descLower.indexOf('2020') !== -1) {
            // Canvas API doesn't directly support Rec.2020
            // Display P3 is closer than sRGB
            return { 
                canvasColorSpace: 'display-p3', 
                name: 'Rec. 2020',
                isWideGamut: true,
                yuvMatrix: 'BT2020',  // Add hint for YUV matrix selection
                note: 'Rec.2020 approximated with Display P3'
            };
        }
        
        // Unknown profile - default to sRGB
        return { 
            canvasColorSpace: 'srgb', 
            name: description || 'Unknown',
            isWideGamut: false,
            yuvMatrix: 'BT709'
        };
    }
    
    // ========================================
    // CANVAS COLOR SPACE MANAGEMENT
    // ========================================
    
    /**
     * Check if Display P3 canvas is supported
     * @returns {boolean} True if display-p3 is supported
     */
    var _p3Supported = null;
    function isDisplayP3Supported() {
        if (_p3Supported !== null) return _p3Supported;
        
        try {
            var testCanvas = document.createElement('canvas');
            testCanvas.width = 1;
            testCanvas.height = 1;
            var ctx = testCanvas.getContext('2d', { colorSpace: 'display-p3' });
            _p3Supported = ctx && ctx.getContextAttributes && 
                           ctx.getContextAttributes().colorSpace === 'display-p3';
        } catch (e) {
            _p3Supported = false;
        }
        return _p3Supported;
    }
    
    /**
     * Single source of truth for "is this transfer characteristic HDR?".
     * Per ITU-T H.273:
     *   16 = SMPTE ST 2084 (PQ)
     *   18 = ARIB STD-B67 (HLG)
     * Everything else is treated as SDR (BT.709 = 1, sRGB = 13, etc).
     * @param {number|undefined} transfer
     * @returns {boolean}
     */
    function isHDRTransfer(transfer) {
        return transfer === 16 || transfer === 18;
    }

    /**
     * Single source of truth for "is this color metadata HDR?".
     *
     * Replaces three near-duplicate heuristics that previously lived in
     * gallery.js (`_isHDRContent`), decoder-manager.js (`_isImageHDR`)
     * and inline checks elsewhere - each with slightly different rules.
     *
     * Decision order (most authoritative first):
     *   1. NCLX transfer characteristics 16/18 -> HDR (definitive).
     *   2. NCLX BT.2020 primaries (9) AND bit depth >= 10 AND transfer is
     *      not explicitly SDR (1=BT.709, 13=sRGB) -> HDR. This catches
     *      files that omit a transfer characteristic but are 10-bit BT.2020.
     *   3. ICC profile description matches a curated list of HDR markers,
     *      AND the ICC color space signature is RGB. Substring matching
     *      is intentionally conservative - the previous version matched
     *      bare "hdr" anywhere in the description, hitting profiles like
     *      "Adobe RGB HDR ramp test" and similar non-HDR samples.
     *
     * @param {Object} colorInfo - {colourType, colourPrimaries, transferCharacteristics, iccProfile}
     * @param {number} [bitDepth] - Optional bit depth hint (used by rule 2)
     * @returns {boolean}
     */
    function isHDRContent(colorInfo, bitDepth) {
        if (!colorInfo) return false;

        // Rule 1: NCLX PQ/HLG
        if (isHDRTransfer(colorInfo.transferCharacteristics)) return true;

        // Rule 2: NCLX BT.2020 + 10-bit + transfer not explicitly SDR
        if (colorInfo.colourPrimaries === 9 && bitDepth && bitDepth >= 10) {
            var t = colorInfo.transferCharacteristics;
            if (t !== 1 && t !== 13) return true;
        }

        // Rule 3: ICC name match (RGB profiles only, curated list).
        if (colorInfo.iccProfile && colorInfo.iccProfile.length > 0) {
            try {
                var parsed = parseICCProfile(colorInfo.iccProfile);
                if (parsed.valid && parsed.colorSpace === 'RGB') {
                    var hayLower = ((parsed.description || '') + ' ' +
                                    ((parsed.identified && parsed.identified.name) || '')).toLowerCase();
                    // Curated needles, ordered by specificity. Bare 'hdr'
                    // is excluded - too prone to false positives.
                    var needles = ['rec.2100', 'rec2100', 'bt.2100', 'bt2100',
                                   'st 2084', 'st2084', 'smpte 2084', 'smpte2084',
                                   ' pq ', '(pq)', 'pq encoded', 'perceptual quantizer',
                                   ' hlg ', '(hlg)', 'hybrid log-gamma', 'arib std-b67'];
                    for (var n = 0; n < needles.length; n++) {
                        if (hayLower.indexOf(needles[n]) !== -1) return true;
                    }
                    // Allow word-boundary 'hdr' only if also accompanied by
                    // 'pq', 'hlg' or '2100' to filter out the worst false positives.
                    if (/\bhdr\b/.test(hayLower) &&
                        /(pq|hlg|2100)/.test(hayLower)) return true;
                }
            } catch (e) { /* malformed ICC - treat as SDR */ }
        }

        return false;
    }

    /**
     * Determine canvas color space from COLR property (nclx or ICC)
     *
     * IMPORTANT: browser canvas-2d only supports two colorSpace values today:
     *   - 'srgb' (default)
     *   - 'display-p3' (where supported, feature-detected via _p3Supported)
     * There is no 'rec2020', 'bt2100', 'pq', or 'hlg' canvas. Stable proposals
     * for HDR canvas (rec2100-pq / rec2100-hlg with unorm16 storage) are
     * Chromium-only behind flags as of 2026 and not used here.
     *
     * Consequence: a true wide-gamut/HDR source CANNOT be displayed
     * faithfully on canvas. Two honest fallbacks:
     *   - Source's primaries match a supported canvas space (sRGB-ish or P3):
     *     use that space.
     *   - Source is BT.2020/BT.2100 PQ/HLG: render as 'srgb' with a clear
     *     note. The previous code returned 'display-p3' here "as the closest
     *     match", but that is colorimetrically wrong - the decoded RGBA
     *     bytes are BT.2020 primaries (often PQ-encoded) crammed into 0-255,
     *     and labelling them P3 makes the browser apply a P3-to-display
     *     transform on top, producing visibly-incorrect oversaturated
     *     output. Truthful sRGB is no more correct gamut-wise but at least
     *     does not double-transform. The HDR badge in the modal builds a
     *     plain <img> for the OS to render natively when the user wants
     *     real HDR.
     *
     * @param {Object} colorInfo - Color info from item.color
     * @returns {Object} { colorSpace, name, isWideGamut, note?, isHDR? }
     */
    function getCanvasColorSpace(colorInfo) {
        if (!colorInfo) {
            return { colorSpace: 'srgb', name: 'sRGB (default)', isWideGamut: false };
        }
        
        // Handle ICC profile (rICC or prof)
        if ((colorInfo.colourType === 'rICC' || colorInfo.colourType === 'prof') && 
            colorInfo.iccProfile && colorInfo.iccProfile.length > 0) {
            
            var iccInfo = parseICCProfile(colorInfo.iccProfile);
            if (iccInfo.valid && iccInfo.identified) {
                var rec = iccInfo.identified;
                // Check if recommended color space is actually supported
                if (rec.canvasColorSpace === 'display-p3' && !isDisplayP3Supported()) {
                    return {
                        colorSpace: 'srgb',
                        name: rec.name + ' (P3 not supported)',
                        isWideGamut: rec.isWideGamut,
                        note: 'Display P3 not supported, using sRGB'
                    };
                }
                return {
                    colorSpace: rec.canvasColorSpace,
                    name: rec.name,
                    isWideGamut: rec.isWideGamut,
                    note: rec.note
                };
            }
        }
        
        // Handle NCLX color type
        if (colorInfo.colourType === 'nclx') {
            var primaries = colorInfo.colourPrimaries;
            var transfer = colorInfo.transferCharacteristics;
            var isPQorHLG = isHDRTransfer(transfer);

            // Map NCLX colour_primaries to canvas color space
            // Values from ITU-T H.273 / ISO/IEC 23091-2
            switch (primaries) {
                case 1:  // BT.709
                    return { colorSpace: 'srgb', name: 'BT.709', isWideGamut: false };
                    
                case 5:  // BT.601 (625 line)
                case 6:  // BT.601 (525 line)  
                    return { colorSpace: 'srgb', name: 'BT.601', isWideGamut: false };
                    
                case 9:  // BT.2020 / BT.2100
                    // Honest fallback: see comment block above. We do NOT
                    // claim this can be approximated by Display P3 - the
                    // primaries don't match and (for PQ/HLG transfers)
                    // the encoding doesn't either. Render as sRGB and let
                    // the modal's HDR badge route the user to native
                    // <img> rendering for true HDR display.
                    return {
                        colorSpace: 'srgb',
                        name: isPQorHLG
                            ? 'BT.2020 ' + (transfer === 16 ? 'PQ' : 'HLG') + ' (canvas preview only)'
                            : 'BT.2020 (canvas preview only)',
                        isWideGamut: true,
                        isHDR: isPQorHLG,
                        note: 'Canvas cannot render BT.2020/HDR faithfully; use the HDR badge for native rendering'
                    };
                    
                case 11: // DCI-P3 (theatrical, different white point)
                case 12: // Display P3 (D65)
                    if (isDisplayP3Supported()) {
                        return { 
                            colorSpace: 'display-p3', 
                            name: primaries === 12 ? 'Display P3' : 'DCI-P3',
                            isWideGamut: true 
                        };
                    }
                    return { colorSpace: 'srgb', name: 'P3 (via sRGB)', isWideGamut: true };
                    
                default:
                    return { colorSpace: 'srgb', name: 'Unknown (' + primaries + ')', isWideGamut: false };
            }
        }
        
        // Fallback
        return { colorSpace: 'srgb', name: 'sRGB (default)', isWideGamut: false };
    }
    
    /**
     * Create a canvas with the appropriate color space for the given color info
     * @param {number} width - Canvas width
     * @param {number} height - Canvas height  
     * @param {Object} [colorInfo] - Color info from item.color
     * @returns {Object} { canvas, ctx, colorSpaceInfo }
     */
    function createColorManagedCanvas(width, height, colorInfo) {
        var csInfo = getCanvasColorSpace(colorInfo);
        var canvas = document.createElement('canvas');
        canvas.width = width;
        canvas.height = height;
        
        var ctx;
        if (csInfo.colorSpace === 'display-p3' && isDisplayP3Supported()) {
            try {
                ctx = canvas.getContext('2d', { colorSpace: 'display-p3' });
            } catch (e) {
                console.warn('Failed to create P3 canvas:', e);
                ctx = canvas.getContext('2d');
                csInfo.colorSpace = 'srgb';
                csInfo.note = 'P3 canvas creation failed, using sRGB';
            }
        } else {
            ctx = canvas.getContext('2d');
        }
        
        // Store color space info on canvas for reference
        canvas.dataset.colorSpace = csInfo.colorSpace;
        canvas.dataset.colorSpaceName = csInfo.name;
        
        return {
            canvas: canvas,
            ctx: ctx,
            colorSpaceInfo: csInfo
        };
    }
    
    // ========================================
    // RESOURCE CLEANUP UTILITIES
    // ========================================
    
    /**
     * Safely dispose of a resource
     * @param {Object} resource - Resource with dispose() method
     * @param {string} [name] - Optional name for error logging
     */
    function safeDispose(resource, name) {
        if (resource && typeof resource.dispose === 'function') {
            try {
                resource.dispose();
            } catch (e) {
                console.error('Error disposing ' + (name || 'resource') + ':', e);
            }
        }
    }
    
    /**
     * Create a cleanup tracker for managing multiple resources
     * @returns {{add: function, dispose: function, disposeAll: function}}
     */
    function createCleanupTracker() {
        var resources = [];
        
        return {
            /**
             * Add a resource to track
             * @param {Object} resource - Resource with dispose() method
             * @param {string} [name] - Optional name for logging
             */
            add: function(resource, name) {
                resources.push({ resource: resource, name: name });
            },
            
            /**
             * Dispose a specific resource
             * @param {Object} resource - Resource to dispose
             */
            dispose: function(resource) {
                for (var i = resources.length - 1; i >= 0; i--) {
                    if (resources[i].resource === resource) {
                        safeDispose(resources[i].resource, resources[i].name);
                        resources.splice(i, 1);
                        break;
                    }
                }
            },
            
            /**
             * Dispose all tracked resources
             */
            disposeAll: function() {
                for (var i = resources.length - 1; i >= 0; i--) {
                    safeDispose(resources[i].resource, resources[i].name);
                }
                resources = [];
            }
        };
    }
    
    // ========================================
    // PUBLIC API
    // ========================================
    
    return {
        // Configuration
        DEFAULT_TIMEOUT: DEFAULT_TIMEOUT,
        DEFAULT_POLL_INTERVAL: DEFAULT_POLL_INTERVAL,
        
        // Brand analysis (AUTHORITATIVE for file type/codec decisions)
        BRANDS: BRANDS,
        analyzeBrands: analyzeBrands,
        
        // File size
        formatFileSize: formatFileSize,
        
        // DOM helpers
        createSeparator: createSeparator,
        
        // Format detection (legacy - use analyzeBrands for new code)
        deriveFormatAndCodec: deriveFormatAndCodec,
        deriveFormatFromExtension: deriveFormatFromExtension,
        isHeifFile: isHeifFile,
        
        // HTTP utilities
        getFileSize: getFileSize,
        getFileSizeAsync: getFileSizeAsync,
        
        // Async utilities
        withTimeout: withTimeout,
        requestFileInfoWithTimeout: requestFileInfoWithTimeout,
        requestImageDataWithTimeout: requestImageDataWithTimeout,
        
        // Element readiness
        waitForElementReady: waitForElementReady,
        waitForElementReadyAsync: waitForElementReadyAsync,
        
        // Error handling
        createErrorResult: createErrorResult,
        logAndCreateError: logAndCreateError,
        
        // Color space
        getColorSpaceName: getColorSpaceName,
        
        // ICC Profile parsing
        parseICCProfile: parseICCProfile,
        
        // Canvas color management
        isDisplayP3Supported: isDisplayP3Supported,
        getCanvasColorSpace: getCanvasColorSpace,
        createColorManagedCanvas: createColorManagedCanvas,

        // HDR detection (single source of truth, used by gallery.js + decoder-manager.js)
        isHDRTransfer: isHDRTransfer,
        isHDRContent: isHDRContent,

        // Resource cleanup
        safeDispose: safeDispose,
        createCleanupTracker: createCleanupTracker
    };
})();

// Export for module systems if available
if (typeof module !== 'undefined' && module.exports) {
    module.exports = HEIFUtils;
}
