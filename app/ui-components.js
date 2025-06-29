// === MODULE 7: UI COMPONENTS ===

/**
 * UI Components and Visualization Helpers
 * 
 * Provides comprehensive user interface utilities for the Rhythm Pattern Explorer,
 * including interactive visualizations, modal dialogs, clipboard operations,
 * and DOM manipulation helpers. Focuses on responsive design and accessibility.
 * 
 * Component Categories:
 * - Visualization: Center of gravity displays, pattern representations
 * - Modal System: Dialog boxes, alerts, confirmations
 * - Clipboard: Copy operations with user feedback
 * - DOM Utilities: Element creation, manipulation, event handling
 * - Progress Indicators: Loading states, operation feedback
 * - Responsive Design: Mobile-friendly layouts and interactions
 * 
 * Design Philosophy:
 * Components are designed to be modular, reusable, and accessible.
 * All visualizations use mathematical precision for accurate representation
 * of rhythm pattern properties while maintaining intuitive user interaction.
 */

// === VISUALIZATION HELPERS ===

/**
 * Render center of gravity visualization with onset points and CoG indicator
 * 
 * Creates an interactive circular visualization showing rhythm pattern onsets
 * positioned around a circle with the calculated center of gravity indicated.
 * Uses precise mathematical positioning for accurate geometric representation.
 * 
 * @param {Object} cogAnalysis - Center of gravity analysis results
 * @param {number} stepCount - Number of steps in the pattern
 * @returns {string} HTML string for CoG visualization
 * 
 * @example
 * const cogViz = renderCogVisualization(analysis, 8);
 * // Returns HTML with circular pattern display and CoG indicator
 * 
 * Visualization Elements:
 * - Circular boundary representing the full pattern cycle
 * - Onset points positioned at mathematically precise angles
 * - Center of gravity indicator at calculated coordinates
 * - Visual scaling for optimal display clarity
 * - Color coding for different pattern characteristics
 * 
 * Mathematical Precision:
 * - Onset angles: (position / stepCount) × 2π - π/2 (top-aligned)
 * - CoG coordinates: Scaled complex number representation
 * - Floating-point cleanup: Values < 1e-10 treated as zero
 * - Responsive scaling: Coordinates scaled to fit display area
 */
function renderCogVisualization(cogAnalysis, stepCount) {
    if (!cogAnalysis || !cogAnalysis.coordinates) {
        return '<div class="cog-visualization">No CoG data available</div>';
    }
    
    const { coordinates, normalizedMagnitude } = cogAnalysis;
    const radius = 120; // Larger radius for bigger visualization
    const centerX = 150; // Larger center coordinates
    const centerY = 150;
    
    // Calculate CoG position - scale coordinates appropriately
    // For perfectly balanced patterns, coordinates should be very close to (0,0)
    // Treat tiny floating-point values as exactly zero
    const adjustedX = Math.abs(coordinates.x) < 1e-10 ? 0 : coordinates.x;
    const adjustedY = Math.abs(coordinates.y) < 1e-10 ? 0 : coordinates.y;
    
    const cogX = centerX + adjustedX * radius * 0.8; // Scale down to fit better in circle
    const cogY = centerY + adjustedY * radius * 0.8;
    
    // Generate onset points - only show dots where there are actual beats
    let onsetPoints = '';
    const onsetPositions = cogAnalysis.onsetPositions || [];
    
    for (const position of onsetPositions) {
        const angle = (position / stepCount) * 2 * Math.PI - Math.PI / 2;
        const x = centerX + Math.cos(angle) * radius;
        const y = centerY + Math.sin(angle) * radius;
        
        onsetPoints += `
            <div class="onset-point" style="
                position: absolute;
                left: ${x - 8}px;
                top: ${y - 8}px;
                width: 16px;
                height: 16px;
                border-radius: 50%;
                background: #4CAF50;
                border: 3px solid #fff;
                box-sizing: border-box;
                transform: translate(0, 0);
                box-shadow: 0 3px 6px rgba(0,0,0,0.3);
            "></div>
        `;
    }
    
    // Also show step markers (lighter) for reference
    let stepMarkers = '';
    for (let i = 0; i < stepCount; i++) {
        const angle = (i / stepCount) * 2 * Math.PI - Math.PI / 2;
        const x = centerX + Math.cos(angle) * radius;
        const y = centerY + Math.sin(angle) * radius;
        
        stepMarkers += `
            <div class="step-marker" style="
                position: absolute;
                left: ${x - 5}px;
                top: ${y - 5}px;
                width: 10px;
                height: 10px;
                border-radius: 50%;
                background: #f0f0f0;
                border: 2px solid #ddd;
                box-sizing: border-box;
                transform: translate(0, 0);
            "></div>
        `;
    }
    
    const cogColor = normalizedMagnitude < 0.1 ? '#4CAF50' : 
                     normalizedMagnitude < 0.3 ? '#FFC107' : '#F44336';
    
    return `
        <div class="cog-visualization" style="
            position: relative;
            width: 300px;
            height: 300px;
            margin: 10px auto;
            border: 2px solid #ddd;
            border-radius: 50%;
            background: #ffffff;
        ">
            ${stepMarkers}
            ${onsetPoints}
            <div class="cog-center" style="
                position: absolute;
                left: ${centerX - 2}px;
                top: ${centerY - 2}px;
                width: 4px;
                height: 4px;
                border-radius: 50%;
                background: #333;
                box-sizing: border-box;
                transform: translate(0, 0);
            "></div>
            <div class="cog-point" style="
                position: absolute;
                left: ${cogX - (normalizedMagnitude < 0.05 ? 3 : 6)}px;
                top: ${cogY - (normalizedMagnitude < 0.05 ? 3 : 6)}px;
                width: ${normalizedMagnitude < 0.05 ? 6 : 12}px;
                height: ${normalizedMagnitude < 0.05 ? 6 : 12}px;
                border-radius: 50%;
                background: ${cogColor};
                border: ${normalizedMagnitude < 0.05 ? 1 : 2}px solid #fff;
                box-shadow: 0 2px 4px rgba(0,0,0,0.3);
                box-sizing: border-box;
                transform: translate(0, 0);
                ${normalizedMagnitude < 0.01 ? 'opacity: 0.7;' : ''}
            "></div>
            ${normalizedMagnitude > 0.01 ? `
            <div class="cog-line" style="
                position: absolute;
                left: ${centerX}px;
                top: ${centerY}px;
                width: ${Math.sqrt((cogX - centerX) ** 2 + (cogY - centerY) ** 2)}px;
                height: 1px;
                background: ${cogColor};
                transform-origin: 0 50%;
                transform: rotate(${Math.atan2(cogY - centerY, cogX - centerX)}rad);
                opacity: ${normalizedMagnitude < 0.05 ? 0.5 : 1};
            "></div>
            ` : ''}
        </div>
    `;
}

