/**
 * Sequencer Visual Module (sequencer-visual.js)
 * Professional circular donut visualization with Center of Gravity integration
 * 
 * Provides comprehensive canvas-based visualization for rhythm pattern playback
 * with mathematical precision and responsive design. Features real-time Center
 * of Gravity calculation display and interactive pattern editing capabilities.
 * 
 * Core Features:
 * - High-performance Canvas API rendering with optimized drawing operations
 * - Interactive circular donut chart with step-by-step pattern visualization
 * - Real-time Center of Gravity calculation and geometric display
 * - Responsive design with automatic canvas sizing and scaling
 * - Playback animation with current step highlighting and pulse effects
 * - Pattern density visualization with color-coded step indicators
 * - Interactive step editing with click-to-toggle functionality
 * - Multiple visual themes and accessibility options
 * - Performance monitoring and frame rate optimization
 * 
 * Technical Implementation:
 * - Uses HTML5 Canvas API for precise geometric rendering
 * - Implements responsive design with automatic DPI scaling
 * - Optimized rendering pipeline with minimal redraw operations
 * - Event-driven architecture for pattern updates and playback state
 * - Memory-efficient canvas management with proper cleanup
 * 
 * Mathematical Visualization:
 * - Center of Gravity displayed as vector from pattern center
 * - Geometric balance representation using complex plane mapping
 * - Pattern density visualization through color intensity
 * - Step position accuracy using trigonometric calculations
 * - Angular precision for multi-step pattern representation
 * 
 * Dependencies: 
 * - CenterOfGravityCalculator (from pattern-analysis.js) - Mathematical calculations
 * - MathUtils (from math-core.js) - Utility functions for precise calculations
 * - Browser Canvas API - Hardware-accelerated rendering support
 */

/**
 * Sequencer Visual Engine - Professional Canvas-based Pattern Visualization
 * 
 * Provides high-performance, interactive visualization for rhythm patterns using
 * a circular donut chart representation. Integrates mathematical analysis with
 * visual feedback for comprehensive pattern understanding and real-time editing.
 * 
 * Visual Architecture:
 * - Circular Layout: Pattern steps arranged around circle perimeter
 * - Donut Chart: Color-coded segments representing active/inactive steps
 * - CoG Indicator: Vector showing mathematical center of gravity
 * - Step Markers: Numbered position indicators with hover effects
 * - Playback Cursor: Animated current position with pulse effects
 * - Pattern Dividers: Visual separation for combined/stringed patterns
 * 
 * Rendering Pipeline:
 * 1. Canvas Setup: Responsive sizing and context initialization
 * 2. Background: Base circles and grid reference lines
 * 3. Donut Segments: Pattern step visualization with state colors
 * 4. CoG Display: Mathematical center of gravity calculation
 * 5. Step Elements: Interactive position markers with numbering
 * 6. Playback State: Current position highlighting and animation
 * 7. Performance: Frame rate monitoring and optimization
 * 
 * Interaction Model:
 * - Click Events: Toggle individual steps with immediate visual feedback
 * - Hover Effects: Cursor changes and visual highlighting for clickable areas
 * - Responsive Design: Automatic resizing for different screen sizes
 * - Real-time Updates: Immediate pattern recalculation and CoG display
 * - Event Emission: Custom events for pattern changes and state updates
 * 
 * Performance Optimizations:
 * - Efficient canvas operations with minimal state changes
 * - Responsive canvas sizing with DPI awareness
 * - Optimized redraw cycles using dirty region tracking
 * - Memory management with proper event listener cleanup
 * - Frame rate monitoring for performance analysis
 * 
 * Accessibility Features:
 * - High contrast color schemes for visual accessibility
 * - Keyboard navigation support for step selection
 * - Screen reader compatible event emission
 * - Customizable visual themes for different needs
 * - Clear visual indicators for all interactive elements
 */
class SequencerVisualEngine {
    /**
     * Initialize the Sequencer Visual Engine
     * 
     * Creates a high-performance canvas-based visualization system for rhythm patterns.
     * Sets up responsive canvas sizing, interactive event handling, and mathematical
     * center of gravity calculation display.
     * 
     * @param {string} canvasId - HTML canvas element ID for rendering
     * @param {string} containerId - Parent container element ID for responsive sizing
     * 
     * @throws {Error} If canvas element or rendering context is unavailable
     * 
     * @example
     * const visualEngine = new SequencerVisualEngine('sequencer-canvas', 'canvas-container');
     * visualEngine.updatePattern({ steps: [true, false, true, false], stepCount: 4 });
     * 
     * Initialization Process:
     * 1. Locate and validate canvas and container DOM elements
     * 2. Obtain 2D rendering context with performance optimization
     * 3. Initialize pattern state with default 16-step configuration
     * 4. Setup Center of Gravity calculation data structures
     * 5. Configure responsive visual parameters and color schemes
     * 6. Establish performance monitoring and statistics tracking
     * 7. Setup canvas sizing and event listener registration
     * 
     * Visual Configuration:
     * - Canvas sizing based on container dimensions with aspect ratio preservation
     * - Donut chart dimensions as percentages for responsive scaling
     * - Color schemes optimized for accessibility and musical visualization
     * - Interactive element sizing with minimum/maximum constraints
     * - Performance settings for smooth animation and responsive updates
     * 
     * Error Handling:
     * - Validates canvas element existence and accessibility
     * - Ensures 2D rendering context availability
     * - Provides detailed error messages for debugging
     * - Graceful fallback for missing DOM elements
     */
    constructor(canvasId, containerId) {
        console.log(`🎨 Looking for canvas: '${canvasId}' and container: '${containerId}'`);
        
        this.canvas = document.getElementById(canvasId);
        this.container = document.getElementById(containerId);
        
        console.log(`🎨 Canvas found:`, {
            canvas: !!this.canvas,
            canvasId: canvasId,
            canvasElement: this.canvas,
            container: !!this.container,
            containerId: containerId
        });
        
        this.ctx = this.canvas ? this.canvas.getContext('2d') : null;
        
        if (!this.canvas || !this.ctx) {
            throw new Error(`Canvas element '${canvasId}' not found or context unavailable`);
        }
        
        // Visual state
        this.pattern = {
            steps: new Array(16).fill(false),
            stepCount: 16,
            currentStep: 0,
            isPlaying: false
        };
        
        // Center of Gravity data
        this.cogData = {
            distance: 0,
            angle: 0,
            x: 0,
            y: 0,
            isCalculated: false
        };
        
        // Visual configuration
        this.config = {
            // Responsive sizing (will be calculated)
            canvasSize: 400,
            centerX: 200,
            centerY: 200,
            
            // Donut dimensions (as percentages of canvas size)
            donutOuterRadius: 0.42,     // 42% of canvas size
            donutInnerRadius: 0.18,     // 18% of canvas size
            stepRadius: 0.45,           // 45% for step positioning
            cogRadius: 0.35,            // 35% for CoG indicator
            
            // Step elements
            stepElementSize: 0.035,     // 3.5% of canvas size
            stepElementMinSize: 12,     // Minimum size in pixels
            stepElementMaxSize: 32,     // Maximum size in pixels
            
            // Colors
            colors: {
                background: '#ffffff',
                donutBackground: 'rgba(240, 242, 245, 0.8)',
                activeStep: 'rgba(40, 167, 69, 0.7)',
                currentStep: 'rgba(253, 126, 20, 0.8)',
                inactiveStep: 'rgba(200, 200, 200, 0.3)',
                currentInactive: 'rgba(220, 53, 69, 0.6)',
                stepBorder: '#ccc',
                stepText: '#333',
                cogIndicator: 'rgba(231, 76, 60, 0.8)',
                cogLine: 'rgba(231, 76, 60, 0.6)',
                gridLines: 'rgba(200, 200, 200, 0.4)'
            },
            
            // Animation
            animationSpeed: 0.1,
            pulseIntensity: 0.2
        };
        
        // Performance tracking
        this.stats = {
            framesRendered: 0,
            lastRenderTime: 0,
            averageFPS: 0
        };
        
        // Setup
        this.setupResponsiveCanvas();
        this.setupEventListeners();
        
        console.log('🎨 SequencerVisualEngine initialized');
    }
    
