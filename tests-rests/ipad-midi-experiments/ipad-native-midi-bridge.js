/**
 * iPad Native MIDI Bridge - JavaScript-only solution
 * Connects directly to Mac via WebSocket without requiring Python MIDI libraries on iPad
 * 
 * This bypasses the need for mido/python-rtmidi on iPad by sending MIDI messages
 * directly to a Mac bridge that handles the actual MIDI output.
 */

class iPadNativeMIDIBridge {
    constructor() {
        this.isConnected = false;
        this.websocket = null;
        this.macIP = null;
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = 5;
        
        console.log('🎹 iPad Native MIDI Bridge initialized');
    }
    
    /**
     * Start the bridge - connect directly to Mac signaling server
     */
    async initialize(macIP, port = 8082) {
        this.macIP = macIP;
        this.port = port;
        const signalServerUrl = `ws://${macIP}:${port}`;
        
        try {
            console.log(`🌐 Connecting to Mac at ${signalServerUrl}...`);
            console.log('💡 Make sure Mac is running: python3 webrtc-midi-signaler.py');
            
            this.websocket = new WebSocket(signalServerUrl);
            
            this.websocket.onopen = () => {
                console.log('✅ Connected to Mac MIDI bridge');
                this.isConnected = true;
                this.reconnectAttempts = 0;
            };
            
            this.websocket.onmessage = (event) => {
                const data = JSON.parse(event.data);
                console.log('📨 Received from Mac:', data);
            };
            
            this.websocket.onerror = (error) => {
                console.error('❌ Connection error:', error);
                this.isConnected = false;
            };
            
            this.websocket.onclose = (event) => {
                console.log(`🔌 Disconnected from Mac (code: ${event.code})`);
                this.isConnected = false;
                
                if (event.code !== 1000) { // Not a normal closure
                    this.attemptReconnect();
                }
            };
            
        } catch (error) {
            console.error('❌ Failed to initialize bridge:', error);
        }
    }
    
    /**
     * Attempt to reconnect with exponential backoff
     */
    attemptReconnect() {
        if (this.reconnectAttempts >= this.maxReconnectAttempts) {
            console.log('❌ Max reconnection attempts reached');
            return;
        }
        
        const delay = Math.pow(2, this.reconnectAttempts) * 1000; // 1s, 2s, 4s, 8s, 16s
        this.reconnectAttempts++;
        
        console.log(`🔄 Reconnect attempt ${this.reconnectAttempts} in ${delay}ms...`);
        setTimeout(() => {
            this.initialize(this.macIP);
        }, delay);
    }
    
    /**
     * Send MIDI note via WebSocket to Mac
     * @param {number} channel - MIDI channel (1-16)
     * @param {number} note - MIDI note number (0-127)
     * @param {number} velocity - Note velocity (0-127, 0 = note off)
     */
    sendMIDINote(channel, note, velocity) {
        if (!this.isConnected || !this.websocket) {
            console.warn('⚠️ MIDI bridge not connected');
            return false;
        }
        
        try {
            const midiMessage = {
                type: 'midi',
                address: '/midi/note',
                args: [
                    { type: 'i', value: channel },
                    { type: 'i', value: note },
                    { type: 'i', value: velocity }
                ],
                timestamp: Date.now()
            };
            
            this.websocket.send(JSON.stringify(midiMessage));
            
            if (velocity > 0) {
                console.log(`🎵 MIDI Note On: Ch${channel} Note${note} Vel${velocity}`);
            } else {
                console.log(`🎵 MIDI Note Off: Ch${channel} Note${note}`);
            }
            
            return true;
            
        } catch (error) {
            console.error('❌ Failed to send MIDI:', error);
            return false;
        }
    }
    
    /**
     * Send MIDI control change
     * @param {number} channel - MIDI channel (1-16)
     * @param {number} control - Control number (0-127)
     * @param {number} value - Control value (0-127)
     */
    sendMIDICC(channel, control, value) {
        if (!this.isConnected || !this.websocket) {
            console.warn('⚠️ MIDI bridge not connected');
            return false;
        }
        
        try {
            const midiMessage = {
                type: 'midi',
                address: '/midi/cc',
                args: [
                    { type: 'i', value: channel },
                    { type: 'i', value: control },
                    { type: 'i', value: value }
                ],
                timestamp: Date.now()
            };
            
            this.websocket.send(JSON.stringify(midiMessage));
            console.log(`🎛️ MIDI CC: Ch${channel} CC${control} Val${value}`);
            
            return true;
            
        } catch (error) {
            console.error('❌ Failed to send MIDI CC:', error);
            return false;
        }
    }
    
