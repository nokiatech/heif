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

function Examples () {

    var self = this;

    this._img = null;
    this._heicId = "heic-image";

    var gridHint = "Click on the images to enlarge their view. Click again to go back.";
    var coverHint = "Click on the thumbnail images to enlarge their view.";
    var stereoHint = "Click on the thumbnail image to enlarge left or right view.";

    this._exampleContentMapping = {
        "still-1": {
            title: "Single image",
            element: "img",
            content: "content/images/autumn_1440x960.heic",
            thumb: "content/pngs/autumn.png",
            detail: "File size: 287 KiB, Resolution: 1440x960"
        },
        "still-2": {
            title: "Single image",
            element: "img",
            content: "content/images/cheers_1440x960.heic",
            thumb: "content/pngs/cheers.png",
            detail: "File size: 41 KiB, Resolution: 1440x960"
        },
        "still-3": {
            title: "Single image",
            element: "img",
            content: "content/images/crowd_1440x960.heic",
            thumb: "content/pngs/crowd.png",
            detail: "File size: 128 Kib, Resolution: 1440x960"
        },
        "still-4": {
            title: "Single image",
            element: "img",
            content: "content/images/old_bridge_1440x960.heic",
            thumb: "content/pngs/old_bridge.png",
            detail: "File size: 132 KiB, Resolution: 1440x960"
        },
        "still-5": {
            title: "Single image",
            element: "img",
            content: "content/images/ski_jump_1440x960.heic",
            thumb: "content/pngs/ski_jump.png",
            detail: "File size: 115 KiB, Resolution: 1440x960"
        },
        "still-6": {
            title: "Single image",
            element: "img",
            content: "content/images/spring_1440x960.heic",
            thumb: "content/pngs/spring.png",
            detail: "File size: 51 KiB, Resolution: 1440x960"
        },
        "still-7": {
            title: "Single image",
            element: "img",
            content: "content/images/summer_1440x960.heic",
            thumb: "content/pngs/summer.png",
            detail: "File size: 196 KiB, Resolution: 1440x960"
        },
        "still-8": {
            title: "Single image",
            element: "img",
            content: "content/images/surfer_1440x960.heic",
            thumb: "content/pngs/surfer.png",
            detail: "File size: 166 KiB, Resolution: 1440x960"
        },
        "still-9": {
            title: "Single image",
            element: "img",
            content: "content/images/winter_1440x960.heic",
            thumb: "content/pngs/winter.png",
            detail: "File size: 243 KiB, Resolution: 1440x960"
        },
        "collection-1": {
            title: "Image collection",
            element: "img",
            content: "content/images/random_collection_1440x960.heic",
            thumb: "content/pngs/cheers.png",
            detail: "File size: 448 KiB",
            hint: gridHint
        },
        "collection-2": {
            title: "Image album - Seasons",
            element: "img",
            fps: "0",
            type: "cover",
            content: "content/images/season_collection_1440x960.heic",
            thumb: "content/pngs/winter.png",
            detail: "File size: 775 KiB",
            hint: coverHint
        },
        "burst-1": {
            title: "Image burst",
            element: "img",
            fps: "0",
            type: "grid",
            content: "content/image_sequences/bird_burst.heic",
            thumb: "content/pngs/bird.png",
            detail: "File size: 459 KiB",
            hint: gridHint
        },
        "burst-2": {
            title: "Image burst",
            element: "img",
            fps: "0",
            type: "grid",
            content: "content/image_sequences/rally_burst.heic",
            thumb: "content/pngs/rally.png",
            detail: "File size: 994 KiB",
            hint: gridHint
        },
        "burst-3": {
            title: "Image burst: Video & HD images",
            element: "img",
            fps: "0",
            content: "content/image_sequences/bird_burst.heic",
            burstVideo: "content/image_sequences/bird_burst.heic",
            burstFps: "24",
            thumb: "content/pngs/bird.png",
            detail: "File size: 459 KiB"
        },
        "burst-4": {
            title: "Image burst: Video & HD images",
            element: "img",
            fps: "0",
            content: "content/image_sequences/rally_burst.heic",
            burstVideo: "content/image_sequences/rally_burst.heic",
            burstFps: "24",
            thumb: "content/pngs/rally.png",
            detail: "File size: 994 KiB"
        },
        "sequence-1": {
            title: "Image sequence",
            element: "img",
            content: "content/image_sequences/starfield_animation.heic",
            thumb: "content/pngs/starfield.png",
            detail: "File size: 373 KiB",
            heifFixed512px: true
        },
        "sequence-2": {
            title: "Image sequence",
            element: "img",
            fps: "3",
            content: "content/image_sequences/sea1_animation.heic",
            thumb: "content/pngs/sea1.png",
            detail: "File size: 419 KiB",
            heifFixed512px: true
        },
        "grid-1": {
            title: "Derived Grid",
            element: "img",
            content: "content/overlay_grid_alpha/grid_960x640.heic",
            thumb: "content/pngs/winter.png",
            detail: "File size: 96 KiB, Resolution: 960x640"
        },
        "overlay-1": {
            title: "Derived Overlay",
            element: "img",
            content: "content/overlay_grid_alpha/overlay_1000x680.heic",
            thumb: "content/pngs/autumn.png",
            detail: "File size: 96 KiB, Resolution: 1000x680"
        },
        "alpha-1": {
            title: "Alpha Mask",
            element: "img",
            content: "content/overlay_grid_alpha/alpha_1440x960.heic",
            thumb: "content/pngs/winter.png",
            detail: "File size: 433 KiB, Resolution: 1440x960"
        },
        "stereo-1": {
            title: "Stereo Pair",
            element: "img",
            content: "content/heifv2/stereo_1200x800.heic",
            thumb: "content/pngs/stereo.png",
            detail: "File size: 254 KiB, Resolution: 1200x800",
            type: "grid",
            hint: stereoHint
        },
        "bothie-1": {
            title: "Time-synchronized capture / bothie",
            element: "img",
            content: "content/heifv2/bothie_1440x960.heic",
            thumb: "content/pngs/bothie.png",
            detail: "File size: 113 KiB, Resolution: 1440x960"
        },
        "udes-1": {
            title: "User description",
            element: "img",
            content: "content/heifv2/lights_1440x960.heic",
            thumb: "content/pngs/lights.png",
            detail: "File size: 60 KiB, Resolution: 1440x960"
        }
    };

    this._scrollY = 0;

    this.initialize = function () {
        console.log("Examples.");

        $("div.heif-example-content").click(function () {
            self.openModal(this.id);
        });

        $("div.heif-examples-modal-quit").click(function () {
            self.closeModal();
        });

        for (var key in this._exampleContentMapping) {
            if (this._exampleContentMapping.hasOwnProperty(key)) {
                var title = this._exampleContentMapping[key].title;
                if (title) {
                    $("div#" + key).find("div.heif-example-title").html(this._exampleContentMapping[key].title);
                }
                var thumb = this._exampleContentMapping[key].thumb;
                if (thumb) {
                    $("div#" + key).css('background-image', 'url("' + thumb + '")');
                }
            }
        }
    };

    var modalBackdropElement = "div#heif-examples-modal-backdrop";
    var modalWindowElement = "div#heif-examples-modal-window";
    var modalContentElementId = "heif-examples-modal-content";
    var modalContentElement = "div#" + modalContentElementId;

    this.openModal = function (id) {
        var modalTitle = $("div#heif-examples-modal-title");
        modalTitle.html(self._exampleContentMapping[id].detail);

        self.injectModalContent(id);
        self._scrollY = $(window).scrollTop();
        $(window).scrollTop(0);

        $(modalBackdropElement).addClass("display").fadeTo("fast", 0.9,
            function() {
                $(modalWindowElement).addClass("display");
            });
    };

    this.closeModal = function () {
        var modalContainer = $(modalBackdropElement);
        modalContainer.fadeTo("fast", 0, function () {
            modalContainer.removeClass("display");
            $(modalWindowElement).removeClass("display");
            var modalContent = $(modalContentElement).empty();
        });
        $(window).scrollTop(self._scrollY);
    };

    this.injectModalContent = function (id) {
        self.img = document.createElement(self._exampleContentMapping[id].element);
        self.img.src = self._exampleContentMapping[id].content;
        self.img.id = self._heicId;

        if (self._exampleContentMapping[id].heifFixed512px !== undefined) {
            self.img.className = "heif-fixed-512px";
        }

        if (self._exampleContentMapping[id].fps) {
            self.img.setAttribute("fps", self._exampleContentMapping[id].fps);
        }

        if (self._exampleContentMapping[id].type) {
            self.img.setAttribute("type", self._exampleContentMapping[id].type);
        }

        var hintElement = $("div#heif-examples-modal-hint");
        if (self._exampleContentMapping[id].hint) {
            hintElement.html(self._exampleContentMapping[id].hint);
            hintElement.fadeTo("fast", 1.0);
        } else {
            hintElement.html("");
        }

        var modalContent = document.getElementById(modalContentElementId);
        modalContent.appendChild(self.img);

        if (self._exampleContentMapping[id].burstVideo) {
            var burstVideoElement = document.createElement("img");
            burstVideoElement.src = self._exampleContentMapping[id].burstVideo;
            burstVideoElement.setAttribute("fps", self._exampleContentMapping[id].burstFps);
            burstVideoElement.className = "heif-burst-video";
            modalContent.appendChild(burstVideoElement);
        }

        modalContent.onclick = function () {
            hintElement.fadeTo("fast", 0.0);
        };

        processImageElements();
        processVideoElements();
    };

    this.initialize();
}