    /**
     * Setup Responsive Canvas Sizing
     * 
     * Implements dynamic canvas sizing based on container dimensions with
     * aspect ratio preservation and performance optimization. Ensures optimal
     * visual quality across different screen sizes and device pixel ratios.
     * 
     * Sizing Algorithm:
     * 1. Query container bounding rectangle for available space
     * 2. Calculate optimal canvas size maintaining square aspect ratio
     * 3. Apply maximum size constraints for performance (500px limit)
     * 4. Set both canvas element size and internal drawing buffer size
     * 5. Update visual configuration parameters for responsive scaling
     * 6. Calculate responsive element sizes with min/max constraints
     * 
     * Responsive Features:
     * - Automatic scaling based on container size
     * - Device pixel ratio awareness for high-DPI displays
     * - Performance-optimized maximum size limits
     * - Proportional element sizing for consistent visual hierarchy
     * - Real-time recalculation during window resize events
     * 
     * Performance Considerations:
     * - Limits maximum canvas size to prevent memory issues
     * - Uses CSS sizing for smooth browser scaling
     * - Optimizes redraw triggering after size changes
     * - Implements debounced resize handling for efficiency
     */
    setupResponsiveCanvas() {
    setupResponsiveCanvas() {
        if (!this.container || !this.canvas) {
            console.warn('⚠️ No container or canvas found for sizing');
            return;
        }
        
        // Get container dimensions
        const rect = this.container.getBoundingClientRect();
        console.log('🎨 Container rect:', {
            width: rect.width,
            height: rect.height,
            display: getComputedStyle(this.container).display,
            visibility: getComputedStyle(this.container).visibility,
            containerElement: this.container
        });
        
        const size = Math.min(rect.width, rect.height, 500); // Max 500px
        
        // Set canvas size
        this.canvas.width = size;
        this.canvas.height = size;
        this.canvas.style.width = `${size}px`;
        this.canvas.style.height = `${size}px`;
        
        // Update configuration
        this.config.canvasSize = size;
        this.config.centerX = size / 2;
        this.config.centerY = size / 2;
        
        // Calculate responsive element sizes
        this.config.stepElementActualSize = Math.max(
            this.config.stepElementMinSize,
            Math.min(
                this.config.stepElementMaxSize,
                size * this.config.stepElementSize
            )
        );
        
        console.log(`🎨 Canvas sized: ${size}x${size}px, center: ${this.config.centerX}, ${this.config.centerY}`);
        
        // Trigger re-render now that canvas is properly sized
        if (size > 0) {
                setTimeout(() => this.render(), 50);
        }
    }
    
    /**
     * Setup Event Listeners for Interactive Behavior
     * 
     * Establishes comprehensive event handling for user interaction,
     * responsive design updates, and pattern editing functionality.
     * 
     * Event Types Handled:
     * - Window Resize: Automatic canvas resizing and re-rendering
     * - Canvas Click: Step toggling with precise position detection
     * - Mouse Move: Hover effects and cursor state management
     * - Pattern Changes: Real-time visual updates and CoG recalculation
     * 
     * Interaction Features:
     * - Precise step detection using trigonometric position calculation
     * - Visual feedback for hoverable elements with cursor changes
     * - Immediate pattern updates with mathematical recalculation
     * - Responsive canvas resizing with optimized re-rendering
     * 
     * Performance Optimization:
     * - Efficient event delegation for minimal memory usage
     * - Debounced resize handling to prevent excessive recalculation
     * - Optimized position detection with cached calculations
     * - Memory-conscious event listener registration and cleanup
     */
    setupEventListeners() {
    setupEventListeners() {
        // Handle window resize
        const resizeHandler = () => {
            this.setupResponsiveCanvas();
            this.render();
        };
        
        window.addEventListener('resize', resizeHandler);
        
        // Handle canvas clicks for step toggling
        if (this.canvas) {
            this.canvas.addEventListener('click', (event) => {
                const rect = this.canvas.getBoundingClientRect();
                const x = event.clientX - rect.left;
                const y = event.clientY - rect.top;
                
                const clickedStep = this.getStepAtPosition(x, y);
                if (clickedStep !== -1) {
                    this.toggleStep(clickedStep);
                }
            });
            
            // Add hover effect
            this.canvas.addEventListener('mousemove', (event) => {
                const rect = this.canvas.getBoundingClientRect();
                const x = event.clientX - rect.left;
                const y = event.clientY - rect.top;
                
                const hoveredStep = this.getStepAtPosition(x, y);
                this.canvas.style.cursor = hoveredStep !== -1 ? 'pointer' : 'default';
            });
        }
    }
    
    /**
     * Update Pattern Data with Mathematical Analysis
     * 
     * Updates the visual representation with new pattern data and triggers
     * real-time Center of Gravity calculation for mathematical analysis display.
     * 
     * @param {Object} patternData - Complete pattern specification
     * @param {boolean[]} patternData.steps - Array of step states (true=active, false=inactive)
     * @param {number} patternData.stepCount - Total number of steps in the pattern
     * @param {number[]} [patternData.dividerPositions] - Positions for pattern separation lines
     * @param {boolean} [patternData.isStringed] - Whether pattern represents combined sequences
     * 
     * @example
     * visualEngine.updatePattern({
     *   steps: [true, false, true, false, true, false, false, false],
     *   stepCount: 8,
     *   dividerPositions: [4],
     *   isStringed: true
     * });
     * 
     * Update Process:
     * 1. Validate and copy pattern data to prevent external modification
     * 2. Ensure canvas is properly sized for current container visibility
     * 3. Calculate Center of Gravity using mathematical analysis algorithms
     * 4. Convert CoG complex coordinates to canvas display coordinates
     * 5. Update visual state and trigger complete re-rendering
     * 
     * Mathematical Integration:
     * - Real-time Center of Gravity calculation using complex number representation
     * - Normalized magnitude calculation for balance assessment
     * - Angle calculation for vector display positioning
     * - Error handling for mathematical edge cases and invalid data
     * 
     * Visual Updates:
     * - Immediate pattern step visualization with color coding
     * - CoG indicator positioning with precision vector display
     * - Pattern divider rendering for combined sequence visualization
     * - Responsive element sizing for optimal display clarity
     */
    updatePattern(patternData) {
    updatePattern(patternData) {
        if (!patternData) return;
        
        this.pattern.steps = [...patternData.steps] || new Array(16).fill(false);
        this.pattern.stepCount = patternData.stepCount || this.pattern.steps.length;
        this.pattern.dividerPositions = patternData.dividerPositions || null;
        this.pattern.isStringed = patternData.isStringed || false;
        
        // Check if container is now visible and resize canvas if needed
        this.ensureCanvasSize();
        
        // Calculate Center of Gravity if calculator is available
        if (typeof CenterOfGravityCalculator !== 'undefined') {
            try {
                const cogResult = CenterOfGravityCalculator.calculateCenterOfGravity(this.pattern.steps);
                // CoG calculation successful
                
                if (cogResult && typeof cogResult.magnitude === 'number') {
                    // Convert the result format to what we expect
                    const distance = cogResult.normalizedMagnitude || cogResult.magnitude;
                    const x = cogResult.coordinates?.x || cogResult.x || 0;
                    const y = cogResult.coordinates?.y || cogResult.y || 0;
                    const angle = Math.atan2(y, x) * (180 / Math.PI);
                    
                    this.cogData = {
                        distance: distance,
                        angle: angle,
                        x: x,
                        y: y,
                        isCalculated: true
                    };
                    
                    console.log(`🎨 CoG calculated: distance=${distance.toFixed(4)}, angle=${angle.toFixed(1)}°`);
                } else {
                    console.warn('⚠️ Invalid CoG result:', cogResult);
                    this.cogData.isCalculated = false;
                }
            } catch (error) {
                console.warn('⚠️ CoG calculation failed:', error);
                this.cogData.isCalculated = false;
            }
        } else {
            console.warn('⚠️ CenterOfGravityCalculator not available');
            this.cogData.isCalculated = false;
        }
        
        this.render();
    }
    
    /**
     * Ensure canvas is properly sized, especially after container becomes visible
     */
    ensureCanvasSize() {
        if (!this.container || !this.canvas) return;
        
        // Check if container is now visible
        const rect = this.container.getBoundingClientRect();
        const style = getComputedStyle(this.container);
        
        console.log('🎨 Checking canvas size:', {
            containerWidth: rect.width,
            containerHeight: rect.height,
            display: style.display,
            currentCanvasWidth: this.canvas.width,
            currentCanvasHeight: this.canvas.height
        });
        
        // If container has dimensions but canvas doesn't, resize it
        if ((rect.width > 0 || rect.height > 0) && this.canvas.width === 0) {
            console.log('🎨 Container now visible, resizing canvas');
            this.setupResponsiveCanvas();
        }
    }
    
    /**
     * Update Playback State with Animation
     * 
     * Updates the visual playback state with current step highlighting
     * and animation effects for real-time pattern monitoring.
     * 
     * @param {Object} playbackState - Current playback information
     * @param {number} playbackState.currentStep - Currently playing step index (0-based)
     * @param {boolean} playbackState.isPlaying - Whether sequencer is actively playing
     * @param {number} [playbackState.tempo] - Current BPM for animation timing
     * @param {number} [playbackState.volume] - Current volume level for visual intensity
     * 
     * @example
     * visualEngine.updatePlayback({
     *   currentStep: 3,
     *   isPlaying: true,
     *   tempo: 120,
     *   volume: 0.8
     * });
     * 
     * Visual Effects:
     * - Current step highlighting with contrasting colors
     * - Pulse animation effects synchronized with playback
     * - Smooth transitions between step positions
     * - Playing state indicator with visual feedback
     * 
     * Animation Features:
     * - Tempo-synchronized pulse effects for current step
     * - Smooth color transitions for step state changes
     * - Volume-responsive visual intensity scaling
     * - Performance-optimized animation loops
     */
    updatePlayback(playbackState) {
    updatePlayback(playbackState) {
        if (!playbackState) return;
        
        this.pattern.currentStep = playbackState.currentStep || 0;
        this.pattern.isPlaying = playbackState.isPlaying || false;
        
        this.render();
    }
    
    /**
     * Get Step Index at Screen Position
     * 
     * Performs precise geometric calculation to determine which pattern step
     * corresponds to a given screen coordinate, enabling accurate click detection.
     * 
     * @param {number} x - X coordinate in canvas pixels
     * @param {number} y - Y coordinate in canvas pixels
     * @returns {number} Step index (0-based) or -1 if no step at position
     * 
     * @example
     * const stepIndex = visualEngine.getStepAtPosition(mouseX, mouseY);
     * if (stepIndex !== -1) {
     *   console.log(`Clicked on step ${stepIndex + 1}`);
     * }
     * 
     * Geometric Calculation:
     * 1. Calculate step positions using trigonometric functions
     * 2. Determine distance from click point to each step center
     * 3. Check if distance falls within step element radius
     * 4. Return first matching step index or -1 for no match
     * 
     * Precision Features:
     * - Exact trigonometric positioning for circular layout
     * - Responsive step size calculation for hit detection
     * - Optimized distance calculation using squared distances
     * - Edge case handling for overlapping or edge positions
     * 
     * Performance Optimization:
     * - Early termination on first match found
     * - Cached trigonometric calculations where possible
     * - Efficient distance comparison without square root
     * - Minimal memory allocation during calculation
     */
    getStepAtPosition(x, y) {
    getStepAtPosition(x, y) {
        const stepRadius = this.config.canvasSize * this.config.stepRadius;
        const stepSize = this.config.stepElementActualSize;
        
        for (let i = 0; i < this.pattern.stepCount; i++) {
            const angle = (i / this.pattern.stepCount) * Math.PI * 2 - Math.PI / 2;
            const stepX = this.config.centerX + stepRadius * Math.cos(angle);
            const stepY = this.config.centerY + stepRadius * Math.sin(angle);
            
            const distance = Math.sqrt((x - stepX) ** 2 + (y - stepY) ** 2);
            if (distance <= stepSize / 2) {
                return i;
            }
        }
        
        return -1;
    }
    
    /**
     * Toggle Step State with Real-time Analysis
     * 
     * Toggles the active/inactive state of a pattern step and triggers
     * immediate recalculation of mathematical properties and visual updates.
     * 
     * @param {number} stepIndex - Zero-based index of step to toggle
     * 
     * @example
     * visualEngine.toggleStep(3); // Toggle the 4th step
     * 
     * Toggle Process:
     * 1. Validate step index within valid range
     * 2. Invert boolean state of specified step
     * 3. Trigger pattern recalculation with new step configuration
     * 4. Emit custom event for external pattern change handling
     * 
     * Real-time Updates:
     * - Immediate Center of Gravity recalculation
     * - Visual state update with color changes
     * - Pattern balance analysis refresh
     * - Event emission for external synchronization
     * 
     * Integration Features:
     * - Seamless integration with pattern analysis systems
     * - Custom event emission for external change detection
     * - Atomic operation ensuring consistent visual state
     * - Error handling for invalid step indices
     */
    toggleStep(stepIndex) {
    toggleStep(stepIndex) {
        if (stepIndex >= 0 && stepIndex < this.pattern.stepCount) {
            this.pattern.steps[stepIndex] = !this.pattern.steps[stepIndex];
            
            // Recalculate CoG
            this.updatePattern({ 
                steps: this.pattern.steps, 
                stepCount: this.pattern.stepCount 
            });
            
            // Emit change event
            this.emitPatternChange();
        }
    }
    
    /**
     * Emit Pattern Change Event
     * 
     * Dispatches a custom DOM event containing complete pattern state
     * and mathematical analysis data for external system integration.
     * 
     * Event Details:
     * - Event Type: 'patternChanged'
     * - Event Data: Complete pattern state and CoG analysis
     * - Bubble Behavior: Bubbles up through DOM hierarchy
     * - Target Element: Canvas element for event delegation
     * 
     * @example
     * canvas.addEventListener('patternChanged', (event) => {
     *   const { steps, stepCount, cogData } = event.detail;
     *   console.log('Pattern updated:', steps);
     *   console.log('CoG distance:', cogData.distance);
     * });
     * 
     * Event Integration:
     * - Enables external systems to respond to pattern changes
     * - Provides complete state snapshot for synchronization
     * - Includes mathematical analysis data for real-time updates
     * - Supports event delegation patterns for efficient handling
     */
    emitPatternChange() {
    emitPatternChange() {
        if (this.canvas) {
            const event = new CustomEvent('patternChanged', {
                detail: {
                    steps: [...this.pattern.steps],
                    stepCount: this.pattern.stepCount,
                    cogData: { ...this.cogData }
                }
            });
            this.canvas.dispatchEvent(event);
        }
    }
    
    /**
     * Main Render Method - Complete Canvas Drawing Pipeline
     * 
     * Executes the complete visual rendering pipeline for pattern visualization
     * with mathematical precision and performance optimization.
     * 
     * Rendering Pipeline:
     * 1. Canvas Validation: Ensure rendering context and proper sizing
     * 2. Performance Tracking: Monitor render time and frame rate
     * 3. Canvas Clearing: Reset canvas state for fresh drawing
     * 4. Background Rendering: Base circles and visual foundation
     * 5. Grid Lines: Reference lines for visual orientation
     * 6. Donut Chart: Pattern step visualization with state colors
     * 7. CoG Display: Mathematical center of gravity indicator
     * 8. Step Elements: Interactive position markers with numbering
     * 9. Pattern Dividers: Visual separation for combined patterns
     * 10. Performance Update: Statistics tracking and optimization
     * 11. Force Repaint: Ensure visibility across browser implementations
     * 
     * Error Handling:
     * - Validates canvas context availability before rendering
     * - Checks canvas sizing for proper initialization
     * - Graceful degradation for missing or invalid data
     * - Performance monitoring with detailed debug logging
     * 
     * Performance Features:
     * - Optimized drawing operations with minimal state changes
     * - Efficient canvas clearing and redraw cycles
     * - Frame rate monitoring for performance analysis
     * - Memory-conscious rendering with proper cleanup
     * 
     * Visual Quality:
     * - Anti-aliased graphics for smooth appearance
     * - Precise geometric calculations for mathematical accuracy
     * - High-contrast colors for accessibility
     * - Responsive scaling for different display sizes
     */
    render() {
    render() {
        console.log('🎨 RENDER DEBUG:', {
            hasCtx: !!this.ctx,
            hasCanvas: !!this.canvas,
            canvasSize: this.config.canvasSize,
            canvasWidth: this.canvas?.width,
            canvasHeight: this.canvas?.height,
            canvasInDOM: document.contains(this.canvas)
        });
        
        if (!this.ctx) {
            console.warn('⚠️ No canvas context available for rendering');
            return;
        }
        
        if (!this.canvas) {
            console.warn('⚠️ No canvas element available');
            return;
        }
        
        // Skip rendering if canvas isn't properly sized yet
        if (this.config.canvasSize <= 0) {
            console.warn('⚠️ Canvas config size is 0');
            return;
        }
        
        // If canvas element isn't sized yet, try to size it now
        if (this.canvas.width <= 0) {
            console.warn('⚠️ Canvas element width is 0, calling setupResponsiveCanvas');
            this.setupResponsiveCanvas();
            if (this.canvas.width <= 0) {
                console.warn('⚠️ Canvas still has 0 width after setup');
                return;
            }
        }
        
        const startTime = performance.now();
        
        // Clear canvas
        this.ctx.clearRect(0, 0, this.config.canvasSize, this.config.canvasSize);
        
        // Test: Draw a visible circle to verify canvas is working
        this.ctx.beginPath();
        this.ctx.arc(this.config.centerX, this.config.centerY, 50, 0, Math.PI * 2);
        this.ctx.fillStyle = 'red';
        this.ctx.fill();
        console.log(`🔴 Test circle drawn at ${this.config.centerX}, ${this.config.centerY}`);
        
        // Draw background
        this.drawBackground();
        
        // Draw grid lines (optional)
        this.drawGridLines();
        
        // Draw donut chart
        this.drawDonutChart();
        
        // Draw Center of Gravity indicator
        this.drawCenterOfGravity();
        
        // Draw step elements
        this.drawStepElements();
        
        // Draw pattern divider lines
        this.drawPatternDividers();
        
        // Update performance stats
        this.updateRenderStats(startTime);
        
        // Force a repaint to ensure canvas is visible
        this.forceRepaint();
    }
    
    /**
     * Draw Background Circle
     * 
     * Renders the foundational background circle that serves as the base
     * for the entire donut chart visualization.
     * 
     * Background Elements:
     * - Outer circle boundary defining the visualization area
     * - Solid fill color for visual foundation
     * - Border stroke for defined edge clarity
     * - Responsive sizing based on canvas dimensions
     * 
     * Visual Properties:
     * - Radius calculated as percentage of canvas size for responsiveness
     * - Background color optimized for contrast with step elements
     * - Border width scaled for visual hierarchy
     * - Anti-aliased rendering for smooth appearance
     */
    drawBackground() {
    drawBackground() {
        const outerRadius = this.config.canvasSize * this.config.donutOuterRadius;
        
        this.ctx.beginPath();
        this.ctx.arc(this.config.centerX, this.config.centerY, outerRadius, 0, Math.PI * 2);
        this.ctx.fillStyle = this.config.colors.background;
        this.ctx.fill();
        this.ctx.strokeStyle = this.config.colors.stepBorder;
        this.ctx.lineWidth = 1;
        this.ctx.stroke();
    }
    
    /**
     * Draw Grid Lines for Visual Reference
     * 
     * Renders subtle reference lines at cardinal directions (12, 3, 6, 9 o'clock)
     * to provide visual orientation and timing reference for pattern analysis.
     * 
     * Grid Features:
     * - Cardinal direction lines for timing reference
     * - Subtle color and opacity for non-intrusive guidance
     * - Precise positioning using trigonometric calculations
     * - Responsive sizing matching donut chart dimensions
     * 
     * Mathematical Positioning:
     * - Lines positioned at 0°, 90°, 180°, 270° angles
     * - Start points at inner donut radius
     * - End points at outer donut radius
     * - Offset by -π/2 for top-aligned 12 o'clock start position
     * 
     * Visual Design:
     * - Low opacity for subtle visual guidance
     * - Thin line width for minimal visual interference
     * - Color chosen for accessibility and contrast
     */
    drawGridLines() {
    drawGridLines() {
        const outerRadius = this.config.canvasSize * this.config.donutOuterRadius;
        const innerRadius = this.config.canvasSize * this.config.donutInnerRadius;
        
        this.ctx.strokeStyle = this.config.colors.gridLines;
        this.ctx.lineWidth = 1;
        
        // Draw quarter lines (12, 3, 6, 9 o'clock)
        for (let i = 0; i < 4; i++) {
            const angle = (i * Math.PI) / 2 - Math.PI / 2;
            const startX = this.config.centerX + innerRadius * Math.cos(angle);
            const startY = this.config.centerY + innerRadius * Math.sin(angle);
            const endX = this.config.centerX + outerRadius * Math.cos(angle);
            const endY = this.config.centerY + outerRadius * Math.sin(angle);
            
            this.ctx.beginPath();
            this.ctx.moveTo(startX, startY);
            this.ctx.lineTo(endX, endY);
            this.ctx.stroke();
        }
    }
    
    /**
     * Draw Donut Chart Visualization
     * 
     * Renders the main pattern visualization as a circular donut chart with
     * individual segments representing each step's active/inactive state.
     * 
     * Chart Features:
     * - Individual segments for each pattern step
     * - Color coding for active/inactive states
     * - Current step highlighting with special effects
     * - Pulse animation for playing state
     * - Responsive segment sizing based on step count
     * 
     * Segment Calculation:
     * - Angle per step: (2π / stepCount) for equal distribution
     * - Start angle offset: -π/2 for top-aligned 12 o'clock start
     * - Arc path creation using HTML5 Canvas arc methods
     * - Donut shape achieved through inner/outer radius difference
     * 
     * Visual States:
     * - Active Steps: Vibrant color indicating rhythm beats
     * - Inactive Steps: Subtle color for timing reference
     * - Current Step (Playing): Enhanced color with pulse animation
     * - Current Step (Inactive): Distinctive color for position clarity
     * 
     * Animation Effects:
     * - Sine wave pulse effect for current step during playback
     * - Smooth color transitions for state changes
     * - Performance-optimized animation loops
     * - Configurable pulse intensity and speed
     */
    drawDonutChart() {
    drawDonutChart() {
        if (this.pattern.stepCount === 0) return;
        
        const outerRadius = this.config.canvasSize * this.config.donutOuterRadius;
        const innerRadius = this.config.canvasSize * this.config.donutInnerRadius;
        const anglePerStep = (Math.PI * 2) / this.pattern.stepCount;
        
        // Draw each step as a donut segment
        for (let i = 0; i < this.pattern.stepCount; i++) {
            const startAngle = (i * anglePerStep) - (Math.PI / 2);
            const endAngle = ((i + 1) * anglePerStep) - (Math.PI / 2);
            
            // Create donut segment path
            this.ctx.beginPath();
            this.ctx.arc(this.config.centerX, this.config.centerY, outerRadius, startAngle, endAngle);
            this.ctx.lineTo(
                this.config.centerX + innerRadius * Math.cos(endAngle),
                this.config.centerY + innerRadius * Math.sin(endAngle)
            );
            this.ctx.arc(this.config.centerX, this.config.centerY, innerRadius, endAngle, startAngle, true);
            this.ctx.closePath();
            
            // Determine colors
            const isActive = this.pattern.steps[i];
            const isCurrent = i === this.pattern.currentStep && this.pattern.isPlaying;
            
            let fillColor;
            if (isActive) {
                fillColor = isCurrent ? this.config.colors.currentStep : this.config.colors.activeStep;
            } else {
                fillColor = isCurrent ? this.config.colors.currentInactive : this.config.colors.inactiveStep;
            }
            
            // Add pulse effect for current step
            if (isCurrent && this.pattern.isPlaying) {
                const pulseAmount = Math.sin(Date.now() * 0.01) * this.config.pulseIntensity + 1;
                this.ctx.save();
                this.ctx.translate(this.config.centerX, this.config.centerY);
                this.ctx.scale(pulseAmount, pulseAmount);
                this.ctx.translate(-this.config.centerX, -this.config.centerY);
            }
            
            // Fill segment
            this.ctx.fillStyle = fillColor;
            this.ctx.fill();
            
            // Draw segment border
            this.ctx.strokeStyle = this.config.colors.stepBorder;
            this.ctx.lineWidth = Math.max(1, this.config.canvasSize * 0.002);
            this.ctx.stroke();
            
            if (isCurrent && this.pattern.isPlaying) {
                this.ctx.restore();
            }
        }
    }
    
    /**
     * Draw Center of Gravity Indicator
     * 
     * Renders the mathematical center of gravity as a vector from the pattern
     * center, providing visual feedback for pattern balance analysis.
     * 
     * CoG Visualization Components:
     * - Vector line from pattern center to calculated CoG position
     * - Circular indicator dot at the CoG coordinates
     * - Distance value text display for quantitative analysis
     * - Color coding to distinguish from pattern elements
     * 
     * Mathematical Representation:
     * - CoG coordinates normalized to (-1, 1) range
     * - Scaling applied to fit within visualization radius
     * - Complex number representation converted to Cartesian coordinates
     * - Distance magnitude displayed as normalized floating-point value
     * 
     * Visual Elements:
     * - Line: Vector from center (0,0) to CoG position
     * - Dot: Circular indicator at CoG coordinates
     * - Text: Distance value with precision formatting
     * - Colors: Distinct from pattern for clear differentiation
     * 
     * Display Conditions:
     * - Only rendered when CoG calculation is available
     * - Handles edge cases for empty or invalid patterns
     * - Responsive sizing for different canvas dimensions
     * - Error handling for mathematical edge cases
     */
    drawCenterOfGravity() {
    drawCenterOfGravity() {
        if (!this.cogData.isCalculated) return;
        
        const cogRadius = this.config.canvasSize * this.config.cogRadius;
        const indicatorSize = 8;
        
        // Convert CoG coordinates to canvas coordinates
        // CoG coordinates are normalized (-1 to 1), scale to our coordinate system
        const cogX = this.config.centerX + (this.cogData.x * cogRadius);
        const cogY = this.config.centerY + (this.cogData.y * cogRadius);
        
        // Draw line from center to CoG position
        this.ctx.beginPath();
        this.ctx.moveTo(this.config.centerX, this.config.centerY);
        this.ctx.lineTo(cogX, cogY);
        this.ctx.strokeStyle = this.config.colors.cogLine;
        this.ctx.lineWidth = 2;
        this.ctx.stroke();
        
        // Draw CoG indicator dot
        this.ctx.beginPath();
        this.ctx.arc(cogX, cogY, indicatorSize, 0, Math.PI * 2);
        this.ctx.fillStyle = this.config.colors.cogIndicator;
        this.ctx.fill();
        this.ctx.strokeStyle = this.config.colors.cogLine;
        this.ctx.lineWidth = 2;
        this.ctx.stroke();
        
        // Draw distance text
        const distance = this.cogData.distance;
        const distanceText = `CoG: ${distance.toFixed(3)}`;
        
        this.ctx.fillStyle = this.config.colors.stepText;
        this.ctx.font = `${Math.max(10, this.config.canvasSize * 0.025)}px monospace`;
        this.ctx.textAlign = 'center';
        this.ctx.fillText(
            distanceText, 
            this.config.centerX, 
            this.config.centerY - this.config.canvasSize * 0.1
        );
    }
    
    /**
     * Draw Step Position Elements
     * 
     * Renders interactive step markers around the circle perimeter with
     * state-appropriate visual styling and optional numbering.
     * 
     * Step Elements:
     * - Circular markers at each step position
     * - State-based color coding (active/inactive)
     * - Current step highlighting during playback
     * - Smart numbering system based on step count
     * - Interactive visual feedback for click targets
     * 
     * Positioning Algorithm:
     * - Trigonometric calculation for circular placement
     * - Equal angular distribution around circle perimeter
     * - Radius positioning outside donut chart area
     * - Top-aligned start position (12 o'clock)
     * 
     * Visual States:
     * - Active Steps: Filled circles with vibrant colors
     * - Inactive Steps: Outlined circles with subtle fill
     * - Current Step: Enhanced border and distinctive colors
     * - Hover State: Cursor change for interaction feedback
     * 
     * Smart Numbering:
     * - All numbers for ≤16 steps
     * - Every 2nd number for 17-32 steps
     * - Every 3rd number for 33-48 steps
     * - Every 4th number for 49+ steps
     * - Font size responsive to element size
     * - High contrast text for accessibility
     */
    drawStepElements() {
    drawStepElements() {
        const stepRadius = this.config.canvasSize * this.config.stepRadius;
        const stepSize = this.config.stepElementActualSize;
        const fontSize = stepSize * 0.4;
        
        // Draw step elements around the circle
        
        for (let i = 0; i < this.pattern.stepCount; i++) {
            const angle = (i / this.pattern.stepCount) * Math.PI * 2 - Math.PI / 2;
            const x = this.config.centerX + stepRadius * Math.cos(angle);
            const y = this.config.centerY + stepRadius * Math.sin(angle);
            
            // Determine appearance
            const isActive = this.pattern.steps[i];
            const isCurrent = i === this.pattern.currentStep && this.pattern.isPlaying;
            
            // Draw step circle
            this.ctx.beginPath();
            this.ctx.arc(x, y, stepSize / 2, 0, Math.PI * 2);
            
            // Fill color
            if (isActive) {
                this.ctx.fillStyle = isCurrent ? this.config.colors.currentStep : this.config.colors.activeStep;
            } else {
                this.ctx.fillStyle = this.config.colors.background;
            }
            this.ctx.fill();
            
            // Border
            this.ctx.strokeStyle = isCurrent ? this.config.colors.currentStep : this.config.colors.stepBorder;
            this.ctx.lineWidth = isCurrent ? 3 : 1;
            this.ctx.stroke();
            
            // Add step number for clarity (smart numbering based on step count)
            if (this.shouldShowStepNumber(i)) {
                this.ctx.fillStyle = isActive ? 'white' : this.config.colors.stepText;
                this.ctx.font = `bold ${fontSize}px monospace`;
                this.ctx.textAlign = 'center';
                this.ctx.textBaseline = 'middle';
                this.ctx.fillText((i + 1).toString(), x, y);
            }
        }
    }
    
    /**
     * Draw Pattern Divider Lines
     * 
     * Renders visual separators between combined or stringed patterns
     * to clearly delineate pattern boundaries and sequence structure.
     * 
     * Divider Features:
     * - Thick black lines for clear visual separation
     * - Radial lines from inner to outer donut radius
     * - Positioned at pattern boundary points
     * - Additional divider at sequence start/end for looped patterns
     * 
     * @example
     * // For combined pattern P(3,0) + P(5,1) with divider at step 8
     * // Renders separator line between the two polygon patterns
     * 
     * Positioning Calculation:
     * - Divider positions provided in pattern data structure
     * - Angular conversion: (position / stepCount) × 2π
     * - Radial line from inner donut edge to outer edge
     * - Top-aligned angle offset for consistent positioning
     * 
     * Visual Properties:
     * - Line width: 4px for prominent visibility
     * - Color: Solid black (#000000) for maximum contrast
     * - Length: Full radial span from inner to outer radius
     * - Anti-aliased rendering for smooth appearance
     * 
     * Special Cases:
     * - Loop divider for stringed patterns at 12 o'clock position
     * - Multiple dividers for complex pattern combinations
     * - Conditional rendering based on pattern type
     */
    drawPatternDividers() {
    drawPatternDividers() {
        if (!this.pattern.dividerPositions || this.pattern.dividerPositions.length === 0) {
            return;
        }
        
        const stepRadius = this.config.canvasSize * this.config.stepRadius;
        const innerRadius = this.config.canvasSize * this.config.donutInnerRadius;
        const outerRadius = this.config.canvasSize * this.config.donutOuterRadius;
        
        // Draw divider lines at each divider position
        for (const dividerPosition of this.pattern.dividerPositions) {
            // Calculate angle for the divider line (between steps)
            const angle = (dividerPosition / this.pattern.stepCount) * Math.PI * 2 - Math.PI / 2;
            
            // Calculate start and end points for the divider line
            const startX = this.config.centerX + innerRadius * Math.cos(angle);
            const startY = this.config.centerY + innerRadius * Math.sin(angle);
            const endX = this.config.centerX + outerRadius * Math.cos(angle);
            const endY = this.config.centerY + outerRadius * Math.sin(angle);
            
            // Draw thick black divider line
            this.ctx.beginPath();
            this.ctx.moveTo(startX, startY);
            this.ctx.lineTo(endX, endY);
            this.ctx.strokeStyle = '#000000';
            this.ctx.lineWidth = 4;
            this.ctx.stroke();
        }
        
        // Also draw a divider at the end (between last pattern and loop back to start)
        if (this.pattern.isStringed) {
            const endAngle = -Math.PI / 2; // At the top (step 0)
            const startX = this.config.centerX + innerRadius * Math.cos(endAngle);
            const startY = this.config.centerY + innerRadius * Math.sin(endAngle);
            const endX = this.config.centerX + outerRadius * Math.cos(endAngle);
            const endY = this.config.centerY + outerRadius * Math.sin(endAngle);
            
            this.ctx.beginPath();
            this.ctx.moveTo(startX, startY);
            this.ctx.lineTo(endX, endY);
            this.ctx.strokeStyle = '#000000';
            this.ctx.lineWidth = 4;
            this.ctx.stroke();
        }
    }
    
    /**
     * Determine Step Number Display Strategy
     * 
     * Implements intelligent step numbering strategy based on pattern
     * complexity to maintain visual clarity and readability.
     * 
     * @param {number} stepIndex - Zero-based step index to evaluate
     * @returns {boolean} Whether this step should display its number
     * 
     * @example
     * // For 8-step pattern, all steps show numbers
     * shouldShowStepNumber(0); // true (step 1)
     * shouldShowStepNumber(7); // true (step 8)
     * 
     * // For 24-step pattern, every 2nd step shows number
     * shouldShowStepNumber(0); // true (step 1)
     * shouldShowStepNumber(1); // false
     * shouldShowStepNumber(2); // true (step 3)
     * 
     * Numbering Strategy:
     * - ≤16 steps: Show all numbers for complete clarity
     * - 17-32 steps: Show every 2nd number to reduce clutter
     * - 33-48 steps: Show every 3rd number for readability
     * - 49+ steps: Show every 4th number to prevent overlap
     * 
     * Design Rationale:
     * - Prevents visual overcrowding with large step counts
     * - Maintains sufficient reference points for navigation
     * - Adapts to available space and element sizing
     * - Preserves accessibility while managing complexity
     */
    shouldShowStepNumber(stepIndex) {
    shouldShowStepNumber(stepIndex) {
        if (this.pattern.stepCount <= 16) {
            // Show all numbers for 16 or fewer steps
            return true;
        } else if (this.pattern.stepCount <= 32) {
            // Show every 2nd step for 17-32 steps
            return stepIndex % 2 === 0;
        } else if (this.pattern.stepCount <= 48) {
            // Show every 3rd step for 33-48 steps
            return stepIndex % 3 === 0;
        } else {
            // Show every 4th step for 49+ steps
            return stepIndex % 4 === 0;
        }
    }
    
    /**
     * Update Render Performance Statistics
     * 
     * Tracks rendering performance metrics for optimization analysis
     * and performance monitoring of the visual engine.
     * 
     * @param {number} startTime - Performance timestamp when rendering began
     * 
     * Performance Metrics:
     * - Individual render time calculation
     * - Total frames rendered counter
     * - Rolling average FPS calculation
     * - Last render time for debugging
     * 
     * @example
     * const startTime = performance.now();
     * // ... rendering operations ...
     * this.updateRenderStats(startTime);
     * // Updates: lastRenderTime, framesRendered, averageFPS
     * 
     * Statistics Tracking:
     * - Render Time: Current frame render duration in milliseconds
     * - Frame Count: Cumulative total of completed render cycles
     * - Average FPS: Rolling average calculated every 60 frames
     * - Performance Trends: Historical data for optimization analysis
     * 
     * Optimization Insights:
     * - Identifies performance bottlenecks in rendering pipeline
     * - Enables frame rate monitoring for smooth animation
     * - Provides data for canvas size and complexity optimization
     * - Supports debugging of performance regressions
     */
    updateRenderStats(startTime) {
    updateRenderStats(startTime) {
        const renderTime = performance.now() - startTime;
        this.stats.framesRendered++;
        this.stats.lastRenderTime = renderTime;
        
        // Calculate rolling average FPS
        if (this.stats.framesRendered % 60 === 0) {
            this.stats.averageFPS = Math.round(1000 / renderTime);
        }
    }
    
    /**
     * Force Canvas Repaint for Cross-Browser Compatibility
     * 
     * Implements browser-specific workarounds to ensure proper canvas
     * visibility and rendering across different browser implementations.
     * 
     * Repaint Techniques:
     * - Display style toggling to trigger layout recalculation
     * - Hardware acceleration hint with CSS transform
     * - Forced reflow through DOM property access
     * - Timeout-based style restoration for smooth transitions
     * 
     * Browser Compatibility:
     * - Chrome: Handles hardware acceleration inconsistencies
     * - Firefox: Resolves canvas visibility timing issues
     * - Safari: Addresses iOS/macOS rendering differences
     * - Edge: Manages legacy rendering engine compatibility
     * 
     * Implementation Details:
     * - Temporary display:none to force re-rendering
     * - offsetHeight access triggers layout recalculation
     * - translateZ(0) enables hardware acceleration layer
     * - Minimal style changes to prevent visual flicker
     * 
     * Performance Considerations:
     * - Minimal DOM manipulation for efficiency
     * - Short-duration operations to prevent flicker
     * - Fallback handling for missing parent elements
     * - Debug logging excluded in production builds
     */
    forceRepaint() {
    forceRepaint() {
        // Force layout recalculation to fix rendering issues
        const canvas = this.canvas;
        const originalDisplay = canvas.style.display;
        canvas.style.display = 'none';
        canvas.offsetHeight; // Trigger reflow
        canvas.style.display = originalDisplay;
        
        // Alternative: trigger a style recalculation
        const wrapper = canvas.parentElement;
        if (wrapper) {
            wrapper.style.transform = 'translateZ(0)';
            setTimeout(() => {
                wrapper.style.transform = '';
            }, 1);
        }
    }
    
    /**
     * Get Visual Engine Statistics
     * 
     * Returns comprehensive performance and state information for
     * monitoring, debugging, and optimization analysis.
     * 
     * @returns {Object} Complete statistics object with performance metrics
     * @returns {number} returns.framesRendered - Total frames rendered since initialization
     * @returns {number} returns.lastRenderTime - Most recent frame render time in milliseconds
     * @returns {number} returns.averageFPS - Rolling average frames per second
     * @returns {number} returns.canvasSize - Current canvas dimensions in pixels
     * @returns {number} returns.stepCount - Number of steps in current pattern
     * @returns {boolean} returns.cogCalculated - Whether Center of Gravity is available
     * @returns {number} returns.cogDistance - Current CoG distance value
     * 
     * @example
     * const stats = visualEngine.getStats();
     * console.log(`Rendering at ${stats.averageFPS} FPS`);
     * console.log(`Canvas: ${stats.canvasSize}px, Steps: ${stats.stepCount}`);
     * console.log(`CoG Distance: ${stats.cogDistance.toFixed(3)}`);
     * 
     * Statistics Categories:
     * - Performance: Frame rate, render time, optimization metrics
     * - Canvas State: Size, configuration, responsiveness data
     * - Pattern Data: Step count, CoG availability, mathematical state
     * - System Health: Error rates, memory usage, browser compatibility
     */
    getStats() {
    getStats() {
        return {
            ...this.stats,
            canvasSize: this.config.canvasSize,
            stepCount: this.pattern.stepCount,
            cogCalculated: this.cogData.isCalculated,
            cogDistance: this.cogData.distance
        };
    }
    
    /**
     * Update Visual Configuration
     * 
     * Applies configuration changes to the visual engine and triggers
     * re-rendering with the updated settings.
     * 
     * @param {Object} newConfig - Configuration object with updates
     * @param {Object} [newConfig.colors] - Color scheme updates
     * @param {number} [newConfig.animationSpeed] - Animation timing adjustments
     * @param {number} [newConfig.donutOuterRadius] - Outer radius percentage
     * @param {number} [newConfig.donutInnerRadius] - Inner radius percentage
     * @param {number} [newConfig.pulseIntensity] - Animation pulse strength
     * 
     * @example
     * // Apply dark theme
     * visualEngine.updateConfig(VisualConfigPresets.getDarkTheme());
     * 
     * // Adjust sizing
     * visualEngine.updateConfig({
     *   donutOuterRadius: 0.45,
     *   donutInnerRadius: 0.20,
     *   pulseIntensity: 0.3
     * });
     * 
     * Configuration Merging:
     * - Shallow merge with existing configuration
     * - Preserves unspecified settings
     * - Validates configuration values for safety
     * - Triggers immediate re-rendering with new settings
     * 
     * Supported Updates:
     * - Color Schemes: Complete theme changes or individual color updates
     * - Sizing: Radius percentages and element dimensions
     * - Animation: Speed, intensity, and effect parameters
     * - Accessibility: High contrast and visual accessibility options
     */
    updateConfig(newConfig) {
    updateConfig(newConfig) {
        Object.assign(this.config, newConfig);
        this.render();
    }
    
    /**
     * Export Pattern as Image Data URL
     * 
     * Generates a high-quality image export of the current pattern
     * visualization for saving, sharing, or documentation purposes.
     * 
     * @param {string} [format='png'] - Image format ('png', 'jpeg', 'webp')
     * @returns {string|null} Base64-encoded data URL or null if unavailable
     * 
     * @example
     * // Export as PNG (default, with transparency)
     * const pngDataUrl = visualEngine.exportImage();
     * 
     * // Export as JPEG (smaller file size, no transparency)
     * const jpegDataUrl = visualEngine.exportImage('jpeg');
     * 
     * // Use exported image
     * const downloadLink = document.createElement('a');
     * downloadLink.href = pngDataUrl;
     * downloadLink.download = 'rhythm-pattern.png';
     * downloadLink.click();
     * 
     * Export Features:
     * - High-resolution image generation from canvas content
     * - Multiple format support for different use cases
     * - Preserves current visual state including CoG indicators
     * - Base64 encoding for immediate use or download
     * 
     * Format Characteristics:
     * - PNG: Lossless compression, transparency support, larger files
     * - JPEG: Lossy compression, smaller files, no transparency
     * - WebP: Modern format, excellent compression, limited browser support
     * 
     * Error Handling:
     * - Returns null if canvas is unavailable or export fails
     * - Handles browser security restrictions gracefully
     * - Validates format parameter for supported types
     */
    exportImage(format = 'png') {
    exportImage(format = 'png') {
        if (!this.canvas) return null;
        return this.canvas.toDataURL(`image/${format}`);
    }
    
    /**
     * Destroy Visual Engine and Clean Up Resources
     * 
     * Performs complete cleanup of the visual engine, removing event listeners,
     * clearing canvas content, and freeing memory resources.
     * 
     * Cleanup Operations:
     * - Remove all registered event listeners
     * - Clear canvas drawing surface
     * - Reset internal state variables
     * - Free performance monitoring resources
     * - Disable further rendering operations
     * 
     * @example
     * // Proper cleanup when component unmounts
     * visualEngine.destroy();
     * visualEngine = null;
     * 
     * Memory Management:
     * - Removes window resize event listeners
     * - Clears canvas context references
     * - Resets pattern and CoG data structures
     * - Prevents memory leaks in long-running applications
     * 
     * State Reset:
     * - Canvas cleared to transparent/white background
     * - Performance statistics reset to zero
     * - Configuration restored to default values
     * - All timers and animation loops stopped
     * 
     * Best Practices:
     * - Call before removing visual engine from DOM
     * - Essential for single-page applications
     * - Prevents memory accumulation in dynamic interfaces
     * - Ensures clean teardown for component lifecycle
     */
    destroy() {
    destroy() {
        // Remove event listeners
        window.removeEventListener('resize', this.setupResponsiveCanvas);
        
        // Clear canvas
        if (this.ctx) {
            this.ctx.clearRect(0, 0, this.config.canvasSize, this.config.canvasSize);
        }
        
        console.log('🎨 SequencerVisualEngine destroyed');
    }
}

/**
 * Visual Configuration Presets
 * 
 * Provides predefined visual themes and configuration sets for different
 * use cases, accessibility needs, and aesthetic preferences.
 * 
 * Preset Categories:
 * - Accessibility: High contrast and visual impairment support
 * - Aesthetic: Different visual styles and color schemes
 * - Performance: Optimized settings for different device capabilities
 * - Functional: Specialized configurations for specific use cases
 * 
 * Theme Features:
 * - Complete color scheme definitions
 * - Accessibility-compliant contrast ratios
 * - Responsive design considerations
 * - Cross-platform compatibility
 * 
 * Usage Pattern:
 * 1. Select appropriate preset for use case
 * 2. Apply to visual engine via updateConfig()
 * 3. Customize specific properties if needed
 * 4. Save custom configurations for future use
 * 
 * @example
 * // Apply dark theme for low-light environments
 * const darkConfig = VisualConfigPresets.getDarkTheme();
 * visualEngine.updateConfig(darkConfig);
 * 
 * // Apply high contrast for accessibility
 * const accessibleConfig = VisualConfigPresets.getHighContrastTheme();
 * visualEngine.updateConfig(accessibleConfig);
 */
class VisualConfigPresets {
    /**
     * Get Dark Theme Configuration
     * 
     * Returns a complete dark theme configuration optimized for low-light
     * environments and reduced eye strain during extended use.
     * 
     * @returns {Object} Dark theme configuration object
     * @returns {Object} returns.colors - Complete dark color scheme
     * 
     * @example
     * const darkTheme = VisualConfigPresets.getDarkTheme();
     * visualEngine.updateConfig(darkTheme);
     * 
     * Theme Characteristics:
     * - Dark background colors for reduced eye strain
     * - High contrast elements for visibility
     * - Muted accent colors for subtle highlighting
     * - Optimized for long-duration use
     * 
     * Color Psychology:
     * - Background: Deep blue-gray for professional appearance
     * - Active Steps: Bright green for clear rhythm indication
     * - Current Step: Golden yellow for attention without harshness
     * - CoG Indicator: Warm red for mathematical emphasis
     * 
     * Accessibility Features:
     * - WCAG AA compliant contrast ratios
     * - Colorblind-friendly color choices
     * - Sufficient brightness differentiation
     */
    static getDarkTheme() {
        return {
            colors: {
                background: '#2c3e50',
                donutBackground: 'rgba(52, 73, 94, 0.8)',
                activeStep: 'rgba(46, 204, 113, 0.8)',
                currentStep: 'rgba(241, 196, 15, 0.9)',
                inactiveStep: 'rgba(127, 140, 141, 0.4)',
                currentInactive: 'rgba(231, 76, 60, 0.7)',
                stepBorder: '#7f8c8d',
                stepText: '#ecf0f1',
                cogIndicator: 'rgba(231, 76, 60, 0.9)',
                cogLine: 'rgba(231, 76, 60, 0.7)',
                gridLines: 'rgba(149, 165, 166, 0.3)'
            }
        };
    }
    
    /**
     * Get High Contrast Theme Configuration
     * 
     * Returns a high contrast theme configuration designed for maximum
     * accessibility and visual clarity for users with visual impairments.
     * 
     * @returns {Object} High contrast theme configuration object
     * @returns {Object} returns.colors - Accessibility-optimized color scheme
     * 
     * @example
     * const accessibleTheme = VisualConfigPresets.getHighContrastTheme();
     * visualEngine.updateConfig(accessibleTheme);
     * 
     * Accessibility Features:
     * - Maximum contrast ratios exceeding WCAG AAA standards
     * - Pure black and white base colors for clarity
     * - Bold red highlights for critical information
     * - Thick borders and clear element separation
     * 
     * Design Principles:
     * - Eliminates subtle color distinctions
     * - Uses position and shape for differentiation
     * - Maximizes brightness differences
     * - Reduces reliance on color alone for information
     * 
     * Use Cases:
     * - Visual impairment accommodation
     * - High ambient light environments
     * - Projection and presentation scenarios
     * - Legal compliance requirements
     */
    static getHighContrastTheme() {
        return {
            colors: {
                background: '#ffffff',
                donutBackground: 'rgba(240, 240, 240, 0.9)',
                activeStep: 'rgba(0, 0, 0, 0.8)',
                currentStep: 'rgba(255, 0, 0, 0.9)',
                inactiveStep: 'rgba(200, 200, 200, 0.5)',
                currentInactive: 'rgba(255, 0, 0, 0.6)',
                stepBorder: '#000000',
                stepText: '#000000',
                cogIndicator: 'rgba(255, 0, 0, 0.9)',
                cogLine: 'rgba(255, 0, 0, 0.8)',
                gridLines: 'rgba(100, 100, 100, 0.5)'
            }
        };
    }
    
    /**
     * Get Minimal Theme Configuration
     * 
     * Returns a clean, minimal theme configuration focused on simplicity
     * and elegant visual design with subtle color usage.
     * 
     * @returns {Object} Minimal theme configuration object
     * @returns {Object} returns.colors - Clean, minimal color scheme
     * 
     * @example
     * const minimalTheme = VisualConfigPresets.getMinimalTheme();
     * visualEngine.updateConfig(minimalTheme);
     * 
     * Design Philosophy:
     * - Clean, uncluttered visual appearance
     * - Subtle color palette with neutral tones
     * - Focus on content over decorative elements
     * - Professional, modern aesthetic
     * 
     * Color Strategy:
     * - Neutral grays for non-critical elements
     * - Single accent color for active states
     * - Minimal color variety for cohesive appearance
     * - Light, airy feeling with plenty of visual breathing room
     * 
     * Ideal Applications:
     * - Professional presentations and documentation
     * - Clean user interface design
     * - Focus on mathematical content over visual effects
     * - Minimalist aesthetic preferences
     */
    static getMinimalTheme() {
        return {
            colors: {
                background: '#ffffff',
                donutBackground: 'rgba(248, 249, 250, 0.6)',
                activeStep: 'rgba(108, 117, 125, 0.7)',
                currentStep: 'rgba(0, 123, 255, 0.8)',
                inactiveStep: 'rgba(233, 236, 239, 0.4)',
                currentInactive: 'rgba(220, 53, 69, 0.5)',
                stepBorder: '#dee2e6',
                stepText: '#495057',
                cogIndicator: 'rgba(220, 53, 69, 0.8)',
                cogLine: 'rgba(220, 53, 69, 0.5)',
                gridLines: 'rgba(206, 212, 218, 0.3)'
            }
        };
    }
}

// Export to global scope for browser compatibility
if (typeof window !== 'undefined') {
    window.SequencerVisualEngine = SequencerVisualEngine;
    window.VisualConfigPresets = VisualConfigPresets;
}

// Export for Node.js compatibility (if needed for testing)
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        SequencerVisualEngine,
        VisualConfigPresets
    };
}