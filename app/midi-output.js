/**
 * MIDI Output Module (midi-output.js)
 * Real-time MIDI note output for rhythm pattern playback
 * 
 * Features:
 * - Multi-protocol support (WebMIDI, OSC, fallbacks)
 * - Safari compatibility via OSC over WebSocket
 * - Chrome/Edge native WebMIDI support
 * - Configurable MIDI channels and note mappings
 * - Real-time clock sync options
 * - External DAW integration
 * 
 * Dependencies: None (uses browser APIs)
 * 
 * Browser Support Strategy:
 * - Chrome/Edge/Opera: WebMIDI API (native)
 * - Safari/Firefox: OSC over WebSocket (requires local bridge)
 * - Fallback: Copy MIDI data to clipboard or show instructions
 */

/**
 * MIDI Output Controller
 * 
 * Manages real-time MIDI output with cross-browser compatibility.
 * Automatically detects the best available method for each browser
 * and provides unified interface for rhythm pattern playback.
 * 
 * Protocol Hierarchy:
 * 1. WebMIDI API (Chrome, Edge, Opera)
 * 2. HTTP OSC Gateway (Direct TouchOSC connection)
 * 3. WebRTC OSC (Direct TouchOSC connection, iPad compatible)
 * 4. OSC over WebSocket (Safari, Firefox with MIDI bridge)
 * 5. User guidance fallback (unsupported browsers)
 * 
 * MIDI Implementation:
 * - Note On/Off messages (0x90/0x80)
 * - Configurable velocity, channel, note mapping
 * - Timing synchronization with Web Audio clock
 * - Multiple output device support
 * 
 * OSC Implementation:
 * - WebSocket connection to local OSC bridge
 * - Standard OSC message format: /midi/note [channel] [note] [velocity]
 * - Automatic reconnection and error handling
 * - Compatible with TouchOSC Bridge, OSCRouter, custom servers
 */
class MIDIOutputController {
    /**
     * Initialize MIDI Output Controller
     * 
     * @param {Object} options - Configuration options
     * @param {number} options.channel - MIDI channel (1-16, default: 1)
     * @param {number} options.baseNote - Base MIDI note (default: 36 - C2 kick)
     * @param {number} options.velocity - Note velocity (1-127, default: 100)
     * @param {string} options.oscHost - OSC WebSocket host (default: 'localhost:8080')
     * @param {boolean} options.autoConnect - Auto-connect on init (default: true)
     */
    constructor(options = {}) {
        this.settings = {
            channel: options.channel || 1,
            baseNote: options.baseNote || 36, // C2 - typical kick drum note
            velocity: options.velocity || 100,
            oscHost: options.oscHost || 'ws://localhost:8080',
            autoConnect: options.autoConnect !== false
        };
        
        // Connection state
        this.webmidi = {
            supported: false,
            enabled: false,
            outputs: [],
            selectedOutput: null
        };
        
        this.osc = {
            supported: false,
            connected: false,
            socket: null,
            reconnectAttempts: 0,
            maxReconnectAttempts: 5
        };
        
        this.webrtcOSC = {
            supported: false,
            connected: false,
            controller: null
        };
        
        this.httpOSC = {
            supported: false,
            connected: false,
            gatewayUrl: options.httpOSCGateway || 'http://localhost:8889/osc'
        };
        
        // Current output method
        this.outputMethod = 'none';
        this.isReady = false;
        
        // Event handlers
        this.onReady = null;
        this.onError = null;
        this.onStateChange = null;
        
        // Don't auto-initialize - wait for user interaction
        // This ensures browsers can show permission prompts properly
    }
    
