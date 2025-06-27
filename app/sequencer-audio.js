/**
 * Sequencer Audio Module (sequencer-audio.js)
 * Web Audio API engine for rhythm pattern playback
 * 
 * Features:
 * - SequencerAudioEngine class - Main audio controller
 * - Web Audio API context management
 * - Configurable waveforms (sine, square, triangle, sawtooth)
 * - Volume, frequency, and envelope controls
 * - Cross-browser compatibility
 * - Error handling and fallbacks
 * 
 * Dependencies: None (standalone module)
 */

/**
 * Audio Engine for the Circular Sequencer
 * Handles all Web Audio API operations for rhythm pattern playback
 */
class SequencerAudioEngine {
    constructor() {
        this.context = null;
        this.isInitialized = false;
        this.isSupported = true;
        
        // Audio parameters with defaults
        this.settings = {
            frequency: 440,        // Hz
            volume: 0.5,          // 0.0 to 1.0
            waveform: 'sine',     // sine, square, triangle, sawtooth
            attackTime: 0.01,     // seconds
            releaseTime: 0.15,    // seconds
            maxGain: 0.3          // prevent clipping
        };
        
        // Performance tracking
        this.stats = {
            soundsPlayed: 0,
            contextsCreated: 0,
            errors: 0,
            lastError: null
        };
        
        console.log('🔊 SequencerAudioEngine initialized');
    }
    
    /**
     * Initialize the Web Audio API context
     * Must be called after user interaction due to browser autoplay policies
     */
    async initialize() {
        if (this.isInitialized && this.context && this.context.state !== 'closed') {
            return true;
        }
        
        try {
            // Try to create audio context
            const AudioContext = window.AudioContext || window.webkitAudioContext;
            if (!AudioContext) {
                this.isSupported = false;
                throw new Error('Web Audio API not supported in this browser');
            }
            
            this.context = new AudioContext();
            this.stats.contextsCreated++;
            
            // Handle context state changes
            this.context.addEventListener('statechange', () => {
                if (this.context) {
                    console.log(`🔊 Audio context state: ${this.context.state}`);
                }
            });
            
            // Resume context if suspended (common on mobile)
            if (this.context.state === 'suspended') {
                await this.context.resume();
            }
            
            this.isInitialized = this.context.state === 'running';
            
            if (this.isInitialized) {
                console.log('✅ Audio engine initialized successfully');
            } else {
                console.warn('⚠️ Audio context not running:', this.context.state);
            }
            
            return this.isInitialized;
            
        } catch (error) {
            this.stats.errors++;
            this.stats.lastError = error.message;
            this.isSupported = false;
            console.error('❌ Failed to initialize audio engine:', error);
            return false;
        }
    }
    
    /**
     * Resume audio context (required for autoplay policy compliance)
     */
    async resume() {
        if (!this.context) {
            return await this.initialize();
        }
        
        if (this.context.state === 'suspended') {
            try {
                await this.context.resume();
                console.log('🔊 Audio context resumed');
                return true;
            } catch (error) {
                this.stats.errors++;
                this.stats.lastError = error.message;
                console.error('❌ Failed to resume audio context:', error);
                return false;
            }
        }
        
        return this.context.state === 'running';
    }
    
    /**
     * Play a single sound with current settings
     * @param {number} duration - Sound duration in seconds (optional)
     * @param {object} overrides - Parameter overrides (optional)
     */
    async playSound(duration = null, overrides = {}) {
        if (!this.isSupported) {
            console.warn('⚠️ Audio not supported, skipping sound');
            return false;
        }
        
        if (!this.isInitialized) {
            const initialized = await this.initialize();
            if (!initialized) return false;
        }
        
        // Resume context if needed
        if (this.context.state !== 'running') {
            const resumed = await this.resume();
            if (!resumed) return false;
        }
        
        try {
            // Merge settings with overrides
            const params = { ...this.settings, ...overrides };
            
            // Create audio nodes
            const oscillator = this.context.createOscillator();
            const gainNode = this.context.createGain();
            
            // Configure oscillator
            oscillator.type = params.waveform;
            oscillator.frequency.setValueAtTime(params.frequency, this.context.currentTime);
            
            // Configure envelope
            const now = this.context.currentTime;
            const soundDuration = duration || params.releaseTime + params.attackTime;
            const maxGain = Math.min(params.volume, params.maxGain);
            
            // ADSR envelope (simplified - just attack and release)
            gainNode.gain.setValueAtTime(0, now);
            gainNode.gain.linearRampToValueAtTime(maxGain, now + params.attackTime);
            gainNode.gain.exponentialRampToValueAtTime(0.001, now + soundDuration);
            
            // Connect audio graph
            oscillator.connect(gainNode);
            gainNode.connect(this.context.destination);
            
            // Schedule playback
            oscillator.start(now);
            oscillator.stop(now + soundDuration);
            
            // Clean up when done
            oscillator.addEventListener('ended', () => {
                try {
                    oscillator.disconnect();
                    gainNode.disconnect();
                } catch (e) {
                    // Node may already be disconnected
                }
            });
            
            this.stats.soundsPlayed++;
            return true;
            
        } catch (error) {
            this.stats.errors++;
            this.stats.lastError = error.message;
            console.error('❌ Error playing sound:', error);
            return false;
        }
    }
    
