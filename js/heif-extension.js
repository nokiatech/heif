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

var css = document.createElement("style");css.type = "text/css";css.innerHTML = '/* This file is part of Nokia HEIF library * * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved. * * Contact: heif@nokia.com * * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved. * * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia. */.video-controls {    position: absolute;    bottom: 4px;    height: 40px;    width: 100%;    left: 0px;    right: 0px;    opacity: 0;    text-align: center;    transition: opacity 0.5s;    background: rgba(0, 0, 0, 0.60);}.video-container {    display: inline-block;    position: relative;    margin: 0;}.video-container .video-thumbnail {    position: absolute;    z-index: 1;    bottom: 44px;    border: 2px solid rgba(0, 0, 0, 0.8);    border-radius: 2px;}.video-container:hover {    cursor: pointer;}.video-controls:hover {    cursor: auto;}.video-container:hover .video-controls {    opacity: 0.9;}.video-controls button {    background: transparent;    border: none;    outline: none;    margin: 7px 5px;    vertical-align: top;    padding: 0px;    width: 25px;    height: 25px;}.video-controls input {    background: transparent;    height: 25px;}.video-controls button div {    width: 25px;    height: 25px;    background-size: contain;    background-position: center;    background-repeat: no-repeat;}.video-controls button:hover {    cursor: pointer;}input[type="range"] {    -webkit-appearance: none;    -webkit-tap-highlight-color: rgba(255, 255, 255, 0);    height: 4px;    width: 250px;    margin: 18px 0 18px 0;    border: none;    padding: 1px 2px;    border-radius: 14px;    background: #999;    outline: none; /* no focus outline */}input[type="range"]::-webkit-slider-thumb {    -webkit-appearance: none;    width: 14px;    height: 14px;    border: 4px solid #ffffff;    border-radius: 12px;    background-color: #333;    cursor: pointer;}.play-overlay {    position: absolute;    left: 0;    right: 0;    top: 0;    bottom: 0;    margin-left: auto;    margin-right: auto;    margin-top: auto;    margin-bottom: auto;    transition: opacity 0.5s;    opacity: 0.5;    background-position: center;    background-repeat: no-repeat;}.video-container:hover .play-overlay {    transition: opacity 0.5s;    opacity: 1.0;}.icon-loop-off {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABkAAAAZCAYAAADE6YVjAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAQVJREFUeNpi/P//PwOtARMDHcDwsYQFmdPY2BgApAyoYO6F+vr6DRg+AVqQAKQaqOT4Bqh5YMAISl1AAQcgewIQOwBd8IFSG4DmCQCpA0BcADTvAHKcfKCGBSAANefDaBIeppYAU1ElEF8hKTOSYDgo0y4AYh4gZqaqJdD8NAOIlUnRC1N4AYgFgIY0QDMRrGj4ADVcAWq4MzbDoZYjAxBfAGouJE6ghoEkFKBFSwOUDwMhQOyOx/UNaFgBufRgJLbSAroWVHAuAWJN9AQDNIwRn15GUmtGaNCsA2J+pJBgpGoSBhV4QCwEZOYC8U+a5hOgRdOAlAQQJxJSyzjakBh0lgAEGAA1oVh6dPo7zQAAAABJRU5ErkJggg==);}.icon-loop-on {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABoAAAAaCAYAAACpSkzOAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAOhJREFUeNpi/P//PwM9ABMDncCoRVSzKAGI/1MJJyAbzILELgDiACAWBOIPFHpAAIg3QOkJYBFQ8obiA0DsgMSnFDtAzQTzR1PdyLDIAIgVaGkRyPAdQHweiBtoYREsb9wHYndygu4DNBjwgUogfgHE+UQGKTzjMyLVRyCXHkCy9AO0pICBd0DMjyMUHgPxPTRfg4ADzDJGLBWfA5JlF5DEs4C4D4jZsVi0GSqHDA6g8EgsVgSAeAIQ//yPChYQ0ktqYvgALXzVgXgnEP+hdT56AMQeQGwKxFeB+BEhDYyjjZNRi2AAIMAA2HvfKdbT6m8AAAAASUVORK5CYII=);}.icon-pause {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABoAAAAaCAYAAACpSkzOAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAQJJREFUeNrElmERwyAMhQEDqwQkRAJSKqkSKmESKqESkMAUMLg97jKutHAdLHfvD4F8hYYQ6b0XFUZBJmjKxl3QFrRfRoigE81B1n/MBW2ZHHwWc4uxSg7DAGsQnQQhzElAqgXNWBS/WF/smEtjjT/aXQmyNAByLUew/LjuQnKYSWOSZZ2FjPiNxWzUkMiPTJ9kn2j0aX6EadAic44WrCz7WnzJbxOIMLmUwtxafF+xFf7Jq+p2t9uO2EahrPSAcNikxCAbBdKjQFbhklJHSIztFH7WoxOMEHsbemGHlKDhRbXXM0F/ffiGPuW86nZvTmrarWdruyVvNpAWF/6y+r8FGABwiQYMSv06DwAAAABJRU5ErkJggg==);}.icon-play-backward {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABoAAAAaCAYAAACpSkzOAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAUtJREFUeNq8ljFOxDAQRZ2QHt9gV6KhTElBkYID5AipadgjUFKmpEJ7BI7gggNkSzq4QbZEIJk/MBbG2N5kFeZLTyvFk/keOzt2Ya1VE1SDBujg+Qsw/JtVlRmjpBvQgRV4jSSkCZyCHejBNpmNKorQgpHpQZ2Ic7Fb+y2Tio292PNLj0BnDELIYODJtYeMnEk3wyDEVdemjNoFTHwzqmwdGmlvP9QCaF5GExrdspGemZBmvEmMNbxCjW80stmcGT/YH6XiDH9Uyt+bqdXcgTf7W6nYzuV2yzZMMLjmyt/tX+Uq/1q+iluLyXQIGr8HZwc6SUwjd5S68h7E9AQuwIk6XtS2dKmEVHoNNKZLcAWewceRHmtasZL3p8kE0vg5uAH7mYaaO/8g+nmL/WFFW5BYUxU7JkQPPtGj/N8uJ0XmXrfodauQukB+CjAArLWlFRjg3coAAAAASUVORK5CYII=);}.icon-play-forward {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABoAAAAaCAYAAACpSkzOAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAWFJREFUeNq8lj9OwzAUxp3QHXOCdgeJHiELgtFL94yMXdkQJ8gRurN0RKqQfIQgwVx6AeqRoSh8D54ly4qNUwU/6adIybO/PP/57KLrOpEQc1AB6b1/B5qf0ZhEvlGnS1CDKdj1dEg/cApeQANWwd6ooh4UMEwD5oE8m7vqfkOHcvsaNtxoDWREwIcEWv459ZeQFakHCPjY6lRISI0g4opRZTNfSDrzIUZA8jBqX+iehWSg0XLgfBEVj1DlChkWizX4BHcDxTQvKuHOjYwI7TnnAD76VlWA2vZd8qajDWcSHOIEnIFH8MpuEYu13dglJ2sxLMhRzsEGPIFZIM+wo/wIicRqQoLXYAseAjlkW7IUmWLiGOgxcQDP4Dbi4DSspuT5qY4QeANX4CYiItn5WxJqwWViVV9gDxbgImERKX62WTdsVgvKZqrZjomsB1/Wo/zfLidF5F436nWryHWB/BZgAOq/BX8ribu7AAAAAElFTkSuQmCC);}.icon-skip-backward {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABoAAAAaCAYAAACpSkzOAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAKxJREFUeNpi+v//PwM9MBMDncCoRaMWkWRRAgF5BSDeD8QGlFo0H4e4ABCvAeL7QOwA5VM16EAGdgDxSyAOJlYTC4mWZAFxGxBzk6qXWMWgoFkExJJkOI4ki/bTK3lfBeI/9LBIB4hDgfg9EP+ldYbdAMRCQFwLxD/pUTK0A7EEEE+kRxH0AYgLgFgRiHdSK9Ul4pF7AMQe0OLnAT5DGEHV7Gg1MWrR8LQIIMAAOHhl7iVNFKEAAAAASUVORK5CYII=);}.icon-skip-forward {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABoAAAAaCAYAAACpSkzOAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAALNJREFUeNpi/P//PwM9ABMDncCoRaMWEW2RARDvB2IFAuoSCNoEykd4sMN/BFgDxAI41P0nYA5JQRcMxC+BuAOIBWgdR2xAXAzED4A4i9aJgQWI+YF4IhA/AmIHWqc6kIWy0MRC8+T9B4iv0tKiv0D8HohDgViHVhb9BOJaIBYC4g2khDMpAJQAGoD4AzkRSgzYCcQZ0GRNHiCQo0ElgQGhXA/ECYTUMI5W5aMWDV+LAAIMAO8QuE7NMVrrAAAAAElFTkSuQmCC);}.play-overlay {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAAHQAAAB0CAYAAABUmhYnAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAyhpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdpbj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+IDx4OnhtcG1ldGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IkFkb2JlIFhNUCBDb3JlIDUuNS1jMDIxIDc5LjE1NTc3MiwgMjAxNC8wMS8xMy0xOTo0NDowMCAgICAgICAgIj4gPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4gPHJkZjpEZXNjcmlwdGlvbiByZGY6YWJvdXQ9IiIgeG1sbnM6eG1wPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvIiB4bWxuczp4bXBNTT0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL21tLyIgeG1sbnM6c3RSZWY9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZVJlZiMiIHhtcDpDcmVhdG9yVG9vbD0iQWRvYmUgUGhvdG9zaG9wIENDIDIwMTQgKE1hY2ludG9zaCkiIHhtcE1NOkluc3RhbmNlSUQ9InhtcC5paWQ6MUI0MUY3RkQzQThGMTFFNEEzRDZCQjU0MzFGNEM0RTUiIHhtcE1NOkRvY3VtZW50SUQ9InhtcC5kaWQ6MUI0MUY3RkUzQThGMTFFNEEzRDZCQjU0MzFGNEM0RTUiPiA8eG1wTU06RGVyaXZlZEZyb20gc3RSZWY6aW5zdGFuY2VJRD0ieG1wLmlpZDoxQjQxRjdGQjNBOEYxMUU0QTNENkJCNTQzMUY0QzRFNSIgc3RSZWY6ZG9jdW1lbnRJRD0ieG1wLmRpZDoxQjQxRjdGQzNBOEYxMUU0QTNENkJCNTQzMUY0QzRFNSIvPiA8L3JkZjpEZXNjcmlwdGlvbj4gPC9yZGY6UkRGPiA8L3g6eG1wbWV0YT4gPD94cGFja2V0IGVuZD0iciI/PqEf6x4AACHsSURBVHja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);}';document.head.appendChild(css);/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

function calculateCanvasSize(canvas, windowWidth, windowHeight) {
    var UNIT = "px";
    if ((windowWidth / windowHeight) > (canvas.width / canvas.height)) {
        canvas.style.height = "" + windowHeight + UNIT;
        canvas.style.marginTop = "0" + UNIT;
        var newWidth = canvas.width * windowHeight / canvas.height;
        canvas.style.width = "" + newWidth + UNIT;
        canvas.style.marginLeft = "" + (windowWidth - newWidth) / 2 + UNIT;
    } else {
        canvas.style.width = "" + windowWidth + UNIT;
        canvas.style.marginLeft = "0" + UNIT;
        var newHeight = canvas.height * windowWidth / canvas.width;
        canvas.style.height = "" + newHeight + UNIT;
        canvas.style.marginTop = "" + (windowHeight - newHeight) / 2 + UNIT;
    }
}

function hookToResize(canvas) {
    window.addEventListener('resize', function(e) {
        var windowWidth = e.target.innerWidth;
        var windowHeight = e.target.innerHeight;
        calculateCanvasSize(canvas, windowWidth, windowHeight);
    });
    calculateCanvasSize(canvas, window.innerWidth, window.innerHeight);
}
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

// TrackFeatureEnum bit indices (heifreaderdatatypes.h). Promoted to
// module scope so _buildFileInfo can use them for filtering and
// _readTrack can use them to decode per-track booleans for the
// website's classifiers.
var TF_IS_MASTER_IMAGE_SEQUENCE   = 1 << 0;
var TF_IS_THUMBNAIL_IMAGE_SEQUENCE = 1 << 1;
var TF_IS_AUXILIARY_IMAGE_SEQUENCE = 1 << 2;
var TF_IS_AUDIO_TRACK = 1 << 17;
var TF_IS_VIDEO_TRACK = 1 << 18;

/* file-info.js — per-file snapshot built once at open time.
 *
 * Why this exists.
 *   The legacy embind binding shipped a deep `requestFileInfo` POD
 *   (`fileFeature`, `rootLevelMetaBoxProperties.{contextId,
 *   metaBoxFeature, imageFeaturesMap, itemInformationMap}`,
 *   `trackProperties[].sampleProperties`) that the website classifies
 *   files with and that every helper down the stack reads as O(1) JS
 *   field access. The c-facade-rewrite dropped that POD because the
 *   modern reader is bytes-in / lazy-everywhere — every accessor is a
 *   wasm round-trip — and a POD-shaped library mirror inside the
 *   binding would have been an anti-pattern (integration-guide §0
 *   principle 2).
 *
 *   But the website still wants a one-shot file overview. Without it,
 *   a 200-card gallery does ~50 wasm round-trips per card just to
 *   classify the file shape, and the heif-object dispatcher has no
 *   honest answer to "is this a burst / sequence / cover / single
 *   image" without re-asking the reader on every gate.
 *
 *   The right home for that one-shot snapshot is *here*, on the
 *   website-adapter side — not in the binding (which stays thin) and
 *   not in the library (which is read-only and already does the right
 *   thing by being lazy). buildFileInfo() walks the live HEIFReader
 *   exactly once at open time, freezes the result, and hands it to
 *   HEIFObject._fileInfo. Everything downstream reads it as plain JS.
 *
 * Shape (intentionally legacy-compatible).
 *   The fields are named after the legacy POD so the existing
 *   classifier expressions in heif-object.js, grid-object.js,
 *   cover-object.js and burst.js continue to work verbatim. The
 *   nesting (fileFeature.* / rootLevelMetaBoxProperties.*) is a C++
 *   shape leak from the original library API, but it is what every
 *   consumer already speaks; flattening it would be a separate
 *   sweep with zero functional benefit.
 *
 *   Snapshot only contains closed-reader-invariant data. Bitstreams,
 *   decoder parameters, full per-sample track tables — anything that
 *   either (a) costs a lot of bytes or (b) depends on what the user
 *   is about to do — stay lazy on the reader.
 *
 * §15 lossy fields (kept absent rather than faked).
 *   Per-item booleans the legacy POD provided that the modern reader
 *   does not yet expose — `isHidden`, `isCover`, `isPredictivelyCoded`,
 *   `isGainmap`, `isPreComputedDerived`, `hasUnrecognizedRequiredReferences`
 *   — would require parsing essential-property flags / `rref` boxes
 *   that the C facade does not yet surface. They are not invented
 *   here; consumers that need them will see `undefined` and either
 *   degrade gracefully or fall back to the reader.
 */

/** Build the once-per-file snapshot consumed by HEIFObject and the
 *  elements/* renderers. Walks the reader; never holds it.
 *
 *  @param  {HEIFReader} reader  Open reader (heif.open(bytes) result).
 *  @param  {string}     url     Source URL, kept for diagnostics only.
 *  @return {object}             Frozen snapshot. See file-level comment
 *                               for the field contract.
 */
