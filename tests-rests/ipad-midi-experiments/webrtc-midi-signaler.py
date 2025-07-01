#!/usr/bin/env python3
"""
WebRTC MIDI Signaling Server for iPad Native MIDI Bridge
Enables direct JavaScript-to-MIDI connection without Python MIDI libraries on iPad

This Mac-side bridge receives MIDI messages from iPad Safari via WebSocket
and outputs them to Mac MIDI system (IAC Driver, etc.) which can then route
to iPad via Network Session or keep on Mac for DAW use.

Requirements:
pip install websockets mido python-rtmidi

Usage:
python3 webrtc-midi-signaler.py

Then on iPad Safari:
- Load ipad-native-midi-bridge.js
- Connect to ws://[mac-ip]:8081
"""

import asyncio
import json
import logging
import websockets
import socket
from datetime import datetime

try:
    import mido
    HAS_MIDO = True
except ImportError:
    HAS_MIDO = False
    print("⚠️ mido not available - MIDI output disabled")
    print("Install with: pip install mido python-rtmidi")

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class WebRTCMIDISignaler:
    def __init__(self, port=8082):
        self.port = port
        self.clients = {}
        self.midi_out = None
        self.message_count = 0
        self.midi_message_count = 0
        
        if HAS_MIDO:
            self.setup_midi()
        else:
            logger.warning("MIDI libraries not available - running in simulation mode")
        
        # Get Mac IP address
        self.mac_ip = self.get_local_ip()
        logger.info(f"Mac IP: {self.mac_ip}")
    
    def get_local_ip(self):
        """Get the Mac's local IP address"""
        try:
            # Connect to external address to determine local IP
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))
            local_ip = s.getsockname()[0]
            s.close()
            return local_ip
        except Exception:
            return "localhost"
    
    def setup_midi(self):
        """Setup MIDI output on Mac"""
        try:
            output_names = mido.get_output_names()
            logger.info(f"Available MIDI outputs: {output_names}")
            
            # Priority order for Mac MIDI destinations
            priority_searches = [
                (['IAC Driver Bus 1', 'IAC Driver'], "IAC Driver"),
                (['IAC'], "IAC Bus"),
                (['Network Session'], "Network Session"),
                (['Virtual'], "Virtual MIDI"),
                (['Bus'], "MIDI Bus"),
            ]
            
            selected_port = None
            for search_terms, description in priority_searches:
                matching_ports = []
                for name in output_names:
                    if any(term in name for term in search_terms):
                        matching_ports.append(name)
                
                if matching_ports:
                    selected_port = matching_ports[0]
                    logger.info(f"✅ Found {description}: {selected_port}")
                    break
            
            # Fallback to first available port
            if not selected_port and output_names:
                selected_port = output_names[0]
                logger.info(f"⚠️ Using fallback port: {selected_port}")
            
            # Connect to selected port
            if selected_port:
                self.midi_out = mido.open_output(selected_port)
                logger.info(f"✅ MIDI output connected successfully")
                
                # Test the connection
                self.test_midi_connection()
            else:
                # Try to create virtual port
                try:
                    self.midi_out = mido.open_output('WebRTC MIDI Bridge', virtual=True)
                    logger.info("✅ Created virtual MIDI port: WebRTC MIDI Bridge")
                except Exception as e:
                    logger.error(f"❌ Failed to create virtual MIDI port: {e}")
                    logger.info("💡 Enable IAC Driver in Audio MIDI Setup")
                    
        except Exception as e:
            logger.error(f"❌ MIDI setup failed: {e}")
            logger.info("💡 Make sure Audio MIDI Setup is configured properly")
    
    def test_midi_connection(self):
        """Test MIDI connection by sending a test note"""
        if not self.midi_out:
            return
            
        try:
            logger.info("🧪 Testing MIDI connection...")
            
            # Send test note on
            test_msg = mido.Message('note_on', channel=0, note=60, velocity=64)
            self.midi_out.send(test_msg)
            logger.info("✅ Test Note On sent (C4, velocity 64)")
            
            # Wait a moment
            import time
            time.sleep(0.1)
            
            # Send test note off
            test_msg = mido.Message('note_off', channel=0, note=60, velocity=0)
            self.midi_out.send(test_msg)
            logger.info("✅ Test Note Off sent")
            
            logger.info("🧪 MIDI test completed - check MIDI monitoring software")
            
        except Exception as e:
            logger.error(f"❌ MIDI test failed: {e}")
    
    async def handle_client(self, websocket, path):
        """Handle WebSocket connections from iPad"""
        client_ip = websocket.remote_address[0] if websocket.remote_address else "unknown"
        client_id = f"{client_ip}:{websocket.remote_address[1] if websocket.remote_address else 'unknown'}"
        
        self.clients[client_id] = websocket
        logger.info(f"🌐 iPad client connected: {client_ip}")
        logger.info(f"Total clients: {len(self.clients)}")
        
        try:
            async for message in websocket:
                self.message_count += 1
                try:
                    data = json.loads(message)
                    logger.info(f"📨 Received message #{self.message_count} from {client_ip}: {data.get('type', 'unknown')}")
                    await self.handle_message(client_id, data)
                except json.JSONDecodeError:
                    logger.error(f"❌ Invalid JSON from {client_ip}: {message}")
                except Exception as e:
                    logger.error(f"❌ Message error from {client_ip}: {e}")
        except websockets.exceptions.ConnectionClosed:
            logger.info(f"🔌 iPad client {client_ip} disconnected")
        finally:
            self.clients.pop(client_id, None)
    
    async def handle_message(self, client_id, data):
        """Handle messages from iPad"""
        message_type = data.get('type')
        
        if message_type == 'midi':
            # Direct MIDI message from iPad
            await self.handle_midi_message(data)
        else:
            # WebRTC signaling (if needed for future expansion)
            await self.relay_signaling(client_id, data)
    
    async def handle_midi_message(self, data):
        """Send MIDI message to Mac output"""
        self.midi_message_count += 1
        
        logger.info(f"🎵 MIDI Message #{self.midi_message_count}: {data}")
        
        if not self.midi_out and HAS_MIDO:
            logger.warning("⚠️ No MIDI output available - simulation mode")
            
        try:
            address = data.get('address', '')
            args = data.get('args', [])
            
            if address == '/midi/note' and len(args) >= 3:
                channel = max(0, min(15, args[0]['value'] - 1))  # Convert to 0-15
                note = max(0, min(127, args[1]['value']))
                velocity = max(0, min(127, args[2]['value']))
                
                logger.info(f"   Channel: {channel} (0-15) / {channel+1} (1-16)")
                logger.info(f"   Note: {note} ({self.note_name(note)})")
                logger.info(f"   Velocity: {velocity}")
                logger.info(f"   Type: {'Note On' if velocity > 0 else 'Note Off'}")
                
                if HAS_MIDO and self.midi_out:
                    if velocity > 0:
                        msg = mido.Message('note_on', channel=channel, note=note, velocity=velocity)
                        logger.info(f"📤 Sending MIDI: {msg}")
                    else:
                        msg = mido.Message('note_off', channel=channel, note=note, velocity=0)
                        logger.info(f"📤 Sending MIDI: {msg}")
                    
                    self.midi_out.send(msg)
                    logger.info("✅ MIDI message sent successfully")
                    
                else:
                    # Simulation mode
                    if velocity > 0:
                        logger.info(f"🎵 [SIMULATION] Note On: Ch{channel+1} Note{note} ({self.note_name(note)}) Vel{velocity}")
                    else:
                        logger.info(f"🎵 [SIMULATION] Note Off: Ch{channel+1} Note{note} ({self.note_name(note)})")
                        
            elif address == '/midi/cc' and len(args) >= 3:
                channel = max(0, min(15, args[0]['value'] - 1))
                control = max(0, min(127, args[1]['value']))
                value = max(0, min(127, args[2]['value']))
                
                logger.info(f"🎛️ MIDI CC: Ch{channel+1} CC{control} Val{value}")
                
                if HAS_MIDO and self.midi_out:
                    msg = mido.Message('control_change', channel=channel, control=control, value=value)
                    self.midi_out.send(msg)
                    logger.info("✅ MIDI CC sent successfully")
                else:
                    logger.info(f"🎛️ [SIMULATION] CC: Ch{channel+1} CC{control} Val{value}")
            
            else:
                logger.warning(f"❓ Unhandled MIDI message: {address}")
                
        except Exception as e:
            logger.error(f"❌ Error processing MIDI message: {e}")
            import traceback
            logger.error(f"❌ Traceback: {traceback.format_exc()}")
    
    def note_name(self, note_number):
        """Convert MIDI note number to note name"""
        note_names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
        octave = (note_number // 12) - 1
        note = note_names[note_number % 12]
        return f"{note}{octave}"
    
    async def relay_signaling(self, sender_id, data):
        """Relay WebRTC signaling between clients (for future use)"""
        for client_id, websocket in self.clients.items():
            if client_id != sender_id:
                try:
                    await websocket.send(json.dumps(data))
                except:
                    pass
    
    async def start_server(self):
        """Start the signaling server"""
        logger.info("WebRTC MIDI Signaling Server for iPad")
        logger.info("=" * 60)
        logger.info(f"🚀 Starting server on {self.mac_ip}:{self.port}")
        logger.info(f"🌐 iPad connection URL: ws://{self.mac_ip}:{self.port}")
        logger.info("")
        
        if HAS_MIDO and self.midi_out:
            logger.info("✅ MIDI Output: ACTIVE")
            logger.info(f"   Connected to: {getattr(self.midi_out, 'name', 'unknown')}")
        else:
            logger.info("⚠️  MIDI Output: SIMULATION ONLY")
            logger.info("   Install mido/python-rtmidi for actual MIDI output")
            
        logger.info("")
        logger.info("🎹 Setup Instructions:")
        logger.info("1. On iPad: Load ipad-native-midi-bridge.js in Safari")
        logger.info(f"2. Connect to: ws://{self.mac_ip}:{self.port}")
        logger.info("3. Or use standalone mode with ?standalone in URL")
        logger.info("")
        logger.info("🔄 Port Management:")
        logger.info(f"   Using port {self.port} (avoiding conflicts with 8080/8081)")
        logger.info("   If port conflicts occur, server will auto-increment")
        logger.info("")
        logger.info("🧪 Testing:")
        logger.info("- JavaScript bridge will send test notes automatically")
        logger.info("- Check console for MIDI message logs")
        logger.info("- Verify MIDI reception in GarageBand or MIDI monitor")
        logger.info("")
        logger.info("Ready! Press Ctrl+C to stop")
        logger.info("=" * 60)
        
        # Start periodic status updates
        asyncio.create_task(self.status_monitor())
        
        # Try to start server with port conflict handling
        max_port_attempts = 10
        for attempt in range(max_port_attempts):
            try:
                async with websockets.serve(self.handle_client, "0.0.0.0", self.port):
                    logger.info(f"✅ Server successfully started on port {self.port}")
                    await asyncio.Future()  # Run forever
            except OSError as e:
                if e.errno == 48:  # Address already in use
                    logger.warning(f"⚠️ Port {self.port} already in use, trying {self.port + 1}")
                    self.port += 1
                    if attempt < max_port_attempts - 1:
                        continue
                    else:
                        logger.error(f"❌ Could not find available port after {max_port_attempts} attempts")
                        raise
                else:
                    logger.error(f"❌ Server start failed: {e}")
                    raise
    
    async def status_monitor(self):
        """Periodic status monitoring"""
        while True:
            await asyncio.sleep(30)  # Every 30 seconds
            logger.info(f"📊 Status: {len(self.clients)} clients, {self.message_count} messages, {self.midi_message_count} MIDI messages")
    
    def close(self):
        """Clean up resources"""
        if self.midi_out:
            logger.info("🛑 Closing MIDI output")
            self.midi_out.close()
        logger.info("🛑 WebRTC MIDI Signaling Server closed")

def main():
    """Main entry point"""
    print("WebRTC MIDI Signaling Server for iPad")
    print("=" * 60)
    print()
    print("This server enables iPad Safari to send MIDI without")
    print("requiring Python MIDI libraries on the iPad itself.")
    print()
    print("Requirements:")
    print("🖥️  Mac with mido and python-rtmidi installed")
    print("📱 iPad with Safari (any iPad)")
    print("🌐 Both devices on same WiFi network")
    print()
    
    server = WebRTCMIDISignaler()
    
    try:
        asyncio.run(server.start_server())
    except KeyboardInterrupt:
        print("\n🛑 Stopping server...")
    finally:
        server.close()

if __name__ == "__main__":
    main()