/**
 * Update progress bar display
 * @param {HTMLElement} progressBar - Progress bar element
 * @param {number} percentage - Progress percentage (0-1)
 * @param {string} text - Progress text to display
 */
function updateProgress(progressBar, percentage, text) {
    if (!progressBar) return;
    
    const fill = progressBar.querySelector('.progress-fill') || createProgressFill(progressBar);
    const label = progressBar.querySelector('.progress-text') || createProgressText(progressBar);
    
    fill.style.width = `${Math.max(0, Math.min(100, percentage * 100))}%`;
    if (label && text) {
        label.textContent = text;
    }
}

/**
 * Create progress bar fill element
 * @param {HTMLElement} container - Progress bar container
 * @returns {HTMLElement} Progress fill element
 */
function createProgressFill(container) {
    const fill = document.createElement('div');
    fill.className = 'progress-fill';
    fill.style.cssText = `
        height: 100%;
        background: linear-gradient(90deg, #4CAF50, #8BC34A);
        border-radius: inherit;
        transition: width 0.3s ease;
        width: 0%;
    `;
    container.appendChild(fill);
    return fill;
}

/**
 * Create progress bar text element
 * @param {HTMLElement} container - Progress bar container
 * @returns {HTMLElement} Progress text element
 */
function createProgressText(container) {
    const text = document.createElement('div');
    text.className = 'progress-text';
    text.style.cssText = `
        position: absolute;
        top: 50%;
        left: 50%;
        transform: translate(-50%, -50%);
        font-size: 12px;
        font-weight: bold;
        color: #333;
        text-shadow: 0 1px 2px rgba(255,255,255,0.8);
    `;
    container.appendChild(text);
    return text;
}

/**
 * Get color for balance score
 * @param {string} balanceScore - Balance score ('perfect', 'excellent', etc.)
 * @returns {string} CSS color
 */
function getBalanceColor(balanceScore) {
    const colorMap = {
        'perfect': '#4CAF50',
        'excellent': '#8BC34A',
        'good': '#FFEB3B',
        'fair': '#FF9800',
        'poor': '#F44336'
    };
    return colorMap[balanceScore] || '#666';
}

// === MODAL MANAGEMENT ===

/**
 * Show modal with specified content
 * @param {string} modalId - Modal element ID
 * @param {string} content - HTML content for modal
 * @param {Object} options - Modal options
 */
function showModal(modalId, content, options = {}) {
    let modal = document.getElementById(modalId);
    
    if (!modal) {
        modal = createModal(modalId, options);
    }
    
    if (content) {
        updateModalContent(modalId, content);
    }
    
    modal.style.display = 'block';
    document.body.style.overflow = 'hidden';
    
    // Add click-outside-to-close
    setTimeout(() => {
        modal.addEventListener('click', (e) => {
            if (e.target === modal) {
                hideModal(modalId);
            }
        });
    }, 100);
}

/**
 * Hide modal
 * @param {string} modalId - Modal element ID
 */
function hideModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) {
        modal.style.display = 'none';
        document.body.style.overflow = 'auto';
    }
}

/**
 * Create modal element
 * @param {string} modalId - Modal element ID
 * @param {Object} options - Modal options
 * @returns {HTMLElement} Modal element
 */
function createModal(modalId, options = {}) {
    const modal = document.createElement('div');
    modal.id = modalId;
    modal.className = 'modal';
    modal.style.cssText = `
        display: none;
        position: fixed;
        z-index: 1000;
        left: 0;
        top: 0;
        width: 100%;
        height: 100%;
        background-color: rgba(0, 0, 0, 0.5);
        backdrop-filter: blur(2px);
    `;
    
    const modalContent = document.createElement('div');
    modalContent.className = 'modal-content';
    modalContent.style.cssText = `
        background-color: #fefefe;
        margin: 5% auto;
        padding: 20px;
        border: none;
        border-radius: 8px;
        width: ${options.width || '80%'};
        max-width: ${options.maxWidth || '600px'};
        max-height: ${options.maxHeight || '80vh'};
        overflow-y: auto;
        box-shadow: 0 4px 20px rgba(0, 0, 0, 0.3);
        position: relative;
    `;
    
    // Add close button
    const closeBtn = document.createElement('span');
    closeBtn.innerHTML = '&times;';
    closeBtn.className = 'modal-close';
    closeBtn.style.cssText = `
        color: #aaa;
        float: right;
        font-size: 28px;
        font-weight: bold;
        cursor: pointer;
        position: absolute;
        right: 15px;
        top: 10px;
        width: 30px;
        height: 30px;
        display: flex;
        align-items: center;
        justify-content: center;
        border-radius: 50%;
        transition: all 0.2s;
    `;
    
    closeBtn.addEventListener('click', () => hideModal(modalId));
    closeBtn.addEventListener('mouseenter', (e) => {
        e.target.style.background = '#f0f0f0';
        e.target.style.color = '#000';
    });
    closeBtn.addEventListener('mouseleave', (e) => {
        e.target.style.background = 'transparent';
        e.target.style.color = '#aaa';
    });
    
    modalContent.appendChild(closeBtn);
    modal.appendChild(modalContent);
    document.body.appendChild(modal);
    
    return modal;
}

/**
 * Update modal content
 * @param {string} modalId - Modal element ID
 * @param {string} content - HTML content
 */
function updateModalContent(modalId, content) {
    const modal = document.getElementById(modalId);
    if (modal) {
        // Try to find modal content container
        let modalContent = modal.querySelector('.modal-content');
        
        // If no modal-content found, use the first child div or the modal itself
        if (!modalContent) {
            modalContent = modal.querySelector('div') || modal;
        }
        
        if (modalContent) {
            // Preserve close button if it exists
            const closeBtn = modalContent.querySelector('.modal-close');
            
            // If this is a proper modal content container, use modal-body wrapper
            if (modalContent.classList.contains('modal-content')) {
                let contentWrapper = modalContent.querySelector('.modal-body');
                if (!contentWrapper) {
                    contentWrapper = document.createElement('div');
                    contentWrapper.className = 'modal-body';
                    modalContent.appendChild(contentWrapper);
                }
                contentWrapper.innerHTML = content;
                
                // Ensure close button is at the end
                if (closeBtn && closeBtn.parentNode === modalContent) {
                    modalContent.appendChild(closeBtn);
                }
            } else {
                // For simple test modals, just update the content directly
                modalContent.innerHTML = content;
            }
        }
    }
}

/**
 * Show report modal with perfect balance analysis
 * @param {string} report - Report content
 */
function showReportModal(report) {
    const content = `
        <h2>Perfect Balance Analysis Report</h2>
        <div style="margin: 20px 0;">
            <button onclick="copyToClipboard('${report.replace(/'/g, "\\'")}'); this.textContent='Copied!'; setTimeout(() => this.textContent='Copy Report', 1000);" 
                    style="background: #4CAF50; color: white; border: none; padding: 8px 16px; border-radius: 4px; cursor: pointer; margin-bottom: 15px;">
                Copy Report
            </button>
        </div>
        <pre style="background: #f5f5f5; padding: 15px; border-radius: 4px; overflow-x: auto; white-space: pre-wrap; font-family: 'Courier New', monospace; font-size: 12px;">${report}</pre>
    `;
    
    showModal('reportModal', content, { width: '90%', maxWidth: '800px' });
}

/**
 * Show export modal for database
 * @param {string} data - Export data
 */