    /**
     * Test the MIDI connection
     */
    testConnection() {
        console.log('🧪 Testing MIDI connection...');
        
        // Send test note
        this.sendMIDINote(1, 60, 64); // C4 note on
        
        setTimeout(() => {
            this.sendMIDINote(1, 60, 0); // C4 note off
        }, 500);
        
        console.log('🧪 Test complete - check GarageBand or other MIDI apps');
    }
    
    /**
     * Get connection status
     */
    getStatus() {
        return {
            connected: this.isConnected,
            websocket: this.websocket ? this.websocket.readyState : 'closed',
            websocketState: this.websocket ? ['CONNECTING', 'OPEN', 'CLOSING', 'CLOSED'][this.websocket.readyState] : 'NONE',
            macIP: this.macIP,
            port: this.port,
            reconnectAttempts: this.reconnectAttempts
        };
    }
    
    /**
     * Close connection
     */
    close() {
        if (this.websocket) {
            this.websocket.close(1000, 'Normal closure');
        }
        
        this.isConnected = false;
        this.reconnectAttempts = 0;
        console.log('🛑 iPad MIDI bridge closed');
    }
}

// Export for use in web app
window.iPadNativeMIDIBridge = iPadNativeMIDIBridge;

// Auto-start if running as standalone page
if (window.location.pathname.includes('ipad-native-midi') || window.location.search.includes('standalone')) {
    console.log('🎹 iPad Native MIDI Bridge - Standalone Mode');
    
    const bridge = new iPadNativeMIDIBridge();
    
    // Prompt for Mac IP address
    const macIP = prompt('Enter Mac IP address (e.g., 192.168.1.100):', '192.168.1.100');
    
    if (macIP) {
        // Connect to Mac
        bridge.initialize(macIP).then(() => {
            console.log('🎹 iPad Native MIDI Bridge ready');
            
            // Test with a note every 3 seconds
            setInterval(() => {
                if (bridge.isConnected) {
                    bridge.sendMIDINote(1, 36, 100); // Kick drum note on
                    setTimeout(() => {
                        bridge.sendMIDINote(1, 36, 0); // Note off
                    }, 100);
                }
            }, 3000);
            
            // Add UI controls
            createStandaloneUI(bridge);
        });
    }
}

/**
 * Create standalone UI for testing
 */
function createStandaloneUI(bridge) {
    // Create UI container
    const container = document.createElement('div');
    container.style.cssText = `
        position: fixed;
        top: 20px;
        left: 20px;
        right: 20px;
        background: #f0f0f0;
        border: 2px solid #333;
        border-radius: 10px;
        padding: 20px;
        font-family: monospace;
        z-index: 1000;
    `;
    
    container.innerHTML = `
        <h2>🎹 iPad Native MIDI Bridge</h2>
        <div id="status">Connecting...</div>
        <br>
        <button id="testBtn">🧪 Test Connection</button>
        <button id="statusBtn">📊 Show Status</button>
        <button id="closeBtn">🛑 Close Bridge</button>
        <br><br>
        <div style="font-size: 12px;">
            <strong>Instructions:</strong><br>
            1. Make sure Mac is running webrtc-midi-signaler.py<br>
            2. Enable iOS Network Session in Settings<br>
            3. Open GarageBand to test MIDI reception<br>
            4. Click Test Connection to verify setup
        </div>
    `;
    
    document.body.appendChild(container);
    
    // Update status
    const statusDiv = container.querySelector('#status');
    const updateStatus = () => {
        const status = bridge.getStatus();
        statusDiv.innerHTML = `
            Connection: ${status.connected ? '✅ Connected' : '❌ Disconnected'}<br>
            WebSocket: ${status.websocketState}<br>
            Mac IP: ${status.macIP}
        `;
    };
    
    setInterval(updateStatus, 1000);
    
    // Button handlers
    container.querySelector('#testBtn').onclick = () => bridge.testConnection();
    container.querySelector('#statusBtn').onclick = () => console.log('Bridge Status:', bridge.getStatus());
    container.querySelector('#closeBtn').onclick = () => {
        bridge.close();
        document.body.removeChild(container);
    };
}