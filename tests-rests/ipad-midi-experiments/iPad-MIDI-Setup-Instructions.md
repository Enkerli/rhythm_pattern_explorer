# iPad MIDI Setup Instructions

## Problem Identified
The diagnostic log shows the iPad MIDI bridge is running in **SIMULATION ONLY** mode because Python MIDI libraries (`mido` and `python-rtmidi`) are not installed on the iPad.

```
WARNING - MIDI not available - bridge will run in simulation mode
⚠️ MIDI Output: SIMULATION ONLY
🎵 [SIMULATION] Note On: Ch1 Note36 (C2) Vel100
```

## Solutions Overview

### Option 1: Install Python MIDI Libraries on iPad (Recommended)
Install the required libraries in your iPad Python environment.

### Option 2: JavaScript-Only Bridge (Alternative)
Use a pure JavaScript solution that doesn't require Python MIDI libraries.

### Option 3: Mac-iPad Relay Bridge (Fallback)
Route MIDI through a Mac bridge that handles the iOS Network Session connection.

---

## Option 1: Install Python MIDI Libraries on iPad

### For Pythonista 3 (Most Popular iPad Python App)

1. **Install mido via pip:**
   ```python
   import pip
   pip.main(['install', 'mido'])
   ```

2. **Install python-rtmidi (may require compilation):**
   ```python
   import pip
   pip.main(['install', 'python-rtmidi'])
   ```

3. **If compilation fails, try pre-built wheel:**
   ```python
   # Download wheel from: https://pypi.org/project/python-rtmidi/#files
   # Look for iOS/arm64 compatible wheel
   pip.main(['install', 'python_rtmidi-1.4.9-cp39-cp39-macosx_11_0_arm64.whl'])
   ```

### For a-Shell (Alternative iPad Python Environment)

1. **Use built-in package manager:**
   ```bash
   pip install mido
   pip install python-rtmidi
   ```

2. **If rtmidi compilation fails:**
   ```bash
   # Try installing without binary dependencies
   pip install --no-binary python-rtmidi python-rtmidi
   ```

### For Code App (Supports pip)

1. **Terminal within Code app:**
   ```bash
   pip3 install mido python-rtmidi
   ```

2. **Check installation:**
   ```python
   import mido
   print("MIDI outputs:", mido.get_output_names())
   ```

---

## Option 2: JavaScript-Only iPad MIDI Bridge

If Python MIDI libraries can't be installed, use this pure JavaScript solution:

