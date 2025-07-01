#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
iPad MIDI Bridge - WebSocket to iOS Network Session MIDI
True equivalent of minimal-midi-bridge.py for iPad/iOS

Sends MIDI to iOS Network Session (equivalent to IAC Driver on Mac)
Works with iPadOS built-in MIDI networking and apps like GarageBand, AUM, etc.

For iPad/iOS usage:
1. Install Pythonista 3 or a-Shell
2. Enable Network Session in iOS Settings > MIDI
3. Run this script
4. Connect from web browser

Requirements:
- websockets: pip install websockets
- mido: pip install mido
- python-rtmidi: pip install python-rtmidi

Usage on iPad:
python3 ipad-midi-bridge.py
"""

import asyncio
import json
import logging
import websockets
import socket

# Try to import MIDI libraries
try:
    import mido
    HAS_MIDO = True
except ImportError:
    HAS_MIDO = False
    print("⚠️ mido not available")
    print("Install with: pip install mido python-rtmidi")
    print("Note: python-rtmidi may need compilation on iOS")

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class iPadMIDIBridge:
    def __init__(self, websocket_port=8080):
        self.websocket_port = websocket_port
        self.clients = set()
        
        # MIDI setup
        self.midi_out = None
        if HAS_MIDO:
            self.setup_midi()
        else:
            logger.warning("MIDI not available - bridge will run in simulation mode")
        
        # Get iPad's IP address
        self.ipad_ip = self.get_local_ip()
        
        logger.info("iPad MIDI Bridge initialized")
        logger.info(f"iPad IP: {self.ipad_ip}")
    
    def get_local_ip(self):
        """Get the iPad's local IP address"""
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
        """Setup MIDI output to iOS Network Session"""
        try:
            output_names = mido.get_output_names()
            logger.info(f"Available MIDI outputs: {output_names}")
            
            # Look for Network Session first (iOS equivalent of IAC Driver)
            network_ports = [name for name in output_names if 'Network' in name or 'Session' in name]
            
            # Look for other common iOS MIDI destinations
            ios_ports = [name for name in output_names if any(term in name.lower() for term in 
                        ['network', 'session', 'bluetooth', 'garageband', 'aum', 'audiobus'])]
            
            if network_ports:
                self.midi_out = mido.open_output(network_ports[0])
                logger.info(f"✅ MIDI connected to Network Session: {network_ports[0]}")
            elif ios_ports:
                self.midi_out = mido.open_output(ios_ports[0])
                logger.info(f"✅ MIDI connected to iOS app: {ios_ports[0]}")
            elif output_names:
                self.midi_out = mido.open_output(output_names[0])
                logger.info(f"✅ MIDI connected to: {output_names[0]}")
            else:
                # Try to create virtual port
                try:
                    self.midi_out = mido.open_output('iPad MIDI Bridge', virtual=True)
                    logger.info("✅ Created virtual MIDI port: iPad MIDI Bridge")
                except:
                    logger.warning("❌ No MIDI outputs available and cannot create virtual port")
                    logger.info("💡 Enable Network Session in iOS Settings > MIDI")
                    logger.info("💡 Or connect to a MIDI app like GarageBand")
                    
        except Exception as e:
            logger.error(f"❌ MIDI setup failed: {e}")
            logger.info("💡 Make sure Network Session is enabled in iOS Settings")
    
    async def handle_websocket(self, websocket):
        """Handle WebSocket connections from web browser"""
        self.clients.add(websocket)
        client_ip = websocket.remote_address[0] if websocket.remote_address else "unknown"
        logger.info(f"🌐 WebSocket client connected from {client_ip}")
        
        try:
            async for message in websocket:
                try:
                    data = json.loads(message)
                    await self.handle_message(data)
                except json.JSONDecodeError:
                    logger.error(f"❌ Invalid JSON: {message}")
                except Exception as e:
                    logger.error(f"❌ Message error: {e}")
        except websockets.exceptions.ConnectionClosed:
            logger.info(f"🔌 WebSocket client {client_ip} disconnected")
        finally:
            self.clients.discard(websocket)
    
    async def handle_message(self, data):
        """Process incoming messages - exactly like minimal-midi-bridge.py"""
        if not self.midi_out and HAS_MIDO:
            logger.warning("⚠️ No MIDI output available")
            return
            
        try:
            address = data.get('address', '')
            args = data.get('args', [])
            
            if address == '/midi/note' and len(args) >= 3:
                channel = max(0, min(15, args[0]['value'] - 1))  # Convert to 0-15
                note = max(0, min(127, args[1]['value']))
                velocity = max(0, min(127, args[2]['value']))
                
                if HAS_MIDO and self.midi_out:
                    if velocity > 0:
                        # Note on
                        msg = mido.Message('note_on', channel=channel, note=note, velocity=velocity)
                        logger.info(f"🎵 MIDI Note On: Ch{channel+1} Note{note} Vel{velocity}")
                    else:
                        # Note off
                        msg = mido.Message('note_off', channel=channel, note=note, velocity=0)
                        logger.info(f"🎵 MIDI Note Off: Ch{channel+1} Note{note}")
                    
                    self.midi_out.send(msg)
                else:
                    # Simulation mode
                    if velocity > 0:
                        logger.info(f"🎵 [SIM] Note On: Ch{channel+1} Note{note} Vel{velocity}")
                    else:
                        logger.info(f"🎵 [SIM] Note Off: Ch{channel+1} Note{note}")
                
            elif address == '/midi/cc' and len(args) >= 3:
                # Control change
                channel = max(0, min(15, args[0]['value'] - 1))
                control = max(0, min(127, args[1]['value']))
                value = max(0, min(127, args[2]['value']))
                
                if HAS_MIDO and self.midi_out:
                    msg = mido.Message('control_change', channel=channel, control=control, value=value)
                    self.midi_out.send(msg)
                    logger.info(f"🎛️ MIDI CC: Ch{channel+1} CC{control} Val{value}")
                else:
                    logger.info(f"🎛️ [SIM] CC: Ch{channel+1} CC{control} Val{value}")
            
            else:
                logger.warning(f"❓ Unhandled message: {address}")
                
        except Exception as e:
            logger.error(f"❌ Error processing message: {e}")
    
    async def start_server(self):
        """Start WebSocket server"""
        logger.info(f"🚀 Starting WebSocket server on {self.ipad_ip}:{self.websocket_port}")
        
        async with websockets.serve(
            self.handle_websocket,
            "0.0.0.0",  # Listen on all interfaces
            self.websocket_port,
            ping_interval=30,
            ping_timeout=10
        ):
            logger.info("✅ WebSocket server ready")
            logger.info("")
            logger.info("iPad MIDI Bridge Configuration")
            logger.info("=" * 50)
            logger.info(f"📱 iPad IP Address: {self.ipad_ip}")
            logger.info(f"🌐 WebSocket URL: ws://{self.ipad_ip}:{self.websocket_port}")
            logger.info("")
            logger.info("🎹 iOS MIDI Setup:")
            logger.info("1. Open iOS Settings > General > AirPlay & Handoff")
            logger.info("2. Enable 'Network MIDI' or go to Settings > MIDI")
            logger.info("3. Enable 'Network Session'")
            logger.info("4. Or open a MIDI app (GarageBand, AUM, etc.)")
            logger.info("")
            logger.info("🌐 Web Browser Setup:")
            logger.info("1. Make sure devices are on same WiFi network")
            if self.ipad_ip != "localhost":
                logger.info(f"2. In webapp MIDI settings, use: ws://{self.ipad_ip}:{self.websocket_port}")
            logger.info("3. Note: Some browsers block local network connections")
            logger.info("")
            if HAS_MIDO and self.midi_out:
                logger.info("✅ MIDI output ready")
            else:
                logger.info("⚠️  MIDI running in simulation mode")
                logger.info("   Install mido/python-rtmidi for actual MIDI output")
            logger.info("")
            logger.info("Ready! Press Ctrl+C to stop")
            logger.info("=" * 50)
            
            # Keep running
            await asyncio.Future()
    
    def close(self):
        """Clean up resources"""
        if self.midi_out:
            self.midi_out.close()
        logger.info("🛑 iPad MIDI Bridge closed")

