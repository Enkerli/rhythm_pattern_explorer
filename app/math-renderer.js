// === MATHEMATICAL EQUATION RENDERER MODULE ===
/**
 * MathJax Integration and Mathematical Equation Rendering
 * 
 * Provides utilities for rendering mathematical equations using MathJax,
 * handling dynamic content updates, and ensuring equations display correctly
 * across different contexts within the Rhythm Pattern Explorer.
 * 
 * Key Features:
 * - MathJax initialization and configuration management
 * - Dynamic equation rendering for generated content
 * - Cross-browser mathematical notation support
 * - Responsive equation scaling
 * - Error handling for equation rendering failures
 * - Performance optimization for equation re-rendering
 */

class MathRenderer {
    /**
     * Initialize the Mathematical Equation Renderer
     */
    constructor() {
        this.isReady = false;
        this.pendingRenders = [];
        this.setupMathJax();
        
        console.log('📐 Mathematical equation renderer initialized');
    }
    
    /**
     * Setup MathJax integration and readiness detection
     */
    setupMathJax() {
        // Check if MathJax is already loaded
        if (window.MathJax && window.MathJax.typesetPromise) {
            this.isReady = true;
            this.processPendingRenders();
            return;
        }
        
        // Wait for MathJax to load
        this.waitForMathJax();
    }
    
    /**
     * Wait for MathJax to become available
     */
    waitForMathJax() {
        const checkInterval = setInterval(() => {
            if (window.MathJax && window.MathJax.typesetPromise) {
                clearInterval(checkInterval);
                this.isReady = true;
                console.log('📐 MathJax ready for equation rendering');
                this.processPendingRenders();
            }
        }, 100);
        
        // Timeout after 10 seconds
        setTimeout(() => {
            clearInterval(checkInterval);
            if (!this.isReady) {
                console.warn('📐 MathJax failed to load within timeout period');
            }
        }, 10000);
    }
    
    /**
     * Process any equations that were queued before MathJax was ready
     */
    processPendingRenders() {
        this.pendingRenders.forEach(({ element, callback }) => {
            this.renderElement(element, callback);
        });
        this.pendingRenders = [];
    }
    
    /**
     * Render mathematical equations in a specific element
     * @param {HTMLElement|string} element - Element or selector to render
     * @param {function} callback - Optional callback when rendering completes
     */
    renderElement(element, callback = null) {
        if (!this.isReady) {
            // Queue for later processing
            this.pendingRenders.push({ element, callback });
            return;
        }
        
        try {
            const targetElement = typeof element === 'string' ? 
                document.querySelector(element) : element;
                
            if (!targetElement) {
                console.warn('📐 Math render target element not found:', element);
                if (callback) callback(false);
                return;
            }
            
            // Use MathJax to typeset the element
            window.MathJax.typesetPromise([targetElement])
                .then(() => {
                    console.log('📐 Successfully rendered equations in element');
                    if (callback) callback(true);
                })
                .catch((error) => {
                    console.error('📐 MathJax rendering error:', error);
                    if (callback) callback(false);
                });
                
        } catch (error) {
            console.error('📐 Math rendering error:', error);
            if (callback) callback(false);
        }
    }
    
    /**
     * Render equations in the entire document
     * @param {function} callback - Optional callback when rendering completes
     */
    renderDocument(callback = null) {
        this.renderElement(document.body, callback);
    }
    
    /**
     * Create and render a mathematical equation element
     * @param {string} equation - LaTeX equation string
     * @param {boolean} displayMode - Whether to use display mode (block) or inline
     * @param {string} description - Optional description for accessibility
     * @returns {HTMLElement} Element containing the rendered equation
     */
    createEquation(equation, displayMode = true, description = null) {
        const container = document.createElement('div');
        container.className = 'math-equation-container';
        
        if (description) {
            container.setAttribute('aria-label', description);
            container.setAttribute('title', description);
        }
        
        if (displayMode) {
            container.innerHTML = `$$${equation}$$`;
            container.classList.add('display-equation');
        } else {
            container.innerHTML = `$${equation}$`;
            container.classList.add('inline-equation');
        }
        
        // Render the equation
        this.renderElement(container);
        
        return container;
    }
    
