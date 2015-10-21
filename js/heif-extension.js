// Copyright (c) 2015, Nokia Technologies Ltd.
// All rights reserved.
//
// Licensed under the Nokia High-Efficiency Image File Format (HEIF)
// License (the "License").
//
// You may not use the High-Efficiency Image File Format except in
// compliance with the License.
// The License accompanies the software and can be found in the file
// "LICENSE.TXT".
//
// You may also obtain the License at:
// https://nokiatech.github.io/heif/license.txt
//

var css = document.createElement("style");css.type = "text/css";css.innerHTML = '.video-controls {    position: absolute;    bottom: 4px;    height: 40px;    width: 100%;    left: 0px;    right: 0px;    opacity: 0;    text-align: center;    transition: opacity 0.5s;    background: rgba(0, 0, 0, 0.60);}.video-container {    display: inline-block;    position: relative;    margin: 0;}.video-container .video-thumbnail {    position: absolute;    z-index: 1;    bottom: 44px;    border: 2px solid rgba(0, 0, 0, 0.8);    border-radius: 2px;}.video-container:hover {    cursor: pointer;}.video-controls:hover {    cursor: auto;}.video-container:hover .video-controls {    opacity: 0.9;}.video-controls button {    background: transparent;    border: none;    outline: none;    margin: 7px 5px;    vertical-align: top;    padding: 0px;    width: 25px;    height: 25px;}.video-controls input {    background: transparent;    height: 25px;}.video-controls button div {    width: 25px;    height: 25px;    background-size: contain;    background-position: center;    background-repeat: no-repeat;}.video-controls button:hover {    cursor: pointer;}input[type="range"] {    -webkit-appearance: none;    -webkit-tap-highlight-color: rgba(255, 255, 255, 0);    height: 4px;    width: 250px;    margin: 18px 0 18px 0;    border: none;    padding: 1px 2px;    border-radius: 14px;    background: #999;    outline: none; /* no focus outline */}input[type="range"]::-webkit-slider-thumb {    -webkit-appearance: none;    width: 14px;    height: 14px;    border: 4px solid #ffffff;    border-radius: 12px;    background-color: #333;    cursor: pointer;}.play-overlay {    position: absolute;    left: 0;    right: 0;    top: 0;    bottom: 0;    margin-left: auto;    margin-right: auto;    margin-top: auto;    margin-bottom: auto;    transition: opacity 0.5s;    opacity: 0.5;    background-position: center;    background-repeat: no-repeat;}.video-container:hover .play-overlay {    transition: opacity 0.5s;    opacity: 1.0;}.icon-loop-off {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABkAAAAZCAYAAADE6YVjAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAQVJREFUeNpi/P//PwOtARMDHcDwsYQFmdPY2BgApAyoYO6F+vr6DRg+AVqQAKQaqOT4Bqh5YMAISl1AAQcgewIQOwBd8IFSG4DmCQCpA0BcADTvAHKcfKCGBSAANefDaBIeppYAU1ElEF8hKTOSYDgo0y4AYh4gZqaqJdD8NAOIlUnRC1N4AYgFgIY0QDMRrGj4ADVcAWq4MzbDoZYjAxBfAGouJE6ghoEkFKBFSwOUDwMhQOyOx/UNaFgBufRgJLbSAroWVHAuAWJN9AQDNIwRn15GUmtGaNCsA2J+pJBgpGoSBhV4QCwEZOYC8U+a5hOgRdOAlAQQJxJSyzjakBh0lgAEGAA1oVh6dPo7zQAAAABJRU5ErkJggg==);}.icon-loop-on {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABoAAAAaCAYAAACpSkzOAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAOhJREFUeNpi/P//PwM9ABMDncCoRVSzKAGI/1MJJyAbzILELgDiACAWBOIPFHpAAIg3QOkJYBFQ8obiA0DsgMSnFDtAzQTzR1PdyLDIAIgVaGkRyPAdQHweiBtoYREsb9wHYndygu4DNBjwgUogfgHE+UQGKTzjMyLVRyCXHkCy9AO0pICBd0DMjyMUHgPxPTRfg4ADzDJGLBWfA5JlF5DEs4C4D4jZsVi0GSqHDA6g8EgsVgSAeAIQ//yPChYQ0ktqYvgALXzVgXgnEP+hdT56AMQeQGwKxFeB+BEhDYyjjZNRi2AAIMAA2HvfKdbT6m8AAAAASUVORK5CYII=);}.icon-pause {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABoAAAAaCAYAAACpSkzOAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAQJJREFUeNrElmERwyAMhQEDqwQkRAJSKqkSKmESKqESkMAUMLg97jKutHAdLHfvD4F8hYYQ6b0XFUZBJmjKxl3QFrRfRoigE81B1n/MBW2ZHHwWc4uxSg7DAGsQnQQhzElAqgXNWBS/WF/smEtjjT/aXQmyNAByLUew/LjuQnKYSWOSZZ2FjPiNxWzUkMiPTJ9kn2j0aX6EadAic44WrCz7WnzJbxOIMLmUwtxafF+xFf7Jq+p2t9uO2EahrPSAcNikxCAbBdKjQFbhklJHSIztFH7WoxOMEHsbemGHlKDhRbXXM0F/ffiGPuW86nZvTmrarWdruyVvNpAWF/6y+r8FGABwiQYMSv06DwAAAABJRU5ErkJggg==);}.icon-play-backward {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABoAAAAaCAYAAACpSkzOAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAUtJREFUeNq8ljFOxDAQRZ2QHt9gV6KhTElBkYID5AipadgjUFKmpEJ7BI7gggNkSzq4QbZEIJk/MBbG2N5kFeZLTyvFk/keOzt2Ya1VE1SDBujg+Qsw/JtVlRmjpBvQgRV4jSSkCZyCHejBNpmNKorQgpHpQZ2Ic7Fb+y2Tio292PNLj0BnDELIYODJtYeMnEk3wyDEVdemjNoFTHwzqmwdGmlvP9QCaF5GExrdspGemZBmvEmMNbxCjW80stmcGT/YH6XiDH9Uyt+bqdXcgTf7W6nYzuV2yzZMMLjmyt/tX+Uq/1q+iluLyXQIGr8HZwc6SUwjd5S68h7E9AQuwIk6XtS2dKmEVHoNNKZLcAWewceRHmtasZL3p8kE0vg5uAH7mYaaO/8g+nmL/WFFW5BYUxU7JkQPPtGj/N8uJ0XmXrfodauQukB+CjAArLWlFRjg3coAAAAASUVORK5CYII=);}.icon-play-forward {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABoAAAAaCAYAAACpSkzOAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAWFJREFUeNq8lj9OwzAUxp3QHXOCdgeJHiELgtFL94yMXdkQJ8gRurN0RKqQfIQgwVx6AeqRoSh8D54ly4qNUwU/6adIybO/PP/57KLrOpEQc1AB6b1/B5qf0ZhEvlGnS1CDKdj1dEg/cApeQANWwd6ooh4UMEwD5oE8m7vqfkOHcvsaNtxoDWREwIcEWv459ZeQFakHCPjY6lRISI0g4opRZTNfSDrzIUZA8jBqX+iehWSg0XLgfBEVj1DlChkWizX4BHcDxTQvKuHOjYwI7TnnAD76VlWA2vZd8qajDWcSHOIEnIFH8MpuEYu13dglJ2sxLMhRzsEGPIFZIM+wo/wIicRqQoLXYAseAjlkW7IUmWLiGOgxcQDP4Dbi4DSspuT5qY4QeANX4CYiItn5WxJqwWViVV9gDxbgImERKX62WTdsVgvKZqrZjomsB1/Wo/zfLidF5F436nWryHWB/BZgAOq/BX8ribu7AAAAAElFTkSuQmCC);}.icon-skip-backward {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABoAAAAaCAYAAACpSkzOAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAKxJREFUeNpi+v//PwM9MBMDncCoRaMWkWRRAgF5BSDeD8QGlFo0H4e4ABCvAeL7QOwA5VM16EAGdgDxSyAOJlYTC4mWZAFxGxBzk6qXWMWgoFkExJJkOI4ki/bTK3lfBeI/9LBIB4hDgfg9EP+ldYbdAMRCQFwLxD/pUTK0A7EEEE+kRxH0AYgLgFgRiHdSK9Ul4pF7AMQe0OLnAT5DGEHV7Gg1MWrR8LQIIMAAOHhl7iVNFKEAAAAASUVORK5CYII=);}.icon-skip-forward {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAABoAAAAaCAYAAACpSkzOAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAALNJREFUeNpi/P//PwM9ABMDncCoRaMWEW2RARDvB2IFAuoSCNoEykd4sMN/BFgDxAI41P0nYA5JQRcMxC+BuAOIBWgdR2xAXAzED4A4i9aJgQWI+YF4IhA/AmIHWqc6kIWy0MRC8+T9B4iv0tKiv0D8HohDgViHVhb9BOJaIBYC4g2khDMpAJQAGoD4AzkRSgzYCcQZ0GRNHiCQo0ElgQGhXA/ECYTUMI5W5aMWDV+LAAIMAO8QuE7NMVrrAAAAAElFTkSuQmCC);}.play-overlay {background-image:url(data:image/gif;base64,iVBORw0KGgoAAAANSUhEUgAAAHQAAAB0CAYAAABUmhYnAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAyhpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdpbj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+IDx4OnhtcG1ldGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IkFkb2JlIFhNUCBDb3JlIDUuNS1jMDIxIDc5LjE1NTc3MiwgMjAxNC8wMS8xMy0xOTo0NDowMCAgICAgICAgIj4gPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4gPHJkZjpEZXNjcmlwdGlvbiByZGY6YWJvdXQ9IiIgeG1sbnM6eG1wPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvIiB4bWxuczp4bXBNTT0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL21tLyIgeG1sbnM6c3RSZWY9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZVJlZiMiIHhtcDpDcmVhdG9yVG9vbD0iQWRvYmUgUGhvdG9zaG9wIENDIDIwMTQgKE1hY2ludG9zaCkiIHhtcE1NOkluc3RhbmNlSUQ9InhtcC5paWQ6MUI0MUY3RkQzQThGMTFFNEEzRDZCQjU0MzFGNEM0RTUiIHhtcE1NOkRvY3VtZW50SUQ9InhtcC5kaWQ6MUI0MUY3RkUzQThGMTFFNEEzRDZCQjU0MzFGNEM0RTUiPiA8eG1wTU06RGVyaXZlZEZyb20gc3RSZWY6aW5zdGFuY2VJRD0ieG1wLmlpZDoxQjQxRjdGQjNBOEYxMUU0QTNENkJCNTQzMUY0QzRFNSIgc3RSZWY6ZG9jdW1lbnRJRD0ieG1wLmRpZDoxQjQxRjdGQzNBOEYxMUU0QTNENkJCNTQzMUY0QzRFNSIvPiA8L3JkZjpEZXNjcmlwdGlvbj4gPC9yZGY6UkRGPiA8L3g6eG1wbWV0YT4gPD94cGFja2V0IGVuZD0iciI/PqEf6x4AACHsSURBVHja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);}';document.head.appendChild(css);

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
"use strict";