function buildFileInfo(reader, url) {
    // ----- file-level identity ------------------------------------------
    // Cheap; all four are single-cwrap reads.
    var majorBrand       = _safeRead(function () { return reader.majorBrand;       }, '');
    var compatibleBrands = _safeRead(function () { return reader.compatibleBrands; }, []);
    var minorVersion     = _safeRead(function () { return reader.minorVersion;     }, 0);
    var primaryItemId    = _safeRead(function () { return reader.primaryItemId;    }, 0);

    // ----- contexts -----------------------------------------------------
    // Meta-context is a sentinel (0) on the modern binding; ImageProvider
    // mirrors this. Track ids come from the live reader.
    //
    // The reader returns *all* MOOV tracks (audio, video, pict). The
    // website only knows how to render image-sequence ('pict') tracks,
    // so filter out audio/video tracks here — otherwise downstream code
    // sets canvas dimensions to 0×0 (track has no displayWidth/Height)
    // and trackSampleBitstream throws UNSUPPORTED_CODE_TYPE on the first
    // mp4a sample. (See module-level TF_IS_* constants for bit indices.)
    var allTrackIds = _safeRead(function () { return _toArray(reader.tracks()); }, []);
    var trackIds = [];
    for (var ti = 0; ti < allTrackIds.length; ti++) {
        var tid = allTrackIds[ti];
        var info = _safeRead(function () { return reader.trackInfo(tid); }, null);
        var feat = (info && info.features) | 0;
        if (feat & (TF_IS_AUDIO_TRACK | TF_IS_VIDEO_TRACK)) continue;
        trackIds.push(tid);
    }
    var metaContextId = 0;

    // ----- metabox image inventory --------------------------------------
    // Walk every item type the website actually classifies on. Cheap
    // even on large gallery files: each itemsByType is one count cwrap
    // plus N id cwraps; we only walk it once and cache the result here.
    var masterIds = _safeRead(function () { return _toArray(reader.masterImages()); }, []);
    var thmbIds   = _safeItemsByType(reader, 'thmb');
    var auxlIds   = _safeItemsByType(reader, 'auxl');
    var gridIds   = _safeItemsByType(reader, 'grid');
    var iovlIds   = _safeItemsByType(reader, 'iovl');
    var tmapIds   = _safeItemsByType(reader, 'tmap');
    var exifIds   = _safeItemsByType(reader, 'Exif');
    var mimeIds   = _safeItemsByType(reader, 'mime');

    var hasMetaBox = (masterIds.length + thmbIds.length + auxlIds.length +
                      gridIds.length + iovlIds.length + tmapIds.length +
                      exifIds.length + mimeIds.length) > 0;

    // Hidden items — marked IsHiddenImage in the per-item feature bitmask.
    // The gallery filters these from display; they remain in masterIds so
    // that reference resolution (thmb/auxl/dimg) still works.
    var hiddenIds = [];
    if (typeof reader.itemIsHidden === 'function') {
        for (var _h = 0; _h < masterIds.length; _h++) {
            try { if (reader.itemIsHidden(masterIds[_h])) hiddenIds.push(masterIds[_h]); } catch (e) {}
        }
    }

    // ----- file-shape classification ------------------------------------
    // These four flags drive the entire heif-object dispatch
    // (still / collection / sequence / cover). Names are the legacy
    // ones; semantics match what the legacy POD computed.
    var hasImageSequence   = trackIds.length > 0;
    var hasImageCollection = masterIds.length > 1;
    var hasSingleImage     = masterIds.length === 1 && trackIds.length === 0;
    var hasCoverImage      = primaryItemId !== 0;
    var hasDerivedImages   = (gridIds.length + iovlIds.length + tmapIds.length) > 0;

    // ----- per-item index (eager walk) ----------------------------------
    // Walks every metabox image item once: type + size + resolved
    // properties + the three reference walks the website cares about
    // (thmb / auxl / dimg). Eager is fine — the data is closed-reader-
    // invariant and the website re-reads it many times per file. If a
    // future fixture has thousands of items and this becomes hot, the
    // imageFeaturesMap walk is the one thing to make lazy.
    var imageFeaturesMap = {};
    var itemInformationMap = {};

    // Master / thumbnail / auxiliary / derived image items.
    var imageItemIds = _concatUnique(masterIds, thmbIds, auxlIds, gridIds, iovlIds, tmapIds);
    for (var i = 0; i < imageItemIds.length; i++) {
        var itemId = imageItemIds[i];
        var entry = _readImageItem(reader, itemId, primaryItemId,
                                   masterIds, thmbIds, auxlIds, gridIds, iovlIds, tmapIds);
        imageFeaturesMap[itemId] = Object.freeze(entry);
        // §15: byte-size of an item is not exposed by the modern reader
        // without a full bitstream probe (rule 5). Keep `size: 0` so
        // consumers that read it (decoder-manager logs `item.size ||
        // '-'`) degrade cleanly. width/height stay on the entry.
        itemInformationMap[itemId] = Object.freeze({
            type: entry.type, size: 0,
        });
    }
    // Thumbnail / auxiliary items in the wild typically carry their
    // codec 4cc (hvc1, vvc1, ...) on the item itself and are only
    // distinguished from masters by being the target of a `thmb`
    // / `auxl` iref. itemsByType('thmb' / 'auxl') therefore misses
    // them on most real-world files. Union the iref targets we
    // already walked into the bucket lists so consumers asking
    // "what are this file's thumbnails" get a useful answer.
    var thmbFromRefs = [];
    var auxlFromRefs = [];
    for (var refKey in imageFeaturesMap) {
        if (!imageFeaturesMap.hasOwnProperty(refKey)) continue;
        var refEntry = imageFeaturesMap[refKey];
        for (var ti = 0; ti < refEntry.thmbRefs.length; ti++) thmbFromRefs.push(refEntry.thmbRefs[ti]);
        for (var ai = 0; ai < refEntry.auxlRefs.length; ai++) auxlFromRefs.push(refEntry.auxlRefs[ai]);
    }
    thmbIds = _concatUnique(thmbIds, thmbFromRefs);
    auxlIds = _concatUnique(auxlIds, auxlFromRefs);

    // Non-image items only need type+size in itemInformationMap.
    // Same §15 caveat — byte size unavailable without a probe.
    for (var j = 0; j < exifIds.length; j++) {
        itemInformationMap[exifIds[j]] = Object.freeze({ type: 'Exif', size: 0 });
    }
    for (var k = 0; k < mimeIds.length; k++) {
        itemInformationMap[mimeIds[k]] = Object.freeze({ type: 'mime', size: 0 });
    }

    // ----- track summaries ----------------------------------------------
    // One entry per track with the scalar fields heif-object reads
    // (frameRate, timeScale, displayWidth/Height, sampleCount). Sample
    // count uses the dedicated cwrap rather than walking the full
    // sample table — the per-sample POD walk stays lazy on the reader.
    var trackProperties = [];
    for (var t = 0; t < trackIds.length; t++) {
        var trackId = trackIds[t];
        trackProperties.push(Object.freeze(_readTrack(reader, trackId)));
    }

    // ----- entity groups (GroupsListBox) --------------------------------
    // Stereo pairs ('ster'), bothie ('tsyn'), brackets ('aebr'/'wbbr'/...),
    // album/favourites/burst/panorama/slideshow/progressive/audio-image
    // are all entity-group flavours the website classifies on. One reader
    // call enumerates the whole list; per-group entity ids walk lazily
    // inside the binding. Returns [] on files with no GroupsListBox.
    var entityGroups = _safeRead(function () {
        var raw = reader.entityGroups() || [];
        var frozen = new Array(raw.length);
        for (var i = 0; i < raw.length; i++) {
            frozen[i] = Object.freeze({
                groupId:   raw[i].groupId,
                type:      raw[i].type,
                entityIds: Object.freeze(raw[i].entityIds.slice()),
            });
        }
        return frozen;
    }, []);

    // ----- assemble (legacy-compatible nesting) -------------------------
    var rootLevelMetaBoxProperties = hasMetaBox ? Object.freeze({
        contextId: metaContextId,
        metaBoxFeature: Object.freeze({
            hasDerivedImages: hasDerivedImages,
            hasGroupLists:    entityGroups.length > 0,
        }),
        // Count summaries consumed by HEIFFileLogger
        // (decoder-manager.logFileInfoSummary). Read straight off
        // the bucket lists rather than re-walking imageFeaturesMap.
        // `itemCount` matches the legacy POD's "every image item"
        // semantics (masters + thumbs + auxiliaries + derived).
        itemCount:          imageItemIds.length,
        masterImageCount:   masterIds.length,
        thumbnailCount:     thmbIds.length,
        auxiliaryCount:     auxlIds.length,
        derivedImageCount:  gridIds.length + iovlIds.length + tmapIds.length,
        imageFeaturesMap: Object.freeze(imageFeaturesMap),
        itemInformationMap: Object.freeze(itemInformationMap),
        // GroupsListBox payload as `[{groupId, type, entityIds}, ...]`.
        // The website's classifiers index by `type` (4-CC); see
        // gallery._getEntityGroupItemIds and image-provider.deriveItemType.
        entityGroups:       Object.freeze(entityGroups),
    }) : null;

    return Object.freeze({
        success: true,
        url: url,

        // Top-level identity (flat — these were never nested in the
        // legacy POD either).
        majorBrand:       majorBrand,
        compatibleBrands: Object.freeze(compatibleBrands.slice()),
        minorVersion:     minorVersion,
        primaryItemId:    primaryItemId,

        // Drives heif-object._isPlayable / hasAnimation /
        // hasImageCollection / hasVideo and the still-image fallback
        // in _getFileInfoCallback.
        fileFeature: Object.freeze({
            hasSingleImage:     hasSingleImage,
            hasImageCollection: hasImageCollection,
            hasImageSequence:   hasImageSequence,
            hasCoverImage:      hasCoverImage,
        }),

        // Drives _getFirstRootMetaItemByType (grid / iovl detection)
        // and the burst-context lookup (burst.js reads .contextId).
        rootLevelMetaBoxProperties: rootLevelMetaBoxProperties,

        // Drives the track-based-sequence guard in _updateFrameBuffer
        // and the hasVideo / hasAnimation classifiers.
        trackProperties: Object.freeze(trackProperties),

        // Type-bucket convenience arrays — same data as
        // imageFeaturesMap, but in the shape image-provider.js
        // wants for its O(1) lookups.
        masterIds: Object.freeze(masterIds.slice()),
        thmbIds:   Object.freeze(thmbIds.slice()),
        auxlIds:   Object.freeze(auxlIds.slice()),
        gridIds:   Object.freeze(gridIds.slice()),
        iovlIds:   Object.freeze(iovlIds.slice()),
        tmapIds:   Object.freeze(tmapIds.slice()),
        exifIds:   Object.freeze(exifIds.slice()),
        mimeIds:   Object.freeze(mimeIds.slice()),
        hiddenIds: Object.freeze(hiddenIds.slice()),
        trackIds:  Object.freeze(trackIds.slice()),
    });
}

// ----- internal helpers ----------------------------------------------

function _safeRead(thunk, fallback) {
    try { var v = thunk(); return (v === undefined || v === null) ? fallback : v; }
    catch (e) { return fallback; }
}

function _toArray(maybeTyped) {
    if (!maybeTyped) return [];
    if (Array.isArray(maybeTyped)) return maybeTyped.slice();
    // Uint32Array etc. — copy into a plain array for JSON-friendliness.
    var out = new Array(maybeTyped.length);
    for (var i = 0; i < maybeTyped.length; i++) out[i] = maybeTyped[i];
    return out;
}

function _safeItemsByType(reader, fourCC) {
    return _toArray(_safeRead(function () { return reader.itemsByType(fourCC); }, []));
}

function _concatUnique() {
    var seen = {};
    var out = [];
    for (var a = 0; a < arguments.length; a++) {
        var arr = arguments[a];
        for (var i = 0; i < arr.length; i++) {
            var id = arr[i];
            if (!seen[id]) { seen[id] = true; out.push(id); }
        }
    }
    return out;
}

function _has(arr, v) {
    for (var i = 0; i < arr.length; i++) if (arr[i] === v) return true;
    return false;
}

function _readImageItem(reader, itemId, primaryItemId,
                        masterIds, thmbIds, auxlIds, gridIds, iovlIds, tmapIds) {
    // Type comes from itemType() rather than the bucket lookup so that
    // an item appearing in an unexpected type (defensive) still gets
    // the right four-CC.
    var type = _safeRead(function () { return reader.itemType(itemId); }, '');

    // Resolved properties: ispe / irot / imir / pixi / colr / raw.
    // One cwrap-storm but bounded by the property list per item;
    // worth eager-caching for the gallery hover / metadata panel.
    var properties = _safeRead(function () { return reader.itemPropertiesResolved(itemId); }, null);

    // Reference walks. The website cares about three:
    //   thmb : items this one *is a thumbnail of*
    //   auxl : items this one *is an auxiliary for*
    //   dimg : items this one *derives from* (grid tiles, iovl layers)
    var thmbRefs = _toArray(_safeRead(function () { return reader.referencedTo(itemId, 'thmb'); }, []));
    var auxlRefs = _toArray(_safeRead(function () { return reader.referencedTo(itemId, 'auxl'); }, []));
    var dimgRefs = _toArray(_safeRead(function () { return reader.referencedTo(itemId, 'dimg'); }, []));

    var width  = (properties && properties.ispe) ? properties.ispe.width  : 0;
    var height = (properties && properties.ispe) ? properties.ispe.height : 0;

    return {
        itemId:  itemId,
        type:    type,
        // §15: byte-size of an item is not exposed by the modern reader
        // without a bitstream probe (rule 5). Pin to 0; itemInformationMap
        // does the same. Note: reader.itemSize() is a width/height POD,
        // not a byte count — do NOT use it here.
        size:    0,
        width:   width,
        height:  height,
        properties: properties,

        // Reference arrays. Frozen at the parent level via Object.freeze
        // on the imageFeaturesMap; arrays here are returned by-value,
        // not aliased into reader-internal storage.
        thmbRefs: thmbRefs,
        auxlRefs: auxlRefs,
        dimgRefs: dimgRefs,

        // Best-effort feature derivation (§15 caveat: the lossy flags
        // are intentionally absent, not faked).
        isMaster:    _has(masterIds, itemId),
        isThumbnail: _has(thmbIds,   itemId),
        isAuxiliary: _has(auxlIds,   itemId),
        isDerived:   _has(gridIds, itemId) || _has(iovlIds, itemId) || _has(tmapIds, itemId),
        isPrimary:   itemId === primaryItemId,

        hasLinkedThumbnails:     thmbRefs.length > 0,
        hasLinkedAuxiliaryImage: auxlRefs.length > 0,
        hasLinkedDimg:           dimgRefs.length > 0,

        isHidden:    (typeof reader.itemIsHidden === 'function') ?
                         _safeRead(function () { return reader.itemIsHidden(itemId); }, false) : false,
    };
}

function _readTrack(reader, trackId) {
    var info          = _safeRead(function () { return reader.trackInfo(trackId);          }, null);
    var displayWidth  = _safeRead(function () { return reader.trackDisplayWidth(trackId);  }, 0);
    var displayHeight = _safeRead(function () { return reader.trackDisplayHeight(trackId); }, 0);
    // Sample id list. The legacy POD shipped a per-sample dict
    // (`sampleProperties`) keyed by sample id; the website only ever
    // read the keys (frame ids in order) and the count. trackSamples()
    // returns full sample-info objects ({sampleId, size, ts, ...}); we
    // project to the bare id array because that is what the field name
    // promises and what consumers (gallery animated/burst loaders,
    // requestImageData) pass straight back to the reader as item ids.
    var sampleIds = _safeRead(function () {
        var samples = _toArray(reader.trackSamples(trackId));
        var ids = new Array(samples.length);
        for (var si = 0; si < samples.length; si++) ids[si] = samples[si].sampleId;
        return ids;
    }, []);
    var sampleCount = sampleIds.length;

    var frameRate = (info && info.frameRate) ? info.frameRate : { num: 0, den: 0 };
    var timeScale = info ? info.timeScale : 0;
    var majorBrand = info ? info.majorBrand : '';
    var features  = (info ? info.features  : 0) | 0;

    // Edit list. The binding exposes scalars (looping + repetitions) and
    // the edit-unit array separately.  Snapshot both so the animation
    // loop can build a presentation timeline without touching the reader.
    var editListMeta = _safeRead(function () { return reader.trackEditListMeta(trackId); }, null);
    var editUnits    = _safeRead(function () { return reader.trackEditUnits(trackId);    }, []);

    // Track-level alternate siblings (same alternateGroupId).
    var alternateTracks = _safeRead(function () {
        var a = reader.trackAlternates(trackId);
        // collectU32 returns a Uint32Array; freeze a plain array.
        return a && a.length > 0 ? Array.prototype.slice.call(a) : [];
    }, []);

    return {
        trackId:       trackId,
        displayWidth:  displayWidth,
        displayHeight: displayHeight,
        frameRate:     Object.freeze({ num: frameRate.num, den: frameRate.den }),
        timeScale:     timeScale,
        majorBrand:    majorBrand,
        sampleCount:   sampleCount,
        sampleIds:     Object.freeze(sampleIds),

        // Edit list data.  editListMeta is null when the track has no
        // edit list (NOT_APPLICABLE); editUnits is the per-entry array.
        editListMeta:  editListMeta ? Object.freeze(editListMeta) : null,
        editUnits:     Object.freeze(editUnits),

        // Alternate-track siblings (same alternateGroupId).
        alternateTracks: Object.freeze(alternateTracks),

        // Per-track feature flags. Legacy POD exposed each bit as a
        // boolean inside trackFeature; we keep the same shape so the
        // website's classifiers (gallery animated/burst, decoder-manager
        // diagnostics) read identically. featuresMask preserves the
        // raw bitmask for any future flag a consumer needs.
        trackFeature: Object.freeze({
            featuresMask:               features,
            isMasterImageSequence:      !!(features & TF_IS_MASTER_IMAGE_SEQUENCE),
            isThumbnailImageSequence:   !!(features & TF_IS_THUMBNAIL_IMAGE_SEQUENCE),
            isAuxiliaryImageSequence:   !!(features & TF_IS_AUXILIARY_IMAGE_SEQUENCE),
            isAudioTrack:               !!(features & TF_IS_AUDIO_TRACK),
            isVideoTrack:               !!(features & TF_IS_VIDEO_TRACK),
        }),
    };
}
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

