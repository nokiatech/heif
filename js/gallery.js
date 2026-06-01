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
 * Gallery - Simplified HEIF gallery display
 * 
 * Structure:
 * - _galleryItems: Static list of file paths (NO hardcoded types)
 * - initialize(): Creates DOM, loads thumbnails
 * - _loadAndCacheItem(): Fetches fileInfo, caches on item, derives type
 * - _openModal(): Uses cached item data (no re-fetch)
 * - UI rendering functions
 * 
 * All HEIF parsing delegated to ImageProvider helper methods.
 */
function Gallery() {
    var self = this;

    // =========================================================================
    // CONFIGURATION
    // =========================================================================
    
    this.SHUFFLE_GALLERY = true;
    this.SHUFFLE_SEED = 69;

    // =========================================================================
    // GALLERY ITEMS - Two separate sets: Normal and Conformance
    // =========================================================================
    
    // Current gallery set: 'normal', 'conformance', 'vvc', or 'experimental'
    // URL ?set= parameter takes priority, then localStorage, then default
    var _urlSet = (new URLSearchParams(window.location.search)).get('set');
    this._currentSet = _urlSet || localStorage.getItem('gallerySet') || 'normal';
    
    // Normal gallery items (examples, demos, etc.)
    this._normalItems = [
        // Legacy examples content
        { id: "gallery-1", title: "Autumn Scene", heifPath: "content/nokia/autumn_1440x960.heic" },
        { id: "gallery-2", title: "Cheers", heifPath: "content/nokia/cheers_1440x960.heic" },
        { id: "gallery-3", title: "Crowd", heifPath: "content/nokia/crowd_1440x960.heic" },
        { id: "gallery-4", title: "Old Bridge", heifPath: "content/nokia/old_bridge_1440x960.heic" },
        { id: "gallery-5", title: "Ski Jump", heifPath: "content/nokia/ski_jump_1440x960.heic" },
        { id: "gallery-6", title: "Spring", heifPath: "content/nokia/spring_1440x960.heic" },
        { id: "gallery-7", title: "Summer", heifPath: "content/nokia/summer_1440x960.heic" },
        { id: "gallery-8", title: "Surfer", heifPath: "content/nokia/surfer_1440x960.heic" },
        { id: "gallery-9", title: "Winter", heifPath: "content/nokia/winter_1440x960.heic" },
        { id: "gallery-10", title: "Bird Burst", heifPath: "content/nokia/bird_burst.heic" },
        { id: "gallery-11", title: "Rally Burst", heifPath: "content/nokia/rally_burst.heic" },
        { id: "gallery-12", title: "Starfield", heifPath: "content/nokia/starfield_animation.heic" },
        { id: "gallery-13", title: "Sea Animation", heifPath: "content/nokia/sea1_animation.heic" },
        { id: "gallery-14", title: "Derived Grid", heifPath: "content/nokia/grid_960x640.heic" },
        { id: "gallery-15", title: "Derived Overlay", heifPath: "content/nokia/overlay_1000x680.heic" },
        { id: "gallery-16", title: "Alpha Mask", heifPath: "content/nokia/alpha_1440x960.heic" },
        { id: "gallery-17", title: "Stereo Pair", heifPath: "content/nokia/stereo_1200x800.heic" },
        { id: "gallery-18", title: "Bothie", heifPath: "content/nokia/bothie_1440x960.heic" },
        { id: "gallery-19", title: "Lights", heifPath: "content/nokia/lights_1440x960.heic" },

        //{ id: "gallery-22", title: "Sun", heifPath: "content/personal/sun_hdr_full_gainmap.heic" },
        { id: "gallery-23", title: "XD", heifPath: "content/conformance/C83_dup.hmg", pixelHonest: true },

        // iPhone test
        { id: "gallery-26", title: "Mug", heifPath: "content/personal/mug.heic" },

        { id: "gallery-29", title: "Shades", heifPath: "content/personal/shades.heic" },
        { id: "gallery-30", title: "Coffee room", heifPath: "content/personal/coffee-room.heic" },


        // Pixabay + Unsplash sources - publication-ready, see CONTENT-LICENSES.md
        { id: "gallery-33", title: "Finnish Road", heifPath: "content/pixabay/ltapsah-road-6594010.heif" },
        { id: "gallery-34", title: "Aurajoki", heifPath: "content/pixabay/661512-finland-1493058.heif" },
        { id: "gallery-35", title: "Kirkonkylä", heifPath: "content/unsplash/tapio-haaja-DTgvAa5pKO4-unsplash.heif" },
        { id: "gallery-36", title: "Helsinki", heifPath: "content/unsplash/tapio-haaja-z9LKMs0P3bo-unsplash.heif" },
        { id: "gallery-37", title: "Naistenlahti", heifPath: "content/unsplash/sini-tiainen-oXEfEJ-hE7Q-unsplash (1).heif" },
        { id: "gallery-38", title: "Turun Linna", heifPath: "content/pixabay/nord-bei-nordost-turku-2810087.heif"},
        { id: "gallery-39", title: "Meadow", heifPath: "content/nokia/meadow_2k_vvc.heif" },
        { id: "gallery-45", title: "Random Collection", heifPath: "content/nokia/random_collection_1440x960.heic" },
        { id: "gallery-46", title: "Season Collection", heifPath: "content/nokia/season_collection_1440x960.heic" },

        //more personal
        { id: "gallery-40", title: "Mr. Victory", heifPath: "content/personal/mr-victory.heif" },
        { id: "gallery-41", title: "Lähde", heifPath: "content/personal/lahde.heif" },
        { id: "gallery-42", title: "Brutal Jurmala", heifPath: "content/personal/brutal-jurmala.heif" },
        { id: "gallery-43", title: "Vintage supercar", heifPath: "content/personal/vintage-supercar.heif" },
        { id: "gallery-44", title: "Felines at the old factory", heifPath: "content/personal/felines-old-factory.heif" },
        ];
    
    // Conformance test files
    this._conformanceItems = [
        { id: "conf-1", title: "C001", heifPath: "content/conformance/C001.heic" },
        { id: "conf-2", title: "C002", heifPath: "content/conformance/C002.heic" },
        { id: "conf-3", title: "C003", heifPath: "content/conformance/C003.heic" },
        { id: "conf-4", title: "C004", heifPath: "content/conformance/C004.heic" },
        { id: "conf-5", title: "C005", heifPath: "content/conformance/C005.heic" },
        { id: "conf-6", title: "C006", heifPath: "content/conformance/C006.heic" },
        { id: "conf-7", title: "C007", heifPath: "content/conformance/C007.heic" },
        { id: "conf-8", title: "C008", heifPath: "content/conformance/C008.heic" },
        { id: "conf-9", title: "C009", heifPath: "content/conformance/C009.heic" },
        { id: "conf-10", title: "C010", heifPath: "content/conformance/C010.heic" },
        { id: "conf-11", title: "C011", heifPath: "content/conformance/C011.heic" },
        { id: "conf-12", title: "C012", heifPath: "content/conformance/C012.heic" },
        { id: "conf-13", title: "C013", heifPath: "content/conformance/C013.heic" },
        { id: "conf-14", title: "C014", heifPath: "content/conformance/C014.heic" },
        { id: "conf-15", title: "C015", heifPath: "content/conformance/C015.heic" },
        { id: "conf-16", title: "C016", heifPath: "content/conformance/C016.heic" },
        { id: "conf-17", title: "C017", heifPath: "content/conformance/C017.heic" },
        { id: "conf-18", title: "C018", heifPath: "content/conformance/C018.heic" },
        { id: "conf-19", title: "C019", heifPath: "content/conformance/C019.heic" },
        { id: "conf-20", title: "C020", heifPath: "content/conformance/C020.heic" },
        { id: "conf-21", title: "C021", heifPath: "content/conformance/C021.heic" },
        { id: "conf-22", title: "C022", heifPath: "content/conformance/C022.heic" },
        { id: "conf-23", title: "C023", heifPath: "content/conformance/C023.heic" },
        { id: "conf-24", title: "C024", heifPath: "content/conformance/C024.heic" },
        { id: "conf-25", title: "C025", heifPath: "content/conformance/C025.heic" },
        { id: "conf-26", title: "C026", heifPath: "content/conformance/C026.heic" },
        { id: "conf-27", title: "C027", heifPath: "content/conformance/C027.heic" },
        { id: "conf-28", title: "C028", heifPath: "content/conformance/C028.heic" },
        { id: "conf-29", title: "C029", heifPath: "content/conformance/C029.heic" },
        { id: "conf-30", title: "C030", heifPath: "content/conformance/C030.heic" },
        { id: "conf-31", title: "C031", heifPath: "content/conformance/C031.heic" },
        { id: "conf-32", title: "C032", heifPath: "content/conformance/C032.heic" },
        { id: "conf-33", title: "C034", heifPath: "content/conformance/C034.heic" },
        { id: "conf-34", title: "C036", heifPath: "content/conformance/C036.heic" },
        { id: "conf-35", title: "C037", heifPath: "content/conformance/C037.heic" },
        { id: "conf-36", title: "C038", heifPath: "content/conformance/C038.heic" },
        { id: "conf-37", title: "C039", heifPath: "content/conformance/C039.heic" },
        { id: "conf-38", title: "C040", heifPath: "content/conformance/C040.heic" },
        { id: "conf-39", title: "C041", heifPath: "content/conformance/C041.heic" },
        { id: "conf-40", title: "C042", heifPath: "content/conformance/C042.heic" },
        { id: "conf-41", title: "C043", heifPath: "content/conformance/C043.heic" },
        { id: "conf-42", title: "C044", heifPath: "content/conformance/C044.heic" },
        { id: "conf-43", title: "C045", heifPath: "content/conformance/C045.heic" },
        { id: "conf-44", title: "C046", heifPath: "content/conformance/C046.heic" },
        { id: "conf-45", title: "C047", heifPath: "content/conformance/C047.heic" },
        { id: "conf-46", title: "C048", heifPath: "content/conformance/C048.heic" },
        { id: "conf-47", title: "C049", heifPath: "content/conformance/C049.heic" },
        { id: "conf-48", title: "C050", heifPath: "content/conformance/C050.heic" },
        { id: "conf-49", title: "C051", heifPath: "content/conformance/C051.heic" },
        { id: "conf-50", title: "C052", heifPath: "content/conformance/C052.heic" },
        { id: "conf-51", title: "C053", heifPath: "content/conformance/C053.heic" },
        { id: "conf-52", title: "multilayer001", heifPath: "content/conformance/multilayer001.heic" },
        { id: "conf-53", title: "multilayer002", heifPath: "content/conformance/multilayer002.heic" },
        { id: "conf-54", title: "multilayer003", heifPath: "content/conformance/multilayer003.heic" },
        { id: "conf-55", title: "multilayer004", heifPath: "content/conformance/multilayer004.heic" },
        { id: "conf-56", title: "multilayer005", heifPath: "content/conformance/multilayer005.heic" },
        { id: "conf-57", title: "MIAF001", heifPath: "content/conformance/MIAF001.heic" },
        { id: "conf-58", title: "MIAF002", heifPath: "content/conformance/MIAF002.heic" },
        { id: "conf-59", title: "MIAF003", heifPath: "content/conformance/MIAF003.heic" },
        { id: "conf-60", title: "MIAF004", heifPath: "content/conformance/MIAF004.heic" },
        { id: "conf-61", title: "MIAF005", heifPath: "content/conformance/MIAF005.heic" },
        { id: "conf-62", title: "MIAF006", heifPath: "content/conformance/MIAF006.heic" },
        { id: "conf-63", title: "MIAF007", heifPath: "content/conformance/MIAF007.heic" },
        { id: "conf-64", title: "C71", heifPath: "content/conformance/C71.heic" },
        { id: "conf-65", title: "C75", heifPath: "content/conformance/C75.hmg" },
        { id: "conf-66", title: "C83", heifPath: "content/conformance/C83.hmg" },
    ];

    // Experimental/test files - accessible via /gallery.html?set=experimental
    this._experimentalItems = [
        { id: "exp-6a", title: "Wind Turbines HDR", heifPath: "content/cc0/3A_HEIC_HDR.heic" },
        { id: "exp-7", title: "AVIF", heifPath: "content/cc0/IMAGE_019.avif" },
        { id: "exp-8", title: "Mug2", heifPath: "content/personal/mug2.heic" },
        { id: "exp-9", title: "Kahvihuone2", heifPath: "content/personal/kahvihuone2.heic" },
        { id: "exp-5", title: "Dual Mini VVC", heifPath: "content/conformance/1dualMiniVVC.hmg" },
        { id: "exp-6b", title: "Wind Turbines SDR", heifPath: "content/cc0/3B_HEIC_SDR.heic"},
        { id: "exp-10", title: "Big tree", heifPath: "content/imagecompression/big_tree.heic" },
        { id: "exp-11", title: "Sun 10-bit HDR + Gainmap", heifPath: "content/personal/sun_hdr_full_gainmap.heic" },
    ];

    // VVC (H.266) encoded content - all files using vvc1/vvi1 codec
    this._vvcItems = [
        { id: "vvc-8", title: "Bricks", heifPath: "content/unsplash/bricks.heif" },
        { id: "vvc-9", title: "Meadow", heifPath: "content/nokia/meadow_2k_vvc.heif" },
        // web26 — Pixabay + Unsplash sources, publication-ready
        { id: "vvc-15", title: "Finnish Road", heifPath: "content/pixabay/ltapsah-road-6594010.heif" },
        { id: "vvc-20", title: "Turun Linna", heifPath: "content/pixabay/nord-bei-nordost-turku-2810087.heif" },
        { id: "vvc-16", title: "Aurajoki", heifPath: "content/pixabay/661512-finland-1493058.heif" },
        { id: "vvc-17", title: "Kirkonkylä", heifPath: "content/unsplash/tapio-haaja-DTgvAa5pKO4-unsplash.heif" },
        { id: "vvc-18", title: "Helsinki", heifPath: "content/unsplash/tapio-haaja-z9LKMs0P3bo-unsplash.heif" },
        { id: "vvc-19", title: "Naistenlahti", heifPath: "content/unsplash/sini-tiainen-oXEfEJ-hE7Q-unsplash (1).heif" },
        { id: "vvc-23", title: "Mr. Victory", heifPath: "content/personal/mr-victory.heif" },
        { id: "vvc-24", title: "Lähde", heifPath: "content/personal/lahde.heif" },
        { id: "vvc-25", title: "Brutal Jurmala", heifPath: "content/personal/brutal-jurmala.heif" },
        { id: "vvc-26", title: "Vintage supercar", heifPath: "content/personal/vintage-supercar.heif" },
        { id: "vvc-27", title: "Felines at the old factory", heifPath: "content/personal/felines-old-factory.heif" },
        { id: "vvc-28", title: "XD", heifPath: "content/conformance/C83.hmg", pixelHonest: true },
    ];


    // =========================================================================
    // STATE
    // =========================================================================
    
    this._loadedCount = 0;
    this._animatedThumbnails = {};
    this._activeProvider = null;
    this._activeReader = null;
    this._sequenceCleanup = null;
    this._memoryCleanupTimer = null;

    // Per-modal-session cleanup callbacks. Modal display fns push teardown
    // hooks here (audio pause+revokeObjectURL, custom timers, etc.) instead
    // of monkey-patching `_closeModal` per-call. The previous pattern
    //   var orig = self._closeModal; self._closeModal = function(){...orig.call(self);};
    // would stack across arrow-key navigation between modal items, leak
    // resources from the previous item, and (rarely) deadlock if two opens
    // raced. _runModalCleanups runs and clears them; both _openModal
    // (inter-item teardown) and _closeModal (full close) invoke it.
    this._modalCleanups = [];

    // Monotonic session id for the active modal. Incremented every time
    // _teardownActiveModalSession() runs, so any async work spawned by
    // the previous session can detect it has been superseded and bail
    // before mutating the new session's DOM (e.g. appending an animated
    // canvas, registering a setInterval, or pushing a cleanup callback).
    // The previous design relied on _modalCleanups + provider.dispose()
    // to halt in-flight work, but provider.requestImageData() callbacks
    // fire regardless of dispose() so they could land on the wrong
    // session. Display fns snapshot _modalSessionId at entry and the
    // _addModalCleanup helper drops late pushes.
    this._modalSessionId = 0;
    this._addModalCleanup = function(sessionId, fn) {
        // Allow the legacy single-arg form (.push-style) for callers that
        // haven't snapshotted - they're invoked synchronously inside the
        // current session anyway. New async-safe callers pass sessionId.
        if (typeof sessionId === 'function') { fn = sessionId; sessionId = self._modalSessionId; }
        if (sessionId !== self._modalSessionId) {
            // Late push from a torn-down session. Run the cleanup
            // immediately so the resource (audio URL, timer, etc.) is
            // still released, then drop it - no point keeping it on
            // _modalCleanups since the new session won't own it.
            try { fn(); } catch (e) { /* ignore */ }
            return;
        }
        self._modalCleanups.push(fn);
    };

    // Cached environment flags - computed once, used everywhere.
    this._isMobileDevice = /iPhone|iPad|iPod|Android/i.test(navigator.userAgent);
    this._reducedMotion = (typeof window.matchMedia === 'function')
        ? window.matchMedia('(prefers-reduced-motion: reduce)').matches
        : false;

    // ---------------------------------------------------------------------
    // Layout mode (Adaptive grid / Native pixels).
    //
    // The page subtitle promises "Native pixels" - the original code didn't
    // honour it (every canvas was scaled by CSS to fill its column). The
    // user can now opt in via the toolbar toggle. Persisted to localStorage
    // alongside the gallery set selection.
    // ---------------------------------------------------------------------
    this._layoutMode = (function() {
        try { return localStorage.getItem('galleryLayoutMode') || 'adaptive'; }
        catch (e) { return 'adaptive'; }
    })();
    this._isNativeMode = function() { return self._layoutMode === 'native'; };

    this._applyLayoutModeClass = function() {
        var grid = document.getElementById('gallery-grid');
        if (!grid) return;
        grid.classList.toggle('gallery-mode-native', self._layoutMode === 'native');
        grid.classList.toggle('gallery-mode-adaptive', self._layoutMode === 'adaptive');
    };

    this._setLayoutMode = function(mode) {
        if (mode !== 'native' && mode !== 'adaptive') return;
        if (mode === self._layoutMode) return;
        self._layoutMode = mode;
        try { localStorage.setItem('galleryLayoutMode', mode); } catch (e) {}

        // Toolbar button state
        var btns = document.querySelectorAll('.gallery-view-btn');
        for (var i = 0; i < btns.length; i++) {
            var on = btns[i].getAttribute('data-layout') === mode;
            btns[i].classList.toggle('active', on);
            btns[i].setAttribute('aria-pressed', on ? 'true' : 'false');
        }

        // Toggling adaptive <-> native is a pure CSS change. The canvases
        // already hold their pixels at thumbnail resolution and don't need
        // re-decoding - the previous implementation called _loadCurrentSet()
        // here, which re-ran every wasm decode for the entire set (~30s on
        // a 60-item gallery). All we actually need to do is:
        //   1. flip the mode class on the grid
        //   2. recompute --gi-span when going back to adaptive (column width
        //      drives visual height, which drives row-span count)
        // The .is-switching opacity dip just hides the swap.
        var grid = document.getElementById('gallery-grid');
        var applySwap = function() {
            self._applyLayoutModeClass();
            if (mode === 'adaptive') self._recomputeAllSpans();
        };
        if (grid && !self._reducedMotion) {
            grid.classList.add('is-switching');
            // Let the opacity dip happen, then swap, then fade back.
            setTimeout(function() {
                applySwap();
                requestAnimationFrame(function() {
                    grid.classList.remove('is-switching');
                });
            }, 160);
        } else {
            applySwap();
        }
    };

    // ---------------------------------------------------------------------
    // Combined status bar.
    //
    // Replaces the spinner-block-above-the-grid with a thin top-of-grid
    // progress strip ("12 of 60 thumbnails decoded"). The bar reserves a
    // fixed slot in normal flow at all times and fades to opacity 0 when
    // loading completes - this avoids the layout shift the previous
    // collapsing version caused on the grid below.
    // ---------------------------------------------------------------------
    this._statusTotal = 0;
    this._statusDone = 0;

    this._statusReset = function() {
        var bar = document.getElementById('gallery-status-bar');
        var fill = document.getElementById('gallery-status-fill');
        var text = document.getElementById('gallery-status-text');
        self._statusTotal = 0;
        self._statusDone = 0;
        if (bar) bar.classList.remove('complete');
        if (fill) fill.style.width = '0%';
        if (text) text.textContent = 'Preparing gallery\u2026';
    };

    this._statusSetTotal = function(total) {
        self._statusTotal = total || 0;
        self._statusDone = 0;
        var fill = document.getElementById('gallery-status-fill');
        var text = document.getElementById('gallery-status-text');
        if (fill) fill.style.width = '0%';
        if (text) {
            text.textContent = self._statusTotal === 0
                ? 'No items in this set.'
                : 'Decoding 0 of ' + self._statusTotal + ' thumbnails\u2026';
        }
    };

    this._statusBumpDone = function() {
        self._statusDone++;
        var fill = document.getElementById('gallery-status-fill');
        var text = document.getElementById('gallery-status-text');
        var bar = document.getElementById('gallery-status-bar');
        if (self._statusTotal > 0) {
            var pct = Math.min(100, (self._statusDone / self._statusTotal) * 100);
            if (fill) fill.style.width = pct.toFixed(1) + '%';
            if (text) {
                text.textContent = self._statusDone >= self._statusTotal
                    ? self._statusTotal + ' thumbnails decoded.'
                    : 'Decoding ' + self._statusDone + ' of ' + self._statusTotal + ' thumbnails\u2026';
            }
        }
        if (self._statusDone >= self._statusTotal && bar) {
            // Brief delay so the user sees the bar reach 100% before it
            // fades out. The CSS transition handles the actual fade.
            setTimeout(function() {
                if (bar) bar.classList.add('complete');
            }, 350);
        }
    };

    // ---------------------------------------------------------------------
    // Shared animated-thumbnail scheduler
    //
    // The previous code drove every animated thumbnail with its own
    // `setTimeout` recursion. That had three problems:
    //   1) setTimeout is not vsync-aligned, so multiple tiles draw on
    //      arbitrary unsynchronised cadences -> visible tearing/jitter.
    //   2) Animations kept ticking even when the tile was scrolled out of
    //      view or the tab was hidden, wasting CPU and pushing GC pressure.
    //   3) On tab refocus, queued timeouts fire in a burst, producing a
    //      noticeable stutter the moment the user looks at the page.
    //
    // The new scheduler is a single rAF loop that:
    //   - is vsync-aligned (smooth, no tearing),
    //   - only advances tiles whose element is currently visible,
    //   - caps the number of tiles advanced per frame (concurrency budget),
    //   - automatically pauses while the tab is hidden,
    //   - automatically stops itself when no animations remain.
    //
    // Each animation state stores its own `nextDue` timestamp so individual
    // tiles still respect their per-frame timing (fps / per-sample timestamps).
    // ---------------------------------------------------------------------
    this._animTickScheduled = false;
    this._maxConcurrentAnims = this._isMobileDevice ? 1 : 4;

    this._scheduleAnimTick = function() {
        if (self._animTickScheduled) return;
        self._animTickScheduled = true;
        requestAnimationFrame(self._animTick);
    };

    this._animTick = function(now) {
        self._animTickScheduled = false;

        // Pause entirely while the tab is hidden. The visibilitychange
        // listener below will re-arm us when the user comes back.
        if (typeof document !== 'undefined' && document.hidden) return;

        var hasAny = false;
        var advanced = 0;
        var maxConcurrent = self._maxConcurrentAnims;
        var ids = Object.keys(self._animatedThumbnails);

        for (var i = 0; i < ids.length; i++) {
            var id = ids[i];
            var st = self._animatedThumbnails[id];
            if (!st || !st.running) continue;
            hasAny = true;

            // Visibility gate: don't burn CPU on tiles the user can't see.
            // _visibleItems is populated by IntersectionObserver. If the
            // observer isn't available, _visibleItems will still contain
            // every id (see _loadThumbnails fallback), so this is safe.
            if (self._visibleItems && self._visibleItems.size > 0
                && !self._visibleItems.has(id)) continue;

            if (advanced >= maxConcurrent) continue;

            if (st.nextDue == null || now >= st.nextDue) {
                advanced++;
                st.currentIndex = (st.currentIndex + 1) % st.frames.length;
                var frame = st.frames[st.currentIndex];

                try {
                    var imageData = new ImageData(
                        frame.canvasFrameData, frame.width, frame.height);
                    st.ctx.putImageData(imageData, 0, 0);
                } catch (e) {
                    // Decode-data race or memory pressure - skip this frame.
                }

                // Compute interval to the next frame. Use per-sample
                // timestamps when available, otherwise fall back to fps.
                var interval = 1000 / (st.fps || 24);
                if (st.timestamps && st.timestamps.length > 1) {
                    var nextIndex = (st.currentIndex + 1) % st.timestamps.length;
                    if (nextIndex > 0) {
                        var d = st.timestamps[nextIndex].t
                              - st.timestamps[st.currentIndex].t;
                        if (d > 0) interval = d;
                    }
                }
                // Clamp to ~60fps max - putImageData is CPU-bound.
                if (interval < 16) interval = 16;
                st.nextDue = now + interval;
            }
        }

        if (hasAny) self._scheduleAnimTick();
    };

    // Re-arm the scheduler when the user returns to the tab.
    if (typeof document !== 'undefined' && document.addEventListener) {
        document.addEventListener('visibilitychange', function() {
            if (!document.hidden) {
                // Reset nextDue on every running animation so they don't
                // burst-fire all the queued frames at once.
                for (var id in self._animatedThumbnails) {
                    var st = self._animatedThumbnails[id];
                    if (st && st.running) st.nextDue = null;
                }
                self._scheduleAnimTick();
            }
        });
    }

    // ---------------------------------------------------------------------
    // Paced thumbnail fade-in scheduler
    //
    // When a canvas is inserted into a tile we want a short opacity-only
    // crossfade to hide the placeholder->canvas swap. If we let every tile
    // start its fade the instant decode finishes, we get up to N parallel
    // fades on the same frame, which lines up exactly with the moment the
    // browser is doing the most work (decode just finished, layout settling,
    // GPU upload). That spike is the visible "lag".
    //
    // Instead we feed fades through a small queue and release at most one
    // per ~24ms via rAF. The total visible delay is invisible to the user
    // (one extra animation-frame per tile) but it spreads composite work
    // out evenly and reads as smoother.
    // ---------------------------------------------------------------------
    this._fadeQueue = [];
    this._fadeScheduled = false;
    this._fadeLastReleaseTs = 0;

    this._scheduleFadeIn = function(el) {
        if (!el) return;
        if (self._reducedMotion) {
            // Reduced motion: no fade, just appear.
            el.classList.remove('gallery-thumb-fade');
            return;
        }
        // Start hidden; will be revealed when the queue releases this entry.
        el.classList.add('gallery-thumb-fade');
        self._fadeQueue.push(el);
        if (!self._fadeScheduled) {
            self._fadeScheduled = true;
            requestAnimationFrame(self._processFadeQueue);
        }
    };

    this._processFadeQueue = function(ts) {
        self._fadeScheduled = false;
        if (self._fadeQueue.length === 0) return;

        // Release at most one tile per ~24ms (~2-3 frames). This caps the
        // composite work per frame regardless of how many tiles finished
        // decoding at the same time.
        var minInterval = 24;
        if (ts - self._fadeLastReleaseTs >= minInterval) {
            var el = self._fadeQueue.shift();
            if (el && el.isConnected) {
                el.classList.remove('gallery-thumb-fade');
                self._fadeLastReleaseTs = ts;
            }
        }
        if (self._fadeQueue.length > 0) {
            self._fadeScheduled = true;
            requestAnimationFrame(self._processFadeQueue);
        }
    };

    // ---------------------------------------------------------------------
    // Reserve real aspect ratio on the placeholder.
    //
    // CSS `column-count` masonry repacks the entire grid every time an item
    // changes height. The previous code shipped a placeholder fixed at 3:2
    // (padding-bottom: 66.67%) and let the height jump to whatever the
    // decoded canvas turned out to be - so every thumbnail decode triggered
    // a full grid reflow. With ~50 items decoding in parallel this is dozens
    // of reflows per second.
    //
    // We can do better: by the time _loadAndCacheItem has fileInfo we know
    // the real width/height. We write that aspect ratio onto the placeholder
    // so it already occupies the final size. The canvas swap then causes
    // zero relayout.
    // ---------------------------------------------------------------------
    this._applyPlaceholderAspect = function(itemEl, w, h, rotation) {
        if (!itemEl || !w || !h) return;
        // For 90/270 rotated content the visual aspect is swapped.
        var r = parseInt(rotation, 10) || 0;
        var visW = (r === 90 || r === 270) ? h : w;
        var visH = (r === 90 || r === 270) ? w : h;
        var aspect = visH / visW;

        // 1) Update the placeholder's padding-bottom so the masonry takes
        //    the right amount of vertical space *immediately* (before the
        //    canvas decode finishes).
        var placeholder = itemEl.querySelector('.gallery-item-placeholder');
        if (placeholder) {
            placeholder.style.paddingBottom = (aspect * 100).toFixed(4) + '%';
        }

        // In native-pixels mode we don't reserve a column-relative span -
        // the tile sizes itself to the bitmap. The flexbox layout in CSS
        // handles flow. We DO need to give the placeholder a concrete pixel
        // width that matches the thumbnail size that's about to land,
        // otherwise every tile causes a layout shift when the canvas
        // replaces a fixed-size square placeholder.
        if (self._isNativeMode && self._isNativeMode()) {
            itemEl.dataset.giAspect = aspect.toFixed(6);
            if (placeholder) {
                // Approximate the eventual thumbnail width: the larger
                // intrinsic edge gets capped at _targetThumbDim().
                var cap = self._targetThumbDim();
                var scale = Math.min(cap / visW, cap / visH, 1);
                var pw = Math.max(1, Math.round(visW * scale));
                placeholder.style.width = pw + 'px';
                // padding-bottom already in % above will give the right
                // height relative to this width.
            }
            return;
        }

        // 2) Set --gi-span for the CSS-Grid masonry. Each implicit row is
        //    `--gi-row-px` tall (4px). The tile's visual height = column
        //    width * aspect. We approximate column width from the live
        //    measurement; if it's not measurable yet (display:none, not in
        //    DOM yet) we fall back to a reasonable default.
        var rowPx = 4; // must match --gi-row-px in CSS
        var gapPx = 4; // must match `gap` in CSS
        // getBoundingClientRect on the item gives us the actual column width.
        var colW = itemEl.getBoundingClientRect().width;
        if (!colW || colW < 8) {
            // Fallback: approximate from the grid container.
            var grid = itemEl.parentElement;
            if (grid) {
                var gridW = grid.getBoundingClientRect().width;
                // Try to read the current grid-template-columns count via CSS.
                var cols = 5;
                var cs = window.getComputedStyle(grid);
                var tpl = cs && cs.gridTemplateColumns;
                if (tpl) {
                    var parts = tpl.split(/\s+/).filter(Boolean);
                    if (parts.length > 0) cols = parts.length;
                }
                colW = (gridW - (cols - 1) * gapPx) / cols;
            }
        }
        if (!colW || colW < 8) colW = 240;

        var visualPx = colW * aspect;
        // Round instead of ceil: with object-fit: cover on the canvas the
        // image fills the cell either way, so we want the cell as close to
        // the natural image height as possible. ceil systematically biased
        // every cell ~4px tall, which made grid gaps look inflated.
        var span = Math.max(1, Math.round((visualPx + gapPx) / (rowPx + gapPx)));
        itemEl.style.setProperty('--gi-span', span);

        // Remember the aspect so we can recompute span on viewport resize
        // without re-fetching fileInfo.
        itemEl.dataset.giAspect = aspect.toFixed(6);
    };

    /**
     * Recompute --gi-span for every tile whose aspect ratio we already know.
     * Called debounced on window resize, since the column width (and thus
     * the visual height per aspect ratio) changes with viewport.
     */
    this._recomputeAllSpans = function() {
        // Native-pixels mode doesn't use the grid-row-span trick - each
        // tile sits at its bitmap's intrinsic size in a flex layout.
        if (self._isNativeMode && self._isNativeMode()) return;
        var grid = document.getElementById('gallery-grid');
        if (!grid) return;
        var rowPx = 4;
        var gapPx = 4;
        var items = grid.querySelectorAll('.gallery-item[data-gi-aspect]');
        if (items.length === 0) return;
        // Read column width once from the first item to avoid forcing a
        // layout per tile.
        var colW = items[0].getBoundingClientRect().width;
        if (!colW || colW < 8) return;
        for (var i = 0; i < items.length; i++) {
            var aspect = parseFloat(items[i].dataset.giAspect);
            if (!aspect) continue;
            var visualPx = colW * aspect;
            // Round (not ceil) - matches _applyPlaceholderAspect; the
            // canvas's object-fit: cover absorbs the residual.
            var span = Math.max(1, Math.round((visualPx + gapPx) / (rowPx + gapPx)));
            items[i].style.setProperty('--gi-span', span);
        }
        // Re-fit any rotation wrappers - the masonry-rounded tile height
        // changes with column width, so the cover-scale needs to follow.
        var wrappers = grid.querySelectorAll('.gallery-rotation-wrapper');
        for (var w = 0; w < wrappers.length; w++) {
            self._fitAdaptiveRotationWrapper(wrappers[w]);
        }
    };

    /**
     * Compute and apply a cover-scale on a rotated thumbnail wrapper so
     * the inner canvas fully fills the masonry-rounded tile.
     *
     * Why this exists:
     * `_wrapRotatedCanvas` sizes the inner canvas via `width: 100%;
     * height: auto`, then applies `rotate(±90deg) scale(canvasW/canvasH)`.
     * That math fills the wrapper at the canvas's natural aspect ratio.
     * The masonry layer, however, rounds the tile's height up to the next
     * row-grid unit, so the tile is almost always 1-7 px taller than the
     * canvas's natural visual height. The hover overlay (.gallery-item::after
     * covers the entire tile) then visibly extends past the bottom of the
     * rotated thumbnail, which reads as a 1-7 px dark strip below the image
     * on hover.
     *
     * The fix is to measure the wrapper's actual rendered dimensions after
     * the giSpan has been applied, then increase the scale factor so the
     * rotated bitmap COVERS the wrapper (cropping a few pixels rather than
     * leaving a strip uncovered). Matches the `object-fit: cover` policy
     * already used on non-rotated tiles.
     *
     * Safe no-op if the wrapper isn't in the DOM yet, isn't actually
     * rotated, or we're in native mode (which uses fixed-pixel sizing
     * with no masonry rounding).
     */
    this._fitAdaptiveRotationWrapper = function(wrapper) {
        if (!wrapper || !wrapper.classList
            || !wrapper.classList.contains('gallery-rotation-wrapper')) return;
        if (self._isNativeMode && self._isNativeMode()) return;
        var canvas = wrapper.querySelector('canvas, img');
        if (!canvas) return;
        var rotation = parseInt(canvas.dataset && canvas.dataset.rotation, 10) || 0;
        if (rotation !== 90 && rotation !== 270) return;
        var rect = wrapper.getBoundingClientRect();
        var W = rect.width, H = rect.height;
        if (!W || !H) return;
        var cw = canvas.width || canvas.naturalWidth;
        var ch = canvas.height || canvas.naturalHeight;
        if (!cw || !ch) return;
        // Pre-rotation, canvas displays at width=W and height=W*ch/cw.
        // After rotate(±90), visible width = W*ch/cw and visible height = W.
        // Natural scale = cw/ch fills width to W; we additionally need
        // visible height to be >= H (the masonry-rounded tile height).
        // Cover scale = max(natural-fill-width, fill-height).
        var naturalScale = cw / ch;
        var fillScale = H / W;
        var s = (fillScale > naturalScale) ? fillScale : naturalScale;
        var deg = (rotation === 90) ? '-90' : '-270';
        canvas.style.transform =
            'translate(-50%, -50%) rotate(' + deg + 'deg) scale(' + s + ')';
    };

    // Debounced resize listener. We don't run on every resize event
    // (Chromium fires those during drag-resize at ~60Hz); instead we coalesce
    // through rAF and only act once per frame.
    this._resizePending = false;
    if (typeof window !== 'undefined' && window.addEventListener) {
        window.addEventListener('resize', function() {
            if (self._resizePending) return;
            self._resizePending = true;
            requestAnimationFrame(function() {
                self._resizePending = false;
                self._recomputeAllSpans();
            });
        }, { passive: true });
    }

    // =========================================================================
    // MEMORY MANAGEMENT
    // =========================================================================
    
    /**
     * Schedule periodic memory cleanup to reduce GC pressure
     * Releases non-visible cached data and triggers worker cleanup
     *
     * Uses requestIdleCallback so cleanup never competes with rendering, and
     * skips re-arming itself when there's nothing to clean (no animated tiles
     * running, nothing pending to load). Previously this was a permanent
     * 30s setTimeout chain that kept the tab off Chrome's "fully idle" list,
     * blocking aggressive frequency scaling.
     */
    this._scheduleMemoryCleanup = function() {
        if (self._memoryCleanupTimer) {
            clearTimeout(self._memoryCleanupTimer);
            self._memoryCleanupTimer = null;
        }
        if (self._memoryCleanupIdleHandle) {
            try { (window.cancelIdleCallback || function(){})(self._memoryCleanupIdleHandle); }
            catch (e) {}
            self._memoryCleanupIdleHandle = null;
        }

        // Nothing to do? Don't re-arm. _triggerVisibleLoad and
        // _createAnimatedThumbnail will call _scheduleMemoryCleanup again
        // when there's mutable state worth cleaning.
        var hasAnims = false;
        for (var k in self._animatedThumbnails) {
            if (self._animatedThumbnails[k] && self._animatedThumbnails[k].running) {
                hasAnims = true; break;
            }
        }
        var hasPending = self._pendingItems && self._pendingItems.size > 0;
        var hasLoading = self._loadingItems && self._loadingItems.size > 0;
        if (!hasAnims && !hasPending && !hasLoading) return;

        // Two-stage delay: a 30s setTimeout sets a hard floor on how
        // often cleanup can run, then requestIdleCallback inside that
        // gates on the renderer being idle. Without the setTimeout
        // floor, an active animated thumbnail (which is always present
        // on this gallery: starfield/sea1/rally/bird) keeps `hasAnims`
        // true so the rIC reschedules itself every idle slot, spinning
        // the cleanup at ~60Hz and burning CPU for no work.
        var run = function() {
            self._memoryCleanupIdleHandle = null;
            self._memoryCleanupTimer = null;
            self._performMemoryCleanup();
            self._scheduleMemoryCleanup(); // Reschedule only if still needed
        };
        var armIdle = function() {
            self._memoryCleanupTimer = null;
            if (typeof window.requestIdleCallback === 'function') {
                self._memoryCleanupIdleHandle = window.requestIdleCallback(run, { timeout: 60000 });
            } else {
                run();
            }
        };
        self._memoryCleanupTimer = setTimeout(armIdle, 30000);
    };
    
    /**
     * Perform memory cleanup - release cached data for non-visible items
     */
    this._performMemoryCleanup = function() {
        var isMobile = self._isMobileDevice;
        
        // On mobile, be more aggressive with cleanup
        if (isMobile) {
            // Release cached fileInfo for all items not currently visible
            var allItems = self._normalItems.concat(self._conformanceItems).concat(self._vvcItems).concat(self._experimentalItems);
            var visibleIds = self._visibleItems || new Set();
            
            allItems.forEach(function(item) {
                if (!visibleIds.has(item.id) && item.fileInfo) {
                    // Keep minimal data, release heavy fileInfo
                    item.fileInfo = null;
                    item.needsReload = true;
                }
            });
        }
        
    };
    
    /**
     * Stop memory cleanup (call when gallery is destroyed)
     */
    this._stopMemoryCleanup = function() {
        if (self._memoryCleanupTimer) {
            clearTimeout(self._memoryCleanupTimer);
            self._memoryCleanupTimer = null;
        }
        if (self._memoryCleanupIdleHandle) {
            try { (window.cancelIdleCallback || function(){})(self._memoryCleanupIdleHandle); }
            catch (e) {}
            self._memoryCleanupIdleHandle = null;
        }
    };

    // =========================================================================
    // THUMBNAIL SIZE LIMITS
    // =========================================================================

    // Maximum dimension for gallery thumbnails (prevents loading 8K images as thumbnails)
    this.MAX_THUMBNAIL_DIMENSION = 640;

    // Primary-image dimension above which the deferred-preview gate kicks in.
    // (Roughly > QHD - typical 4K, 6K, 8K source.) Co-located with
    // MAX_THUMBNAIL_DIMENSION so the policy lives in one place.
    this.HEAVY_PRIMARY_DIMENSION = 2400;

    /**
     * Single source of truth for "how big should a thumbnail be?".
     * Used by every thumbnail-producing path (still, grid, overlay, animated)
     * so they all converge on the same ceiling. Previously grid/overlay used
     * a hard-coded 300 while the still path used 640, which made grid/overlay
     * tiles look noticeably lower-resolution in Native Pixels mode.
     */
    this._targetThumbDim = function() {
        return self.MAX_THUMBNAIL_DIMENSION;
    };

    /**
     * Rate-limited console.error.
     *
     * A set with N unsupported-codec files would otherwise dump N full
     * stack traces on every load attempt. We log the first occurrence of
     * each (key, path) pair fully; subsequent ones within the dedupe window
     * get a single short suppressed-count summary.
     *
     * @param {string} key  - logical category, e.g. 'grid-decode-fail'
     * @param {string} path - per-item differentiator, e.g. item.heifPath
     * @param {...*}    rest - normal console.error arguments
     */
    this._errorLog = {};
    this._logError = function(key, path) {
        var rest = Array.prototype.slice.call(arguments, 2);
        var bucket = self._errorLog[key] || (self._errorLog[key] = { firstAt: 0, count: 0, paths: {} });
        var now = Date.now();
        if (now - bucket.firstAt > 5000) {
            // Window expired: reset the bucket.
            bucket.firstAt = now;
            bucket.count = 0;
            bucket.paths = {};
        }
        bucket.count++;
        if (!bucket.paths[path]) {
            bucket.paths[path] = true;
            console.error.apply(console, [key + ':', path].concat(rest));
        } else if (bucket.count === 4) {
            // Once we've suppressed a few, drop a single summary so the dev
            // doesn't think the page silently broke.
            console.warn(key + ': further errors suppressed (5s window)');
        }
    };
    // =========================================================================
    // ROTATION HELPERS
    // =========================================================================
    
    /**
     * Apply CSS rotation to a canvas element based on HEIF rotation metadata
     * Per HEIF spec: irot rotates the reconstructed image in ANTI-CLOCKWISE direction
     * 
     * For thumbnails: 90°/270° rotations are handled by _wrapRotatedCanvas instead
     * For modal: this applies CSS classes for simple rotation
     * 
     * @param {HTMLCanvasElement} canvas - The canvas to rotate
     * @param {number} rotation - Rotation in degrees (0, 90, 180, 270)
     * @param {boolean} forThumbnail - If true, skip 90/270 (handled by wrapper)
     */
    this._applyRotation = function(canvas, rotation, forThumbnail) {
        if (!canvas || !rotation) return;
        
        rotation = parseInt(rotation, 10);
        if (rotation === 0) return;
        
        canvas.dataset.rotation = rotation;
        
        // For thumbnails, 90/270 are handled by _wrapRotatedCanvas
        if (forThumbnail && (rotation === 90 || rotation === 270)) {
            return; // Wrapper will handle the transform
        }
        
        if (rotation === 90 || rotation === 270) {
            canvas.className = 'heif-rotated-' + rotation;
            canvas.style.transformOrigin = 'center center';
        } else if (rotation === 180) {
            canvas.className = 'heif-rotated-180';
            canvas.style.transformOrigin = 'center center';
        }
    };
    
    /**
     * Apply mirror transformation to a canvas element
     * @param {HTMLCanvasElement} canvas - The canvas to apply mirror to
     * @param {Object} mirror - Mirror object with horizontalAxis property
     */
    this._applyMirror = function(canvas, mirror) {
        if (!canvas || !mirror) return;
        
        var existingClass = canvas.className || '';
        // Modern reader returns {axis: "vertical"|"horizontal"}.
        // Legacy embind returned {horizontalAxis: bool}.  Accept both.
        var isHorizontalAxis = mirror.horizontalAxis
            || mirror.axis === 'horizontal';
        // horizontalAxis: vertical mirroring (top/bottom exchanged)
        // vertical axis: horizontal mirroring (left/right exchanged)
        if (isHorizontalAxis) {
            canvas.className = existingClass + (existingClass ? ' ' : '') + 'heif-mirrored-horizontal';
        } else {
            canvas.className = existingClass + (existingClass ? ' ' : '') + 'heif-mirrored-vertical';
        }
    };
    
    /**
     * Apply all transformative properties to a canvas
     * Order per HEIF spec: LSEL -> CLAP -> ISCL -> IROT -> IMIR
     * 
     * @param {HTMLCanvasElement} canvas - Canvas with the original image data
     * @param {Object} meta - Metadata with transformation properties
     * @param {boolean} forThumbnail - If true, use thumbnail-specific handling
     * @returns {HTMLCanvasElement} - The transformed canvas (may be different from input for CLAP/ISCL)
     */

    /**
     * Step-downsample a canvas for modal display using iterative 2× halving.
     *
     * Browsers (Chrome especially) CSS-downscale <canvas> with a single-pass
     * bilinear filter, which produces visible staircase aliasing when the
     * backing store is much larger than the on-screen size (typical for 8K
     * content displayed in a ≤4K viewport).  Halving in successive drawImage
     * passes is the standard workaround: each 2× step is well within the
     * bilinear filter's capability, so the final result is smooth.
     *
     * The target size is `screen pixels × devicePixelRatio` so the canvas
     * backs the CSS display 1 : 1 at the native panel resolution — no further
     * CSS scaling needed.
     *
     * Only fires when the image is >2× the target on at least one axis;
     * images that fit naturally are returned untouched.
     *
     * @param {HTMLCanvasElement} src - Full-resolution canvas with image data
     * @returns {HTMLCanvasElement} - Downsampled canvas (may be src itself)
     */
    this._stepDownsampleForModal = function(src) {
        var dpr = window.devicePixelRatio || 1;
        var targetW = Math.round(window.innerWidth * dpr);
        var targetH = Math.round(window.innerHeight * dpr);

        // Only act when the source is significantly larger than 2× the target
        // (we intentionally stop at ~2× so the browser's final CSS downscale
        // is a gentle 2:1 — sharp without staircase aliasing).
        if (src.width <= targetW * 2 && src.height <= targetH * 2) return src;

        // Compute final dimensions: fit inside 2× target while preserving aspect
        var aspect = src.width / src.height;
        var finalW, finalH;
        if (aspect > targetW / targetH) {
            finalW = targetW * 2;
            finalH = Math.round((targetW * 2) / aspect);
        } else {
            finalH = targetH * 2;
            finalW = Math.round((targetH * 2) * aspect);
        }
        // Never upscale
        if (finalW >= src.width && finalH >= src.height) return src;

        // Iterative 2× halving
        var cur = src;
        while (cur.width > finalW * 2 || cur.height > finalH * 2) {
            var halfW = Math.max(Math.round(cur.width / 2), finalW);
            var halfH = Math.max(Math.round(cur.height / 2), finalH);
            var step = document.createElement('canvas');
            step.width = halfW;
            step.height = halfH;
            var sctx = step.getContext('2d');
            if (!sctx) return src;
            sctx.imageSmoothingEnabled = true;
            sctx.imageSmoothingQuality = 'high';
            sctx.drawImage(cur, 0, 0, halfW, halfH);
            // Release intermediate (but not the original src — caller may still reference it)
            if (cur !== src) { cur.width = 0; cur.height = 0; }
            cur = step;
        }

        // Final pass to exact 2× target size
        var out = document.createElement('canvas');
        out.width = finalW;
        out.height = finalH;
        var octx = out.getContext('2d');
        if (!octx) return src;
        octx.imageSmoothingEnabled = true;
        octx.imageSmoothingQuality = 'high';
        octx.drawImage(cur, 0, 0, finalW, finalH);
        if (cur !== src) { cur.width = 0; cur.height = 0; }
        return out;
    };

    this._applyTransformations = function(canvas, meta, forThumbnail) {
        if (!canvas || !meta) return canvas;
        
        var outputCanvas = canvas;
        
        // 1. LSEL - Layer Selection (placeholder - not yet implemented)
        if (meta.layerSelection) {
            console.warn('Gallery: LSEL layer_id=' + (meta.layerSelection.layerId || 0) + ' (not yet implemented)');
        }
        
        // 2. CLAP - Clean Aperture (crop)
        // 3. ISCL - Image Scaling
        // For thumbnails, these are already handled during thumbnail generation
        // For full-size display, we need to apply them here
        if (!forThumbnail && (meta.cleanAperture || meta.scale)) {
            outputCanvas = self._applyCanvasTransformations(canvas, meta.cleanAperture, meta.scale);
        }
        
        // 4. IROT - Rotation (applied via CSS)
        if (meta.rotation) {
            self._applyRotation(outputCanvas, meta.rotation, forThumbnail);
        }
        
        // 5. IMIR - Mirror (applied via CSS)
        if (meta.mirror) {
            self._applyMirror(outputCanvas, meta.mirror);
        }
        
        return outputCanvas;
    };
    
    /**
     * Wrap a canvas in a container for proper modal display with 90/270 rotation
     * Unlike _wrapRotatedCanvas (for thumbnails), this respects viewport constraints
     * 
     * @param {HTMLCanvasElement} canvas - The rotated canvas
     * @param {number} rotation - Rotation in degrees (90 or 270)
     * @returns {HTMLElement} - Wrapper element or original canvas if no wrap needed
     */
    this._wrapRotatedCanvasForModal = function(canvas, rotation) {
        rotation = parseInt(rotation, 10);
        
        // Only need wrapper for 90/270 where dimensions swap
        if (rotation !== 90 && rotation !== 270) {
            return canvas;
        }
        
        var canvasW = canvas.width;   // Original (pre-rotation) width
        var canvasH = canvas.height;  // Original (pre-rotation) height
        
        // After rotation, visual dimensions are swapped
        var visualW = canvasH;
        var visualH = canvasW;
        
        // Calculate the maximum available space
        var maxW = window.innerWidth - 120;  // Account for padding
        var maxH = window.innerHeight - 200; // Account for info panel
        
        // Calculate the display size that fits within constraints
        var scale = Math.min(maxW / visualW, maxH / visualH, 1);
        var displayW = Math.round(visualW * scale);
        var displayH = Math.round(visualH * scale);
        
        // Create wrapper with explicit dimensions
        var wrapper = document.createElement('div');
        wrapper.className = 'gallery-modal-rotated-wrapper';
        wrapper.style.cssText = 
            'position: relative; ' +
            'width: ' + displayW + 'px; ' +
            'height: ' + displayH + 'px; ' +
            'overflow: hidden;';
        
        // The canvas needs to fill the wrapper after rotation
        // Canvas displays at: width=displayW*(canvasW/canvasH), height=displayW
        // After rotation, this becomes: width=displayW, height=displayW*(canvasW/canvasH)
        // We want rotated width = displayW, rotated height = displayH
        var canvasDisplayW = displayH;  // After rotation, this becomes the visual height
        var canvasDisplayH = displayW;  // After rotation, this becomes the visual width... wait
        
        // Simpler approach: size canvas so that after rotation it fills displayW x displayH
        // Pre-rotation canvas should be displayH wide (becomes displayH tall after 90° rotation)
        // and displayW tall (becomes displayW wide after 90° rotation)
        canvas.style.cssText = 
            'position: absolute; ' +
            'top: 50%; ' +
            'left: 50%; ' +
            'width: ' + displayH + 'px; ' +
            'height: ' + displayW + 'px; ' +
            'max-width: none; ' +
            'max-height: none; ' +
            'transform-origin: center center; ' +
            'transform: translate(-50%, -50%) rotate(' + (rotation === 90 ? '-90' : '-270') + 'deg);';
        
        wrapper.appendChild(canvas);
        return wrapper;
    };

    /**
     * Apply CLAP (crop) and ISCL (scale) transformations by redrawing to a new canvas
     * @param {HTMLCanvasElement} srcCanvas - Source canvas with image
     * @param {Object|null} cleanAperture - CLAP parameters
     * @param {Object|null} scale - ISCL parameters
     * @returns {HTMLCanvasElement} - New canvas with transformations applied
     */
    this._applyCanvasTransformations = function(srcCanvas, cleanAperture, scale) {
        var srcWidth = srcCanvas.width;
        var srcHeight = srcCanvas.height;
        
        // Source rectangle (for CLAP crop)
        var sx = 0, sy = 0, sw = srcWidth, sh = srcHeight;
        
        if (cleanAperture && cleanAperture.widthD && cleanAperture.heightD) {
            var clapWidth = cleanAperture.widthN / cleanAperture.widthD;
            var clapHeight = cleanAperture.heightN / cleanAperture.heightD;
            var horizOff = (cleanAperture.horizontalOffsetN || 0) / (cleanAperture.horizontalOffsetD || 1);
            var vertOff = (cleanAperture.verticalOffsetN || 0) / (cleanAperture.verticalOffsetD || 1);
            
            // Calculate picture center
            var pcX = horizOff + (srcWidth - 1) / 2;
            var pcY = vertOff + (srcHeight - 1) / 2;
            
            // Calculate crop rectangle
            sx = Math.max(0, Math.round(pcX - (clapWidth - 1) / 2));
            sy = Math.max(0, Math.round(pcY - (clapHeight - 1) / 2));
            sw = Math.min(Math.round(clapWidth), srcWidth - sx);
            sh = Math.min(Math.round(clapHeight), srcHeight - sy);
        }
        
        // Output dimensions (after crop, before scale)
        var outWidth = sw;
        var outHeight = sh;
        
        // Apply ISCL scaling
        if (scale && scale.targetWidthD && scale.targetHeightD) {
            outWidth = Math.ceil((sw * scale.targetWidthN) / scale.targetWidthD);
            outHeight = Math.ceil((sh * scale.targetHeightN) / scale.targetHeightD);
        }
        
        // If no actual transformation needed, return original
        if (sx === 0 && sy === 0 && sw === srcWidth && sh === srcHeight && 
            outWidth === srcWidth && outHeight === srcHeight) {
            return srcCanvas;
        }
        
        // Create new canvas with transformed content
        var outCanvas = document.createElement('canvas');
        outCanvas.width = outWidth;
        outCanvas.height = outHeight;
        var ctx = outCanvas.getContext('2d');
        
        // Draw with crop and scale
        ctx.drawImage(srcCanvas, sx, sy, sw, sh, 0, 0, outWidth, outHeight);
        
        return outCanvas;
    };
    
    /**
     * Wrap a rotated canvas in a container that has the correct (rotated) aspect ratio
     * For 90°/270° rotations, the visual width/height are swapped.
     * 
     * @param {HTMLCanvasElement} canvas - The canvas to wrap
     * @param {number} rotation - Rotation in degrees (90 or 270)
     * @returns {HTMLElement} - The wrapper element (or canvas if no wrapper needed)
     */
    this._wrapRotatedCanvas = function(canvas, rotation) {
        rotation = parseInt(rotation, 10);
        
        // Only need wrapper for 90/270 where dimensions swap
        if (rotation !== 90 && rotation !== 270) {
            return canvas;
        }
        
        var canvasW = canvas.width;   // e.g., 1920 (landscape source)
        var canvasH = canvas.height;  // e.g., 1080
        
        // After rotation, visual is swapped: 1080 × 1920
        var visualW = canvasH;  // 1080
        var visualH = canvasW;  // 1920
        var visualAspect = visualH / visualW;  // 1.78 (portrait)
        
        // Create wrapper with visual aspect ratio
        var wrapper = document.createElement('div');
        wrapper.className = 'gallery-rotation-wrapper';

        // Native-pixels mode uses a flexbox with `width: auto` on each tile,
        // so a wrapper sized via `width: 100%; padding-bottom: X%` collapses
        // to 0×0 (no parent width to anchor against). The tile then disappears
        // entirely, which is why portrait/landscape Apple HEICs were missing
        // from native view while square ones still rendered. In native mode
        // give the wrapper explicit pixel dimensions matching the rotated
        // canvas so flexbox has something to lay out.
        if (self._isNativeMode && self._isNativeMode()) {
            wrapper.style.cssText =
                'position: relative; ' +
                'width: ' + visualW + 'px; ' +
                'height: ' + visualH + 'px;';
            canvas.style.cssText =
                'position: absolute; ' +
                'top: 50%; ' +
                'left: 50%; ' +
                'width: ' + canvasW + 'px; ' +
                'height: ' + canvasH + 'px; ' +
                'max-width: none; ' +
                'transform-origin: center center; ' +
                'transform: translate(-50%, -50%) rotate(' + (rotation === 90 ? '-90' : '-270') + 'deg);';
            wrapper.appendChild(canvas);
            return wrapper;
        }

        wrapper.style.cssText =
            'position: relative; ' +
            'width: 100%; ' +
            'padding-bottom: ' + (visualAspect * 100) + '%;';
        
        // The canvas needs to:
        // 1. Be sized so that after rotation it fills the wrapper
        // 2. Wrapper width = W (unknown), wrapper height = W * visualAspect
        // 
        // After rotation:
        //   visual width = canvasH (in canvas pixels)
        //   visual height = canvasW (in canvas pixels)
        // 
        // We want visual width to equal wrapper width (W)
        // So canvasH should display as W pixels
        // Canvas CSS width should be: W * (canvasW / canvasH) to maintain aspect ratio
        // But after rotation, displayed width becomes height...
        // 
        // Simpler: set canvas width to 100%, which makes canvas display width = W
        // Canvas display height = W * (canvasH / canvasW)
        // After 90° rotation:
        //   rotated width = original height = W * (canvasH / canvasW)
        //   rotated height = original width = W
        // We want rotated width = W, so we need to scale by canvasW / canvasH
        
        var scaleAfterRotation = canvasW / canvasH;
        
        canvas.style.cssText = 
            'position: absolute; ' +
            'top: 50%; ' +
            'left: 50%; ' +
            'width: 100%; ' +
            'height: auto; ' +
            'transform-origin: center center; ' +
            'transform: translate(-50%, -50%) rotate(' + (rotation === 90 ? '-90' : '-270') + 'deg) scale(' + scaleAfterRotation + ');';
        
        wrapper.appendChild(canvas);
        return wrapper;
    };
    
    /**
     * Downscale a frame's image data if it exceeds MAX_THUMBNAIL_DIMENSION
     * This prevents large images (without embedded thumbnails) from bloating the gallery
     *
     * Uses a single shared (src, dst) canvas pair, resized in place. The
     * previous implementation allocated 2 fresh canvases per call, which
     * meant 120 allocs for a 60-frame 4K animation - measurable allocator
     * pressure on lower-end devices.
     *
     * @param {Object} frame - Frame object with canvasFrameData, width, height
     * @returns {Object} - Possibly downscaled frame (same structure)
     */
    this._dsSrcCanvas = null;
    this._dsDstCanvas = null;
    this._dsLastUse = 0;
    this._dsShrinkTimer = null;
    // Idle threshold after which the pooled canvases are shrunk back to
    // 0x0. The browser releases the backing bitmap when a canvas is
    // resized to 0, which matters because the pooled src canvas can hold
    // the full primary-image buffer (8000x6000 RGBA = ~192 MB) for as
    // long as the user keeps the gallery tab open. Without this, peak
    // memory only grows.
    this._DS_IDLE_MS = 15000;
    this._dsScheduleShrink = function() {
        if (self._dsShrinkTimer) clearTimeout(self._dsShrinkTimer);
        self._dsShrinkTimer = setTimeout(function() {
            self._dsShrinkTimer = null;
            // Only shrink if truly idle - another decode may have landed
            // between the schedule and the fire.
            if ((Date.now() - self._dsLastUse) < self._DS_IDLE_MS) {
                self._dsScheduleShrink();
                return;
            }
            try {
                if (self._dsSrcCanvas) { self._dsSrcCanvas.width = 0; self._dsSrcCanvas.height = 0; }
                if (self._dsDstCanvas) { self._dsDstCanvas.width = 0; self._dsDstCanvas.height = 0; }
            } catch (e) { /* ignore */ }
        }, self._DS_IDLE_MS);
    };
    this._downscaleForThumbnail = function(frame) {
        var maxDim = self.MAX_THUMBNAIL_DIMENSION;

        // Check if downscaling is needed
        if (frame.width <= maxDim && frame.height <= maxDim) {
            return frame; // No scaling needed
        }

        // Calculate scale factor to fit within maxDim
        var scale = Math.min(maxDim / frame.width, maxDim / frame.height);
        var newWidth = Math.round(frame.width * scale);
        var newHeight = Math.round(frame.height * scale);

        // Ensure canvasFrameData is Uint8ClampedArray for ImageData constructor
        var srcData = frame.canvasFrameData;
        if (!(srcData instanceof Uint8ClampedArray)) {
            srcData = new Uint8ClampedArray(srcData);
        }

        // Lazy-init pooled canvases. Setting width/height clears the buffer,
        // which is exactly what we want before each putImageData / drawImage.
        if (!self._dsSrcCanvas) self._dsSrcCanvas = document.createElement('canvas');
        if (!self._dsDstCanvas) self._dsDstCanvas = document.createElement('canvas');
        var srcCanvas = self._dsSrcCanvas;
        var dstCanvas = self._dsDstCanvas;

        srcCanvas.width = frame.width;
        srcCanvas.height = frame.height;
        var srcCtx = srcCanvas.getContext('2d');
        var srcImageData = new ImageData(srcData, frame.width, frame.height);
        srcCtx.putImageData(srcImageData, 0, 0);

        dstCanvas.width = newWidth;
        dstCanvas.height = newHeight;
        var dstCtx = dstCanvas.getContext('2d', { willReadFrequently: true });

        // Use high quality scaling
        dstCtx.imageSmoothingEnabled = true;
        dstCtx.imageSmoothingQuality = 'high';
        dstCtx.drawImage(srcCanvas, 0, 0, newWidth, newHeight);

        // Get scaled image data BEFORE the next call resizes the dst buffer.
        // Note: getImageData allocates a fresh Uint8ClampedArray, so the
        // returned data is safely owned by the frame object even after the
        // pooled canvas is reused.
        var dstImageData = dstCtx.getImageData(0, 0, newWidth, newHeight);

        // Touch idle clock and (re)schedule shrink. Cheap - no allocation.
        self._dsLastUse = Date.now();
        self._dsScheduleShrink();

        // Return new frame object with scaled data
        return {
            width: newWidth,
            height: newHeight,
            canvasFrameData: dstImageData.data,
            bitDepth: frame.bitDepth,
            originalWidth: frame.width,
            originalHeight: frame.height
        };
    };

    // =========================================================================
    // INITIALIZATION
    // =========================================================================
    
    /**
     * Initialize the gallery
     */
    this.initialize = function() {
        // Create the set switcher UI
        self._createSetSwitcher();
        
        // Load the current set
        self._loadCurrentSet();
        self._setupModal();
        
        // Start periodic memory cleanup
        self._scheduleMemoryCleanup();
    };
    
    /**
     * Create the gallery set switcher and the layout-mode toggle.
     *
     * Set switcher is appended directly to the header (matches the
     * original flat design). The layout-mode toggle is placed in its own
     * thin, right-aligned text row between the status bar and the grid -
     * separate from the status bar so it stays visible (and stable) after
     * loading completes regardless of the status bar's fade state.
     *
     * Both are plain text controls; this site avoids pill / heavy-chrome UI.
     */
    this._createSetSwitcher = function() {
        var header = document.querySelector('.gallery-header');
        if (!header) return;

        // -------- Set switcher (Examples / Conformance / Experimental) --------
        var switcher = document.createElement('div');
        switcher.className = 'gallery-set-switcher';
        switcher.setAttribute('role', 'group');
        switcher.setAttribute('aria-label', 'Gallery set');

        var sets = [
            { key: 'normal',       label: 'Examples' },
            { key: 'vvc',          label: 'VVC (H.266)' },
            { key: 'conformance',  label: 'Conformance Files' }
        ];
        sets.forEach(function(s) {
            var btn = document.createElement('button');
            btn.type = 'button';
            btn.className = 'gallery-set-btn' + (self._currentSet === s.key ? ' active' : '');
            btn.setAttribute('data-set', s.key);
            btn.setAttribute('aria-pressed', self._currentSet === s.key ? 'true' : 'false');
            btn.textContent = s.label;
            btn.addEventListener('click', function() {
                var newSet = this.getAttribute('data-set');
                if (newSet !== self._currentSet) self._switchSet(newSet);
            });
            switcher.appendChild(btn);
        });
        header.appendChild(switcher);

        // -------- Layout-mode toggle (Adaptive grid / Native pixels) --------
        // Plain text toggle: "View:  Adaptive  ·  Native pixels".
        // Inserted just before the grid so it sits above the first row of
        // thumbnails and is independent of the loading status bar.
        var grid = document.getElementById('gallery-grid');
        if (grid && grid.parentNode) {
            var toggleRow = document.createElement('div');
            toggleRow.className = 'gallery-view-control';

            var label = document.createElement('span');
            label.className = 'gallery-view-control-label';
            label.textContent = 'View:';
            toggleRow.appendChild(label);

            var toggle = document.createElement('span');
            toggle.className = 'gallery-view-toggle';
            toggle.setAttribute('role', 'group');
            toggle.setAttribute('aria-label', 'Thumbnail layout mode');

            var layouts = [
                { key: 'adaptive', label: 'Grid',
                  title: 'Masonry layout - thumbnails scaled to fill columns' },
                { key: 'native',   label: 'Adaptive',
                  title: 'Each thumbnail at its real embedded bitmap size, no scaling' }
            ];
            layouts.forEach(function(l, idx) {
                if (idx > 0) {
                    var sep = document.createElement('span');
                    sep.className = 'gallery-view-toggle-sep';
                    sep.setAttribute('aria-hidden', 'true');
                    sep.textContent = '\u00B7'; // middle dot
                    toggle.appendChild(sep);
                }
                var btn = document.createElement('button');
                btn.type = 'button';
                btn.className = 'gallery-view-btn' + (self._layoutMode === l.key ? ' active' : '');
                btn.setAttribute('data-layout', l.key);
                btn.setAttribute('aria-pressed', self._layoutMode === l.key ? 'true' : 'false');
                btn.title = l.title;
                btn.textContent = l.label;
                btn.addEventListener('click', function() {
                    var mode = this.getAttribute('data-layout');
                    if (mode !== self._layoutMode) self._setLayoutMode(mode);
                });
                toggle.appendChild(btn);
            });
            toggleRow.appendChild(toggle);
            grid.parentNode.insertBefore(toggleRow, grid);
        }

        // Apply the currently-selected layout mode to the grid up front so
        // CSS rules for native mode are in place before the first item is
        // inserted.
        self._applyLayoutModeClass();
    };
    
    /**
     * Switch to a different gallery set.
     *
     * Crossfades the grid out, swaps content, fades back in. Uses the
     * .is-switching class on .gallery-grid (CSS handles the opacity).
     */
    this._switchSet = function(setName) {
        if (setName === self._currentSet) return;
        self._currentSet = setName;

        // Persist selection to localStorage
        try {
            localStorage.setItem('gallerySet', setName);
        } catch (e) {
            console.warn('Could not save gallery set preference:', e);
        }

        // Update both the active class and aria-pressed state.
        var buttons = document.querySelectorAll('.gallery-set-btn');
        for (var i = 0; i < buttons.length; i++) {
            var on = buttons[i].getAttribute('data-set') === setName;
            buttons[i].classList.toggle('active', on);
            buttons[i].setAttribute('aria-pressed', on ? 'true' : 'false');
        }
        
        // Reset all per-set load state (animations, queues, counters,
        // parallel-decode ramp). Single entry point shared with
        // _loadCurrentSet.
        self._resetLoadState();

        // Crossfade: fade the grid out, swap content, fade back in. The
        // CSS class `.is-switching` carries the opacity transition; we
        // wait one transition duration before rebuilding so the user sees
        // the old grid fade out smoothly instead of a hard wipe.
        var grid = document.getElementById('gallery-grid');
        if (grid && !self._reducedMotion) {
            grid.classList.add('is-switching');
            // Status bar reset to "preparing" while we swap.
            self._statusReset();
            setTimeout(function() {
                self._loadCurrentSet();
                // Allow one frame for the new DOM to commit before fading
                // back in.
                requestAnimationFrame(function() {
                    grid.classList.remove('is-switching');
                });
            }, 200); // matches CSS .gallery-grid transition duration
        } else {
            // Reduced-motion users skip the fade.
            self._statusReset();
            self._loadCurrentSet();
        }
    };
    
    /**
     * Load the current gallery set
     */
    this._loadCurrentSet = function() {
        // Single source of truth for per-set state reset.
        self._resetLoadState();

        var grid = document.getElementById('gallery-grid');
        grid.replaceChildren(); // Clear existing items
        // Re-apply the layout-mode class - innerHTML wipe doesn't touch
        // grid attributes, but be defensive in case CSS class state was
        // toggled elsewhere.
        self._applyLayoutModeClass();
        
        var displayItems = self._getDisplayItems();

        // Drive the combined status bar from the actual queue size.
        self._statusSetTotal(displayItems.length);
        
        // Build all placeholder items into a DocumentFragment so we hit the
        // DOM exactly once, instead of triggering a layout invalidation per
        // appendChild. With ~60 conformance items this is a measurable win
        // on the very first frame the user sees.
        var frag = document.createDocumentFragment();
        for (var i = 0; i < displayItems.length; i++) {
            frag.appendChild(self._createGalleryItem(displayItems[i]));
        }
        grid.appendChild(frag);
        
        // Load thumbnails (this will fetch fileInfo and cache it)
        self._loadThumbnails(displayItems);
    };

    /**
     * Get items in display order (optionally shuffled)
     */
    this._getDisplayItems = function() {
        var items;
        if (self._currentSet === 'conformance') {
            items = self._conformanceItems;
        } else if (self._currentSet === 'vvc') {
            items = self._vvcItems;
        } else if (self._currentSet === 'experimental') {
            items = self._experimentalItems;
        } else {
            items = self._normalItems;
        }
        
        if (self.SHUFFLE_GALLERY && self._currentSet === 'normal') {
            return self._shuffleArray(items.slice(), self.SHUFFLE_SEED);
        }
        return items;
    };

    /**
     * Seeded pseudo-random number generator
     */
    this._seededRandom = function(seed) {
        return function() {
            var t = seed += 0x6D2B79F5;
            t = Math.imul(t ^ t >>> 15, t | 1);
            t ^= t + Math.imul(t ^ t >>> 7, t | 61);
            return ((t ^ t >>> 14) >>> 0) / 4294967296;
        };
    };

    /**
     * Fisher-Yates shuffle
     */
    this._shuffleArray = function(array, seed) {
        var shuffled = array.slice();
        var random = seed !== null ? self._seededRandom(seed) : Math.random;
        for (var i = shuffled.length - 1; i > 0; i--) {
            var j = Math.floor(random() * (i + 1));
            var temp = shuffled[i];
            shuffled[i] = shuffled[j];
            shuffled[j] = temp;
        }
        return shuffled;
    };

    // =========================================================================
    // DOM CREATION
    // =========================================================================

    /**
     * Create gallery item DOM element (without type badge - added after load)
     */
    this._createGalleryItem = function(item) {
        var div = document.createElement('div');
        div.className = 'gallery-item';
        if (item.pixelHonest) div.classList.add('gallery-pixel-honest');
        div.id = item.id;
        div.dataset.heifPath = item.heifPath;
        div.dataset.title = item.title;

        // Accessibility: each tile is a keyboard-operable list item.
        // Previously these were plain <div>s with a click handler -
        // unreachable by keyboard, invisible to screen readers, and missing
        // any focus styling. The CSS adds :focus-visible outline.
        div.setAttribute('role', 'listitem');
        div.setAttribute('tabindex', '0');
        // aria-label uses the human-readable title; the modal contains the
        // full file metadata for screen readers that drill in.
        div.setAttribute('aria-label', item.title);

        var placeholder = document.createElement('div');
        placeholder.className = 'gallery-item-placeholder';
        // The skeleton is decorative - hide it from AT.
        placeholder.setAttribute('aria-hidden', 'true');
        div.appendChild(placeholder);

        var info = document.createElement('div');
        info.className = 'gallery-item-info';
        // Build with textContent rather than innerHTML to avoid HTML
        // injection from item.title (theoretically the gallery list is
        // hard-coded but this is hygienic).
        var titleEl = document.createElement('div');
        titleEl.className = 'gallery-item-title';
        titleEl.textContent = item.title;
        var metaEl = document.createElement('div');
        metaEl.className = 'gallery-item-meta';
        var dimsEl = document.createElement('span');
        dimsEl.className = 'gallery-item-dims';
        metaEl.appendChild(dimsEl);
        info.appendChild(titleEl);
        info.appendChild(metaEl);
        div.appendChild(info);

        var open = function() { self._openModal(item); };
        div.addEventListener('click', open);
        // Keyboard activation: Enter and Space behave like a button.
        // preventDefault on Space avoids scrolling the page.
        div.addEventListener('keydown', function(e) {
            if (e.key === 'Enter' || e.key === ' ' || e.key === 'Spacebar') {
                e.preventDefault();
                open();
            }
        });

        return div;
    };

    /**
     * Add type badge to gallery item after type is derived
     */
    this._addTypeBadge = function(itemEl, type) {
        if (!type || type === 'still') return;
        
        var badge = document.createElement('span');
        badge.className = 'gallery-item-badge';
        switch (type) {
            case 'animation': badge.textContent = 'Animation'; break;
            case 'sequence': badge.textContent = 'Sequence'; break;
            case 'burst': badge.textContent = 'Burst'; break;
            case 'grid': badge.textContent = 'Grid'; break;
            case 'overlay': badge.textContent = 'Overlay'; break;
            case 'alpha': badge.textContent = 'Alpha'; break;
            case 'tmap': badge.textContent = 'HDR'; break;
            case 'stereo': badge.textContent = 'Stereo'; break;
            case 'bothie': badge.textContent = 'Bothie'; break;
            case 'collection': badge.textContent = 'Collection'; break;
            // New entity group types
            case 'panorama': badge.textContent = 'Panorama'; break;
            case 'slideshow': badge.textContent = 'Slideshow'; break;
            case 'progressive': badge.textContent = 'Progressive'; break;
            case 'audio-image': badge.textContent = 'Image with Audio'; break;
            case 'bracket-exposure': badge.textContent = 'AE Bracket'; break;
            case 'bracket-wb': badge.textContent = 'WB Bracket'; break;
            case 'bracket-focus': badge.textContent = 'Focus Bracket'; break;
            case 'bracket-flash': badge.textContent = 'Flash Bracket'; break;
            case 'bracket-dof': badge.textContent = 'DoF Bracket'; break;
            case 'album': badge.textContent = 'Album'; break;
            case 'favorites': badge.textContent = 'Favorites'; break;
            case 'altr': badge.textContent = 'Alternates'; break;
            case 'iden': badge.textContent = 'Identity Derived'; break;
            default: badge.textContent = type; break;
        }
        
        var info = itemEl.querySelector('.gallery-item-info');
        if (info) {
            itemEl.insertBefore(badge, info);
        } else {
            itemEl.appendChild(badge);
        }
    };

    /**
     * Add a codec badge (e.g. "VVC") to a gallery tile.
     * Shown alongside the type badge so VVC content is visually distinct.
     */
    this._addCodecBadge = function(itemEl, codec) {
        if (!codec) return;
        var upper = codec.toUpperCase();
        if (upper !== 'VVC' && upper !== 'VVC1' && upper !== 'VVI1') return;

        var badge = document.createElement('span');
        badge.className = 'gallery-item-badge gallery-item-badge-codec';
        badge.textContent = 'VVC';
        var info = itemEl.querySelector('.gallery-item-info');
        if (info) {
            itemEl.insertBefore(badge, info);
        } else {
            itemEl.appendChild(badge);
        }
    };

    /**
     * Add a SlimHEIF (mif3) badge to a gallery tile.
     * Shown alongside the codec badge on the left so mini files are
     * visually distinct.
     */
    this._addSlimBadge = function(itemEl, meta) {
        if (!meta) return;
        var ba = meta.brandAnalysis;
        if (!(ba && ba.isSlimHEIF)) return;

        var badge = document.createElement('span');
        badge.className = 'gallery-item-badge gallery-item-badge-slim';
        badge.textContent = 'SlimHEIF';
        var info = itemEl.querySelector('.gallery-item-info');
        if (info) {
            itemEl.insertBefore(badge, info);
        } else {
            itemEl.appendChild(badge);
        }
    };

    /**
     * Centralised reset of per-set load state.
     *
     * Called from BOTH _switchSet (before swap) and _loadCurrentSet (before
     * each rebuild). Previously these two entry points each cleared a
     * partially-overlapping subset of state, which made it possible (e.g.
     * during a fast double-click on the set switcher) to leave a stale
     * pending-items entry pointing at a torn-down DOM tile.
     */
    this._resetLoadState = function() {
        // Stop any running animations before dropping their handles.
        for (var id in self._animatedThumbnails) {
            if (self._animatedThumbnails[id]) {
                self._animatedThumbnails[id].running = false;
            }
        }
        self._animatedThumbnails = {};

        // Disconnect the IntersectionObserver. It still holds refs to the
        // soon-to-be-detached tiles otherwise; over many set switches
        // those refs accumulate even though the elements have been wiped
        // by `grid.innerHTML = ''`. _initIntersectionObserver also calls
        // disconnect() but only when reused mid-flight; the explicit reset
        // here makes ownership clear.
        if (self._intersectionObserver) {
            try { self._intersectionObserver.disconnect(); } catch (e) {}
        }

        // Drop pending/visible/loading sets - the new tiles get re-registered
        // by _loadThumbnails when it walks the new DOM.
        if (self._pendingItems && self._pendingItems.clear) self._pendingItems.clear();
        if (self._visibleItems && self._visibleItems.clear) self._visibleItems.clear();
        if (self._loadingItems && self._loadingItems.clear) self._loadingItems.clear();
        if (self._fadeQueue && self._fadeQueue.length) self._fadeQueue.length = 0;

        self._loadedCount = 0;
        self._loadQueueStartTs = null;
    };

    // =========================================================================
    // THUMBNAIL LOADING
    // =========================================================================

    // Intersection observer for lazy loading
    this._intersectionObserver = null;
    this._pendingItems = new Map(); // Map of itemId -> item
    this._visibleItems = new Set(); // Set of item IDs that are visible
    this._loadingItems = new Set(); // Set of item IDs currently loading

    /**
     * Initialize IntersectionObserver for lazy loading.
     *
     * The observer is created exactly once and reused across set switches.
     * Previously this was disconnected and rebuilt every switch, which
     * triggered a synchronous layout-tree walk for re-observation - the
     * worst possible time, since we were also rebuilding 60+ DOM nodes
     * on the same frame.
     */
    this._initIntersectionObserver = function() {
        if (self._intersectionObserver) {
            // Already exists - just stop watching the previous set's tiles.
            // The new set's tiles will be re-observed by _loadThumbnails.
            try { self._intersectionObserver.disconnect(); } catch (e) {}
            return;
        }

        if ('IntersectionObserver' in window) {
            self._intersectionObserver = new IntersectionObserver(function(entries) {
                entries.forEach(function(entry) {
                    var itemId = entry.target.getAttribute('data-item-id');
                    if (!itemId) return;

                    if (entry.isIntersecting) {
                        self._visibleItems.add(itemId);
                        // Trigger load if item is pending
                        if (self._pendingItems.has(itemId) && !self._loadingItems.has(itemId)) {
                            self._triggerVisibleLoad();
                        }
                        // Newly visible animated tile? Wake the scheduler so
                        // it doesn't have to wait for another tile to advance.
                        if (self._animatedThumbnails && self._animatedThumbnails[itemId]) {
                            self._scheduleAnimTick();
                        }
                    } else {
                        self._visibleItems.delete(itemId);
                    }
                });
            }, {
                rootMargin: '200px', // Pre-load items 200px before they enter viewport
                threshold: 0.01
            });
        }
    };

    /**
     * Trigger loading of visible items
     */
    this._triggerVisibleLoad = function() {
        // Use requestIdleCallback if available for non-blocking loading
        if (typeof requestIdleCallback !== 'undefined') {
            requestIdleCallback(function() {
                self._processLoadQueue();
            }, { timeout: 100 });
        } else {
            setTimeout(function() {
                self._processLoadQueue();
            }, 0);
        }
    };

    /**
     * Process the load queue, prioritizing visible items
     */
    this._processLoadQueue = function() {
        var isMobile = self._isMobileDevice;
        // Staircase the parallel decoder. Starting at the full desktop limit
        // (6) means six wasm decode jobs land at roughly the same time, and
        // the post-decode pipeline (putImageData + composite) all hits the
        // main thread within a few frames - drowning out the entrance fades
        // for the very tiles the user is looking at on first paint.
        //
        // Instead, ramp 2 -> 4 -> 6 over the first ~1.5s. The first viewport
        // gets to paint stably, then we open the floodgates.
        var parallelLimit;
        if (isMobile) {
            parallelLimit = 1;
        } else {
            if (self._loadQueueStartTs == null) self._loadQueueStartTs = performance.now();
            var elapsed = performance.now() - self._loadQueueStartTs;
            if (elapsed < 700) parallelLimit = 2;
            else if (elapsed < 1500) parallelLimit = 4;
            else parallelLimit = 6;
        }
        
        // Count currently loading items
        var currentlyLoading = self._loadingItems.size;
        
        // Fill up to parallel limit
        while (currentlyLoading < parallelLimit && self._pendingItems.size > 0) {
            // First, try to load visible items
            var itemToLoad = null;
            
            // Check visible items first
            for (var visibleId of self._visibleItems) {
                if (self._pendingItems.has(visibleId) && !self._loadingItems.has(visibleId)) {
                    itemToLoad = self._pendingItems.get(visibleId);
                    break;
                }
            }
            
            // If no visible items need loading, load any pending item
            if (!itemToLoad) {
                for (var [pendingId, pendingItem] of self._pendingItems) {
                    if (!self._loadingItems.has(pendingId)) {
                        itemToLoad = pendingItem;
                        break;
                    }
                }
            }
            
            if (!itemToLoad) break;
            
            self._loadingItems.add(itemToLoad.id);
            currentlyLoading++;
            
            (function(item) {
                self._loadAndCacheItem(item, function() {
                    self._loadedCount++;
                    self._loadingItems.delete(item.id);
                    self._pendingItems.delete(item.id);

                    // Drive the combined progress bar (replaces the old
                    // spinner-block-above-the-grid loader).
                    self._statusBumpDone();

                    if (self._pendingItems.size > 0 || self._loadingItems.size > 0) {
                        // Continue loading more
                        self._triggerVisibleLoad();
                    }
                });
            })(itemToLoad);
        }
    };

    /**
     * Load thumbnails for all items
     * Uses IntersectionObserver for viewport-based lazy loading when available
     */
    this._loadThumbnails = function(displayItems) {
        var items = displayItems || self._getDisplayItems();
        
        // Reset state
        self._pendingItems.clear();
        self._visibleItems.clear();
        self._loadingItems.clear();
        
        // Initialize intersection observer
        self._initIntersectionObserver();
        
        // Register all items with intersection observer
        items.forEach(function(item) {
            self._pendingItems.set(item.id, item);
            
            // Observe the DOM element if observer is available
            if (self._intersectionObserver) {
                // NOTE: item.id is already the full element id ("gallery-1", "conf-7", "2", ...).
                // The previous code prefixed "gallery-" again, which produced ids like
                // "gallery-gallery-1" / "gallery-conf-7" that never resolved. As a
                // result IntersectionObserver never observed anything and every tile
                // was treated as "non-visible / load anyway", defeating lazy loading.
                var itemEl = document.getElementById(item.id);
                if (itemEl) {
                    itemEl.setAttribute('data-item-id', item.id);
                    self._intersectionObserver.observe(itemEl);
                } else {
                    // Element not yet in the DOM - fall back to "treat as visible"
                    // for this item only, so it will still be picked up.
                    self._visibleItems.add(item.id);
                }
            } else {
                // No observer - mark all as visible
                self._visibleItems.add(item.id);
            }
        });
        
        // Start loading
        self._triggerVisibleLoad();
    };

    /**
     * Load, cache fileInfo, derive type, and display thumbnail
     * This is the SINGLE point where HEIF files are fetched and parsed
     * 
     * IMPORTANT: This function ensures callback is ALWAYS called, even on errors,
     * to prevent the parallel loading queue from stalling.
     */
    /** Fetch + open a HEIF URL using the modern HEIF.load() flow.
     *  Returns Promise<{reader, fileInfo}>. The caller owns `reader`
     *  and MUST call reader.close() when done — buildFileInfo() has
     *  already walked every closed-reader-invariant query, but the
     *  reader stays open here so the caller can still pull bitstreams
     *  for thumbnail / modal decode. */
    this._openHeifReader = function(url) {
        if (typeof HEIF === 'undefined' || typeof HEIF.load !== 'function') {
            return Promise.reject(new Error('heif.js / heif-raw.js not loaded'));
        }
        return HEIF.load().then(function(heif) {
            return fetch(url).then(function(resp) {
                if (!resp.ok) throw new Error('fetch ' + url + ' -> HTTP ' + resp.status);
                return resp.arrayBuffer();
            }).then(function(buf) {
                var reader = heif.open(new Uint8Array(buf));
                var fileInfo;
                try {
                    fileInfo = buildFileInfo(reader, url);
                } catch (e) {
                    try { reader.close(); } catch (_) {}
                    throw e;
                }
                return { reader: reader, fileInfo: fileInfo };
            });
        });
    };

    this._loadAndCacheItem = function(item, callback) {
        var reader = null;
        var provider = null;

        // Idempotency guard: typed loaders nest try/catch around an async
        // callback, and if both the inner and the outer fail-path fire we
        // would otherwise double-dispose AND double-bump the progress bar.
        var finished = false;
        var safeFinish = function() {
            if (finished) return;
            finished = true;
            try {
                if (provider) provider.dispose();
            } catch (e) { /* ignore cleanup errors */ }
            try {
                if (reader) reader.close();
            } catch (e) { /* ignore cleanup errors */ }
            if (callback) callback();
        };

        self._openHeifReader(item.heifPath).then(function(opened) {
            reader = opened.reader;
            var fileInfo = opened.fileInfo;
            try {
                if (!fileInfo || !fileInfo.success) {
                    self._showItemError(item.id);
                    safeFinish();
                    return;
                }
                
                // Create ImageProvider (NOT cached - disposed after thumbnail load)
                provider = new ImageProvider(reader, item.heifPath, fileInfo);
                
                // Derive type from fileInfo
                var type = provider.deriveItemType(fileInfo);
                
                // Get primary item info
                var primaryInfo = provider.getPrimaryItemInfo(fileInfo);
                
                // For tmap (HDR/gainmap), resolve to base image
                if (type === 'tmap') {
                    primaryInfo = self._resolveTmapToBaseImage(reader, fileInfo, primaryInfo);
                }
                
                // Extract metadata (pass primaryInfo for sequence detection)
                var meta = provider.extractMetadata(fileInfo, primaryInfo.primaryId, primaryInfo);
                
                // As soon as we know the real dimensions, lock the
                // placeholder to the correct aspect ratio so the masonry
                // doesn't reflow when the canvas is inserted later.
                var earlyEl = document.getElementById(item.id);
                if (earlyEl && meta && meta.width && meta.height) {
                    self._applyPlaceholderAspect(earlyEl, meta.width, meta.height, meta.rotation);
                }
                
                // Check if we're on mobile - we'll minimize caching to save memory
                var isMobileDevice = self._isMobileDevice;
                
                // CACHE essential data on the item (but NOT provider/reader for thumbnails)
                // On mobile, only cache minimal data needed for re-opening
                if (isMobileDevice) {
                    // Mobile: cache fileInfo (small metadata object, needed
                    // by type-specific loaders), but mark for re-open reload
                    // so we don't keep provider/reader alive.
                    item.fileInfo = fileInfo;
                    item.type = type;
                    item.primaryInfo = primaryInfo;
                    item.meta = meta;
                    item.needsReload = true;
                } else {
                    item.fileInfo = fileInfo;
                    item.type = type;
                    item.primaryInfo = primaryInfo;
                    item.meta = meta;
                }

                // Codec gate: only attempt decode for codecs we actually
                // ship a wasm decoder for (HEVC via libde265, VVC via vvdec).
                // The previous logic was an explicit AV1/av01 deny-list -
                // any newly-tagged unsupported codec (AVC, JPEG, future
                // formats) would otherwise sneak through and crash the
                // decode pipeline. We allow `Unknown` through because some
                // files genuinely don't carry a usable codec tag and might
                // still be HEVC under the hood; if they're not, the decode
                // will fail naturally and surface as a normal error tile.
                var codec = (meta.codec || '').toString();
                var codecUpper = codec.toUpperCase();
                var SUPPORTED_CODECS = ['HEVC', 'HVC1', 'HEV1', 'VVC', 'VVC1', 'VVI1', 'UNKNOWN', ''];
                if (SUPPORTED_CODECS.indexOf(codecUpper) === -1) {
                    self._showItemError(item.id, true); // true = noDecoder
                    var itemElEarly = document.getElementById(item.id);
                    if (itemElEarly) {
                        self._addTypeBadge(itemElEarly, type);
                        self._addCodecBadge(itemElEarly, codecUpper);
                        self._addSlimBadge(itemElEarly, meta);
                        itemElEarly.dataset.type = type;
                    }
                    safeFinish();
                    return;
                }

                // Update DOM with derived type badge
                var itemEl = document.getElementById(item.id);
                if (itemEl) {
                    self._addTypeBadge(itemEl, type);
                    self._addCodecBadge(itemEl, codecUpper);
                    self._addSlimBadge(itemEl, meta);
                    itemEl.dataset.type = type;
                    itemEl.dataset.contextId = primaryInfo.contextId;
                    itemEl.dataset.primaryId = primaryInfo.primaryId;
                    itemEl.dataset.isSequence = primaryInfo.isSequence;
                    // Flag tiles whose decoded image carries an alpha
                    // channel (either dedicated `alpha` aux item, or any
                    // type with hasAlpha set during extractMetadata).
                    // CSS uses this to swap the tile's `--bg-muted` for
                    // the page background so transparent thumbnails sit
                    // flush with the surrounding canvas instead of
                    // appearing as a darker swatch.
                    if (type === 'alpha' || (meta && meta.hasAlpha)) {
                        itemEl.classList.add('gallery-item-has-alpha');
                    }
                }
                
                // Wrapper for type-specific loaders that ensures callback is always called
                var safeLoaderCallback = function() {
                    safeFinish();
                };
                
                // Display thumbnail based on type
                // animation: animated thumbnail (GIF-like)
                // burst: static thumbnail from metabox (user browses frames in modal)
                try {
                    // Tiered "no embedded thumbnail" policy.
                    //
                    // For STILL images, if the file doesn't ship an embedded
                    // thumbnail (no `thmb` reference) AND the primary image
                    // is huge, we'd otherwise wasm-decode an 8K image just
                    // to fill a ~250px tile.
                    //
                    // Instead, render a "Heavy preview" card and only decode
                    // when the user explicitly opts in. Modal still works
                    // normally - clicking the card opens the full image.
                    // Tiered "no embedded thumbnail" policy.
                    //
                    // Applies to any STATIC item type (still + grid + overlay
                    // + tmap + alpha) whose file ships no `thmb` AND whose
                    // primary image is heavier than HEAVY_PRIMARY_DIMENSION.
                    // Without this, iPhone HEICs (always grids) would always
                    // wasm-decode the full primary just to fill a ~250px tile.
                    //
                    // Animation and burst types are intentionally excluded -
                    // their thumbnail comes from a sequence track / metabox
                    // master, not the primary image.
                    var isHeavyableType = (type === 'still' || type === 'tmap'
                        || type === 'alpha' || type === 'grid' || type === 'overlay'
                        || type === 'iden');
                    var hasEmbeddedThumb = !!(primaryInfo && primaryInfo.thumbId);
                    var primaryW = (meta && meta.width) || 0;
                    var primaryH = (meta && meta.height) || 0;
                    var HEAVY_DIM = self.HEAVY_PRIMARY_DIMENSION;
                    var isHeavyPrimary = (primaryW > HEAVY_DIM || primaryH > HEAVY_DIM);

                    // Grid / overlay refinement.
                    //
                    // The gate above checks the COMPOSITE output dimension,
                    // but the actual decode work for a grid scales with the
                    // TILE dimension and tile count - not the composite size.
                    // A typical Apple HEIC is 3024-8064 px wide, made of 36-49
                    // 512x512 HEVC tiles, each of which decodes in a few ms
                    // on libde265. The composite is just N drawImage() calls.
                    // Forcing the user to click "Decode preview" here is
                    // user-hostile for a workload that takes <500 ms total,
                    // and it broke the previous behaviour where these grids
                    // showed up automatically as a downscaled preview.
                    //
                    // Refinement: for grid/overlay, query the actual tile
                    // dimensions and only gate when the largest tile is
                    // itself heavier than HEAVY_DIM. That preserves the gate
                    // for the pathological case (a "grid" of two 4K halves)
                    // while letting all real-world Apple captures through.
                    var tilesAreLight = false;
                    if (isHeavyPrimary && (type === 'grid' || type === 'overlay')) {
                        try {
                            var info = (type === 'grid')
                                ? provider.getGridItemInfo(fileInfo, primaryInfo.primaryId)
                                : provider.getOverlayItemInfo(fileInfo, primaryInfo.primaryId);
                            // Modern shape: grid → {tileIds, columns, rows, ...},
                            // overlay → {images: [{imageId, hOffset, vOffset}], ...}.
                            var probeIds = null;
                            if (info) {
                                if (info.tileIds && info.tileIds.length > 0) {
                                    probeIds = info.tileIds;
                                } else if (info.images && info.images.length > 0) {
                                    probeIds = info.images.map(function (im) { return im.imageId; });
                                }
                            }
                            if (probeIds && probeIds.length > 0) {
                                // Average tile size from the composite output.
                                var maxTileEdge = 0;
                                if (type === 'grid'
                                    && typeof info.columns === 'number'
                                    && typeof info.rows === 'number'
                                    && info.columns > 0 && info.rows > 0) {
                                    maxTileEdge = Math.max(
                                        Math.ceil(info.outputWidth / info.columns),
                                        Math.ceil(info.outputHeight / info.rows));
                                } else {
                                    // Overlay (or grid without col/row hints):
                                    // probe the first source item's dimensions.
                                    try {
                                        var firstId = probeIds[0];
                                        var w = reader.itemWidth(firstId) || 0;
                                        var h = reader.itemHeight(firstId) || 0;
                                        maxTileEdge = Math.max(w, h);
                                    } catch (probeErr) { /* leave 0, gate will fire */ }
                                }
                                if (maxTileEdge > 0 && maxTileEdge <= HEAVY_DIM) {
                                    tilesAreLight = true;
                                }
                            }
                        } catch (gridProbeErr) {
                            // If we can't probe, fall through and apply the
                            // gate as before. Conservative.
                        }
                    }

                    if (isHeavyableType && !hasEmbeddedThumb && isHeavyPrimary
                        && !tilesAreLight && !item._previewOptedIn) {
                        // Show a deferred-preview card. The card's "Decode
                        // anyway" button sets _previewOptedIn and re-runs
                        // the loader for this single tile.
                        self._showDeferredPreview(item, primaryW, primaryH);
                        safeFinish();
                        return;
                    }

                    if (type === 'animation') {
                        self._loadAnimatedThumbnail(item, provider, safeLoaderCallback);
                    } else if (type === 'burst') {
                        // Burst needs special handling - use metabox contextId, not track
                        self._loadBurstThumbnail(item, provider, safeLoaderCallback);
                    } else if (type === 'grid') {
                        self._loadGridThumbnail(item, provider, safeLoaderCallback);
                    } else if (type === 'overlay') {
                        self._loadOverlayThumbnail(item, provider, safeLoaderCallback);
                    } else {
                        // Still, tmap, alpha, collection - static thumbnail
                        self._loadStaticThumbnail(item, provider, safeLoaderCallback);
                    }
                } catch (loaderError) {
                    console.error('Error in type-specific loader for:', item.heifPath, loaderError);
                    self._showItemError(item.id);
                    safeFinish();
                }
            } catch (e) {
                console.error('Error processing fileInfo for:', item.heifPath, e);
                self._showItemError(item.id);
                safeFinish();
            }
        }).catch(function(e) {
            console.error('Failed to open HEIF for:', item.heifPath, e);
            self._showItemError(item.id);
            safeFinish();
        });
    };

    /**
     * Resolve tmap (gainmap/HDR) to its base image for thumbnail display
     */
    this._resolveTmapToBaseImage = function(reader, fileInfo, primaryInfo) {
        var contextId = primaryInfo.contextId;
        var tmapId = primaryInfo.primaryId;
        var itemInfoMap = fileInfo.rootLevelMetaBoxProperties.itemInformationMap;
        
        // Get dimg (derived-from) references to find source images
        try {
            var dimgRefs = reader.referencedFrom(tmapId, 'dimg');
            if (dimgRefs && dimgRefs.length > 0) {
                // Find the base image (not the gainmap)
                // The base image typically has a codec type, not 'tmap'
                for (var i = 0; i < dimgRefs.length; i++) {
                    var refId = dimgRefs[i];
                    if (itemInfoMap && itemInfoMap[refId]) {
                        var refType = itemInfoMap[refId].type;
                        if (refType !== 'tmap') {
                            return {
                                contextId: contextId,
                                primaryId: refId,
                                thumbId: primaryInfo.thumbId,
                                isSequence: false,
                                isDerived: false
                            };
                        }
                    }
                }
                // Fallback to first dimg reference
                return {
                    contextId: contextId,
                    primaryId: dimgRefs[0],
                    thumbId: primaryInfo.thumbId,
                    isSequence: false,
                    isDerived: false
                };
            }
        } catch (e) {
            // Rate-limit: tmap-heavy galleries (e.g. an HDR-only set) would
            // otherwise spam this once per item AND again on every modal
            // open. Logging the first failure preserves the diagnostic
            // signal without drowning the console.
            if (!self._tmapWarnLogged) {
                self._tmapWarnLogged = true;
                console.warn('Failed to resolve tmap (further tmap-resolve failures suppressed):', e);
            }
        }
        
        // Cannot resolve - return original (will likely fail to decode)
        return primaryInfo;
    };

    /**
     * Load static thumbnail for still images
     * @param {Object} item - Gallery item
     * @param {ImageProvider} provider - Provider instance (will be disposed by caller)
     * @param {Function} callback - Called when done (ALWAYS called, even on error)
     */
    this._loadStaticThumbnail = function(item, provider, callback) {
        try {
            var primaryInfo = item.primaryInfo;
            var contextId = primaryInfo.contextId;
            
            // Use thumbnail if available, otherwise primary image
            var itemToLoad = primaryInfo.thumbId || primaryInfo.primaryId;
            
            // Filter out metadata items
            itemToLoad = provider.filterImageItems(item.fileInfo, itemToLoad);
            
            // If the sequence-derived primaryInfo didn't resolve to a valid
            // metabox item (e.g. bothie/stereo with track context), fall back
            // to the metabox cover image (primaryItemId from fileInfo).
            if (itemToLoad === null && item.fileInfo && item.fileInfo.primaryItemId) {
                var coverId = item.fileInfo.primaryItemId;
                var coverFiltered = provider.filterImageItems(item.fileInfo, coverId);
                if (coverFiltered !== null) {
                    itemToLoad = coverFiltered;
                    contextId = 0;  // metabox context
                }
            }

            if (itemToLoad === null) {
                console.error('No valid item to load for thumbnail:', item.heifPath);
                self._showItemError(item.id);
                if (callback) callback();
                return;
            }

        
            provider.requestImageData(contextId, [itemToLoad], function(result) {
                try {
                    if (result.success && result.frames.length > 0) {
                        self._displayThumbnail(item.id, result.frames[0], item.meta);
                    } else {
                        self._logError('decode-fail', item.heifPath, result.noDecoder ? '(no decoder)' : '');
                        self._showItemError(item.id, result.noDecoder);
                    }
                } catch (displayError) {
                    console.error('Error displaying thumbnail for:', item.heifPath, displayError);
                    self._showItemError(item.id);
                }
                if (callback) callback();
            });
        } catch (e) {
            console.error('Error in _loadStaticThumbnail for:', item.heifPath, e);
            self._showItemError(item.id);
            if (callback) callback();
        }
    };

    /**
     * Load burst thumbnail - uses METABOX contextId and cover/first master image
     * Burst files have both tracks and metabox items, but thumbnails come from metabox
     * @param {Object} item - Gallery item
     * @param {ImageProvider} provider - Provider instance (will be disposed by caller)
     * @param {Function} callback - Called when done (ALWAYS called, even on error)
     */
    this._loadBurstThumbnail = function(item, provider, callback) {
        try {
            var fileInfo = item.fileInfo;
            var rootMeta = fileInfo.rootLevelMetaBoxProperties;
            // MUST use metabox contextId for burst items (not track contextId)
            var contextId = rootMeta ? rootMeta.contextId : 0;

            // Cover image (`primaryItemId`) first, then first master from
            // the snapshot's masterIds. Walking imageFeaturesMap is slower
            // and would need an `isMaster` lookup anyway.
            var itemToLoad = fileInfo.primaryItemId || 0;
            if (!itemToLoad && fileInfo.masterIds && fileInfo.masterIds.length > 0) {
                itemToLoad = fileInfo.masterIds[0];
            }

            if (!itemToLoad) {
                console.error('Burst: No valid item found for thumbnail:', item.heifPath);
                self._showItemError(item.id);
                if (callback) callback();
                return;
            }

            // Prefer the per-master `thmb` reference if the file ships one.
            // Burst files are intentionally many full-resolution masters; if
            // a master has its own embedded thumbnail we should use that
            // instead of decoding the full master at native resolution just
            // to fill a ~250px tile. Mirrors the still/grid/overlay policy.
            if (typeof provider.getThumbnailIdByItemId === 'function') {
                try {
                    var burstThumbId = provider.getThumbnailIdByItemId(fileInfo, contextId, itemToLoad);
                    if (burstThumbId) {
                        itemToLoad = burstThumbId;
                    }
                } catch (e) { /* keep itemToLoad as cover/master fallback */ }
            }

            provider.requestImageData(contextId, [itemToLoad], function(result) {
                try {
                    if (result.success && result.frames.length > 0) {
                        self._displayThumbnail(item.id, result.frames[0], item.meta);
                    } else {
                        self._logError('burst-decode-fail', item.heifPath, result.noDecoder ? '(no decoder)' : '');
                        self._showItemError(item.id, result.noDecoder);
                    }
                } catch (displayError) {
                    console.error('Error displaying burst thumbnail for:', item.heifPath, displayError);
                    self._showItemError(item.id);
                }
                if (callback) callback();
            });
        } catch (e) {
            console.error('Error in _loadBurstThumbnail for:', item.heifPath, e);
            self._showItemError(item.id);
            if (callback) callback();
        }
    };

    /**
     * Load grid image thumbnail - composites tile images into single canvas
     * @param {Object} item - Gallery item
     * @param {ImageProvider} provider - Provider instance (will be disposed by caller)
     * @param {Function} callback - Called when done (ALWAYS called, even on error)
     */
    this._loadGridThumbnail = function(item, provider, callback) {
        try {
            var primaryInfo = item.primaryInfo;

            // Fast path: if the file ships an embedded thumbnail (`thmb`
            // reference) for the primary, decode that single small item
            // instead of decoding every tile and compositing them. This
            // matches the still-image path's behaviour and avoids wasm-
            // decoding an 8K grid just to fill a ~250px tile.
            if (primaryInfo && primaryInfo.thumbId) {
                self._loadStaticThumbnail(item, provider, callback);
                return;
            }

            // Get grid definition
            var gridInfo = provider.getGridItemInfo(item.fileInfo, primaryInfo.primaryId);
            if (!gridInfo || !gridInfo.tileIds || gridInfo.tileIds.length === 0) {
                console.error('Grid: No tile IDs found for', item.heifPath);
                self._showItemError(item.id);
                if (callback) callback();
                return;
            }
            
            var cols = gridInfo.columns;
            var rows = gridInfo.rows;
            var outputWidth = gridInfo.outputWidth;
            var outputHeight = gridInfo.outputHeight;
            
            // Update meta with grid dimensions
            item.meta.width = outputWidth;
            item.meta.height = outputHeight;
            
            // Request all tile images
            provider.requestImageData(primaryInfo.contextId, gridInfo.tileIds, function(result) {
                try {
                    if (!result.success || result.frames.length === 0) {
                        self._logError('grid-decode-fail', item.heifPath, result.noDecoder ? '(no decoder)' : '');
                        self._showItemError(item.id, result.noDecoder);
                        if (callback) callback();
                        return;
                    }
                    
                    // Create composite canvas at output dimensions
                    var compositeCanvas = document.createElement('canvas');
                    compositeCanvas.width = outputWidth;
                    compositeCanvas.height = outputHeight;
                    var ctx = compositeCanvas.getContext('2d');
                    
                    // Fill black background
                    ctx.fillStyle = 'black';
                    ctx.fillRect(0, 0, outputWidth, outputHeight);
                    
                    // Get actual tile dimensions from first decoded tile
                    var actualTileW = result.frames[0].width;
                    var actualTileH = result.frames[0].height;
                    
                    // Reuse a single tile canvas across all tiles. Allocating
                    // a fresh canvas + getContext per tile produces N temp
                    // canvases for the GC to chase. With 12+ tiles this is a
                    // measurable allocator/GC win on lower-end machines.
                    var tileCanvas = document.createElement('canvas');
                    tileCanvas.width = actualTileW;
                    tileCanvas.height = actualTileH;
                    var tileCtx = tileCanvas.getContext('2d');
                    
                    // Composite tiles in grid order
                    // Tiles are placed at their actual dimensions, edge tiles are cropped
                    for (var j = 0; j < rows; j++) {
                        for (var i = 0; i < cols; i++) {
                            var tileIndex = j * cols + i;
                            if (tileIndex < result.frames.length) {
                                var frame = result.frames[tileIndex];
                                
                                // Resize the shared tile canvas only if this
                                // tile differs in size from the cached one.
                                // Setting width/height clears the buffer.
                                if (tileCanvas.width !== frame.width || tileCanvas.height !== frame.height) {
                                    tileCanvas.width = frame.width;
                                    tileCanvas.height = frame.height;
                                }
                                var imageData = new ImageData(frame.canvasFrameData, frame.width, frame.height);
                                tileCtx.putImageData(imageData, 0, 0);
                                
                                // Calculate destination position
                                var destX = i * actualTileW;
                                var destY = j * actualTileH;
                                
                                // Calculate how much of the tile to draw (crop edge tiles)
                                var srcW = Math.min(frame.width, outputWidth - destX);
                                var srcH = Math.min(frame.height, outputHeight - destY);
                                
                                // Draw tile at grid position (crop if it extends beyond output)
                                if (srcW > 0 && srcH > 0) {
                                    ctx.drawImage(tileCanvas, 0, 0, srcW, srcH, destX, destY, srcW, srcH);
                                }
                            }
                        }
                    }
                    // Free the temp canvas eagerly.
                    tileCanvas.width = 0; tileCanvas.height = 0;

                    // Create thumbnail-sized version for display.
                    // Uses the shared _targetThumbDim() so grid/overlay/still
                    // all converge on the same resolution ceiling.
                    var thumbCanvas = document.createElement('canvas');
                    var maxDim = self._targetThumbDim();
                    var scale = Math.min(maxDim / outputWidth, maxDim / outputHeight, 1);
                    thumbCanvas.width = Math.max(1, Math.round(outputWidth * scale));
                    thumbCanvas.height = Math.max(1, Math.round(outputHeight * scale));
                    thumbCanvas.getContext('2d').drawImage(compositeCanvas, 0, 0, thumbCanvas.width, thumbCanvas.height);

                    // Free the full-resolution composite immediately - up to
                    // 48MB for a 4K source - instead of waiting for GC.
                    compositeCanvas.width = 0; compositeCanvas.height = 0;

                    // Hand the canvas straight to the display path - no
                    // pointless getImageData/putImageData round-trip just to
                    // satisfy the legacy frame-data API.
                    self._displayThumbnailFromCanvas(item.id, thumbCanvas, item.meta, {
                        width: thumbCanvas.width,
                        height: thumbCanvas.height,
                        bitDepth: result.frames[0].bitDepth || 8
                    });
                } catch (compositeError) {
                    console.error('Error compositing grid thumbnail for:', item.heifPath, compositeError);
                    self._showItemError(item.id);
                }
                if (callback) callback();
            });
        } catch (e) {
            console.error('Error in _loadGridThumbnail for:', item.heifPath, e);
            self._showItemError(item.id);
            if (callback) callback();
        }
    };

    /**
     * Load overlay image thumbnail - composites overlay images with offsets
     * @param {Object} item - Gallery item
     * @param {ImageProvider} provider - Provider instance (will be disposed by caller)
     * @param {Function} callback - Called when done (ALWAYS called, even on error)
     */
    this._loadOverlayThumbnail = function(item, provider, callback) {
        try {
            var primaryInfo = item.primaryInfo;

            // Fast path: prefer the embedded `thmb` if the file ships one.
            // Same rationale as _loadGridThumbnail.
            if (primaryInfo && primaryInfo.thumbId) {
                self._loadStaticThumbnail(item, provider, callback);
                return;
            }

            // Get overlay definition
            var iovlInfo = provider.getOverlayItemInfo(item.fileInfo, primaryInfo.primaryId);
            if (!iovlInfo || !iovlInfo.images || iovlInfo.images.length === 0) {
                console.error('Overlay: No source IDs found for', item.heifPath);
                self._showItemError(item.id);
                if (callback) callback();
                return;
            }
            
            var outputWidth = iovlInfo.outputWidth;
            var outputHeight = iovlInfo.outputHeight;
            
            // Update meta with overlay dimensions
            item.meta.width = outputWidth;
            item.meta.height = outputHeight;
            
            // Request all source images
            var overlayImageIds = iovlInfo.images.map(function (im) { return im.imageId; });
            provider.requestImageData(primaryInfo.contextId, overlayImageIds, function(result) {
                try {
                    if (!result.success || result.frames.length === 0) {
                        self._logError('overlay-decode-fail', item.heifPath, result.noDecoder ? '(no decoder)' : '');
                        self._showItemError(item.id, result.noDecoder);
                        if (callback) callback();
                        return;
                    }
                    
                    // Create composite canvas at output dimensions
                    var compositeCanvas = document.createElement('canvas');
                    compositeCanvas.width = outputWidth;
                    compositeCanvas.height = outputHeight;
                    var ctx = compositeCanvas.getContext('2d');
                    
                    // Fill with canvas fill value (16-bit sRGB → 8-bit rgba)
                    if (iovlInfo.fillRgba) {
                        var s = 256.0 / 65536;
                        var r = Math.round(iovlInfo.fillRgba[0] * s);
                        var g = Math.round(iovlInfo.fillRgba[1] * s);
                        var b = Math.round(iovlInfo.fillRgba[2] * s);
                        var a = (iovlInfo.fillRgba[3] * s) / 255;
                        ctx.fillStyle = 'rgba(' + r + ',' + g + ',' + b + ',' + a + ')';
                    } else {
                        ctx.fillStyle = 'black';
                    }
                    ctx.fillRect(0, 0, outputWidth, outputHeight);
                    
                    // Reuse a single source canvas across overlay layers,
                    // resizing only when the next layer differs in size.
                    var srcCanvas = document.createElement('canvas');
                    var srcCtx = srcCanvas.getContext('2d');
                    
                    // Composite images at their offsets
                    for (var i = 0; i < result.frames.length && i < iovlInfo.images.length; i++) {
                        var frame = result.frames[i];
                        var layer = iovlInfo.images[i];
                        
                        if (srcCanvas.width !== frame.width || srcCanvas.height !== frame.height) {
                            srcCanvas.width = frame.width;
                            srcCanvas.height = frame.height;
                        }
                        var imageData = new ImageData(frame.canvasFrameData, frame.width, frame.height);
                        srcCtx.putImageData(imageData, 0, 0);
                        
                        // Draw at offset
                        ctx.drawImage(srcCanvas, layer.hOffset, layer.vOffset);
                    }
                    // Free the shared source canvas.
                    srcCanvas.width = 0; srcCanvas.height = 0;

                    // Create thumbnail-sized version for display.
                    // Shared _targetThumbDim() with the still/grid paths.
                    var thumbCanvas = document.createElement('canvas');
                    var maxDim = self._targetThumbDim();
                    var scale = Math.min(maxDim / outputWidth, maxDim / outputHeight, 1);
                    thumbCanvas.width = Math.max(1, Math.round(outputWidth * scale));
                    thumbCanvas.height = Math.max(1, Math.round(outputHeight * scale));
                    thumbCanvas.getContext('2d').drawImage(compositeCanvas, 0, 0, thumbCanvas.width, thumbCanvas.height);

                    // Free the full-resolution composite immediately.
                    compositeCanvas.width = 0; compositeCanvas.height = 0;

                    // Skip the getImageData/putImageData round-trip - the
                    // composite canvas already holds the final pixels.
                    self._displayThumbnailFromCanvas(item.id, thumbCanvas, item.meta, {
                        width: thumbCanvas.width,
                        height: thumbCanvas.height,
                        bitDepth: result.frames[0].bitDepth || 8
                    });
                } catch (compositeError) {
                    console.error('Error compositing overlay thumbnail for:', item.heifPath, compositeError);
                    self._showItemError(item.id);
                }
                if (callback) callback();
            });
        } catch (e) {
            console.error('Error in _loadOverlayThumbnail for:', item.heifPath, e);
            self._showItemError(item.id);
            if (callback) callback();
        }
    };

    /**
     * Load animated thumbnail for sequences/animations
     * Decodes all frames and creates animated canvas
     * @param {Object} item - Gallery item
     * @param {ImageProvider} provider - Provider instance (will be disposed by caller)
     * @param {Function} callback - Called when done (ALWAYS called, even on error)
     */
    this._loadAnimatedThumbnail = function(item, provider, callback) {
        try {
            var fileInfo = item.fileInfo || (provider && provider._fileInfo) || null;
            var contextId = item.primaryInfo.contextId;

            if (!fileInfo) {
                console.warn('_loadAnimatedThumbnail: no fileInfo for', item.heifPath);
                self._loadStaticThumbnail(item, provider, callback);
                return;
            }
            
            // Get frame ids and timing from the per-track snapshot.
            var frameIds = [];
            var fps = 24;  // Default fallback
            var timeScale = 1000;
            
            if (fileInfo.trackProperties && fileInfo.trackProperties.length > 0) {
                for (var i = 0; i < fileInfo.trackProperties.length; i++) {
                    var track = fileInfo.trackProperties[i];
                    if (track.trackFeature && track.trackFeature.isMasterImageSequence) {
                        // Get FPS from track frameRate
                        if (track.frameRate && track.frameRate.den > 0) {
                            fps = track.frameRate.num / track.frameRate.den;
                        }
                        // Get timeScale for duration calculations
                        if (track.timeScale) {
                            timeScale = track.timeScale;
                        }
                        if (track.sampleIds && track.sampleIds.length > 0) {
                            frameIds = track.sampleIds.slice();
                        }
                        break;
                    }
                }
            }
            
            if (frameIds.length === 0) {
                console.warn('No frames found for sequence:', item.heifPath);
                self._loadStaticThumbnail(item, provider, callback);
                return;
            }

            // Progressive decode.
            //
            // Previously we requested ALL frame IDs in one go, then drew
            // anything. For a 60-frame sequence on slower devices that
            // meant the tile sat as a placeholder for several seconds
            // before suddenly popping in.
            //
            // Two-stage approach:
            //   1. Decode + display frame 0 immediately (tile is alive,
            //      shows the right pixels, animation playing on a
            //      single-frame loop = static).
            //   2. Decode the remaining frames in the background; once
            //      they arrive, append them to the running animState so
            //      the rAF scheduler picks them up on its next tick.
            //
            // The loader-queue callback fires only when stage 2 finishes
            // (otherwise the provider gets disposed mid-request), but the
            // user sees the tile populate in stage 1 - exactly the UX win.
            var interval = 1000 / fps;
            var allTimestamps = frameIds.map(function(id, index) {
                return { id: id, t: index * interval };
            });
            var needsDownscale = false;

            var capFrame = function(f) {
                if (!f) return f;
                if (f.width > self.MAX_THUMBNAIL_DIMENSION
                    || f.height > self.MAX_THUMBNAIL_DIMENSION) {
                    needsDownscale = true;
                    return self._downscaleForThumbnail(f);
                }
                return f;
            };

            // Stage 1: just frame 0.
            provider.requestImageData(contextId, [frameIds[0]], function(stage1) {
                var stage1Ok = stage1 && stage1.success && stage1.frames.length > 0;
                if (!stage1Ok) {
                    self._logError('sequence-decode-fail', item.heifPath, stage1 && stage1.noDecoder ? '(no decoder)' : '');
                    self._showItemError(item.id, stage1 && stage1.noDecoder);
                    if (callback) callback();
                    return;
                }

                try {
                    item.meta.frameCount = frameIds.length; // intent, not yet decoded count
                    item.meta.fps = fps;
                    var firstFrame = capFrame(stage1.frames[0]);
                    self._createAnimatedThumbnail(item, [firstFrame],
                        [allTimestamps[0]], fps);
                } catch (animError) {
                    console.error('Error initialising animated thumbnail for:', item.heifPath, animError);
                    self._showItemError(item.id);
                    if (callback) callback();
                    return;
                }

                // Single-frame sequence: nothing to stream.
                if (frameIds.length === 1) {
                    if (callback) callback();
                    return;
                }

                // Stage 2: re-decode the whole sequence.
                //
                // We MUST include the IDR/IRAP at frameIds[0] — HEVC
                // inter-frames reference earlier pictures, and asking
                // libde265 to decode `frameIds.slice(1)` standalone
                // produces 0 frames (no key frame to anchor the GOP),
                // which is what kept thumbnails frozen on frame 0.
                // The duplicated frame 0 cost is one extra decode; we
                // discard the redundant first result below.
                provider.requestImageData(contextId, frameIds, function(stage2) {
                    try {
                        if (stage2 && stage2.success && stage2.frames.length > 1) {
                            var st = self._animatedThumbnails[item.id];
                            // Tile may have been torn down (set switch,
                            // error, etc.) while stage 2 was decoding -
                            // bail silently if so.
                            if (st && st.running) {
                                // Skip stage2.frames[0] (re-decode of the
                                // keyframe we already have on the canvas).
                                for (var i = 1; i < stage2.frames.length; i++) {
                                    st.frames.push(capFrame(stage2.frames[i]));
                                    st.timestamps.push(allTimestamps[i]);
                                }
                                self._scheduleAnimTick();
                            }
                        } else {
                            // Frame 0 already visible; just log and accept
                            // a static-looking tile.
                            self._logError('sequence-decode-fail', item.heifPath,
                                stage2 && stage2.noDecoder ? '(no decoder, partial)' : '(partial)');
                        }
                    } catch (e) {
                        console.error('Error appending streamed frames for:', item.heifPath, e);
                    }
                    if (callback) callback();
                });
            });
        } catch (e) {
            console.error('Error in _loadAnimatedThumbnail for:', item.heifPath, e);
            self._showItemError(item.id);
            if (callback) callback();
        }
    };

    /**
     * Display a fully-rendered canvas as a thumbnail.
     *
     * Used by the grid/overlay/composite paths, which already have a canvas
     * containing the final pixels and don't need _displayThumbnail's
     * frame.canvasFrameData -> ImageData -> putImageData round-trip.
     *
     * If color information is present we copy via drawImage into a colour
     * managed canvas (one GPU blit) instead of going through getImageData
     * + putImageData (two CPU image traversals). For a 12-tile grid composite
     * this saves ~10-20 ms on desktop and considerably more on mobile.
     *
     * On iOS Safari we additionally convert to <img> via toBlob, matching
     * _displayThumbnail's mobile branch - otherwise grid/overlay tiles would
     * still hit Safari's canvas-buffer-discard bug under memory pressure.
     *
     * @param {string} itemId  - Gallery item element id
     * @param {HTMLCanvasElement} srcCanvas - Composite canvas with final pixels
     * @param {Object} meta    - Image metadata (rotation/mirror/color/...)
     * @param {Object} origFrame - Optional - the original (pre-composite) frame
     *                             used by _updateThumbnailMeta to display the
     *                             "decoded WxH -> displayed WxH" hint.
     */
    this._displayThumbnailFromCanvas = function(itemId, srcCanvas, meta, origFrame) {
        var itemEl = document.getElementById(itemId);
        if (!itemEl) return;

        // Race guards (parallel loaders / set switches). Also check for
        // .gallery-thumbnail which is what the mobile <img> path inserts.
        if (itemEl.querySelector('.gallery-item-error')) return;
        if (itemEl.querySelector('canvas, img.gallery-thumbnail')) return;

        var placeholder = itemEl.querySelector('.gallery-item-placeholder');
        if (placeholder) placeholder.remove();

        var displayCanvas = srcCanvas;
        var colorInfo = meta && meta.color ? meta.color : null;

        // If we have colour info, the destination canvas needs the right
        // colorSpace context attribute, which means we have to render into a
        // fresh canvas rather than displaying srcCanvas directly. Copying via
        // drawImage is still cheap (single GPU blit, no CPU traversal).
        if (colorInfo) {
            var managed = HEIFUtils.createColorManagedCanvas(
                srcCanvas.width, srcCanvas.height, colorInfo);
            managed.ctx.drawImage(srcCanvas, 0, 0);
            displayCanvas = managed.canvas;
        }

        // Mobile branch: convert to <img> before insertion so iOS Safari's
        // canvas-buffer-discard bug doesn't blank out grid/overlay tiles.
        var elementToInsert = self._isMobileDevice
            ? self._canvasToMobileImg(displayCanvas)
            : displayCanvas;

        self._insertThumbnailElement(itemEl, elementToInsert, meta);

        // displayFrame fallback for the meta strip - we've discarded the
        // pre-composite raw pixels by this point, so just report the on-screen
        // canvas dimensions.
        var displayFrame = {
            width: displayCanvas.width,
            height: displayCanvas.height,
            bitDepth: (origFrame && origFrame.bitDepth) || 8
        };
        self._updateThumbnailMeta(itemId, origFrame || displayFrame, meta, displayFrame);
    };

    /**
     * Shared insertion + fade-in tail used by both _displayThumbnail and
     * _displayThumbnailFromCanvas. Applies rotation/mirror transforms,
     * inserts the element into the tile, and queues a paced fade-in.
     */
    this._insertThumbnailElement = function(itemEl, elementToInsert, meta) {
        // Set alt text on <img> elements from the item title for accessibility.
        var titleEl = itemEl.querySelector('.gallery-item-title');
        var altText = titleEl ? titleEl.textContent : '';
        if (elementToInsert && elementToInsert.tagName === 'IMG') {
            elementToInsert.alt = altText;
        }

        // Apply rotation/mirror via CSS (CLAP/ISCL are baked into the bitmap
        // already by the thumbnail downscale path).
        if (meta) {
            if (meta.rotation) {
                self._applyRotation(elementToInsert, meta.rotation, true);
                elementToInsert = self._wrapRotatedCanvas(elementToInsert, meta.rotation);
            }
            if (meta.mirror) {
                self._applyMirror(elementToInsert, meta.mirror);
            }
        }

        var badge = itemEl.querySelector('.gallery-item-badge');
        var info = itemEl.querySelector('.gallery-item-info');
        if (badge) {
            itemEl.insertBefore(elementToInsert, badge);
        } else if (info) {
            itemEl.insertBefore(elementToInsert, info);
        } else {
            itemEl.appendChild(elementToInsert);
        }

        // If we just inserted a rotation wrapper, refit its inner canvas
        // so the cover-scale matches the masonry-rounded tile height.
        // _applyPlaceholderAspect has already set --gi-span by this point,
        // so getBoundingClientRect on the wrapper returns the final size.
        if (elementToInsert && elementToInsert.classList
            && elementToInsert.classList.contains('gallery-rotation-wrapper')) {
            self._fitAdaptiveRotationWrapper(elementToInsert);
        }

        // Paced opacity-only fade. Fade the inner canvas/img if we ended up
        // inserting a rotation wrapper <div>.
        var fadeTarget = elementToInsert;
        if (fadeTarget && fadeTarget.tagName === 'DIV') {
            var inner = fadeTarget.querySelector('canvas, img');
            if (inner) fadeTarget = inner;
        }
        self._scheduleFadeIn(fadeTarget);
    };

    /**
     * Convert a canvas to an <img> for the mobile insertion path.
     *
     * On iOS Safari we use <img> rather than <canvas> because Safari
     * aggressively discards canvas backing stores under memory pressure
     * (the gallery would visibly blank out tiles after a few seconds).
     *
     * Encoding the bitmap is unavoidable here, but we use the async
     * toBlob/Object URL path instead of synchronous toDataURL. toDataURL is
     * base64+PNG-encode on the main thread (50-200ms per image on mobile)
     * and was a major contributor to the laggy feel: every finished decode
     * would freeze the entrance animations of every other tile for the
     * duration of the encode.
     *
     * The canvas backing store is zeroed as soon as encoding completes.
     */
    this._canvasToMobileImg = function(srcCanvas) {
        var img = document.createElement('img');
        img.className = 'gallery-thumbnail';
        img.width = srcCanvas.width;
        img.height = srcCanvas.height;
        var done = function(blobUrl) {
            if (blobUrl) {
                img.onload = function() {
                    try { URL.revokeObjectURL(blobUrl); } catch (e) {}
                };
                img.src = blobUrl;
            }
            // Free the canvas backing store regardless.
            try { srcCanvas.width = 0; srcCanvas.height = 0; } catch (e) {}
        };
        if (typeof srcCanvas.toBlob === 'function') {
            srcCanvas.toBlob(function(blob) {
                done(blob ? URL.createObjectURL(blob) : null);
            }, 'image/png');
        } else {
            // Fallback for very old browsers (shouldn't be reached on any
            // iOS we care about, but cheap to keep).
            try { img.src = srcCanvas.toDataURL('image/png'); } catch (e) {}
            done(null);
        }
        return img;
    };

    /**
     * Display static thumbnail on canvas
     */
    this._displayThumbnail = function(itemId, frame, meta) {
        var itemEl = document.getElementById(itemId);
        if (!itemEl) return;

        // Check if error is already shown (parallel load race - don't overwrite error)
        var existingError = itemEl.querySelector('.gallery-item-error');
        if (existingError) return;

        // Check if a thumbnail already landed (parallel load race). On mobile
        // the inserted element is <img class="gallery-thumbnail">, not a
        // <canvas>, so we have to check for both.
        var existing = itemEl.querySelector('canvas, img.gallery-thumbnail');
        if (existing) return;
        
        var placeholder = itemEl.querySelector('.gallery-item-placeholder');
        if (placeholder) placeholder.remove();
        
        // Downscale large images to prevent gallery bloat
        var displayFrame = self._downscaleForThumbnail(frame);
        
        // On mobile, use <img> elements instead of <canvas> to prevent iOS Safari
        // from clearing canvas buffers when memory is tight
        var isMobile = self._isMobileDevice;
        
        // Create color-managed canvas if color info is available
        var colorInfo = meta && meta.color ? meta.color : null;
        var canvasResult = HEIFUtils.createColorManagedCanvas(
            displayFrame.width, 
            displayFrame.height, 
            colorInfo
        );
        var canvas = canvasResult.canvas;
        var ctx = canvasResult.ctx;
        
        // Ensure canvasFrameData is Uint8ClampedArray for ImageData constructor
        var frameData = displayFrame.canvasFrameData;
        if (!(frameData instanceof Uint8ClampedArray)) {
            frameData = new Uint8ClampedArray(frameData);
        }
        
        var imageData = new ImageData(frameData, displayFrame.width, displayFrame.height);
        ctx.putImageData(imageData, 0, 0);
        
        // Determine what to insert
        var elementToInsert;

        if (isMobile) {
            elementToInsert = self._canvasToMobileImg(canvas);
        } else {
            elementToInsert = canvas;
        }

        self._insertThumbnailElement(itemEl, elementToInsert, meta);
        self._updateThumbnailMeta(itemId, frame, meta, displayFrame);
    };

    /**
     * Create animated thumbnail canvas
     */
    this._createAnimatedThumbnail = function(item, frames, timestamps, fps) {
        var itemEl = document.getElementById(item.id);
        if (!itemEl) return;
        
        // Check if error is already shown (parallel load race - don't overwrite error)
        var existingError = itemEl.querySelector('.gallery-item-error');
        if (existingError) return;

        // Race guard: matches _displayThumbnail's, covering both desktop
        // <canvas> and mobile <img class="gallery-thumbnail">.
        var existing = itemEl.querySelector('canvas, img.gallery-thumbnail');
        if (existing) return;
        
        var placeholder = itemEl.querySelector('.gallery-item-placeholder');
        if (placeholder) placeholder.remove();
        
        var firstFrame = frames[0];

        // Create color-managed canvas if color info is available
        var colorInfo = item.meta && item.meta.color ? item.meta.color : null;
        var canvasResult = HEIFUtils.createColorManagedCanvas(
            firstFrame.width,
            firstFrame.height,
            colorInfo
        );
        var canvas = canvasResult.canvas;
        var ctx = canvasResult.ctx;

        // Use the shared insertion helper so animated tiles get the same
        // rotation/mirror handling, anchor logic and paced fade-in as static
        // tiles. Previously this path open-coded the insert and skipped the
        // mirror branch entirely - a real bug for sideways animated tracks.
        self._insertThumbnailElement(itemEl, canvas, item.meta);

        // Store animation state
        var animState = {
            frames: frames,
            timestamps: timestamps,
            currentIndex: 0,
            canvas: canvas,
            ctx: ctx,
            running: true,
            fps: fps
        };

        // Dedupe: if a previous animation state exists for this id (e.g. a
        // partial reset followed by re-observation), stop it before
        // overwriting. Without this the old state's `running=false` keeps
        // it dormant but the entry would leak frames refs (potentially
        // 30 x full-resolution buffers per stale entry) until the next
        // _resetLoadState.
        var prior = self._animatedThumbnails[item.id];
        if (prior) {
            prior.running = false;
            prior.frames = null;
            prior.canvas = null;
            prior.ctx = null;
        }
        self._animatedThumbnails[item.id] = animState;

        // Draw first frame
        var imageData = new ImageData(firstFrame.canvasFrameData, firstFrame.width, firstFrame.height);
        ctx.putImageData(imageData, 0, 0);

        // Hand the new state over to the shared rAF scheduler. It will
        // automatically pause/resume based on tab visibility, viewport
        // visibility and the global concurrency cap. (_insertThumbnailElement
        // already scheduled the fade-in.)
        self._scheduleAnimTick();
        self._updateThumbnailMeta(item.id, firstFrame, item.meta);
    };

    /**
     * Legacy per-tile animation entry point.
     *
     * The actual frame-stepping now happens inside the shared rAF scheduler
     * (`_animTick` above). This stub is kept so any code path that still
     * calls it just nudges the scheduler instead of starting a parallel
     * setTimeout chain.
     */
    this._animateFrame = function(/* itemId */) {
        self._scheduleAnimTick();
    };

    /**
     * Update thumbnail metadata display
     * @param {string} itemId - Gallery item element ID
     * @param {Object} frame - Original decoded frame (from HEIF)
     * @param {Object} meta - Image metadata (width, height from fileInfo)
     * @param {Object} displayFrame - Optional downscaled frame for display
     */
    this._updateThumbnailMeta = function(itemId, frame, meta, displayFrame) {
        var itemEl = document.getElementById(itemId);
        if (!itemEl) return;
        
        // Use frame's bitDepth if available, otherwise use meta
        var bitDepth = (frame.bitDepth && frame.bitDepth > 0) ? frame.bitDepth : meta.bitDepth;
        
        var dimsEl = itemEl.querySelector('.gallery-item-dims');
        if (dimsEl) {
            // Show original image dimensions
            var html = '<span class="dim-original">' + meta.width + '×' + meta.height + '</span>';
            
            // Show thumbnail dimensions if different from original
            if (frame.width !== meta.width || frame.height !== meta.height) {
                html += '<span class="dim-arrow">→</span>';
                html += '<span class="dim-thumb">' + frame.width + '×' + frame.height + '</span>';
            }
            
            // If additionally downscaled for display, show that too
            if (displayFrame && (displayFrame.width !== frame.width || displayFrame.height !== frame.height)) {
                html += '<span class="dim-arrow">→</span>';
                html += '<span class="dim-display">' + displayFrame.width + '×' + displayFrame.height + '</span>';
            }
            
            dimsEl.innerHTML = html;
        }
        
        // Show bit depth if not 8
        var metaEl = itemEl.querySelector('.gallery-item-meta');
        if (metaEl && bitDepth && bitDepth !== 8) {
            var existing = metaEl.querySelector('.gallery-item-bitdepth');
            if (!existing) {
                var bitEl = document.createElement('span');
                bitEl.className = 'gallery-item-bitdepth';
                bitEl.textContent = bitDepth + 'bit';
                metaEl.appendChild(bitEl);
            }
        }
    };

    /**
     * Render the deferred-preview card for a tile that has no embedded
     * thumbnail and a heavy primary image.
     *
     * Decoding (e.g.) an 8K still through the wasm decoder for a 250px tile
     * is expensive and is the main reason certain conformance files made
     * the gallery feel slow. This card lets the user opt in to that work
     * explicitly, and adds a "Heavy preview" type badge so the reason is
     * visible.
     *
     * The user can still open the file in the modal at any time; the card
     * itself is also clickable for that.
     */
    this._showDeferredPreview = function(item, w, h) {
        var itemEl = document.getElementById(item.id);
        if (!itemEl) return;

        // Don't overwrite an existing canvas / error / deferred card.
        if (itemEl.querySelector('canvas')) return;
        if (itemEl.querySelector('.gallery-item-error')) return;
        if (itemEl.querySelector('.gallery-item-deferred')) return;

        // Reserve a sensible visual size on the masonry. Use a fixed
        // landscape ratio (4:3) for the deferred card; the real aspect of
        // the still image is preserved later if the user opts in.
        self._applyPlaceholderAspect(itemEl, 4, 3, 0);

        var card = document.createElement('div');
        card.className = 'gallery-item-deferred';
        // Decorative card; tile root already exposes role/aria for AT.

        // Document-style icon (inline SVG so we don't ship another file).
        var icon = document.createElement('div');
        icon.className = 'gallery-item-deferred-icon';
        icon.innerHTML = '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor"' +
            ' stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"' +
            ' xmlns="http://www.w3.org/2000/svg" width="100%" height="100%">' +
            '<path d="M14 3H7a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2V8z"/>' +
            '<path d="M14 3v5h5"/>' +
            '<path d="M9 13h6M9 17h4"/>' +
            '</svg>';
        card.appendChild(icon);

        var label = document.createElement('div');
        label.className = 'gallery-item-deferred-label';
        label.textContent = 'No embedded thumbnail';
        card.appendChild(label);

        var hint = document.createElement('div');
        hint.className = 'gallery-item-deferred-hint';
        hint.textContent = 'Source is ' + w + '\u00d7' + h + ' \u2014 decoding takes a moment.';
        card.appendChild(hint);

        var action = document.createElement('button');
        action.type = 'button';
        action.className = 'gallery-item-deferred-action';
        action.textContent = 'Decode preview';
        action.addEventListener('click', function(ev) {
            ev.stopPropagation();
            // Mark that the user has explicitly asked for the preview, then
            // re-trigger this single tile through the normal load pipeline.
            item._previewOptedIn = true;
            // Remove the card and reset to a plain placeholder so the
            // skeleton sweep gives a "loading" cue.
            card.remove();
            var ph = document.createElement('div');
            ph.className = 'gallery-item-placeholder';
            ph.setAttribute('aria-hidden', 'true');
            itemEl.insertBefore(ph, itemEl.firstChild);
            // Re-run only this item; bypass the visibility queue.
            self._pendingItems.set(item.id, item);
            self._visibleItems.add(item.id);
            self._triggerVisibleLoad();
        });
        card.appendChild(action);

        itemEl.appendChild(card);

        // The badge slot is already added separately by _addTypeBadge if
        // there's a derived type. We additionally surface the reason as a
        // small badge if none was added.
        if (!itemEl.querySelector('.gallery-item-badge')) {
            self._addTypeBadge(itemEl, 'still');
        }
    };

    /**
     * Show error state for an item
     * @param {string} itemId - The item element ID
     * @param {boolean} noDecoder - True if the error is due to missing decoder
     */
    this._showItemError = function(itemId, noDecoder) {
        var itemEl = document.getElementById(itemId);
        if (!itemEl) return;
        
        // Check if error is already shown
        var existingError = itemEl.querySelector('.gallery-item-error');
        if (existingError) return;

        // Remove any existing thumbnail (from parallel load race). On mobile
        // this is <img class="gallery-thumbnail">, on desktop a <canvas>.
        var existing = itemEl.querySelector('canvas, img.gallery-thumbnail');
        if (existing) existing.remove();
        
        var placeholder = itemEl.querySelector('.gallery-item-placeholder');
        var errorEl;
        
        if (placeholder) {
            // Transform placeholder into error
            placeholder.className = 'gallery-item-error';
            errorEl = placeholder;
        } else {
            // Create new error element (placeholder was removed)
            errorEl = document.createElement('div');
            errorEl.className = 'gallery-item-error';
            // Insert at beginning (before badge/info)
            itemEl.insertBefore(errorEl, itemEl.firstChild);
        }
        
        errorEl.style.cssText =
            'display: flex; flex-direction: column; align-items: center; justify-content: center; ' +
            // Fill the masonry-allocated tile completely. The tile's height
            // is set by --gi-span on .gallery-item (rows * 8px). With the
            // earlier `min-height: 120px` the error box only occupied the
            // top ~120px of a taller tile, which made the centered text
            // visually float above the tile center. position:absolute +
            // inset:0 nails the box to the (already position:relative)
            // tile so flex centering is centered in what the user sees.
            'position: absolute; inset: 0; padding-bottom: 0; min-height: 120px;';
        
        if (noDecoder) {
            var label1 = document.createElement('span');
            label1.style.cssText = 'font-size: 13px; opacity: 0.55; letter-spacing: 0.5px; font-weight: 600;';
            label1.textContent = 'NO DECODER';
            var label2 = document.createElement('span');
            label2.style.cssText = 'margin-top: 4px; font-size: 10px; opacity: 0.7;';
            label2.textContent = 'Codec not supported';
            errorEl.appendChild(label1);
            errorEl.appendChild(label2);
        } else {
            var label1 = document.createElement('span');
            label1.style.cssText = 'font-size: 13px; opacity: 0.55; letter-spacing: 0.5px; font-weight: 600;';
            label1.textContent = 'LOAD FAILED';
            var label2 = document.createElement('span');
            label2.style.cssText = 'margin-top: 4px; font-size: 10px; opacity: 0.7;';
            label2.textContent = 'Could not decode';
            errorEl.appendChild(label1);
            errorEl.appendChild(label2);
        }
    };

    // =========================================================================
    // MODAL
    // =========================================================================

    /**
     * Setup modal event handlers (a11y-aware).
     *
     * Keyboard contract while the modal is open:
     *   Esc          - close
     *   Left  / 'h'  - previous item in the current display order
     *   Right / 'l'  - next item
     *   Tab          - focus cycles WITHIN modal-focusable elements
     *                  (focus trap; previously Tab leaked back into the
     *                  page behind the dialog, which AT users couldn't
     *                  recover from without a mouse).
     */
    this._setupModal = function() {
        var backdrop = document.getElementById('gallery-modal-backdrop');
        var quit = document.getElementById('gallery-modal-quit');

        if (backdrop) {
            backdrop.addEventListener('click', function(e) {
                if (e.target === backdrop) {
                    self._closeModal();
                }
            });
        }

        if (quit) {
            quit.addEventListener('click', function() {
                self._closeModal();
            });
            // Native <button> already accepts Enter/Space, no extra handler.
        }

        document.addEventListener('keydown', function(e) {
            // Bail unless the modal is currently open.
            var modal = document.getElementById('gallery-modal-window');
            if (!modal || !modal.classList.contains('visible')) return;

            if (e.key === 'Escape') {
                e.preventDefault();
                self._closeModal();
                return;
            }

            // Arrow / vim-key navigation between gallery items.
            if (e.key === 'ArrowLeft' || e.key === 'ArrowRight'
                || e.key === 'h' || e.key === 'l') {
                // Don't hijack arrows that target a typing surface.
                var t = e.target;
                if (t && (t.tagName === 'INPUT' || t.tagName === 'TEXTAREA' || t.isContentEditable)) return;
                e.preventDefault();
                var dir = (e.key === 'ArrowRight' || e.key === 'l') ? 1 : -1;
                self._openSiblingItem(dir);
                return;
            }

            // Tab focus trap.
            if (e.key === 'Tab') {
                var focusables = self._getModalFocusables();
                if (!focusables.length) {
                    // Nothing focusable yet (modal still loading) - keep
                    // focus on the modal window itself.
                    e.preventDefault();
                    modal.focus();
                    return;
                }
                var first = focusables[0];
                var last = focusables[focusables.length - 1];
                var active = document.activeElement;
                if (e.shiftKey && active === first) {
                    e.preventDefault();
                    last.focus();
                } else if (!e.shiftKey && active === last) {
                    e.preventDefault();
                    first.focus();
                } else if (active && !modal.contains(active)) {
                    // Focus escaped (e.g. clicked outside) - pull it back.
                    e.preventDefault();
                    first.focus();
                }
            }
        });
    };

    /**
     * Collect tabbable elements inside the modal for the focus trap.
     * Order matches DOM order (which is the visible reading order).
     */
    this._getModalFocusables = function() {
        var modal = document.getElementById('gallery-modal-window');
        if (!modal) return [];
        var sel = 'button, [href], input, select, textarea, '
                + '[tabindex]:not([tabindex="-1"])';
        var nodes = modal.querySelectorAll(sel);
        var out = [];
        for (var i = 0; i < nodes.length; i++) {
            var n = nodes[i];
            // Skip hidden / disabled.
            if (n.disabled) continue;
            if (n.getAttribute('aria-hidden') === 'true') continue;
            // offsetParent === null catches display:none and detached nodes.
            if (n.offsetParent === null && n !== modal) continue;
            out.push(n);
        }
        return out;
    };

    /**
     * Open the previous (dir=-1) or next (dir=+1) item in display order.
     * Wraps at the ends. No-op if there's no current item or only one item.
     */
    this._openSiblingItem = function(dir) {
        var items = self._getDisplayItems();
        if (!items || items.length < 2) return;
        var currentId = self._modalCurrentItem && self._modalCurrentItem.id;
        var idx = -1;
        for (var i = 0; i < items.length; i++) {
            if (items[i].id === currentId) { idx = i; break; }
        }
        if (idx === -1) return;
        var n = items.length;
        var nextIdx = ((idx + dir) % n + n) % n;
        // Tear down the previous item's modal-scoped resources BEFORE
        // opening the next one. Without this the old reader/provider,
        // sequence animation timer, audio element and burst interval all
        // leak (and audio keeps playing) until the user manually closes
        // the modal.
        self._teardownActiveModalSession();
        self._openModal(items[nextIdx]);
    };

    /**
     * Run and clear queued modal cleanups, dispose the active reader/
     * provider, and stop any sequence/burst timers tied to the previous
     * modal session. Used both on full _closeModal and on arrow-key
     * navigation between items (where the modal stays open but the
     * underlying file changes). Safe to call when nothing is active.
     */
    this._teardownActiveModalSession = function() {
        // Bump session id BEFORE running cleanups so any cleanup that
        // (synchronously or async) re-enters _addModalCleanup is treated
        // as belonging to the now-defunct session.
        self._modalSessionId++;
        // Run user-registered cleanups first (audio pause, etc.).
        if (self._modalCleanups && self._modalCleanups.length) {
            var cbs = self._modalCleanups;
            self._modalCleanups = [];
            for (var i = 0; i < cbs.length; i++) {
                try { cbs[i](); } catch (e) { /* ignore cleanup errors */ }
            }
        }
        if (self._sequenceCleanup) {
            try { self._sequenceCleanup(); } catch (e) {}
            self._sequenceCleanup = null;
        }
        // Burst animation timers live on the gallery item objects.
        if (self._galleryItems) {
            for (var j = 0; j < self._galleryItems.length; j++) {
                var gi = self._galleryItems[j];
                if (gi && gi.burstState && gi.burstState.animationTimer) {
                    clearInterval(gi.burstState.animationTimer);
                    gi.burstState = null;
                }
            }
        }
        if (self._activeProvider) {
            try { self._activeProvider.dispose(); } catch (e) {}
            self._activeProvider = null;
        }
        if (self._activeReader) {
            try {
                if (typeof self._activeReader.close === 'function') {
                    self._activeReader.close();
                }
            } catch (e) {}
            self._activeReader = null;
        }
    };

    /**
     * Open modal for an item - USES CACHED DATA
     */
    this._openModal = function(item) {
        var backdrop = document.getElementById('gallery-modal-backdrop');
        var modal = document.getElementById('gallery-modal-window');
        var content = document.getElementById('gallery-modal-content');

        // Record what triggered this open so we can return focus on close.
        // Only when there's no existing opener (don't overwrite during
        // arrow-key navigation - we want to land back on the FIRST tile
        // the user invoked, not the last one they navigated to).
        if (!self._modalCurrentItem) {
            self._modalLastOpener = document.activeElement || null;
        }
        self._modalCurrentItem = item;

        // Set original source for context menu download
        content.dataset.originalSrc = item.heifPath;
        
        // Clear previous content and remove burst-view class
        content.replaceChildren();
        var loadingDiv = document.createElement('div');
        loadingDiv.className = 'gallery-modal-loading';
        loadingDiv.id = 'gallery-modal-loading';
        var spinner = document.createElement('div');
        spinner.className = 'gallery-spinner';
        loadingDiv.appendChild(spinner);
        var loadingText = document.createElement('span');
        loadingText.textContent = 'Decoding full image...';
        loadingDiv.appendChild(loadingText);
        content.appendChild(loadingDiv);
        content.classList.remove('burst-view');
        
        // Remove old info panels
        var oldPanel = document.getElementById('gallery-modal-info-panel');
        if (oldPanel) oldPanel.remove();
        var oldFullInfo = document.getElementById('gallery-modal-full-info');
        if (oldFullInfo) oldFullInfo.remove();
        
        backdrop.classList.add('visible');
        modal.classList.add('visible');
        document.body.style.overflow = 'hidden';
        
        // Defer expensive backdrop-filter blurs until the modal's scale-in
        // transition finishes. Blurring a moving/scaling element is one of
        // the most expensive composite operations the browser can do, and
        // it's exactly what produced the small hitch when the modal opens.
        // The .blur-ready class on the modal window gates `backdrop-filter`
        // in CSS so blur kicks in only after the transform settles.
        modal.classList.remove('blur-ready');
        // Cancel any leftover fallback timer from a previous open. Without
        // this the prior session's 600ms fallback would still fire and
        // (harmlessly) re-add blur-ready to the new session.
        if (self._modalBlurFallbackTimer) {
            clearTimeout(self._modalBlurFallbackTimer);
            self._modalBlurFallbackTimer = null;
        }
        var onScaleEnd = function(e) {
            if (e.target !== content) return;
            if (e.propertyName !== 'transform') return;
            modal.classList.add('blur-ready');
            content.removeEventListener('transitionend', onScaleEnd);
            // The fallback is no longer needed - we hit the real event.
            if (self._modalBlurFallbackTimer) {
                clearTimeout(self._modalBlurFallbackTimer);
                self._modalBlurFallbackTimer = null;
            }
            // Move focus into the dialog. Close button is the safest
            // landing spot - it always exists and Esc/Enter on it both
            // close, matching user expectation for a dialog.
            var quitBtn = document.getElementById('gallery-modal-quit');
            if (quitBtn && typeof quitBtn.focus === 'function') {
                try { quitBtn.focus({ preventScroll: true }); } catch (_) { quitBtn.focus(); }
            } else if (modal && typeof modal.focus === 'function') {
                try { modal.focus({ preventScroll: true }); } catch (_) { modal.focus(); }
            }
        };
        content.addEventListener('transitionend', onScaleEnd);
        // Safety fallback: if the transition is interrupted/not fired,
        // enable blur after a generous timeout so it never gets stuck off.
        self._modalBlurFallbackTimer = setTimeout(function() {
            self._modalBlurFallbackTimer = null;
            if (modal && !modal.classList.contains('blur-ready')) {
                modal.classList.add('blur-ready');
                content.removeEventListener('transitionend', onScaleEnd);
            }
        }, 600);
        
        // Check if we have cached fileInfo (we should from thumbnail loading)
        if (!item.fileInfo) {
            // Data not cached (shouldn't happen normally), load it
            self._loadAndOpenModal(item, content);
            return;
        }
        
        // Create fresh provider for modal display
        var sessionId = self._modalSessionId;
        self._openHeifReader(item.heifPath).then(function(opened) {
            // Bail if the user navigated/closed before the open resolved -
            // otherwise we'd allocate a provider against a torn-down
            // reader and dispatch a display fn into the wrong session's
            // content element.
            if (sessionId !== self._modalSessionId) {
                try { opened.reader.close(); } catch (_) {}
                return;
            }
            var reader = opened.reader;
            var fileInfo = opened.fileInfo;
            self._activeReader = reader;

            if (!fileInfo || !fileInfo.success) {
                self._showModalError(content, 'HEIF reader could not parse this file');
                return;
            }

            var provider = new ImageProvider(reader, item.heifPath, fileInfo);
            self._activeProvider = provider;
            
            // For tmap, resolve to base image
            var primaryInfo = item.primaryInfo;
            if (item.type === 'tmap') {
                primaryInfo = self._resolveTmapToBaseImage(reader, fileInfo, primaryInfo);
            }
            
            // Dispatch based on type
            var type = item.type || 'still';
            
            switch (type) {
                case 'animation':
                    self._displayAnimationModal(item, content, provider);
                    break;
                case 'burst':
                    self._displayBurstModal(item, content, provider);
                    break;
                case 'grid':
                    self._displayGridModal(item, content, provider, primaryInfo);
                    break;
                case 'overlay':
                    self._displayOverlayModal(item, content, provider, primaryInfo);
                    break;
                case 'stereo':
                    self._displayStereoModal(item, content, provider, primaryInfo);
                    break;
                case 'bothie':
                    self._displayBothieModal(item, content, provider, primaryInfo);
                    break;
                case 'collection':
                    self._displayCollectionModal(item, content, provider, primaryInfo);
                    break;
                // New entity group types
                case 'panorama':
                    self._displayPanoramaModal(item, content, provider, primaryInfo);
                    break;
                case 'slideshow':
                    self._displaySlideshowModal(item, content, provider, primaryInfo);
                    break;
                case 'progressive':
                    self._displayProgressiveModal(item, content, provider, primaryInfo);
                    break;
                case 'audio-image':
                    self._displayAudioImageModal(item, content, provider, primaryInfo);
                    break;
                case 'bracket-exposure':
                case 'bracket-wb':
                case 'bracket-focus':
                case 'bracket-flash':
                case 'bracket-dof':
                    self._displayBracketModal(item, content, provider, primaryInfo);
                    break;
                case 'album':
                    self._displayAlbumFavoritesModal(item, content, provider, primaryInfo, 'albc', 'Album');
                    break;
                case 'favorites':
                    self._displayAlbumFavoritesModal(item, content, provider, primaryInfo, 'favc', 'Favorites');
                    break;
                case 'altr':
                    self._displayAltrModal(item, content, provider, primaryInfo);
                    break;
                case 'iden':
                    self._displayIdenModal(item, content, provider, primaryInfo);
                    break;
                default:
                    // still, tmap, alpha display as still image
                    self._displayStillModal(item, content, provider, primaryInfo);
            }
        }).catch(function(e) {
            // Anything thrown after `self._activeReader = reader` (e.g. a
            // synchronous display fn) lands here with the reader still
            // stashed on the gallery instance. Teardown closes it; the
            // helper is a no-op when nothing is active.
            console.error('Failed to open HEIF for modal:', item.heifPath, e);
            self._showModalError(content, 'Could not open file \u2014 ' + (e.message || 'unexpected error'));
            self._teardownActiveModalSession();
        });
    };

    /**
     * Fallback: load data and open modal (if cache missed)
     */
    this._loadAndOpenModal = function(item, content) {
        var sessionId = self._modalSessionId;
        self._openHeifReader(item.heifPath).then(function(opened) {
            if (sessionId !== self._modalSessionId) {
                try { opened.reader.close(); } catch (_) {}
                return;
            }
            var reader = opened.reader;
            var fileInfo = opened.fileInfo;
            self._activeReader = reader;

            if (!fileInfo || !fileInfo.success) {
                self._showModalError(content, 'HEIF reader could not parse this file');
                return;
            }

            var provider = new ImageProvider(reader, item.heifPath, fileInfo);
            self._activeProvider = provider;
            
            // Cache fileInfo and derived data for future use
            item.fileInfo = fileInfo;
            item.type = provider.deriveItemType(fileInfo);
            item.primaryInfo = provider.getPrimaryItemInfo(fileInfo);
            item.meta = provider.extractMetadata(fileInfo, item.primaryInfo.primaryId, item.primaryInfo);
            
            // For tmap, resolve to base image
            var primaryInfo = item.primaryInfo;
            if (item.type === 'tmap') {
                primaryInfo = self._resolveTmapToBaseImage(reader, fileInfo, primaryInfo);
            }

            // Codec gate. Mirrors the SUPPORTED_CODECS list in
            // _loadAndCacheItem so the modal also surfaces a real "no
            // decoder" state instead of silently bailing (the previous
            // `if(item.meta.codec === "AV1"){return;}` left the spinner
            // running forever AND leaked the just-allocated reader +
            // provider).
            var codecUpper = (item.meta.codec || '').toString().toUpperCase();
            var SUPPORTED = ['HEVC', 'HVC1', 'HEV1', 'VVC', 'VVC1', 'VVI1', 'UNKNOWN', ''];
            if (SUPPORTED.indexOf(codecUpper) === -1) {
                self._showModalError(content,
                    'No decoder available for ' + (item.meta.codec || 'this codec') + '.');
                self._teardownActiveModalSession();
                return;
            }

            // Dispatch based on type
            var type = item.type || 'still';
            
            switch (type) {
                case 'animation':
                    self._displayAnimationModal(item, content, provider);
                    break;
                case 'burst':
                    self._displayBurstModal(item, content, provider);
                    break;
                case 'grid':
                    self._displayGridModal(item, content, provider, primaryInfo);
                    break;
                case 'overlay':
                    self._displayOverlayModal(item, content, provider, primaryInfo);
                    break;
                case 'stereo':
                    self._displayStereoModal(item, content, provider, primaryInfo);
                    break;
                case 'bothie':
                    self._displayBothieModal(item, content, provider, primaryInfo);
                    break;
                case 'collection':
                    self._displayCollectionModal(item, content, provider, primaryInfo);
                    break;
                // New entity group types
                case 'panorama':
                    self._displayPanoramaModal(item, content, provider, primaryInfo);
                    break;
                case 'slideshow':
                    self._displaySlideshowModal(item, content, provider, primaryInfo);
                    break;
                case 'progressive':
                    self._displayProgressiveModal(item, content, provider, primaryInfo);
                    break;
                case 'audio-image':
                    self._displayAudioImageModal(item, content, provider, primaryInfo);
                    break;
                case 'bracket-exposure':
                case 'bracket-wb':
                case 'bracket-focus':
                case 'bracket-flash':
                case 'bracket-dof':
                    self._displayBracketModal(item, content, provider, primaryInfo);
                    break;
                case 'album':
                    self._displayAlbumFavoritesModal(item, content, provider, primaryInfo, 'albc', 'Album');
                    break;
                case 'favorites':
                    self._displayAlbumFavoritesModal(item, content, provider, primaryInfo, 'favc', 'Favorites');
                    break;
                case 'altr':
                    self._displayAltrModal(item, content, provider, primaryInfo);
                    break;
                case 'iden':
                    self._displayIdenModal(item, content, provider, primaryInfo);
                    break;
                default:
                    // still, tmap, alpha display as still image
                    self._displayStillModal(item, content, provider, primaryInfo);
            }
        }).catch(function(e) {
            // Same leak guard as _openModalForItem: anything thrown after
            // the reader is stashed on `self._activeReader` would otherwise
            // strand the wasm reader.
            console.error('Failed to open HEIF for modal:', item.heifPath, e);
            self._showModalError(content, 'Could not open file \u2014 ' + (e.message || 'unexpected error'));
            self._teardownActiveModalSession();
        });
    };

    /**
     * Display identity-derived image in modal.
     *
     * Shows the source coded image alongside the derived presentation so the
     * viewer can see what the `iden` wrapper adds (typically irot / imir).
     */
    this._displayIdenModal = function(item, content, provider, primaryInfo) {
        var meta = item.meta;
        var reader = provider._reader;
        var primaryId = primaryInfo.primaryId;

        // Resolve iden chain → coded source, collecting transforms at each step
        var chain = [];  // [{id, irot, imir}, ...]
        var cur = primaryId;
        var seen = {};
        while (true) {
            try { if (reader.itemType(cur) !== 'iden') break; } catch (e) { break; }
            if (seen[cur]) break;
            seen[cur] = true;
            var props = null;
            try { props = reader.itemPropertiesResolved(cur); } catch (e) {}
            chain.push({
                id: cur,
                irot: props ? props.irot : undefined,
                imir: props ? props.imir : undefined
            });
            var sources = null;
            try { sources = reader.referencedFrom(cur, 'dimg'); } catch (e) {}
            if (sources && sources.length > 0) {
                cur = sources[0];
            } else {
                break;
            }
        }
        var sourceId = (chain.length > 0 && cur !== primaryId) ? cur : null;

        if (sourceId === null) {
            self._displayStillModal(item, content, provider, primaryInfo);
            return;
        }

        // Build combined transform description from the chain
        var transformParts = [];
        for (var ci = 0; ci < chain.length; ci++) {
            var step = chain[ci];
            var parts = [];
            if (typeof step.irot === 'number' && step.irot !== 0) {
                parts.push('rotation ' + step.irot + '°');
            }
            if (step.imir) {
                parts.push('mirror (' + (step.imir.axis || '?') + ' axis)');
            }
            if (parts.length > 0) {
                transformParts.push('item ' + step.id + ': ' + parts.join(' + '));
            }
        }
        var transformLabel = transformParts.length > 0
            ? transformParts.join(' → ')
            : 'no visible transforms';

        // Compute cumulative rotation/mirror for the derived canvas
        var totalRot = 0;
        var totalMir = null;
        for (var ci = chain.length - 1; ci >= 0; ci--) {
            if (typeof chain[ci].irot === 'number') {
                totalRot = (totalRot + chain[ci].irot) % 360;
            }
            if (chain[ci].imir) totalMir = chain[ci].imir;
        }
        var idenProps = { irot: totalRot, imir: totalMir };

        var sessionId = self._modalSessionId;

        // Decode the source item
        provider.requestImageData(primaryInfo.contextId, [sourceId], function(result) {
            if (sessionId !== self._modalSessionId) return;

            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');

            if (result.noDecoder) {
                self._showNoDecoderPlaceholder(content, item, meta, result);
                return;
            }

            if (!result.success || result.frames.length === 0) {
                self._showModalError(content, 'Failed to decode source image');
                return;
            }

            var frame = result.frames[0];

            // --- Source canvas (no transforms) ---
            var srcColorInfo = meta && meta.color ? meta.color : null;
            var srcCR = HEIFUtils.createColorManagedCanvas(frame.width, frame.height, srcColorInfo);
            var srcCanvas = srcCR.canvas;
            srcCR.ctx.putImageData(new ImageData(frame.canvasFrameData, frame.width, frame.height), 0, 0);
            srcCanvas = self._stepDownsampleForModal(srcCanvas);
            srcCanvas.style.maxWidth = '100%';
            srcCanvas.style.height = 'auto';

            // --- Derived canvas (with iden transforms, pixel-level rotation) ---
            var drvCR = HEIFUtils.createColorManagedCanvas(frame.width, frame.height, srcColorInfo);
            drvCR.ctx.putImageData(
                new ImageData(new Uint8ClampedArray(frame.canvasFrameData), frame.width, frame.height), 0, 0);
            var drvCanvas = drvCR.canvas;
            var rot = idenProps ? idenProps.irot : 0;
            var mir = idenProps ? idenProps.imir : null;
            // Pixel-level rotation: draw into a new canvas with swapped dims
            if (rot === 90 || rot === 270) {
                var rc = document.createElement('canvas');
                rc.width = frame.height;
                rc.height = frame.width;
                var rctx = rc.getContext('2d');
                rctx.translate(rc.width / 2, rc.height / 2);
                rctx.rotate(rot * Math.PI / 180);
                rctx.drawImage(drvCanvas, -frame.width / 2, -frame.height / 2);
                drvCanvas = rc;
            } else if (rot === 180) {
                var rc = document.createElement('canvas');
                rc.width = frame.width;
                rc.height = frame.height;
                var rctx = rc.getContext('2d');
                rctx.translate(rc.width / 2, rc.height / 2);
                rctx.rotate(Math.PI);
                rctx.drawImage(drvCanvas, -frame.width / 2, -frame.height / 2);
                drvCanvas = rc;
            }
            if (mir) {
                var mc = document.createElement('canvas');
                mc.width = drvCanvas.width;
                mc.height = drvCanvas.height;
                var mctx = mc.getContext('2d');
                if (mir.axis === 'horizontal') {
                    mctx.translate(0, mc.height);
                    mctx.scale(1, -1);
                } else {
                    mctx.translate(mc.width, 0);
                    mctx.scale(-1, 1);
                }
                mctx.drawImage(drvCanvas, 0, 0);
                drvCanvas = mc;
            }
            drvCanvas = self._stepDownsampleForModal(drvCanvas);
            drvCanvas.style.maxWidth = '100%';
            drvCanvas.style.height = 'auto';

            // --- Info box describing the derivation ---
            var infoBox = document.createElement('div');
            infoBox.style.cssText = 'display:flex; flex-direction:column; align-items:center; justify-content:center; padding:12px 18px; background:rgba(255,255,255,0.07); border-radius:8px; min-width:120px; text-align:center; align-self:center;';
            var title = document.createElement('div');
            title.style.cssText = 'font-weight:600; font-size:14px; color:#fff; margin-bottom:6px;';
            title.textContent = 'iden';
            infoBox.appendChild(title);
            var desc = document.createElement('div');
            desc.style.cssText = 'font-size:12px; color:#aaa; line-height:1.5;';
            var chainText = chain.map(function(s) { return 'item ' + s.id; }).join(' \u2192 ') +
                ' \u2192 item ' + sourceId;
            desc.appendChild(document.createTextNode(chainText));
            desc.appendChild(document.createElement('br'));
            desc.appendChild(document.createTextNode(transformLabel || 'identity'));
            infoBox.appendChild(desc);

            // --- Layout: source | info | derived ---
            var wrapper = document.createElement('div');
            wrapper.style.cssText = 'display:flex; flex-wrap:wrap; gap:16px; justify-content:center; align-items:center; width:100%;';

            var srcPanel = document.createElement('div');
            srcPanel.style.cssText = 'display:flex; flex-direction:column; align-items:center; max-width:42%;';
            var srcLabel = document.createElement('div');
            srcLabel.style.cssText = 'font-size:12px; color:#888; margin-bottom:4px;';
            srcLabel.textContent = 'Source (item ' + sourceId + ')';
            srcPanel.appendChild(srcLabel);
            srcPanel.appendChild(srcCanvas);

            var drvPanel = document.createElement('div');
            drvPanel.style.cssText = 'display:flex; flex-direction:column; align-items:center; max-width:42%;';
            var drvLabel = document.createElement('div');
            drvLabel.style.cssText = 'font-size:12px; color:#888; margin-bottom:4px;';
            drvLabel.textContent = 'Derived (item ' + primaryId + ')';
            drvPanel.appendChild(drvLabel);
            drvPanel.appendChild(drvCanvas);

            wrapper.appendChild(srcPanel);
            wrapper.appendChild(infoBox);
            wrapper.appendChild(drvPanel);
            content.appendChild(wrapper);

            if (meta) {
                meta.width = frame.width;
                meta.height = frame.height;
                if (frame.bitDepth) meta.bitDepth = frame.bitDepth;
            }

            self._addInfoPanels(item, meta);
        });
    };

    /**
     * Display stereo pair in modal - simple single-image view with toggle
     * Stereo pairs are detected via 'ster' entity groups in the metabox
     */
    this._displayStereoModal = function(item, content, provider, primaryInfo) {
        var meta = item.meta;
        var fileInfo = item.fileInfo;
        
        // Get stereo pair item IDs from entity groups
        var stereoItemIds = self._getStereoItemIds(fileInfo);
        
        if (!stereoItemIds || stereoItemIds.length < 2) {
            console.warn('Stereo: Could not find stereo pair items, falling back to still display');
            self._displayStillModal(item, content, provider, primaryInfo);
            return;
        }
        
        // Request both stereo images
        provider.requestImageData(primaryInfo.contextId, stereoItemIds, function(result) {
            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');
            
            if (!result.success || result.frames.length < 2) {
                console.error('Stereo: Failed to decode both images, got', result.frames.length, 'frames');
                if (result.frames.length === 1) {
                    self._displaySingleFrameAsFallback(content, result.frames[0], meta, item);
                } else {
                    self._showModalError(content, 'Failed to decode stereo pair');
                }
                return;
            }
            
            var leftFrame = result.frames[0];
            var rightFrame = result.frames[1];
            
            // Store frames for view switching
            var stereoState = {
                leftFrame: leftFrame,
                rightFrame: rightFrame,
                currentView: 'left'
            };
            
            // Create main canvas container
            var canvasWrapper = document.createElement('div');
            canvasWrapper.className = 'gallery-burst-main'; // Reuse burst styling
            
            // Create the display canvas
            var canvas = document.createElement('canvas');
            canvas.width = leftFrame.width;
            canvas.height = leftFrame.height;
            canvasWrapper.appendChild(canvas);
            
            // Create view mode buttons container (reuse burst controls styling)
            var controls = document.createElement('div');
            controls.className = 'gallery-burst-controls';
            
            // Create view buttons using existing button styles
            var views = [
                { id: 'left', label: 'Left' },
                { id: 'right', label: 'Right' },
                { id: 'anaglyph', label: '3D' }
            ];
            
            var buttons = {};
            views.forEach(function(view) {
                var btn = document.createElement('button');
                btn.className = 'gallery-burst-btn' + (view.id === 'left' ? ' active' : '');
                btn.textContent = view.label;
                btn.onclick = function() {
                    // Update button states
                    for (var id in buttons) {
                        buttons[id].classList.remove('active');
                    }
                    btn.classList.add('active');
                    
                    // Switch view
                    stereoState.currentView = view.id;
                    self._updateStereoCanvas(canvas, stereoState);
                };
                buttons[view.id] = btn;
                controls.appendChild(btn);
            });
            
            content.appendChild(controls);
            content.appendChild(canvasWrapper);
            
            // Initial render (left eye)
            self._updateStereoCanvas(canvas, stereoState);
            
            // Update meta
            meta.width = leftFrame.width;
            meta.height = leftFrame.height;
            if (leftFrame.bitDepth) meta.bitDepth = leftFrame.bitDepth;
            
            self._addInfoPanels(item, meta);
        });
    };
    
    /**
     * Update stereo canvas based on current view mode
     */
    this._updateStereoCanvas = function(canvas, stereoState) {
        var ctx = canvas.getContext('2d');
        var leftFrame = stereoState.leftFrame;
        var rightFrame = stereoState.rightFrame;
        
        switch (stereoState.currentView) {
            case 'left':
                canvas.width = leftFrame.width;
                canvas.height = leftFrame.height;
                ctx.putImageData(new ImageData(leftFrame.canvasFrameData, leftFrame.width, leftFrame.height), 0, 0);
                break;
                
            case 'right':
                canvas.width = rightFrame.width;
                canvas.height = rightFrame.height;
                ctx.putImageData(new ImageData(rightFrame.canvasFrameData, rightFrame.width, rightFrame.height), 0, 0);
                break;
                
            case 'anaglyph':
                var width = Math.max(leftFrame.width, rightFrame.width);
                var height = Math.max(leftFrame.height, rightFrame.height);
                canvas.width = width;
                canvas.height = height;
                
                var leftData = leftFrame.canvasFrameData;
                var rightData = rightFrame.canvasFrameData;
                var outputData = ctx.createImageData(width, height);
                var output = outputData.data;
                
                for (var y = 0; y < height; y++) {
                    for (var x = 0; x < width; x++) {
                        var outIdx = (y * width + x) * 4;
                        var leftIdx = (x < leftFrame.width && y < leftFrame.height) 
                            ? (y * leftFrame.width + x) * 4 : 0;
                        var rightIdx = (x < rightFrame.width && y < rightFrame.height) 
                            ? (y * rightFrame.width + x) * 4 : 0;
                        
                        // Red from left, cyan (G+B) from right
                        output[outIdx] = leftData[leftIdx];
                        output[outIdx + 1] = rightData[rightIdx + 1];
                        output[outIdx + 2] = rightData[rightIdx + 2];
                        output[outIdx + 3] = 255;
                    }
                }
                ctx.putImageData(outputData, 0, 0);
                break;
        }
    };
    
    /**
     * Get stereo item IDs from entity groups in fileInfo
     * @returns {Array|null} Array of [leftItemId, rightItemId] or null
     */
    this._getStereoItemIds = function(fileInfo) {
        var ids = self._getEntityGroupItemIds(fileInfo, 'ster');
        return (ids && ids.length >= 2) ? [ids[0], ids[1]] : null;
    };
    
    /**
     * Get bothie (time-synchronized) item IDs from entity groups in fileInfo
     * 'tsyn' FourCC indicates time-synchronized captures (e.g., front+back camera)
     * @returns {Array|null} Array of [frontItemId, backItemId] or null
     */
    this._getBothieItemIds = function(fileInfo) {
        var ids = self._getEntityGroupItemIds(fileInfo, 'tsyn');
        return (ids && ids.length >= 2) ? ids : null;
    };
    
    // =========================================================================
    // ENTITY GROUP HELPER FUNCTIONS
    // =========================================================================
    
    /**
     * Generic helper to get item IDs from a specific entity group type
     * @param {Object} fileInfo - FileInfo from HEIFReader
     * @param {string} groupType - FourCC group type (e.g., 'pano', 'slid', 'aebr')
     * @returns {Array|null} Array of item IDs or null
     */
    this._getEntityGroupItemIds = function(fileInfo, groupType) {
        if (!fileInfo || !fileInfo.rootLevelMetaBoxProperties) {
            return null;
        }
        var groups = fileInfo.rootLevelMetaBoxProperties.entityGroups;
        if (!groups) return null;
        for (var i = 0; i < groups.length; i++) {
            if (groups[i].type === groupType
                && groups[i].entityIds
                && groups[i].entityIds.length >= 1) {
                return groups[i].entityIds.slice();
            }
        }
        return null;
    };
    
    /**
     * Get panorama item IDs from 'pano' entity group
     */
    this._getPanoramaItemIds = function(fileInfo) {
        return this._getEntityGroupItemIds(fileInfo, 'pano');
    };
    
    /**
     * Get slideshow item IDs from 'slid' entity group
     */
    this._getSlideshowItemIds = function(fileInfo) {
        return this._getEntityGroupItemIds(fileInfo, 'slid');
    };
    
    /**
     * Get progressive rendering item IDs from 'prgr' entity group
     */
    this._getProgressiveItemIds = function(fileInfo) {
        return this._getEntityGroupItemIds(fileInfo, 'prgr');
    };
    
    /**
     * Get audio-image association from 'iaug' entity group
     * @returns {Object|null} {audioItemId, imageItemIds[]} or null
     */
    this._getAudioImageGroup = function(fileInfo) {
        var itemIds = this._getEntityGroupItemIds(fileInfo, 'iaug');
        if (itemIds && itemIds.length >= 2) {
            return {
                audioItemId: itemIds[0],
                imageItemIds: itemIds.slice(1)
            };
        }
        return null;
    };
    
    /**
     * Get bracketing item IDs from various bracket entity groups
     * @param {string} bracketType - One of: 'aebr', 'wbbr', 'fobr', 'afbr', 'dobr'
     */
    this._getBracketItemIds = function(fileInfo, bracketType) {
        return this._getEntityGroupItemIds(fileInfo, bracketType);
    };
    
    /**
     * Get all bracketing item IDs (checks all bracket types)
     */
    this._getAnyBracketItemIds = function(fileInfo) {
        var bracketTypes = ['aebr', 'wbbr', 'fobr', 'afbr', 'dobr'];
        for (var i = 0; i < bracketTypes.length; i++) {
            var itemIds = this._getEntityGroupItemIds(fileInfo, bracketTypes[i]);
            if (itemIds) {
                return { type: bracketTypes[i], itemIds: itemIds };
            }
        }
        return null;
    };
    
    /**
     * Get album group item IDs from 'albc' entity group
     */
    this._getAlbumItemIds = function(fileInfo) {
        return this._getEntityGroupItemIds(fileInfo, 'albc');
    };
    
    /**
     * Get ALL groups of a specific type (returns array of groups)
     * Each group contains: { groupId, itemIds[] }
     * @param {Object} fileInfo - FileInfo from HEIFReader
     * @param {string} groupType - FourCC group type (e.g., 'albc', 'favc')
     * @returns {Array} Array of group objects with itemIds
     */
    this._getAllEntityGroups = function(fileInfo, groupType) {
        var groups = [];
        if (!fileInfo || !fileInfo.rootLevelMetaBoxProperties) {
            return groups;
        }
        var entityGroups = fileInfo.rootLevelMetaBoxProperties.entityGroups;
        if (!entityGroups) return groups;
        for (var i = 0; i < entityGroups.length; i++) {
            var g = entityGroups[i];
            if (g.type === groupType && g.entityIds && g.entityIds.length >= 1) {
                groups.push({
                    groupIndex: groups.length,
                    groupId:    g.groupId,
                    itemIds:    g.entityIds.slice()
                });
            }
        }
        return groups;
    };
    
    /**
     * Get all album groups from 'albc' entity groups
     */
    this._getAllAlbumGroups = function(fileInfo) {
        return this._getAllEntityGroups(fileInfo, 'albc');
    };
    
    /**
     * Get all favorites groups from 'favc' entity groups
     */
    this._getAllFavoritesGroups = function(fileInfo) {
        return this._getAllEntityGroups(fileInfo, 'favc');
    };
    
    /**
     * Get favorites item IDs from 'favc' entity group
     */
    this._getFavoritesItemIds = function(fileInfo) {
        return this._getEntityGroupItemIds(fileInfo, 'favc');
    };
    
    /**
     * Get alternative representations from 'altr' entity group
     */
    this._getAlternativeItemIds = function(fileInfo) {
        return this._getEntityGroupItemIds(fileInfo, 'altr');
    };
    
    /**
     * Get equivalence group item IDs from 'eqiv' entity group
     */
    this._getEquivalenceItemIds = function(fileInfo) {
        return this._getEntityGroupItemIds(fileInfo, 'eqiv');
    };
    
    /**
     * Fallback display for single frame when stereo decode partial fails
     */
    this._displaySingleFrameAsFallback = function(content, frame, meta, item) {
        var canvas = document.createElement('canvas');
        canvas.width = frame.width;
        canvas.height = frame.height;
        // Let CSS handle sizing
        var ctx = canvas.getContext('2d');
        ctx.putImageData(new ImageData(frame.canvasFrameData, frame.width, frame.height), 0, 0);
        
        content.appendChild(canvas);
        
        meta.width = frame.width;
        meta.height = frame.height;
        if (frame.bitDepth) meta.bitDepth = frame.bitDepth;
        
        self._addInfoPanels(item, meta);
    };

    /**
     * Display bothie (time-synchronized front+back camera) images in modal
     * Shows one image at a time with buttons to switch cameras
     */
    this._displayBothieModal = function(item, content, provider, primaryInfo) {
        var meta = item.meta;
        var fileInfo = item.fileInfo;
        
        // Get bothie item IDs from entity groups
        var bothieItemIds = this._getBothieItemIds(fileInfo);
        if (!bothieItemIds || bothieItemIds.length < 2) {
            console.warn('Bothie: Unable to determine bothie item IDs, falling back to still display');
            this._displayStillModal(item, content, provider, primaryInfo);
            return;
        }
        
        // Request both bothie images using requestImageData (like stereo does)
        provider.requestImageData(primaryInfo.contextId, bothieItemIds, function(result) {
            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');
            
            if (!result.success || result.frames.length < 2) {
                console.error('Bothie: Failed to decode both images, got', result.frames ? result.frames.length : 0, 'frames');
                if (result.frames && result.frames.length === 1) {
                    self._displaySingleFrameAsFallback(content, result.frames[0], meta, item);
                } else {
                    self._showModalError(content, 'Failed to decode bothie images');
                }
                return;
            }
            
            var frontFrame = result.frames[0];
            var backFrame = result.frames[1];
            
            // Store state for view switching
            var bothieState = {
                frontFrame: frontFrame,
                backFrame: backFrame,
                currentView: 'front'
            };
            
            // Create view mode buttons container (reuse burst controls styling)
            var controls = document.createElement('div');
            controls.className = 'gallery-burst-controls';
            
            // Create view buttons
            var views = [
                { id: 'front', label: 'Front' },
                { id: 'back', label: 'Back' }
            ];
            
            var buttons = {};
            views.forEach(function(view) {
                var btn = document.createElement('button');
                btn.className = 'gallery-burst-btn' + (view.id === 'front' ? ' active' : '');
                btn.textContent = view.label;
                btn.onclick = function() {
                    // Update button states
                    for (var id in buttons) {
                        buttons[id].classList.remove('active');
                    }
                    btn.classList.add('active');
                    
                    // Switch view
                    bothieState.currentView = view.id;
                    self._updateBothieCanvas(canvas, bothieState);
                    
                    // Update meta for current frame
                    var frame = view.id === 'front' ? frontFrame : backFrame;
                    meta.width = frame.width;
                    meta.height = frame.height;
                    if (frame.bitDepth) meta.bitDepth = frame.bitDepth;
                };
                buttons[view.id] = btn;
                controls.appendChild(btn);
            });
            
            // Create canvas container
            var canvasWrapper = document.createElement('div');
            canvasWrapper.className = 'gallery-burst-main';
            
            // Create canvas for image display
            var canvas = document.createElement('canvas');
            canvas.width = frontFrame.width;
            canvas.height = frontFrame.height;
            canvasWrapper.appendChild(canvas);
            
            content.appendChild(controls);
            content.appendChild(canvasWrapper);
            
            // Initial render (front camera)
            self._updateBothieCanvas(canvas, bothieState);
            
            // Update meta with front frame info
            meta.width = frontFrame.width;
            meta.height = frontFrame.height;
            if (frontFrame.bitDepth) meta.bitDepth = frontFrame.bitDepth;
            
            self._addInfoPanels(item, meta);
        });
    };
    
    /**
     * Update bothie canvas based on current view
     */
    this._updateBothieCanvas = function(canvas, bothieState) {
        var frame = bothieState.currentView === 'front' 
            ? bothieState.frontFrame 
            : bothieState.backFrame;
        
        canvas.width = frame.width;
        canvas.height = frame.height;
        var ctx = canvas.getContext('2d');
        ctx.putImageData(new ImageData(frame.canvasFrameData, frame.width, frame.height), 0, 0);
    };

    // =========================================================================
    // ENTITY GROUP MODAL DISPLAYS (Placeholders)
    // =========================================================================

    /**
     * Display panorama images in modal
     * 'pano' entity group contains images that form a panorama
     * NOTE: Currently displays panorama tiles as a browsable collection.
     * Full equirectangular stitching/WebGL viewer deferred until a
     * 'pano' entity-group fixture is available for testing.
     */
    this._displayPanoramaModal = function(item, content, provider, primaryInfo) {
        var meta = item.meta;
        var fileInfo = item.fileInfo;
        var itemIds = self._getPanoramaItemIds(fileInfo);
        
        if (!itemIds || itemIds.length < 1) {
            console.warn('Panorama: No panorama items found, falling back to still display');
            self._displayStillModal(item, content, provider, primaryInfo);
            return;
        }
        
        // For now, display as a browsable collection
        self._displayEntityGroupAsCollection(item, content, provider, primaryInfo, itemIds, 'Panorama');
    };

    /**
     * Display slideshow in modal
     * 'slid' entity group contains images for sequential display
     * NOTE: Currently displays slideshow items as a browsable collection.
     * Auto-advance with per-slide timing deferred until a 'slid'
     * entity-group fixture is available for testing.
     */
    this._displaySlideshowModal = function(item, content, provider, primaryInfo) {
        var meta = item.meta;
        var fileInfo = item.fileInfo;
        var itemIds = self._getSlideshowItemIds(fileInfo);
        
        if (!itemIds || itemIds.length < 2) {
            console.warn('Slideshow: Not enough items for slideshow, falling back to still display');
            self._displayStillModal(item, content, provider, primaryInfo);
            return;
        }
        
        // Display as browsable collection with slideshow controls
        self._displayEntityGroupAsCollection(item, content, provider, primaryInfo, itemIds, 'Slideshow');
    };

    /**
     * Display progressive rendering layers in modal
     * 'prgr' entity group contains base + enhancement layers
     * NOTE: Currently displays progressive layers as a browsable collection.
     * Layer-by-layer refinement visualization deferred until a 'prgr'
     * entity-group fixture is available for testing.
     */
    this._displayProgressiveModal = function(item, content, provider, primaryInfo) {
        var meta = item.meta;
        var fileInfo = item.fileInfo;
        var itemIds = self._getProgressiveItemIds(fileInfo);
        
        if (!itemIds || itemIds.length < 1) {
            console.warn('Progressive: No progressive items found, falling back to still display');
            self._displayStillModal(item, content, provider, primaryInfo);
            return;
        }
        
        // Display final (highest quality) layer as still image
        
        // Use last item (highest quality) for display
        var finalItemId = itemIds[itemIds.length - 1];
        provider.requestImageData(primaryInfo.contextId, [finalItemId], function(result) {
            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');
            
            if (result.success && result.frames.length > 0) {
                var frame = result.frames[0];
                self._displaySingleFrameAsFallback(content, frame, meta, item);
            } else {
                self._showModalError(content, 'Failed to decode progressive image');
            }
        });
    };

    /**
     * Display audio-associated image in modal
     * 'iaug' entity group associates audio with images
     * Plays audio track alongside image display
     */
    this._displayAudioImageModal = function(item, content, provider, primaryInfo) {
        var meta = item.meta;
        var fileInfo = item.fileInfo;
        var audioImageGroup = self._getAudioImageGroup(fileInfo);
        
        if (!audioImageGroup) {
            console.warn('AudioImage: No audio-image association found, falling back to still display');
            self._displayStillModal(item, content, provider, primaryInfo);
            return;
        }
        
        
        // Get the reader to extract audio data
        var reader = self._activeReader;
        
        // Request images first
        provider.requestImageData(primaryInfo.contextId, audioImageGroup.imageItemIds, function(result) {
            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');
            
            if (!result.success || result.frames.length < 1) {
                self._showModalError(content, 'Failed to decode audio-image content');
                return;
            }
            
            var frames = result.frames;
            var currentIndex = 0;
            var audioElement = null;
            var audioUrl = null;
            
            // Try to extract and play audio
            try {
                var audioData = reader.itemBitstream(audioImageGroup.audioItemId);
                
                if (audioData && audioData.byteLength > 0) {
                    // Detect MIME type from item info
                    var mimeType = 'audio/mp4'; // Default
                    var itemInfoMap = fileInfo.rootLevelMetaBoxProperties.itemInformationMap;
                    if (itemInfoMap && itemInfoMap[audioImageGroup.audioItemId]) {
                        var audioItemInfo = itemInfoMap[audioImageGroup.audioItemId];
                        if (audioItemInfo.contentType) {
                            mimeType = audioItemInfo.contentType;
                        } else if (audioItemInfo.type) {
                            // Map item types to MIME types
                            var typeMap = {
                                'mp4a': 'audio/mp4',
                                'Opus': 'audio/ogg; codecs=opus',
                                'ac-3': 'audio/ac3',
                                'ec-3': 'audio/eac3',
                                'mp3 ': 'audio/mpeg',
                                'fLaC': 'audio/flac'
                            };
                            mimeType = typeMap[audioItemInfo.type] || 'audio/mp4';
                        }
                    }
                    
                    
                    var audioBlob = new Blob([new Uint8Array(audioData)], { type: mimeType });
                    audioUrl = URL.createObjectURL(audioBlob);
                    audioElement = new Audio(audioUrl);
                    audioElement.preload = 'auto';
                }
            } catch (e) {
                console.warn('AudioImage: Failed to extract audio data:', e);
            }
            
            // === BUILD UI ===
            
            // Audio controls bar (if audio available)
            if (audioElement) {
                var audioControls = document.createElement('div');
                audioControls.className = 'gallery-burst-controls';
                
                var playPauseBtn = document.createElement('button');
                playPauseBtn.className = 'gallery-burst-btn';
                playPauseBtn.textContent = 'Play';
                playPauseBtn.onclick = function() {
                    if (audioElement.paused) {
                        audioElement.play();
                        playPauseBtn.textContent = 'Pause';
                    } else {
                        audioElement.pause();
                        playPauseBtn.textContent = 'Play';
                    }
                };
                
                var stopBtn = document.createElement('button');
                stopBtn.className = 'gallery-burst-btn';
                stopBtn.textContent = 'Stop';
                stopBtn.onclick = function() {
                    audioElement.pause();
                    audioElement.currentTime = 0;
                    playPauseBtn.textContent = 'Play';
                };
                
                var timeLabel = document.createElement('span');
                timeLabel.className = 'audio-time';
                timeLabel.textContent = '0:00 / 0:00';
                
                // Volume control
                var volumeLabel = document.createElement('span');
                volumeLabel.className = 'audio-volume-label';
                volumeLabel.textContent = 'Vol';
                
                var volumeSlider = document.createElement('input');
                volumeSlider.type = 'range';
                volumeSlider.min = '0';
                volumeSlider.max = '100';
                volumeSlider.value = '80';
                volumeSlider.oninput = function() {
                    audioElement.volume = volumeSlider.value / 100;
                };
                audioElement.volume = 0.8;
                
                // Update time display
                audioElement.ontimeupdate = function() {
                    var current = formatTime(audioElement.currentTime);
                    var duration = formatTime(audioElement.duration || 0);
                    timeLabel.textContent = current + ' / ' + duration;
                };
                
                audioElement.onended = function() {
                    playPauseBtn.textContent = 'Play';
                };
                
                function formatTime(seconds) {
                    if (isNaN(seconds)) return '0:00';
                    var mins = Math.floor(seconds / 60);
                    var secs = Math.floor(seconds % 60);
                    return mins + ':' + (secs < 10 ? '0' : '') + secs;
                }
                
                audioControls.appendChild(playPauseBtn);
                audioControls.appendChild(stopBtn);
                audioControls.appendChild(timeLabel);
                audioControls.appendChild(volumeLabel);
                audioControls.appendChild(volumeSlider);
                
                content.appendChild(audioControls);
            } else {
                // Show "no audio" indicator
                var noAudioBar = document.createElement('div');
                noAudioBar.className = 'gallery-burst-controls';
                noAudioBar.style.marginBottom = '10px';
                var noAudioLabel = document.createElement('span');
                noAudioLabel.className = 'audio-status-label';
                noAudioLabel.textContent = 'Audio track could not be loaded';
                noAudioBar.appendChild(noAudioLabel);
                content.appendChild(noAudioBar);
            }
            
            // Image navigation controls (if multiple images)
            var navControls = document.createElement('div');
            navControls.className = 'gallery-burst-controls';
            
            if (frames.length > 1) {
                var prevBtn = document.createElement('button');
                prevBtn.className = 'gallery-burst-btn';
                prevBtn.textContent = 'Prev';
                prevBtn.onclick = function() {
                    if (currentIndex > 0) {
                        currentIndex--;
                        updateDisplay();
                    }
                };
                
                var indexLabel = document.createElement('span');
                indexLabel.className = 'audio-time';
                
                var nextBtn = document.createElement('button');
                nextBtn.className = 'gallery-burst-btn';
                nextBtn.textContent = 'Next';
                nextBtn.onclick = function() {
                    if (currentIndex < frames.length - 1) {
                        currentIndex++;
                        updateDisplay();
                    }
                };
                
                navControls.appendChild(prevBtn);
                navControls.appendChild(indexLabel);
                navControls.appendChild(nextBtn);
            }
            // Only add nav controls if there are multiple images (no "Image with Audio" label for single images)
            if (frames.length > 1) {
                content.appendChild(navControls);
            }
            
            // Canvas for image display
            var canvasWrapper = document.createElement('div');
            canvasWrapper.className = 'gallery-burst-main';
            
            var canvas = document.createElement('canvas');
            canvasWrapper.appendChild(canvas);
            content.appendChild(canvasWrapper);
            
            function updateDisplay() {
                var frame = frames[currentIndex];
                canvas.width = frame.width;
                canvas.height = frame.height;
                var ctx = canvas.getContext('2d');
                ctx.putImageData(new ImageData(frame.canvasFrameData, frame.width, frame.height), 0, 0);
                
                if (frames.length > 1) {
                    indexLabel.textContent = 'Image ' + (currentIndex + 1) + '/' + frames.length;
                    prevBtn.disabled = (currentIndex === 0);
                    nextBtn.disabled = (currentIndex === frames.length - 1);
                    prevBtn.style.opacity = prevBtn.disabled ? '0.5' : '1';
                    nextBtn.style.opacity = nextBtn.disabled ? '0.5' : '1';
                }
                
                meta.width = frame.width;
                meta.height = frame.height;
                if (frame.bitDepth) meta.bitDepth = frame.bitDepth;
            }
            
            // Initial display
            updateDisplay();
            self._addInfoPanels(item, meta);
            
            // Register audio teardown via the modal cleanup list. This
            // runs on full close AND on arrow-key navigation away from the
            // current item (see _runModalCleanups), so audio doesn't keep
            // playing in the background after the user moves on.
            self._addModalCleanup(function() {
                try {
                    if (audioElement) {
                        audioElement.pause();
                        audioElement.src = '';
                    }
                } catch (e) { /* ignore */ }
                try {
                    if (audioUrl) URL.revokeObjectURL(audioUrl);
                } catch (e) { /* ignore */ }
            });
        });
    };

    /**
     * Display bracketing images in modal
     * Handles all bracket types: aebr, wbbr, fobr, afbr, dobr
     * Shows multiple exposures/settings as browsable collection
     */
    this._displayBracketModal = function(item, content, provider, primaryInfo) {
        var meta = item.meta;
        var fileInfo = item.fileInfo;
        var bracketInfo = self._getAnyBracketItemIds(fileInfo);
        
        if (!bracketInfo) {
            console.warn('Bracket: No bracketing group found, falling back to still display');
            self._displayStillModal(item, content, provider, primaryInfo);
            return;
        }
        
        var bracketLabels = {
            'aebr': 'Exposure Bracket',
            'wbbr': 'White Balance Bracket',
            'fobr': 'Focus Bracket',
            'afbr': 'Flash Bracket',
            'dobr': 'Depth of Field Bracket'
        };
        
        var label = bracketLabels[bracketInfo.type] || 'Bracket';
        
        self._displayEntityGroupAsCollection(item, content, provider, primaryInfo, 
                                             bracketInfo.itemIds, label);
    };

    /**
     * Display album or favorites groups in modal
     * Shows groups separately with item IDs, allowing browsing one group at a time
     * Example: Group 1 (0001, 0002, 0003) | Group 2 (0001, 0005, 0004)
     */
    this._displayAlbumFavoritesModal = function(item, content, provider, primaryInfo, groupType, typeLabel) {
        var meta = item.meta;
        var fileInfo = item.fileInfo;
        var allGroups = self._getAllEntityGroups(fileInfo, groupType);
        
        if (!allGroups || allGroups.length < 1) {
            console.warn(typeLabel + ': No groups found, falling back to still display');
            self._displayStillModal(item, content, provider, primaryInfo);
            return;
        }
        
        
        // Collect all unique item IDs across all groups for batch loading
        var allItemIds = [];
        var itemIdSet = {};
        for (var g = 0; g < allGroups.length; g++) {
            for (var i = 0; i < allGroups[g].itemIds.length; i++) {
                var id = allGroups[g].itemIds[i];
                if (!itemIdSet[id]) {
                    itemIdSet[id] = true;
                    allItemIds.push(id);
                }
            }
        }
        
        // Request all images at once
        provider.requestImageData(primaryInfo.contextId, allItemIds, function(result) {
            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');
            
            if (!result.success || result.frames.length < 1) {
                self._showModalError(content, 'Failed to decode ' + typeLabel + ' images');
                return;
            }
            
            // Add album-view class for CSS targeting
            content.classList.add('album-view');
            
            // Build map of itemId -> frame for quick lookup
            var frameMap = {};
            for (var f = 0; f < result.frames.length; f++) {
                var frame = result.frames[f];
                // Match by index since requestImageData returns in same order
                frameMap[allItemIds[f]] = frame;
            }
            
            // State
            var currentGroupIndex = 0;
            var currentItemIndex = 0;
            
            // === BUILD UI ===
            
            // Top controls: Group selector
            var groupControls = document.createElement('div');
            groupControls.className = 'gallery-album-controls';
            
            // Group navigation
            var prevGroupBtn = document.createElement('button');
            prevGroupBtn.className = 'gallery-burst-btn';
            prevGroupBtn.textContent = 'Previous';
            prevGroupBtn.onclick = function() {
                if (currentGroupIndex > 0) {
                    currentGroupIndex--;
                    currentItemIndex = 0;
                    updateDisplay();
                }
            };
            
            var groupLabel = document.createElement('span');
            groupLabel.className = 'gallery-album-label';
            
            var nextGroupBtn = document.createElement('button');
            nextGroupBtn.className = 'gallery-burst-btn';
            nextGroupBtn.textContent = 'Next';
            nextGroupBtn.onclick = function() {
                if (currentGroupIndex < allGroups.length - 1) {
                    currentGroupIndex++;
                    currentItemIndex = 0;
                    updateDisplay();
                }
            };
            
            groupControls.appendChild(prevGroupBtn);
            groupControls.appendChild(groupLabel);
            groupControls.appendChild(nextGroupBtn);
            
            // Group items display (shows item IDs in current group)
            var groupItemsBar = document.createElement('div');
            groupItemsBar.className = 'gallery-album-items-bar';
            
            // Item navigation controls
            var itemControls = document.createElement('div');
            itemControls.className = 'gallery-album-controls';
            
            var prevItemBtn = document.createElement('button');
            prevItemBtn.className = 'gallery-burst-btn';
            prevItemBtn.textContent = '<';
            prevItemBtn.onclick = function() {
                if (currentItemIndex > 0) {
                    currentItemIndex--;
                    updateDisplay();
                }
            };
            
            var itemLabel = document.createElement('span');
            itemLabel.className = 'gallery-album-label';
            
            var nextItemBtn = document.createElement('button');
            nextItemBtn.className = 'gallery-burst-btn';
            nextItemBtn.textContent = '>';
            nextItemBtn.onclick = function() {
                var currentGroup = allGroups[currentGroupIndex];
                if (currentItemIndex < currentGroup.itemIds.length - 1) {
                    currentItemIndex++;
                    updateDisplay();
                }
            };
            
            itemControls.appendChild(prevItemBtn);
            itemControls.appendChild(itemLabel);
            itemControls.appendChild(nextItemBtn);
            
            // Canvas container
            var canvasWrapper = document.createElement('div');
            canvasWrapper.className = 'gallery-burst-main';
            
            var canvas = document.createElement('canvas');
            canvasWrapper.appendChild(canvas);
            
            // Assemble UI
            content.appendChild(groupControls);
            content.appendChild(groupItemsBar);
            content.appendChild(itemControls);
            content.appendChild(canvasWrapper);
            
            function formatItemId(id) {
                // Format as 4-digit padded number
                return String(id).padStart(4, '0');
            }
            
            function updateDisplay() {
                var currentGroup = allGroups[currentGroupIndex];
                var itemIds = currentGroup.itemIds;
                var currentItemId = itemIds[currentItemIndex];
                var frame = frameMap[currentItemId];
                
                // Update group label
                groupLabel.textContent = typeLabel + ' Group ' + (currentGroupIndex + 1) + '/' + allGroups.length;
                
                // Update group navigation buttons
                prevGroupBtn.disabled = (currentGroupIndex === 0);
                nextGroupBtn.disabled = (currentGroupIndex === allGroups.length - 1);
                prevGroupBtn.style.opacity = prevGroupBtn.disabled ? '0.5' : '1';
                nextGroupBtn.style.opacity = nextGroupBtn.disabled ? '0.5' : '1';
                
                // Update group items bar (show all IDs in current group)
                groupItemsBar.replaceChildren();
                for (var i = 0; i < itemIds.length; i++) {
                    var idBtn = document.createElement('button');
                    idBtn.className = 'gallery-album-id-btn' + (i === currentItemIndex ? ' active' : '');
                    idBtn.textContent = formatItemId(itemIds[i]);
                    (function(idx) {
                        idBtn.onclick = function() {
                            currentItemIndex = idx;
                            updateDisplay();
                        };
                    })(i);
                    groupItemsBar.appendChild(idBtn);
                }
                
                // Update item navigation
                itemLabel.textContent = 'Item ' + (currentItemIndex + 1) + '/' + itemIds.length + ' (ID: ' + formatItemId(currentItemId) + ')';
                prevItemBtn.disabled = (currentItemIndex === 0);
                nextItemBtn.disabled = (currentItemIndex === itemIds.length - 1);
                prevItemBtn.style.opacity = prevItemBtn.disabled ? '0.5' : '1';
                nextItemBtn.style.opacity = nextItemBtn.disabled ? '0.5' : '1';
                
                // Update canvas
                if (frame) {
                    canvas.width = frame.width;
                    canvas.height = frame.height;
                    var ctx = canvas.getContext('2d');
                    ctx.putImageData(new ImageData(frame.canvasFrameData, frame.width, frame.height), 0, 0);
                    
                    // Update meta
                    meta.width = frame.width;
                    meta.height = frame.height;
                    if (frame.bitDepth) meta.bitDepth = frame.bitDepth;
                } else {
                    // Frame not found - show placeholder
                    canvas.width = 400;
                    canvas.height = 300;
                    var ctx = canvas.getContext('2d');
                    ctx.fillStyle = '#333';
                    ctx.fillRect(0, 0, 400, 300);
                    ctx.fillStyle = '#fff';
                    ctx.font = '16px sans-serif';
                    ctx.textAlign = 'center';
                    ctx.fillText('Item ' + formatItemId(currentItemId) + ' not found', 200, 150);
                }
            }
            
            // Initial display
            updateDisplay();
            self._addInfoPanels(item, meta);
        });
    };

    /**
     * Display alternate-item group (altr) in modal.
     * Shows one alternate at a time with buttons to switch between them.
     * Each alternate is a different rendition of the same content
     * (different resolution, codec, etc.).
     */
    this._displayAltrModal = function(item, content, provider, primaryInfo) {
        var meta = item.meta;
        var fileInfo = item.fileInfo;
        var altrIds = self._getAlternativeItemIds(fileInfo);

        if (!altrIds || altrIds.length < 2) {
            // Not enough alternates — fall back to still display.
            self._displayStillModal(item, content, provider, primaryInfo);
            return;
        }

        // Filter out hidden items (C011: hidden + altr grouped).
        var visibleIds = altrIds;
        if (fileInfo.hiddenIds && fileInfo.hiddenIds.length > 0) {
            visibleIds = altrIds.filter(function (id) { return fileInfo.hiddenIds.indexOf(id) === -1; });
        }
        if (visibleIds.length === 0) visibleIds = altrIds;  // all hidden → show anyway

        // Request each alternate individually so one unsupported codec
        // (e.g. AVC) does not block a supported sibling (e.g. HEVC).
        var pending = visibleIds.length;
        // Slot array keeps results aligned with visibleIds by index.
        var slots = new Array(visibleIds.length);

        function onAllSettled() {
            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');

            // Build a unified entries array: { frame, itemId, error }
            var entries = [];
            for (var i = 0; i < visibleIds.length; i++) {
                var slot = slots[i];
                if (slot && slot.frame) {
                    entries.push({ frame: slot.frame, itemId: visibleIds[i], error: null });
                } else {
                    var reason = (slot && slot.error) ? slot.error : 'Decode failed';
                    entries.push({ frame: null, itemId: visibleIds[i], error: reason });
                }
            }

            // If every single alternate failed, show a blanket error.
            var anyOk = entries.some(function(e) { return e.frame !== null; });
            if (!anyOk) {
                // Collect per-item reasons so the user sees why each
                // alternate was not decodable (e.g. AVC base layer vs
                // L-HEVC enhancement layer that depends on it).
                var reasons = [];
                for (var ri = 0; ri < entries.length; ri++) {
                    reasons.push('Item ' + entries[ri].itemId + ': ' + entries[ri].error);
                }
                self._showModalError(content,
                    'None of the ' + entries.length + ' alternate items could be decoded\n'
                    + reasons.join('\n'));
                return;
            }

            var currentIdx = 0;

            // Canvas
            var canvasWrapper = document.createElement('div');
            canvasWrapper.className = 'gallery-burst-main';
            var canvas = document.createElement('canvas');
            var errorOverlay = document.createElement('div');
            errorOverlay.className = 'gallery-altr-error-overlay';
            errorOverlay.style.cssText = 'display:none; color:rgba(255,255,255,0.7); text-align:center; padding:40px 20px; font-size:14px;';
            canvasWrapper.appendChild(canvas);
            canvasWrapper.appendChild(errorOverlay);

            // Controls
            var controls = document.createElement('div');
            controls.className = 'gallery-album-controls';

            var prevBtn = document.createElement('button');
            prevBtn.className = 'gallery-burst-btn';
            prevBtn.textContent = 'Prev';

            var label = document.createElement('span');
            label.className = 'gallery-album-label';

            var nextBtn = document.createElement('button');
            nextBtn.className = 'gallery-burst-btn';
            nextBtn.textContent = 'Next';

            prevBtn.onclick = function() { if (currentIdx > 0) { currentIdx--; update(); } };
            nextBtn.onclick = function() { if (currentIdx < entries.length - 1) { currentIdx++; update(); } };

            controls.appendChild(prevBtn);
            controls.appendChild(label);
            controls.appendChild(nextBtn);

            content.appendChild(canvasWrapper);
            content.appendChild(controls);

            function update() {
                var entry = entries[currentIdx];
                if (entry.frame) {
                    // Show decoded image
                    canvas.style.display = '';
                    errorOverlay.style.display = 'none';
                    canvas.width = entry.frame.width;
                    canvas.height = entry.frame.height;
                    var ctx = canvas.getContext('2d');
                    ctx.putImageData(new ImageData(entry.frame.canvasFrameData, entry.frame.width, entry.frame.height), 0, 0);

                    label.textContent = 'Alternate ' + (currentIdx + 1) + ' / ' + entries.length
                        + ' (item ' + entry.itemId + ', '
                        + entry.frame.width + '×' + entry.frame.height + ')';

                    meta.width = entry.frame.width;
                    meta.height = entry.frame.height;
                    if (entry.frame.bitDepth) meta.bitDepth = entry.frame.bitDepth;
                } else {
                    // Show error placeholder for this alternate
                    canvas.style.display = 'none';
                    errorOverlay.style.display = '';
                    errorOverlay.textContent = 'Item ' + entry.itemId + ': ' + entry.error;

                    label.textContent = 'Alternate ' + (currentIdx + 1) + ' / ' + entries.length
                        + ' (item ' + entry.itemId + ')';
                }

                prevBtn.disabled = (currentIdx === 0);
                nextBtn.disabled = (currentIdx === entries.length - 1);
                prevBtn.style.opacity = prevBtn.disabled ? '0.5' : '1';
                nextBtn.style.opacity = nextBtn.disabled ? '0.5' : '1';
            }

            update();
            self._addInfoPanels(item, meta);
        }

        for (var i = 0; i < visibleIds.length; i++) {
            (function(idx) {
                provider.requestImageData(primaryInfo.contextId, [visibleIds[idx]], function(result) {
                    if (result.noDecoder) {
                        slots[idx] = { frame: null, error: 'No decoder available for this codec' };
                    } else if (result.success && result.frames.length > 0) {
                        slots[idx] = { frame: result.frames[0] };
                    } else {
                        var msg = result.error || 'Decode failed';
                        // Replace raw library errors with user-friendly messages.
                        if (msg.indexOf('UNSUPPORTED_CODE_TYPE') !== -1 || msg.indexOf('codec type not support') !== -1) {
                            msg = 'Codec not supported';
                        } else if (msg.indexOf('Bitstream gather failed') !== -1) {
                            msg = 'Could not extract image data';
                        }
                        slots[idx] = { frame: null, error: msg };
                    }
                    pending--;
                    if (pending === 0) onAllSettled();
                });
            })(i);
        }
    };

    /**
     * Generic helper to display entity group items as a browsable collection
     * Used by panorama, slideshow, bracket, audio-image, etc.
     */
    this._displayEntityGroupAsCollection = function(item, content, provider, primaryInfo, itemIds, label) {
        var meta = item.meta;
        
        // Request all images in the group
        provider.requestImageData(primaryInfo.contextId, itemIds, function(result) {
            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');
            
            if (!result.success || result.frames.length < 1) {
                self._showModalError(content, 'Failed to decode ' + label + ' images');
                return;
            }
            
            var frames = result.frames;
            var currentIndex = 0;
            
            // Create controls container
            var controls = document.createElement('div');
            controls.className = 'gallery-burst-controls';
            
            // Create navigation buttons
            var prevBtn = document.createElement('button');
            prevBtn.className = 'gallery-burst-btn';
            prevBtn.textContent = 'Prev';
            prevBtn.onclick = function() {
                if (currentIndex > 0) {
                    currentIndex--;
                    updateDisplay();
                }
            };
            
            var indexLabel = document.createElement('span');
            indexLabel.className = 'gallery-album-label';
            
            var nextBtn = document.createElement('button');
            nextBtn.className = 'gallery-burst-btn';
            nextBtn.textContent = 'Next';
            nextBtn.onclick = function() {
                if (currentIndex < frames.length - 1) {
                    currentIndex++;
                    updateDisplay();
                }
            };
            
            controls.appendChild(prevBtn);
            controls.appendChild(indexLabel);
            controls.appendChild(nextBtn);
            
            // Create canvas container
            var canvasWrapper = document.createElement('div');
            canvasWrapper.className = 'gallery-burst-main';
            
            var canvas = document.createElement('canvas');
            canvasWrapper.appendChild(canvas);
            
            content.appendChild(controls);
            content.appendChild(canvasWrapper);
            
            function updateDisplay() {
                var frame = frames[currentIndex];
                canvas.width = frame.width;
                canvas.height = frame.height;
                var ctx = canvas.getContext('2d');
                ctx.putImageData(new ImageData(frame.canvasFrameData, frame.width, frame.height), 0, 0);
                
                indexLabel.textContent = label + ' ' + (currentIndex + 1) + '/' + frames.length;
                
                prevBtn.disabled = (currentIndex === 0);
                nextBtn.disabled = (currentIndex === frames.length - 1);
                prevBtn.style.opacity = prevBtn.disabled ? '0.5' : '1';
                nextBtn.style.opacity = nextBtn.disabled ? '0.5' : '1';
                
                // Update meta
                meta.width = frame.width;
                meta.height = frame.height;
                if (frame.bitDepth) meta.bitDepth = frame.bitDepth;
            }
            
            // Initial display
            updateDisplay();
            self._addInfoPanels(item, meta);
        });
    };

    /**
     * Display still image in modal
     */
    this._displayStillModal = function(item, content, provider, primaryInfo) {
        var meta = item.meta;
        
        // Get full-size image (not thumbnail)
        var itemToLoad = provider.filterImageItems(item.fileInfo, primaryInfo.primaryId);
        
        if (itemToLoad === null) {
            // The primary item exists but is not a coded image (e.g.
            // Exif-only or mime metadata item). This typically happens
            // with conformance files whose primary item is a derived
            // definition with no decodable pixel data.
            var primaryType = '';
            try { primaryType = provider._reader.itemType(primaryInfo.primaryId); } catch (e) {}
            var detail = primaryType
                ? 'Primary item (type \u201C' + primaryType + '\u201D) is not a decodable image'
                : 'Primary item contains no decodable image data';
            self._showModalError(content, detail);
            return;
        }
        
        provider.requestImageData(primaryInfo.contextId, [itemToLoad], function(result) {
            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');
            
            // Handle no decoder available case - show placeholder with metadata
            if (result.noDecoder) {
                self._showNoDecoderPlaceholder(content, item, meta, result);
                return;
            }
            
            if (result.success && result.frames.length > 0) {
                var frame = result.frames[0];
                
                // Create color-managed canvas
                var colorInfo = meta && meta.color ? meta.color : null;
                var canvasResult = HEIFUtils.createColorManagedCanvas(
                    frame.width, 
                    frame.height, 
                    colorInfo
                );
                var canvas = canvasResult.canvas;
                var ctx = canvasResult.ctx;
                
                var imageData = new ImageData(frame.canvasFrameData, frame.width, frame.height);
                ctx.putImageData(imageData, 0, 0);
                
                // Apply all transformations (CLAP, ISCL, IROT, IMIR)
                var transformedCanvas = self._applyTransformations(canvas, meta, false);

                // Step-downsample for smooth modal display of large images.
                // Guarded: if the source canvas is invalid (e.g. decode partially
                // failed producing 0×0), skip downsampling to avoid masking the
                // real error.
                var displayCanvas = transformedCanvas;
                if (transformedCanvas && transformedCanvas.width > 0 && transformedCanvas.height > 0) {
                    displayCanvas = self._stepDownsampleForModal(transformedCanvas);
                }
                
                // For 90/270 rotations, wrap in container for proper viewport constraints
                var elementToAppend = displayCanvas;
                if (meta && (meta.rotation === 90 || meta.rotation === 270)) {
                    elementToAppend = self._wrapRotatedCanvasForModal(displayCanvas, meta.rotation);
                }
                
                content.appendChild(elementToAppend);
                
                // Update meta with actual decoded dimensions
                meta.width = frame.width;
                meta.height = frame.height;
                if (frame.bitDepth) meta.bitDepth = frame.bitDepth;
                
                self._addInfoPanels(item, meta);
            } else {
                self._showModalError(content,
                    'File does not contain decodable image data');
            }
        });
    };

    /**
     * Display animation in modal (GIF-like - decodes frames from track and loops automatically)
     * Animations have NO playback controls - they just loop like GIFs
     */
    this._displayAnimationModal = function(item, content, provider) {
        var meta = item.meta;
        var fileInfo = item.fileInfo;
        var contextId = item.primaryInfo.contextId;
        
        // Get frame IDs and timing from track
        var frameIds = [];
        var fps = 24;  // Default
        var trackProp = null;  // snapshot for edit-list data
        
        if (fileInfo.trackProperties && fileInfo.trackProperties.length > 0) {
            for (var i = 0; i < fileInfo.trackProperties.length; i++) {
                var track = fileInfo.trackProperties[i];
                if (track.trackFeature && track.trackFeature.isMasterImageSequence) {
                    trackProp = track;
                    if (track.frameRate && track.frameRate.den > 0) {
                        fps = track.frameRate.num / track.frameRate.den;
                    }
                    if (track.sampleIds && track.sampleIds.length > 0) {
                        frameIds = track.sampleIds.slice();
                    }
                    break;
                }
            }
        }
        
        if (frameIds.length === 0) {
            self._showModalError(content, 'No frames found in animation');
            return;
        }

        // Snapshot session at decode start - if the user closes or
        // arrow-navigates while the (potentially long) decode is in
        // flight the callback will fire after teardown, attach a canvas
        // to the new modal session's content element and spawn a
        // setInterval that drives the wrong frames into it.
        var sessionId = self._modalSessionId;

        // Decode all frames
        provider.requestImageData(contextId, frameIds, function(result) {
            if (sessionId !== self._modalSessionId) return;
            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');
            
            if (!result.success || result.frames.length === 0) {
                self._showModalError(content, 'Failed to decode animation');
                return;
            }
            
            meta.frameCount = result.frames.length;
            meta.width = result.frames[0].width;
            meta.height = result.frames[0].height;
            meta.fps = fps;
            if (result.frames[0].bitDepth) meta.bitDepth = result.frames[0].bitDepth;
            
            // Create color-managed canvas for animation
            var colorInfo = meta && meta.color ? meta.color : null;
            var canvasResult = HEIFUtils.createColorManagedCanvas(
                result.frames[0].width, 
                result.frames[0].height, 
                colorInfo
            );
            var canvas = canvasResult.canvas;
            // Let CSS handle sizing
            content.appendChild(canvas);
            
            var ctx = canvasResult.ctx;
            var frames = result.frames;
            var currentFrame = 0;
            var interval = fps > 0 ? 1000 / fps : 100;
            var animationTimer = null;
            
            // Pre-create ImageData objects for each frame
            var imageDataArray = frames.map(function(frame) {
                var imageData = ctx.createImageData(frame.width, frame.height);
                imageData.data.set(frame.canvasFrameData);
                return imageData;
            });
            
            function drawFrame(index) {
                if (index >= 0 && index < imageDataArray.length) {
                    ctx.putImageData(imageDataArray[index], 0, 0);
                }
            }

            // ---------------------------------------------------------
            // Edit-list-aware presentation timeline
            //
            // ISO 14496-12 §8.6.6: the edit list maps *presentation*
            // time to *media* time. Each edit unit defines a segment:
            //   EMPTY  — no media; hold blank for durationMovieTs
            //   DWELL  — hold a single frame (mediaTime) for duration
            //   SHIFT  — play media starting at mediaTime for duration
            //   RAW    — same as SHIFT (unrecognised sub-type)
            //
            // When there is no edit list we fall back to the simple
            // setInterval loop as before.
            // ---------------------------------------------------------
            var editUnits = (trackProp && trackProp.editUnits && trackProp.editUnits.length > 0)
                          ? trackProp.editUnits : null;
            var editMeta  = (trackProp && trackProp.editListMeta) ? trackProp.editListMeta : null;
            var timeScale = (trackProp && trackProp.timeScale) ? trackProp.timeScale : 0;

            // Build a flat presentation schedule: [{frameIndex, durationMs}, ...]
            // that the rAF loop walks sequentially.
            var schedule = null;
            if (editUnits && timeScale > 0) {
                schedule = self._buildEditListSchedule(editUnits, timeScale, frames.length, fps);
            }

            // Looping metadata.  repetitions=0 + looping → infinite.
            var looping       = editMeta ? editMeta.looping    : true;  // default: loop forever
            var repetitions   = editMeta ? editMeta.repetitions : 0;
            var loopCount     = 0;  // completed loops so far
            var schedIdx      = 0;

            if (schedule && schedule.length > 0) {
                // Edit-list-aware rAF loop
                drawFrame(schedule[0].frameIndex);
                var lastDraw = performance.now();
                var rAFid = null;

                function animateScheduled(now) {
                    if (!rAFid) return;  // cancelled
                    var entry = schedule[schedIdx];
                    if (now - lastDraw >= entry.durationMs) {
                        schedIdx++;
                        if (schedIdx >= schedule.length) {
                            loopCount++;
                            if (!looping) { rAFid = null; return; }
                            if (repetitions > 0 && loopCount >= repetitions) { rAFid = null; return; }
                            schedIdx = 0;
                        }
                        drawFrame(schedule[schedIdx].frameIndex);
                        lastDraw = now;
                    }
                    rAFid = requestAnimationFrame(animateScheduled);
                }

                rAFid = requestAnimationFrame(animateScheduled);

                self._sequenceCleanup = function() {
                    if (rAFid) { cancelAnimationFrame(rAFid); rAFid = null; }
                };
            } else {
                // Simple fixed-interval loop (no edit list)
                function animate() {
                    drawFrame(currentFrame);
                    currentFrame = (currentFrame + 1) % frames.length;
                }
                drawFrame(0);
                if (frames.length > 1) {
                    animationTimer = setInterval(animate, interval);
                }
                self._sequenceCleanup = function() {
                    if (animationTimer) {
                        clearInterval(animationTimer);
                        animationTimer = null;
                    }
                };
            }
            
            // Show frame info (non-interactive)
            if (frames.length > 1) {
                var info = document.createElement('div');
                info.className = 'gallery-burst-indicator';
                var label = frames.length + ' frames @ ' + fps.toFixed(1) + ' fps';
                if (schedule) {
                    label += ' · ' + schedule.length + ' edit steps';
                    if (editMeta && editMeta.looping) {
                        label += editMeta.repetitions > 0
                            ? ' · repeat ×' + editMeta.repetitions
                            : ' · loop ∞';
                    }
                }
                info.textContent = label;
                content.appendChild(info);
            }
            
            self._addInfoPanels(item, meta);
        });
    };

    /**
     * Build a flat presentation schedule from edit-list units.
     *
     * Each entry in the returned array is {frameIndex, durationMs}.
     * The schedule represents one full pass of the edit list.
     *
     * Edit unit types (ISO 14496-12 §8.6.6):
     *   EMPTY — blank for durationMovieTs (no frame drawn; we emit
     *           frameIndex -1 to hold the previous frame)
     *   DWELL — hold one frame (mediaTimeTrackTs) for durationMovieTs
     *   SHIFT — play media from mediaTimeTrackTs for durationMovieTs
     *   RAW   — treated as SHIFT
     */
    this._buildEditListSchedule = function(editUnits, timeScale, frameCount, fps) {
        var schedule = [];
        var frameDurTs = (fps > 0 && timeScale > 0) ? (timeScale / fps) : 1;

        for (var i = 0; i < editUnits.length; i++) {
            var eu = editUnits[i];
            var durMs = (timeScale > 0)
                ? (eu.durationMovieTs / timeScale) * 1000
                : 1000 / fps;

            if (eu.editType === 'EMPTY') {
                // Hold previous frame (or blank) for the duration.
                schedule.push({ frameIndex: -1, durationMs: durMs });
                continue;
            }

            if (eu.editType === 'DWELL') {
                // Hold a single frame for the whole duration.
                var dwellIdx = Math.round(eu.mediaTimeTrackTs / frameDurTs);
                if (dwellIdx >= frameCount) dwellIdx = frameCount - 1;
                if (dwellIdx < 0) dwellIdx = 0;
                schedule.push({ frameIndex: dwellIdx, durationMs: durMs });
                continue;
            }

            // SHIFT / RAW — play media starting at mediaTimeTrackTs
            // for durationMovieTs.
            var rate = (eu.mediaRate > 0) ? eu.mediaRate : 1;
            var startIdx = Math.round(eu.mediaTimeTrackTs / frameDurTs);
            if (startIdx < 0) startIdx = 0;
            // Number of frames this segment covers at the given rate.
            var mediaDurTs = eu.durationMovieTs * rate;
            var segFrames  = Math.round(mediaDurTs / frameDurTs);
            if (segFrames < 1) segFrames = 1;
            var perFrameMs = durMs / segFrames;
            if (perFrameMs < 16) perFrameMs = 16;  // clamp ~60fps

            for (var f = 0; f < segFrames; f++) {
                var fi = startIdx + f;
                if (fi >= frameCount) fi = frameCount - 1;
                schedule.push({ frameIndex: fi, durationMs: perFrameMs });
            }
        }

        return schedule;
    };

    /**
     * Display burst images in modal - user can browse frames individually
     * Bursts have both track samples (for video/grid animation) AND metabox items (for stills).
     * 
     * For Video and Grid modes: Uses track samples (90 frames in rally_burst.heic)
     * For Single mode: Uses high-quality metabox items (4 items in rally_burst.heic)
     */
    this._displayBurstModal = function(item, content, provider) {
        var fileInfo = item.fileInfo;
        var reader = self._activeReader;  // Use the active reader stored during modal open
        
        // Store provider on item for use in _renderBurstVideo
        item.provider = provider;
        
        // Get track context and timestamps for track frames (Video/Grid modes).
        // Modern reader exposes trackTimestamps(trackId) returning
        // [{timestampMs, sampleId}, ...]. The legacy contextId is the trackId.
        var trackContextId = provider.getMasterContextId(fileInfo);
        var timestamps = [];
        if (trackContextId) {
            try { timestamps = reader.trackTimestamps(trackContextId) || []; }
            catch (e) { timestamps = []; }
        }
        var trackFrameIds = timestamps.map(function(t) { return t.sampleId; });
        
        // Get metabox context and master IDs for still images (Single mode)
        var metaboxContextId = fileInfo.rootLevelMetaBoxProperties ? 
            fileInfo.rootLevelMetaBoxProperties.contextId : 0;
        var metaboxFrameIds = provider.getMetaBoxMasterIds(fileInfo) || [];
        metaboxFrameIds = provider.filterImageItems(fileInfo, metaboxFrameIds);
        
        // Use track frames if available (Video/Grid), fallback to metabox if no track
        var hasTrackFrames = trackFrameIds && trackFrameIds.length > 0;
        var hasMetaboxFrames = metaboxFrameIds && metaboxFrameIds.length > 0;
        
        if (!hasTrackFrames && !hasMetaboxFrames) {
            self._showModalError(content, 'No frames found in burst');
            return;
        }
        
        // Request track frames for Video/Grid modes
        var contextId = hasTrackFrames ? trackContextId : metaboxContextId;
        var frameIds = hasTrackFrames ? trackFrameIds : metaboxFrameIds;
        
        provider.requestImageData(contextId, frameIds, function(result) {
            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');
            
            if (!result.success || result.frames.length === 0) {
                self._showModalError(content, 'Failed to decode burst');
                return;
            }
            
            var meta = item.meta;
            meta.frameCount = result.frames.length;
            meta.width = result.frames[0].width;
            meta.height = result.frames[0].height;
            if (result.frames[0].bitDepth) meta.bitDepth = result.frames[0].bitDepth;
            
            // Store metabox data for Single mode if different from track data
            item.metaboxContextId = metaboxContextId;
            item.metaboxFrameIds = metaboxFrameIds;
            item.hasMetaboxStills = hasMetaboxFrames && hasTrackFrames && (metaboxFrameIds.length !== trackFrameIds.length);
            // Store frame IDs for indicator display
            item.frameIds = frameIds;
            
            self._renderBurstView(content, item, result.frames);
            self._addInfoPanels(item, meta);
        });
    };

    /**
     * Display derived image (grid/overlay) in modal - uses cached data for compositing
     */
    this._displayDerivedModal = function(item, content) {
        if (item.type === 'grid') {
            self._displayGridModal(item, content);
        } else {
            self._displayOverlayModal(item, content);
        }
    };

    /**
     * Display grid image in modal - composites tile images
     */
    this._displayGridModal = function(item, content, provider, primaryInfo) {
        var meta = item.meta;
        
        // Get grid definition
        var gridInfo = provider.getGridItemInfo(item.fileInfo, primaryInfo.primaryId);
        if (!gridInfo || !gridInfo.tileIds || gridInfo.tileIds.length === 0) {
            self._showModalError(content, 'No grid tiles found');
            return;
        }
        
        var cols = gridInfo.columns;
        var rows = gridInfo.rows;
        var outputWidth = gridInfo.outputWidth;
        var outputHeight = gridInfo.outputHeight;
        
        // Request all tile images
        provider.requestImageData(primaryInfo.contextId, gridInfo.tileIds, function(result) {
            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');
            
            // Handle no decoder available case - show placeholder with metadata
            if (result.noDecoder) {
                // Update meta with grid dimensions even without decoding
                meta.width = outputWidth;
                meta.height = outputHeight;
                self._showNoDecoderPlaceholder(content, item, meta, result);
                return;
            }
            
            if (!result.success || result.frames.length === 0) {
                self._showModalError(content, 'Failed to decode grid tiles');
                return;
            }
            
            // Create composite canvas at output dimensions
            var canvas = document.createElement('canvas');
            canvas.width = outputWidth;
            canvas.height = outputHeight;
            // Let CSS handle sizing - no inline styles
            var ctx = canvas.getContext('2d');
            
            // Fill black background
            ctx.fillStyle = 'black';
            ctx.fillRect(0, 0, outputWidth, outputHeight);
            
            // Get actual tile dimensions from first decoded tile
            var actualTileW = result.frames[0].width;
            var actualTileH = result.frames[0].height;
            
            // Composite tiles in grid order
            // Tiles are placed at their actual dimensions, edge tiles are cropped
            for (var j = 0; j < rows; j++) {
                for (var i = 0; i < cols; i++) {
                    var tileIndex = j * cols + i;
                    if (tileIndex < result.frames.length) {
                        var frame = result.frames[tileIndex];
                        
                        // Create temp canvas for tile
                        var tileCanvas = document.createElement('canvas');
                        tileCanvas.width = frame.width;
                        tileCanvas.height = frame.height;
                        var tileCtx = tileCanvas.getContext('2d');
                        var imageData = new ImageData(frame.canvasFrameData, frame.width, frame.height);
                        tileCtx.putImageData(imageData, 0, 0);
                        
                        // Calculate destination position
                        var destX = i * actualTileW;
                        var destY = j * actualTileH;
                        
                        // Calculate how much of the tile to draw (crop edge tiles)
                        var srcW = Math.min(frame.width, outputWidth - destX);
                        var srcH = Math.min(frame.height, outputHeight - destY);
                        
                        // Draw tile at grid position (crop if it extends beyond output)
                        if (srcW > 0 && srcH > 0) {
                            ctx.drawImage(tileCanvas, 0, 0, srcW, srcH, destX, destY, srcW, srcH);
                        }
                    }
                }
            }
            
            // Apply all transformations (CLAP, ISCL, IROT, IMIR)
            var transformedCanvas = self._applyTransformations(canvas, meta, false);
            
            // For 90/270 rotations, wrap in container for proper viewport constraints
            var elementToAppend = transformedCanvas;
            if (meta && (meta.rotation === 90 || meta.rotation === 270)) {
                elementToAppend = self._wrapRotatedCanvasForModal(transformedCanvas, meta.rotation);
            }
            
            content.appendChild(elementToAppend);
            
            // Update meta
            meta.width = outputWidth;
            meta.height = outputHeight;
            if (result.frames[0].bitDepth) meta.bitDepth = result.frames[0].bitDepth;
            
            self._addInfoPanels(item, meta);
        });
    };

    /**
     * Display overlay image in modal - composites with offsets
     */
    this._displayOverlayModal = function(item, content, provider, primaryInfo) {
        var meta = item.meta;
        
        // Get overlay definition
        var iovlInfo = provider.getOverlayItemInfo(item.fileInfo, primaryInfo.primaryId);
        if (!iovlInfo || !iovlInfo.images || iovlInfo.images.length === 0) {
            self._showModalError(content, 'No overlay sources found');
            return;
        }
        
        var outputWidth = iovlInfo.outputWidth;
        var outputHeight = iovlInfo.outputHeight;
        
        // Request all source images
        var overlayImageIds = iovlInfo.images.map(function (im) { return im.imageId; });
        provider.requestImageData(primaryInfo.contextId, overlayImageIds, function(result) {
            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');
            
            // Handle no decoder available case - show placeholder with metadata
            if (result.noDecoder) {
                // Update meta with overlay dimensions even without decoding
                meta.width = outputWidth;
                meta.height = outputHeight;
                self._showNoDecoderPlaceholder(content, item, meta, result);
                return;
            }
            
            if (!result.success || result.frames.length === 0) {
                self._showModalError(content, 'Failed to decode overlay sources');
                return;
            }
            
            // Create composite canvas at output dimensions
            var canvas = document.createElement('canvas');
            canvas.width = outputWidth;
            canvas.height = outputHeight;
            // Let CSS handle sizing - no inline styles
            var ctx = canvas.getContext('2d');
            
            // Fill with canvas fill value (16-bit sRGB → 8-bit rgba)
            if (iovlInfo.fillRgba) {
                var s = 256.0 / 65536;
                var r = Math.round(iovlInfo.fillRgba[0] * s);
                var g = Math.round(iovlInfo.fillRgba[1] * s);
                var b = Math.round(iovlInfo.fillRgba[2] * s);
                var a = (iovlInfo.fillRgba[3] * s) / 255;
                ctx.fillStyle = 'rgba(' + r + ',' + g + ',' + b + ',' + a + ')';
            } else {
                ctx.fillStyle = 'black';
            }
            ctx.fillRect(0, 0, outputWidth, outputHeight);
            
            // Composite images at their offsets
            for (var i = 0; i < result.frames.length && i < iovlInfo.images.length; i++) {
                var frame = result.frames[i];
                var layer = iovlInfo.images[i];
                
                // Create temp canvas for source
                var srcCanvas = document.createElement('canvas');
                srcCanvas.width = frame.width;
                srcCanvas.height = frame.height;
                var srcCtx = srcCanvas.getContext('2d');
                var imageData = new ImageData(frame.canvasFrameData, frame.width, frame.height);
                srcCtx.putImageData(imageData, 0, 0);
                
                // Draw at offset
                ctx.drawImage(srcCanvas, layer.hOffset, layer.vOffset);
            }
            
            // Apply all transformations (CLAP, ISCL, IROT, IMIR)
            var transformedCanvas = self._applyTransformations(canvas, meta, false);
            
            // For 90/270 rotations, wrap in container for proper viewport constraints
            var elementToAppend = transformedCanvas;
            if (meta && (meta.rotation === 90 || meta.rotation === 270)) {
                elementToAppend = self._wrapRotatedCanvasForModal(transformedCanvas, meta.rotation);
            }
            
            content.appendChild(elementToAppend);
            
            // Update meta
            meta.width = outputWidth;
            meta.height = outputHeight;
            if (result.frames[0].bitDepth) meta.bitDepth = result.frames[0].bitDepth;
            
            self._addInfoPanels(item, meta);
        });
    };

    /**
     * OLD: Display derived image (grid/overlay) in modal - uses img + processImageElements
     * NOTE: This creates a duplicate reader and wastes resources. Prefer _displayGridModal/_displayOverlayModal
     */
    this._displayDerivedModalViaExtension = function(item, content) {
        var meta = item.meta;
        
        // Use native img element for derived images (decoder handles composition)
        var heifImg = document.createElement('img');
        heifImg.src = item.heifPath;
        // Let CSS handle sizing
        
        var loading = document.getElementById('gallery-modal-loading');
        if (loading) loading.classList.add('hidden');
        
        content.appendChild(heifImg);
        processImageElements();
        
        heifImg.onload = function() {
            var canvas = content.querySelector('canvas');
            if (canvas) {
                meta.width = canvas.width || meta.width;
                meta.height = canvas.height || meta.height;
            }
        };
        
        self._addInfoPanels(item, meta);
    };

    /**
     * Display collection (multiple images) in modal with filmstrip navigation
     * Shows one large image with a scrollable strip of thumbnails below
     */
    this._displayCollectionModal = function(item, content, provider, primaryInfo) {
        var fileInfo = item.fileInfo;
        var contextId = primaryInfo.contextId;
        var meta = item.meta;
        
        // Get all master image IDs in the collection
        var masterIds = provider.getMasterIds(fileInfo) || [];
        masterIds = provider.filterImageItems(fileInfo, masterIds);
        
        if (!masterIds || masterIds.length === 0) {
            // Fallback to still display if no masters found
            self._displayStillModal(item, content, provider, primaryInfo);
            return;
        }
        
        // If only one image, just show it as a still
        if (masterIds.length === 1) {
            self._displayStillModal(item, content, provider, primaryInfo);
            return;
        }
        
        meta.frameCount = masterIds.length;
        
        // Request all images in the collection
        provider.requestImageData(contextId, masterIds, function(result) {
            var loading = document.getElementById('gallery-modal-loading');
            if (loading) loading.classList.add('hidden');
            
            if (!result.success || result.frames.length === 0) {
                self._showModalError(content, 'Failed to decode collection');
                return;
            }
            
            // Determine the initial image index (prefer primary if in collection)
            var initialIndex = 0;
            var primaryId = primaryInfo.primaryId;
            for (var i = 0; i < masterIds.length; i++) {
                if (masterIds[i] === primaryId) {
                    initialIndex = i;
                    break;
                }
            }
            
            // Create collection state
            var state = {
                frames: result.frames,
                itemIds: masterIds,
                currentIndex: initialIndex
            };
            
            // Store state on item for cleanup
            item.collectionState = state;
            
            // Render collection view
            self._renderCollectionView(content, state, item, meta);
        });
    };
    
    /**
     * Render collection view with main image and filmstrip
     */
    this._renderCollectionView = function(content, state, item, meta) {
        content.replaceChildren();
        
        // Add collection-view class for CSS targeting
        content.classList.add('collection-view');
        
        // Container for proper layout
        var container = document.createElement('div');
        container.className = 'gallery-collection-container';
        
        // Main image display area
        var mainDiv = document.createElement('div');
        mainDiv.className = 'gallery-collection-main';
        
        var mainCanvas = document.createElement('canvas');
        mainCanvas.className = 'gallery-collection-canvas';
        var frame = state.frames[state.currentIndex];
        mainCanvas.width = frame.width;
        mainCanvas.height = frame.height;
        
        mainCanvas.getContext('2d').putImageData(
            new ImageData(frame.canvasFrameData, frame.width, frame.height), 0, 0);
        
        mainDiv.appendChild(mainCanvas);
        
        // Image indicator (includes item ID for debugging/reference)
        var indicator = document.createElement('div');
        indicator.className = 'gallery-collection-indicator';
        var currentItemId = state.itemIds ? state.itemIds[state.currentIndex] : null;
        indicator.textContent = 'Image ' + (state.currentIndex + 1) + ' of ' + state.frames.length + 
            (currentItemId !== null ? ' (ID: ' + currentItemId + ')' : '');
        mainDiv.appendChild(indicator);
        
        container.appendChild(mainDiv);
        
        // Filmstrip for navigation
        var strip = document.createElement('div');
        strip.className = 'gallery-collection-strip';
        
        for (var i = 0; i < state.frames.length; i++) {
            (function(index) {
                var f = state.frames[index];
                var thumbCanvas = document.createElement('canvas');
                
                // Calculate thumbnail size (fixed height of 80px)
                var thumbHeight = 80;
                var scale = thumbHeight / f.height;
                thumbCanvas.width = Math.round(f.width * scale);
                thumbCanvas.height = thumbHeight;
                thumbCanvas.className = 'gallery-collection-thumb';
                
                // Draw scaled thumbnail
                var tempCanvas = document.createElement('canvas');
                tempCanvas.width = f.width;
                tempCanvas.height = f.height;
                tempCanvas.getContext('2d').putImageData(
                    new ImageData(f.canvasFrameData, f.width, f.height), 0, 0);
                
                thumbCanvas.getContext('2d').drawImage(tempCanvas, 0, 0, thumbCanvas.width, thumbCanvas.height);
                
                // Mark active thumbnail
                if (index === state.currentIndex) {
                    thumbCanvas.classList.add('active');
                }
                
                // Click handler to switch images
                thumbCanvas.addEventListener('click', function(e) {
                    e.stopPropagation(); // Prevent event bubbling
                    
                    state.currentIndex = index;
                    var fr = state.frames[index];
                    
                    // Update main canvas
                    mainCanvas.width = fr.width;
                    mainCanvas.height = fr.height;
                    mainCanvas.getContext('2d').putImageData(
                        new ImageData(fr.canvasFrameData, fr.width, fr.height), 0, 0);
                    
                    // Update indicator with item ID
                    var clickedItemId = state.itemIds ? state.itemIds[index] : null;
                    indicator.textContent = 'Image ' + (index + 1) + ' of ' + state.frames.length +
                        (clickedItemId !== null ? ' (ID: ' + clickedItemId + ')' : '');
                    
                    // Update active thumbnail
                    strip.querySelectorAll('.gallery-collection-thumb').forEach(function(c) {
                        c.classList.remove('active');
                    });
                    thumbCanvas.classList.add('active');
                    
                    // Update metadata in info panel
                    meta.width = fr.width;
                    meta.height = fr.height;
                    if (fr.bitDepth) meta.bitDepth = fr.bitDepth;
                    
                    // Scroll the clicked thumbnail into view
                    thumbCanvas.scrollIntoView({ behavior: 'smooth', inline: 'center', block: 'nearest' });
                });
                
                strip.appendChild(thumbCanvas);
            })(i);
        }
        
        container.appendChild(strip);
        content.appendChild(container);
        
        // Update meta with first frame info
        meta.width = state.frames[state.currentIndex].width;
        meta.height = state.frames[state.currentIndex].height;
        if (state.frames[state.currentIndex].bitDepth) {
            meta.bitDepth = state.frames[state.currentIndex].bitDepth;
        }
        
        self._addInfoPanels(item, meta);
        
        // Scroll initial thumbnail into view
        setTimeout(function() {
            var activeThumb = strip.querySelector('.gallery-collection-thumb.active');
            if (activeThumb) {
                activeThumb.scrollIntoView({ inline: 'center', block: 'nearest' });
            }
        }, 100);
    };

    /**
     * Render burst view with controls
     */
    this._renderBurstView = function(content, item, frames, timestamps) {
        content.replaceChildren();
        
        // Add burst-view class for landscape mobile styling
        content.classList.add('burst-view');
        
        var state = {
            frames: frames,
            timestamps: timestamps,
            currentIndex: 0,
            viewMode: 'video',
            animationTimer: null
        };
        
        // Store state for cleanup
        item.burstState = state;
        
        // Controls
        var controls = document.createElement('div');
        controls.className = 'gallery-burst-controls';
        
        var buttons = {};
        ['video', 'single', 'grid'].forEach(function(mode) {
            var btn = document.createElement('button');
            btn.className = 'gallery-burst-btn' + (state.viewMode === mode ? ' active' : '');
            btn.textContent = mode === 'single' ? 'Single' : mode === 'video' ? 'Video' : 'Grid';
            btn.onclick = function() {
                state.viewMode = mode;
                if (state.animationTimer) {
                    clearInterval(state.animationTimer);
                    state.animationTimer = null;
                }
                // Update button active states
                for (var m in buttons) {
                    buttons[m].classList.remove('active');
                }
                btn.classList.add('active');
                self._renderBurstContent(content, state, item);
            };
            buttons[mode] = btn;
            controls.appendChild(btn);
        });
        
        // Store controls reference for _renderBurstContent
        state.controls = controls;
        
        content.appendChild(controls);
        self._renderBurstContent(content, state, item);
    };

    /**
     * Render burst content based on view mode
     */
    this._renderBurstContent = function(content, state, item) {
        // Remove old content except controls (use stored reference)
        var controls = state.controls;
        content.replaceChildren();
        if (controls) content.appendChild(controls);
        
        if (state.viewMode === 'video') {
            self._renderBurstVideo(content, state, item);
        } else if (state.viewMode === 'single') {
            self._renderBurstSingle(content, state, item);
        } else {
            self._renderBurstGrid(content, state, item);
        }
    };

    /**
     * Render burst as video animation
     */
    this._renderBurstVideo = function(content, state, item) {
        var mainDiv = document.createElement('div');
        mainDiv.className = 'gallery-burst-main';
        
        var mainCanvas = document.createElement('canvas');
        var frame = state.frames[0];
        mainCanvas.width = frame.width;
        mainCanvas.height = frame.height;
        // Let CSS handle sizing
        
        mainCanvas.getContext('2d').putImageData(
            new ImageData(frame.canvasFrameData, frame.width, frame.height), 0, 0);
        
        mainDiv.appendChild(mainCanvas);
        content.appendChild(mainDiv);
        
        // Get FPS from API
        var fps = item.provider.getPlaybackFPS(item.fileInfo);
        if (fps <= 0) fps = 24; // Fallback only if API returns nothing
        
        var frameIndex = 0;
        state.animationTimer = setInterval(function() {
            frameIndex = (frameIndex + 1) % state.frames.length;
            var f = state.frames[frameIndex];
            mainCanvas.getContext('2d').putImageData(
                new ImageData(f.canvasFrameData, f.width, f.height), 0, 0);
        }, 1000 / fps);
    };

    /**
     * Render burst as single frame with filmstrip
     */
    this._renderBurstSingle = function(content, state, item) {
        var frameIds = item && item.frameIds ? item.frameIds : null;
        
        var mainDiv = document.createElement('div');
        mainDiv.className = 'gallery-burst-main';
        
        var mainCanvas = document.createElement('canvas');
        var frame = state.frames[state.currentIndex];
        mainCanvas.width = frame.width;
        mainCanvas.height = frame.height;
        // Let CSS handle sizing
        
        mainCanvas.getContext('2d').putImageData(
            new ImageData(frame.canvasFrameData, frame.width, frame.height), 0, 0);
        
        mainDiv.appendChild(mainCanvas);
        
        // Frame indicator with ID
        var frameIndicator = document.createElement('div');
        frameIndicator.className = 'gallery-burst-indicator';
        var currentFrameId = frameIds ? frameIds[state.currentIndex] : null;
        frameIndicator.textContent = 'Frame ' + (state.currentIndex + 1) + ' of ' + state.frames.length +
            (currentFrameId !== null ? ' (ID: ' + currentFrameId + ')' : '');
        mainDiv.appendChild(frameIndicator);
        
        content.appendChild(mainDiv);
        
        // Filmstrip
        var strip = document.createElement('div');
        strip.className = 'gallery-burst-strip';
        
        for (var i = 0; i < state.frames.length; i++) {
            (function(index) {
                var f = state.frames[index];
                var thumbCanvas = document.createElement('canvas');
                var scale = 60 / f.height;
                thumbCanvas.width = Math.round(f.width * scale);
                thumbCanvas.height = 60;
                
                var tempCanvas = document.createElement('canvas');
                tempCanvas.width = f.width;
                tempCanvas.height = f.height;
                tempCanvas.getContext('2d').putImageData(
                    new ImageData(f.canvasFrameData, f.width, f.height), 0, 0);
                
                thumbCanvas.getContext('2d').drawImage(tempCanvas, 0, 0, thumbCanvas.width, thumbCanvas.height);
                
                if (index === state.currentIndex) thumbCanvas.classList.add('active');
                
                thumbCanvas.onclick = function() {
                    state.currentIndex = index;
                    var fr = state.frames[index];
                    mainCanvas.width = fr.width;
                    mainCanvas.height = fr.height;
                    mainCanvas.getContext('2d').putImageData(
                        new ImageData(fr.canvasFrameData, fr.width, fr.height), 0, 0);
                    // Update frame indicator with ID
                    var clickedFrameId = frameIds ? frameIds[index] : null;
                    frameIndicator.textContent = 'Frame ' + (index + 1) + ' of ' + state.frames.length +
                        (clickedFrameId !== null ? ' (ID: ' + clickedFrameId + ')' : '');
                    strip.querySelectorAll('canvas').forEach(function(c) { c.classList.remove('active'); });
                    thumbCanvas.classList.add('active');
                };
                
                strip.appendChild(thumbCanvas);
            })(i);
        }
        
        content.appendChild(strip);
    };

    /**
     * Render burst as grid
     */
    this._renderBurstGrid = function(content, state, item) {
        var grid = document.createElement('div');
        grid.className = 'gallery-modal-grid';
        // Let CSS handle sizing
        
        for (var i = 0; i < state.frames.length; i++) {
            var f = state.frames[i];
            var canvas = document.createElement('canvas');
            var maxSize = 200;
            var scale = Math.min(maxSize / f.width, maxSize / f.height);
            canvas.width = Math.round(f.width * scale);
            canvas.height = Math.round(f.height * scale);
            
            var tempCanvas = document.createElement('canvas');
            tempCanvas.width = f.width;
            tempCanvas.height = f.height;
            tempCanvas.getContext('2d').putImageData(
                new ImageData(f.canvasFrameData, f.width, f.height), 0, 0);
            
            canvas.getContext('2d').drawImage(tempCanvas, 0, 0, canvas.width, canvas.height);
            grid.appendChild(canvas);
        }
        
        content.appendChild(grid);
    };

    /**
     * Add info panels to modal
     */
    this._addInfoPanels = function(item, meta) {
        HEIFUtils.getFileSize(item.heifPath, function(fileSize) {
            // Remove existing panels
            var existingPanel = document.getElementById('gallery-modal-info-panel');
            if (existingPanel) existingPanel.remove();
            var existingFullInfo = document.getElementById('gallery-modal-full-info');
            if (existingFullInfo) existingFullInfo.remove();
            
            var infoPanel = self._buildInfoPanel(item, meta, fileSize);
            document.body.appendChild(infoPanel);
            
            var fullInfoPanel = self._buildFullInfoPanel(item, meta, item.fileInfo);
            document.body.appendChild(fullInfoPanel);
        });
    };

    /**
     * Build info panel
     */
    this._buildInfoPanel = function(item, meta, fileSize) {
        var container = document.createElement('div');
        container.className = 'gallery-modal-info-panel';
        container.id = 'gallery-modal-info-panel';
        
        // Title row with info button
        var titleRow = document.createElement('div');
        titleRow.className = 'gallery-modal-info-title-row';
        
        var title = document.createElement('div');
        title.className = 'gallery-modal-info-title';
        title.textContent = item.title;
        titleRow.appendChild(title);
        
        // Info button - next to title
        var infoBtn = document.createElement('span');
        infoBtn.className = 'gallery-modal-info-btn';
        infoBtn.textContent = 'i';
        infoBtn.title = 'Show full details';
        infoBtn.onclick = function(e) {
            e.stopPropagation();
            self._toggleFullInfo();
        };
        titleRow.appendChild(infoBtn);
        
        container.appendChild(titleRow);
        
        var details = document.createElement('div');
        details.className = 'gallery-modal-info-details';
        
        // Format / Codec
        if (meta.format && meta.format !== 'Unknown') {
            var formatItem = document.createElement('span');
            formatItem.className = 'info-item info-format';
            var formatText = meta.format;
            if (meta.codec && meta.codec !== 'Unknown') {
                formatText += ' / ' + meta.codec;
            }
            formatItem.textContent = formatText;
            details.appendChild(formatItem);
        }
        
        // Dimensions
        if (meta.width && meta.height) {
            details.appendChild(HEIFUtils.createSeparator());
            var dimItem = document.createElement('span');
            dimItem.className = 'info-item';
            dimItem.innerHTML = meta.width + '×' + meta.height;
            details.appendChild(dimItem);
        }
        
        // Bit depth (if not 8)
        if (meta.bitDepth && meta.bitDepth !== 8) {
            details.appendChild(HEIFUtils.createSeparator());
            var bitItem = document.createElement('span');
            bitItem.className = 'info-item';
            bitItem.textContent = meta.bitDepth + '-bit';
            details.appendChild(bitItem);
        }
        
        // Frame count
        if (meta.frameCount > 1) {
            details.appendChild(HEIFUtils.createSeparator());
            var frameItem = document.createElement('span');
            frameItem.className = 'info-item';
            frameItem.textContent = meta.frameCount + ' frames';
            details.appendChild(frameItem);
        }
        
        // File size
        if (fileSize) {
            details.appendChild(HEIFUtils.createSeparator());
            var sizeItem = document.createElement('span');
            sizeItem.className = 'info-item';
            sizeItem.textContent = HEIFUtils.formatFileSize(fileSize);
            details.appendChild(sizeItem);
        }
        
        // HDR badge (clickable - toggles native HDR rendering in modal)
        if (self._isHDRContent(meta.color)) {
            details.appendChild(HEIFUtils.createSeparator());
            var hdrBadge = document.createElement('span');
            hdrBadge.className = 'info-item info-hdr-badge';
            hdrBadge.id = 'gallery-hdr-toggle';
            hdrBadge.textContent = 'HDR';
            hdrBadge.title = 'Toggle native HDR rendering (browser must support format)';
            hdrBadge.onclick = function(e) {
                e.stopPropagation();
                self._dismissHDRTooltip();
                self._toggleNativeHDRView(item, meta);
            };
            // Set active state if we're currently in native view
            var nativeContainer = document.getElementById('gallery-native-hdr-container');
            if (nativeContainer) {
                hdrBadge.classList.add('active');
            }
            details.appendChild(hdrBadge);
            
            // Show HDR tooltip bubble (only once per session)
            self._showHDRTooltip(hdrBadge);
        }
        
        // 10-bit native view badge (shown for >8-bit content that isn't already HDR-badged)
        if (meta.bitDepth && meta.bitDepth > 8 && !self._isHDRContent(meta.color)) {
            details.appendChild(HEIFUtils.createSeparator());
            var deepBadge = document.createElement('span');
            deepBadge.className = 'info-item info-hdr-badge';
            deepBadge.id = 'gallery-deepcolor-toggle';
            deepBadge.textContent = meta.bitDepth + '-bit';
            deepBadge.title = 'View native ' + meta.bitDepth + '-bit rendering (browser must support format)';
            deepBadge.onclick = function(e) {
                e.stopPropagation();
                self._toggleNativeHDRView(item, meta, 'deepcolor');
            };
            // Set active state if we're currently in native view
            var nativeContainer = document.getElementById('gallery-native-hdr-container');
            if (nativeContainer && self._nativeViewSource === 'deepcolor') {
                deepBadge.classList.add('active');
            }
            details.appendChild(deepBadge);
        }
        
        // Compare button - shown when in native HDR view or compare view
        var isInNativeView = document.getElementById('gallery-native-hdr-container') || document.getElementById('gallery-compare-container');
        if (isInNativeView && (self._isHDRContent(meta.color) || (meta.bitDepth && meta.bitDepth > 8))) {
            details.appendChild(HEIFUtils.createSeparator());
            var compareBadge = document.createElement('span');
            compareBadge.className = 'info-item info-hdr-badge';
            compareBadge.id = 'gallery-compare-toggle';
            compareBadge.textContent = 'Compare';
            compareBadge.title = 'Side-by-side: decoded canvas vs native browser rendering';
            compareBadge.onclick = function(e) {
                e.stopPropagation();
                self._toggleCompareView(item, meta);
            };
            var compareContainer = document.getElementById('gallery-compare-container');
            if (compareContainer) {
                compareBadge.classList.add('active');
            }
            details.appendChild(compareBadge);
        }
        
        container.appendChild(details);
        
        // Back button (shown when in native view or compare mode) - positioned to the right of info panel
        var nativeContainerCheck = document.getElementById('gallery-native-hdr-container') || document.getElementById('gallery-compare-container');
        if (nativeContainerCheck) {
            var backBtn = document.createElement('div');
            backBtn.className = 'gallery-modal-back-btn';
            backBtn.id = 'gallery-modal-back-btn';
            backBtn.textContent = 'Back';
            backBtn.title = 'Return to decoded view';
            backBtn.onclick = function(e) {
                e.stopPropagation();
                var hdrBadge = document.getElementById('gallery-hdr-toggle');
                if (hdrBadge) hdrBadge.classList.remove('active');
                var deepBadge = document.getElementById('gallery-deepcolor-toggle');
                if (deepBadge) deepBadge.classList.remove('active');
                var compareBadge = document.getElementById('gallery-compare-toggle');
                if (compareBadge) compareBadge.classList.remove('active');
                self._nativeViewItem = null;
                self._nativeViewSource = null;
                self._openModal(item);
            };
            container.appendChild(backBtn);
        }
        
        return container;
    };

    /**
     * Build full info panel
     */
    this._buildFullInfoPanel = function(item, meta, fileInfo) {
        var container = document.createElement('div');
        container.className = 'gallery-modal-full-info';
        container.id = 'gallery-modal-full-info';
        
        var title = document.createElement('h4');
        title.textContent = 'File Details';
        container.appendChild(title);
        
        var grid = document.createElement('div');
        grid.className = 'gallery-modal-full-info-grid';
        
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
        
        addRow('Filename', item.heifPath.split('/').pop());
        // Credit line for content with known authorship
        if (item.heifPath && item.heifPath.indexOf('content/personal/') === 0) {
            addRow('Author', 'Otto Palmgren');
        }
        addRow('Type', item.type);
        if (meta.width && meta.height) addRow('Dimensions', meta.width + ' × ' + meta.height + ' px');
        if (meta.bitDepth) addRow('Bit Depth', meta.bitDepth + ' bits/channel');
        if (meta.rotation) addRow('Rotation', meta.rotation + '°');
        if (meta.mirror) {
            addRow('Mirror', meta.mirror.horizontalAxis ? 'Vertical (axis=0)' : 'Horizontal (axis=1)');
        }
        if (meta.cleanAperture) {
            var clap = meta.cleanAperture;
            var clapWidth = (clap.widthN / clap.widthD).toFixed(1);
            var clapHeight = (clap.heightN / clap.heightD).toFixed(1);
            addRow('Clean Aperture', clapWidth + ' × ' + clapHeight);
        }
        if (meta.scale) {
            var s = meta.scale;
            var scaleX = (s.targetWidthN / s.targetWidthD).toFixed(2);
            var scaleY = (s.targetHeightN / s.targetHeightD).toFixed(2);
            addRow('Scale', scaleX + '× / ' + scaleY + '×');
        }
        if (meta.layerSelection) {
            addRow('Layer ID', meta.layerSelection.layerId);
        }
        if (meta.frameCount > 1) addRow('Frames', meta.frameCount);
        if (meta.fps) addRow('Frame Rate', meta.fps.toFixed(2) + ' fps');
        if (meta.isLooping) addRow('Playback', 'Looping');
        if (meta.timeScale) addRow('Time Scale', meta.timeScale);
        if (meta.hasThumbnail) addRow('Thumbnail', meta.thumbWidth + ' × ' + meta.thumbHeight + ' px');
        if (meta.hasAlpha) addRow('Alpha Channel', 'Yes');
        if (meta.brands && meta.brands.length > 0) addRow('Brands', meta.brands.join(', '));
        
        // File features
        if (fileInfo && fileInfo.fileFeature) {
            var features = [];
            if (fileInfo.fileFeature.hasSingleImage) features.push('Single Image');
            if (fileInfo.fileFeature.hasImageCollection) features.push('Collection');
            if (fileInfo.fileFeature.hasImageSequence) features.push('Sequence');
            if (fileInfo.fileFeature.hasCoverImage) features.push('Cover Image');
            if (features.length > 0) addRow('Features', features.join(', '));
        }
        
        // Color information
        if (meta.color) {
            var colorInfo = meta.color;
            if (colorInfo.colourType) {
                // Expand the four-char ISOBMFF colour-type code into a
                // user-readable name. Showing the bare code ('nclx',
                // 'rICC', 'prof') leaks an internal identifier with no
                // meaning to anyone who hasn't read ITU-T H.273.
                var colourTypeLabels = {
                    nclx: 'NCLX (coding-independent code points)',
                    rICC: 'Restricted ICC profile',
                    prof: 'ICC profile'
                };
                var ctLabel = colourTypeLabels[colorInfo.colourType] ||
                              colorInfo.colourType.toUpperCase();
                addRow('Color Type', ctLabel);
            }
            if (colorInfo.colourPrimaries !== undefined) {
                // NCLX colour_primaries (ITU-T H.273 Table 2). Note that
                // primary 1 is shared between BT.709 and sRGB - they have
                // the same chromaticities and white point, but different
                // transfer functions. The transfer function is shown on
                // its own row, so we don't tack '(sRGB)' onto the primary
                // name here (it would be misleading when the transfer
                // is PQ/HLG/etc.).
                var primariesNames = {
                    1: 'BT.709 / sRGB',
                    5: 'BT.601 (625)',
                    6: 'BT.601 (525)',
                    9: 'BT.2020 / BT.2100',
                    11: 'DCI-P3',
                    12: 'Display P3'
                };
                var primName = primariesNames[colorInfo.colourPrimaries] || colorInfo.colourPrimaries;
                addRow('Color Primaries', primName);
            }
            if (colorInfo.transferCharacteristics !== undefined) {
                var transferNames = {
                    1: 'BT.709',
                    13: 'sRGB',
                    16: 'PQ (HDR)',
                    18: 'HLG (HDR)'
                };
                var transferName = transferNames[colorInfo.transferCharacteristics] || colorInfo.transferCharacteristics;
                addRow('Transfer Function', transferName);
            }
            if (colorInfo.matrixCoefficients !== undefined) {
                var matrixNames = {
                    1: 'BT.709',
                    5: 'BT.601 (525)',
                    6: 'BT.601 (625)',
                    9: 'BT.2020'
                };
                var matrixName = matrixNames[colorInfo.matrixCoefficients] || colorInfo.matrixCoefficients;
                addRow('Matrix Coefficients', matrixName);
            }
            if (colorInfo.fullRangeFlag !== undefined) {
                addRow('Range', colorInfo.fullRangeFlag ? 'Full (0-255)' : 'Limited (16-235)');
            }
            // ICC Profile info
            if (colorInfo.iccProfile && typeof HEIFUtils !== 'undefined' && HEIFUtils.parseICCProfile) {
                try {
                    var bytes = colorInfo.iccProfile instanceof Uint8Array ? 
                        colorInfo.iccProfile : new Uint8Array(colorInfo.iccProfile);
                    var parsed = HEIFUtils.parseICCProfile(bytes);
                    if (parsed.valid) {
                        if (parsed.description) addRow('ICC Profile', parsed.description);
                        if (parsed.identified && parsed.identified.name) {
                            addRow('Identified As', parsed.identified.name);
                        }
                    }
                } catch (e) { /* ignore */ }
            }
        }
        
        container.appendChild(grid);
        
        container.addEventListener('click', function(e) {
            e.stopPropagation();
        });
        
        return container;
    };

    /**
     * Toggle full info panel visibility
     */
    this._toggleFullInfo = function() {
        var fullInfo = document.getElementById('gallery-modal-full-info');
        if (fullInfo) {
            var isVisible = fullInfo.classList.toggle('visible');
            
            if (isVisible) {
                setTimeout(function() {
                    document.addEventListener('click', self._handleClickOutsideFullInfo);
                }, 10);
            } else {
                document.removeEventListener('click', self._handleClickOutsideFullInfo);
            }
        }
    };

    /**
     * Handle click outside full info panel
     */
    this._handleClickOutsideFullInfo = function(e) {
        var fullInfo = document.getElementById('gallery-modal-full-info');
        var infoBtn = document.querySelector('.gallery-modal-info-btn');
        
        if (fullInfo && !fullInfo.contains(e.target) && (!infoBtn || !infoBtn.contains(e.target))) {
            fullInfo.classList.remove('visible');
            document.removeEventListener('click', self._handleClickOutsideFullInfo);
        }
    };

    /**
     * Show error message in modal
     */
    this._showModalError = function(content, message) {
        var loading = document.getElementById('gallery-modal-loading');
        if (loading) loading.classList.add('hidden');
        
        var errorEl = document.createElement('div');
        errorEl.className = 'gallery-modal-error';
        errorEl.style.cssText = 'display:flex; flex-direction:column; align-items:center; justify-content:center; height:100%; min-height:200px; gap:6px;';

        // Split on newlines so callers can pass multi-line messages
        // (e.g. per-item error details for altr groups).
        var lines = message.split('\n');
        for (var li = 0; li < lines.length; li++) {
            var span = document.createElement('span');
            span.style.cssText = li === 0
                ? 'color:rgba(255,255,255,0.75); font-size:14px; font-weight:600; letter-spacing:0.3px;'
                : 'color:rgba(255,255,255,0.5); font-size:12px;';
            span.textContent = lines[li];
            errorEl.appendChild(span);
        }

        content.appendChild(errorEl);

        // Attach info panels even on error so the user can still
        // inspect file metadata (dimensions, codec, type, etc.).
        var item = self._modalCurrentItem;
        if (item && item.meta) {
            self._addInfoPanels(item, item.meta);
        }
    };
    
    /**
     * Check if content uses HDR transfer function (PQ or HLG) or otherwise
     * carries HDR markers. Thin wrapper around HEIFUtils.isHDRContent so
     * gallery.js, decoder-manager.js and any future consumer all agree on
     * the same definition of "HDR".
     *
     * The previous in-line implementation here used a looser ICC-name
     * substring match that fired on any profile description containing
     * 'hdr' (including non-HDR samples named e.g. "Adobe RGB HDR ramp").
     * The shared helper requires either an HDR transfer characteristic
     * (PQ=16 / HLG=18), BT.2020 + 10-bit + non-SDR transfer, or a curated
     * ICC name needle - see HEIFUtils.isHDRContent for the full rules.
     *
     * @param {Object} colorInfo - Color information from metadata
     * @returns {boolean} True if content is HDR
     */
    this._isHDRContent = function(colorInfo) {
        if (typeof HEIFUtils === 'undefined' || !HEIFUtils.isHDRContent) return false;
        var bitDepth = (self._modalCurrentItem && self._modalCurrentItem.meta)
            ? self._modalCurrentItem.meta.bitDepth : null;
        return HEIFUtils.isHDRContent(colorInfo, bitDepth);
    };
    
    /**
     * Show HDR tooltip bubble pointing to the HDR badge
     * Only shows once per page load (uses session flag)
     * @param {HTMLElement} hdrBadge - The HDR badge element to point to
     */
    this._showHDRTooltip = function(hdrBadge) {
        // Check if tooltip was already dismissed this page load
        if (self._hdrTooltipDismissed) {
            return;
        }
        
        // Don't show if already in native HDR view
        var nativeContainer = document.getElementById('gallery-native-hdr-container');
        if (nativeContainer) return;
        
        // Remove any existing tooltip
        self._dismissHDRTooltip();
        
        // Create tooltip bubble
        var tooltip = document.createElement('div');
        tooltip.className = 'hdr-tooltip-bubble';
        tooltip.id = 'hdr-tooltip-bubble';
        
        // Close button
        var closeBtn = document.createElement('button');
        closeBtn.className = 'hdr-tooltip-close';
        closeBtn.innerHTML = '×';
        closeBtn.onclick = function(e) {
            e.stopPropagation();
            self._dismissHDRTooltip();
        };
        tooltip.appendChild(closeBtn);
        
        // Tooltip text
        var text = document.createTextNode('Click tag to view in HDR');
        tooltip.appendChild(text);
        
        // Click on tooltip dismisses it
        tooltip.onclick = function(e) {
            e.stopPropagation();
            self._dismissHDRTooltip();
        };
        
        // Append to badge (badge is position:relative, tooltip is position:absolute)
        hdrBadge.appendChild(tooltip);
        
        // Show with slight delay for smooth animation
        setTimeout(function() {
            tooltip.classList.add('visible');
        }, 100);
    };
    
    /**
     * Dismiss the HDR tooltip and remember the dismissal for this page load
     */
    this._dismissHDRTooltip = function() {
        var tooltip = document.getElementById('hdr-tooltip-bubble');
        if (tooltip) {
            tooltip.classList.remove('visible');
            setTimeout(function() {
                if (tooltip.parentNode) {
                    tooltip.parentNode.removeChild(tooltip);
                }
            }, 300);
        }
        
        // Remember dismissal for this page load
        self._hdrTooltipDismissed = true;
    };
    
    /**
     * Toggle between canvas-rendered view and native browser HDR rendering
     * Attempts to display the image using <img> element for native HDR support
     * Falls back to error state with open/download options if browser can't render
     * @param {Object} item - Gallery item with heifPath
     * @param {Object} meta - Image metadata
     */
    this._toggleNativeHDRView = function(item, meta, source) {
        source = source || 'hdr';
        var content = document.getElementById('gallery-modal-content');
        var hdrBadge = document.getElementById('gallery-hdr-toggle');
        if (!content || !item || !item.heifPath) return;
        
        // Check if we're currently showing native view or compare view
        var existingNativeView = document.getElementById('gallery-native-hdr-container');
        var existingCompare = document.getElementById('gallery-compare-container');
        
        if (existingCompare) {
            // From compare view, HDR button returns to native HDR view
            existingCompare.remove();
            var compareBadge = document.getElementById('gallery-compare-toggle');
            if (compareBadge) compareBadge.classList.remove('active');
            // Fall through to show native view below
        } else if (existingNativeView) {
            // From native view, HDR button returns to canvas view
            if (hdrBadge) {
                hdrBadge.classList.remove('active');
            }
            var deepBadge = document.getElementById('gallery-deepcolor-toggle');
            if (deepBadge) deepBadge.classList.remove('active');
            self._nativeViewItem = null;
            self._nativeViewSource = null;
            self._openModal(item);
            return;
        }
        
        // Switch to native HDR view
        if (hdrBadge) {
            hdrBadge.classList.add('active');
        }
        
        // Get absolute URL for the image
        var fullPath = item.heifPath;
        if (!fullPath.startsWith('http') && !fullPath.startsWith('//')) {
            var base = window.location.href.replace(/\/[^\/]*$/, '/');
            fullPath = base + fullPath;
        }
        
        // Create container for native view
        var nativeContainer = document.createElement('div');
        nativeContainer.id = 'gallery-native-hdr-container';
        nativeContainer.className = 'gallery-native-hdr-container';
        
        // Create native img element
        var nativeImg = document.createElement('img');
        nativeImg.style.cssText = 'max-width:calc(100% - 80px);max-height:calc(100vh - 180px);object-fit:contain;';
        nativeImg.alt = item.title || 'HDR Image';
        
        // Show loading state
        var loadingDiv = document.createElement('div');
        loadingDiv.className = 'gallery-modal-loading';
        loadingDiv.innerHTML = '<div class="gallery-spinner"></div><span>Loading native HDR view...</span>';
        nativeContainer.appendChild(loadingDiv);
        
        // Handle successful load
        nativeImg.onload = function() {
            loadingDiv.remove();
            nativeContainer.insertBefore(nativeImg, nativeContainer.firstChild);
            
            // Add status text below image
            var statusText = document.createElement('div');
            statusText.className = 'gallery-native-view-status';
            statusText.textContent = 'Native browser rendering';
            nativeContainer.appendChild(statusText);
            
            // Mark that we're in native view mode and rebuild info panel to show back button
            self._nativeViewItem = item;
            self._nativeViewSource = source;
            self._addInfoPanels(item, meta);
        };
        
        // Handle failed load - show fallback options
        nativeImg.onerror = function() {
            loadingDiv.remove();
            self._showNativeFallback(nativeContainer, item, fullPath, source);
            self._nativeViewItem = item;
            self._nativeViewSource = source;
            self._addInfoPanels(item, meta);
        };
        
        // Clear content and add native container
        // Keep the canvas hidden rather than removing, in case we toggle back
        var existingContent = content.querySelectorAll('canvas, .heif-transformed-wrapper, .gallery-no-decoder-placeholder');
        existingContent.forEach(function(el) {
            el.style.display = 'none';
        });
        
        content.appendChild(nativeContainer);
        
        // Start loading
        nativeImg.src = fullPath;
    };

    /**
     * Toggle side-by-side comparison: decoded canvas on left, native <img> on right.
     * Re-uses the same native-load pipeline as _toggleNativeHDRView.
     * @param {Object} item - Gallery item with heifPath
     * @param {Object} meta - Image metadata
     */
    this._toggleCompareView = function(item, meta) {
        var content = document.getElementById('gallery-modal-content');
        if (!content || !item || !item.heifPath) return;

        // If already in compare mode, toggle back to native HDR view
        var existing = document.getElementById('gallery-compare-container');
        if (existing) {
            existing.remove();
            // Restore original source ('hdr' or 'deepcolor'), not 'compare'
            var originalSource = (self._nativeViewSource === 'compare')
                ? (self._nativeViewSourceOriginal || 'hdr')
                : (self._nativeViewSource || 'hdr');
            self._toggleNativeHDRView(item, meta, originalSource);
            return;
        }

        // Also remove any active native-only view
        var existingNative = document.getElementById('gallery-native-hdr-container');
        if (existingNative) existingNative.remove();

        // Grab the existing canvas/wrapper before we rearrange
        var canvasEl = content.querySelector('canvas, .heif-transformed-wrapper');
        if (!canvasEl) {
            // No decoded content to compare against
            return;
        }

        // Build absolute URL
        var fullPath = item.heifPath;
        if (!fullPath.startsWith('http') && !fullPath.startsWith('//')) {
            var base = window.location.href.replace(/\/[^\/]*$/, '/');
            fullPath = base + fullPath;
        }

        // Build compare container
        var compareContainer = document.createElement('div');
        compareContainer.id = 'gallery-compare-container';
        compareContainer.className = 'gallery-compare-container';

        // Left pane: decoded canvas
        var leftPane = document.createElement('div');
        leftPane.className = 'gallery-compare-pane';
        var leftLabel = document.createElement('div');
        leftLabel.className = 'gallery-compare-label';
        leftLabel.textContent = 'Decoded (canvas)';
        leftPane.appendChild(canvasEl);
        canvasEl.style.display = '';
        canvasEl.style.maxWidth = '100%';
        canvasEl.style.maxHeight = 'calc(100vh - 220px)';
        canvasEl.style.objectFit = 'contain';
        leftPane.appendChild(leftLabel);
        compareContainer.appendChild(leftPane);

        // Divider
        var divider = document.createElement('div');
        divider.className = 'gallery-compare-divider';
        compareContainer.appendChild(divider);

        // Right pane: native img
        var rightPane = document.createElement('div');
        rightPane.className = 'gallery-compare-pane';
        var rightLabel = document.createElement('div');
        rightLabel.className = 'gallery-compare-label';
        rightLabel.textContent = 'Native (browser)';

        var nativeImg = document.createElement('img');
        nativeImg.alt = item.title || 'Native render';
        nativeImg.style.cssText = 'max-width:100%;max-height:calc(100vh - 220px);object-fit:contain;';

        var spinner = document.createElement('div');
        spinner.className = 'gallery-spinner';
        rightPane.appendChild(spinner);

        nativeImg.onload = function() {
            spinner.remove();
            rightPane.insertBefore(nativeImg, rightLabel);
            rightLabel.textContent = 'Native (browser)';
            self._nativeViewItem = item;
            self._nativeViewSourceOriginal = self._nativeViewSource || 'hdr';
            self._nativeViewSource = 'compare';
            self._addInfoPanels(item, meta);
        };
        nativeImg.onerror = function() {
            spinner.remove();
            rightLabel.textContent = 'Native: unsupported by browser';
            rightPane.style.opacity = '0.5';
            self._nativeViewSourceOriginal = self._nativeViewSource || 'hdr';
            self._nativeViewSource = 'compare';
            self._addInfoPanels(item, meta);
        };

        rightPane.appendChild(rightLabel);
        compareContainer.appendChild(rightPane);

        // Replace modal content
        content.replaceChildren();
        content.appendChild(compareContainer);

        nativeImg.src = fullPath;

        self._addInfoPanels(item, meta);
    };
    
    /**
     * Show fallback UI when browser can't render the image natively
     * @param {HTMLElement} container - Container element
     * @param {Object} item - Gallery item
     * @param {string} fullPath - Full URL to the image
     * @param {string} source - 'hdr' or 'nodecoder' to determine context
     */
    this._showNativeFallback = function(container, item, fullPath, source) {
        container.replaceChildren();
        
        // Error panel
        var errorDiv = document.createElement('div');
        errorDiv.className = 'gallery-native-fallback';
        
        // Title row with info button
        var titleRow = document.createElement('div');
        titleRow.className = 'gallery-native-fallback-title-row';
        
        var title = document.createElement('div');
        title.className = 'gallery-native-fallback-title';
        title.textContent = 'Native Rendering Not Available';
        titleRow.appendChild(title);
        
        // Info button - styled as clickable text
        var infoBtn = document.createElement('button');
        infoBtn.className = 'gallery-native-fallback-info-btn';
        infoBtn.textContent = 'Why?';
        infoBtn.title = 'Learn more about this';
        infoBtn.onclick = function(e) {
            e.preventDefault();
            e.stopPropagation();
            self._showNativeFallbackInfoModal(source);
        };
        titleRow.appendChild(infoBtn);
        
        errorDiv.appendChild(titleRow);
        
        // Brief description
        var desc = document.createElement('div');
        desc.className = 'gallery-native-fallback-desc';
        desc.textContent = 'Your browser cannot display this image natively.';
        errorDiv.appendChild(desc);
        
        // Action buttons
        var btnContainer = document.createElement('div');
        btnContainer.className = 'gallery-native-fallback-buttons';
        
        // Download button - use anchor element to avoid white flash
        var downloadBtn = document.createElement('a');
        downloadBtn.className = 'gallery-burst-btn gallery-native-hdr-btn-primary';
        downloadBtn.textContent = 'Download';
        downloadBtn.title = 'Download the original file';
        downloadBtn.href = fullPath;
        downloadBtn.download = item.title || 'image';
        downloadBtn.onclick = function(e) {
            e.stopPropagation();
        };
        btnContainer.appendChild(downloadBtn);
        
        errorDiv.appendChild(btnContainer);
        
        // Red back button (same style as native view success)
        var backBtn = document.createElement('div');
        backBtn.className = 'gallery-native-fallback-back-btn';
        backBtn.textContent = 'Back';
        backBtn.title = 'Return to previous view';
        backBtn.onclick = function(e) {
            e.stopPropagation();
            var hdrBadge = document.getElementById('gallery-hdr-toggle');
            if (hdrBadge) hdrBadge.classList.remove('active');
            self._nativeViewItem = null;
            self._nativeViewSource = null;
            self._openModal(item);
        };
        errorDiv.appendChild(backBtn);
        
        container.appendChild(errorDiv);
    };
    
    /**
     * Show informational modal explaining why native rendering failed
     * @param {string} source - 'hdr' or 'nodecoder'
     */
    this._showNativeFallbackInfoModal = function(source) {
        // Remove existing modal if any
        var existing = document.getElementById('gallery-info-modal');
        if (existing) existing.remove();
        
        var overlay = document.createElement('div');
        overlay.id = 'gallery-info-modal';
        overlay.className = 'gallery-info-modal-overlay';
        
        var modal = document.createElement('div');
        modal.className = 'gallery-info-modal';
        
        var closeBtn = document.createElement('button');
        closeBtn.className = 'gallery-info-modal-close';
        closeBtn.innerHTML = '&times;';
        closeBtn.onclick = function() { overlay.remove(); };
        
        var title = document.createElement('h3');
        title.className = 'gallery-info-modal-title';
        
        var content = document.createElement('div');
        content.className = 'gallery-info-modal-content';
        
        if (source !== 'nodecoder') {
            title.textContent = 'About HDR Image Support';
            content.innerHTML = 
                '<p><strong>What is HDR?</strong></p>' +
                '<p>HDR (High Dynamic Range) images use WCG (Wide colour gamut) color profiles that can present colors outside the traditional sRGB range. These colours profiles are not yet supported in HTML canvas elements that this implementation uses.</p>' +
                '<p><strong>Why can\'t my browser display this?</strong></p>' +
                '<p>Native HDR image rendering requires:</p>' +
                '<ul>' +
                '<li>An image file format that supports HDR (HEIF, AVIF, JPEG XL)</li>' +
                '<li>A browser that supports that format (Has built-in decoder for the encoded image)</li>' +
                '<li>An HDR-capable display</li>' +
                '</ul>' +
                '<p><strong>Current support:</strong></p>' +
                '<ul>' +
                '<li><strong>Safari (macOS/iOS):</strong>' +
                '<ul>' +
                '<li>Native HEIF browser support in Safari</li>' +
                '<li>AVIF support</li>' +
                '<li>Most modern Macs and iPhones have HDR displays</li>' +
                '<li>Experimental JPEG XL support</li>' +
                '</ul></li>' +
                '<li><strong>Chrome/Edge:</strong>' +
                '<ul>' +
                '<li>AVIF support</li>' +
                '<li>No HEIF support (HEVC supported if device has hardware decoder)<details><summary>But?</summary><i>That hardware decoder needs an active license :D. Some manufacturers have been cancelling their h265 support on machines with already built in decoders to avoid licensing fees, actively preventing usage of the hardware decoding. There is endless amounts of discussion and anger about the HEVC licensing issues online.</i></details></li>' + 
                '<li>Experimental JPEG XL support</li>' +
                '</ul></li>' +
                '<li><strong>Firefox:</strong>' +
                '<ul>' + 
                '<li>AVIF support</li>' +
                '<li>No HEIF support, HEVC supported for devices with hardware support, software fallback in some cases</li>' +
                '<li>Experimental JPEG XL support</li>' +
                '</ul></li>' +
                '</ul>' +
                '<p><strong>What was the image before?</strong></p>' +
                '<p>The decoded view shown earlier uses WebAssembly version of decoder (libde265 or vvdec) which works in all browsers. This presentation is limited in the colour gamut, as it uses a canvas element which supports P3 colorspace at max. For what it is worth YUV to RGBA conversion was done with adjusted conversion matrices, retaining more visuality of the original image.</p>' +
                '<p><strong>TLDR;</strong></p>' +
                '<p> Support for HDR is limited and working implementation here would\'ve required too much effort</p>';
        } else {
            title.textContent = 'About Codec Support';
            content.innerHTML = 
                '<p><strong>What happened?</strong></p>' +
                '<p>This HEIF image uses a codec that is not included in this web demo. Currently only libde265 and vvdec have implementations, as these are the most common HEIF codecs <b>that do not have native decoding support</b>.</p>' +
                '<p><strong>Why does this happen?</strong></p>' +
                '<p>HEIF is a container format that can hold images encoded with different codecs:</p>' +
                '<ul>' +
                '<li><strong>HEVC (H.265):</strong> A.K.A HEIC, the most common HEIF files, as it is the original codec used in the standardation process and is used across the Apple ecosystem.</li>' +
                '<li><strong>AV1:</strong> Used in AVIF files, which is a derivative of the HEIF standard. AVIF(AV1) decoding support has became a standard feature on most platforms and browsers.</li>' +
                '<li><strong>VVC (H.266):</strong> Latest video codec supported by the HEIF standard. Has practically no native support, so this demo website is a rare treat!</li>' +
                '</ul>' +
                '<p><strong>What can I do?</strong></p>' +
                '<ul>' +
                '<li>Try "Native Rendering" - your browser may support the format directly</li>' +
                '<li>Download the file and open with a compatible image viewer</li>' +
                '<li>Use Safari on macOS/iOS for best HEIF compatibility</li>' +
                '</ul>';
        }
        
        // Add scroll indicator at top (before title)
        var topIndicator = document.createElement('div');
        topIndicator.className = 'gallery-info-modal-scroll-indicator top';
        topIndicator.innerHTML = '<svg viewBox="0 0 24 24"><path d="M7.41 15.41L12 10.83l4.59 4.58L18 14l-6-6-6 6z"/></svg>';
        
        modal.appendChild(closeBtn);
        modal.appendChild(topIndicator);
        modal.appendChild(title);
        modal.appendChild(content);
        
        // Add scroll indicator at bottom (after content)
        var bottomIndicator = document.createElement('div');
        bottomIndicator.className = 'gallery-info-modal-scroll-indicator bottom';
        bottomIndicator.innerHTML = '<svg viewBox="0 0 24 24"><path d="M7.41 8.59L12 13.17l4.59-4.58L18 10l-6 6-6-6z"/></svg>';
        
        modal.appendChild(bottomIndicator);
        
        // Update scroll indicators based on scroll position
        function updateScrollIndicators() {
            var scrollTop = modal.scrollTop;
            var scrollHeight = modal.scrollHeight;
            var clientHeight = modal.clientHeight;
            var scrollBottom = scrollHeight - scrollTop - clientHeight;
            
            // Show top indicator if scrolled down
            if (scrollTop > 10) {
                topIndicator.classList.add('visible');
            } else {
                topIndicator.classList.remove('visible');
            }
            
            // Show bottom indicator if more content below
            if (scrollBottom > 10) {
                bottomIndicator.classList.add('visible');
            } else {
                bottomIndicator.classList.remove('visible');
            }
        }
        
        modal.addEventListener('scroll', updateScrollIndicators);
        
        overlay.appendChild(modal);
        
        // Close on overlay click
        overlay.onclick = function(e) {
            if (e.target === overlay) overlay.remove();
        };
        
        document.body.appendChild(overlay);
        
        // Initial check after DOM is ready
        setTimeout(updateScrollIndicators, 50);
    };

    /**
     * Open the original HEIF file in a new tab for native browser HDR viewing.
     * Browsers with HEIF + HDR support (Safari, iOS) can display the file
     * with native tone mapping, which our software decoder cannot replicate.
     * @param {Object} item - Gallery item with heifPath
     */
    this._openForHDRViewing = function(item) {
        if (!item || !item.heifPath) {
            console.warn('No file path available for HDR viewing');
            return;
        }
        
        var fullPath = item.heifPath;
        if (!fullPath.startsWith('http') && !fullPath.startsWith('//')) {
            var base = window.location.href.replace(/\/[^\/]*$/, '/');
            fullPath = base + fullPath;
        }
        
        window.open(fullPath, '_blank');
    };
    
    /**
     * Show a brief toast notification
     * @param {string} message - Main message
     * @param {string} submessage - Secondary message (optional)
     */
    this._showToast = function(message, submessage) {
        var existingToast = document.getElementById('gallery-toast');
        if (existingToast) existingToast.remove();
        
        var toast = document.createElement('div');
        toast.id = 'gallery-toast';
        toast.innerHTML = '<strong>' + message + '</strong>' + 
            (submessage ? '<br><span style="font-size:12px;opacity:0.8;">' + submessage + '</span>' : '');
        toast.style.cssText = 
            'position: fixed; bottom: 120px; left: 50%; transform: translateX(-50%); ' +
            'background: rgba(0,0,0,0.85); color: #fff; padding: 12px 20px; ' +
            'border-radius: 8px; font-size: 14px; z-index: 10000; text-align: center; ' +
            'backdrop-filter: blur(10px); border: 1px solid rgba(255,255,255,0.1); ' +
            'animation: fadeInUp 0.3s ease;';
        
        document.body.appendChild(toast);
        
        setTimeout(function() {
            if (toast.parentNode) {
                toast.style.opacity = '0';
                toast.style.transform = 'translateX(-50%) translateY(10px)';
                toast.style.transition = 'opacity 0.3s, transform 0.3s';
                setTimeout(function() {
                    if (toast.parentNode) toast.remove();
                }, 300);
            }
        }, 3000);
    };
    
    /**
     * Show toast notification when opening for native HDR viewing.
     */
    this._showHDRViewingToast = function() {
        this._showToast('Opening for HDR viewing', 'Requires browser with HEIF + HDR support (Safari, iOS)');
    };
    
    /**
     * Show "no decoder" placeholder with metadata info panels
     * Used when image codec is not supported but metadata was parsed successfully
     */
    this._showNoDecoderPlaceholder = function(content, item, meta, result) {
        var loading = document.getElementById('gallery-modal-loading');
        if (loading) loading.classList.add('hidden');
        
        // Create placeholder container with class for proper toggle handling
        var placeholder = document.createElement('div');
        placeholder.className = 'gallery-no-decoder-placeholder';
        
        // Title row with info button
        var titleRow = document.createElement('div');
        titleRow.className = 'gallery-no-decoder-title-row';
        
        // Error message
        var errorMsg = document.createElement('div');
        errorMsg.className = 'gallery-no-decoder-title';
        errorMsg.textContent = 'No decoder available';
        titleRow.appendChild(errorMsg);
        
        // Info button - styled as clickable text
        var infoBtn = document.createElement('button');
        infoBtn.className = 'gallery-no-decoder-info-btn';
        infoBtn.textContent = 'Why?';
        infoBtn.title = 'Learn more about this';
        infoBtn.onclick = function(e) {
            e.preventDefault();
            e.stopPropagation();
            self._showNativeFallbackInfoModal('nodecoder');
        };
        titleRow.appendChild(infoBtn);
        
        placeholder.appendChild(titleRow);
        
        // Details
        var details = document.createElement('div');
        details.className = 'gallery-no-decoder-desc';
        details.textContent = result.error || 'This codec is not supported for decoding.';
        placeholder.appendChild(details);
        
        // Codec info if available
        if (result.heifInfo && result.heifInfo.items) {
            var itemIds = Object.keys(result.heifInfo.items);
            if (itemIds.length > 0) {
                var firstItem = result.heifInfo.items[itemIds[0]];
                if (firstItem && firstItem.codec) {
                    var codecInfo = document.createElement('div');
                    codecInfo.className = 'gallery-no-decoder-codec';
                    codecInfo.textContent = 'Codec: ' + firstItem.codec;
                    placeholder.appendChild(codecInfo);
                }
            }
        }
        
        // Native rendering button
        var btnContainer = document.createElement('div');
        btnContainer.className = 'gallery-no-decoder-buttons';
        
        var nativeBtn = document.createElement('button');
        nativeBtn.className = 'gallery-burst-btn gallery-native-hdr-btn-primary';
        nativeBtn.textContent = 'View native';
        nativeBtn.title = 'Try to view image file as an "img" HTML element.';
        nativeBtn.onclick = function(e) {
            e.preventDefault();
            e.stopPropagation();
            self._tryNativeRendering(item, content);
        };
        btnContainer.appendChild(nativeBtn);
        
        placeholder.appendChild(btnContainer);
        
        // Metadata available notice
        var metaNotice = document.createElement('div');
        metaNotice.className = 'gallery-no-decoder-meta-notice';
        metaNotice.innerHTML = 'Metadata from HEIF available - click <span class="gallery-modal-info-btn" title="Not this one! The one below in the info section!">i</span> button below';
        placeholder.appendChild(metaNotice);
        
        content.appendChild(placeholder);
        
        // Still show the info panels with available metadata
        self._addInfoPanels(item, meta, result.heifInfo);
    };

    /**
     * Try native browser rendering for unsupported codec
     * @param {Object} item - Gallery item
     * @param {HTMLElement} content - Modal content element
     */
    this._tryNativeRendering = function(item, content) {
        if (!content || !item || !item.heifPath) return;
        
        // Set HDR badge to active state if it exists (visual indicator of native view)
        var hdrBadge = document.getElementById('gallery-hdr-toggle');
        if (hdrBadge) {
            hdrBadge.classList.add('active');
        }
        
        // Get absolute URL for the image
        var fullPath = item.heifPath;
        if (!fullPath.startsWith('http') && !fullPath.startsWith('//')) {
            var base = window.location.href.replace(/\/[^\/]*$/, '/');
            fullPath = base + fullPath;
        }
        
        // Create container for native view
        var nativeContainer = document.createElement('div');
        nativeContainer.id = 'gallery-native-hdr-container';
        nativeContainer.className = 'gallery-native-hdr-container';
        
        // Create native img element
        var nativeImg = document.createElement('img');
        nativeImg.style.cssText = 'max-width:calc(100% - 80px);max-height:calc(100vh - 180px);object-fit:contain;';
        nativeImg.alt = item.title || 'Image';
        
        // Show loading state
        var loadingDiv = document.createElement('div');
        loadingDiv.className = 'gallery-modal-loading';
        loadingDiv.innerHTML = '<div class="gallery-spinner"></div><span>Loading native view...</span>';
        nativeContainer.appendChild(loadingDiv);
        
        // Handle successful load
        nativeImg.onload = function() {
            loadingDiv.remove();
            nativeContainer.insertBefore(nativeImg, nativeContainer.firstChild);
            
            // Add status text below image
            var statusText = document.createElement('div');
            statusText.className = 'gallery-native-view-status';
            statusText.textContent = 'Native browser rendering';
            nativeContainer.appendChild(statusText);
            
            // Mark that we're in native view mode and rebuild info panel to show back button
            self._nativeViewItem = item;
            self._nativeViewSource = 'nodecoder';
            self._addInfoPanels(item, item.meta);
        };
        
        // Handle failed load - show fallback options
        nativeImg.onerror = function() {
            loadingDiv.remove();
            self._showNativeFallback(nativeContainer, item, fullPath, 'nodecoder');
        };
        
        // Hide existing content (including no-decoder placeholder)
        var existingContent = content.querySelectorAll('canvas, .heif-transformed-wrapper, .gallery-no-decoder-placeholder');
        existingContent.forEach(function(el) {
            el.style.display = 'none';
        });
        
        content.appendChild(nativeContainer);
        
        // Start loading
        nativeImg.src = fullPath;
    };

    /**
     * Close modal
     */
    this._closeModal = function() {
        document.removeEventListener('click', self._handleClickOutsideFullInfo);

        // Cancel any in-flight modal-open timers so they don't fire
        // after close (would re-add blur-ready to a hidden modal and,
        // worse, do it just as the next open() runs - producing a brief
        // blurred frame on the new session).
        if (self._modalBlurFallbackTimer) {
            clearTimeout(self._modalBlurFallbackTimer);
            self._modalBlurFallbackTimer = null;
        }

        var backdrop = document.getElementById('gallery-modal-backdrop');
        var modal = document.getElementById('gallery-modal-window');
        var content = document.getElementById('gallery-modal-content');

        // Capture the opener BEFORE we tear down state so we can restore
        // focus to it once the close animation has settled.
        var openerToRestore = self._modalLastOpener;
        self._modalCurrentItem = null;
        self._modalLastOpener = null;

        var infoPanel = document.getElementById('gallery-modal-info-panel');
        if (infoPanel) infoPanel.remove();
        var fullInfo = document.getElementById('gallery-modal-full-info');
        if (fullInfo) fullInfo.remove();
        
        // Dismiss HDR tooltip if visible
        var hdrTooltip = document.getElementById('hdr-tooltip-bubble');
        if (hdrTooltip) hdrTooltip.remove();
        
        // Clean up native view state
        self._nativeViewItem = null;
        self._nativeViewSource = null;
        
        // Reset HDR badge state
        var hdrBadge = document.getElementById('gallery-hdr-toggle');
        if (hdrBadge) hdrBadge.classList.remove('active');
        
        if (backdrop) backdrop.classList.remove('visible');
        if (modal) {
            modal.classList.remove('visible');
            // Drop the blur immediately on close so the scale-down doesn't
            // animate a blurred surface (same reason we deferred it on open).
            modal.classList.remove('blur-ready');
        }

        // Restore focus to the tile (or other element) that opened the
        // modal, so keyboard users land back where they were instead of
        // at the top of the page. Defensive: if the original opener was
        // detached during the modal session, fall back to body.
        if (openerToRestore && typeof openerToRestore.focus === 'function'
            && document.contains(openerToRestore)) {
            try { openerToRestore.focus({ preventScroll: true }); }
            catch (_) { openerToRestore.focus(); }
        }
        
        setTimeout(function() {
            document.body.style.overflow = '';

            // Run modal-scoped cleanups (audio pause, sequence + burst
            // timers, active reader/provider dispose). Using the shared
            // teardown helper keeps the close path and the arrow-key
            // navigation path in sync, so resources are released the
            // same way regardless of how the previous modal session ends.
            self._teardownActiveModalSession();

            if (content) {
                content.querySelectorAll('canvas, img, .gallery-modal-grid, .gallery-burst-controls, .gallery-burst-strip, .gallery-burst-main')
                    .forEach(function(el) { el.remove(); });
            }
        }, 300);
    };

    // =========================================================================
    // CLEANUP
    // =========================================================================

    window.addEventListener('beforeunload', function() {
        for (var id in self._animatedThumbnails) {
            if (self._animatedThumbnails.hasOwnProperty(id)) {
                self._animatedThumbnails[id].running = false;
            }
        }
    });

    // =========================================================================
    // AUTO-INIT
    // =========================================================================

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', function() { self.initialize(); });
    } else {
        self.initialize();
    }
}
