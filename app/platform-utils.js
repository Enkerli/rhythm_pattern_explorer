// === PLATFORM UTILITIES MODULE ===
/**
 * Platform Detection and Cross-Platform Utilities
 * 
 * Provides robust platform detection and standardized keyboard event handling
 * to ensure consistent user experience across Mac, Windows, Linux, and mobile devices.
 * 
 * Key Features:
 * - Platform detection (Mac, Windows, Linux, iOS, Android)
 * - Standardized keyboard event handling
 * - Cross-platform modifier key detection
 * - Mobile/touch device adaptations
 * - Accessibility support for keyboard navigation
 */

class PlatformUtils {
    /**
     * Initialize platform detection and utilities
     */
    constructor() {
        this.platform = this.detectPlatform();
        this.isMobile = this.detectMobile();
        this.hasTouch = this.detectTouch();
        
        console.log(`🖥️ Platform detected: ${this.platform.name} (Mobile: ${this.isMobile}, Touch: ${this.hasTouch})`);
    }
    
    /**
     * Detect the current platform
     * @returns {Object} Platform information
     */
    detectPlatform() {
        const userAgent = navigator.userAgent;
        const platform = navigator.platform;
        
        // Mac platforms (macOS, iOS)
        if (/Mac|iPhone|iPad|iPod/.test(platform) || /Macintosh/.test(userAgent)) {
            if (/iPhone|iPad|iPod/.test(platform) || /iPhone|iPad/.test(userAgent)) {
                return {
                    name: 'iOS',
                    family: 'Apple',
                    isMac: false,
                    isIOS: true,
                    isWindows: false,
                    isLinux: false,
                    isAndroid: false,
                    modifierKey: 'metaKey',
                    modifierLabel: 'Cmd'
                };
            } else {
                return {
                    name: 'macOS',
                    family: 'Apple',
                    isMac: true,
                    isIOS: false,
                    isWindows: false,
                    isLinux: false,
                    isAndroid: false,
                    modifierKey: 'metaKey',
                    modifierLabel: 'Cmd'
                };
            }
        }
        
        // Windows platforms
        if (/Win/.test(platform) || /Windows/.test(userAgent)) {
            return {
                name: 'Windows',
                family: 'Microsoft',
                isMac: false,
                isIOS: false,
                isWindows: true,
                isLinux: false,
                isAndroid: false,
                modifierKey: 'ctrlKey',
                modifierLabel: 'Ctrl'
            };
        }
        
        // Android
        if (/Android/.test(userAgent)) {
            return {
                name: 'Android',
                family: 'Google',
                isMac: false,
                isIOS: false,
                isWindows: false,
                isLinux: false,
                isAndroid: true,
                modifierKey: 'ctrlKey',
                modifierLabel: 'Ctrl'
            };
        }
        
        // Linux and Unix-like systems
        if (/Linux|X11|FreeBSD|OpenBSD/.test(platform) || /Linux/.test(userAgent)) {
            return {
                name: 'Linux',
                family: 'Unix',
                isMac: false,
                isIOS: false,
                isWindows: false,
                isLinux: true,
                isAndroid: false,
                modifierKey: 'ctrlKey',
                modifierLabel: 'Ctrl'
            };
        }
        
        // Fallback to Windows-like behavior
        return {
            name: 'Unknown',
            family: 'Unknown',
            isMac: false,
            isIOS: false,
            isWindows: false,
            isLinux: false,
            isAndroid: false,
            modifierKey: 'ctrlKey',
            modifierLabel: 'Ctrl'
        };
    }
    
    /**
     * Detect if the current device is mobile
     * @returns {boolean} True if mobile device
     */
    detectMobile() {
        return /Android|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini|Mobile|mobile/.test(navigator.userAgent) ||
               window.innerWidth <= 768;  // Fallback based on screen width
    }
    
    /**
     * Detect if the current device supports touch
     * @returns {boolean} True if touch is supported
     */
    detectTouch() {
        return 'ontouchstart' in window ||
               navigator.maxTouchPoints > 0 ||
               navigator.msMaxTouchPoints > 0;
    }
    
    /**
     * Check if a modifier key is pressed (cross-platform)
     * @param {KeyboardEvent} event - The keyboard event
     * @returns {boolean} True if the platform's modifier key is pressed
     */
    isModifierPressed(event) {
        if (this.platform.isMac || this.platform.isIOS) {
            return event.metaKey;
        } else {
            return event.ctrlKey;
        }
    }
    