function showExportModal(data) {
    const content = `
        <h2>Export Pattern Database</h2>
        <p>Copy this JSON data to save your patterns:</p>
        <div style="margin: 15px 0;">
            <button onclick="copyToClipboard('${data.replace(/'/g, "\\'")}'); this.textContent='Copied!'; setTimeout(() => this.textContent='Copy All Data', 1000);" 
                    style="background: #2196F3; color: white; border: none; padding: 8px 16px; border-radius: 4px; cursor: pointer;">
                Copy All Data
            </button>
        </div>
        <textarea readonly style="width: 100%; height: 300px; font-family: 'Courier New', monospace; font-size: 11px; padding: 10px; border: 1px solid #ddd; border-radius: 4px;">${data}</textarea>
    `;
    
    showModal('exportModal', content, { width: '90%', maxWidth: '700px' });
}

/**
 * Show import modal for database
 */
function showImportModal() {
    const content = `
        <h2>Import Pattern Database</h2>
        <p>Paste JSON data to import patterns:</p>
        <textarea id="importData" placeholder="Paste JSON data here..." 
                  style="width: 100%; height: 200px; font-family: 'Courier New', monospace; font-size: 11px; padding: 10px; border: 1px solid #ddd; border-radius: 4px; margin: 10px 0;"></textarea>
        <div style="text-align: right; margin-top: 15px;">
            <button onclick="hideModal('importModal')" 
                    style="background: #666; color: white; border: none; padding: 8px 16px; border-radius: 4px; cursor: pointer; margin-right: 10px;">
                Cancel
            </button>
            <button onclick="performImport()" 
                    style="background: #4CAF50; color: white; border: none; padding: 8px 16px; border-radius: 4px; cursor: pointer;">
                Import
            </button>
        </div>
    `;
    
    showModal('importModal', content, { width: '90%', maxWidth: '600px' });
}

// === CLIPBOARD UTILITIES ===

/**
 * Copy text to clipboard with fallback support
 * @param {string} text - Text to copy
 * @returns {Promise<boolean>} Success status
 */
async function copyToClipboard(text) {
    try {
        // Try modern clipboard API first
        if (navigator.clipboard && navigator.clipboard.writeText) {
            await navigator.clipboard.writeText(text);
            showNotification(`Copied to clipboard: ${text}`, 'success', 3000);
            return true;
        } else {
            // Fallback for older browsers
            return fallbackCopy(text);
        }
    } catch (err) {
        console.warn('Clipboard copy failed, trying fallback:', err);
        return fallbackCopy(text);
    }
}

/**
 * Fallback clipboard copy using document.execCommand
 * @param {string} text - Text to copy
 * @returns {boolean} Success status
 */
function fallbackCopy(text) {
    try {
        const textArea = document.createElement('textarea');
        textArea.value = text;
        textArea.style.position = 'fixed';
        textArea.style.left = '-999999px';
        textArea.style.top = '-999999px';
        document.body.appendChild(textArea);
        textArea.focus();
        textArea.select();
        
        const success = document.execCommand('copy');
        document.body.removeChild(textArea);
        
        if (success) {
            showNotification(`Copied to clipboard: ${text}`, 'success', 3000);
        }
        return success;
    } catch (err) {
        console.error('Fallback copy failed:', err);
        return false;
    }
}

/**
 * Show visual feedback for successful copy operation
 */
function showCopyFeedback() {
    // Try to find and update copy buttons
    const copyButtons = document.querySelectorAll('button[onclick*="copyToClipboard"], .copy-btn');
    copyButtons.forEach(btn => {
        const originalText = btn.textContent;
        btn.textContent = 'Copied!';
        btn.style.background = '#4CAF50';
        
        setTimeout(() => {
            btn.textContent = originalText;
            btn.style.background = '';
        }, 1000);
    });
}

/**
 * Copy current pattern's hex representation
 */
function copyCurrentHex() {
    const hexDisplay = document.querySelector('#output .hex-value, .pattern-hex, [data-hex]');
    if (hexDisplay) {
        const hexValue = hexDisplay.textContent || hexDisplay.dataset.hex;
        if (hexValue) {
            copyToClipboard(hexValue);
        }
    }
}

// === DOM MANIPULATION HELPERS ===

/**
 * Create DOM element with attributes and content
 * @param {string} tag - HTML tag name
 * @param {Object} attributes - Element attributes
 * @param {string} content - Element content
 * @returns {HTMLElement} Created element
 */
function createElement(tag, attributes = {}, content = '') {
    const element = document.createElement(tag);
    
    Object.keys(attributes).forEach(attr => {
        if (attr === 'class') {
            element.className = attributes[attr];
        } else if (attr === 'style' && typeof attributes[attr] === 'object') {
            Object.assign(element.style, attributes[attr]);
        } else {
            element.setAttribute(attr, attributes[attr]);
        }
    });
    
    if (content) {
        element.innerHTML = content;
    }
    
    return element;
}

/**
 * Add CSS class to element
 * @param {HTMLElement} element - Target element
 * @param {string} className - Class name to add
 */
function addClass(element, className) {
    if (element && element.classList) {
        element.classList.add(className);
    }
}

/**
 * Remove CSS class from element
 * @param {HTMLElement} element - Target element
 * @param {string} className - Class name to remove
 */
