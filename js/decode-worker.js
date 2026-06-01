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
 * Web Worker for HEVC (libde265) decoding.
 *
 * Loads the stock libde265.min.js asm.js build via importScripts().
 * Receives encoded bitstreams, decodes them using the high-level
 * Decoder API, and posts back RGBA frames with transferable buffers.
 *
 * Stateless per-request: a fresh Decoder is created for each decode
 * so that a crash on one bitstream cannot corrupt subsequent decodes.
 */

/* global libde265, importScripts, self */

// libde265.min.js expects `window` and `this` to be the global.
// In a worker the global is `self`, so shim `window` to point there.
if (typeof window === 'undefined') {
    self.window = self;
}
if (typeof document === 'undefined') {
    self.document = { createElement: function() { return {}; } };
}

// Load libde265.min.js from jsDelivr CDN (pinned to a specific commit).
importScripts('https://cdn.jsdelivr.net/gh/strukturag/libde265.js@191f20b17a3a248805629784215c2c65e995d8b2/lib/libde265.min.js');

/**
 * Decode an HEVC bitstream into RGBA frames.
 * Uses the same async setTimeout-driven chunk loop as the original
 * main-thread adapter — libde265's decode(callback) fires its
 * callback asynchronously even inside a worker.
 *
 * @param {ArrayBuffer} ab  - Raw Annex-B bitstream
 * @param {number}      id  - Request id for the response message
 */
function decodeAsync(ab, id) {
    var lib = self.libde265;

    // libde265's Decoder constructor checks for Worker to decide between
    // a worker-based YUV→RGB converter (ab) and a synchronous one (bb).
    // Inside our decode-worker, Worker exists but the sub-worker path
    // (window.URL.createObjectURL) doesn't work, causing display() to
    // silently fail.  Hide Worker temporarily so libde265 picks the
    // synchronous converter.
    var savedWorker = self.Worker;
    self.Worker = undefined;
    var decoder = new lib.Decoder();
    self.Worker = savedWorker;

    var frames = [];

    decoder.set_image_callback(function(image) {
        var w = image.get_width();
        var h = image.get_height();
        var imgData = new ImageData(w, h);

        var bitDepth = 8;
        try {
            if (typeof image.get_bits_per_pixel === 'function') {
                bitDepth = image.get_bits_per_pixel();
            }
        } catch (e) {}

        image.display(imgData, function(displayData) {
            frames.push({
                width: w,
                height: h,
                rgba: new Uint8ClampedArray(displayData.data),
                bitDepth: bitDepth
            });
        });

        image.free();
    });

    var position = 0;
    var remaining = ab.byteLength;

    function sendResult() {
        var transferList = [];
        for (var i = 0; i < frames.length; i++) {
            transferList.push(frames[i].rgba.buffer);
        }
        self.postMessage({
            type: 'decode-complete',
            id: id,
            frames: frames
        }, transferList);
    }

    function processChunk() {
        var error;
        try {
            if (remaining === 0) {
                error = decoder.flush();
            } else {
                var chunkSize = Math.min(4096, remaining);
                var chunk = new Uint8Array(ab, position, chunkSize);
                error = decoder.push_data(chunk);
                position += chunkSize;
                remaining -= chunkSize;
            }

            if (!lib.de265_isOK(error)) {
                decoder.free();
                sendResult();
                return;
            }

            decoder.decode(function(decodeError) {
                if (decodeError === lib.DE265_ERROR_WAITING_FOR_INPUT_DATA) {
                    setTimeout(processChunk, 0);
                    return;
                }

                if (!lib.de265_isOK(decodeError)) {
                    decoder.free();
                    sendResult();
                    return;
                }

                if (remaining > 0 || decoder.has_more()) {
                    setTimeout(processChunk, 0);
                    return;
                }

                decoder.free();
                sendResult();
            });
        } catch (e) {
            console.error('decode-worker error:', e);
            if (decoder) decoder.free();
            sendResult();
        }
    }

    setTimeout(processChunk, 0);
}

self.onmessage = function(e) {
    var msg = e.data;

    if (msg.type === 'decode') {
        try {
            decodeAsync(msg.buffer, msg.id);
        } catch (err) {
            self.postMessage({
                type: 'error',
                id: msg.id,
                error: err.message || 'Unknown decode error'
            });
        }
    }
};

self.postMessage({ type: 'ready' });
