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
 * Synchronized image magnifier for comparing two images
 * Works with both <img> and <canvas> elements
 */
function ComparisonZoom(options) {
    var config = {
        containerSelector: '.heif-image-comparison-content',
        previewSize: 200,
        zoomLevel: 3
    };
    
    if (options) {
        for (var key in options) {
            if (options.hasOwnProperty(key)) {
                config[key] = options[key];
            }
        }
    }
    
    // Pre-calculate constants
    var previewSize = config.previewSize;
    var zoomLevel = config.zoomLevel;
    
    // Active state
    var activePair = null;
    var activeSource = null;
    var rafId = null;
    var lastX = 0;
    var lastY = 0;
    
    // All pairs
    var pairs = [];
    
    this.init = function() {
        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', setup);
        } else {
            setup();
        }
        return this;
    };
    
    function setup() {
        var containers = document.querySelectorAll(config.containerSelector);
        
        for (var i = 0; i < containers.length; i++) {
            var container = containers[i];
            var items = container.querySelectorAll('.heif-image-comparison-item');
            
            if (items.length >= 2) {
                var leftSide = createSide(items[0]);
                var rightSide = createSide(items[1]);
                
                if (leftSide && rightSide) {
                    var pair = {
                        left: leftSide,
                        right: rightSide
                    };
                    
                    // Direct reference for O(1) lookup
                    leftSide.wrapper._zoomPair = pair;
                    leftSide.wrapper._zoomSide = leftSide;
                    rightSide.wrapper._zoomPair = pair;
                    rightSide.wrapper._zoomSide = rightSide;
                    
                    pairs.push(pair);
                    
                    // Attach events to wrapper (contains source)
                    leftSide.wrapper.addEventListener('mouseenter', onEnter);
                    leftSide.wrapper.addEventListener('mouseleave', onLeave);
                    leftSide.wrapper.addEventListener('mousemove', onMove);
                    rightSide.wrapper.addEventListener('mouseenter', onEnter);
                    rightSide.wrapper.addEventListener('mouseleave', onLeave);
                    rightSide.wrapper.addEventListener('mousemove', onMove);
                }
            }
        }
    }
    
    function createSide(item) {
        var wrapper = item.querySelector('.heif-image-comparison-wrapper');
        var previewContainer = item.querySelector('.heif-zoom-preview');
        
        if (!wrapper) {
            console.warn('ComparisonZoom: no wrapper found');
            return null;
        }
        
        // Find source - could be img or canvas (HEIF decoded)
        var source = wrapper.querySelector('canvas') || wrapper.querySelector('img');
        
        if (!source) {
            console.warn('ComparisonZoom: no source image/canvas found');
            return null;
        }
        
        // Create preview container if not exists
        if (!previewContainer) {
            previewContainer = document.createElement('div');
            previewContainer.className = 'heif-zoom-preview';
            previewContainer.style.cssText = 
                'width:' + previewSize + 'px;' +
                'height:' + previewSize + 'px;' +
                'border:2px solid rgba(255,255,255,0.8);' +
                'box-shadow:0 2px 10px rgba(0,0,0,0.3);' +
                'border-radius:4px;' +
                'position:absolute;' +
                'top:10px;right:10px;' +
                'display:none;' +
                'background:#000;' +
                'z-index:100;';
            wrapper.appendChild(previewContainer);
        }
        
        // Create preview canvas
        var previewCanvas = document.createElement('canvas');
        previewCanvas.width = previewSize;
        previewCanvas.height = previewSize;
        previewCanvas.style.cssText = 'display:block;width:100%;height:100%;';
        
        previewContainer.replaceChildren();
        previewContainer.appendChild(previewCanvas);
        
        var previewCtx = previewCanvas.getContext('2d', { alpha: false });
        
        var side = {
            wrapper: wrapper,
            source: source,
            previewContainer: previewContainer,
            previewCanvas: previewCanvas,
            previewCtx: previewCtx,
            // Cached dimensions
            sourceWidth: 0,
            sourceHeight: 0,
            displayWidth: 0,
            displayHeight: 0,
            scaleX: 1,
            scaleY: 1,
            rectLeft: 0,
            rectTop: 0,
            ready: false
        };
        
        // Setup dimensions when source is ready
        function setupDimensions() {
            var isImg = side.source.tagName === 'IMG';
            
            if (isImg) {
                side.sourceWidth = side.source.naturalWidth || side.source.width;
                side.sourceHeight = side.source.naturalHeight || side.source.height;
            } else {
                side.sourceWidth = side.source.width;
                side.sourceHeight = side.source.height;
            }
            
            if (!side.sourceWidth || !side.sourceHeight) {
                console.warn('ComparisonZoom: source not ready yet');
                return;
            }
            
            var rect = side.source.getBoundingClientRect();
            side.displayWidth = rect.width;
            side.displayHeight = rect.height;
            side.rectLeft = rect.left;
            side.rectTop = rect.top;
            
            side.scaleX = side.sourceWidth / side.displayWidth;
            side.scaleY = side.sourceHeight / side.displayHeight;
            
            side.ready = true;
        }
        
        // Check if source might change (HEIF decoder replaces img with canvas)
        function checkForSourceChange() {
            var newSource = wrapper.querySelector('canvas') || wrapper.querySelector('img');
            if (newSource && newSource !== side.source) {
                side.source = newSource;
                side.ready = false;
                waitForSource();
            }
        }
        
        function waitForSource() {
            var isImg = side.source.tagName === 'IMG';
            
            if (isImg) {
                if (side.source.complete && side.source.naturalWidth) {
                    setupDimensions();
                } else {
                    side.source.addEventListener('load', setupDimensions, { once: true });
                }
            } else {
                // Canvas - check if it has content
                if (side.source.width > 0 && side.source.height > 0) {
                    setupDimensions();
                } else {
                    // Wait for canvas to receive dimensions (decoder sets
                    // width/height when it paints). ResizeObserver fires when
                    // the element's content-box changes from 0×0 to real size.
                    var ro = new ResizeObserver(function(entries) {
                        if (side.source.width > 0 && side.source.height > 0) {
                            ro.disconnect();
                            setupDimensions();
                        }
                    });
                    ro.observe(side.source);
                }
            }
        }
        
        // Initial setup
        waitForSource();
        
        // Watch for HEIF decoder replacing img with canvas
        var observer = new MutationObserver(function(mutations) {
            for (var i = 0; i < mutations.length; i++) {
                if (mutations[i].addedNodes.length > 0) {
                    checkForSourceChange();
                }
            }
        });
        observer.observe(wrapper, { childList: true, subtree: true });
        
        return side;
    }
    
    function onEnter(e) {
        var wrapper = e.currentTarget;
        var pair = wrapper._zoomPair;
        var side = wrapper._zoomSide;
        
        if (!pair) return;
        
        activePair = pair;
        activeSource = side;
        
        // Refresh rect positions (may have scrolled)
        refreshRects(pair);
        
        // Show both previews
        pair.left.previewContainer.style.display = 'block';
        pair.right.previewContainer.style.display = 'block';
    }
    
    function onLeave() {
        if (!activePair) return;
        
        // Hide previews
        activePair.left.previewContainer.style.display = 'none';
        activePair.right.previewContainer.style.display = 'none';
        
        // Clear canvases
        activePair.left.previewCtx.clearRect(0, 0, previewSize, previewSize);
        activePair.right.previewCtx.clearRect(0, 0, previewSize, previewSize);
        
        activePair = null;
        activeSource = null;
        
        if (rafId) {
            cancelAnimationFrame(rafId);
            rafId = null;
        }
    }
    
    function onMove(e) {
        lastX = e.clientX;
        lastY = e.clientY;
        
        if (!rafId) {
            rafId = requestAnimationFrame(updatePreviews);
        }
    }
    
    function refreshRects(pair) {
        var leftRect = pair.left.source.getBoundingClientRect();
        pair.left.rectLeft = leftRect.left;
        pair.left.rectTop = leftRect.top;
        pair.left.displayWidth = leftRect.width;
        pair.left.displayHeight = leftRect.height;
        
        var rightRect = pair.right.source.getBoundingClientRect();
        pair.right.rectLeft = rightRect.left;
        pair.right.rectTop = rightRect.top;
        pair.right.displayWidth = rightRect.width;
        pair.right.displayHeight = rightRect.height;
    }
    
    function updatePreviews() {
        rafId = null;
        
        if (!activePair || !activeSource) return;
        
        // Re-measure on every tick. The cached rectLeft/rectTop captured
        // by refreshRects() on hover-enter goes stale the moment the user
        // scrolls or the layout shifts (e.g. an info panel opens above
        // the comparison), and the magnifier then samples the wrong
        // pixel. getBoundingClientRect is layout-cached by the browser
        // and rAF-gated here, so the cost is negligible (~4 reads/frame
        // while hovering), and we restore drag-correct tracking.
        refreshRects(activePair);
        
        // Calculate relative position (0-1) on the active source
        var relX = (lastX - activeSource.rectLeft) / activeSource.displayWidth;
        var relY = (lastY - activeSource.rectTop) / activeSource.displayHeight;
        
        // Clamp
        if (relX < 0) relX = 0;
        else if (relX > 1) relX = 1;
        if (relY < 0) relY = 0;
        else if (relY > 1) relY = 1;
        
        // Update both previews at the same relative position
        drawPreview(activePair.left, relX, relY);
        drawPreview(activePair.right, relX, relY);
    }
    
    function drawPreview(side, relX, relY) {
        if (!side.ready) return;
        
        var ctx = side.previewCtx;
        var source = side.source;
        var sourceWidth = side.sourceWidth;
        var sourceHeight = side.sourceHeight;
        
        // Calculate the region to extract from source
        // extractSize is how much of the source image we grab (smaller = more zoom)
        var extractWidth = previewSize / zoomLevel;
        var extractHeight = previewSize / zoomLevel;
        
        // Position in source coordinates
        var srcX = relX * sourceWidth;
        var srcY = relY * sourceHeight;
        
        // Center extraction around the position
        var extractX = srcX - extractWidth / 2;
        var extractY = srcY - extractHeight / 2;
        
        // Clamp to source bounds
        if (extractX < 0) extractX = 0;
        if (extractY < 0) extractY = 0;
        if (extractX + extractWidth > sourceWidth) {
            extractX = sourceWidth - extractWidth;
        }
        if (extractY + extractHeight > sourceHeight) {
            extractY = sourceHeight - extractHeight;
        }
        
        // Ensure non-negative
        if (extractX < 0) extractX = 0;
        if (extractY < 0) extractY = 0;
        
        // Draw zoomed region
        // This single drawImage call extracts a region and scales it up
        ctx.drawImage(
            source,
            extractX, extractY,           // Source position
            extractWidth, extractHeight,  // Source size (small = zoomed)
            0, 0,                          // Destination position
            previewSize, previewSize       // Destination size (fills preview)
        );
    }
    
    this.setZoomLevel = function(level) {
        config.zoomLevel = level;
        zoomLevel = level;
    };
    
    this.setPreviewSize = function(size) {
        config.previewSize = size;
        previewSize = size;
    };
    
    this.refresh = function() {
        pairs = [];
        activePair = null;
        activeSource = null;
        setup();
    };
    
    this.destroy = function() {
        for (var i = 0; i < pairs.length; i++) {
            var pair = pairs[i];
            pair.left.wrapper.removeEventListener('mouseenter', onEnter);
            pair.left.wrapper.removeEventListener('mouseleave', onLeave);
            pair.left.wrapper.removeEventListener('mousemove', onMove);
            pair.right.wrapper.removeEventListener('mouseenter', onEnter);
            pair.right.wrapper.removeEventListener('mouseleave', onLeave);
            pair.right.wrapper.removeEventListener('mousemove', onMove);
        }
        pairs = [];
        activePair = null;
    };
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = ComparisonZoom;
}