function removeClass(element, className) {
    if (element && element.classList) {
        element.classList.remove(className);
    }
}

/**
 * Toggle CSS class on element
 * @param {HTMLElement} element - Target element
 * @param {string} className - Class name to toggle
 * @returns {boolean} Whether class is now present
 */
function toggleClass(element, className) {
    if (element && element.classList) {
        return element.classList.toggle(className);
    }
    return false;
}

/**
 * Clear error styling from input elements
 */
function clearErrors() {
    const errorElements = document.querySelectorAll('.error, .input-error');
    errorElements.forEach(element => {
        removeClass(element, 'error');
        removeClass(element, 'input-error');
    });
    
    const outputElement = document.getElementById('output');
    if (outputElement) {
        outputElement.style.display = 'block';
    }
}

/**
 * Show error styling on input elements
 */
function showError() {
    const inputElement = document.querySelector('#input, .pattern-input');
    if (inputElement) {
        addClass(inputElement, 'error');
    }
    
    const outputElement = document.getElementById('output');
    if (outputElement) {
        outputElement.style.display = 'none';
    }
}

/**
 * Create pattern entry for database display
 * @param {Object} pattern - Pattern object
 * @returns {HTMLElement} Pattern entry element
 */
function createPatternEntry(pattern) {
    const entry = createElement('div', { 
        class: 'pattern-entry',
        'data-id': pattern.id 
    });
    
    const favoriteClass = pattern.favorite ? 'favorite-active' : '';
    const balanceColor = pattern.perfectBalance ? getBalanceColor(pattern.perfectBalance.balanceScore) : '#666';
    
    // Generate combination description if it's a combined pattern
    let combinationInfo = '';
    const originalPatterns = pattern.originalPatterns || (pattern.combined && pattern.combined.originalPatterns);
    
    if (pattern.isCombined && originalPatterns) {
        const descriptions = originalPatterns.map(p => {
            if (p.isRegularPolygon) {
                return `P(${p.vertices},${p.offset || 0}${p.expansion && p.expansion !== 1 ? ',' + p.expansion : ''})`;
            } else if (p.isEuclidean) {
                return `E(${p.beats},${p.stepCount},${p.offset || 0})`;
            } else if (p.binary) {
                return `b${p.binary}`;
            } else if (p.stepCount) {
                return `${p.stepCount}steps`;
            } else {
                return 'pattern';
            }
        });
        combinationInfo = `
            <div class="combination-info" style="background: #e3f2fd; color: #1976d2; padding: 5px 10px; border-radius: 3px; margin: 5px 0; font-weight: bold; border-left: 4px solid #2196f3;">
                🎯 COMBINED: ${descriptions.join(' + ')}
            </div>
        `;
    }
    
    entry.innerHTML = `
        <div class="pattern-header">
            <span class="pattern-name" onclick="editPatternName(this, '${pattern.id}')">${pattern.name || 'Unnamed Pattern'}</span>
            <div class="pattern-actions">
                <button class="favorite-btn ${favoriteClass}" onclick="toggleFavorite('${pattern.id}')" title="Toggle Favorite">
                    ★
                </button>
                <button class="copy-btn" onclick="copyToClipboard('${pattern.binary}')" title="Copy Binary">
                    📋
                </button>
                <button class="delete-btn" onclick="deletePattern('${pattern.id}')" title="Delete Pattern">
                    🗑️
                </button>
            </div>
        </div>
        ${combinationInfo}
        <div class="pattern-details">
            <div class="pattern-info">
                <span class="step-count">${pattern.stepCount} steps</span>
                <span class="pattern-binary" style="font-family: monospace;">${pattern.binary}</span>
                <span class="pattern-hex" style="font-family: monospace;">${pattern.hex || ''}</span>
            </div>
            ${pattern.perfectBalance ? `
                <div class="balance-info" style="color: ${balanceColor};">
                    Balance: ${pattern.perfectBalance.balanceScore}
                    ${pattern.perfectBalance.magnitude ? `(${pattern.perfectBalance.magnitude.toFixed(3)})` : ''}
                </div>
            ` : ''}
        </div>
    `;
    
    return entry;
}

// === UI INTERACTION UTILITIES ===

/**
 * Toggle info box visibility
 * @param {string} infoBoxId - ID of the info box to toggle
 */
function toggleInfoBox(infoBoxId) {
    const infoBox = document.getElementById(infoBoxId);
    if (infoBox) {
        infoBox.style.display = infoBox.style.display === 'none' ? 'block' : 'none';
    }
}

// === EVENT HANDLING UTILITIES ===

/**
 * Add event listener to element
 * @param {HTMLElement} element - Target element
 * @param {string} event - Event type
 * @param {Function} handler - Event handler
 */
function addEvent(element, event, handler) {
    if (element && element.addEventListener) {
        element.addEventListener(event, handler);
    }
}

/**
 * Remove event listener from element
 * @param {HTMLElement} element - Target element
 * @param {string} event - Event type
 * @param {Function} handler - Event handler
 */