    /**
     * Get platform-appropriate keyboard shortcut text
     * @param {string} key - The key (e.g., 'Enter', 'S', 'C')
     * @returns {string} Formatted shortcut text
     */
    getShortcutText(key) {
        return `${this.platform.modifierLabel}+${key}`;
    }
    
    /**
     * Standardized keyboard event handler factory
     * @param {function} callback - Function to call when key is pressed
     * @param {Object} options - Configuration options
     * @returns {function} Event handler function
     */
    createKeyHandler(callback, options = {}) {
        const {
            key = 'Enter',
            requireModifier = false,
            preventDefault = true,
            allowRepeat = false
        } = options;
        
        return (event) => {
            // Check for correct key
            if (event.key !== key) return;
            
            // Check for key repeat if not allowed
            if (!allowRepeat && event.repeat) return;
            
            // Check modifier key requirement
            if (requireModifier && !this.isModifierPressed(event)) return;
            if (!requireModifier && this.isModifierPressed(event)) return;
            
            // Prevent default behavior if requested
            if (preventDefault) {
                event.preventDefault();
                event.stopPropagation();
            }
            
            // Call the callback
            try {
                callback(event);
            } catch (error) {
                console.error('Keyboard handler error:', error);
            }
        };
    }
    
    /**
     * Add standardized keyboard event listener
     * @param {HTMLElement} element - Element to attach listener to
     * @param {function} callback - Function to call when key is pressed
     * @param {Object} options - Configuration options
     */
    addKeyListener(element, callback, options = {}) {
        if (!element) {
            console.warn('PlatformUtils.addKeyListener: element is null or undefined');
            return;
        }
        
        const handler = this.createKeyHandler(callback, options);
        
        // Always use keydown for consistency across platforms
        element.addEventListener('keydown', handler);
        
        // Return cleanup function
        return () => element.removeEventListener('keydown', handler);
    }
    
    /**
     * Get platform-specific CSS classes for styling
     * @returns {string} Space-separated CSS class names
     */
    getPlatformClasses() {
        const classes = [];
        
        classes.push(`platform-${this.platform.name.toLowerCase()}`);
        classes.push(`family-${this.platform.family.toLowerCase()}`);
        
        if (this.isMobile) classes.push('mobile');
        if (this.hasTouch) classes.push('touch');
        if (this.platform.isMac) classes.push('mac');
        if (this.platform.isWindows) classes.push('windows');
        if (this.platform.isLinux) classes.push('linux');
        
        return classes.join(' ');
    }
    
    /**
     * Apply platform-specific CSS classes to document body
     */
    applyPlatformClasses() {
        document.body.className += ' ' + this.getPlatformClasses();
    }
    
    /**
     * Get mobile-friendly alternatives for keyboard shortcuts
     * @returns {Object} Mobile alternative suggestions
     */
    getMobileAlternatives() {
        return {
            'Enter': 'Tap the button',
            'Cmd+Enter': 'Tap "Add to Database" button',
            'Ctrl+Enter': 'Tap "Add to Database" button',
            'Escape': 'Tap outside the dialog',
            'Tab': 'Swipe or tap to navigate'
        };
    }
    
    /**
     * Update UI text for platform-specific instructions
     * @param {string} selector - CSS selector for elements to update
     * @param {string} baseText - Base instruction text with {modifier} placeholder
     */
    updatePlatformText(selector, baseText) {
        const elements = document.querySelectorAll(selector);
        const text = baseText.replace('{modifier}', this.platform.modifierLabel);
        
        elements.forEach(el => {
            if (this.isMobile) {
                const mobileText = this.getMobileText(text);
                el.textContent = mobileText;
            } else {
                el.textContent = text;
            }
        });
    }
    
    /**
     * Convert desktop instruction to mobile-friendly text
     * @param {string} text - Desktop instruction text
     * @returns {string} Mobile-friendly text
     */
    getMobileText(text) {
        return text
            .replace(/Press Enter/gi, 'Tap the button')
            .replace(/Cmd\+Enter|Ctrl\+Enter/gi, 'Tap "Add to Database"')
            .replace(/keyboard/gi, 'touch');
    }
    
