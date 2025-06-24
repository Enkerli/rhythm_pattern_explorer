/**
 * Sequencer Visual Module (sequencer-visual.js)
 * Circular donut visualization with Center of Gravity integration
 * 
 * Features:
 * - SequencerVisualEngine class - Main visual controller
 * - Canvas-based circular donut chart rendering
 * - CoG distance visualization integration
 * - Step position indicators and numbering
 * - Playback animation with current step highlighting
 * - Responsive design and automatic sizing
 * - Pattern density and beat visualization
 * 
 * Dependencies: 
 * - CenterOfGravityCalculator (from pattern-analysis.js)
 * - MathUtils (from math-core.js)
 */

/**
 * Visual Engine for the Circular Sequencer
 * Handles all canvas-based rendering and visual effects
 */
class SequencerVisualEngine {
    constructor(canvasId, containerId) {
        this.canvas = document.getElementById(canvasId);
        this.container = document.getElementById(containerId);
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
     * Setup responsive canvas sizing
     */
    setupResponsiveCanvas() {
        if (!this.container || !this.canvas) return;
        
        // Get container dimensions
        const rect = this.container.getBoundingClientRect();
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
        
        console.log(`🎨 Canvas sized: ${size}x${size}px`);
        
        // Trigger re-render now that canvas is properly sized
        if (size > 0) {
                setTimeout(() => this.render(), 50);
        }
    }
    
    /**
     * Setup event listeners for responsive behavior
     */
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
     * Update pattern data
     * @param {object} patternData - Pattern with steps, stepCount, etc.
     */
    updatePattern(patternData) {
        if (!patternData) return;
        
        this.pattern.steps = [...patternData.steps] || new Array(16).fill(false);
        this.pattern.stepCount = patternData.stepCount || this.pattern.steps.length;
        
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
     * Update playback state
     * @param {object} playbackState - Current step, playing status, etc.
     */
    updatePlayback(playbackState) {
        if (!playbackState) return;
        
        this.pattern.currentStep = playbackState.currentStep || 0;
        this.pattern.isPlaying = playbackState.isPlaying || false;
        
        this.render();
    }
    
    /**
     * Get step index at screen position
     * @param {number} x - X coordinate
     * @param {number} y - Y coordinate
     * @returns {number} Step index or -1 if not found
     */
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
     * Toggle step state
     * @param {number} stepIndex - Step to toggle
     */
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
     * Emit pattern change event
     */
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
     * Main render method
     */
    render() {
        if (!this.ctx) {
            console.warn('⚠️ No canvas context available for rendering');
            return;
        }
        
        // Skip rendering if canvas isn't properly sized yet
        if (this.config.canvasSize <= 0) {
            return;
        }
        
        // If canvas element isn't sized yet, try to size it now
        if (this.canvas.width <= 0) {
            this.setupResponsiveCanvas();
            if (this.canvas.width <= 0) {
                return;
            }
        }
        
        const startTime = performance.now();
        
        // Clear canvas
        this.ctx.clearRect(0, 0, this.config.canvasSize, this.config.canvasSize);
        
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
        
        // Update performance stats
        this.updateRenderStats(startTime);
    }
    
    /**
     * Draw background circle
     */
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
     * Draw grid lines for visual reference
     */
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
     * Draw donut chart visualization
     */
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
     * Draw Center of Gravity indicator
     */
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
     * Draw step position elements
     */
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
     * Determine if step number should be shown
     * @param {number} stepIndex - Step index
     * @returns {boolean} Whether to show number
     */
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
     * Update render performance statistics
     * @param {number} startTime - Render start time
     */
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
     * Get visual engine statistics
     */
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
     * Update visual configuration
     * @param {object} newConfig - Configuration updates
     */
    updateConfig(newConfig) {
        Object.assign(this.config, newConfig);
        this.render();
    }
    
    /**
     * Export pattern as image data URL
     * @param {string} format - Image format ('png', 'jpeg')
     * @returns {string} Data URL
     */
    exportImage(format = 'png') {
        if (!this.canvas) return null;
        return this.canvas.toDataURL(`image/${format}`);
    }
    
    /**
     * Destroy the visual engine and clean up
     */
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
 * Predefined visual themes and configurations
 */
class VisualConfigPresets {
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