var DEFAULT_FRAMERATE = 24; // FPS to use if no frameRate is specified
var PREFERRED_BUFFER_SIZE = DEFAULT_FRAMERATE * 5;
var FRAME_REQUEST_SIZE = 64;
var MAXIMUM_CACHE_SIZE = PREFERRED_BUFFER_SIZE + FRAME_REQUEST_SIZE;

/** @constructor */
function HEIFObject(heifCanvas, type) {

    var self = this;

    this._autoplay = heifCanvas.autoplay;
    this._context = heifCanvas.getContext("2d");
    this._delayedStart = false;
    this._displayHeight = null;
    this._displayWidth = null;
    this._drawPendingId = -1;
    this._fileInfo = undefined;
    this._frameCache = undefined;
    this._frameRate = undefined;
    this._collectionViewObject = null;
    this._collectionViewType = null;
    this._derivedGridObject = null;
    this._heifCanvas = heifCanvas;
    this._heifReader = new HEIFReader(heifCanvas.getAttribute("filename"));
    this._imageProvider = new ImageProvider(this._heifReader, new HevcDecoder());
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
    };

    this.drawCanvasFrameData = function (width, height, data) {
        var imageData = this._context.createImageData(width, height);
        imageData.data.set(data);

        if (this._context.canvas.width !== width || this._context.canvas.height !== height) {
            var thumbnailCanvas = document.createElement("canvas");
            thumbnailCanvas.width = width;
            thumbnailCanvas.height = height;
            thumbnailCanvas.getContext("2d").putImageData(imageData, 0, 0);
            this._context.drawImage(thumbnailCanvas, 0, 0, this._context.canvas.width, this._context.canvas.height);
        } else {
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
        return (this._type === "img" && (this._fileInfo.fileFeature.hasImageSequence || this._fileInfo.fileFeature.hasImageCollection && this._frameRate > 0));
    };

    this.hasImageCollection = function () {
        return this._type === "img" &&
            (this._fileInfo.fileFeature.hasImageCollection && !this._frameRate);
    };

    this.hasVideo = function () {
        return this._type === "video" && (this._fileInfo.fileFeature.hasImageSequence || this._fileInfo.fileFeature.hasImageCollection && this._frameRate > 0);
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
        this._frameCache = new Cache(PREFERRED_BUFFER_SIZE, MAXIMUM_CACHE_SIZE);
        this._sentRequests = {};

        var masterContextId = this._imageProvider.getMasterContextId(this._fileInfo);
        if (this._frameRate > 0 && this._fileInfo.fileFeature.hasRootLevelMetaBox) {
            this._heifReader.setForcedTimedPlayback(masterContextId, this._frameRate);
        }

        this._timestamps = this._heifReader.getItemTimestamps(masterContextId);

        if (this._videoControlObject) {
            this._videoControlObject.setCurrentFrame(0, this._timestamps.length);
            this._videoControlObject.buildThumbnailIdLookup();
        }
    };

    this.onAnimationTick = function () {
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
        this._heifReader.requestFileInfo(this._getFileInfoCallback());
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
            if (payload.success) {

                var frame = payload.frames[0];
                var width = frame.width;
                var height = frame.height;
                var data = frame.canvasFrameData;

                self.drawCanvasFrameData(width, height, data);
                self.callOnload();

            } else  {
                self.drawErrorImage();
            }
        };
    };

    this._getFirstRootMetaItemByType = function (type) {
        if (self._fileInfo.rootLevelMetaBoxProperties &&
            self._fileInfo.rootLevelMetaBoxProperties.metaBoxFeature) {
            if (self._fileInfo.rootLevelMetaBoxProperties.metaBoxFeature.hasDerivedImages) {
                self._masterContextId = self._imageProvider.getMasterContextId(self._fileInfo);
                var masterContextId = self._imageProvider.getMasterContextId(self._fileInfo);
                var items = self._heifReader.getItemListByType(masterContextId, type);
                if (items && items.length > 0) {
                    return items[0];
                }
            }
        }

        return null;
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
                console.log("Error occurred while loading file info");
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
            if (payload.success) {
                var frames = payload.frames;
                for (var i = 0; i < frames.length; ++i) {
                    var itemId = frames[i].itemId;
                    self._frameCache.put(itemId, frames[i].canvasFrameData);
                    self._sentRequests[itemId] = false;
                }

                if (self._drawPendingId >= 0 && self._frameCache.get(self._drawPendingId)) {
                    self.drawCanvasFrameData(self.frameWidth, self.frameHeight, self._frameCache.get(self._drawPendingId));
                    self._drawPendingId = -1;
                }

                self.callOnload();

                self._isPreloading = false;
                if (self._delayedStart) {
                    self._delayedStart = false;
                    self.startAnimation();
                }

            } else {
                console.error("Error getting frame data", payload);
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
        var coverImage = null;
        if (self._fileInfo.fileFeature.hasCoverImage) {
            coverImage = self._heifReader.getCoverImageItemId(self._masterContextId);
        } else {
            coverImage = self._masterIds[0];
            console.log(self._name + ": Cover image not set, using id " + coverImage);
        }

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
        var requestFrames = [];
        if (requestAll) {
            requestFrames = self._masterIds.slice(0);
            self._frameCache = new Cache(requestFrames.length, requestFrames.length);
        } else {
            this._getNextFrames(this._timestampsLastDrawnIndex).forEach(function(frameIndex) {
                var id = self._masterIds[frameIndex];
                if (!self._frameCache.get(id) && !self._sentRequests[id]) {
                    if (requestFrames.length < FRAME_REQUEST_SIZE) {
                        self._sentRequests[id] = true;
                        requestFrames.push(id);
                    }
                }
            });
        }

        if (requestFrames.length > 0) {
            self._imageProvider.requestImageData(
                self._imageProvider.getMasterContextId(self._fileInfo), requestFrames, self._getImageDataCallback());
        }
    };
}
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
/**
 * This script replaces all <img> and <video> tags containing a .heic file on
 * page load and replaces them with canvas elements.
 */