function removeEvent(element, event, handler) {
    if (element && element.removeEventListener) {
        element.removeEventListener(event, handler);
    }
}

/**
 * Debounce function calls
 * @param {Function} func - Function to debounce
 * @param {number} wait - Wait time in milliseconds
 * @returns {Function} Debounced function
 */
function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        const later = () => {
            clearTimeout(timeout);
            func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
    };
}

/**
 * Throttle function calls
 * @param {Function} func - Function to throttle
 * @param {number} limit - Time limit in milliseconds
 * @returns {Function} Throttled function
 */
function throttle(func, limit) {
    let inThrottle;
    return function() {
        const args = arguments;
        const context = this;
        if (!inThrottle) {
            func.apply(context, args);
            inThrottle = true;
            setTimeout(() => inThrottle = false, limit);
        }
    };
}

// === ANIMATION AND EFFECTS ===

/**
 * Animate element with CSS transitions
 * @param {HTMLElement} element - Element to animate
 * @param {Object} properties - CSS properties to animate
 * @param {number} duration - Animation duration in milliseconds
 * @returns {Promise} Promise that resolves when animation completes
 */
function animate(element, properties, duration = 300) {
    return new Promise((resolve) => {
        if (!element) {
            resolve();
            return;
        }
        
        // Set transition
        element.style.transition = `all ${duration}ms ease`;
        
        // Apply properties
        Object.keys(properties).forEach(prop => {
            element.style[prop] = properties[prop];
        });
        
        // Clean up after animation
        setTimeout(() => {
            element.style.transition = '';
            resolve();
        }, duration);
    });
}

/**
 * Fade in element
 * @param {HTMLElement} element - Element to fade in
 * @param {number} duration - Animation duration
 * @returns {Promise} Animation promise
 */
function fadeIn(element, duration = 300) {
    if (!element) return Promise.resolve();
    
    element.style.opacity = '0';
    element.style.display = 'block';
    
    return animate(element, { opacity: '1' }, duration);
}

/**
 * Fade out element
 * @param {HTMLElement} element - Element to fade out
 * @param {number} duration - Animation duration
 * @returns {Promise} Animation promise
 */
function fadeOut(element, duration = 300) {
    if (!element) return Promise.resolve();
    
    return animate(element, { opacity: '0' }, duration).then(() => {
        element.style.display = 'none';
    });
}

// === FORM AND INPUT UTILITIES ===

/**
 * Validate input element
 * @param {HTMLElement} input - Input element
 * @param {string} type - Validation type
 * @returns {boolean} Validation result
 */
function validateInput(input, type) {
    if (!input) return false;
    
    const value = input.value.trim();
    
    switch (type) {
        case 'number':
            const num = parseInt(value);
            return !isNaN(num) && num > 0;
        
        case 'email':
            return /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(value);
        
        case 'required':
            return value.length > 0;
        
        case 'pattern':
            return /^[01]+$/.test(value) || /^0x[0-9A-Fa-f]+$/.test(value);
        
        default:
            return true;
    }
}

/**
 * Show validation error on input
 * @param {HTMLElement} input - Input element
 * @param {string} message - Error message
 */
function showInputError(input, message) {
    if (!input) return;
    
    addClass(input, 'error');
    
    // Remove existing error message
    const existingError = input.parentNode.querySelector('.error-message');
    if (existingError) {
        existingError.remove();
    }
    
    // Add error message
    if (message) {
        const errorElement = createElement('div', { class: 'error-message' }, message);
        errorElement.style.cssText = 'color: #f44336; font-size: 12px; margin-top: 4px;';
        input.parentNode.appendChild(errorElement);
    }
}

/**
 * Clear validation error from input
 * @param {HTMLElement} input - Input element
 */
function clearInputError(input) {
    if (!input) return;
    
    removeClass(input, 'error');
    
    const errorMessage = input.parentNode.querySelector('.error-message');
    if (errorMessage) {
        errorMessage.remove();
    }
}

// === UTILITY CLASSES ===

/**
 * UIComponents class for compatibility
 * Provides a single class interface to all UI utilities
 */
class UIComponents {
    static renderCogVisualization = renderCogVisualization;
    static updateProgress = updateProgress;
    static getBalanceColor = getBalanceColor;
    static showModal = showModal;
    static hideModal = hideModal;
    static updateModalContent = updateModalContent;
    static showReportModal = showReportModal;
    static showExportModal = showExportModal;
    static showImportModal = showImportModal;
    static copyToClipboard = copyToClipboard;
    static showCopyFeedback = showCopyFeedback;
    static copyCurrentHex = copyCurrentHex;
    static createElement = createElement;
    static addClass = addClass;
    static removeClass = removeClass;
    static toggleClass = toggleClass;
    static clearErrors = clearErrors;
    static showError = showError;
    static createPatternEntry = createPatternEntry;
    static addEvent = addEvent;
    static removeEvent = removeEvent;
    static debounce = debounce;
    static throttle = throttle;
    static animate = animate;
    static fadeIn = fadeIn;
    static fadeOut = fadeOut;
    static validateInput = validateInput;
    static showInputError = showInputError;
    static clearInputError = clearInputError;
}

