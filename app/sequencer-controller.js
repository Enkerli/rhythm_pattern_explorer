/**
 * Sequencer Controller Module (sequencer-controller.js)
 * Central coordinator for audio and visual sequencer engines
 * 
 * Features:
 * - SequencerController class - Main coordination controller
 * - Audio and visual engine management
 * - Playback control and synchronization
 * - Pattern loading and updating
 * - Settings management (tempo, volume, audio parameters)
 * - Event system for main application integration
 * - Auto-sync with pattern analysis workflow
 * - Performance monitoring and optimization
 * 
 * Dependencies:
 * - SequencerAudioEngine (from sequencer-audio.js)
 * - SequencerVisualEngine (from sequencer-visual.js)
 */

/**
 * Main Sequencer Controller
 * Coordinates audio and visual engines for rhythm pattern playback
 */
class SequencerController {
    constructor(canvasId, containerId, options = {}) {
        // Configuration
        this.config = {
            canvasId,
            containerId,
            autoPlay: options.autoPlay || false,
            defaultTempo: options.defaultTempo || 120,
            defaultVolume: options.defaultVolume || 0.5,
            defaultWaveform: options.defaultWaveform || 'sine',
            stepDivision: options.stepDivision || 4, // 16th notes (4 per beat)
            maxPatternLength: options.maxPatternLength || 64,
            enableAudio: options.enableAudio !== false,
            enableVisual: options.enableVisual !== false
        };
        
        // Playback state
        this.state = {
            isPlaying: false,
            isPaused: false,
            currentStep: 0,
            tempo: this.config.defaultTempo,
            volume: this.config.defaultVolume,
            waveform: this.config.defaultWaveform,
            pattern: {
                steps: new Array(16).fill(false),
                stepCount: 16,
                name: 'Empty Pattern'
            },
            playbackInterval: null,
            stepDuration: this.calculateStepDuration(this.config.defaultTempo)
        };
        
        // Engines
        this.audioEngine = null;
        this.visualEngine = null;
        
        // Event callbacks
        this.callbacks = {
            onPatternChange: [],
            onPlaybackChange: [],
            onStepChange: [],
            onError: [],
            onSettingsChange: []
        };
        
        // Performance tracking
        this.stats = {
            stepsPlayed: 0,
            patternsLoaded: 0,
            playbackSessions: 0,
            lastStepTime: 0,
            averageStepTiming: 0,
            errors: 0
        };
        
        // Initialize engines
        this.initialize();
        
        console.log('🎼 SequencerController initialized');
    }
    
    /**
     * Initialize audio and visual engines
     */
    async initialize() {
        try {
            // Initialize visual engine if enabled
            if (this.config.enableVisual) {
                this.visualEngine = new SequencerVisualEngine(this.config.canvasId, this.config.containerId);
                this.setupVisualEventListeners();
                console.log('✅ Visual engine initialized');
            }
            
            // Initialize audio engine if enabled
            if (this.config.enableAudio) {
                this.audioEngine = new SequencerAudioEngine();
                await this.audioEngine.initialize();
                this.audioEngine.updateSettings({
                    volume: this.state.volume,
                    waveform: this.state.waveform
                });
                console.log('✅ Audio engine initialized');
            }
            
            // Load default pattern
            this.updatePattern(this.state.pattern);
            
            return true;
            
        } catch (error) {
            console.error('❌ Failed to initialize sequencer:', error);
            this.handleError('initialization', error);
            return false;
        }
    }
    
    /**
     * Setup visual engine event listeners
     */
    setupVisualEventListeners() {
        if (!this.visualEngine?.canvas) return;
        
        // Listen for pattern changes from visual interaction
        this.visualEngine.canvas.addEventListener('patternChanged', (event) => {
            const { steps, stepCount, cogData } = event.detail;
            
            // Update internal state
            this.state.pattern.steps = [...steps];
            this.state.pattern.stepCount = stepCount;
            
            // Emit pattern change event
            this.emitEvent('onPatternChange', {
                pattern: { ...this.state.pattern },
                cogData,
                source: 'visual'
            });
            
            console.log('🎵 Pattern updated from visual interaction');
        });
    }
    