// DEFAULT_FRAMERATE = 24 is a hardcoded fallback when no frameRate is specified in the file.
// Ideally, files should always provide timing info, but if not, consider making this configurable
// or deriving a reasonable default from common use cases (e.g., 30fps for video, 1fps for slideshows).
var DEFAULT_FRAMERATE = 24; // FPS to use if no frameRate is specified
// PREFERRED_BUFFER_SIZE is derived from DEFAULT_FRAMERATE - consider making buffer sizing adaptive
var PREFERRED_BUFFER_SIZE = DEFAULT_FRAMERATE * 5;
// FRAME_REQUEST_SIZE = 64 is a magic number for batch frame requests - could be tuned based on network/memory constraints
var FRAME_REQUEST_SIZE = 64;
var MAXIMUM_CACHE_SIZE = PREFERRED_BUFFER_SIZE + FRAME_REQUEST_SIZE;

// Module-level singleton: one HEIF wasm instance shared across every
// HEIFObject on the page. HEIF.load() is async (it awaits the emcc
// factory) but we only ever call it once.
var _heifInstancePromise = null;
function _loadHeif() {
    if (!_heifInstancePromise) {
        if (typeof HEIF === 'undefined' || typeof HEIF.load !== 'function') {
            return Promise.reject(new Error(
                'HEIFObject: heif.js / heif-raw.js not loaded'));
        }
        _heifInstancePromise = HEIF.load();
    }
    return _heifInstancePromise;
}

// Per-file snapshot construction lives in file-info.js (`buildFileInfo`).
// We hold the result on HEIFObject._fileInfo and every consumer
// downstream (heif-object classifiers, image-provider helpers,
// elements/* renderers) reads it as plain JS. The reader itself is
// kept on HEIFObject._heifReader for bitstream / per-decode calls.

