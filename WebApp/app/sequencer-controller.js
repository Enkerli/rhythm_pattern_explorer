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
 * Sequencer Controller - Advanced rhythm pattern playback system
 * 
 * Provides high-precision audio and visual sequencer capabilities for rhythm
 * pattern playback with perfect synchronization. Integrates Web Audio API
 * for sub-millisecond timing accuracy with real-time visual feedback.
 * 
 * Core Features:
 * - Precision timing using Web Audio API with lookahead scheduling
 * - Perfect audio-visual synchronization
 * - Real-time tempo and volume control
 * - Multiple waveform support (sine, square, triangle, sawtooth)
 * - Circular pattern visualization with center of gravity display
 * - Pattern loading from any source (parsed, database, generated)
 * - Comprehensive playback state management
 * - Performance monitoring and optimization
 * 
 * Technical Architecture:
 * - Audio Engine: Web Audio API with precise scheduling
 * - Visual Engine: Canvas-based circular sequencer display
 * - Controller Layer: Coordinates engines and manages state
 * - Integration Layer: Connects with main application workflow
 * 
 * Timing System:
 * Uses Web Audio's currentTime for scheduling with lookahead buffering
 * to eliminate JavaScript timing jitter. Audio events are scheduled
 * precisely in the audio thread while visual updates sync to the
 * audio timeline for perfect coordination.
 * 
 * Musical Applications:
 * - Pattern composition and experimentation
 * - Rhythm analysis and comparison
 * - Live performance with mathematical patterns
 * - Educational demonstration of algorithmic rhythms
 */
