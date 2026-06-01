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

/* ============================================
 * HEIF Website Theme Manager
 * ============================================
 * Handles theme switching between light and dark modes.
 * Persists user preference in localStorage.
 * Respects system preference when no explicit choice is made.
 * ============================================ */

"use strict";

var ThemeManager = (function() {
    var STORAGE_KEY = 'heif-theme-preference';
    var THEME_ATTR = 'data-theme';
    var THEMES = {
        LIGHT: 'light',
        DARK: 'dark'
    };
    
    // ========================================
    // PRIVATE METHODS
    // ========================================
    
    /**
     * Get the system's preferred color scheme
     * @returns {string} 'dark' or 'light'
     */
    function getSystemPreference() {
        if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
            return THEMES.DARK;
        }
        return THEMES.LIGHT;
    }
    
    /**
     * Get the user's saved theme preference from localStorage
     * @returns {string|null} Saved theme or null
     */
    function getSavedTheme() {
        try {
            return localStorage.getItem(STORAGE_KEY);
        } catch (e) {
            // localStorage may be unavailable (private browsing, etc.)
            return null;
        }
    }
    
    /**
     * Save the theme preference to localStorage
     * @param {string} theme - Theme to save
     */
    function saveTheme(theme) {
        try {
            localStorage.setItem(STORAGE_KEY, theme);
        } catch (e) {
            // localStorage unavailable - preference won't persist
        }
    }
    
    /**
     * Clear the saved theme preference
     */
    function clearSavedTheme() {
        try {
            localStorage.removeItem(STORAGE_KEY);
        } catch (e) {
            // Ignore errors
        }
    }
    
    /**
     * Apply a theme to the document
     * @param {string} theme - Theme to apply
     */
    function applyTheme(theme) {
        document.documentElement.setAttribute(THEME_ATTR, theme);
        
        // Dispatch custom event for other components to react
        var event;
        try {
            event = new CustomEvent('themechange', { detail: { theme: theme } });
        } catch (e) {
            // IE fallback
            event = document.createEvent('CustomEvent');
            event.initCustomEvent('themechange', true, true, { theme: theme });
        }
        document.dispatchEvent(event);
    }
    
    /**
     * Update the toggle button icon/state
     * @param {string} theme - Current theme
     */
    function updateToggleButton(theme) {
        var buttons = document.querySelectorAll('.theme-toggle-btn');
        buttons.forEach(function(btn) {
            var icon = btn.querySelector('.theme-icon');
            if (icon) {
                // Sun icon for dark mode (clicking switches to light)
                // Moon icon for light mode (clicking switches to dark)
                icon.textContent = theme === THEMES.DARK ? 'Salt' : 'Pepper';
                // Visually-hidden accessible label for screen readers
                var srLabel = btn.querySelector('.sr-only');
                if (!srLabel) {
                    srLabel = document.createElement('span');
                    srLabel.className = 'sr-only';
                    srLabel.style.cssText = 'position:absolute;width:1px;height:1px;padding:0;margin:-1px;overflow:hidden;clip:rect(0,0,0,0);border:0;';
                    btn.appendChild(srLabel);
                }
                srLabel.textContent = theme === THEMES.DARK ? 'Light theme' : 'Dark theme';
            }
            btn.setAttribute('aria-label', 
                theme === THEMES.DARK ? 'Switch to light mode' : 'Switch to dark mode');
            btn.setAttribute('title',
                theme === THEMES.DARK ? 'Switch to light mode' : 'Switch to dark mode');
        });
    }
    
    // ========================================
    // PUBLIC API
    // ========================================
    
    return {
        /**
         * Initialize the theme manager
         * Should be called as early as possible to prevent flash
         */
        init: function() {
            var saved = getSavedTheme();
            var theme = saved || getSystemPreference();
            applyTheme(theme);
            
            // Update toggle button when DOM is ready
            if (document.readyState === 'loading') {
                document.addEventListener('DOMContentLoaded', function() {
                    updateToggleButton(theme);
                });
            } else {
                updateToggleButton(theme);
            }
            
            // Listen for system preference changes
            if (window.matchMedia) {
                var mediaQuery = window.matchMedia('(prefers-color-scheme: dark)');
                var handleChange = function(e) {
                    // Only auto-switch if user hasn't set explicit preference
                    if (!getSavedTheme()) {
                        var newTheme = e.matches ? THEMES.DARK : THEMES.LIGHT;
                        applyTheme(newTheme);
                        updateToggleButton(newTheme);
                    }
                };
                
                // Modern browsers
                if (mediaQuery.addEventListener) {
                    mediaQuery.addEventListener('change', handleChange);
                } else if (mediaQuery.addListener) {
                    // Legacy Safari
                    mediaQuery.addListener(handleChange);
                }
            }
        },
        
        /**
         * Toggle between light and dark themes
         * @returns {string} The new theme
         */
        toggle: function() {
            var current = this.getCurrentTheme();
            var next = (current === THEMES.DARK) ? THEMES.LIGHT : THEMES.DARK;
            applyTheme(next);
            saveTheme(next);
            updateToggleButton(next);
            return next;
        },
        
        /**
         * Set a specific theme
         * @param {string} theme - 'light' or 'dark'
         */
        setTheme: function(theme) {
            if (theme === THEMES.LIGHT || theme === THEMES.DARK) {
                applyTheme(theme);
                saveTheme(theme);
                updateToggleButton(theme);
            }
        },
        
        /**
         * Reset to system preference
         */
        useSystemPreference: function() {
            clearSavedTheme();
            var theme = getSystemPreference();
            applyTheme(theme);
            updateToggleButton(theme);
        },
        
        /**
         * Get the current theme
         * @returns {string} Current theme ('light' or 'dark')
         */
        getCurrentTheme: function() {
            return document.documentElement.getAttribute(THEME_ATTR) || THEMES.LIGHT;
        },
        
        /**
         * Check if dark mode is active
         * @returns {boolean}
         */
        isDarkMode: function() {
            return this.getCurrentTheme() === THEMES.DARK;
        },
        
        /**
         * Theme constants for external use
         */
        THEMES: THEMES
    };
})();

// Initialize immediately to prevent flash of wrong theme
// This script should be in <head> or loaded early
ThemeManager.init();