/** @constructor */
function HEIFObject(heifCanvas, type) {

    var self = this;

    this._autoplay = heifCanvas.autoplay;
    this._context = heifCanvas.getContext("2d");
    this._delayedStart = false;
    this._displayHeight = null;
    this._displayWidth = null;
    this._isDisposed = false;  // Track if object has been cleaned up
    this._drawPendingId = -1;
    this._fileInfo = undefined;
    this._frameCache = undefined;
    this._frameRate = undefined;
    this._collectionViewObject = null;
    this._collectionViewType = null;
    this._derivedGridObject = null;
    this._heifCanvas = heifCanvas;
    // Modern binding is bytes-in: defer reader construction until
    // requestFileInfo() has fetched the file. _heifReader / _imageProvider
    // remain null until then. Callers must not touch them before the
    // requestFileInfo callback fires.
    this._url = heifCanvas.getAttribute("filename");
    this._heifReader = null;
    this._imageProvider = null;
    this._isDirectionForward = true;
    this._isOnloadCalled = false;
    this._isPreloading = false;
    this._isStopped = true;
    this._name = "HEIFObject";
    this._masterContextId = null;
    this._masterIds = [];
    this._sentRequests = {};
    this._timestamps = [];
    this._timestampsLastDrawnIndex = -1;
    this._type = type;
    this._videoControlObject = undefined;

    this._loop = true;
    if (type === "video") {
        this._loop = heifCanvas.loop;
    }

    if (heifCanvas.fps >= 0) {
        this._frameRate = heifCanvas.fps;
    }

    if (heifCanvas.type) {
        this._collectionViewType = heifCanvas.type;
    }

    this.callOnload = function() {
        if (this._heifCanvas && this._heifCanvas.onload && !this._isOnloadCalled) {
            this._heifCanvas.onload();
            this._isOnloadCalled = true;
        }
        // Notify observers (e.g. comparison overlay tracker) that this
        // canvas has finished decoding. The event bubbles from the canvas
        // so listeners on `document` can match by section.
        if (this._heifCanvas) {
            this._heifCanvas.dispatchEvent(new CustomEvent('heif-decoded', { bubbles: true }));
        }
    };

    this.drawCanvasFrameData = function (width, height, data) {
        var imageData = this._context.createImageData(width, height);
        imageData.data.set(data);

        // For rotated images, we need to handle the internal canvas differently
        var canvasWidth = this._context.canvas.width;
        var canvasHeight = this._context.canvas.height;
        
        if (canvasWidth !== width || canvasHeight !== height) {
            // Create temporary canvas at original dimensions
            var thumbnailCanvas = document.createElement("canvas");
            thumbnailCanvas.width = width;
            thumbnailCanvas.height = height;
            thumbnailCanvas.getContext("2d").putImageData(imageData, 0, 0);
            
            // Draw scaled/fitted to display canvas
            this._context.drawImage(thumbnailCanvas, 0, 0, canvasWidth, canvasHeight);
        } else {
            // Direct draw - no scaling needed
            this._context.putImageData(imageData, 0, 0);
        }
    };

    this.drawErrorImage = function () {
        this._context.fillStyle = "#cccccc";
        this._context.fillRect(0, 0, this._context.canvas.width, this._context.canvas.height);

        var fontHeight = this._context.canvas.width / 10;
        this._context.font =  fontHeight + "px monospace";

        this._context.fillStyle = "#aaaaaa";
        this._context.textAlign = "center";
        this._context.textBaseline = "middle";
        this._context.fillText("NO IMAGE", this._context.canvas.width / 2, this._context.canvas.height / 2);
    };

    this.getFileInfo = function () {
        return this._fileInfo;
    };

    this.getImageProvider = function () {
        return this._imageProvider;
    };

    this.hasAnimation = function () {
        return (this._type === "img" && !!this._fileInfo && !!this._fileInfo.fileFeature && (this._fileInfo.fileFeature.hasImageSequence || this._fileInfo.fileFeature.hasImageCollection && this._frameRate > 0));
    };

    this.hasImageCollection = function () {
        return this._type === "img" && !!this._fileInfo && !!this._fileInfo.fileFeature &&
            (this._fileInfo.fileFeature.hasImageCollection && !this._frameRate);
    };

    this.hasVideo = function () {
        return this._type === "video" && !!this._fileInfo && !!this._fileInfo.fileFeature && (this._fileInfo.fileFeature.hasImageSequence || this._fileInfo.fileFeature.hasImageCollection && this._frameRate > 0);
    };

    this.isForcedCoverView = function () {
        return self._collectionViewType === "cover";
    };

    this.isForcedGridView = function () {
        return self._collectionViewType === "grid";
    };

    this.isLastFrame = function (_frameOrderIndex) {
        return (this._isDirectionForward && _frameOrderIndex === this._timestamps.length - 1 ||
                !this._isDirectionForward && _frameOrderIndex === 0);
    };

    this.initAnimation = function () {
        // Ensure cache is large enough for all frames in the animation
        var cacheSize = Math.max(PREFERRED_BUFFER_SIZE, this._timestamps ? this._timestamps.length : PREFERRED_BUFFER_SIZE);
        var maxSize = Math.max(MAXIMUM_CACHE_SIZE, cacheSize);
        this._frameCache = new Cache(cacheSize, maxSize);
        this._sentRequests = {};

        var masterContextId = this._imageProvider.getMasterContextId(this._fileInfo);

        // The modern reader exposes per-track timestamps as
        // [{timestampMs, sampleId}, ...] via trackTimestamps. The
        // animation loop downstream reads `_timestamps[i].id` and
        // `_timestamps[i].t`, so bridge once at the call site rather
        // than rename eight callsites. Returns [] for non-track
        // contexts (still-image masterContextId === 0).
        if (this._imageProvider._isTrackContext(masterContextId)) {
            var raw = this._heifReader.trackTimestamps(masterContextId) || [];
            this._timestamps = new Array(raw.length);
            for (var i = 0; i < raw.length; i++) {
                this._timestamps[i] = { id: raw[i].sampleId, t: raw[i].timestampMs };
            }
        } else {
            this._timestamps = [];
        }

        // Re-initialize cache with correct size now that we know timestamp count
        if (this._timestamps && this._timestamps.length > 0) {
            var animationSize = this._timestamps.length;
            this._frameCache = new Cache(animationSize, animationSize);
        }

        if (this._videoControlObject) {
            this._videoControlObject.setCurrentFrame(0, this._timestamps.length);
            this._videoControlObject.buildThumbnailIdLookup();
        }
    };

    this.onAnimationTick = function () {
        // Exit early if object was disposed during animation
        if (self._isDisposed) {
            self.stopAnimation();
            return;
        }

        this._updateFrameBuffer();

        if (!this._loop && this.isLastFrame(this._timestampsLastDrawnIndex)) {
            this.stopAnimation();

            if (this._videoControlObject) {
                this._videoControlObject.pause();
            }

            return;
        }

        var nextFrameOrderIndex = this._getNextFrameOrderIndex();
        var id = this._timestamps[nextFrameOrderIndex].id;
        if (this._frameCache.get(id)) {
            this.drawCanvasFrameData(this._displayWidth, this._displayHeight, this._frameCache.get(id));
            this._timestampsLastDrawnIndex = nextFrameOrderIndex;

            if (this._videoControlObject) {
                this._videoControlObject.setCurrentFrame(this._timestampsLastDrawnIndex, this._timestamps.length);
            }
        }
    };


    this.requestFileInfo = function () {
        var cb = this._getFileInfoCallback();
        var url = this._url;
        _loadHeif().then(function (heif) {
            return fetch(url).then(function (resp) {
                if (!resp.ok) {
                    throw new Error('fetch ' + url + ' -> HTTP ' + resp.status);
                }
                return resp.arrayBuffer();
            }).then(function (buf) {
                if (self._isDisposed) return;
                var bytes  = new Uint8Array(buf);
                var reader = heif.open(bytes);
                // Re-check disposal *after* open: the element can be torn
                // down mid-fetch, and at that point _dispose has already
                // run with _heifReader === null, so the freshly-opened
                // reader would be orphaned with no further close path.
                if (self._isDisposed) {
                    try { reader.close(); } catch (_) {}
                    return;
                }
                self._heifReader = reader;
                // Snapshot first, then thread it into the provider so its
                // helpers can read the closed-reader-invariant data without
                // round-tripping the wasm reader (§15 active item 2).
                var fileInfo = buildFileInfo(reader, url);
                self._imageProvider = new ImageProvider(reader, url, fileInfo);
                cb(fileInfo);
            });
        }).catch(function (err) {
            // If buildFileInfo / ImageProvider throws after the reader is
            // stashed, the element's _dispose path would still close it -
            // but only if the element is later torn down. Defensively
            // close here so a transient parse failure doesn't pin a wasm
            // reader for the lifetime of the page.
            if (self._heifReader && typeof self._heifReader.close === 'function') {
                try { self._heifReader.close(); } catch (_) {}
                self._heifReader = null;
            }
            console.error('HEIFObject.requestFileInfo failed for ' + url, err);
            cb({ success: false, error: String(err && err.message || err) });
        });
    };

    this.requestThumbnailImageData = function (frameIndices, callback) {
        this._imageProvider.requestImageData(
            this._imageProvider.getThumbnailContextId(this._fileInfo), frameIndices, callback);
    };

    this.setFrameOrderIndex = function (index) {
        this._timestampsLastDrawnIndex = index;
        var id = this._timestamps[index].id;
        if (this._frameCache.get(id)) {
            this._drawPendingId = -1;
            this.drawCanvasFrameData(this._displayWidth, this._displayHeight, this._frameCache.get(id));
        } else {
            this._drawPendingId = id;
            this._imageProvider.requestImageData(
                this._imageProvider.getMasterContextId(this._fileInfo), [id], this._getImageDataCallback());
        }
    };

    this.setDirection = function (forward) {
        this._isDirectionForward = forward;
    };

    this.setLoop = function (loop) {
        this._loop = loop;
    };

    this.setVideoControlObject = function (videoControlObject) {
        this._videoControlObject = videoControlObject;
    };

    this.startAnimation = function () {

        self._isStopped = false;

        if (!this._isPreloading) {
            if (!this._frameCache) {
                this.initAnimation();
            }

            this._sentRequests = {};
            this._drawPendingId = -1;

            var timeout = function () {

                self.onAnimationTick();

                var interval = 0;
                if (self._timestamps) {
                    if (this._isDirectionForward) {
                        if (self._timestampsLastDrawnIndex >= 0 &&
                            self._timestampsLastDrawnIndex + 1 < self._timestamps.length) {
                            interval =
                                self._timestamps[self._timestampsLastDrawnIndex + 1].t -
                                self._timestamps[self._timestampsLastDrawnIndex].t;
                        }
                    } else {
                        if (self._timestampsLastDrawnIndex - 1 >= 0) {
                            interval =
                                self._timestamps[self._timestampsLastDrawnIndex].t -
                                self._timestamps[self._timestampsLastDrawnIndex - 1].t;
                        }
                    }
                }

                if (!self._isStopped) {
                    this.animationIntervalTimerId = setTimeout(timeout, interval);
                }
            };

            timeout();

        } else {
            this._delayedStart = true;
        }
    };

    this.stopAnimation = function () {
        self._isStopped = true;
        clearTimeout(this.animationIntervalTimerId);
    };

    this._getDrawImageDataCallback = function () {
        return function (payload) {
            if (payload.success && payload.frames && payload.frames.length > 0) {
                var frame = payload.frames[0];
                var transformations = frame.transformations || {};
                
                // Fallback to legacy fields if transformations object not present
                var rotation = transformations.rotation !== undefined ? transformations.rotation : (frame.rotation || 0);
                var mirror = transformations.mirror || frame.mirror || null;
                var cleanAperture = transformations.cleanAperture || null;
                var scale = transformations.scale || null;
                var layerSelection = transformations.layerSelection || null;
                
                // Start with decoded dimensions
                var sourceWidth = frame.width;
                var sourceHeight = frame.height;
                var sourceData = frame.canvasFrameData;
                
                // === TRANSFORMATION ORDER PER HEIF SPEC ===
                // 1. LSEL (Layer Selection) - would affect which decoded layer we use
                // 2. CLAP (Clean Aperture) - crop
                // 3. ISCL (Image Scaling) - scale
                // 4. IROT (Rotation) - rotate via CSS
                // 5. IMIR (Mirror) - mirror via CSS
                
                // 1. LSEL - Layer Selection (placeholder)
                if (layerSelection) {
                    // NOTE: multi-layer selection not yet supported by the decoder.
                    // When it is, use layerSelection.layerId to pick the layer.
                    console.debug('LSEL: Layer selection requested for layer_id=' + layerSelection.layerId + ' (not yet implemented)');
                }
                
                // Calculate output dimensions after CLAP and ISCL
                var outputWidth = sourceWidth;
                var outputHeight = sourceHeight;
                var cropParams = null;
                
                // 2. CLAP - Clean Aperture (crop)
                if (cleanAperture) {
                    outputWidth = cleanAperture.cropWidth;
                    outputHeight = cleanAperture.cropHeight;
                    cropParams = {
                        sx: cleanAperture.cropX,
                        sy: cleanAperture.cropY,
                        sw: cleanAperture.cropWidth,
                        sh: cleanAperture.cropHeight
                    };
                }
                
                // 3. ISCL - Image Scaling
                if (scale && scale.targetWidthD && scale.targetHeightD) {
                    // Per spec: ceil((input * numerator) / denominator)
                    outputWidth = Math.ceil((outputWidth * scale.targetWidthN) / scale.targetWidthD);
                    outputHeight = Math.ceil((outputHeight * scale.targetHeightN) / scale.targetHeightD);
                }
                
                // Determine canvas dimensions (before CSS rotation/mirror)
                var canvasWidth = outputWidth;
                var canvasHeight = outputHeight;
                
                // Set up the canvas
                self._heifCanvas.width = canvasWidth;
                self._heifCanvas.height = canvasHeight;
                
                // Draw with transformations applied
                self._drawTransformedImage(sourceWidth, sourceHeight, sourceData, cropParams, outputWidth, outputHeight);
                
                // Build CSS class list for rotation and mirror (applied after canvas drawing)
                var classList = [];
                
                // 4. IROT - Apply CSS rotation class for visual rotation
                if (rotation === 90 || rotation === 270) {
                    classList.push('heif-rotated-' + rotation);
                    // Swap displayed dimensions for 90/270 rotation
                    self._heifCanvas.style.width = canvasHeight + 'px';
                    self._heifCanvas.style.height = canvasWidth + 'px';
                } else if (rotation === 180) {
                    classList.push('heif-rotated-180');
                    self._heifCanvas.style.width = '';
                    self._heifCanvas.style.height = '';
                } else {
                    self._heifCanvas.style.width = '';
                    self._heifCanvas.style.height = '';
                }
                
                // 5. IMIR - Apply CSS mirror class if needed
                if (mirror) {
                    // horizontalAxis=true (axis=0): vertical mirroring (scaleY(-1))
                    // horizontalAxis=false (axis=1): horizontal mirroring (scaleX(-1))
                    if (mirror.horizontalAxis) {
                        classList.push('heif-mirrored-horizontal');
                    } else {
                        classList.push('heif-mirrored-vertical');
                    }
                }
                
                self._heifCanvas.className = classList.join(' ');
                
                self.callOnload();
            } else {
                self.drawErrorImage();
            }
        };
    };
    
    /**
     * Draw image with CLAP (crop) and ISCL (scale) transformations applied
     * @param {number} srcWidth - Source image width
     * @param {number} srcHeight - Source image height  
     * @param {Uint8ClampedArray} srcData - Source RGBA pixel data
     * @param {Object|null} cropParams - Crop parameters {sx, sy, sw, sh} or null
     * @param {number} outputWidth - Final output width
     * @param {number} outputHeight - Final output height
     */
    this._drawTransformedImage = function(srcWidth, srcHeight, srcData, cropParams, outputWidth, outputHeight) {
        // Create ImageData from source
        var srcImageData = this._context.createImageData(srcWidth, srcHeight);
        srcImageData.data.set(srcData);
        
        // Create temporary canvas for source image
        var srcCanvas = document.createElement('canvas');
        srcCanvas.width = srcWidth;
        srcCanvas.height = srcHeight;
        var srcCtx = srcCanvas.getContext('2d');
        srcCtx.putImageData(srcImageData, 0, 0);
        
        // Determine source rectangle (for CLAP)
        var sx = 0, sy = 0, sw = srcWidth, sh = srcHeight;
        if (cropParams) {
            sx = Math.max(0, cropParams.sx);
            sy = Math.max(0, cropParams.sy);
            sw = Math.min(cropParams.sw, srcWidth - sx);
            sh = Math.min(cropParams.sh, srcHeight - sy);
        }
        
        // Draw to output canvas with optional crop and scale
        // drawImage(image, sx, sy, sw, sh, dx, dy, dw, dh)
        this._context.drawImage(srcCanvas, sx, sy, sw, sh, 0, 0, outputWidth, outputHeight);
    };

    this._getFirstRootMetaItemByType = function (type) {
        // Snapshot already enumerated grid/iovl/tmap buckets at open time —
        // no need for a wasm round-trip (and the legacy
        // getItemListByType isn't on the modern reader anyway).
        var fi = self._fileInfo;
        if (!fi) return null;
        var bucket = (type === 'grid') ? fi.gridIds
                   : (type === 'iovl') ? fi.iovlIds
                   : (type === 'tmap') ? fi.tmapIds
                   : null;
        return (bucket && bucket.length > 0) ? bucket[0] : null;
    };

    this._getGridItemId = function () {
        return this._getFirstRootMetaItemByType("grid");
    };

    this._getIovlItemId = function () {
        return this._getFirstRootMetaItemByType("iovl");
    };

    this._getFileInfoCallback = function () {
        return function (payload) {
            if (payload.success !== true) {
                console.error("Error occurred while loading file info: " +
                    (payload.error || "(no detail)") +
                    " — " + (self._url || "(unknown URL)"));
                self.drawErrorImage();
                return;
            }

            self._fileInfo = payload;

            if ((self._frameRate === null || self._frameRate === undefined) && self.hasVideo()) {
                self._frameRate = DEFAULT_FRAMERATE;
            }

            var iovlItemId = self._getIovlItemId();
            var gridItemId = self._getGridItemId();

            if (gridItemId !== null) {
                // Initialize a derived grid (defined in the file)
                self._masterContextId = self._imageProvider.getMasterContextId(self._fileInfo);
                self._derivedGridObject = new DerivedGridObject(
                    self._heifCanvas, self._fileInfo, self._heifReader, self._imageProvider, self._masterContextId, gridItemId);
            } else if (iovlItemId !== null) {
                // Initialize a derived overlay (defined in the file)
                self._masterContextId = self._imageProvider.getMasterContextId(self._fileInfo);
                self._derivedIovlObject = new DerivedIovlObject(
                    self._heifCanvas, self._fileInfo, self._heifReader, self._imageProvider, self._masterContextId, iovlItemId);
            } else if ((self.hasAnimation() || self.hasVideo()) && self.hasImageCollection() && !self.isForcedCoverView() && !self.isForcedGridView()) {
                self._burstObject = new Burst(self._heifCanvas, self._fileInfo, self._heifReader, self._imageProvider);
            } else if ((self.hasAnimation() || self.hasVideo()) && !self.isForcedCoverView() && !self.isForcedGridView()) {
                // Initialize an animation
                self._masterIds = self._imageProvider.getMasterIds(self._fileInfo);
                self._masterContextId = self._imageProvider.getMasterContextId(self._fileInfo);
                self._displayWidth = self._imageProvider.getDisplayWidth(self._fileInfo, self._masterContextId);
                self._displayHeight = self._imageProvider.getDisplayHeight(self._fileInfo, self._masterContextId);
                self._heifCanvas.width = self._displayWidth;
                self._heifCanvas.height = self._displayHeight;

                self.initAnimation();
                if (self.hasAnimation() || self._autoplay) {
                    if (self._videoControlObject) {
                        self._videoControlObject.play();
                    } else {
                        self.startAnimation();
                    }
                }

                if (self.hasVideo())
                {
                    self._loadCoverImage();
                }

            } else if (self.hasImageCollection() || self.isForcedGridView() || self.isForcedCoverView()) {
                // Initialize an image collection
                if (self.isForcedCoverView()) {
                    // Initialize a cover view object
                    self._collectionViewObject = new CoverObject(self._heifCanvas, self._fileInfo, self._heifReader, self._imageProvider);
                } else {
                    // Initialize a grid object
                    self._collectionViewObject = new GridObject(self._heifCanvas, self._fileInfo, self._heifReader, self._imageProvider);
                }
                self._collectionViewObject.animate();

            } else if (self._fileInfo.fileFeature.hasSingleImage) {
                // Initiliaze a still image
                self._masterIds = self._imageProvider.getMasterIds(self._fileInfo);
                self._masterContextId = self._imageProvider.getMasterContextId(self._fileInfo);
                self._displayWidth = self._imageProvider.getDisplayWidth(self._fileInfo, self._masterContextId);
                self._displayHeight = self._imageProvider.getDisplayHeight(self._fileInfo, self._masterContextId);
                self._heifCanvas.width = self._displayWidth;
                self._heifCanvas.height = self._displayHeight;
                self._loadCoverImage();
            }
        };
    };

    this._getImageDataCallback = function () {
        return function (payload) {
            // Clear the all-frames request flag
            delete self._sentRequests['__all_frames__'];
            
            if (payload.success) {
                for (var i = 0; i < payload.frames.length; i++) {
                    var frame = payload.frames[i];
                    self._frameCache.put(frame.itemId, frame.canvasFrameData);
                    delete self._sentRequests[frame.itemId];

                    if (frame.itemId === self._drawPendingId) {
                        self._drawPendingId = -1;
                        self.drawCanvasFrameData(self._displayWidth, self._displayHeight, frame.canvasFrameData);
                    }
                }
            } else {
                console.error(" Error getting frame data", payload);
            }
        };
    };

    this._getNextFrameOrderIndex = function () {
        if (this._timestampsLastDrawnIndex < 0) {
            return 0;
        }

        var nextFrameOrderIndex = 0;
        if (this._isDirectionForward) {
            nextFrameOrderIndex = (this._timestampsLastDrawnIndex + 1) % this._timestamps.length;
        } else {
            if (this._timestampsLastDrawnIndex > 0) {
                nextFrameOrderIndex = this._timestampsLastDrawnIndex - 1;
            } else {
                nextFrameOrderIndex = this._timestamps.length - 1;
            }
        }

        return nextFrameOrderIndex;
    };

    this._getNextFrames = function (previousIndex) {
        if (previousIndex < 0) {
            if (this._isDirectionForward) {
                previousIndex = -1;
            } else {
                previousIndex = 0;
            }
        }

        var nextFrames = [];

        var i = previousIndex;
        var bufferMaxLength = Math.min(PREFERRED_BUFFER_SIZE, this._timestamps.length);
        if (this._isDirectionForward) {
            while (nextFrames.length < bufferMaxLength) {
                ++i;
                nextFrames.push(i % this._timestamps.length);
            }
        } else {
            while (nextFrames.length < bufferMaxLength) {
                --i;
                if (i < 0) {
                    i = this._timestamps.length - 1;
                }
                nextFrames.push(i % this._timestamps.length);
            }
        }

        return nextFrames;
    };

    this._loadCoverImage = function () {
        // Modern reader: the cover/primary item is whatever
        // `primaryItemId` resolves to on the snapshot. Sentinel 0
        // means the file has no `pitm` set, in which case the spec
        // allows any consumer choice — we follow the legacy
        // behaviour and use the first master image.
        var coverImage = self._fileInfo.fileFeature.hasCoverImage
            ? self._fileInfo.primaryItemId
            : self._masterIds[0];

        var thumbnailContextId = self._imageProvider.getThumbnailContextId(self._fileInfo);
        if (thumbnailContextId !== null) {
            var thumbnailId = self._imageProvider.getThumbnailIdByItemId(self._fileInfo, thumbnailContextId, coverImage);
            if (thumbnailId !== null) {
                self._imageProvider.requestImageData(thumbnailContextId, thumbnailId, self._getDrawImageDataCallback());
            }
        }

        if (self._heifCanvas.preload && self._heifCanvas.preload === "auto") {
            self._isPreloading = true;
            self._imageProvider.requestImageData(self._masterContextId, [coverImage], function (coverImageData) {
                self._getDrawImageDataCallback()(coverImageData); // Show primary frame before requesting all
                self._updateFrameBuffer(true);
            });
        } else {
            self._imageProvider.requestImageData(self._masterContextId, [coverImage], self._getDrawImageDataCallback());
        }
    };

this._updateFrameBuffer = function (requestAll) {
    // Safety check - don't proceed if disposed or provider missing
    if (self._isDisposed || !self._imageProvider) {
        return;
    }

    var requestFrames = [];
    
    // Check if this is a track-based sequence (video/animation)
    // Track samples share a single encoded bitstream and must be decoded together
    var isTrackBased = self._fileInfo && 
                       self._fileInfo.trackProperties && 
                       self._fileInfo.trackProperties.length > 0 &&
                       !self._fileInfo.rootLevelMetaBoxProperties;
    
    if (requestAll) {
        requestFrames = self._masterIds.slice(0);
        self._frameCache = new Cache(requestFrames.length, requestFrames.length);
    } else {
        // For track-based sequences (image sequences/animations), 
        // all frames were pre-loaded on first requestAll=true call.
        // Don't request individual frames as they share a single bitstream.
        if (isTrackBased) {
            // Check if we already have all frames cached
            var hasMissingFrames = false;
            for (var i = 0; i < self._timestamps.length && !hasMissingFrames; i++) {
                if (!self._frameCache.get(self._timestamps[i].id)) {
                    hasMissingFrames = true;
                }
            }
            
            if (hasMissingFrames) {
                // Need to re-request ALL frames (shouldn't happen normally)
                // This could happen if cache was cleared
                if (!self._sentRequests['__all_frames__']) {
                    console.debug('Track-based sequence: re-requesting all frames');
                    self._sentRequests['__all_frames__'] = true;
                    requestFrames = self._masterIds.slice(0);
                    self._frameCache = new Cache(requestFrames.length, requestFrames.length);
                }
            }
            // If no missing frames, just return - loop from cache
            if (requestFrames.length === 0) {
                return;
            }
        } else {
            // For meta box items, can request individual frames
            var nextFrameIndices = this._getNextFrames(this._timestampsLastDrawnIndex);
            
            nextFrameIndices.forEach(function(frameIndex) {
                var id = self._timestamps[frameIndex].id;
                if (!self._frameCache.get(id) && !self._sentRequests[id]) {
                    requestFrames.push(id);
                    self._sentRequests[id] = true;
                }
            });
        }
    }

    // Only request if we actually have frames to request
    if (requestFrames.length > 0 && self._imageProvider) {
        self._imageProvider.requestImageData(
            self._imageProvider.getMasterContextId(self._fileInfo), 
            requestFrames, 
            self._getImageDataCallback()
        );
    }
};

    /** Draw decoded image on the display canvas */
    this._drawImageOnCanvas = function(imageData) {
        if (!imageData || !imageData.frames || imageData.frames.length === 0) {
            console.error('[HEIFObject] No frames to draw');
            return;
        }

        var frame = imageData.frames[0]; // Get first frame
        
        // CRITICAL: Use the frame's dimensions (which are post-rotation)
        // NOT the display dimensions from the file metadata
        var canvasWidth = frame.width;
        var canvasHeight = frame.height;
        
        console.debug('[HEIFObject] Drawing frame:', {
            itemId: frame.itemId,
            width: frame.width,
            height: frame.height,
            rotation: frame.rotation,
            canvasWidth: canvasWidth,
            canvasHeight: canvasHeight
        });
        
        // Set canvas to rotated dimensions
        self._heifCanvas.width = canvasWidth;
        self._heifCanvas.height = canvasHeight;
        
        // Create image data with rotated dimensions
        var imgData = self._context.createImageData(canvasWidth, canvasHeight);
        imgData.data.set(frame.canvasFrameData);
        
        // Draw to canvas
        self._context.putImageData(imgData, 0, 0);
    };

    /** Clean up HEIFObject resources */
    this.dispose = function() {
        // Clean up ImageProvider (which cleans up HEIFReader)
        if (this.imageProvider) {
            this.imageProvider.dispose();
            this.imageProvider = null;
        }
        // Clean up any other references
        this._fileInfo = null;

        // Mark as disposed immediately to stop any pending callbacks
        self._isDisposed = true;

        // Stop animation first to prevent further timer callbacks
        self._isStopped = true;
        if (self.animationIntervalTimerId) {
            clearTimeout(self.animationIntervalTimerId);
            self.animationIntervalTimerId = null;
        }

        // Clear frame cache
        if (self._frameCache && typeof self._frameCache.clear === 'function') {
            self._frameCache.clear();
        }
        self._frameCache = null;
        self._sentRequests = {};

        // Dispose image provider last
        if (self._imageProvider && typeof self._imageProvider.dispose === 'function') {
            self._imageProvider.dispose();
        }
        self._imageProvider = null;
        // Modern reader: explicit close() releases the wasm-side handle
        // and the staged file bytes. Legacy reader had no close().
        if (self._heifReader && typeof self._heifReader.close === 'function') {
            try { self._heifReader.close(); } catch (e) { /* swallow */ }
        }
        self._heifReader = null;
        self._fileInfo = null;
    };
}
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