    /**
     * Show platform-specific help text
     * @param {string} feature - Feature name (e.g., 'universal-input', 'progressive-offsets')
     * @returns {string} Help text for the feature
     */
    getFeatureHelp(feature) {
        const helpTexts = {
            'universal-input': {
                desktop: `Press Enter to parse pattern, ${this.getShortcutText('Enter')} to parse and add to database`,
                mobile: 'Tap "Parse & Analyze" to parse pattern, tap "Add to Database" to save'
            },
            'progressive-offsets': {
                desktop: 'Press Enter in step boxes to apply progressive offsets',
                mobile: 'Tap in step boxes then tap "Manual Offset" to apply offsets'
            },
            'barlow-transformer': {
                desktop: 'Press Enter in Target Onsets box for progressive transformations',
                mobile: 'Set target onsets then tap "Progressive" for step-by-step transformations'
            },
            'euclidean-transformer': {
                desktop: 'Press Enter in Target Onsets box for progressive transformations',
                mobile: 'Set target onsets then tap "Progressive" for step-by-step transformations'
            }
        };
        
        const help = helpTexts[feature];
        if (!help) return 'Feature help not available';
        
        return this.isMobile ? help.mobile : help.desktop;
    }
    
    /**
     * Update the entire UI with platform-specific content
     */
    updatePlatformUI() {
        // Update platform shortcuts in documentation
        this.updatePlatformShortcuts();
        
        // Add platform indicator badge
        this.addPlatformBadge();
        
        // Update tooltips with platform-specific instructions
        this.updateTooltips();
        
        console.log(`🎨 UI updated for ${this.platform.name} platform`);
    }
    
    /**
     * Update keyboard shortcuts displayed in the UI
     */
    updatePlatformShortcuts() {
        const shortcuts = document.querySelectorAll('.platform-shortcut');
        shortcuts.forEach(shortcut => {
            const text = shortcut.textContent;
            if (text.includes('Cmd+Enter') || text.includes('Ctrl+Enter')) {
                shortcut.textContent = this.getShortcutText('Enter');
            }
        });
    }
    
    /**
     * Add platform indicator badge to the UI
     */
    addPlatformBadge() {
        // Remove existing badge if present
        const existingBadge = document.querySelector('.platform-badge');
        if (existingBadge) {
            existingBadge.remove();
        }
        
        // Create new platform badge
        const badge = document.createElement('div');
        badge.className = 'platform-badge';
        badge.textContent = `${this.platform.name}${this.isMobile ? ' Mobile' : ''}`;
        badge.title = `Platform: ${this.platform.name}, Modifier key: ${this.platform.modifierLabel}`;
        
        document.body.appendChild(badge);
    }
    
    /**
     * Update tooltips with platform-specific instructions
     */
    updateTooltips() {
        // Update Progressive Offsets tooltips
        const forwardInput = document.getElementById('forwardOffsetStep');
        const backwardInput = document.getElementById('backwardOffsetStep');
        
        if (forwardInput) {
            forwardInput.title = this.isMobile ? 
                'Set increment value then tap Manual Offset' : 
                'Press Enter to step forward progressively';
        }
        
        if (backwardInput) {
            backwardInput.title = this.isMobile ? 
                'Set increment value then tap Manual Offset' : 
                'Press Enter to step backward progressively';
        }
        
        // Update target onsets tooltips
        const targetOnsets = document.getElementById('targetOnsets');
        const targetEuclideanOnsets = document.getElementById('targetEuclideanOnsets');
        
        if (targetOnsets) {
            targetOnsets.title = this.isMobile ? 
                'Set target then tap Progressive button' : 
                'Press Enter for progressive transformations';
        }
        
        if (targetEuclideanOnsets) {
            targetEuclideanOnsets.title = this.isMobile ? 
                'Set target then tap Progressive button' : 
                'Press Enter for progressive transformations';
        }
    }
}

// Create and export global platform utilities instance
const platformUtils = new PlatformUtils();

// Apply platform classes and update UI on load
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => {
        platformUtils.applyPlatformClasses();
        platformUtils.updatePlatformUI();
    });
} else {
    platformUtils.applyPlatformClasses();
    platformUtils.updatePlatformUI();
}

// Export for use in other modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { PlatformUtils, platformUtils };
}