    /**
     * Initialize MIDI output system
     * Detects available methods and connects to the best option
     */
    async initialize() {
        console.log('🎹 Initializing MIDI Output...');
        
        // Try WebMIDI first (Chrome, Edge, Opera)
        if (await this.initializeWebMIDI()) {
            this.outputMethod = 'webmidi';
            this.isReady = true;
            console.log('✅ WebMIDI initialized successfully');
            this.notifyReady();
            return true;
        }
        
        // Try OSC over WebSocket first (Python bridge - most reliable)
        if (await this.initializeOSC()) {
            this.outputMethod = 'osc';
            this.isReady = true;
            console.log('✅ OSC over WebSocket initialized successfully');
            this.notifyReady();
            return true;
        }
        
        // Try HTTP OSC Gateway (TouchOSC direct connection)
        if (await this.initializeHTTPOSC()) {
            this.outputMethod = 'http_osc';
            this.isReady = true;
            console.log('✅ HTTP OSC gateway initialized successfully');
            this.notifyReady();
            return true;
        }
        
        // Try WebRTC OSC (Direct TouchOSC, iPad compatible)
        if (await this.initializeWebRTCOSC()) {
            this.outputMethod = 'webrtc_osc';
            this.isReady = true;
            console.log('✅ WebRTC OSC initialized successfully');
            this.notifyReady();
            return true;
        }
        
        // Fallback mode
        this.outputMethod = 'fallback';
        console.log('⚠️ No MIDI output available - fallback mode');
        this.showFallbackInstructions();
        return false;
    }
    
    /**
     * Initialize WebMIDI API (Chrome, Edge, Opera)
     */
    async initializeWebMIDI() {
        if (!navigator.requestMIDIAccess) {
            console.log('❌ WebMIDI API not supported in this browser');
            console.log('Browser:', navigator.userAgent);
            return false;
        }
        
        try {
            console.log('🔍 Requesting WebMIDI access...');
            console.log('Browser supports WebMIDI API');
            
            // Request MIDI access with sysex for broader compatibility
            const midiAccess = await navigator.requestMIDIAccess({ sysex: false });
            console.log('✅ WebMIDI access granted');
            
            this.webmidi.supported = true;
            this.webmidi.enabled = true;
            
            // Get available outputs
            this.webmidi.outputs = Array.from(midiAccess.outputs.values());
            console.log(`🔍 Found ${this.webmidi.outputs.length} MIDI outputs total`);
            
            // Log all available outputs for debugging
            midiAccess.outputs.forEach((output, key) => {
                console.log(`  Output ${key}: ${output.name} (${output.manufacturer})`);
            });
            
            if (this.webmidi.outputs.length > 0) {
                this.webmidi.selectedOutput = this.webmidi.outputs[0];
                console.log(`🎹 Selected MIDI output: ${this.webmidi.selectedOutput.name}`);
                return true;
            } else {
                console.log('⚠️ No MIDI outputs found - check MIDI setup');
                console.log('💡 Try: Enable IAC Driver or connect MIDI device');
                return false;
            }
        } catch (error) {
            console.error('❌ WebMIDI initialization failed:', error);
            console.log('💡 Brave users: Check if site permissions allow MIDI access');
            return false;
        }
    }
    
    /**
     * Initialize OSC over WebSocket (Safari, Firefox fallback)
     */
    async initializeOSC() {
        if (!window.WebSocket) {
            console.log('❌ WebSocket not supported');
            return false;
        }
        
        return new Promise((resolve) => {
            try {
                console.log(`🌐 Connecting to OSC bridge at ${this.settings.oscHost}...`);
                console.log('💡 Make sure bridge is running: python3 minimal-midi-bridge.py');
                
                this.osc.socket = new WebSocket(this.settings.oscHost);
                
                // Set up timeout with proper cleanup
                const timeoutId = setTimeout(() => {
                    if (!this.osc.connected) {
                        console.log('⏰ OSC connection timeout - is bridge running?');
                        console.log('💡 To start bridge: python3 minimal-midi-bridge.py');
                        try {
                            this.osc.socket.close();
                        } catch (e) {
                            // Ignore close errors
                        }
                        resolve(false);
                    }
                }, 5000);
                
                this.osc.socket.onopen = () => {
                    clearTimeout(timeoutId);
                    console.log('✅ OSC WebSocket connected to bridge');
                    this.osc.connected = true;
                    this.osc.supported = true;
                    this.osc.reconnectAttempts = 0;
                    resolve(true);
                };
                
                this.osc.socket.onerror = (error) => {
                    clearTimeout(timeoutId);
                    console.log('❌ OSC WebSocket connection failed:', error);
                    console.log('💡 Is the bridge running on localhost:8080?');
                    resolve(false);
                };
                
                this.osc.socket.onclose = (event) => {
                    console.log(`🔌 OSC WebSocket disconnected (code: ${event.code})`);
                    this.osc.connected = false;
                    if (event.code !== 1000) { // Not a normal closure
                        this.attemptOSCReconnect();
                    }
                };
                
            } catch (error) {
                console.error('❌ OSC initialization error:', error);
                resolve(false);
            }
        });
    }
    