function Burst (heifCanvas, fileInfo, heifReader, imageProvider) {

    var self = this;

    this._ctx = heifCanvas.getContext('2d');
    this._heifCanvas = heifCanvas;
    this._heifReader = heifReader;
    this._fileInfo = fileInfo;
    this._canvases = [];
    this._canvasIdToItemIdMap = {};
    this._imageProvider = imageProvider;
    this._masterContextId = fileInfo.rootLevelMetaBoxProperties.contextId;
    this._masterIds = imageProvider.getMetaBoxMasterIds(fileInfo);

    this._imageProvider.requestImageData(this._masterContextId, this._masterIds, function (payload) {

        var canvasContainer = document.createElement("div");
        canvasContainer.className = "heif-burst-thumbnail-canvas-container";

        var source = $("#burst-template").html();
        var template = Handlebars.compile(source);

        var data = {
            canvases: [],
            mainCanvasColSpan: payload.frames.length
        };

        for (var i = 0; i < payload.frames.length; i++) {
            var width = payload.frames[i].width;
            var height = payload.frames[i].height;
            var itemCanvas = document.createElement("canvas");
            itemCanvas.width = width;
            itemCanvas.height = height;
            itemCanvas.className = "heif-burst-thumbnail-canvas";
            var context = itemCanvas.getContext("2d");
            var imageData = context.createImageData(width, height);
            imageData.data.set(payload.frames[i].canvasFrameData);
            context.putImageData(imageData, 0, 0);

            self._canvases.push(itemCanvas);
            itemCanvas.id = "heif-burst-thumbnail-canvas-" + i;
            data.canvases.push({canvasId: itemCanvas.id, canvas: itemCanvas});

            self._canvasIdToItemIdMap[itemCanvas.id] = payload.frames[i].itemId;

            itemCanvas.addEventListener("click", function() {
                self._imageProvider.requestImageData(
                    self._masterContextId, self._canvasIdToItemIdMap[this.id], self._onImageDataReceived);
            });
        }

        var modalContent = document.getElementById("heif-examples-modal-content");
        modalContent.appendChild(canvasContainer);
        canvasContainer.innerHTML = template(data);

        for (i = 0; i < data.canvases.length; i++) {
            var container = document.getElementById(data.canvases[i].canvasId);
            if (container) {
                container.appendChild(data.canvases[i].canvas);
            }
        }

        self._imageProvider.requestImageData(
            self._masterContextId, self._canvasIdToItemIdMap["heif-burst-thumbnail-canvas-0"], self._onImageDataReceived);
    });

    this._onImageDataReceived = function (payload) {
        if (payload.success) {
            var mainCanvas = document.getElementById("heif-burst-main-canvas");
            var mainCanvasContext = mainCanvas.getContext("2d");
            var mainCanvasImageData = mainCanvasContext.getImageData(0, 0, payload.displayWidth, payload.displayHeight);
            mainCanvas.width = payload.displayWidth;
            mainCanvas.height = payload.displayHeight;
            mainCanvasImageData.data.set(payload.frames[0].canvasFrameData);
            mainCanvasContext.putImageData(mainCanvasImageData, 0, 0);
        }
    };
}
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

/*
 * This script file contains a simple LRU cache implementation
*/

"use strict";

/** @constructor */
function Cache(minimumCapacity, maximumCapacity) {
    this.minimumCapacity = minimumCapacity;
    this.maximumCapacity = maximumCapacity;
    this._map = {};
    this._keys = [];
}

Cache.prototype.remove = function (count) {
    var self = this;
    this._keys.sort(function (first, second) {
        if (self._map[first].timestamp > self._map[second].timestamp) {
            return -1;
        } else if (self._map[first].timestamp < self._map[second].timestamp) {
            return 1;
        } else {
            return 0;
        }
    });

    for (var i = 0; i < count && this._keys.length > 0; ++i) {
        delete this._map[this._keys[this._keys.length - 1]];
        this._keys.pop();
    }
};

Cache.prototype.put = function (key, value) {
    if (this._map[key] === undefined) {
        this._keys.push(key);
    }

    this._map[key] = { value: value, timestamp: Date.now() };

    if (this._keys.length > this.maximumCapacity) {
        this.remove((this._keys.length) - this.minimumCapacity);
    }
};

Cache.prototype.get = function(key) {
    if (!this._map[key]) {
        return undefined;
    }

    this._map[key].timestamp = Date.now();

    return this._map[key].value;
};

Cache.prototype.clear = function() {
    this._map = {};
    this._keys = [];
};
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

function Cell(initX, initY, width, height, image) {
    var self = this;
    self._initX = initX;
    self._initY = initY;
    self._pos = {x: initX, y: initY};
    self._zIndex = 0;
    self._initWidth = width;
    self._initHeight = height;
    self._width = width;
    self._height = height;
    self._image = image;
    self._margin = 0;
    self._xDisplacement = 0;

    this.contains = function (x, y) {
        return x >= self._pos.x - self._width / 2 && x <= self._pos.x + self._width / 2 &&
            y >= self._pos.y - self._height / 2 && y <= self._pos.y + self._height / 2;
    };

    // Test if the cell excludes the other cell
    this.excludes = function (other) {

        var w2 = self._width / 2;
        var l  = self._pos.x - w2;
        var r  = self._pos.x + w2;
        w2 = other._width / 2;
        var lo = other._pos.x - w2;
        var ro = other._pos.x + w2;

        if (r < lo || l > ro) {
            return true;
        }

        var h2 = self._height / 2;
        var t  = self._pos.y - h2;
        var b  = self._pos.y + h2;
        h2 = other._height / 2;
        var to = other._pos.y - h2;
        var bo = other._pos.y + h2;

        return t < bo || b > to;
    };

    // Test if the cell intersects (but doesn't contain) the other cell
    this.intersects = function (other) {

        var h2 = self._height / 2;
        var w2 = self._width / 2;
        var l  = self._pos.x - w2;
        var r  = self._pos.x + w2;
        var t  = self._pos.y - h2;
        var b  = self._pos.y + h2;

        if (other.contains(l, t) || other.contains(l, b) || other.contains(r, t) || other.contains(r, b)) {
            return true;
        }

        h2 = other._height / 2;
        w2 = other._width / 2;
        l  = other._pos.x - w2;
        r  = other._pos.x + w2;
        t  = other._pos.y - h2;
        b  = other._pos.y + h2;

        return self.contains(l, t) || self.contains(l, b) || self.contains(r, t) || self.contains(r, b);
    };

    this.size = function () {
        return {width: self._width, height: self._height};
    };

    this.image = function () {
        return self._image;
    };

    this.initialSize = function () {
        return {width: self._initWidth, height: self._initHeight};
    };

    this.setSize = function (width, height) {
        self._width = width;
        self._height = height;
    };

    this.pos = function () {
        return self._pos;
    };

    this.initialPos = function () {
        return {x: self._initX, y: self._initY};
    };

    this.setPos = function (x, y) {
        self._pos.x = x;
        self._pos.y = y;
    };

    this.margin = function () {
        return self._margin;
    };

    this.setMargin = function (value) {
        self._margin = value;
    };

    this.xDisplacement = function(x) {
        return self._xDisplacement;
    };

    this.setXDisplacement = function(x) {
        self._xDisplacement = x;
    };

    this.render = function (ctx) {
        self.renderImage(ctx, self._image._thumbnailCanvas);
    };

    this.renderImage = function (ctx, image) {
        ctx.save();
        ctx.translate(self._pos.x + self._xDisplacement, self._pos.y);
        var m = self._margin;
        ctx.drawImage(image,
            -self._width  / 2 + m,
            -self._height / 2 + m,
             self._width  - m * 2,
             self._height - m * 2);
        ctx.restore();
    };

    this.reset = function () {
        self._pos = {x: self._initX, y: self._initY};
        self._zIndex = 0;
    };

    this.setZIndex = function (z) {
        self._zIndex = z;
    };

    this.zIndex = function () {
        return self._zIndex;
    };
}
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

/**
 * This script replaces all <img> and <video> tags containing a .heic file on
 * page load and replaces them with canvas elements.
 */

"use strict";

var FILE_EXTENSIONS = ["heic", "heif", "hmg"]; // Array of supported extensions

var UNIQUE_ID = Date.now();

var HEIF_OBJECTS = {};

var CANVAS_ATTRIBUTE_FILENAME = "filename";

var HEIF_REPLACED_STR = "__HEIF_replaced";

var DOM_SUBTREE_MODIFIED_STR = "DOMSubtreeModified";

var VIDEO_STR = "video";

var IMG_STR = "img";

function hasSupportedFileType(imageElement) {
    if (imageElement.src) {
        var re = /\.([^.]+)?$/;
        var match = re.exec(imageElement.src);
        if (match && match[1]) {
            var extension = match[1].toLowerCase();
            return FILE_EXTENSIONS.indexOf(extension) !== -1;
        }
    }

    return false;
}

function createHeifCanvas(element) {
    if (!hasSupportedFileType(element)) {
        return null;
    }

    var canvas = document.createElement("canvas");

    // Get absolute file path of the URL
    var url = document.createElement("a");
    url.href = element.src;

    canvas.setAttribute(CANVAS_ATTRIBUTE_FILENAME, url.pathname);
    canvas.width  = element.width;
    canvas.height = element.height;
    canvas.loop = element.loop;
    canvas.autoplay = element.autoplay;
    canvas.controls = element.controls;
    canvas.onload = element.onload;
    canvas.preload = element.preload;
    canvas.user = element.user;

    var fpsAttribute = element.getAttribute("fps");
    if (fpsAttribute !== null && fpsAttribute !== undefined) {
        var fps = Number(fpsAttribute);
        if (fps >= 0) {
            canvas.fps = fps;
        }
    }

    canvas.type = element.getAttribute("type");

    // Canvas must have unique id
    if (element.id !== "") {
        canvas.id = element.id;
    } else {
        canvas.id = UNIQUE_ID;
        UNIQUE_ID++;
    }

    if (element.className !== "") {
        canvas.className = element.className;
    }

    if (element.style.cssText !== "") {
        canvas.style.cssText = element.style.cssText;
    }

    // Copy data-original-src for context menu download functionality
    if (element.dataset && element.dataset.originalSrc) {
        canvas.dataset.originalSrc = element.dataset.originalSrc;
    } else if (element.src) {
        // Fallback: use the img src as original source
        canvas.dataset.originalSrc = element.src;
    }

    return canvas;
}

function processImageElements() {
    var imageElements = document.getElementsByTagName(IMG_STR);
    for (var i = 0; i < imageElements.length; ++i) {
        var imageElement = imageElements[i];
        imageElement.removeEventListener(DOM_SUBTREE_MODIFIED_STR, processImageElements, true);

        var heifCanvas = createHeifCanvas(imageElement);
        if (heifCanvas) {
            var heifObject = new HEIFObject(heifCanvas, IMG_STR)
            HEIF_OBJECTS[heifCanvas.id] = heifObject;
            imageElement.parentNode.insertBefore(heifCanvas, imageElement);
            imageElement.setAttribute("class", HEIF_REPLACED_STR);
            heifObject.requestFileInfo();
        } else {
            imageElement.addEventListener(DOM_SUBTREE_MODIFIED_STR, processImageElements, true);
        }
    }

    var replacedElement = document.querySelector("." + HEIF_REPLACED_STR);
    while (replacedElement) {
        replacedElement.parentNode.removeChild(replacedElement);
        replacedElement = document.querySelector("." + HEIF_REPLACED_STR);
    }
}

function processVideoElements() {
    var videoElements = document.getElementsByTagName(VIDEO_STR);
    for (var i = 0; i < videoElements.length; ++i) {
        var videoElement = videoElements[i];
        videoElement.removeEventListener(DOM_SUBTREE_MODIFIED_STR, processVideoElements, true);

        var heifCanvas = createHeifCanvas(videoElement);
        if (heifCanvas && heifCanvas.id) {
            HEIF_OBJECTS[heifCanvas.id] = new HEIFObject(heifCanvas, VIDEO_STR);
            var videoContainer = createVideoContainer(heifCanvas, HEIF_OBJECTS[heifCanvas.id]);
            videoElement.parentNode.insertBefore(videoContainer, videoElement);
            videoElement.setAttribute("class", HEIF_REPLACED_STR);
            HEIF_OBJECTS[heifCanvas.id].requestFileInfo();
        } else {
            videoElement.addEventListener(DOM_SUBTREE_MODIFIED_STR, processVideoElements, true);
        }
    }

    var replacedElement = document.querySelector("." + HEIF_REPLACED_STR);
    while (replacedElement) {
        replacedElement.parentNode.removeChild(replacedElement);
        replacedElement = document.querySelector("." + HEIF_REPLACED_STR);
    }
}

var canvasObserver = new MutationObserver(function(mutations) {
    mutations.forEach(function(mutation) {
        mutation.removedNodes.forEach(function(node) {
            // Check if removed node is a HEIF canvas or contains one
            if (node.nodeType === 1) { // Element node
                var canvasElements = [];
                
                // Check if node itself is a canvas
                if (node.tagName === 'CANVAS' && node.hasAttribute(CANVAS_ATTRIBUTE_FILENAME)) {
                    canvasElements.push(node);
                }
                
                // Check if node contains canvas elements
                if (node.querySelectorAll) {
                    var canvases = node.querySelectorAll('canvas[' + CANVAS_ATTRIBUTE_FILENAME + ']');
                    for (var i = 0; i < canvases.length; i++) {
                        canvasElements.push(canvases[i]);
                    }
                }
                
                // Dispose HEIFObjects for removed canvases
                canvasElements.forEach(function(canvas) {
                    if (HEIF_OBJECTS[canvas.id]) {
                        // CRITICAL: Always release our references when the canvas leaves the DOM
                        if (!document.body.contains(canvas)) {
                            console.debug('[Cleanup] Canvas genuinely removed from DOM, disposing:', canvas.id);
                            var heifObj = HEIF_OBJECTS[canvas.id];
                            try {
                                if (heifObj && typeof heifObj.dispose === 'function') {
                                    heifObj.dispose();
                                }
                            } catch (e) {
                                console.error('[Cleanup] Error disposing object:', canvas.id, e);
                            }
                            delete HEIF_OBJECTS[canvas.id];
                        } else {
                            console.debug('[Cleanup] Canvas still in DOM despite parent removal, keeping:', canvas.id);
                        }
                    }
                });
            }
        });
    });
});

// Start observing - wait for DOM to be ready
if (document.body) {
    canvasObserver.observe(document.body, {
        childList: true,
        subtree: true
    });
} else {
    document.addEventListener('DOMContentLoaded', function() {
        canvasObserver.observe(document.body, {
            childList: true,
            subtree: true
        });
    });
}

window.addEventListener('beforeunload', function() {
    console.debug('[Cleanup] Page unload - disposing all HEIF objects');
    
    for (var canvasId in HEIF_OBJECTS) {
        if (HEIF_OBJECTS.hasOwnProperty(canvasId)) {
            try {
                if(HEIF_OBJECTS[canvasId].imageProvider) HEIF_OBJECTS[canvasId].imageProvider.dispose();
                HEIF_OBJECTS[canvasId].dispose();
            } catch (e) {
                console.error('[Cleanup] Error disposing object:', canvasId, e);
            }
        }
    }
    
    HEIF_OBJECTS = {};
});
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