def print_intro():
    """Print introduction and setup instructions"""
    print("iPad MIDI Bridge")
    print("=" * 40)
    print()
    print("This script creates a WebSocket to MIDI bridge for iPad/iOS.")
    print("It's the iPad equivalent of minimal-midi-bridge.py")
    print()
    print("Features:")
    print("✅ WebSocket server for web browser connections")
    print("✅ MIDI output to iOS Network Session (like IAC Driver)")
    print("✅ Compatible with GarageBand, AUM, and other iOS MIDI apps")
    print("✅ Cross-platform - works with any device on same WiFi")
    print()
    print("Requirements:")
    print("📱 iPad/iPhone with iOS 12+ ")
    print("🐍 Python environment (Pythonista 3 or a-Shell)")
    print("📦 websockets library: pip install websockets")
    print("🎵 mido library: pip install mido python-rtmidi (optional)")
    print()

def main():
    """Main entry point"""
    print_intro()
    
    # Check platform
    import platform
    system = platform.system()
    logger.info(f"Detected platform: {system}")
    
    bridge = iPadMIDIBridge()
    
    try:
        asyncio.run(bridge.start_server())
    except KeyboardInterrupt:
        print("\n🛑 Stopping iPad MIDI bridge...")
    finally:
        bridge.close()

if __name__ == "__main__":
    main()