    /**
     * Play a sound at a specific Web Audio context time
     * @param {number} startTime - When to start the sound (in audio context time)
     * @param {number} duration - Optional duration override
     * @returns {boolean} Success status
     */
    playSoundAtTime(startTime, duration = null) {
        if (!this.isSupported || !this.isInitialized) {
            return false;
        }
        
        // Ensure context is running
        if (this.context.state !== 'running') {
            console.warn('⚠️ Audio context not running, cannot schedule sound');
            return false;
        }
        
        try {
            const params = this.settings;
            
            // Create oscillator and gain nodes
            const oscillator = this.context.createOscillator();
            const gainNode = this.context.createGain();
            
            // Configure oscillator
            oscillator.type = params.waveform;
            oscillator.frequency.setValueAtTime(params.frequency, startTime);
            
            // Configure envelope with precise timing
            const soundDuration = duration || params.releaseTime + params.attackTime;
            const maxGain = Math.min(params.volume, params.maxGain);
            
            // ADSR envelope scheduled for precise start time
            gainNode.gain.setValueAtTime(0, startTime);
            gainNode.gain.linearRampToValueAtTime(maxGain, startTime + params.attackTime);
            gainNode.gain.exponentialRampToValueAtTime(0.001, startTime + soundDuration);
            
            // Connect audio graph
            oscillator.connect(gainNode);
            gainNode.connect(this.context.destination);
            
            // Schedule playback at precise time
            oscillator.start(startTime);
            oscillator.stop(startTime + soundDuration);
            
            // Clean up when done
            oscillator.addEventListener('ended', () => {
                try {
                    oscillator.disconnect();
                    gainNode.disconnect();
                } catch (e) {
                    // Node may already be disconnected
                }
            });
            
            this.stats.soundsPlayed++;
            return true;
            
        } catch (error) {
            this.stats.errors++;
            this.stats.lastError = error.message;
            console.error('❌ Error scheduling sound:', error);
            return false;
        }
    }
    
    /**
     * Update audio settings
     * @param {object} newSettings - Settings to update
     */
    updateSettings(newSettings) {
        const oldSettings = { ...this.settings };
        let hasValidChanges = false;
        
        // Only process known setting properties and validate them
        if (newSettings.hasOwnProperty('frequency')) {
            if (typeof newSettings.frequency === 'number' && newSettings.frequency > 0) {
                const clampedFreq = Math.max(20, Math.min(20000, newSettings.frequency));
                if (clampedFreq !== this.settings.frequency) {
                    this.settings.frequency = clampedFreq;
                    hasValidChanges = true;
                }
            }
        }
        
        if (newSettings.hasOwnProperty('volume')) {
            if (typeof newSettings.volume === 'number') {
                const clampedVol = Math.max(0, Math.min(1, newSettings.volume));
                if (clampedVol !== this.settings.volume) {
                    this.settings.volume = clampedVol;
                    hasValidChanges = true;
                }
            }
        }
        
        if (newSettings.hasOwnProperty('waveform')) {
            if (typeof newSettings.waveform === 'string') {
                const validWaveforms = ['sine', 'square', 'triangle', 'sawtooth'];
                if (validWaveforms.includes(newSettings.waveform) && newSettings.waveform !== this.settings.waveform) {
                    this.settings.waveform = newSettings.waveform;
                    hasValidChanges = true;
                }
            }
        }
        
        if (newSettings.hasOwnProperty('attackTime')) {
            if (typeof newSettings.attackTime === 'number' && newSettings.attackTime >= 0) {
                const clampedAttack = Math.max(0.001, Math.min(1, newSettings.attackTime));
                if (clampedAttack !== this.settings.attackTime) {
                    this.settings.attackTime = clampedAttack;
                    hasValidChanges = true;
                }
            }
        }
        
        if (newSettings.hasOwnProperty('releaseTime')) {
            if (typeof newSettings.releaseTime === 'number' && newSettings.releaseTime >= 0) {
                const clampedRelease = Math.max(0.001, Math.min(2, newSettings.releaseTime));
                if (clampedRelease !== this.settings.releaseTime) {
                    this.settings.releaseTime = clampedRelease;
                    hasValidChanges = true;
                }
            }
        }
        
        // Log significant changes
        if (hasValidChanges) {
            console.log('🔊 Audio settings updated:', this.settings);
        }
        
        return hasValidChanges;
    }
    