function CoverAnimation(canvas, thbHeight) {

    var self = this;
    self._canvas = canvas;
    self._cell = null;
    self._curve = new Curve2(15, 1.0);
    self._curveIndex = 0;
    self._isInProgress = false;
    self._target = null;
    self._source = null;
    self._zoomIn = false;
    self._thbHeight = thbHeight;
    self._margin = 1;

    this.cell = function () {
        return self._cell;
    };

    this.initialize = function (cell, zoomIn, event) {

        self._curveIndex = 0;
        self._zoomIn = zoomIn;
        self._cell = cell;

        self._source = {
            x: cell.pos().x,
            y: cell.pos().y,
            w: cell.size().width,
            h: cell.size().height
        };

        if (zoomIn) {

            var zoom = 2;
            var newW = cell.initialSize().width * zoom;
            var newH = cell.initialSize().height * zoom;
            var newX = cell.pos().x;

            if (event) {

                var clientX;
                if (event.touches) {
                    clientX = event.touches[0].clientX;
                } else {
                    clientX = event.clientX;
                }

                var rect = self._canvas.getBoundingClientRect();
                newX = (clientX - rect.left) * self._canvas.width / rect.width - cell.xDisplacement();
            }

            var newY = self._canvas.height - newH / 2 - self._margin;
            if (newW / 2 + newX + cell.xDisplacement() >= self._canvas.width) {
                newX = self._canvas.width - newW / 2 - self._margin - cell.xDisplacement();
            } else if (newX + cell.xDisplacement() - newW / 2 < 0) {
                newX = newW / 2 + self._margin - cell.xDisplacement();
            }

            self._target = {
                x: newX,
                y: newY,
                w: newW,
                h: newH
            };
        } else {
            self._target = {
                x: cell.initialPos().x,
                y: cell.initialPos().y,
                w: cell.initialSize().width,
                h: cell.initialSize().height
            };
        }
    };

    this.isZoomIn = function () {
        return self._zoomIn;
    };

    this.isInProgress = function () {
        return self._isInProgress;
    };

    this.curveVal = function () {
        return self._curve.value(self._curveIndex);
    };

    this.update = function () {
        if (self._cell) {

            var curveVal = self._curve.value(self._curveIndex);

            // Animate position
            var newX = self._source.x + (self._target.x - self._source.x) * curveVal;
            var newY = self._source.y + (self._target.y - self._source.y) * curveVal;
            self._cell.setPos(newX, newY);

            // Animate size
            var newWidth = self._source.w + (self._target.w - self._source.w) * curveVal;
            var newHeight = self._source.h + (self._target.h - self._source.h) * curveVal;
            self._cell.setSize(newWidth, newHeight);

            if (self._curveIndex + 1 < self._curve.length()) {
                self._curveIndex++;
                self._isInProgress = true;
            } else {
                self._isInProgress = false;
                self._cell.setPos(self._target.x, self._target.y);
                self._cell.setSize(self._target.w, self._target.h);
            }
        }

        return self._isInProgress;
    };

    this.target = function () {
        return this._target;
    };
}

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

function CoverObject(canvas, fileInfo, heifReader, imageProvider) {

    var self = this;

    self._activeThb = null;
    self._animations = [];
    self._canvas = canvas;
    self._clientX = null; // Latest mouse or touch X
    self._clientY = null; // Latest mouse or touch Y
    self._context = self._canvas.getContext("2d");
    self._handlersSet = false;
    self._heifReader = heifReader;
    self._imageProvider = imageProvider;
    self._fileInfo = fileInfo;
    self._frameCount = 0;
    self._frameHeight = 0;
    self._frameWidthv = 0;
    self._imageData = null;
    self._margin = 1.0;
    self._masterIds = null;
    self._masterContextId = null;
    self._prevThb = null;
    self._selected = null;
    self._thumbnails = [];
    self._thumbnailHeight = 0;
    self._thumbnailIds = null;
    self._thumbnailWidth = 0;
    self._thumbnailContextId = null;

    function calculateThumbnailSize(displayWidth, displayHeight) {
        var thbRatio = 0.1;
        self._thumbnailWidth = displayWidth * thbRatio;
        if (self._thumbnailIds.length * self._thumbnailWidth < displayWidth) {
            thbRatio = 1.0 / self._thumbnailIds.length;
            self._thumbnailHeight = displayHeight * thbRatio;
            self._thumbnailWidth = displayWidth * thbRatio;
        }
        self._thumbnailHeight = displayHeight * thbRatio;
    }

    function initialize() {
        if (self._fileInfo.rootLevelMetaBoxProperties || self._fileInfo.trackProperties) {
            self._masterContextId = self._imageProvider.getMasterContextId(self._fileInfo);
            self._masterIds = self._imageProvider.getMasterIds(self._fileInfo);

            self._thumbnailContextId = self._imageProvider.getThumbnailContextId(self._fileInfo);
            self._thumbnailIds = self._imageProvider.getThumbnailIds(self._fileInfo);

            var hasThumbnails = self._thumbnailIds && self._thumbnailIds.length > 0;
            if (!hasThumbnails) {
                self._thumbnailIds = self._masterIds;
                self._thumbnailContextId = self._masterContextId;
            }

            var displayWidth = self._imageProvider.getDisplayWidth(self._fileInfo, self._masterContextId);
            var displayHeight = self._imageProvider.getDisplayHeight(self._fileInfo, self._masterContextId);

            calculateThumbnailSize(displayWidth, displayHeight);

            self._canvas.width = displayWidth;
            self._canvas.height = displayHeight + self._thumbnailHeight;

            self._frameCount = self._masterIds.length;
            self._frameHeight = displayHeight;
            self._frameWidth = displayWidth;

            self._imageProvider.requestImageData(self._thumbnailContextId, self._thumbnailIds, onThumbnailsReceived);

            var coverImageId = self._fileInfo.fileFeature.hasCoverImage
                ? self._fileInfo.primaryItemId
                : self._masterIds[0];

            self._imageProvider.requestImageData(self._masterContextId, [coverImageId], onFrameDataReceived);

        } else {
            alert("Not a meta or trak file: " + self._heifReader.url());
        }
    }

    function onFrameDataReceived(payload) {
        if (payload.success) {
            self._imageData = self._context.getImageData(0, 0, payload.displayWidth, payload.displayHeight);
            self._imageData.data.set(payload.frames[0].canvasFrameData);
            zoomOutAll();
            render(true);
        }
    }

    function updateMousePosition(event) {
        if (event.touches) {
            self._clientX = event.touches[0].clientX;
            self._clientY = event.touches[0].clientY;
        } else {
            self._clientX = event.clientX;
            self._clientY = event.clientY;
        }
    }

    function getThumbnailAtMouse() {

        if (!self._thumbnails.length) {
            return null;
        }

        if (self._clientX && self._clientY) {
            var thumbnailMinYCoord = self._canvas.height - self._thumbnailHeight;
            var rect = self._canvas.getBoundingClientRect();
            var y = (self._clientY - rect.top) * self._canvas.height / rect.height;
            if (y >= thumbnailMinYCoord) {
                var thumbnailIndex = (self._clientX - rect.left) * self._thumbnails.length / rect.width | 0;
                return self._thumbnails[thumbnailIndex];
            }
        }

        return null;
    }

    function handleMouseMove(event) {
        updateMousePosition(event);
        var hoveredThumbnail = getThumbnailAtMouse();
        if (hoveredThumbnail) {
            self._prevThb = self._activeThb;
            self._activeThb = hoveredThumbnail;
            if (self._prevThb !== self._activeThb) {
                cancelAllAnimationsButActive();
                if (!reinitializeAnimation(self._activeThb, true)) {
                    var animation = new CoverAnimation(self._canvas, self._thumbnailHeight);

                    // Align with mouse cursor only if thumbnails are "stacked"
                    if (isStacked()) {
                        animation.initialize(self._activeThb, true, event);
                    } else {
                        animation.initialize(self._activeThb, true, null);
                    }

                    self._animations.push(animation);
                }
                if (self._prevThb && !reinitializeAnimation(self._prevThb, false)) {
                    var animation = new CoverAnimation(self._canvas, self._thumbnailHeight);
                    animation.initialize(self._prevThb, false, null);
                    self._animations.push(animation);
                }
            }
        } else {
            zoomOutAll();
            self._activeThb = null;
            self._prevThb = null;
        }
    }

    function isStacked() {
        return self._thumbnails.length * self._thumbnailWidth > self._canvas.width * 1.05;
    }

    function onThumbnailsReceived(payload) {
        if (payload.success) {

            createThumbnails(payload);

            if (!self._handlersSet) {
                self._canvas.addEventListener("mouseup", function(event) {
                    updateMousePosition(event);
                    self._selected = getThumbnailAtMouse();
                    if (self._selected) {
                        self._imageProvider.requestImageData(self._masterContextId, [self._selected.image().id()], onFrameDataReceived);
                    }
                });

                self._canvas.addEventListener("touchend", function(event) { // touchend event doesn't come with position information
                    self._selected = getThumbnailAtMouse();
                    if (self._selected) {
                        self._imageProvider.requestImageData(self._masterContextId, [self._selected.image().id()], onFrameDataReceived);
                    }
                });

                self._canvas.addEventListener("touchmove", handleMouseMove);

                self._canvas.addEventListener("mousemove", handleMouseMove);

                self._canvas.addEventListener("mouseout", function(event) {
                    zoomOutAll();
                    self._activeThb = null;
                    self._prevThb = null;
                });

                self._handlersSet = true;
            }

        } else {
            alert("Error when requesting image data of " + self._heifReader.url());
        }
    }

    function createThumbnails(payload) {
        self._activeThb = null;
        self._prevThb = null;
        self._thumbnails.length = 0;
        var count = payload.frames.length;
        for (var i = 0; i < count; ++i) {
            var thumbnailCanvas = document.createElement("canvas");
            var frame = payload.frames[i];
            thumbnailCanvas.width = frame.width;
            thumbnailCanvas.height = frame.height;

            var thumbnailContext = thumbnailCanvas.getContext("2d");
            var thumbnailImageData = thumbnailContext.getImageData(0, 0, frame.width, frame.height);
            thumbnailImageData.data.set(frame.canvasFrameData);
            thumbnailContext.putImageData(thumbnailImageData, 0, 0);

            var image = new Image(
                i, self._imageProvider.getItemIdByThumbnailId(self._fileInfo, self._thumbnailContextId, frame.itemId), thumbnailCanvas);
            var width = self._thumbnailWidth;
            var height = self._thumbnailHeight;
            var initX = i * self._canvas.width / count + width / 2;
            var initY = self._canvas.height - height / 2;
            var cell = new Cell(initX, initY, width, height, image);
            self._thumbnails.push(cell);
        }
    }

    function reinitializeAnimation(cell, zoomIn) {
        for (var i = 0; i < self._animations.length; i++) {
            if (self._animations[i].cell() === cell) {
                self._animations[i].initialize(cell, zoomIn);
                return true;
            }
        }
        return false;
    }

    function cancelAllAnimationsButActive() {
        for (var i = 0; i < self._animations.length; i++) {
            if (self._animations[i].cell() !== self._activeThb) {
                self._animations[i].initialize(self._animations[i].cell(), false);
            }
        }
    }

    function zoomOutAll() {
        var activeCanceled = false;
        for (var i = 0; i < self._animations.length; i++) {
            if (self._animations[i].cell() === self._activeThb) {
                activeCanceled = true;
            }
            if (self._animations[i].isZoomIn()) {
                self._animations[i].initialize(self._animations[i].cell(), false);
            }
        }
        if (self._activeThb && !activeCanceled) {
            var animation = new CoverAnimation(self._canvas, self._thumbnailHeight);
            animation.initialize(self._activeThb, false);
            self._animations.push(animation);
        }
    }

    function render(force) {
        renderImage();
        renderThumbnails();
    }

    function renderImage() {
        if (self._imageData) {
            self._context.putImageData(self._imageData, 0, 0);
        }
    }

    function spreadOutThumbnails() {
        var count = self._thumbnails.length;
        var i, thumbnail;
        var curveVal = 1.0;
        if (self._animations.length) {
            // Use curve value of the latest animation
            curveVal = self._animations[self._animations.length - 1].curveVal();
        }
        if (self._activeThb) {
            var distance, displacement;
            var activeIndex = self._activeThb.image().index();
            var initSpacing = self._thumbnails[1].initialPos().x - self._thumbnails[0].initialPos().x;
            var newSpacing = self._thumbnailWidth - initSpacing + 1;
            for (i = 0; i < count; i++) {
                thumbnail = self._thumbnails[i];
                if (thumbnail !== self._activeThb || thumbnail === self._selected) {
                    distance = i - activeIndex;
                    displacement = distance * newSpacing * curveVal;
                    thumbnail.setXDisplacement(displacement);
                }
                if (thumbnail !== self._activeThb) {
                    thumbnail.render(self._context);
                }
            }
            self._activeThb.render(self._context);
        } else {
            for (i = 0; i < count; i++) {
                thumbnail = self._thumbnails[i];
                thumbnail.setXDisplacement(thumbnail.xDisplacement() * (1.0 - curveVal));
                thumbnail.render(self._context);
            }
        }
    }

    function renderThumbnails() {
        self._context.fillStyle = "black";
        self._context.fillRect(0, self._canvas.height - self._thumbnailHeight, self._canvas.width, self._thumbnailHeight);

        if (isStacked()) {
            spreadOutThumbnails();
        } else {
            var i, thumbnail;
            var count = self._thumbnails.length;
            for (i = 0; i < count; i++) {
                thumbnail = self._thumbnails[i];
                if (thumbnail !== self._activeThb) {
                    thumbnail.render(self._context);
                }
            }
            if (self._activeThb) {
                self._activeThb.render(self._context);
            }
        }
    }

    var requestAnimFrame = window.requestAnimationFrame.bind(window);

    self.animate = function() {
        if (self._animations.length) {
            for (var i = 0; i < self._animations.length; i++) {
                var animation = self._animations[i];
                animation.update();
                if (!animation.isInProgress()) {
                    self._animations[i] = self._animations[self._animations.length - 1];
                    self._animations.pop();
                }
            }

            render();
        }

        // request new frame
        requestAnimFrame(function() {
            self.animate();
        });
    };

    initialize();
}

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

function Curve(length, power) {

    var self = this;
    self._values = [];
    self._length = length;

    self.value = function (index) {
        return self._values[index];
    };

    self.length = function () {
        return self._length;
    };

    // Builds a quadratic animation curve that can be
    // "boosted" by giving power > 1.0
    var build = function (length, power) {
        self._values.length = 0;
        var a = -1 / (length * length);
        var b =  2 / length;
        for (var i = 1; i < length + 1; i++) {
            self._values.push(Math.pow(a * i * i + b * i, power));
        }
    };

    build(length, power);
}

// Like Curve, but with a small bounce.
function Curve2(length, power) {

    var self = this;
    self._values = [];
    self._length = length;

    self.value = function (index) {
        return self._values[index];
    };

    self.length = function () {
        return self._length;
    };

    // Builds a quadratic animation curve that can be
    // "boosted" by giving power > 1.0
    var build = function (length, power) {
        self._values.length = 0;
        var a = -1 / (length * length);
        var b =  2 / length;
        for (var i = 1; i < length + 1; i++) {
            var base = a * i * i + b * i;
            var amp = 0.75;
            base = base + (length - i) * Math.sin(i * Math.PI / length) * amp / length;
            self._values.push(Math.pow(base, power));
        }
    };

    build(length, power);
}
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

function DerivedGridObject (heifCanvas, fileInfo, heifReader, imageProvider, masterContextId, gridItemId) {

    var self = this;

    this._cols = null;
    this._ctx = heifCanvas.getContext('2d');
    this._heifCanvas = heifCanvas;
    this._heifReader = heifReader;
    this._fileInfo = fileInfo;
    this._gridItemId = gridItemId;
    this._imageProvider = imageProvider;
    // grid() returns the modern shape: {outputWidth, outputHeight,
    // columns, rows, tileIds}. The legacy POD spelled the count fields
    // `columnsMinusOne` / `rowsMinusOne` (the on-wire encoding) and
    // listed tiles under `itemIds`. Bridge to those names so the rest
    // of this file (and any external consumer reading the same shape)
    // keeps working without a wider rename sweep.
    this._itemGridObject = self._heifReader.grid(gridItemId);
    if (this._itemGridObject) {
        var g = this._itemGridObject;
        g.columnsMinusOne = (g.columns || 1) - 1;
        g.rowsMinusOne    = (g.rows    || 1) - 1;
        g.itemIds         = g.tileIds || [];
    }
    this._canvases = [];
    this._masterContextId = masterContextId;
    this._rows = null;

    if (this._itemGridObject) {

        this._heifCanvas.width = this._itemGridObject.outputWidth;
        this._heifCanvas.height = this._itemGridObject.outputHeight;
        this._cols = this._itemGridObject.columnsMinusOne + 1;
        this._rows = this._itemGridObject.rowsMinusOne + 1;

        self._imageProvider.requestImageData(masterContextId, this._itemGridObject.itemIds, function (payload) {
            for (var i = 0; i < payload.frames.length; i++) {
                var width = payload.frames[i].width;
                var height = payload.frames[i].height;
                var itemCanvas = document.createElement("canvas");
                itemCanvas.width = width;
                itemCanvas.height = height;
                var context = itemCanvas.getContext("2d");
                var imageData = context.createImageData(width, height);
                imageData.data.set(payload.frames[i].canvasFrameData);
                context.putImageData(imageData, 0, 0);
                self._canvases.push(itemCanvas);
            }

            self._render();
        });
    }

    this._render = function () {
        self._ctx.save();
        self._ctx.fillStyle = "black";
        self._ctx.fillRect(0, 0, self._heifCanvas.width, self._heifCanvas.height);
        self._ctx.translate(0, 0);

        var itemIndex = 0;
        var imageWidth = self._heifCanvas.width / self._cols;
        var imageHeight = self._heifCanvas.height / self._rows;
        for (var j = 0; j < self._rows; j++) {
            for (var i = 0; i < self._cols; i++) {
                self._ctx.drawImage(
                    self._canvases[itemIndex],
                    self._heifCanvas.width * i / self._cols,
                    self._heifCanvas.height * j / self._rows,
                    imageWidth,
                    imageHeight);
                itemIndex++;
            }
        }

        self._ctx.restore();
    };
}
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