    /**
     * Create Milne's perfect balance equation with proper formatting
     * @returns {HTMLElement} Element containing Milne's equation
     */
    createMilneEquation() {
        const container = document.createElement('div');
        container.className = 'milne-equation-container';
        container.innerHTML = `
            <div class="equation-title">Milne's Perfect Balance Condition:</div>
            <div class="equation-display">$$\\sum_{j=1}^{k} e^{i2\\pi k_j/n} = 0$$</div>
            <div class="equation-legend">
                Where: $k_j$ are the onset positions, $n$ is the total number of steps, $i$ is the imaginary unit
            </div>
        `;
        
        this.renderElement(container);
        return container;
    }
    
    /**
     * Create an equation showing the center of gravity calculation
     * @returns {HTMLElement} Element containing the center of gravity equation
     */
    createCenterOfGravityEquation() {
        const container = document.createElement('div');
        container.className = 'cog-equation-container';
        container.innerHTML = `
            <div class="equation-title">Center of Gravity Calculation:</div>
            <div class="equation-display">$$CoG = \\frac{1}{k}\\sum_{j=1}^{k} e^{i2\\pi k_j/n}$$</div>
            <div class="equation-legend">
                The center of gravity is the normalized sum of onset vectors in the complex plane
            </div>
        `;
        
        this.renderElement(container);
        return container;
    }
    
    /**
     * Create an equation showing balance magnitude calculation
     * @returns {HTMLElement} Element containing the balance magnitude equation
     */
    createBalanceMagnitudeEquation() {
        const container = document.createElement('div');
        container.className = 'balance-equation-container';
        container.innerHTML = `
            <div class="equation-title">Balance Magnitude:</div>
            <div class="equation-display">$$|Balance| = \\left|\\sum_{j=1}^{k} e^{i2\\pi k_j/n}\\right|$$</div>
            <div class="equation-legend">
                The magnitude of the sum of onset vectors indicates how balanced the pattern is
            </div>
        `;
        
        this.renderElement(container);
        return container;
    }
    
    /**
     * Update mathematical content in dynamic UI elements
     * @param {string} containerId - ID of container to update
     * @param {string} mathContent - Mathematical content to render
     */
    updateDynamicMath(containerId, mathContent) {
        const container = document.getElementById(containerId);
        if (!container) {
            console.warn('📐 Dynamic math container not found:', containerId);
            return;
        }
        
        container.innerHTML = mathContent;
        this.renderElement(container);
    }
    
    /**
     * Add mathematical equations to analysis results
     * @param {Object} analysisData - Pattern analysis data
     * @returns {string} HTML with mathematical representations
     */
    generateAnalysisMath(analysisData) {
        const { balance, stepCount, onsetCount } = analysisData;
        
        return `
            <div class="analysis-math">
                <div class="math-section">
                    <strong>Pattern Parameters:</strong>
                    <div class="inline-math">
                        Steps: $n = ${stepCount}$, Onsets: $k = ${onsetCount}$
                    </div>
                </div>
                <div class="math-section">
                    <strong>Balance Magnitude:</strong>
                    <div class="inline-math">
                        $|Balance| = ${balance.magnitude.toFixed(3)}$
                    </div>
                </div>
                ${balance.isPerfectlyBalanced ? 
                    '<div class="perfect-balance-note">✨ Perfect Balance: $\\sum e^{i2\\pi k_j/n} = 0$ ✨</div>' : 
                    ''
                }
            </div>
        `;
    }
    
    /**
     * Check if MathJax is available and ready
     * @returns {boolean} True if MathJax is ready for rendering
     */
    isAvailable() {
        return this.isReady && window.MathJax && window.MathJax.typesetPromise;
    }
    
    /**
     * Get rendering statistics
     * @returns {Object} Statistics about equation rendering
     */
    getStats() {
        return {
            isReady: this.isReady,
            pendingRenders: this.pendingRenders.length,
            mathJaxLoaded: !!(window.MathJax && window.MathJax.typesetPromise)
        };
    }
}

// Create and export global math renderer instance
const mathRenderer = new MathRenderer();

// Export for use in other modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = { MathRenderer, mathRenderer };
}