    /**
     * Calculate step duration based on tempo
     * @param {number} tempo - BPM
     * @returns {number} Duration in milliseconds
     */
    calculateStepDuration(tempo) {
        // Calculate duration for one step based on tempo and step division
        // 60000ms per minute / tempo BPM / stepDivision steps per beat
        return 60000 / tempo / this.config.stepDivision;
    }
    
    /**
     * Load a new pattern
     * @param {object} patternData - Pattern with steps, stepCount, metadata
     */
    updatePattern(patternData) {
        if (!patternData) return;
        
        try {
            // Validate pattern
            const validatedPattern = this.validatePattern(patternData);
            
            // Update state
            this.state.pattern = { ...validatedPattern };
            this.state.currentStep = 0;
            
            // Update engines
            if (this.visualEngine) {
                this.visualEngine.updatePattern(validatedPattern);
            }
            
            // Reset playback position
            this.updatePlaybackState();
            
            // Update stats
            this.stats.patternsLoaded++;
            
            // Emit pattern change event
            this.emitEvent('onPatternChange', {
                pattern: validatedPattern,
                source: 'controller'
            });
            
            console.log(`🎵 Pattern loaded: ${validatedPattern.name || 'Unnamed'} (${validatedPattern.stepCount} steps)`);
            
        } catch (error) {
            console.error('❌ Failed to update pattern:', error);
            this.handleError('pattern_update', error);
        }
    }
    
    /**
     * Validate and sanitize pattern data
     * @param {object} patternData - Raw pattern data
     * @returns {object} Validated pattern
     */
    validatePattern(patternData) {
        const pattern = {
            steps: Array.isArray(patternData.steps) ? [...patternData.steps] : new Array(16).fill(false),
            stepCount: patternData.stepCount || patternData.steps?.length || 16,
            name: patternData.name || 'Untitled Pattern'
        };
        
        // Validate step count
        if (pattern.stepCount < 1) pattern.stepCount = 1;
        if (pattern.stepCount > this.config.maxPatternLength) {
            pattern.stepCount = this.config.maxPatternLength;
        }
        
        // Ensure steps array matches step count
        if (pattern.steps.length !== pattern.stepCount) {
            if (pattern.steps.length < pattern.stepCount) {
                // Pad with false
                pattern.steps = pattern.steps.concat(new Array(pattern.stepCount - pattern.steps.length).fill(false));
            } else {
                // Truncate
                pattern.steps = pattern.steps.slice(0, pattern.stepCount);
            }
        }
        
        // Copy additional metadata if present
        if (patternData.isEuclidean) pattern.isEuclidean = patternData.isEuclidean;
        if (patternData.isRegularPolygon) pattern.isRegularPolygon = patternData.isRegularPolygon;
        if (patternData.formula) pattern.formula = patternData.formula;
        if (patternData.binary) pattern.binary = patternData.binary;
        if (patternData.cogData) pattern.cogData = patternData.cogData;
        if (patternData.dividerPositions) pattern.dividerPositions = patternData.dividerPositions;
        if (patternData.isStringed) pattern.isStringed = patternData.isStringed;
        
        return pattern;
    }
    
    /**
     * Start playback
     */
    async play() {
        if (this.state.isPlaying) return;
        
        try {
            // Initialize audio context if needed
            if (this.audioEngine) {
                await this.audioEngine.resume();
            }
            
            // Update state
            this.state.isPlaying = true;
            this.state.isPaused = false;
            
            // Start playback loop
            this.startPlaybackLoop();
            
            // Update stats
            this.stats.playbackSessions++;
            
            // Update visual state
            this.updatePlaybackState();
            
            // Emit playback change event
            this.emitEvent('onPlaybackChange', {
                isPlaying: true,
                isPaused: false,
                currentStep: this.state.currentStep,
                tempo: this.state.tempo
            });
            
            console.log(`▶️ Playback started at ${this.state.tempo} BPM`);
            
        } catch (error) {
            console.error('❌ Failed to start playback:', error);
            this.handleError('playback_start', error);
        }
    }
    