function DerivedIovlObject (heifCanvas, fileInfo, heifReader, imageProvider, masterContextId, IovlItemId) {

    var self = this;

    this._ctx = heifCanvas.getContext('2d');
    this._heifCanvas = heifCanvas;
    this._heifReader = heifReader;
    this._fileInfo = fileInfo;
    this._IovlItemId = IovlItemId;
    this._imageProvider = imageProvider;
    // overlay() returns the modern shape: {outputWidth, outputHeight,
    // fillRgba:[r,g,b,a], images:[{imageId, hOffset, vOffset}, ...]}.
    // The legacy POD used canvasFillValue / itemIds /
    // offsets[i].horizontal|vertical. Bridge inline so the renderer
    // below stays untouched.
    this._itemIovlObject = self._heifReader.overlay(IovlItemId);
    if (this._itemIovlObject) {
        var ov = this._itemIovlObject;
        ov.canvasFillValue = ov.fillRgba || [0, 0, 0, 0];
        ov.itemIds = (ov.images || []).map(function (e) { return e.imageId; });
        ov.offsets = (ov.images || []).map(function (e) {
            return { horizontal: e.hOffset, vertical: e.vOffset };
        });
    }
    this._canvases = [];
    this._masterContextId = masterContextId;
    this._alphaMaskIdMap = {};
    this._alphaMaskCanvasMap = {};
    this._decodeIds = [];

    this._applyAlphaMask = function (canvas, alphaMask) {

        var canvasContext = canvas.getContext("2d");
        var canvasData = canvasContext.getImageData(0, 0, canvas.width, canvas.height);
        var canvasPix = canvasData.data;

        var alphaContext = alphaMask.getContext("2d");
        var alphaData = alphaContext.getImageData(0, 0, alphaMask.width, alphaMask.height);
        var alphaPix = alphaData.data;

        for (var i = 0; i < canvasPix.length; i += 4) {
            canvasPix[i + 3] = alphaPix[i];
        }

        canvasContext.putImageData(canvasData, 0, 0);
    };

    this._isAlphaMask = function (auxItemId) {
        // §15: the AUXC typed accessor (heif_reader_get_property_auxc)
        // is not yet bound on this binding. Without it we can detect
        // the presence of an `auxC` property on the candidate item but
        // cannot read its auxType string
        // (`urn:mpeg:hevc:2015:auxid:1` for alpha). Treat any item that
        // carries an `auxC` property as alpha — matches the single-aux
        // assumption already made in image-provider.deriveItemType.
        var props;
        try { props = self._heifReader.itemProperties(auxItemId) || []; }
        catch (e) { return false; }
        for (var i = 0; i < props.length; i++) {
            if (props[i].type === 'auxC') {
                console.debug(auxItemId + ' carries auxC; assuming alpha (§15: subtype gated).');
                return true;
            }
        }
        return false;
    };

    this._render = function () {

        this._ctx.save();

        var s = 256.0 / 65536;
        var r = this._itemIovlObject.canvasFillValue[0] * s | 1;
        var g = this._itemIovlObject.canvasFillValue[1] * s | 1;
        var b = this._itemIovlObject.canvasFillValue[2] * s | 1;
        var a = this._itemIovlObject.canvasFillValue[3] * s | 1;

        this._ctx.fillStyle = "rgba(" + r + "," + g + "," + b + "," + a + ")";
        this._ctx.fillRect(0, 0, self._heifCanvas.width, self._heifCanvas.height);
        this._ctx.translate(0, 0);

        for (var i = 0; i < this._canvases.length; i++) {
            var canvas = this._canvases[i].canvas;
            var itemId = this._canvases[i].itemId;
            var canvasWidth = canvas.width;
            var canvasHeight = canvas.height;

            var alphaCanvas;
            if (this._alphaMaskIdMap[itemId]) {
                alphaCanvas = this._alphaMaskCanvasMap[this._alphaMaskIdMap[itemId]];
                this._applyAlphaMask(canvas, alphaCanvas);
            }

            this._ctx.drawImage(
                canvas,
                this._itemIovlObject.offsets[i].horizontal,
                this._itemIovlObject.offsets[i].vertical,
                canvasWidth,
                canvasHeight);
        }

        this._ctx.restore();
    };

    if (this._itemIovlObject) {

        this._heifCanvas.width = this._itemIovlObject.outputWidth;
        this._heifCanvas.height = this._itemIovlObject.outputHeight;

        for (var i = 0; i < this._itemIovlObject.itemIds.length; i++) {
            var itemId = this._itemIovlObject.itemIds[i];
            var refs = this._heifReader.referencedTo(itemId, 'auxl');
            if (refs && refs.length > 0) {
                console.debug(itemId + " has a reference to " + refs[0]);
                if (this._isAlphaMask(refs[0])) {
                    this._alphaMaskIdMap[itemId] = refs[0];
                    this._alphaMaskCanvasMap[refs[0]] = null;
                    this._decodeIds.push(refs[0]);
                }
            }
            this._decodeIds.push(itemId);
        }

        this._imageProvider.requestImageData(masterContextId, self._decodeIds, function (payload) {
            for (var i = 0; i < payload.frames.length; i++) {
                var width = payload.frames[i].width;
                var height = payload.frames[i].height;
                var itemCanvas = document.createElement("canvas");
                itemCanvas.width = width;
                itemCanvas.height = height;
                var context = itemCanvas.getContext("2d");
                var imageData = context.createImageData(width, height);
                imageData.data.set(payload.frames[i].canvasFrameData);
                context.putImageData(imageData, 0, 0);

                if (self._alphaMaskCanvasMap.hasOwnProperty(payload.frames[i].itemId)) {
                    self._alphaMaskCanvasMap[payload.frames[i].itemId] = itemCanvas;
                } else {
                    self._canvases.push({itemId: payload.frames[i].itemId, canvas: itemCanvas});
                }
            }

            self._render();
        });
    }
}
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

function Grid() {
    var self = this;
    self._width = 0;
    self._height = 0;
    self._cells = [];
    self._images = [];
    self._scale = {x: 1.0, y: 1.0};
    self._pos = {x: 0, y: 0};
    self._animation = new GridAnimation();
    self._rows = 0;
    self._cols = 0;
    self._selectedCell = null;

    this.addImage = function (image) {
        self._images.push(image);
        self._rows = Math.ceil(Math.sqrt(self._images.length));
        self._cols = Math.ceil(self._images.length / self._rows);
    };

    this.animation = function () {
        return self._animation;
    };

    this.setPos = function (x, y) {
        self._pos.x = x;
        self._pos.y = y;
    };

    this.pos = function () {
        return self._pos;
    };

    this.setScale = function (x, y) {
        self._scale.x = x;
        self._scale.y = y;
    };

    this.scale = function () {
        return self._scale;
    };

    this.size = function () {
        return {width: self._width, height: self._height};
    };

    this.rows = function () {
        return self._rows;
    };

    this.cols = function () {
        return self._cols;
    };

    this.handleClickAt = function (relativeX, relativeY) {

        if (self._animation.isZoomIn()) {
            self._animation.initialize(self, self._selectedCell, false);
        } else {

            // Scale and pos need to be taken into account in order to translate the
            // click position correctly during zoom out animation.
            var x = (self._width  * relativeX - self._width / 2) / self._scale.x + self._pos.x;
            var y = (self._height * relativeY - self._height / 2) / self._scale.y + self._pos.y;

            for (var i = 0; i < self._cells.length; i++) {
                var cell = self._cells[i];
                if (cell.contains(x, y)) {
                    self._selectedCell = cell;
                    self._animation.initialize(self, cell, true);

                    // Call the start handler if the animation got interrupted while zooming out
                    if (self._animation.isInProgress()) {
                        if (self._animationStartedHandler) {
                            self._animationStartedHandler(self._animation.cell());
                        }
                    }

                    break;
                }
            }
        }
    };

    // Cells are positioned so that (0, 0) is at the center of the view.
    this.initializeCells = function (width, height) {

        self._width = width;
        self._height = height;

        var nImages     = self._images.length;
        var nCellsInRow = self.rows();
        var nCellsInCol = self.cols();
        var rowHeight   = self._height / nCellsInCol;
        var cellWidth   = self._width / nCellsInRow;
        var cellHeight  = self._height / nCellsInCol;

        var y = rowHeight / 2;
        for (var i = 0; i < nImages; i++) {
            var image = self._images[i];
            var x = (i % nCellsInRow) * cellWidth + cellWidth / 2;
            if (i > 0 && i % nCellsInRow === 0) {
                y += rowHeight;
            }
            var cell = new Cell(x - self._width / 2, y - self._height / 2, cellWidth, cellHeight, image);
            cell.setMargin(0.5);
            self._cells.push(cell);
        }
    };

    this.setupContext = function (ctx) {
        ctx.save();
        ctx.fillStyle = "black";
        ctx.fillRect(0, 0, self._width, self._height);
        ctx.translate(
            self._width / 2  - self._pos.x * self._scale.x,
            self._height / 2 - self._pos.y * self._scale.y);
        ctx.scale(self._scale.x, self._scale.y);
    };

    var firstRender = true;
    this.render = function (ctx) {

        if (firstRender || self._animation.isInProgress()) {
            firstRender = false;

            self.setupContext(ctx);

            var nCells = self._cells.length;
            for (var i = 0; i < nCells; i++) {
                var cell = self._cells[i];
                cell.render(ctx);
            }

            ctx.restore();
        }

        var wasInProgress = self._animation.isInProgress();
        if (!self._animation.update()) {
            if (self._animationFinishedHandler && wasInProgress) {
                self._animationFinishedHandler(self._animation.cell());
            }
        } else {
            if (self._animationStartedHandler && !wasInProgress) {
                self._animationStartedHandler(self._animation.cell());
            }
        }
    };

    self._animationStartedHandler = null;
    this.setAnimationStartedHandler = function (handler) {
        self._animationStartedHandler = handler;
    };

    self._animationFinishedHandler = null;
    this.setAnimationFinishedHandler = function (handler) {
        self._animationFinishedHandler = handler;
    };
}
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

function GridAnimation() {

    var self = this;
    self._cell = null;
    self._grid = null;
    self._isInProgress = false;
    self._target = null;
    self._source = null;
    self._zoomIn = false;
    self._curve = new Curve(60, 2.0);
    self._curveIndex = 0;

    this.cell = function () {
        return self._cell;
    };

    this.grid = function () {
        return self._grid;
    };

    this.initialize = function (grid, cell, zoomIn) {

        self._curveIndex = 0;

        self._zoomIn = zoomIn;
        self._grid = grid;
        self._cell = cell;

        self._source = {
            x: grid.pos().x,
            y: grid.pos().y,
            scaleX: grid.scale().x,
            scaleY: grid.scale().y,
            margin: cell.margin()
        };

        if (zoomIn) {
            self._target = {
                x: cell.pos().x,
                y: cell.pos().y,
                scaleX: grid.size().width / cell.size().width,
                scaleY: grid.size().height / cell.size().height,
                margin: 0
            };
        } else {
            self._target = {
                x: 0,
                y: 0,
                scaleX: 1.0,
                scaleY: 1.0,
                margin: 0.5
            };
        }
    };

    this.isZoomIn = function () {
        return self._zoomIn;
    };

    this.isInProgress = function () {
        return self._isInProgress;
    };

    this.update = function () {
        if (self._grid) {

            if (self._curveIndex + 1 < self._curve.length()) {

                var curveVal = self._curve.value(self._curveIndex);

                // Animate position
                var newX = self._source.x + (self._target.x - self._source.x) * curveVal;
                var newY = self._source.y + (self._target.y - self._source.y) * curveVal;
                self._grid.setPos(newX, newY);

                // Animate scale
                var newScaleX = self._source.scaleX + (self._target.scaleX - self._source.scaleX) * curveVal;
                var newScaleY = self._source.scaleY + (self._target.scaleY - self._source.scaleY) * curveVal;
                self._grid.setScale(newScaleX, newScaleY);

                // Animate margin
                if (self._cell) {
                    self._cell.setMargin(self._source.margin + (self._target.margin - self._source.margin) * curveVal);
                }

                self._curveIndex++;
                self._isInProgress = true;

            } else {
                self._isInProgress = false;
            }
        }

        return self._isInProgress;
    };

    this.target = function () {
        return this._target;
    };
}
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