    /**
     * Initialize WebRTC OSC (Direct TouchOSC connection)
     */
    async initializeWebRTCOSC() {
        if (typeof WebRTCOSCController === 'undefined') {
            console.log('❌ WebRTC OSC controller not available');
            return false;
        }
        
        try {
            console.log('🌐 Initializing WebRTC OSC for TouchOSC...');
            console.log('💡 Configure TouchOSC: UDP Server on your device IP:8000');
            
            this.webrtcOSC.controller = new WebRTCOSCController({
                touchOSCHost: this.settings.touchOSCHost || 'localhost',
                touchOSCPort: this.settings.touchOSCPort || 8000,
                autoConnect: false
            });
            
            // Set up event handlers
            this.webrtcOSC.controller.onReady = (info) => {
                console.log('✅ WebRTC OSC ready for TouchOSC');
                this.webrtcOSC.connected = true;
                this.webrtcOSC.supported = true;
            };
            
            this.webrtcOSC.controller.onError = (error) => {
                console.log('❌ WebRTC OSC error:', error.message);
            };
            
            const success = await this.webrtcOSC.controller.initialize();
            return success;
            
        } catch (error) {
            console.error('❌ WebRTC OSC initialization error:', error);
            return false;
        }
    }
    
    /**
     * Initialize HTTP OSC Gateway (TouchOSC via HTTP bridge)
     */
    async initializeHTTPOSC() {
        try {
            console.log('🌐 Testing HTTP OSC gateway...');
            console.log('💡 Start gateway: python3 touchosc-gateway.py');
            
            // Test gateway availability
            const testResponse = await fetch(this.httpOSC.gatewayUrl, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    address: '/test',
                    args: []
                })
            });
            
            if (testResponse.ok) {
                this.httpOSC.supported = true;
                this.httpOSC.connected = true;
                console.log('✅ HTTP OSC gateway is available');
                return true;
            } else {
                console.log('❌ HTTP OSC gateway not responding');
                return false;
            }
            
        } catch (error) {
            console.log('❌ HTTP OSC gateway not available:', error.message);
            return false;
        }
    }
    
    /**
     * Attempt OSC reconnection with exponential backoff
     */
    attemptOSCReconnect() {
        if (this.osc.reconnectAttempts >= this.osc.maxReconnectAttempts) {
            console.log('❌ OSC reconnection attempts exceeded');
            this.outputMethod = 'fallback';
            this.isReady = false;
            return;
        }
        
        const delay = Math.pow(2, this.osc.reconnectAttempts) * 1000; // 1s, 2s, 4s, 8s, 16s
        this.osc.reconnectAttempts++;
        
        console.log(`🔄 OSC reconnect attempt ${this.osc.reconnectAttempts} in ${delay}ms...`);
        setTimeout(() => {
            this.initializeOSC();
        }, delay);
    }
    
    /**
     * Send MIDI note on message
     * 
     * @param {number} note - MIDI note number (0-127)
     * @param {number} velocity - Note velocity (1-127, default: from settings)
     * @param {number} channel - MIDI channel (1-16, default: from settings)
     */
    noteOn(note = null, velocity = null, channel = null) {
        if (!this.isReady) {
            console.warn('⚠️ MIDI output not ready');
            return false;
        }
        
        const midiNote = note !== null ? note : this.settings.baseNote;
        const midiVelocity = velocity !== null ? velocity : this.settings.velocity;
        const midiChannel = channel !== null ? channel : this.settings.channel;
        
        switch (this.outputMethod) {
            case 'webmidi':
                return this.sendWebMIDINote(midiNote, midiVelocity, midiChannel, true);
            case 'osc':
                return this.sendOSCNote(midiNote, midiVelocity, midiChannel, true);
            case 'webrtc_osc':
                return this.sendWebRTCOSCNote(midiNote, midiVelocity, midiChannel, true);
            case 'http_osc':
                return this.sendHTTPOSCNote(midiNote, midiVelocity, midiChannel, true);
            default:
                console.log(`🎹 Note On: Ch${midiChannel} Note${midiNote} Vel${midiVelocity}`);
                return false;
        }
    }
    
    /**
     * Send MIDI note off message
     * 
     * @param {number} note - MIDI note number (0-127)
     * @param {number} channel - MIDI channel (1-16, default: from settings)
     */
    noteOff(note = null, channel = null) {
        if (!this.isReady) {
            return false;
        }
        
        const midiNote = note !== null ? note : this.settings.baseNote;
        const midiChannel = channel !== null ? channel : this.settings.channel;
        
        switch (this.outputMethod) {
            case 'webmidi':
                return this.sendWebMIDINote(midiNote, 0, midiChannel, false);
            case 'osc':
                return this.sendOSCNote(midiNote, 0, midiChannel, false);
            case 'webrtc_osc':
                return this.sendWebRTCOSCNote(midiNote, 0, midiChannel, false);
            case 'http_osc':
                return this.sendHTTPOSCNote(midiNote, 0, midiChannel, false);
            default:
                console.log(`🎹 Note Off: Ch${midiChannel} Note${midiNote}`);
                return false;
        }
    }
    
    /**
     * Send WebMIDI note message
     */
    sendWebMIDINote(note, velocity, channel, noteOn) {
        if (!this.webmidi.selectedOutput) {
            return false;
        }
        
        try {
            const status = noteOn ? (0x90 + (channel - 1)) : (0x80 + (channel - 1));
            const message = [status, note, velocity];
            
            this.webmidi.selectedOutput.send(message);
            return true;
        } catch (error) {
            console.error('❌ WebMIDI send error:', error);
            return false;
        }
    }
    
    /**
     * Send WebRTC OSC note message
     */
    sendWebRTCOSCNote(note, velocity, channel, noteOn) {
        if (!this.webrtcOSC.connected || !this.webrtcOSC.controller) {
            return false;
        }
        
        try {
            return this.webrtcOSC.controller.sendOSCNote(channel, note, velocity, noteOn);
        } catch (error) {
            console.error('❌ WebRTC OSC send error:', error);
            return false;
        }
    }
    
    /**
     * Send HTTP OSC note message
     */
    async sendHTTPOSCNote(note, velocity, channel, noteOn) {
        if (!this.httpOSC.connected) {
            return false;
        }
        
        try {
            const address = noteOn ? '/note/on' : '/note/off';
            const args = noteOn ? [channel, note, velocity] : [channel, note];
            
            const response = await fetch(this.httpOSC.gatewayUrl, {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({
                    address: address,
                    args: args
                })
            });
            
            if (response.ok) {
                console.log(`📤 HTTP OSC ${noteOn ? 'Note On' : 'Note Off'}: Ch${channel} Note${note}${noteOn ? ` Vel${velocity}` : ''}`);
                
                // Send trigger for TouchOSC visualization
                if (noteOn) {
                    fetch(this.httpOSC.gatewayUrl, {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({ address: '/trigger', args: [note] })
                    }).catch(() => {}); // Ignore errors for trigger
                }
                
                return true;
            }
            
            return false;
            
        } catch (error) {
            console.error('❌ HTTP OSC send error:', error);
            return false;
        }
    }
    
    /**
     * Send OSC note message
     */
    sendOSCNote(note, velocity, channel, noteOn) {
        if (!this.osc.connected || !this.osc.socket) {
            return false;
        }
        
        try {
            const message = {
                address: '/midi/note',
                args: [
                    { type: 'i', value: channel },
                    { type: 'i', value: note },
                    { type: 'i', value: noteOn ? velocity : 0 }
                ]
            };
            
            this.osc.socket.send(JSON.stringify(message));
            return true;
        } catch (error) {
            console.error('❌ OSC send error:', error);
            return false;
        }
    }
    
    /**
     * Get available MIDI outputs for user selection
     */
    getAvailableOutputs() {
        if (this.outputMethod === 'webmidi') {
            return this.webmidi.outputs.map(output => ({
                id: output.id,
                name: output.name,
                manufacturer: output.manufacturer
            }));
        }
        return [];
    }
    
    /**
     * Select MIDI output device
     */
    selectOutput(outputId) {
        if (this.outputMethod === 'webmidi') {
            const output = this.webmidi.outputs.find(o => o.id === outputId);
            if (output) {
                this.webmidi.selectedOutput = output;
                console.log(`🎹 Selected MIDI output: ${output.name}`);
                return true;
            }
        }
        return false;
    }
    
    /**
     * Update settings
     */
    updateSettings(newSettings) {
        Object.assign(this.settings, newSettings);
        console.log('⚙️ MIDI settings updated:', this.settings);
    }
    
    /**
     * Show fallback instructions for unsupported browsers
     */
    showFallbackInstructions() {
        const instructions = `
🎹 MIDI Output Not Available

For MIDI output support:

Chrome/Edge/Opera:
• Direct MIDI support available
• Connect MIDI devices or use virtual MIDI (loopMIDI, IAC Driver)

Safari/Firefox:
• Install OSC bridge software:
  - TouchOSC Bridge (free)
  - OSCRouter 
  - Custom Python/Node.js OSC-to-MIDI bridge
• Start bridge on localhost:8080
• Reload this page

Alternative:
• Copy pattern data and import to your DAW
• Use Web MIDI Browser app (iOS)
• Try Chrome for native MIDI support
        `.trim();
        
        console.log(instructions);
        
        // You could show this in a modal or notification
        if (this.onError) {
            this.onError({
                type: 'no_midi_support',
                message: 'MIDI output not available',
                instructions: instructions
            });
        }
    }
    
    /**
     * Notify ready state
     */
    notifyReady() {
        if (this.onReady) {
            this.onReady({
                method: this.outputMethod,
                outputs: this.getAvailableOutputs()
            });
        }
        
        if (this.onStateChange) {
            this.onStateChange({
                ready: this.isReady,
                method: this.outputMethod,
                connected: this.outputMethod === 'webmidi' ? this.webmidi.enabled : this.osc.connected
            });
        }
    }
    
    /**
     * Get current status
     */
    getStatus() {
        return {
            ready: this.isReady,
            method: this.outputMethod,
            webmidi: {
                supported: this.webmidi.supported,
                enabled: this.webmidi.enabled,
                outputs: this.webmidi.outputs.length,
                selectedOutput: this.webmidi.selectedOutput?.name
            },
            osc: {
                supported: this.osc.supported,
                connected: this.osc.connected,
                host: this.settings.oscHost
            },
            webrtcOSC: {
                supported: this.webrtcOSC.supported,
                connected: this.webrtcOSC.connected,
                host: this.settings.touchOSCHost || 'localhost',
                port: this.settings.touchOSCPort || 8000
            },
            settings: { ...this.settings }
        };
    }
    
    /**
     * Clean up resources
     */
    destroy() {
        if (this.osc.socket) {
            this.osc.socket.close();
            this.osc.socket = null;
        }
        
        if (this.webrtcOSC.controller) {
            this.webrtcOSC.controller.destroy();
            this.webrtcOSC.controller = null;
        }
        
        this.isReady = false;
        this.outputMethod = 'none';
        
        console.log('🗑️ MIDI output controller destroyed');
    }
}

// Export for browser global scope
window.MIDIOutputController = MIDIOutputController;

console.log('✅ MIDI Output module loaded - MIDIOutputController:', typeof MIDIOutputController);