class SequencerController {
    /**
     * Initialize the Sequencer Controller
     * 
     * @param {string} canvasId - ID of canvas element for visual display
     * @param {string} containerId - ID of container element for UI controls
     * @param {Object} options - Configuration options
     * @param {boolean} options.autoPlay - Auto-start playback when pattern loaded
     * @param {number} options.defaultTempo - Initial tempo in BPM (default: 120)
     * @param {number} options.defaultVolume - Initial volume 0-1 (default: 0.5)
     * @param {string} options.defaultWaveform - Initial waveform type (default: 'sine')
     * @param {number} options.stepDivision - Steps per beat (default: 4 for 16th notes)
     * @param {number} options.maxPatternLength - Maximum pattern length (default: 64)
     * @param {boolean} options.enableAudio - Enable audio engine (default: true)
     * @param {boolean} options.enableVisual - Enable visual engine (default: true)
     * 
     * Initialization Process:
     * 1. Configure sequencer parameters and options
     * 2. Initialize playback state variables
     * 3. Create audio and visual engine instances
     * 4. Set up timing and synchronization systems
     * 5. Configure default pattern and settings
     * 6. Establish event system for main app integration
     * 
     * Performance Considerations:
     * - Audio context initialization requires user gesture
     * - Visual updates optimized for 60fps
     * - Memory efficient pattern storage
     * - Graceful degradation if audio unavailable
     */
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
            enableVisual: options.enableVisual !== false,
            enableMIDI: false, // Start disabled but always create controller
            midiChannel: options.midiChannel || 1,
            midiNote: options.midiNote || 36, // C2 kick drum
            midiVelocity: options.midiVelocity || 100
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
                if (typeof SequencerVisualEngine !== 'undefined') {
                    this.visualEngine = new SequencerVisualEngine(this.config.canvasId, this.config.containerId);
                    this.setupVisualEventListeners();
                    console.log('✅ Visual engine initialized');
                } else {
                    console.warn('⚠️ SequencerVisualEngine not available, visual disabled');
                    this.config.enableVisual = false;
                }
            }
            
            // Initialize audio engine if enabled
            if (this.config.enableAudio) {
                try {
                    this.audioEngine = new SequencerAudioEngine();
                    await this.audioEngine.initialize();
                    this.audioEngine.updateSettings({
                        volume: this.state.volume,
                        waveform: this.state.waveform
                    });
                    console.log('✅ Audio engine initialized');
                } catch (error) {
                    console.warn('⚠️ Audio engine initialization failed (will initialize on user interaction):', error.message);
                    // Audio will be initialized later on user interaction
                }
            }
            
            // Always create MIDI output controller, but don't auto-connect
            // This ensures the controller exists even when MIDI is disabled
            console.log('🔍 DEBUG: Checking for MIDIOutputController...', typeof MIDIOutputController);
            if (typeof MIDIOutputController !== 'undefined') {
                console.log('🔍 DEBUG: Creating MIDI output controller...');
                this.midiOutput = new MIDIOutputController({
                    channel: this.config.midiChannel,
                    baseNote: this.config.midiNote,
                    velocity: this.config.midiVelocity,
                    autoConnect: false
                });
                
                // Set up event handlers
                this.midiOutput.onReady = (info) => {
                    console.log(`✅ MIDI output ready via ${info.method}`);
                    if (info.outputs.length > 0) {
                        console.log('🎹 Available MIDI outputs:', info.outputs.map(o => o.name));
                    }
                };
                
                this.midiOutput.onError = (error) => {
                    console.warn('⚠️ MIDI output error:', error.message);
                };
                
                console.log('🎹 MIDI output controller created (not initialized) - v2.0');
            } else {
                console.warn('⚠️ MIDIOutputController not available, MIDI disabled');
                console.log('🔍 DEBUG: MIDIOutputController type:', typeof MIDIOutputController);
                this.config.enableMIDI = false;
                this.midiOutput = null;
            }
            
            console.log('🔍 DEBUG: Final midiOutput:', this.midiOutput);
            
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
            
            // console.log(`🎵 Pattern loaded: ${validatedPattern.name || 'Unnamed'} (${validatedPattern.stepCount} steps)`);
            
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
     * Start the playback timing loop with precise Web Audio scheduling
     */
    startPlaybackLoop() {
        this.stopPlaybackLoop(); // Ensure no existing loop
        
        if (!this.audioEngine || !this.audioEngine.context) {
            console.warn('Audio context not available for precise timing');
            this.startFallbackPlaybackLoop();
            return;
        }
        
        // Initialize precise timing variables
        this.schedulerState = {
            nextStepTime: this.audioEngine.context.currentTime,
            stepDurationSeconds: this.state.stepDuration / 1000,
            lookaheadTime: 0.1, // 100ms lookahead
            scheduleAheadTime: 0.025, // 25ms schedule precision
            currentVisualStep: this.state.currentStep,
            lastScheduledStep: -1
        };
        
        // Start the scheduler with precise timing
        this.startPreciseScheduler();
        
        // Start visual update loop separately
        this.startVisualUpdateLoop();
    }
    
    /**
     * Precise Web Audio API scheduler
     */
    startPreciseScheduler() {
        const scheduler = () => {
            if (!this.state.isPlaying) return;
            
            // Schedule audio events ahead of time
            while (this.schedulerState.nextStepTime < 
                   this.audioEngine.context.currentTime + this.schedulerState.lookaheadTime) {
                
                this.scheduleStep(this.schedulerState.nextStepTime);
                this.schedulerState.nextStepTime += this.schedulerState.stepDurationSeconds;
            }
            
            // Continue scheduling
            this.state.schedulerTimeout = setTimeout(scheduler, this.schedulerState.scheduleAheadTime * 1000);
        };
        
        scheduler();
    }
    
    /**
     * Schedule a single step with precise timing
     */
    scheduleStep(audioTime) {
        const stepIndex = (this.schedulerState.lastScheduledStep + 1) % this.state.pattern.stepCount;
        this.schedulerState.lastScheduledStep = stepIndex;
        
        // Schedule audio if step is active
        if (this.state.pattern.steps[stepIndex]) {
            if (this.audioEngine) {
                this.audioEngine.playSoundAtTime(audioTime);
            }
            
            // Schedule MIDI note if step is active
            if (this.midiOutput && this.midiOutput.isReady) {
                // For real-time MIDI, send immediately (MIDI timing is handled by the OS)
                this.midiOutput.noteOn();
                
                // Schedule note off after a short duration (100ms)
                setTimeout(() => {
                    if (this.midiOutput && this.midiOutput.isReady) {
                        this.midiOutput.noteOff();
                    }
                }, 100);
            }
        }
        
        // Schedule visual update to happen at the right time
        const visualDelay = Math.max(0, (audioTime - this.audioEngine.context.currentTime) * 1000);
        setTimeout(() => {
            if (this.state.isPlaying) {
                this.updateVisualStep(stepIndex, audioTime);
            }
        }, visualDelay);
    }
    
    /**
     * Visual update loop using requestAnimationFrame
     */
    startVisualUpdateLoop() {
        const updateVisuals = () => {
            if (!this.state.isPlaying) return;
            
            // Update visual rendering
            this.updatePlaybackState();
            
            // Continue visual updates
            this.state.visualUpdateId = requestAnimationFrame(updateVisuals);
        };
        
        updateVisuals();
    }
    
    /**
     * Update visual step position with timing sync
     */
    updateVisualStep(stepIndex, audioTime) {
        this.schedulerState.currentVisualStep = stepIndex;
        this.state.currentStep = stepIndex;
        
        // Emit step change event with precise timing
        this.emitEvent('onStepChange', {
            currentStep: stepIndex,
            isActive: this.state.pattern.steps[stepIndex],
            audioTime: audioTime,
            stepTime: performance.now()
        });
        
        // Update stats
        this.stats.stepsPlayed++;
        this.updateStepTiming(performance.now());
    }
    
    /**
     * Fallback playback loop for when Web Audio context is not available
     */
    startFallbackPlaybackLoop() {
        const playStep = () => {
            if (!this.state.isPlaying) return;
            
            const stepTime = performance.now();
            
            // Play audio for current step
            if (this.state.pattern.steps[this.state.currentStep]) {
                if (this.audioEngine) {
                    this.audioEngine.playSound();
                }
                
                // Send MIDI note if step is active
                if (this.midiOutput && this.midiOutput.isReady) {
                    this.midiOutput.noteOn();
                    
                    // Schedule note off after a short duration (100ms)
                    setTimeout(() => {
                        if (this.midiOutput && this.midiOutput.isReady) {
                            this.midiOutput.noteOff();
                        }
                    }, 100);
                }
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
        // Stop old interval-based timer
        if (this.state.playbackInterval) {
            clearInterval(this.state.playbackInterval);
            this.state.playbackInterval = null;
        }
        
        // Stop precise scheduler
        if (this.state.schedulerTimeout) {
            clearTimeout(this.state.schedulerTimeout);
            this.state.schedulerTimeout = null;
        }
        
        // Stop visual updates
        if (this.state.visualUpdateId) {
            cancelAnimationFrame(this.state.visualUpdateId);
            this.state.visualUpdateId = null;
        }
        
        // Clear scheduler state
        this.schedulerState = null;
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
        
        // Update scheduler state if using precise timing
        if (this.schedulerState) {
            this.schedulerState.stepDurationSeconds = this.state.stepDuration / 1000;
            
            // If playing, restart scheduler to apply new timing immediately
            if (this.state.isPlaying) {
                this.startPlaybackLoop();
            }
        } else if (this.state.isPlaying) {
            // Fallback: restart old interval-based loop
            this.startPlaybackLoop();
        }
        
        this.emitEvent('onSettingsChange', {
            setting: 'tempo',
            oldValue: oldTempo,
            newValue: newTempo
        });
        
        console.log(`🎵 Tempo changed: ${oldTempo} → ${newTempo} BPM (${this.schedulerState ? 'precise' : 'fallback'} timing)`);
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
     * Get MIDI output status and available devices
     * @returns {object} MIDI status information
     */
    getMIDIStatus() {
        if (!this.midiOutput) {
            return {
                enabled: false,
                ready: false,
                method: 'none',
                message: 'MIDI output not initialized'
            };
        }
        
        return this.midiOutput.getStatus();
    }
    
    /**
     * Get available MIDI output devices
     * @returns {Array} Array of available MIDI outputs
     */
    getMIDIOutputs() {
        return this.midiOutput ? this.midiOutput.getAvailableOutputs() : [];
    }
    
    /**
     * Select MIDI output device
     * @param {string} outputId - ID of MIDI output to select
     * @returns {boolean} Success status
     */
    selectMIDIOutput(outputId) {
        return this.midiOutput ? this.midiOutput.selectOutput(outputId) : false;
    }
    
    /**
     * Update MIDI settings
     * @param {object} settings - MIDI settings to update
     * @param {number} settings.channel - MIDI channel (1-16)
     * @param {number} settings.baseNote - Base MIDI note (0-127)
     * @param {number} settings.velocity - Note velocity (1-127)
     */
    updateMIDISettings(settings) {
        if (this.midiOutput) {
            this.midiOutput.updateSettings(settings);
            
            // Update config as well
            if (settings.channel) this.config.midiChannel = settings.channel;
            if (settings.baseNote) this.config.midiNote = settings.baseNote;
            if (settings.velocity) this.config.midiVelocity = settings.velocity;
        }
    }
    
    /**
     * Enable or disable MIDI output
     * @param {boolean} enabled - Whether to enable MIDI output
     */
    setMIDIEnabled(enabled) {
        this.config.enableMIDI = enabled;
        console.log(`🎹 MIDI output ${enabled ? 'enabled' : 'disabled'}`);
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
        if (this.midiOutput) {
            this.midiOutput.destroy();
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