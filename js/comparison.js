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

function Comparison () {
    var self = this;

    this._imageComparisonMapping = {
        "comparison-0": {
            heicUrl: "content/unsplash/bricksQP26.heif",
            jpgUrl: "content/unsplash/bricks.jpg",
            heicText: "HEIF: 2468 KiB",
            jpgText: "JPG: 5170 KiB"
        },
        "comparison-1": {
            heicUrl: "content/nokia/compare_still_1.heic",
            jpgUrl: "content/nokia/compare_still_1.jpg",
            heicText: "HEIF: 38 KiB",
            jpgText: "JPG: 49 KiB"
        },
        "comparison-2": {
            heicUrl: "content/nokia/compare_still_2.heic",
            jpgUrl: "content/nokia/compare_still_2.jpg",
            heicText: "HEIF: 50 KiB",
            jpgText: "JPG: 52 KiB"
        },
        "comparison-3": {
            heicUrl: "content/nokia/compare_still_3.heic",
            jpgUrl: "content/nokia/compare_still_3.jpg",
            heicText: "HEIF: 115 KiB",
            jpgText: "JPG: 120 KiB"
        },
        "comparison-5": {
            heicUrl: "content/conformance/C83_dup.hmg",
            jpgUrl: "content/nokia/XD64.heic",
            heicText: "SlimHEIF: 971 Bytes",
            jpgText: "HEIC: 1302 Bytes",
            nativeSize: true,
            noZoom: true  // Disable zoom for tiny images
        },
        "comparison-6": {
            heicUrl: "content/nokia/meadow.heif",
            jpgUrl: "content/nokia/meadow.heic",
            heicText: "",
            jpgText: ""
        }
    };



    /** Fetch + open + snapshot a HEIF URL using the modern HEIF.load()
     *  flow. Returns a Promise<{fileInfo}>. The reader is closed before
     *  resolution — every field comparison.js displays is closed-reader-
     *  invariant and lives on the snapshot. */
    this._fetchHeifSnapshot = function(url) {
        if (typeof HEIF === 'undefined' || typeof HEIF.load !== 'function') {
            return Promise.reject(new Error('heif.js / heif-raw.js not loaded'));
        }
        return HEIF.load().then(function(heif) {
            return fetch(url).then(function(resp) {
                if (!resp.ok) throw new Error('fetch ' + url + ' -> HTTP ' + resp.status);
                return resp.arrayBuffer();
            }).then(function(buf) {
                var reader = heif.open(new Uint8Array(buf));
                try {
                    return { fileInfo: buildFileInfo(reader, url) };
                } finally {
                    reader.close();
                }
            });
        });
    };

    /** Build a meta object from an existing fileInfo snapshot, reusing
     *  the data the decoder already parsed (avoids a redundant fetch). */
    this._metaFromFileInfo = function(fileInfo, el) {
        var meta = {
            width: 0, height: 0, bitDepth: 8,
            codec: 'Unknown', format: 'Unknown',
            brands: [], frameCount: 1, hasThumbnail: false
        };

        meta.brands = fileInfo.compatibleBrands || [];
        var derivedInfo = HEIFUtils.deriveFormatAndCodec(meta.brands);
        meta.format = derivedInfo.format;
        meta.codec  = derivedInfo.codec;

        if (fileInfo.rootLevelMetaBoxProperties) {
            var ifMap = fileInfo.rootLevelMetaBoxProperties.imageFeaturesMap;
            var picked = null;
            for (var itemId in ifMap) {
                if (!ifMap.hasOwnProperty(itemId)) continue;
                var entry = ifMap[itemId];
                if (entry.isMaster) { picked = entry; break; }
                if (!picked && entry.isPrimary) picked = entry;
            }
            if (picked) {
                meta.width  = picked.width  || meta.width;
                meta.height = picked.height || meta.height;
                if (picked.thmbRefs && picked.thmbRefs.length > 0) meta.hasThumbnail = true;
                if (picked.properties && picked.properties.pixi && picked.properties.pixi.length > 0) {
                    meta.bitDepth = picked.properties.pixi[0];
                }
            }
        }

        if (fileInfo.trackProperties && fileInfo.trackProperties.length > 0) {
            for (var t = 0; t < fileInfo.trackProperties.length; t++) {
                if (fileInfo.trackProperties[t].sampleCount > 1) {
                    meta.frameCount = fileInfo.trackProperties[t].sampleCount;
                    break;
                }
            }
        }

        if (!meta.width && el) meta.width = el.naturalWidth || el.width;
        if (!meta.height && el) meta.height = el.naturalHeight || el.height;

        return meta;
    };

    /** Extract metadata from HEIF file */
    this._extractHeifMetadata = function(src, callback) {
        if (!src) {
            callback({ width: 0, height: 0, format: 'Unknown', codec: 'Unknown' });
            return;
        }

        self._fetchHeifSnapshot(src).then(function(result) {
            var fileInfo = result.fileInfo;
            var meta = {
                width: 0,
                height: 0,
                bitDepth: 8,
                codec: 'Unknown',
                format: 'Unknown',
                brands: [],
                frameCount: 1,
                hasThumbnail: false
            };

            meta.brands = fileInfo.compatibleBrands || [];
            var derivedInfo = HEIFUtils.deriveFormatAndCodec(meta.brands);
            meta.format = derivedInfo.format;
            meta.codec  = derivedInfo.codec;

            // Primary master image: width / height / bitDepth / thumb-presence.
            // Snapshot's imageFeaturesMap entries carry .isMaster (modern
            // field name; legacy was .isMasterImage), .width, .height,
            // .thmbRefs, and .properties.{ispe,pixi,colr,...}.
            if (fileInfo.rootLevelMetaBoxProperties) {
                var ifMap = fileInfo.rootLevelMetaBoxProperties.imageFeaturesMap;
                for (var itemId in ifMap) {
                    if (!ifMap.hasOwnProperty(itemId)) continue;
                    var entry = ifMap[itemId];
                    if (!entry.isMaster) continue;
                    meta.width  = entry.width  || meta.width;
                    meta.height = entry.height || meta.height;
                    if (entry.thmbRefs && entry.thmbRefs.length > 0) {
                        meta.hasThumbnail = true;
                    }
                    if (entry.properties && entry.properties.pixi && entry.properties.pixi.length > 0) {
                        meta.bitDepth = entry.properties.pixi[0];
                    }
                    break;
                }
            }

            // Frame count for image sequences: snapshot exposes
            // sampleCount per track directly (the legacy POD's per-sample
            // sampleProperties map is not rebuilt eagerly).
            if (fileInfo.trackProperties && fileInfo.trackProperties.length > 0) {
                for (var t = 0; t < fileInfo.trackProperties.length; t++) {
                    var track = fileInfo.trackProperties[t];
                    if (track.sampleCount > 1) {
                        meta.frameCount = track.sampleCount;
                        break;
                    }
                }
            }

            callback(meta);
        }).catch(function(err) {
            console.error('comparison: HEIF snapshot for ' + src + ' failed', err);
            callback({ width: 0, height: 0, format: 'Unknown', codec: 'Unknown',
                       brands: [], frameCount: 1, hasThumbnail: false });
        });
    };

    /** Build info panel for comparison item - updated to show format */
    this._buildComparisonInfoPanel = function(containerId, isHeif, meta, fileSize, labelText) {
        var container = document.createElement('div');
        container.className = 'heif-comparison-info-panel';
        container.id = containerId + '-info';
        
        var details = document.createElement('div');
        details.className = 'heif-comparison-info-details';
        
        var hasInfo = false;
        
        // Format and Codec
        if (meta.format && meta.format !== 'Unknown') {
            var formatItem = document.createElement('span');
            formatItem.className = 'info-item info-format';
            var formatText = meta.format;
            if (meta.codec && meta.codec !== 'Unknown') {
                formatText += ' | ' + meta.codec;
            }
            formatItem.textContent = formatText;
            details.appendChild(formatItem);
            hasInfo = true;
        }
        
        // Dimensions
        if (meta.width && meta.height) {
            if (hasInfo) details.appendChild(HEIFUtils.createSeparator());
            var dimItem = document.createElement('span');
            dimItem.className = 'info-item';
            dimItem.textContent = meta.width + '×' + meta.height;
            details.appendChild(dimItem);
            hasInfo = true;
        }
        
        // Bit depth (if not 8)
        if (meta.bitDepth && meta.bitDepth !== 8) {
            if (hasInfo) details.appendChild(HEIFUtils.createSeparator());
            var bitItem = document.createElement('span');
            bitItem.className = 'info-item';
            bitItem.textContent = meta.bitDepth + '-bit';
            details.appendChild(bitItem);
            hasInfo = true;
        }
        
        // Frame count (for sequences)
        if (meta.frameCount > 1) {
            if (hasInfo) details.appendChild(HEIFUtils.createSeparator());
            var frameItem = document.createElement('span');
            frameItem.className = 'info-item';
            frameItem.textContent = meta.frameCount + ' frames';
            details.appendChild(frameItem);
            hasInfo = true;
        }
        
        // File size
        if (fileSize) {
            if (hasInfo) details.appendChild(HEIFUtils.createSeparator());
            var sizeItem = document.createElement('span');
            sizeItem.className = 'info-item';
            sizeItem.textContent = HEIFUtils.formatFileSize(fileSize);
            details.appendChild(sizeItem);
            hasInfo = true;
        }
        
        // If no info available
        if (!hasInfo) {
            var noInfo = document.createElement('span');
            noInfo.className = 'info-item info-none';
            noInfo.textContent = 'No details available';
            details.appendChild(noInfo);
        }
        
        // Info button for HEIF files with additional data
        if (isHeif && meta.brands && meta.brands.length > 0) {
            var infoBtn = document.createElement('span');
            infoBtn.className = 'heif-comparison-info-btn';
            infoBtn.textContent = 'ⓘ';
            infoBtn.title = 'Show full details';
            infoBtn.onclick = function(e) {
                e.stopPropagation();
                self._toggleComparisonFullInfo(containerId);
            };
            details.appendChild(infoBtn);
            
            var fullInfo = self._buildComparisonFullInfoPanel(containerId, meta);
            container.style.position = 'relative';
            container.appendChild(fullInfo);
        }
        
        container.appendChild(details);
        return container;
    };

    /** Build full info panel - updated to show format/codec */
    this._buildComparisonFullInfoPanel = function(containerId, meta) {
        var container = document.createElement('div');
        container.className = 'heif-comparison-full-info';
        container.id = containerId + '-full-info';
        
        var title = document.createElement('h4');
        title.textContent = 'File Details';
        container.appendChild(title);
        
        var grid = document.createElement('div');
        grid.className = 'heif-comparison-full-info-grid';
        
        var addRow = function(key, value) {
            if (!value) return;
            var keyEl = document.createElement('span');
            keyEl.className = 'info-key';
            keyEl.textContent = key + ':';
            grid.appendChild(keyEl);
            
            var valEl = document.createElement('span');
            valEl.className = 'info-value';
            valEl.textContent = value;
            grid.appendChild(valEl);
        };
        
        if (meta.format) addRow('Format', meta.format);
        if (meta.codec) addRow('Codec', meta.codec);
        addRow('Dimensions', meta.width + ' × ' + meta.height + ' px');
        if (meta.bitDepth) addRow('Bit Depth', meta.bitDepth + ' bits/channel');
        if (meta.frameCount > 1) addRow('Frames', meta.frameCount);
        if (meta.hasThumbnail) addRow('Thumbnail', 'Embedded');
        if (meta.brands && meta.brands.length > 0) addRow('Brands', meta.brands.join(', '));
        
        container.appendChild(grid);
        
        container.addEventListener('click', function(e) {
            e.stopPropagation();
        });
        
        return container;
    };

    /** Toggle full info visibility */
    this._toggleComparisonFullInfo = function(containerId) {
        var fullInfo = document.getElementById(containerId + '-full-info');
        if (fullInfo) {
            var isVisible = fullInfo.classList.toggle('visible');
            
            // Store current container ID for the click handler
            if (isVisible) {
                // Close any other open panels first
                var allPanels = document.querySelectorAll('.heif-comparison-full-info.visible, .sequences-comparison-info-panel .heif-comparison-full-info.visible');
                allPanels.forEach(function(panel) {
                    if (panel.id !== containerId + '-full-info') {
                        panel.classList.remove('visible');
                    }
                });
                
                // Add click-outside listener with delay to prevent immediate close
                setTimeout(function() {
                    self._activeFullInfoId = containerId + '-full-info';
                    document.addEventListener('click', self._handleClickOutsideComparisonInfo);
                }, 10);
            } else {
                document.removeEventListener('click', self._handleClickOutsideComparisonInfo);
                self._activeFullInfoId = null;
            }
        }
    };
    
    /** Handle click outside comparison full info panel */
    this._handleClickOutsideComparisonInfo = function(e) {
        if (!self._activeFullInfoId) return;
        
        var fullInfo = document.getElementById(self._activeFullInfoId);
        
        // Find the info button that opened this panel
        var infoBtn = null;
        var panel = fullInfo ? fullInfo.parentElement : null;
        if (panel) {
            infoBtn = panel.querySelector('.heif-comparison-info-btn');
        }
        
        // Check if click was outside the panel and not on the info button
        if (fullInfo && !fullInfo.contains(e.target) && (!infoBtn || !infoBtn.contains(e.target))) {
            fullInfo.classList.remove('visible');
            document.removeEventListener('click', self._handleClickOutsideComparisonInfo);
            self._activeFullInfoId = null;
        }
    };
    
    // Initialize the active full info ID tracker
    this._activeFullInfoId = null;

    /** Build and add info panel for a side of the comparison */
    /** Build and add info panel for a side of the comparison.
     *  For HEIF files, metadata is pulled from the existing HEIF_OBJECTS
     *  snapshot (which the decoder already built), avoiding a redundant
     *  fetch + parse cycle. Falls back to _extractHeifMetadata only if
     *  the snapshot is unavailable. */
    this._buildInfoPanelForSide = function(container, src, fileSize, side, comparisonId) {
        var el = container.querySelector('canvas, img');
        if (!el) return;
        
        var isHeif = HEIFUtils.isHeifFile(src);
        if (isHeif) {
            // Try to reuse the fileInfo snapshot the decoder already built
            var cachedFileInfo = null;
            if (typeof HEIF_OBJECTS !== 'undefined' && el.id && HEIF_OBJECTS[el.id]) {
                cachedFileInfo = HEIF_OBJECTS[el.id].getFileInfo();
            }

            if (cachedFileInfo) {
                var meta = self._metaFromFileInfo(cachedFileInfo, el);
                var panel = self._buildComparisonInfoPanel(comparisonId + '-' + side, true, meta, fileSize);
                container.appendChild(panel);
            } else {
                // Fallback: re-fetch (e.g. if HEIF_OBJECTS cleaned up)
                self._extractHeifMetadata(src, function(meta) {
                    if (!meta.width) meta.width = el.naturalWidth || el.width;
                    if (!meta.height) meta.height = el.naturalHeight || el.height;
                    var panel = self._buildComparisonInfoPanel(comparisonId + '-' + side, true, meta, fileSize);
                    container.appendChild(panel);
                });
            }
        } else {
            var extInfo = HEIFUtils.deriveFormatFromExtension(src);
            var meta = {
                width: el.naturalWidth || el.width,
                height: el.naturalHeight || el.height,
                format: extInfo.format,
                codec: extInfo.codec,
                bitDepth: 8,
                brands: []
            };
            var panel = self._buildComparisonInfoPanel(comparisonId + '-' + side, false, meta, fileSize);
            container.appendChild(panel);
        }
    };

    /** Wait for a ready canvas or natively-loaded img inside a
     *  comparison-item container. For HEIF sources the decoder replaces
     *  the original <img> with a <canvas>, so we must re-query the DOM
     *  rather than hold a stale element reference. */
    this._waitForReadyElement = function(item, callback) {
        var check = function() {
            var el = item.querySelector('canvas, img');
            if (!el) return false;
            if (el.tagName === 'CANVAS' && el.width > 0 && el.height > 0) { callback(el); return true; }
            if (el.tagName === 'IMG' && (el.naturalWidth > 0 || el.width > 0)) { callback(el); return true; }
            return false;
        };
        if (check()) return;
        var obs = new MutationObserver(function() { if (check()) obs.disconnect(); });
        obs.observe(item, { childList: true, subtree: true, attributes: true,
                            attributeFilter: ['width', 'height', 'src'] });
    };

    /** Add info panels to image comparison items */
    this._addInfoPanels = function(comparisonId, mapping) {
        var container = document.getElementById(comparisonId);
        if (!container) return;
        
        var items = container.querySelectorAll('.heif-image-comparison-item');
        if (items.length !== 2) return;
        
        var leftSrc = mapping.heicUrl;
        var rightSrc = mapping.jpgUrl;
        
        // Process left side
        HEIFUtils.getFileSize(leftSrc, function(leftSize) {
            self._waitForReadyElement(items[0], function() {
                self._buildInfoPanelForSide(items[0], leftSrc, leftSize, 'left', comparisonId);
            });
        });
        
        // Process right side
        HEIFUtils.getFileSize(rightSrc, function(rightSize) {
            self._waitForReadyElement(items[1], function() {
                self._buildInfoPanelForSide(items[1], rightSrc, rightSize, 'right', comparisonId);
            });
        });
    };

    /** Build sequence comparison info panel with info button */
    this._buildSequenceInfoPanel = function(containerId, format, fileSize, dimensions, frameCount, meta) {
        var container = document.createElement('div');
        container.className = 'sequences-comparison-info-panel';
        container.id = containerId + '-seq-info';
        container.style.position = 'relative'; // For popup positioning
        
        var title = document.createElement('div');
        title.className = 'info-title';
        title.textContent = format;
        container.appendChild(title);
        
        var details = document.createElement('div');
        details.className = 'info-details';
        
        if (dimensions) {
            var dimItem = document.createElement('span');
            dimItem.className = 'info-item';
            dimItem.textContent = dimensions;
            details.appendChild(dimItem);
            details.appendChild(HEIFUtils.createSeparator());
        }
        
        if (frameCount) {
            var frameItem = document.createElement('span');
            frameItem.className = 'info-item';
            frameItem.textContent = frameCount + ' frames';
            details.appendChild(frameItem);
            details.appendChild(HEIFUtils.createSeparator());
        }
        
        var sizeItem = document.createElement('span');
        sizeItem.className = 'info-item';
        sizeItem.textContent = HEIFUtils.formatFileSize(fileSize);
        details.appendChild(sizeItem);
        
        // Add info button if we have metadata (HEIF files)
        if (meta && meta.brands && meta.brands.length > 0) {
            var infoBtn = document.createElement('span');
            infoBtn.className = 'heif-comparison-info-btn';
            infoBtn.textContent = 'ⓘ';
            infoBtn.title = 'Show full details';
            infoBtn.onclick = function(e) {
                e.stopPropagation();
                self._toggleComparisonFullInfo(containerId + '-seq');
            };
            details.appendChild(infoBtn);
            
            // Create full info panel
            var fullInfo = self._buildComparisonFullInfoPanel(containerId + '-seq', meta);
            container.appendChild(fullInfo);
        }
        
        container.appendChild(details);
        return container;
    };

    /** Updated initializeSequencesComparison to extract HEIF metadata */
    this.initializeSequencesComparison = function () {
        var self = this;
        var img = document.getElementById("heif-image");
        img.setAttribute("src", "content/nokia/starfield_animation.heic");

        var checkReady = function(containerId, loadingId, heifSrc, heifSize, gifId, gifSrc, gifSize) {
            var container = document.getElementById(containerId);
            
            var tryFinish = function() {
                var canvas = container.querySelector('canvas');
                if (!canvas || canvas.width <= 0) return false;

                var loadingText = document.getElementById(loadingId);
                loadingText.style.visibility = "hidden";
                loadingText.style.opacity = 0;

                // Remove old p label if exists
                var oldLabel = container.querySelector('p');
                if (oldLabel) oldLabel.remove();
                
                // Extract HEIF metadata from the existing HEIF_OBJECTS
                // snapshot to avoid a redundant fetch + parse cycle.
                var cachedFileInfo = null;
                if (typeof HEIF_OBJECTS !== 'undefined' && canvas.id && HEIF_OBJECTS[canvas.id]) {
                    cachedFileInfo = HEIF_OBJECTS[canvas.id].getFileInfo();
                }

                var buildSeqPanel = function(heifMeta) {
                    var formatText = heifMeta.format;
                    if (heifMeta.codec && heifMeta.codec !== 'Unknown') {
                        formatText += ' | ' + heifMeta.codec;
                    }
                    var heifInfoPanel = self._buildSequenceInfoPanel(
                        containerId,
                        formatText,
                        heifSize,
                        canvas.width + '×' + canvas.height,
                        heifMeta.frameCount,
                        heifMeta
                    );
                    container.appendChild(heifInfoPanel);
                };

                if (cachedFileInfo) {
                    buildSeqPanel(self._metaFromFileInfo(cachedFileInfo, canvas));
                } else {
                    // Fallback: re-fetch if snapshot unavailable
                    self._fetchHeifSnapshot(heifSrc).then(function(result) {
                        buildSeqPanel(self._metaFromFileInfo(result.fileInfo, canvas));
                    }).catch(function(err) {
                        console.error('comparison: HEIF snapshot for ' + heifSrc + ' failed', err);
                    });
                }
                
                container.style.opacity = 1;

                // Load and show GIF
                var gif = document.getElementById(gifId);
                gif.setAttribute("src", gifSrc);
                var gifContainer = gif.parentElement;
                
                gif.onload = function() {
                    var oldGifLabel = gifContainer.querySelector('p');
                    if (oldGifLabel) oldGifLabel.remove();
                    
                    // NOTE: GIF metadata is derived from the <img> element (dimensions
                    // only). Full GIF structure parsing (frame count, loop count, timing)
                    // is not implemented — the comparison page only needs dimensions.
                    var gifMeta = {
                        width: gif.naturalWidth,
                        height: gif.naturalHeight,
                        format: 'GIF',
                        codec: null,
                        brands: []
                    };
                    
                    var gifInfoPanel = self._buildSequenceInfoPanel(
                        gifId,
                        'GIF',
                        gifSize,
                        gif.naturalWidth + '×' + gif.naturalHeight,
                        null,
                        gifMeta
                    );
                    gifContainer.appendChild(gifInfoPanel);
                };
                
                gifContainer.style.opacity = 1;
                return true;
            };

            // Try immediately (canvas may already be ready)
            if (tryFinish()) return;

            // Otherwise observe the container for the decoder inserting a canvas
            var obs = new MutationObserver(function() {
                if (tryFinish()) obs.disconnect();
            });
            obs.observe(container, { childList: true, subtree: true });
        };

        // Start checking for first animation with file sizes
        var heif1Src = "content/nokia/starfield_animation.heic";
        var gif1Src = "content/derived/starfield_256x144.gif";
        
        HEIFUtils.getFileSize(heif1Src, function(heifSize1) {
            HEIFUtils.getFileSize(gif1Src, function(gifSize1) {
                checkReady("heif-container", "sequences-comparison-loading", 
                           heif1Src, heifSize1, "gif-image", gif1Src, gifSize1);
            });
        });

        var img2 = document.getElementById("heif-image-2");
        var heif2Src = "content/nokia/candle_animation.heic";
        var gif2Src = "content/derived/candle_256x144.gif";
        img2.setAttribute("src", heif2Src);

        HEIFUtils.getFileSize(heif2Src, function(heifSize2) {
            HEIFUtils.getFileSize(gif2Src, function(gifSize2) {
                checkReady("heif-container-2", "sequences-comparison-loading-2", 
                           heif2Src, heifSize2, "gif-image-2", gif2Src, gifSize2);
            });
        });
    };

    /** Load a single comparison pair: set img sources and observe for
     *  canvas insertion to add info panels. */
    this._loadComparisonPair = function(key, mapping) {
        var self = this;
        var container = document.getElementById(key);
        if (!container) return;

        var items = container.querySelectorAll('.heif-image-comparison-item');
        if (items.length !== 2) return;

        // Left side (HEIF)
        if (mapping.heicUrl) {
            var leftWrapper = items[0].querySelector('.heif-image-comparison-wrapper');
            var leftImg = items[0].querySelector('.heif-comparison-image');

            var oldLeftLabel = items[0].querySelector('.heif-comparison-label');
            if (oldLeftLabel) oldLeftLabel.remove();

            leftImg.src = mapping.heicUrl;
            leftImg.dataset.originalSrc = mapping.heicUrl;

            if (mapping.nativeSize) {
                leftWrapper.classList.add('heif-native-size');
            }
            if (mapping.noZoom) {
                var lens = leftWrapper.querySelector('.heif-zoom-lens');
                if (lens) lens.style.display = 'none';
            }
        }

        // Right side (JPG/other)
        if (mapping.jpgUrl) {
            var rightWrapper = items[1].querySelector('.heif-image-comparison-wrapper');
            var rightImg = items[1].querySelector('.heif-comparison-image');

            var oldRightLabel = items[1].querySelector('.heif-comparison-label');
            if (oldRightLabel) oldRightLabel.remove();

            rightImg.src = mapping.jpgUrl;
            rightImg.dataset.originalSrc = mapping.jpgUrl;

            if (mapping.nativeSize) {
                rightWrapper.classList.add('heif-native-size');
            }
            if (mapping.noZoom) {
                var lens = rightWrapper.querySelector('.heif-zoom-lens');
                if (lens) lens.style.display = 'none';
            }
        }

        // Trigger the HEIF decoder pipeline for the newly-sourced images.
        if (typeof processImageElements === 'function') processImageElements();

        // Add info panels once the HEIF decoder has produced a canvas.
        var panelContainer = container;
        var existing = panelContainer.querySelector('canvas');
        if (existing && existing.width > 0) {
            self._addInfoPanels(key, mapping);
            return;
        }
        var panelObs = new MutationObserver(function(mutations) {
            var c = panelContainer.querySelector('canvas');
            if (c && c.width > 0) {
                panelObs.disconnect();
                document.removeEventListener('heif-decoded', onPanelDecoded);
                self._addInfoPanels(key, mapping);
            }
        });
        panelObs.observe(panelContainer, { childList: true, subtree: true,
                                           attributes: true,
                                           attributeFilter: ['width', 'height'] });
        // Also listen for heif-decoded as a fallback — the canvas may
        // already be in the DOM when its dimensions are set.
        function onPanelDecoded(e) {
            var section = e.target.closest('.heif-image-comparison-content');
            if (section && section.id === key) {
                panelObs.disconnect();
                document.removeEventListener('heif-decoded', onPanelDecoded);
                self._addInfoPanels(key, mapping);
            }
        }
        document.addEventListener('heif-decoded', onPanelDecoded);
    };

    this.initializeImageComparison = function () {
        var self = this;
        var loadedKeys = {};

        // Use IntersectionObserver to defer loading of off-screen sections.
        // The first visible section loads immediately; others load when the
        // user scrolls near them (rootMargin gives 200px of look-ahead).
        // VVC sections (comparison-0, comparison-6) are loaded eagerly so
        // the loading overlay can wait on their decode without deadlocking.
        var eagerKeys = { 'comparison-0': true, 'comparison-6': true };

        if ('IntersectionObserver' in window) {
            var observer = new IntersectionObserver(function(entries) {
                entries.forEach(function(entry) {
                    if (!entry.isIntersecting) return;
                    var key = entry.target.id;
                    if (loadedKeys[key]) return;
                    loadedKeys[key] = true;
                    observer.unobserve(entry.target);
                    var mapping = self._imageComparisonMapping[key];
                    if (mapping) self._loadComparisonPair(key, mapping);
                });
            }, { rootMargin: '200px' });

            for (var key in this._imageComparisonMapping) {
                if (this._imageComparisonMapping.hasOwnProperty(key)) {
                    if (eagerKeys[key]) {
                        loadedKeys[key] = true;
                        this._loadComparisonPair(key, this._imageComparisonMapping[key]);
                    } else {
                        var container = document.getElementById(key);
                        if (container) observer.observe(container);
                    }
                }
            }
        } else {
            // Fallback: load all immediately
            for (var key in this._imageComparisonMapping) {
                if (this._imageComparisonMapping.hasOwnProperty(key)) {
                    this._loadComparisonPair(key, this._imageComparisonMapping[key]);
                }
            }
        }
    };

    this.initialize = function () {

        this.initializeSequencesComparison();

        this.initializeImageComparison();

        processImageElements();
        processVideoElements();
    };
}
