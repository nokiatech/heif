/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

"use strict";

// Fix Function#name on browsers that do not support it (IE):
if (!(function f() {}).name) {
    Object.defineProperty(Function.prototype, 'name', {
        get: function() {
            var name = this.toString().match(/^\s*function\s*(\S*)\s*\(/)[1];
            // For better performance only parse once, and then cache the
            // result through a new accessor for repeated access.
            Object.defineProperty(this, 'name', { value: name });
            return name;
        }
    });
}

/** Decoder for Hevc files (x265 stream.)
 *  @constructor */
function HevcDecoder () {

    var _name = "HevcDecoder";

    var self = this;

    this._decoderParameters = null;
    this._isRequestActive = false;
    this._player = null;
    this._requestContext = null;
    this._requestContextQueue = [];

    // **** PUBLIC API BEGINS ****

    /** Push a new RequestContext to the decoding queue.
     *  @param {RequestContext} Decoding request context containing information needed for decoding. */
    this.pushRequestContext = function (requestContext) {
        self._requestContextQueue.push(requestContext);
    };

    /** Start decoding the request queue. */
    this.decode = function () {

        if (self._isRequestActive) {
            return;
        }

        if (self._requestContextQueue.length) {
            self._isRequestActive = true;
            self._requestContext = self._requestContextQueue.pop();
            self._playStream(self._requestContext.stream);
        }
    };

    // **** PUBLIC API ENDS ****

    this._createDecodeCanvas = function(parentElement) {
        self.canvas = document.createElement("canvas");
        self.canvas.style.display = "none";
        self.canvas.id = "decode-canvas";
        parentElement.appendChild(self.canvas);

        self.ctx = self.canvas.getContext("2d");
    };

    this._playStream = function (buffer) {
        this._reset();
        this._handleOnLoad(buffer);
    }

    this._onImageDecoded = function (image) {
        var width = image.get_width();
        var height = image.get_height();
        this.canvas.width = width;
        this.canvas.height = height;
        this._imageData = this.ctx.createImageData(width, height);

        // Note that the image is not *really* displayed here. We just hook
        // to the display callback of an image and grab the canvas data.
        image.display(this._imageData, function (displayImageData) {
            var itemId = self._requestContext.itemIds[self._requestContext.currentFrameIndex];
            var payload = self._requestContext.payload;

            if (height > payload.displayHeight) {
                payload.displayHeight = height;
            }

            if (!(itemId in self._requestContext.dependencies)) {
                if (width > payload.displayWidth) {
                    payload.displayWidth = width;
                }

                payload.frames.push({
                    canvasFrameData: displayImageData.data,
                    itemId: itemId,
                    width: width,
                    height: height
                });
            }

            // Call the client callback once all frames are decoded
            self._requestContext.currentFrameIndex++;
            if (self._requestContext.currentFrameIndex >= self._requestContext.itemIds.length) {
                self._requestContext.callback(payload);
                self._isRequestActive = false;
                self.decode(); // Decode next queued request
            }
        });
    };

    this._handleOnLoad = function (buffer) {
        var decoder = new libde265.Decoder();
        decoder.set_image_callback(function(image) {
            self._onImageDecoded(image);
            image.free();
        });

        var pos = 0;
        var remaining = buffer.byteLength;
        var filters = false;

        var decode = function() {
            var err;
            if (remaining === 0) {
                err = decoder.flush();
            } else {
                var l = 4096;
                if (l > remaining) {
                    l = remaining;
                }
                var tmp = new Uint8Array(buffer, pos, l);
                err = decoder.push_data(tmp);
                pos += l;
                remaining -= l;
            }
            if (!libde265.de265_isOK(err)) {
                console.error(libde265.de265_get_error_text(err));
                return;
            }

            if (self.filters !== filters) {
                decoder.disable_filters(self.filters);
                filters = self.filters;
            }

            decoder.decode(function(err) {
                switch(err) {
                case libde265.DE265_ERROR_WAITING_FOR_INPUT_DATA:
                    setTimeout(decode, 0);
                    return;

                default:
                    if (!libde265.de265_isOK(err)) {
                        console.error(libde265.de265_get_error_text(err));
                        return;
                    }
                }

                if (remaining > 0 || decoder.has_more()) {
                    setTimeout(decode, 0);
                    return;
                }

                decoder.free();
                self._reset();
            });
        };

        setTimeout(decode, 0);
    };

    this._reset = function() {
        self._imageData = null;
    };

    this._createDecodeCanvas(document.documentElement);
    this._reset();
}