    /**
     * Pause playback
     */
    pause() {
        if (!this.state.isPlaying) return;
        
        this.state.isPlaying = false;
        this.state.isPaused = true;
        
        this.stopPlaybackLoop();
        this.updatePlaybackState();
        
        this.emitEvent('onPlaybackChange', {
            isPlaying: false,
            isPaused: true,
            currentStep: this.state.currentStep,
            tempo: this.state.tempo
        });
        
        console.log('⏸️ Playback paused');
    }
    
    /**
     * Stop playback and reset position
     */
    stop() {
        this.state.isPlaying = false;
        this.state.isPaused = false;
        this.state.currentStep = 0;
        
        this.stopPlaybackLoop();
        this.updatePlaybackState();
        
        this.emitEvent('onPlaybackChange', {
            isPlaying: false,
            isPaused: false,
            currentStep: 0,
            tempo: this.state.tempo
        });
        
        console.log('⏹️ Playback stopped');
    }
    
    /**
     * Toggle play/pause
     */
    togglePlayback() {
        if (this.state.isPlaying) {
            this.pause();
        } else {
            this.play();
        }
    }
    
    /**
     * Start the playback timing loop
     */
    startPlaybackLoop() {
        this.stopPlaybackLoop(); // Ensure no existing loop
        
        const playStep = () => {
            if (!this.state.isPlaying) return;
            
            const stepTime = performance.now();
            
            // Play audio for current step
            if (this.state.pattern.steps[this.state.currentStep] && this.audioEngine) {
                this.audioEngine.playSound();
            }
            
            // Update visual state
            this.updatePlaybackState();
            
            // Emit step change event
            this.emitEvent('onStepChange', {
                currentStep: this.state.currentStep,
                isActive: this.state.pattern.steps[this.state.currentStep],
                stepTime
            });
            
            // Update stats
            this.stats.stepsPlayed++;
            this.updateStepTiming(stepTime);
            
            // Advance to next step
            this.state.currentStep = (this.state.currentStep + 1) % this.state.pattern.stepCount;
        };
        
        // Start the timing loop
        this.state.playbackInterval = setInterval(playStep, this.state.stepDuration);
        
        // Play first step immediately
        playStep();
    }
    
    /**
     * Stop the playback timing loop
     */
    stopPlaybackLoop() {
        if (this.state.playbackInterval) {
            clearInterval(this.state.playbackInterval);
            this.state.playbackInterval = null;
        }
    }
    
    /**
     * Update playback state in visual engine
     */
    updatePlaybackState() {
        if (this.visualEngine) {
            this.visualEngine.updatePlayback({
                currentStep: this.state.currentStep,
                isPlaying: this.state.isPlaying
            });
        }
    }
    
    /**
     * Update step timing statistics
     * @param {number} stepTime - Current step timestamp
     */
    updateStepTiming(stepTime) {
        if (this.stats.lastStepTime > 0) {
            const actualInterval = stepTime - this.stats.lastStepTime;
            
            // Calculate rolling average of step timing
            if (this.stats.averageStepTiming === 0) {
                this.stats.averageStepTiming = actualInterval;
            } else {
                this.stats.averageStepTiming = (this.stats.averageStepTiming * 0.9) + (actualInterval * 0.1);
            }
        }
        
        this.stats.lastStepTime = stepTime;
    }
    
    /**
     * Update tempo
     * @param {number} newTempo - New tempo in BPM
     */
    setTempo(newTempo) {
        if (typeof newTempo !== 'number' || newTempo < 30 || newTempo > 300) {
            console.warn('⚠️ Invalid tempo:', newTempo);
            return;
        }
        
        const oldTempo = this.state.tempo;
        this.state.tempo = newTempo;
        this.state.stepDuration = this.calculateStepDuration(newTempo);
        
        // Restart playback loop if playing to apply new timing
        if (this.state.isPlaying) {
            this.startPlaybackLoop();
        }
        
        this.emitEvent('onSettingsChange', {
            setting: 'tempo',
            oldValue: oldTempo,
            newValue: newTempo
        });
        
        console.log(`🎵 Tempo changed: ${oldTempo} → ${newTempo} BPM`);
    }
    
