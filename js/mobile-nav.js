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
 * Mobile Navigation Controller
 * Handles hamburger menu toggle, dropdown functionality, and scroll-based hiding
 * 
 * Navigation behavior:
 * - index.html (has hero section): Navbar hidden at top, slides in when user scrolls
 * - Other pages: Navbar always visible (static)
 * - Mobile (all pages): Navbar can hide on scroll down, show on scroll up
 */

(function() {
    'use strict';

    /**
     * Initialize static navbar for non-index pages
     * Index page has its own scroll-based reveal logic in index.html
     * Other pages should show navbar immediately and keep it static
     */
    function initStaticNav() {
        const navContainer = document.querySelector('.heif-nav-container');
        if (!navContainer) return;
        
        // Check if we're on the index page (has hero/welcome section)
        const isIndexPage = document.querySelector('.heif-welcome-container') !== null;
        
        if (!isIndexPage) {
            // Non-index pages: show navbar immediately (static)
            navContainer.classList.add('nav-visible');
        }
        // Index page navbar is handled by inline script in index.html
    }

    function initMobileNav() {
        const menuToggle = document.querySelector('.mobile-menu-toggle');
        const navItems = document.querySelector('.heif-nav-items');
        const navContainer = document.querySelector('.heif-nav-container');
        const dropdowns = document.querySelectorAll('.mobile-dropdown');

        // Initialize static nav for non-index pages (desktop and mobile)
        initStaticNav();

        if (!menuToggle || !navItems) return;

        // Create page overlay for darkening effect
        const pageOverlay = document.createElement('div');
        pageOverlay.className = 'mobile-menu-overlay';
        document.body.appendChild(pageOverlay);

        // Function to open menu
        function openMenu() {
            menuToggle.classList.add('active');
            navItems.classList.add('mobile-open');
            pageOverlay.classList.add('active');
            document.body.style.overflow = 'hidden';
        }

        // Function to close menu
        function closeMenu() {
            menuToggle.classList.remove('active');
            navItems.classList.remove('mobile-open');
            pageOverlay.classList.remove('active');
            document.body.style.overflow = '';
        }

        // Toggle mobile menu
        menuToggle.addEventListener('click', function(e) {
            e.preventDefault();
            if (navItems.classList.contains('mobile-open')) {
                closeMenu();
            } else {
                openMenu();
            }
        });

        // Close menu when clicking overlay
        pageOverlay.addEventListener('click', closeMenu);

        // Handle dropdown toggles
        dropdowns.forEach(function(dropdown) {
            const header = dropdown.querySelector('.mobile-dropdown-header');
            if (header) {
                header.addEventListener('click', function(e) {
                    e.preventDefault();
                    
                    // Close other dropdowns
                    dropdowns.forEach(function(other) {
                        if (other !== dropdown) {
                            other.classList.remove('open');
                        }
                    });
                    
                    dropdown.classList.toggle('open');
                });
            }
        });

        // Close menu when clicking a nav link (not dropdown header)
        navItems.querySelectorAll('a.heif-nav-item').forEach(function(link) {
            link.addEventListener('click', closeMenu);
        });

        // Close menu when clicking outside
        document.addEventListener('click', function(e) {
            if (!navItems.contains(e.target) && 
                !menuToggle.contains(e.target) && 
                !pageOverlay.contains(e.target) &&
                navItems.classList.contains('mobile-open')) {
                closeMenu();
            }
        });

        // Close menu on window resize if width > 768px
        var resizeRafPending = false;
        window.addEventListener('resize', function() {
            if (resizeRafPending) return;
            resizeRafPending = true;
            requestAnimationFrame(function() {
                resizeRafPending = false;
                if (window.innerWidth > 768) {
                    closeMenu();
                    dropdowns.forEach(function(dropdown) {
                        dropdown.classList.remove('open');
                    });
                }
            });
        });

        // Close menu on escape key
        document.addEventListener('keydown', function(e) {
            if (e.key === 'Escape' && navItems.classList.contains('mobile-open')) {
                closeMenu();
            }
        });

        // Scroll-based nav hiding on mobile
        if (navContainer && !window.matchMedia('(prefers-reduced-motion: reduce)').matches) {
            let lastScrollTop = 0;
            let scrollThreshold = 10;
            let isMenuOpen = false;

            // Track menu state
            const observer = new MutationObserver(function() {
                isMenuOpen = navItems.classList.contains('mobile-open');
            });
            observer.observe(navItems, { attributes: true, attributeFilter: ['class'] });

            var scrollRafPending = false;
            window.addEventListener('scroll', function() {
                if (scrollRafPending) return;
                scrollRafPending = true;
                requestAnimationFrame(function() {
                    scrollRafPending = false;

                    // Only apply on mobile
                    if (window.innerWidth > 768) {
                        navContainer.classList.remove('nav-hidden');
                        return;
                    }

                    // Don't hide when menu is open
                    if (isMenuOpen) {
                        navContainer.classList.remove('nav-hidden');
                        return;
                    }

                    const scrollTop = window.pageYOffset || document.documentElement.scrollTop;

                    // Scrolling down - hide nav
                    if (scrollTop > lastScrollTop && scrollTop > scrollThreshold) {
                        navContainer.classList.add('nav-hidden');
                    }
                    // Scrolling up - show nav
                    else if (scrollTop < lastScrollTop) {
                        navContainer.classList.remove('nav-hidden');
                    }

                    lastScrollTop = Math.max(0, scrollTop);
                });
            }, { passive: true });
        }
    }

    // Initialize when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', initMobileNav);
    } else {
        initMobileNav();
    }
})();