/**
 * UI Utilities namespace for organized access
 */
const UIUtils = {
    // Visualization
    renderCogVisualization,
    updateProgress,
    getBalanceColor,
    
    // Modals
    showModal,
    hideModal,
    updateModalContent,
    showReportModal,
    showExportModal,
    showImportModal,
    
    // Clipboard
    copyToClipboard,
    showCopyFeedback,
    copyCurrentHex,
    
    // DOM
    createElement,
    addClass,
    removeClass,
    toggleClass,
    clearErrors,
    showError,
    createPatternEntry,
    
    // Events
    addEvent,
    removeEvent,
    debounce,
    throttle,
    
    // Animation
    animate,
    fadeIn,
    fadeOut,
    
    // Forms
    validateInput,
    showInputError,
    clearInputError
};

/**
 * Visualization Helpers namespace
 */
const VisualizationHelpers = {
    renderCogVisualization,
    updateProgress,
    getBalanceColor,
    createProgressFill,
    createProgressText
};

/**
 * Modal Manager namespace
 */
const ModalManager = {
    show: showModal,
    hide: hideModal,
    setContent: updateModalContent,
    showReport: showReportModal,
    showExport: showExportModal,
    showImport: showImportModal
};

/**
 * Clipboard Utilities namespace
 */
const ClipboardUtils = {
    copy: copyToClipboard,
    showFeedback: showCopyFeedback,
    copyHex: copyCurrentHex
};

/**
 * DOM Utilities namespace
 */
const DOMUtils = {
    create: createElement,
    addClass,
    removeClass,
    toggleClass,
    select: (selector) => document.querySelector(selector),
    selectAll: (selector) => document.querySelectorAll(selector),
    on: addEvent,
    off: removeEvent,
    clearErrors,
    showError
};

// Export to global scope for browser compatibility
if (typeof window !== 'undefined') {
    // Main UIComponents class for compatibility
    window.UIComponents = UIComponents;
    
    // Core UI utilities
    window.UIUtils = UIUtils;
    window.VisualizationHelpers = VisualizationHelpers;
    window.ModalManager = ModalManager;
    window.ClipboardUtils = ClipboardUtils;
    window.DOMUtils = DOMUtils;
    
    // Individual functions for direct access
    window.renderCogVisualization = renderCogVisualization;
    window.updateProgress = updateProgress;
    window.showModal = showModal;
    window.hideModal = hideModal;
    window.updateModalContent = updateModalContent;
    window.showReportModal = showReportModal;
    window.showExportModal = showExportModal;
    window.showImportModal = showImportModal;
    window.copyToClipboard = copyToClipboard;
    window.createElement = createElement;
    window.addClass = addClass;
    window.removeClass = removeClass;
    window.toggleClass = toggleClass;
    window.clearErrors = clearErrors;
    window.showError = showError;
    window.createPatternEntry = createPatternEntry;
    window.addEvent = addEvent;
    window.removeEvent = removeEvent;
    window.debounce = debounce;
    window.throttle = throttle;
    window.animate = animate;
    window.fadeIn = fadeIn;
    window.fadeOut = fadeOut;
    window.validateInput = validateInput;
    window.showInputError = showInputError;
    window.clearInputError = clearInputError;
    window.getBalanceColor = getBalanceColor;
    
    // Notification system
    window.showNotification = showNotification;
    window.hideNotification = hideNotification;
    window.clearNotifications = clearNotifications;
    
    // File operations
    window.downloadFile = downloadFile;
    window.selectFile = selectFile;
}

// === NOTIFICATION SYSTEM ===

/**
 * Show a notification message
 * @param {string} message - Notification message
 * @param {string} type - Notification type ('success', 'error', 'warning', 'info')
 * @param {number} duration - Auto-hide duration in milliseconds (0 = no auto-hide)
 * @returns {HTMLElement} Notification element
 */