### Step 1: Create Mac WebRTC Signaling Server
```python
#!/usr/bin/env python3
"""
WebRTC Signaling Server for iPad Native MIDI Bridge
Enables direct JavaScript-to-MIDI connection without Python MIDI libraries on iPad
"""

import asyncio
import json
import websockets
import mido
from datetime import datetime

class WebRTCMIDISignaler:
    def __init__(self, port=8081):
        self.port = port
        self.clients = {}
        self.midi_out = None
        self.setup_midi()
    
    def setup_midi(self):
        """Setup MIDI output on Mac"""
        try:
            output_names = mido.get_output_names()
            print(f"Available MIDI outputs: {output_names}")
            
            # Look for IAC Driver
            iac_ports = [name for name in output_names if 'IAC' in name]
            if iac_ports:
                self.midi_out = mido.open_output(iac_ports[0])
                print(f"✅ MIDI connected: {iac_ports[0]}")
            elif output_names:
                self.midi_out = mido.open_output(output_names[0])
                print(f"✅ MIDI connected: {output_names[0]}")
        except Exception as e:
            print(f"❌ MIDI setup failed: {e}")
    
    async def handle_client(self, websocket, path):
        """Handle WebSocket connections from iPad"""
        client_id = f"{websocket.remote_address[0]}:{websocket.remote_address[1]}"
        self.clients[client_id] = websocket
        print(f"🌐 Client connected: {client_id}")
        
        try:
            async for message in websocket:
                data = json.loads(message)
                await self.handle_message(client_id, data)
        except websockets.exceptions.ConnectionClosed:
            print(f"🔌 Client disconnected: {client_id}")
        finally:
            self.clients.pop(client_id, None)
    
    async def handle_message(self, client_id, data):
        """Handle messages from iPad"""
        message_type = data.get('type')
        
        if message_type == 'midi':
            # Direct MIDI message from iPad
            await self.handle_midi_message(data)
        else:
            # WebRTC signaling
            await self.relay_signaling(client_id, data)
    
    async def handle_midi_message(self, data):
        """Send MIDI message to Mac output"""
        if not self.midi_out:
            return
            
        try:
            args = data.get('args', [])
            if len(args) >= 3:
                channel = max(0, min(15, args[0]['value'] - 1))
                note = max(0, min(127, args[1]['value']))
                velocity = max(0, min(127, args[2]['value']))
                
                if velocity > 0:
                    msg = mido.Message('note_on', channel=channel, note=note, velocity=velocity)
                    print(f"🎵 MIDI Note On: Ch{channel+1} Note{note} Vel{velocity}")
                else:
                    msg = mido.Message('note_off', channel=channel, note=note, velocity=0)
                    print(f"🎵 MIDI Note Off: Ch{channel+1} Note{note}")
                
                self.midi_out.send(msg)
                
        except Exception as e:
            print(f"❌ MIDI send error: {e}")
    
    async def relay_signaling(self, sender_id, data):
        """Relay WebRTC signaling between clients"""
        for client_id, websocket in self.clients.items():
            if client_id != sender_id:
                try:
                    await websocket.send(json.dumps(data))
                except:
                    pass
    
    async def start_server(self):
        """Start the signaling server"""
        print("WebRTC MIDI Signaling Server")
        print("=" * 40)
        print(f"🚀 Starting server on port {self.port}")
        print("📱 iPad: Connect to ws://[mac-ip]:8081")
        print("🎹 MIDI ready for iOS Network Session routing")
        print("=" * 40)
        
        async with websockets.serve(self.handle_client, "0.0.0.0", self.port):
            await asyncio.Future()  # Run forever

if __name__ == "__main__":
    server = WebRTCMIDISignaler()
    try:
        asyncio.run(server.start_server())
    except KeyboardInterrupt:
        print("\n🛑 Stopping server...")
```

### Step 2: Update iPad Bridge to Use JavaScript MIDI

Create `ipad-native-midi-bridge.js`:
```javascript
/**
 * iPad Native MIDI Bridge - JavaScript-only solution
 * Connects directly to Mac via WebRTC without requiring Python MIDI libraries on iPad
 */

class iPadNativeMIDIBridge {
    constructor() {
        this.isConnected = false;
        this.websocket = null;
        this.macIP = null;
        
        console.log('🎹 iPad Native MIDI Bridge initialized');
    }
    
    /**
     * Start the bridge - connect directly to Mac signaling server
     */
    async initialize(macIP) {
        this.macIP = macIP;
        const signalServerUrl = `ws://${macIP}:8081`;
        
        try {
            console.log(`🌐 Connecting to Mac at ${signalServerUrl}...`);
            
            this.websocket = new WebSocket(signalServerUrl);
            
            this.websocket.onopen = () => {
                console.log('✅ Connected to Mac MIDI bridge');
                this.isConnected = true;
            };
            
            this.websocket.onmessage = (event) => {
                const data = JSON.parse(event.data);
                console.log('📨 Received from Mac:', data);
            };
            
            this.websocket.onerror = (error) => {
                console.error('❌ Connection error:', error);
                this.isConnected = false;
            };
            
            this.websocket.onclose = () => {
                console.log('🔌 Disconnected from Mac');
                this.isConnected = false;
            };
            
        } catch (error) {
            console.error('❌ Failed to initialize bridge:', error);
        }
    }
    
    /**
     * Send MIDI note via WebSocket to Mac
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
                ]
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
     * Get connection status
     */
    getStatus() {
        return {
            connected: this.isConnected,
            websocket: this.websocket ? this.websocket.readyState : 'closed',
            macIP: this.macIP
        };
    }
    
    /**
     * Close connection
     */
    close() {
        if (this.websocket) {
            this.websocket.close();
        }
        
        this.isConnected = false;
        console.log('🛑 iPad MIDI bridge closed');
    }
}