    /**
     * Update audio settings
     * @param {object} settings - Audio settings to update
     */
    updateAudioSettings(settings) {
        if (!this.audioEngine) return;
        
        const oldSettings = { ...this.state };
        
        // Update internal state
        if (typeof settings.volume === 'number') {
            this.state.volume = Math.max(0, Math.min(1, settings.volume));
        }
        if (typeof settings.waveform === 'string') {
            this.state.waveform = settings.waveform;
        }
        
        // Update audio engine
        this.audioEngine.updateSettings({
            volume: this.state.volume,
            waveform: this.state.waveform,
            ...settings
        });
        
        this.emitEvent('onSettingsChange', {
            setting: 'audio',
            oldValue: oldSettings,
            newValue: this.state
        });
        
        console.log('🔊 Audio settings updated');
    }
    
    /**
     * Get current sequencer state
     * @returns {object} Current state
     */
    getState() {
        return {
            isPlaying: this.state.isPlaying,
            isPaused: this.state.isPaused,
            currentStep: this.state.currentStep,
            tempo: this.state.tempo,
            volume: this.state.volume,
            waveform: this.state.waveform,
            pattern: { ...this.state.pattern },
            stepDuration: this.state.stepDuration
        };
    }
    
    /**
     * Get sequencer statistics
     * @returns {object} Performance statistics
     */
    getStats() {
        const audioStats = this.audioEngine ? this.audioEngine.getStats() : {};
        const visualStats = this.visualEngine ? this.visualEngine.getStats() : {};
        
        return {
            controller: { ...this.stats },
            audio: audioStats,
            visual: visualStats,
            timing: {
                expectedStepDuration: this.state.stepDuration,
                actualAverageStepDuration: this.stats.averageStepTiming,
                timingAccuracy: this.stats.averageStepTiming > 0 ? 
                    (this.state.stepDuration / this.stats.averageStepTiming) : 1
            }
        };
    }
    
    /**
     * Register event callback
     * @param {string} eventType - Event type
     * @param {function} callback - Callback function
     */
    on(eventType, callback) {
        if (this.callbacks[eventType] && typeof callback === 'function') {
            this.callbacks[eventType].push(callback);
        }
    }
    
    /**
     * Unregister event callback
     * @param {string} eventType - Event type
     * @param {function} callback - Callback function to remove
     */
    off(eventType, callback) {
        if (this.callbacks[eventType]) {
            this.callbacks[eventType] = this.callbacks[eventType].filter(cb => cb !== callback);
        }
    }
    
    /**
     * Emit event to registered callbacks
     * @param {string} eventType - Event type
     * @param {object} data - Event data
     */
    emitEvent(eventType, data) {
        if (this.callbacks[eventType]) {
            this.callbacks[eventType].forEach(callback => {
                try {
                    callback(data);
                } catch (error) {
                    console.error(`❌ Error in ${eventType} callback:`, error);
                }
            });
        }
    }
    
    /**
     * Handle errors
     * @param {string} context - Error context
     * @param {Error} error - Error object
     */
    handleError(context, error) {
        this.stats.errors++;
        
        const errorData = {
            context,
            message: error.message,
            timestamp: new Date().toISOString(),
            state: this.getState()
        };
        
        this.emitEvent('onError', errorData);
        
        // Stop playback if critical error
        if (['initialization', 'audio_critical'].includes(context)) {
            this.stop();
        }
    }
    
    /**
     * Export current pattern as data
     * @returns {object} Pattern data for export
     */
    exportPattern() {
        return {
            ...this.state.pattern,
            metadata: {
                tempo: this.state.tempo,
                volume: this.state.volume,
                waveform: this.state.waveform,
                exportedAt: new Date().toISOString()
            }
        };
    }
    