"use strict";

var FILE_EXTENSION = "heic";

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
        var type = re.exec(imageElement.src)[1];
        return (type.toLowerCase() === FILE_EXTENSION);
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

            var coverImageId = null;
            if (self._fileInfo.fileFeature.hasCoverImage) {
                coverImageId = self._heifReader.getCoverImageItemId(self._masterContextId);
            } else {
                coverImageId = self._masterIds[0];
            }

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

    window.requestAnimFrame = (function(callback) {
        return window.requestAnimationFrame    ||
            window.webkitRequestAnimationFrame ||
            window.mozRequestAnimationFrame    ||
            window.oRequestAnimationFrame      ||
            window.msRequestAnimationFrame     ||
            function (callback) {
                window.setTimeout(callback, 1000 / 60);
            };
    })();

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
    this._itemGridObject = self._heifReader.getItemGrid(masterContextId, gridItemId);
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
"use strict";

function DerivedIovlObject (heifCanvas, fileInfo, heifReader, imageProvider, masterContextId, IovlItemId) {

    var self = this;

    this._ctx = heifCanvas.getContext('2d');
    this._heifCanvas = heifCanvas;
    this._heifReader = heifReader;
    this._fileInfo = fileInfo;
    this._IovlItemId = IovlItemId;
    this._imageProvider = imageProvider;
    this._itemIovlObject = self._heifReader.getItemIovl(masterContextId, IovlItemId);
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

    this._isAlphaMask = function (itemId) {
        var itemProperties = this._heifReader.getItemProperties(masterContextId, refs[0]);
        var auxcIndex = null;
        for (var property in itemProperties) {
            if (itemProperties[property].type === "AUXC") {
                auxcIndex = itemProperties[property].index;
                var auxcProperties = this._heifReader.getPropertyAuxc(this._masterContextId, auxcIndex);
                if (auxcProperties.auxType === "urn:mpeg:hevc:2015:auxid:1") {
                    console.log(refs[0] + " is an alpha mask.");
                    return true;
                }
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
            var refs = this._heifReader.getReferencedToItemListByType(masterContextId, itemId, "auxl");
            if (refs && refs.length > 0) {
                console.log(itemId + " has a reference to " + refs[0]);
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

    window.requestAnimFrame = (function(callback) {
        return window.requestAnimationFrame    ||
            window.webkitRequestAnimationFrame ||
            window.mozRequestAnimationFrame    ||
            window.oRequestAnimationFrame      ||
            window.msRequestAnimationFrame     ||
            function (callback) {
                window.setTimeout(callback, 1000 / 60);
            };
    })();

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
                console.log("Error occurred while loading thumbnail image data");
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

window.addEventListener("load", function() {
    processImageElements();
    processVideoElements();
});