// Export for use in web app
window.iPadNativeMIDIBridge = iPadNativeMIDIBridge;
```

---

## Option 3: Mac-iPad Relay Bridge

If both above options fail, use a Mac relay:

### Step 1: Enhanced Mac Bridge
```python
#!/usr/bin/env python3
"""
Mac-iPad MIDI Relay Bridge
Receives WebSocket from iPad, sends MIDI to both Mac IAC and iPad Network Session
"""

import asyncio
import websockets
import mido
import json

class MaciPadMIDIRelay:
    def __init__(self):
        self.mac_midi = None
        self.ipad_midi = None
        self.setup_midi_outputs()
    
    def setup_midi_outputs(self):
        """Setup both Mac and iPad MIDI outputs"""
        output_names = mido.get_output_names()
        print(f"Available outputs: {output_names}")
        
        # Mac IAC Driver
        iac_ports = [name for name in output_names if 'IAC' in name]
        if iac_ports:
            self.mac_midi = mido.open_output(iac_ports[0])
            print(f"✅ Mac MIDI: {iac_ports[0]}")
        
        # iPad Network Session
        network_ports = [name for name in output_names if 'Network' in name or 'iPad' in name]
        if network_ports:
            self.ipad_midi = mido.open_output(network_ports[0])
            print(f"✅ iPad MIDI: {network_ports[0]}")
    
    async def handle_websocket(self, websocket, path):
        """Handle WebSocket from iPad Safari"""
        print(f"🌐 iPad connected: {websocket.remote_address[0]}")
        
        try:
            async for message in websocket:
                data = json.loads(message)
                await self.relay_midi(data)
        except websockets.exceptions.ConnectionClosed:
            print("🔌 iPad disconnected")
    
    async def relay_midi(self, data):
        """Send MIDI to both Mac and iPad outputs"""
        if data.get('address') == '/midi/note':
            args = data.get('args', [])
            if len(args) >= 3:
                channel = max(0, min(15, args[0]['value'] - 1))
                note = max(0, min(127, args[1]['value']))
                velocity = max(0, min(127, args[2]['value']))
                
                # Create MIDI message
                if velocity > 0:
                    msg = mido.Message('note_on', channel=channel, note=note, velocity=velocity)
                else:
                    msg = mido.Message('note_off', channel=channel, note=note, velocity=0)
                
                # Send to Mac
                if self.mac_midi:
                    self.mac_midi.send(msg)
                    print(f"🖥️ Mac MIDI: {msg}")
                
                # Send to iPad
                if self.ipad_midi:
                    self.ipad_midi.send(msg)
                    print(f"📱 iPad MIDI: {msg}")
    
    async def start_server(self):
        """Start relay server"""
        print("Mac-iPad MIDI Relay Bridge")
        print("=" * 40)
        print("🖥️ Mac IAC Driver: Ready")
        print("📱 iPad Network Session: Ready")
        print("🌐 WebSocket: ws://localhost:8080")
        print("=" * 40)
        
        async with websockets.serve(self.handle_websocket, "localhost", 8080):
            await asyncio.Future()

if __name__ == "__main__":
    relay = MaciPadMIDIRelay()
    try:
        asyncio.run(relay.start_server())
    except KeyboardInterrupt:
        print("\n🛑 Stopping relay...")
```

---

## Quick Setup Summary

### Immediate Fix (Recommended):
1. **Install mido on iPad**: `pip install mido python-rtmidi`
2. **Re-run diagnostic bridge**: `python3 ipad-midi-bridge-diagnostic.py`
3. **Check for "✅ MIDI Output: ACTIVE" instead of "SIMULATION ONLY"**

### If Installation Fails:
1. **Use JavaScript bridge** (Option 2)
2. **Run Mac signaling server**: `python3 webrtc-midi-signaler.py`
3. **Load JavaScript bridge on iPad Safari**
4. **Connect iPad to Mac IP address**

### Test MIDI Reception:
1. **Enable iOS Network Session**: Settings > General > AirPlay & Handoff > Network MIDI
2. **Open GarageBand** and check for "Network Session 1" input
3. **Run rhythm pattern player** and verify MIDI notes appear in GarageBand

The root cause is clear: **Python MIDI libraries missing on iPad**. Option 1 will fix it directly, Option 2 provides a workaround using JavaScript only.