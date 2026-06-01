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
 * Reusable context menu for images and canvases
 * @param {Object} options Configuration options
 */
function ContextMenu(options) {
    var self = this;
    
    // Defaults
    var config = {
        menuId: 'contextMenu',
        containerSelector: 'body',
        targetTags: ['IMG', 'CANVAS'],
        actions: null
    };
    
    // Merge options
    if (options) {
        for (var key in options) {
            if (options.hasOwnProperty(key)) {
                config[key] = options[key];
            }
        }
    }
    
    // State
    var contextMenu = null;
    var targetElement = null;
    var cachedUrl = null;
    var targetTagsMap = {};
    var isVisible = false;
    
    // Cache menu dimensions (set once after first show)
    var menuWidth = 160;
    var menuHeight = 120;
    var dimensionsCached = false;
    
    // Reusable download link
    var downloadLink = null;
    
    // Long-press state for touch devices
    var longPressTimer = null;
    var longPressDelay = 500; // ms to hold before triggering
    var touchStartPos = null;
    var touchMoveThreshold = 10; // px movement allowed before canceling
    var longPressTriggered = false; // Track if our menu was triggered

    // Listener references kept for destroy(). The previous version
    // attached `hideMenu`, the Escape keydown closure and the capture-
    // phase contextmenu blocker to document/window without ever removing
    // them, so calling destroy() in a SPA-style teardown left dead
    // closures bound to the live document forever (and a stale reference
    // to the now-removed `contextMenu` element). We hold the actual
    // function values here so destroy() can call removeEventListener
    // with the matching reference + capture flag.
    var listenerEscapeHandler = null;
    var listenerContextMenuCaptureHandler = null;
    var attachedContainers = [];
    
    // Build tag lookup map
    for (var i = 0; i < config.targetTags.length; i++) {
        targetTagsMap[config.targetTags[i].toUpperCase()] = true;
    }
    
    // Default actions
    var defaultActions = {
        download: {
            label: 'Download Original',
            handler: downloadOriginal
        },
        downloadPng: {
            label: 'Download as PNG',
            handler: downloadAsImage
        },
        fullscreen: {
            label: 'View Fullscreen',
            handler: openFullscreen
        },
        copyClipboard: {
            label: 'Copy to Clipboard',
            handler: copyToClipboard
        }
    };
    
    /** Initialize the context menu */
    this.init = function() {
        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', doInit);
        } else {
            doInit();
        }
        return self;
    };
    
    function doInit() {
        contextMenu = document.getElementById(config.menuId);
        
        if (!contextMenu) {
            contextMenu = createMenuElement();
            document.body.appendChild(contextMenu);
        }
        
        // Create reusable download link
        downloadLink = document.createElement('a');
        downloadLink.style.display = 'none';
        document.body.appendChild(downloadLink);
        
        attachEventListeners();
    }
    
    /** Create menu element */
    function createMenuElement() {
        var menu = document.createElement('div');
        menu.id = config.menuId;
        menu.className = 'context-menu';
        menu.style.cssText = 'display:none;position:fixed;z-index:100000';
        
        var ul = document.createElement('ul');
        ul.setAttribute('role', 'menu');
        var actions = config.actions || ['download', 'downloadPng', 'divider', 'fullscreen', 'copyClipboard'];
        
        for (var i = 0; i < actions.length; i++) {
            var actionKey = actions[i];
            var li = document.createElement('li');
            
            if (actionKey === 'divider') {
                li.className = 'divider';
                li.setAttribute('role', 'separator');
            } else {
                li.setAttribute('role', 'menuitem');
                var action = defaultActions[actionKey];
                if (action) {
                    var a = document.createElement('a');
                    a.href = '#';
                    a.dataset.action = actionKey;
                    a.textContent = action.label;
                    li.appendChild(a);
                }
            }
            
            ul.appendChild(li);
        }
        
        menu.appendChild(ul);
        return menu;
    }
    
    /** Hide menu */
    function hideMenu() {
        if (isVisible) {
            contextMenu.style.display = 'none';
            isVisible = false;
        }
    }
    
    /** Attach event listeners */
    function attachEventListeners() {
        // Hide on click - use capture phase
        document.addEventListener('click', hideMenu, true);
        
        // Hide on scroll - passive
        window.addEventListener('scroll', hideMenu, { passive: true });
        
        // Hide on escape key
        listenerEscapeHandler = function(e) {
            if (e.key === 'Escape') hideMenu();
        };
        document.addEventListener('keydown', listenerEscapeHandler);
        
        // Context menu on containers
        var containers = document.querySelectorAll(config.containerSelector);
        for (var i = 0; i < containers.length; i++) {
            attachedContainers.push(containers[i]);
            containers[i].addEventListener('contextmenu', handleContextMenu);
            
            // Touch events for long-press on mobile
            containers[i].addEventListener('touchstart', handleTouchStart, { passive: true });
            containers[i].addEventListener('touchend', handleTouchEnd, { passive: false });
            containers[i].addEventListener('touchmove', handleTouchMove, { passive: true });
            containers[i].addEventListener('touchcancel', cancelLongPress, { passive: true });
        }
        
        // Prevent native context menu on touch devices when our menu is active
        listenerContextMenuCaptureHandler = function(e) {
            // If our long-press triggered, prevent native menu
            if (longPressTriggered) {
                e.preventDefault();
                longPressTriggered = false;
            }
        };
        document.addEventListener('contextmenu', listenerContextMenuCaptureHandler, { capture: true });
        
        // Menu action clicks - delegate
        contextMenu.addEventListener('click', handleMenuClick);
    }
    
    /** Cancel any pending long-press */
    function cancelLongPress() {
        if (longPressTimer) {
            clearTimeout(longPressTimer);
            longPressTimer = null;
        }
        touchStartPos = null;
        longPressTriggered = false;
    }
    
    /** Handle touch start - begin long-press detection */
    function handleTouchStart(e) {
        var touch = e.touches[0];
        var target = document.elementFromPoint(touch.clientX, touch.clientY);
        
        if (!target || !targetTagsMap[target.tagName]) {
            return;
        }
        
        // Don't preventDefault here - it blocks scrolling and clicks
        // We'll prevent default only when long-press actually triggers
        
        // Store touch position for move detection
        touchStartPos = { x: touch.clientX, y: touch.clientY };
        
        // Store target for use in timeout
        var longPressTarget = target;
        var longPressTouchPos = { x: touch.clientX, y: touch.clientY };
        
        // Start long-press timer
        longPressTimer = setTimeout(function() {
            longPressTimer = null;
            longPressTriggered = true; // Mark that our menu triggered
            
            // Trigger context menu at touch position
            showMenuAtPosition(longPressTarget, longPressTouchPos.x, longPressTouchPos.y);
            
            // Haptic feedback if available
            if (navigator.vibrate) {
                navigator.vibrate(50);
            }
        }, longPressDelay);
    }
    
    /** Handle touch move - cancel if moved too far */
    function handleTouchMove(e) {
        if (!longPressTimer || !touchStartPos) return;
        
        var touch = e.touches[0];
        var dx = Math.abs(touch.clientX - touchStartPos.x);
        var dy = Math.abs(touch.clientY - touchStartPos.y);
        
        if (dx > touchMoveThreshold || dy > touchMoveThreshold) {
            cancelLongPress();
        }
    }
    
    /** Handle touch end - cancel long-press if timer still pending, prevent native menu if ours triggered */
    function handleTouchEnd(e) {
        // If our long-press menu was triggered, prevent default to stop native menu
        if (longPressTriggered) {
            e.preventDefault();
        }
        cancelLongPress();
    }
    
    /** Show menu at specific position */
    function showMenuAtPosition(target, x, y) {
        targetElement = target;
        cachedUrl = null;
        
        var viewW = window.innerWidth;
        var viewH = window.innerHeight;
        
        // Bounds check
        var posX = x;
        var posY = y;
        
        if (x + menuWidth > viewW) {
            posX = x - menuWidth;
        }
        if (y + menuHeight > viewH) {
            posY = y - menuHeight;
        }
        
        // Ensure menu stays on screen
        posX = Math.max(5, posX);
        posY = Math.max(5, posY);
        
        contextMenu.style.cssText = 'display:block;position:fixed;z-index:100000;left:' + posX + 'px;top:' + posY + 'px';
        isVisible = true;
        
        // Cache dimensions on first show
        if (!dimensionsCached) {
            requestAnimationFrame(function() {
                menuWidth = contextMenu.offsetWidth || 160;
                menuHeight = contextMenu.offsetHeight || 120;
                dimensionsCached = true;
            });
        }
    }
    
    /** Handle right-click */
    function handleContextMenu(e) {
        var target = e.target;
        
        if (!targetTagsMap[target.tagName]) {
            return;
        }
        
        e.preventDefault();
        e.stopPropagation();
        
        showMenuAtPosition(target, e.clientX, e.clientY);
    }
    
    /** Handle menu item click */
    function handleMenuClick(e) {
        var target = e.target;
        if (target.tagName !== 'A') return;
        
        e.preventDefault();
        
        var actionKey = target.dataset.action;
        if (!actionKey || !targetElement) return;
        
        hideMenu();
        
        var action = defaultActions[actionKey];
        if (action && action.handler) {
            action.handler();
        }
    }
    
    /** Get original URL with caching */
    function getOriginalUrl() {
        if (cachedUrl !== null) return cachedUrl;
        
        var el = targetElement;
        
        cachedUrl = el.dataset.originalSrc ||
                    (el.tagName === 'IMG' ? el.src : null) ||
                    el.getAttribute('filename') ||
                    null;
        
        if (!cachedUrl) {
            var parent = el.parentElement;
            var depth = 0;
            while (parent && depth < 5) {
                if (parent.dataset && parent.dataset.originalSrc) {
                    cachedUrl = parent.dataset.originalSrc;
                    break;
                }
                parent = parent.parentElement;
                depth++;
            }
        }
        
        return cachedUrl;
    }
    
    /** Download original file */
    function downloadOriginal() {
        var url = getOriginalUrl();
        
        if (url) {
            var filename = url.substring(url.lastIndexOf('/') + 1) || 'image';
            downloadLink.href = url;
            downloadLink.download = filename;
            downloadLink.click();
        } else {
            downloadAsImage();
        }
    }
    
    /** Download as PNG */
    function downloadAsImage() {
        try {
            var el = targetElement;
            var canvas = el;
            
            if (el.tagName !== 'CANVAS') {
                canvas = document.createElement('canvas');
                var w = el.naturalWidth || el.width;
                var h = el.naturalHeight || el.height;
                if (!w || !h) { return; }
                canvas.width = w;
                canvas.height = h;
                canvas.getContext('2d').drawImage(el, 0, 0);
            }
            
            downloadLink.download = 'image.png';
            downloadLink.href = canvas.toDataURL('image/png');
            downloadLink.click();
        } catch (e) {
        }
    }
    
    /** Open fullscreen */
    function openFullscreen() {
        var el = targetElement;
        if (el.requestFullscreen) {
            el.requestFullscreen();
        } else if (el.webkitRequestFullscreen) {
            el.webkitRequestFullscreen();
        }
    }
    
    /** Copy to clipboard */
    function copyToClipboard() {
        try {
            var el = targetElement;
            var canvas = el;
            
            if (el.tagName !== 'CANVAS') {
                canvas = document.createElement('canvas');
                var w = el.naturalWidth || el.width;
                var h = el.naturalHeight || el.height;
                if (!w || !h) { return; }
                canvas.width = w;
                canvas.height = h;
                canvas.getContext('2d').drawImage(el, 0, 0);
            }
            
            canvas.toBlob(function(blob) {
                if (navigator.clipboard && navigator.clipboard.write) {
                    navigator.clipboard.write([
                        new ClipboardItem({ 'image/png': blob })
                    ]).catch(function(err) {
                        console.error('Copy failed:', err);
                    });
                }
            });
        } catch (e) {
        }
    }
    
    /** Add custom action */
    this.addAction = function(key, label, handler) {
        defaultActions[key] = { label: label, handler: handler };
        return self;
    };
    
    /** Get current target element */
    this.getTargetElement = function() {
        return targetElement;
    };
    
    /** Destroy and cleanup */
    this.destroy = function() {
        cancelLongPress();

        // Detach document/window listeners attached in attachEventListeners.
        // Note: removeEventListener must match the original capture flag
        // exactly, otherwise the listener stays bound (silently).
        try { document.removeEventListener('click', hideMenu, true); } catch (e) {}
        try { window.removeEventListener('scroll', hideMenu, { passive: true }); } catch (e) {}
        if (listenerEscapeHandler) {
            try { document.removeEventListener('keydown', listenerEscapeHandler); } catch (e) {}
            listenerEscapeHandler = null;
        }
        if (listenerContextMenuCaptureHandler) {
            try { document.removeEventListener('contextmenu', listenerContextMenuCaptureHandler, { capture: true }); } catch (e) {}
            listenerContextMenuCaptureHandler = null;
        }
        for (var ci = 0; ci < attachedContainers.length; ci++) {
            var c = attachedContainers[ci];
            try { c.removeEventListener('contextmenu', handleContextMenu); } catch (e) {}
            try { c.removeEventListener('touchstart', handleTouchStart, { passive: true }); } catch (e) {}
            try { c.removeEventListener('touchend', handleTouchEnd, { passive: false }); } catch (e) {}
            try { c.removeEventListener('touchmove', handleTouchMove, { passive: true }); } catch (e) {}
            try { c.removeEventListener('touchcancel', cancelLongPress, { passive: true }); } catch (e) {}
        }
        attachedContainers = [];
        if (contextMenu) {
            try { contextMenu.removeEventListener('click', handleMenuClick); } catch (e) {}
            if (contextMenu.parentNode) {
                contextMenu.parentNode.removeChild(contextMenu);
            }
        }
        if (downloadLink && downloadLink.parentNode) {
            downloadLink.parentNode.removeChild(downloadLink);
        }
        contextMenu = null;
        targetElement = null;
        downloadLink = null;
    };
}

if (typeof module !== 'undefined' && module.exports) {
    module.exports = ContextMenu;
}