function GridObject(canvas, fileInfo, heifReader, imageProvider) {

    var self = this;

    self._canvas = canvas;
    self._ctx = self._canvas.getContext('2d');
    self._contextId = null;
    self._heifReader = heifReader;
    self._imageProvider = imageProvider;
    self._fileInfo = fileInfo;
    self._fullWidth = null;
    self._fullHeight = null;
    self._grid = null;
    self._masterIds = null;
    self._thumbnailIds = null;
    self._thmbDisplayH = 0;
    self._thmbDisplayW = 0;

    function onFrameDataReceived(payload) {
        if (payload.success) {

            self._grid = new Grid();

            // Add thumbnails to the grid
            for (var i = 0; i < payload.frames.length; ++i) {
                var thumbnailCanvas = document.createElement("canvas");
                thumbnailCanvas.width = payload.frames[i].width;
                thumbnailCanvas.height = payload.frames[i].height;

                var thumbnailContext = thumbnailCanvas.getContext("2d");
                var thumbnailImageData = thumbnailContext.getImageData(0, 0, thumbnailCanvas.width, thumbnailCanvas.height);
                thumbnailImageData.data.set(payload.frames[i].canvasFrameData);
                thumbnailContext.putImageData(thumbnailImageData, 0, 0);

                var image = new Image(i, self._imageProvider.getItemIdByThumbnailId(
                    self._fileInfo, self._contextId, payload.frames[i].itemId), thumbnailCanvas);
                self._grid.addImage(image);

                if (thumbnailCanvas.width > self._thmbDisplayW) {
                    self._thmbDisplayW = thumbnailCanvas.width;
                }

                if (thumbnailCanvas.height > self._thmbDisplayH) {
                    self._thmbDisplayH = thumbnailCanvas.height;
                }
            }

            var sizeMultiplier = self._grid.cols();
            self._canvas.width = payload.displayWidth * sizeMultiplier;
            self._canvas.height = payload.displayHeight * sizeMultiplier;

            self._grid.initializeCells(self._canvas.width, self._canvas.height);

            self._grid.setAnimationStartedHandler(function(cell) {
                // Restore thumbnail configuration before animation
                restoreThumbnailConfiguration();
            });

            // Set a handler to draw the full size image data once the zoom in animation has finished.
            self._grid.setAnimationFinishedHandler(function(cell) {
                if (cell && self._grid.animation().isZoomIn()) {
                    self._imageProvider.requestImageData(self._contextId, [cell.image().id()], function(payloadData) {
                        if (self._grid.animation().isZoomIn()) {
                            if (payloadData.success) {
                                saveFullSizeImage(payloadData);
                                renderFullSizeImage();
                            } else {
                                alert("Error when requesting image data of " + self._heifReader.url());
                            }
                        }
                    });
                }
            });

        } else {
            alert("Error when requesting image data of " + self._heifReader.url());
        }
    }

    function saveFullSizeImage(payloadData) {
        self.imageCanvas = document.createElement("canvas");
        var frame = payloadData.frames[0];
        self.imageCanvas.width = frame.width;
        self.imageCanvas.height = frame.height;
        var imageContext = self.imageCanvas.getContext("2d");
        var imageData = imageContext.getImageData(0, 0, frame.width, frame.height);
        imageData.data.set(frame.canvasFrameData);
        imageContext.putImageData(imageData, 0, 0);
    }

    function restoreThumbnailConfiguration() {
        var sizeMultiplier = self._grid.cols();
        self._canvas.width = self._thmbDisplayW * sizeMultiplier;
        self._canvas.height = self._thmbDisplayH * sizeMultiplier;
        if (self._grid) {
            self._grid.render(self._ctx);
        }
    }

    function renderFullSizeImage() {
        self._canvas.width = self._fullWidth;
        self._canvas.height = self._fullHeight;
        self._ctx.save();
        self._ctx.drawImage(self.imageCanvas, 0, 0, self._fullWidth, self._fullHeight);
        self._ctx.restore();
    }

    function allFramesHaveThumbnails() {
        return self._thumbnailIds && self._thumbnailIds.length === self._masterIds.length;
    }

    function initialize() {
        if (self._fileInfo.rootLevelMetaBoxProperties || self._fileInfo.trackProperties) {
            self._contextId = self._imageProvider.getMasterContextId(self._fileInfo);
            self._fullWidth = self._imageProvider.getDisplayWidth(self._fileInfo, self._contextId);
            self._fullHeight = self._imageProvider.getDisplayHeight(self._fileInfo, self._contextId);
            self._thumbnailIds = self._imageProvider.getThumbnailIds(self._fileInfo);
            self._masterIds = self._imageProvider.getMasterIds(self._fileInfo);

            var thumbnailContextId;
            if (allFramesHaveThumbnails()) {
                thumbnailContextId = self._imageProvider.getThumbnailContextId(self._fileInfo);
            } else {
                thumbnailContextId = self._contextId;
                self._thumbnailIds = self._masterIds;
            }

            self._imageProvider.requestImageData(thumbnailContextId, self._thumbnailIds, onFrameDataReceived);

        } else {
            alert("Not a meta or trak file: " + self._heifReader.url());
        }
    }

    self._canvas.addEventListener("click", function(event) {
        if (self._grid) {
            // Calculate a relative click position, because the canvas client rect
            // size and the actual grid size might differ.
            var rect = self._canvas.getBoundingClientRect();
            var relativeX = (event.clientX - rect.left) / rect.width;
            var relativeY = (event.clientY - rect.top) / rect.height;

            self._grid.handleClickAt(relativeX, relativeY);
        }
    });

    var requestAnimFrame = window.requestAnimationFrame.bind(window);

    initialize();

    self.animate = function() {

        if (self._grid) {
            self._grid.render(self._ctx);
        }

        // request new frame
        requestAnimFrame(function() {
            self.animate();
        });
    }
}

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

function Image(index, id, thumbnailCanvas) {
    var self = this;
    self._id = id;
    self._index = index;
    self._thumbnailCanvas = thumbnailCanvas;

    this.id = function () {
        return self._id;
    };

    this.index = function () {
        return self._index;
    };
}
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

/*
 * This script file contains functions related to HEVC animation video controls.
 */

"use strict";

var VIDEOCONTROL_OBJECTS = {};
var THUMBNAIL_WIDTH = 120;

var PAUSE_STR = "pause";
var PLAY_STR = "play";
var NONE_STR = "none";

var ICON_PLAY_BACKWARD_CLASS = "icon-play-backward";
var ICON_PLAY_FORWARD_CLASS = "icon-play-forward";
var ICON_PAUSE_CLASS = "icon-pause";
var ICON_SKIP_BACKWARD_CLASS = "icon-skip-backward";
var ICON_SKIP_FORWARD_CLASS = "icon-skip-forward";
var ICON_LOOP_ON_CLASS = "icon-loop-on";
var ICON_LOOP_OFF_CLASS = "icon-loop-off";

function VideoControlObject (div, heifObject, heifCanvas, thumbnailCanvas) {

    var self = this;

    this._heifObject = heifObject;
    this._heifCanvas = heifCanvas;
    this._id = div.id;
    this._isThumbnailIdLookupBuilt = false;
    this._lastRequestedThumbnailId = null;
    this._mouseup = true;
    this._thumbnailIdLookup = [];
    this._thumbnailDataCache = {};

    this.buildThumbnailIdLookup = function () {
        if (!this._isThumbnailIdLookupBuilt) {
            var imageProvider = this._heifObject.getImageProvider();
            var fileInfo = this._heifObject.getFileInfo();
            var contextId = imageProvider.getThumbnailContextId(fileInfo);
            var masterIds = imageProvider.getMasterIds(fileInfo);
            var seekBarWidth = this._seekBar.offsetWidth | 0;
            var gapFilledIds = new Array(masterIds.length);

            var latestThumbnailId = this._findFirstThumbnail();
            if (latestThumbnailId === null) {
                return;
            }

            var masterId, thumbnailId;
            for (var j = 0; j < masterIds.length; j++) {
                masterId = masterIds[j];
                thumbnailId = imageProvider.getThumbnailIdByItemId(fileInfo, contextId, masterId);
                if (thumbnailId >= 0) {
                    latestThumbnailId = thumbnailId;
                }
                gapFilledIds[j] = latestThumbnailId;
            }

            this._thumbnailIdLookup.length = seekBarWidth;
            for (var i = 0; i < seekBarWidth; i++) {
                this._thumbnailIdLookup[i] = gapFilledIds[i * gapFilledIds.length / seekBarWidth | 0]
            }
            this._isThumbnailIdLookupBuilt = true;
        }
    };

    this.setCurrentFrame = function (currentFrame, frameCount) {
        this._seekBar.min = 0;
        this._seekBar.max = frameCount - 1;
        this._seekBar.value = currentFrame;
    };

    this.onLoopChange = function () {
        this._loop = !this._loop;
        this._heifObject.setLoop(this._loop);

        if (this._loop) {
            this._loopIcon.className = ICON_LOOP_ON_CLASS;
        } else {
            this._loopIcon.className = ICON_LOOP_OFF_CLASS;
        }
    };

    this.onSeekBarChange = function () {
        this.pause();
        this._heifObject.setFrameOrderIndex(Number(this._seekBar.value));
        this._overlayPlayIcon.style.opacity = 0;
    };

    this.pause = function () {
        this._heifObject.stopAnimation();

        this._playPauseForward.action = PLAY_STR;
        this._playPauseForwardIcon.className = ICON_PLAY_FORWARD_CLASS;

        this._playPauseBackward.action = PLAY_STR;
        this._playPauseBackwardIcon.className = ICON_PLAY_BACKWARD_CLASS;
    };

    this.play = function () {
        this.playForward();
    };

    this.playBackward = function () {
        if (this._seekBar.value === this._seekBar.min) {
            this._seekBar.value = this._seekBar.max;
            this.onSeekBarChange();
        }

        this._heifObject.setDirection(false);
        this._heifObject.startAnimation();

        this._playPauseBackward.action = PAUSE_STR;
        this._playPauseBackwardIcon.className = ICON_PAUSE_CLASS;

        this._playPauseForward.action = PLAY_STR;
        this._playPauseForwardIcon.className = ICON_PLAY_FORWARD_CLASS;

        this._overlayPlayIcon.style.opacity = 0;
    };

    this.playForward = function () {
        if (this._seekBar.value === this._seekBar.max) {
            this._seekBar.value = this._seekBar.min;
            this.onSeekBarChange();
        }

        this._heifObject.setDirection(true);
        this._heifObject.startAnimation();

        this._playPauseForward.action = PAUSE_STR;
        this._playPauseForwardIcon.className = ICON_PAUSE_CLASS;

        this._playPauseBackward.action = PLAY_STR;
        this._playPauseBackwardIcon.className = ICON_PLAY_BACKWARD_CLASS;

        this._overlayPlayIcon.style.opacity = 0;
    };

    this._createControls = function () {
        var DIV_STR = "div";
        this._overlayPlayIcon = document.createElement(DIV_STR);
        this._overlayPlayIcon.className = "play-overlay";
        this._overlayPlayIcon.onclick = function () {
            heifCanvas.onclick();
        };
        heifCanvas.parentNode.insertBefore(this._overlayPlayIcon, heifCanvas);

        this._playPauseBackwardIcon = document.createElement(DIV_STR);
        this._playPauseBackwardIcon.className = ICON_PLAY_BACKWARD_CLASS;

        this._playPauseForwardIcon = document.createElement(DIV_STR);
        this._playPauseForwardIcon.className = ICON_PLAY_FORWARD_CLASS;

        this._seekBar = document.createElement("input");
        this._seekBar.type = "range";
        this._seekBar.value = 0;

        this._seekBar.onchange = function () {
            self.onSeekBarChange();
        };

        this._seekBar.onclick = function () {
            self.pause();
        };

        this._seekBar.oninput = function () {
            self.onSeekBarChange();
        };

        this._seekBar.onmousedown = function () {
            self._mouseup = false;
            self._thumbnailCanvas.style.display = NONE_STR;
        };

        this._seekBar.onmousemove = function (event) {
            if (self._mouseup && self._isThumbnailIdLookupBuilt) {
                var seekBarRect = self._seekBar.getBoundingClientRect();
                var xPercent = (event.clientX - seekBarRect.left) / seekBarRect.width;
                var thumbnailIndex = Math.max(0, Math.floor(xPercent * self._thumbnailIdLookup.length));

                var mouseOffset = event.clientX - self._heifCanvas.getBoundingClientRect().left;
                self._thumbnailCanvas.style.left = (mouseOffset - THUMBNAIL_WIDTH / 2) + "px";
                self._thumbnailCanvas.style.marginLeft = self._heifCanvas.style.marginLeft;

                var thumbnailId = self._thumbnailIdLookup[thumbnailIndex];
                if (thumbnailId !== null) {
                    self._thumbnailCanvas.style.display = "";
                    if (thumbnailId !== self._lastRequestedThumbnailId) {
                        if (!self._updateThumbnailCanvasFromCache(thumbnailId)) {
                            self._heifObject.requestThumbnailImageData([thumbnailId], self._getThumbnailDataCallback());
                            self._lastRequestedThumbnailId = thumbnailId;
                        }
                    }
                }
            }
        };

        this._seekBar.onmouseout = function () {
            self._thumbnailCanvas.style.display = NONE_STR;
        };

        this._seekBar.onmouseup = function () {
            self._mouseup = true;
        };

        var BUTTON_STR = "button";
        this._skipBackwardIcon = document.createElement(DIV_STR);
        this._skipBackwardIcon.className = ICON_SKIP_BACKWARD_CLASS;
        this._skipBackward = document.createElement(BUTTON_STR);
        this._skipBackward.appendChild(this._skipBackwardIcon);
        this._skipBackward.onclick = function () {
            self.pause();
            self.seekBar.value = 0;
            self.onSeekBarChange();
        };

        this._skipForwardIcon = document.createElement(DIV_STR);
        this._skipForwardIcon.className = ICON_SKIP_FORWARD_CLASS;
        this._skipForward = document.createElement(BUTTON_STR);
        this._skipForward.appendChild(this._skipForwardIcon);
        this._skipForward.onclick = function () {
            self.pause();
            self.seekBar.value = self.seekBar.max;
            self.onSeekBarChange();
        };

        this._thumbnailCanvas = thumbnailCanvas;
        this._thumbnailCanvas.className = "video-thumbnail";
        this._thumbnailCanvas.style.display = NONE_STR;
        this._thumbnailCanvas.style.width = THUMBNAIL_WIDTH + "px";

        this._loopIcon = document.createElement(DIV_STR);
        this._loop = heifCanvas.loop;
        if (this._loop) {
            this._loopIcon.className = ICON_LOOP_ON_CLASS;
        } else {
            this._loopIcon.className = ICON_LOOP_OFF_CLASS;
        }
        this._loopButton = document.createElement(BUTTON_STR);
        this._loopButton.appendChild(this._loopIcon);
        this._loopButton.onclick = function () {
            self.onLoopChange();
        };

        this._playPauseBackward = document.createElement(BUTTON_STR);
        this._playPauseBackward.appendChild(this._playPauseBackwardIcon);
        this._playPauseBackward.action = PLAY_STR;
        this._playPauseBackward.onclick = function () {
            if (self._playPauseBackward.action === PLAY_STR) {
                self.playBackward();
            } else {
                self.pause();
            }
        };

        this._playPauseForward = document.createElement(BUTTON_STR);
        this._playPauseForward.appendChild(this._playPauseForwardIcon);
        this._playPauseForward.action = PLAY_STR;
        this._playPauseForward.onclick = function () {
            if (self._playPauseForward.action === PLAY_STR) {
                self.playForward();
            } else {
                self.pause();
            }
        };
        heifCanvas.onclick = function () {
            self._playPauseForward.click();
        };

        div.className = "video-controls";
        div.appendChild(this._playPauseBackward);
        div.appendChild(this._skipBackward);
        div.appendChild(this._seekBar);
        div.appendChild(this._skipForward);
        div.appendChild(this._playPauseForward);
        div.appendChild(this._loopButton);
    };

    this._findFirstThumbnail = function () {
        var fileInfo = this._heifObject.getFileInfo();
        var imageProvider = this._heifObject.getImageProvider();
        var masterIds = imageProvider.getMasterIds(fileInfo);
        var contextId = imageProvider.getThumbnailContextId(fileInfo);
        for (i = 0; i < masterIds.length; i++) {
            var masterId = masterIds[i];
            var thumbnailId = imageProvider.getThumbnailIdByItemId(fileInfo, contextId, masterId);
            if (thumbnailId >= 0) {
                return thumbnailId;
            }
        }
        return null;
    };

    this._getThumbnailDataCallback = function () {
        var self = this;

        return function (payload) {
            if (payload.success !== true) {
                console.error("Error occurred while loading thumbnail image data");
                return;
            }

            self._updateThumbnailCanvasAndCache(payload);
        };
    };

    this._updateThumbnailCanvasAndCache = function (payload) {
        var frame = payload.frames[0];
        var rgbaData = frame.canvasFrameData;
        var itemId = frame.itemId;
        if (itemId === self._lastRequestedThumbnailId) {
            self._thumbnailCanvas.width = payload.displayWidth;
            self._thumbnailCanvas.height = payload.displayHeight;
            var context = self._thumbnailCanvas.getContext("2d");
            var imageData = context.getImageData(0, 0, payload.displayWidth, payload.displayHeight);
            imageData.data.set(new Uint8ClampedArray(rgbaData));
            context.putImageData(imageData, 0, 0);
        } else if (!(itemId in self._thumbnailDataCache)) {
            self._thumbnailDataCache[itemId] = new Uint8ClampedArray(rgbaData);
        }
    };

    this._updateThumbnailCanvasFromCache = function (itemId) {
        if (itemId in self._thumbnailDataCache) {
            var context = self._thumbnailCanvas.getContext("2d");
            var imageData = context.getImageData(0, 0, self._thumbnailCanvas.width, self._thumbnailCanvas.height);
            imageData.data.set(self._thumbnailDataCache[itemId]);
            context.putImageData(imageData, 0, 0);
            return true;
        }
        return false;
    };

    this._createControls();
}

function createVideoContainer (heifCanvas, heifObject) {
    if (!heifCanvas || !heifObject) {
        return null;
    }

    var videoControl = document.createElement("div");
    videoControl.id = UNIQUE_ID;
    UNIQUE_ID++;

    var _thumbnailCanvas = document.createElement("canvas");

    if (!heifCanvas.controls) {
        videoControl.style.display = NONE_STR;
    }

    var container = document.createElement("div");
    container.className = "video-container";
    container.width = heifCanvas.width;
    container.height = heifCanvas.height;
    container.appendChild(heifCanvas);
    container.appendChild(videoControl);
    container.appendChild(_thumbnailCanvas);

    VIDEOCONTROL_OBJECTS[videoControl.id] = new VideoControlObject(videoControl, heifObject, heifCanvas, _thumbnailCanvas);
    heifObject.setVideoControlObject(VIDEOCONTROL_OBJECTS[videoControl.id]);

    return container;
}
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

window.addEventListener("load", function() {
    processImageElements();
    processVideoElements();
});