    /**
     * Import pattern from data
     * @param {object} patternData - Pattern data to import
     */
    importPattern(patternData) {
        try {
            this.updatePattern(patternData);
            
            // Apply metadata if present
            if (patternData.metadata) {
                if (patternData.metadata.tempo) {
                    this.setTempo(patternData.metadata.tempo);
                }
                if (patternData.metadata.volume !== undefined) {
                    this.updateAudioSettings({ volume: patternData.metadata.volume });
                }
                if (patternData.metadata.waveform) {
                    this.updateAudioSettings({ waveform: patternData.metadata.waveform });
                }
            }
            
            console.log('📥 Pattern imported successfully');
            
        } catch (error) {
            console.error('❌ Failed to import pattern:', error);
            this.handleError('pattern_import', error);
        }
    }
    
    /**
     * Destroy the sequencer and clean up resources
     */
    destroy() {
        // Stop playback
        this.stop();
        
        // Destroy engines
        if (this.audioEngine) {
            this.audioEngine.destroy();
        }
        if (this.visualEngine) {
            this.visualEngine.destroy();
        }
        
        // Clear callbacks
        Object.keys(this.callbacks).forEach(key => {
            this.callbacks[key] = [];
        });
        
        console.log('🎼 SequencerController destroyed');
    }
}

/**
 * Sequencer Integration Helper
 * Utility functions for integrating with main application
 */
class SequencerIntegration {
    /**
     * Create sequencer from pattern analysis result
     * @param {object} analysisResult - Pattern analysis result from main app
     * @param {string} canvasId - Canvas element ID
     * @param {string} containerId - Container element ID
     * @param {object} options - Additional options
     * @returns {SequencerController} Configured sequencer
     */
    static createFromAnalysis(analysisResult, canvasId, containerId, options = {}) {
        const sequencer = new SequencerController(canvasId, containerId, options);
        
        // Extract pattern data from analysis result
        let patternData;
        
        if (analysisResult.type === 'single') {
            patternData = analysisResult.pattern;
        } else if (analysisResult.type === 'combination') {
            patternData = analysisResult.combined;
        } else if (analysisResult.type === 'stringed') {
            patternData = analysisResult.pattern;
        } else if (analysisResult.type === 'quantized') {
            patternData = analysisResult.pattern;
        } else {
            patternData = analysisResult;
        }
        
        // Add metadata from analysis
        if (patternData) {
            patternData.name = patternData.name || 
                             patternData.formula || 
                             patternData.description || 
                             'Analyzed Pattern';
        }
        
        // Load the pattern
        sequencer.updatePattern(patternData);
        
        return sequencer;
    }
    
    /**
     * Convert pattern result to sequencer-friendly format
     * @param {object} patternResult - Pattern from UnifiedPatternParser
     * @returns {object} Sequencer-compatible pattern
     */
    static convertPatternResult(patternResult) {
        const pattern = {
            steps: [...patternResult.steps],
            stepCount: patternResult.stepCount,
            name: patternResult.name || patternResult.formula || 'Pattern'
        };
        
        // Copy metadata
        const metadataFields = [
            'isEuclidean', 'isRegularPolygon', 'formula', 'binary', 
            'vertices', 'offset', 'beats', 'expansion', 'polygonType',
            'dividerPositions', 'isStringed', 'isQuantized', 
            'quantizationDirection', 'originalStepCount', 'originalPattern', 
            'quantizationRatio'
        ];
        
        metadataFields.forEach(field => {
            if (patternResult[field] !== undefined) {
                pattern[field] = patternResult[field];
            }
        });
        
        return pattern;
    }
}

// Export to global scope for browser compatibility
if (typeof window !== 'undefined') {
    window.SequencerController = SequencerController;
    window.SequencerIntegration = SequencerIntegration;
}

// Export for Node.js compatibility (if needed for testing)
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        SequencerController,
        SequencerIntegration
    };
}