function showNotification(message, type = 'info', duration = 5000) {
    const container = document.getElementById('notification-container') || createNotificationContainer();
    
    // Create notification element
    const notification = createElement('div', {
        class: `notification ${type}`
    });
    
    // Add icon based on type
    const iconMap = {
        success: '✅',
        error: '❌',
        warning: '⚠️',
        info: 'ℹ️'
    };
    
    const icon = createElement('span', {
        class: 'notification-icon'
    }, iconMap[type] || iconMap.info);
    
    // Add content
    const content = createElement('div', {
        class: 'notification-content'
    });
    
    // Split message into title and content if it contains newlines
    const lines = message.split('\n');
    if (lines.length > 1) {
        const title = createElement('div', {
            class: 'notification-title'
        }, lines[0]);
        
        const messageDiv = createElement('div', {
            class: 'notification-message'
        }, lines.slice(1).join('\n'));
        
        content.appendChild(title);
        content.appendChild(messageDiv);
    } else {
        content.textContent = message;
    }
    
    // Add close button
    const closeBtn = createElement('button', {
        class: 'notification-close',
        'aria-label': 'Close notification'
    }, '×');
    
    // Assemble notification
    notification.appendChild(icon);
    notification.appendChild(content);
    notification.appendChild(closeBtn);
    
    // Add to container
    container.appendChild(notification);
    
    // Show with animation
    setTimeout(() => {
        notification.classList.add('show');
    }, 10);
    
    // Auto-hide
    let autoHideTimeout;
    if (duration > 0) {
        autoHideTimeout = setTimeout(() => {
            hideNotification(notification);
        }, duration);
    }
    
    // Close button event
    closeBtn.addEventListener('click', () => {
        if (autoHideTimeout) clearTimeout(autoHideTimeout);
        hideNotification(notification);
    });
    
    // Store reference for potential cleanup
    notification._autoHideTimeout = autoHideTimeout;
    
    return notification;
}

/**
 * Hide a specific notification
 * @param {HTMLElement} notification - Notification element to hide
 */
function hideNotification(notification) {
    if (!notification || !notification.parentNode) return;
    
    // Clear auto-hide timeout
    if (notification._autoHideTimeout) {
        clearTimeout(notification._autoHideTimeout);
    }
    
    // Hide with animation
    notification.classList.remove('show');
    
    // Remove from DOM after animation
    setTimeout(() => {
        if (notification.parentNode) {
            notification.parentNode.removeChild(notification);
        }
    }, 300);
}

/**
 * Clear all notifications
 */
function clearNotifications() {
    const container = document.getElementById('notification-container');
    if (!container) return;
    
    const notifications = container.querySelectorAll('.notification');
    notifications.forEach(notification => {
        hideNotification(notification);
    });
}

/**
 * Create notification container if it doesn't exist
 * @returns {HTMLElement} Notification container
 */
function createNotificationContainer() {
    let container = document.getElementById('notification-container');
    if (!container) {
        container = createElement('div', {
            id: 'notification-container',
            class: 'notification-container'
        });
        document.body.appendChild(container);
    }
    return container;
}

// === FILE OPERATIONS ===

/**
 * Download content as a file
 * @param {string} content - File content
 * @param {string} filename - Name of the file to download
 * @param {string} mimeType - MIME type of the file (default: 'text/plain')
 */
function downloadFile(content, filename, mimeType = 'text/plain') {
    try {
        const blob = new Blob([content], { type: mimeType });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = filename;
        a.style.display = 'none';
        
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        URL.revokeObjectURL(url);
        
        return true;
    } catch (error) {
        console.error('Download failed:', error);
        return false;
    }
}

/**
 * Open file selection dialog
 * @param {string} accept - File types to accept (e.g., '.json', '.txt', 'image/*')
 * @param {boolean} multiple - Allow multiple file selection
 * @returns {Promise<File[]>} Promise that resolves with selected files
 */
function selectFile(accept = '*', multiple = false) {
    return new Promise((resolve, reject) => {
        const fileInput = document.createElement('input');
        fileInput.type = 'file';
        fileInput.accept = accept;
        fileInput.multiple = multiple;
        fileInput.style.display = 'none';
        
        fileInput.addEventListener('change', (event) => {
            const files = Array.from(event.target.files);
            document.body.removeChild(fileInput);
            resolve(files);
        });
        
        fileInput.addEventListener('cancel', () => {
            document.body.removeChild(fileInput);
            resolve([]);
        });
        
        document.body.appendChild(fileInput);
        fileInput.click();
    });
}

/**
 * Toggle visibility of info boxes with MathJax rendering support
 * @param {string} infoBoxId - ID of the info box to toggle
 */
function toggleInfoBox(infoBoxId) {
    const infoBox = document.getElementById(infoBoxId);
    if (infoBox) {
        const wasHidden = infoBox.style.display === 'none';
        infoBox.style.display = wasHidden ? 'block' : 'none';
        
        // If the box is now visible and contains mathematical equations, render with MathJax
        if (wasHidden && typeof mathRenderer !== 'undefined') {
            const hasMathContent = infoBox.classList.contains('tex2jax_process') || 
                                 infoBox.querySelector('.MathJax') ||
                                 infoBox.textContent.includes('$$') ||
                                 infoBox.textContent.includes('$');
            
            if (hasMathContent) {
                setTimeout(() => {
                    mathRenderer.renderElement(infoBox, (success) => {
                        if (success) {
                            console.log(`📐 Mathematical equations rendered in ${infoBoxId}`);
                        } else {
                            console.warn(`📐 Failed to render equations in ${infoBoxId}`);
                        }
                    });
                }, 10); // Small delay to ensure DOM is updated
            }
        }
    }
}