    /**
     * Get current audio settings
     */
    getSettings() {
        return { ...this.settings };
    }
    
    /**
     * Get audio engine statistics
     */
    getStats() {
        return {
            ...this.stats,
            isInitialized: this.isInitialized,
            isSupported: this.isSupported,
            contextState: this.context ? this.context.state : 'none',
            sampleRate: this.context ? this.context.sampleRate : 0
        };
    }
    
    /**
     * Test audio functionality
     * @param {number} testFrequency - Frequency to test with
     */
    async testAudio(testFrequency = 440) {
        console.log('🧪 Testing audio engine...');
        
        const originalFreq = this.settings.frequency;
        this.settings.frequency = testFrequency;
        
        const result = await this.playSound(0.2);
        
        this.settings.frequency = originalFreq;
        
        if (result) {
            console.log('✅ Audio test successful');
        } else {
            console.log('❌ Audio test failed');
        }
        
        return result;
    }
    
    /**
     * Destroy the audio engine and clean up resources
     */
    destroy() {
        if (this.context && this.context.state !== 'closed') {
            try {
                this.context.close();
                console.log('🔊 Audio context closed');
            } catch (error) {
                console.warn('⚠️ Error closing audio context:', error);
            }
        }
        
        this.context = null;
        this.isInitialized = false;
        
        console.log('🔊 Audio engine destroyed');
    }
}

/**
 * Audio Settings Validator
 * Utility class for validating audio parameter ranges
 */
class AudioSettingsValidator {
    static validateFrequency(freq) {
        if (typeof freq !== 'number' || isNaN(freq)) return false;
        return freq >= 20 && freq <= 20000;
    }
    
    static validateVolume(vol) {
        if (typeof vol !== 'number' || isNaN(vol)) return false;
        return vol >= 0 && vol <= 1;
    }
    
    static validateWaveform(wave) {
        const valid = ['sine', 'square', 'triangle', 'sawtooth'];
        return typeof wave === 'string' && valid.includes(wave);
    }
    
    static validateTime(time) {
        if (typeof time !== 'number' || isNaN(time)) return false;
        return time >= 0.001 && time <= 2;
    }
    
    static validateSettings(settings) {
        const errors = [];
        
        if (settings.frequency !== undefined && !this.validateFrequency(settings.frequency)) {
            errors.push('Invalid frequency: must be 20-20000 Hz');
        }
        
        if (settings.volume !== undefined && !this.validateVolume(settings.volume)) {
            errors.push('Invalid volume: must be 0.0-1.0');
        }
        
        if (settings.waveform !== undefined && !this.validateWaveform(settings.waveform)) {
            errors.push('Invalid waveform: must be sine, square, triangle, or sawtooth');
        }
        
        if (settings.attackTime !== undefined && !this.validateTime(settings.attackTime)) {
            errors.push('Invalid attack time: must be 0.001-2.0 seconds');
        }
        
        if (settings.releaseTime !== undefined && !this.validateTime(settings.releaseTime)) {
            errors.push('Invalid release time: must be 0.001-2.0 seconds');
        }
        
        return {
            isValid: errors.length === 0,
            errors
        };
    }
}

// Export to global scope for browser compatibility
if (typeof window !== 'undefined') {
    window.SequencerAudioEngine = SequencerAudioEngine;
    window.AudioSettingsValidator = AudioSettingsValidator;
}

// Export for Node.js compatibility (if needed for testing)
if (typeof module !== 'undefined' && module.exports) {
    module.exports = {
        SequencerAudioEngine,
        AudioSettingsValidator
    };
}