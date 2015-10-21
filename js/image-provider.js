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

"use strict";

/** Decoder for HEIF files. Glues HEIFReader and HevcDecoder together.
 * @constructor
 * @param {HEIFReader} heifReader HEIF file reader object to read hevc encoded data.
 * @param {HevcDecoder} hevcDecoder HEVC decoder object to decode hevc encoded data. */
function ImageProvider (heifReader, hevcDecoder) {

    var _name = "ImageProvider";

    var self = this;

    this._decoderParameters = null;
    this._heifReader = heifReader;
    this._hevcDecoder = hevcDecoder;

    function _appendBuffer(buffer1, buffer2) {
        var tmp = new Uint8Array(buffer1.byteLength + buffer2.byteLength);
        tmp.set(new Uint8Array(buffer1), 0);
        tmp.set(new Uint8Array(buffer2), buffer1.byteLength);
        return tmp.buffer;
    }

    // **** PUBLIC API BEGINS ****

    /** Payload structure returned to the caller as a callback parameter.
     *  @constructor
     *  @param {boolean} success True if decoding was successful.
     *  @param {Uint8Array} frames Array of frame data.
     *  @param {number} displayWidth Max width of all frames.
     *  @param {number} displayHeight Max height of all frames. */
    function Payload (success, frames, displayWidth, displayHeight) {
        this.success = success;
        this.frames = frames;
        this.displayWidth = displayWidth;
        this.displayHeight = displayHeight;
    }

    /** RequestContext is used to queue data requests and is pushed to HevcDecoder.
     *  @constructor
     *  @param {function} callback Called when all items (frames) are decoded. The result is passed a as parameter.
     *  @param {Array.<number>} itemIds Array of item id's to be decoded.
     *  @param {Uint8Array} stream The data stream as Uint8Array.
     *  @param dependencies Item id map of dependency/reference frames that are not to be displayed.
     *  @param {Payload} payload The object returned to the caller. */
    function RequestContext (callback, itemIds, stream, dependencies, payload) {
        this.callback = callback;
        this.itemIds = itemIds;
        this.currentFrameIndex = 0;
        this.stream = stream;
        this.dependencies = dependencies;
        this.payload = payload;
    };

    /** Request decoded image data for given item id's.
     *  @param {number} contextId Id of the context.
     *  @param {Array.<number>} itemIds Array of item id's to be decoded.
     *  @param {function} callback Callback function that receives the payload as a parameter. */
    this.requestImageData = function (contextId, itemIds, callback) {

        console.log(_name + ": REQUEST IMAGE DATA: " + itemIds);

        if (itemIds.constructor !== Array) {
            itemIds = [itemIds];
        }

        if (itemIds.length < 1) {
            return;
        }

        var stream = new Uint8Array();
        var decodeIds = [];
        var dependencies = {};
        var displayDependencies = {};
        for (var i = 0; i < itemIds.length; i++) {
            var refs = self._heifReader.getItemDecodeDependencies(contextId, itemIds[i]);
            for (var j = 0; j < refs.length; j++) {
                if (refs[j] !== itemIds[i]) {
                    // Add dependency data to the stream if the id is not already listed in dependencies
                    if(!(refs[j] in dependencies) && !(refs[j] in displayDependencies)) {
                        dependencies[refs[j]] = true;
                        stream = _appendBuffer(stream, new Uint8Array(self._heifReader.getItemDataWithDecoderParameters(contextId, refs[j])));
                        decodeIds.push(refs[j]);
                    }
                } else {
                    // Item has itself as dependency => we want to display that item despite being a dependency
                    displayDependencies[refs[j]] = true;
                }
            }
        }

        // Add data of the actual items to the stream (that already contains dependency data)
        for (i = 0; i < itemIds.length; i++) {
            stream = _appendBuffer(stream, new Uint8Array(self._heifReader.getItemDataWithDecoderParameters(contextId, itemIds[i])));
            decodeIds.push(itemIds[i]);
        }

        var payload = new Payload(true, [], 0, 0);
        self._hevcDecoder.pushRequestContext(new RequestContext(callback, decodeIds, stream, dependencies, payload));
        self._hevcDecoder.decode();
    };

    this._getDisplayItems = function (fileInfo, contextId) {
        if (fileInfo.getContextType(contextId) === "trak") {
            return self._heifReader.getItemListByType(contextId, "display");
        } else {
            return self._heifReader.getItemListByType(contextId, "master");
        }
    };

    /** A convenience method to get display height of given context.
     *  @param {HEIFReader~FileInfo} fileInfo FileInfo structure.
     *  @param {number} contextId Id of the context.
     *  @return {number} Display height. */
    this.getDisplayHeight = function (fileInfo, contextId) {
        var items = self._getDisplayItems(fileInfo, contextId);
        var maxHeight = 0;
        for (var i = 0; i < items.length; i++) {
            var height = self._heifReader.getItemHeight(contextId, items[i]);
            if (height > maxHeight) {
                maxHeight = height;
            }
        }
        return maxHeight;
    };

    /** A convenience method to get display width of given context.
     *  @param {HEIFReader~FileInfo} fileInfo FileInfo structure.
     *  @param {number} contextId Id of the context.
     *  @return {number} Display width. */
    this.getDisplayWidth = function (fileInfo, contextId) {
        var items = self._getDisplayItems(fileInfo, contextId);
        var maxWidth = 0;
        for (var i = 0; i < items.length; i++) {
            var width = self._heifReader.getItemWidth(contextId, items[i]);
            if (width > maxWidth) {
                maxWidth = width;
            }
        }
        return maxWidth;
    };

    /** A convenience function to get item id by thumbnail id.
     *  @param {HEIFReader~FileInfo} fileInfo FileInfo structure.
     *  @param {number} contextId Id of the context.
     *  @param {number} thumbnailId Id of the thumbnail.
     *  @return {number} Id of the corresponding item. */
    this.getItemIdByThumbnailId = function (fileInfo, contextId, thumbnailId) {
        // In Trak thumbnails and master images are on separate tracks with corresponding ids.
        if (fileInfo.trackProperties && fileInfo.trackProperties.length) {
            return thumbnailId;
        } else if (fileInfo.fileFeature.hasRootLevelMetaBox) {
            var refs = self._heifReader.getReferencedFromItemListByType(contextId, thumbnailId, "thmb");
            // For now, return the first id
            if (refs && refs.length) {
                return refs[0];
            }
        }
        return null;
    };

    /** A convenience method to get thumbnail context id or null.
     *  @param {HEIFReader~FileInfo} fileInfo FileInfo structure.
     *  @return {number} Thumbnail contextID or null if no thumbanails are present. */
    this.getThumbnailContextId = function (fileInfo) {
        if (fileInfo.trackProperties && fileInfo.trackProperties.length) {
            for (var i in fileInfo.trackProperties) {
                if (fileInfo.trackProperties[i].trackFeature.isThumbnailImageSequence === true) {
                    return fileInfo.trackProperties[i].trackId;
                }
            }
        }

        if (fileInfo.rootLevelMetaBoxProperties) {
            if (fileInfo.rootLevelMetaBoxProperties.metaBoxFeature.hasThumbnails === true) {
                return fileInfo.rootLevelMetaBoxProperties.contextId;
            }
        }

        return null;
    };

    /** A convenience method to get master context id or null.
     *  @param {HEIFReader~FileInfo} fileInfo FileInfo structure.
     *  @return {number} Master contextID or null if no master context is present. */
    this.getMasterContextId = function (fileInfo) {
        if (fileInfo.trackProperties && fileInfo.trackProperties.length) {
            for (var i in fileInfo.trackProperties) {
                if (fileInfo.trackProperties[i].trackFeature.isMasterImageSequence === true) {
                    return fileInfo.trackProperties[i].trackId;
                }
            }
        }

        if (fileInfo.rootLevelMetaBoxProperties) {
            return fileInfo.rootLevelMetaBoxProperties.contextId;
        }

        return null;
    };

    /** A convenience function to get thumbnail id by item id.
     *  @param {HEIFReader~FileInfo} fileInfo FileInfo structure.
     *  @param {number} contextId Id of the context.
     *  @param {number} itemId Id of the item.
     *  @return {number} Id of the corresponding thumbnail or null if there is no thumbnail. */
    this.getThumbnailIdByItemId = function (fileInfo, contextId, itemId) {
        // In Trak thumbnails and master images are on separate tracks with corresponding ids.
        if (fileInfo.trackProperties && fileInfo.trackProperties.length) {
            return itemId;
        } else if (fileInfo.fileFeature.hasRootLevelMetaBox) {
            var refs = self._heifReader.getReferencedToItemListByType(contextId, itemId, "thmb");
            // For now, return the first id
            if (refs && refs.length) {
                return refs[0];
            }
        }
        return null;
    };

    /** A convenience function to get master ids.
     *  @param {HEIFReader~FileInfo} fileInfo FileInfo structure.
     *  @return {Array.<number>} Array of master item IDs. */
    this.getMasterIds = function (fileInfo) {
        var ids = null;
        var masterContextId = self.getMasterContextId(fileInfo);
        if (fileInfo.trackProperties && fileInfo.trackProperties.length) {
            ids = [];
            for (var i in fileInfo.trackProperties) {
                if (fileInfo.trackProperties[i].trackId === masterContextId) {
                    var sampleProperties = fileInfo.trackProperties[i].sampleProperties;
                    for (var j in sampleProperties) {
                        if (sampleProperties.hasOwnProperty(j)) {
                            ids.push(parseInt(j));
                        }
                    }
                }
            }
        } else if (fileInfo.fileFeature.hasRootLevelMetaBox) {
            ids = [];
            if (masterContextId === fileInfo.rootLevelMetaBoxProperties.contextId) {
                var imageFeaturesMap = fileInfo.rootLevelMetaBoxProperties.imageFeaturesMap;
                for (i in imageFeaturesMap) {
                    if (imageFeaturesMap.hasOwnProperty(i) && imageFeaturesMap[i].isMasterImage === true) {
                        ids.push(parseInt(i));
                    }
                }
            }
        }
        return ids;
    };

    /** A convenience function to get thumbnail ids.
     *  @param {HEIFReader~FileInfo} fileInfo FileInfo structure.
     *  @return {Array.<number>} Array of thumbnail item IDs. */
    this.getThumbnailIds = function (fileInfo) {
        var ids = null;
        var thumbnailContextId = self.getThumbnailContextId(fileInfo);
        if (fileInfo.trackProperties && fileInfo.trackProperties.length) {
            ids = [];
            for (var i in fileInfo.trackProperties) {
                if (fileInfo.trackProperties[i].trackId === thumbnailContextId) {
                    var sampleProperties = fileInfo.trackProperties[i].sampleProperties;
                    for (var j in sampleProperties) {
                        if (sampleProperties.hasOwnProperty(j)) {
                            ids.push(parseInt(j));
                        }
                    }
                }
            }
        } else if (fileInfo.fileFeature.hasRootLevelMetaBox) {
            ids = [];
            if (thumbnailContextId === fileInfo.rootLevelMetaBoxProperties.contextId) {
                var imageFeaturesMap = fileInfo.rootLevelMetaBoxProperties.imageFeaturesMap;
                for (i in imageFeaturesMap) {
                    if (imageFeaturesMap.hasOwnProperty(i) && imageFeaturesMap[i].isThumbnailImage === true) {
                        ids.push(parseInt(i));
                    }
                }
            }
        }
        return ids;
    };

    /** A convenience function to get meta box thumbnail ids.
     *  @param {HEIFReader~FileInfo} fileInfo FileInfo structure.
     *  @return {Array.<number>} Array of thumbnail item IDs. */
    this.getMetaBoxThumbnailIds = function (fileInfo) {
        var ids = [];
        if (fileInfo.fileFeature.hasRootLevelMetaBox) {
            var imageFeaturesMap = fileInfo.rootLevelMetaBoxProperties.imageFeaturesMap;
            for (var i in imageFeaturesMap) {
                if (imageFeaturesMap.hasOwnProperty(i) && imageFeaturesMap[i].isThumbnailImage === true) {
                    ids.push(parseInt(i));
                }
            }
        }
        return ids;
    };

    /** A convenience function to get meta box master ids.
     *  @param {HEIFReader~FileInfo} fileInfo FileInfo structure.
     *  @return {Array.<number>} Array of thumbnail item IDs. */
    this.getMetaBoxMasterIds = function (fileInfo) {
        var ids = [];
        if (fileInfo.fileFeature.hasRootLevelMetaBox) {
            var imageFeaturesMap = fileInfo.rootLevelMetaBoxProperties.imageFeaturesMap;
            for (var i in imageFeaturesMap) {
                if (imageFeaturesMap.hasOwnProperty(i) && imageFeaturesMap[i].isMasterImage === true) {
                    ids.push(parseInt(i));
                }
            }
        }
        return ids;
    };

    // **** PUBLIC API ENDS ****
}
