#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
iPad MIDI Bridge - Enhanced Diagnostic Version
WebSocket to iOS Network Session MIDI with detailed diagnostics

This version provides extensive logging and diagnostics to help identify
why MIDI messages might not be reaching iOS Network Session.

Requirements:
- websockets: pip install websockets
- mido: pip install mido
- python-rtmidi: pip install python-rtmidi

Usage on iPad:
python3 ipad-midi-bridge-diagnostic.py
"""

import asyncio
import json
import logging
import websockets
import socket
import time
import platform

# Try to import MIDI libraries
try:
    import mido
    import rtmidi
    HAS_MIDO = True
    HAS_RTMIDI = True
except ImportError as e:
    HAS_MIDO = False
    HAS_RTMIDI = False
    print(f"⚠️ MIDI libraries not available: {e}")
    print("Install with: pip install mido python-rtmidi")

# Configure detailed logging
logging.basicConfig(
    level=logging.INFO, 
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler('ipad-midi-bridge.log')
    ]
)
logger = logging.getLogger(__name__)

class iPadMIDIBridgeDiagnostic:
    def __init__(self, websocket_port=8080):
        self.websocket_port = websocket_port
        self.clients = set()
        self.message_count = 0
        self.midi_message_count = 0
        
        # MIDI setup
        self.midi_out = None
        self.rtmidi_out = None
        
        if HAS_MIDO:
            self.diagnose_midi_environment()
            self.setup_midi()
        else:
            logger.warning("MIDI not available - bridge will run in simulation mode")
        
        # Get iPad's IP address
        self.ipad_ip = self.get_local_ip()
        
        logger.info("iPad MIDI Bridge Diagnostic initialized")
        logger.info(f"iPad IP: {self.ipad_ip}")
        logger.info(f"Platform: {platform.system()} {platform.release()}")
    
    def diagnose_midi_environment(self):
        """Comprehensive MIDI environment diagnosis"""
        logger.info("=== MIDI ENVIRONMENT DIAGNOSIS ===")
        
        try:
            # Check mido version
            logger.info(f"mido version: {mido.__version__}")
        except:
            logger.warning("Could not get mido version")
        
        try:
            # Check rtmidi version and backend
            if HAS_RTMIDI:
                logger.info(f"python-rtmidi available: {rtmidi.get_api_display_name(rtmidi.get_compiled_api())}")
                logger.info(f"Available APIs: {[rtmidi.get_api_display_name(api) for api in rtmidi.get_compiled_api()]}")
        except Exception as e:
            logger.warning(f"rtmidi diagnosis failed: {e}")
        
        # Test MIDI port discovery
        try:
            input_names = mido.get_input_names()
            output_names = mido.get_output_names()
            
            logger.info(f"Found {len(input_names)} MIDI input ports:")
            for i, name in enumerate(input_names):
                logger.info(f"  Input {i}: {name}")
            
            logger.info(f"Found {len(output_names)} MIDI output ports:")
            for i, name in enumerate(output_names):
                logger.info(f"  Output {i}: {name}")
                
            # Analyze port types
            network_ports = [name for name in output_names if any(term in name.lower() for term in ['network', 'session'])]
            ios_ports = [name for name in output_names if any(term in name.lower() for term in ['bluetooth', 'garageband', 'aum', 'audiobus'])]
            virtual_ports = [name for name in output_names if any(term in name.lower() for term in ['virtual', 'iac', 'bus'])]
            
            logger.info(f"Network/Session ports: {network_ports}")
            logger.info(f"iOS app ports: {ios_ports}")
            logger.info(f"Virtual ports: {virtual_ports}")
            
        except Exception as e:
            logger.error(f"MIDI port discovery failed: {e}")
        
        logger.info("=== END MIDI DIAGNOSIS ===")
    
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
        """Setup MIDI output with detailed diagnostics"""
        try:
            output_names = mido.get_output_names()
            logger.info(f"Setting up MIDI from {len(output_names)} available outputs")
            
            # Priority order for iPad MIDI destinations
            priority_searches = [
                (['Network Session 1', 'Network Session'], "iOS Network Session"),
                (['Network'], "Network MIDI"),
                (['Session'], "MIDI Session"),
                (['GarageBand'], "GarageBand"),
                (['AUM'], "AUM"),
                (['Audiobus'], "Audiobus"),
                (['Bluetooth'], "Bluetooth MIDI"),
            ]
            
            selected_port = None
            for search_terms, description in priority_searches:
                matching_ports = []
                for name in output_names:
                    if any(term.lower() in name.lower() for term in search_terms):
                        matching_ports.append(name)
                
                if matching_ports:
                    selected_port = matching_ports[0]
                    logger.info(f"✅ Found {description}: {selected_port}")
                    break
                else:
                    logger.info(f"❌ No {description} ports found")
            
            # Fallback to any available port
            if not selected_port and output_names:
                selected_port = output_names[0]
                logger.info(f"⚠️ Using fallback port: {selected_port}")
            
            # Attempt connection
            if selected_port:
                logger.info(f"Attempting to connect to: {selected_port}")
                self.midi_out = mido.open_output(selected_port)
                logger.info(f"✅ MIDI output connected successfully")
                
                # Test the connection
                self.test_midi_connection()
                
            else:
                # Try to create virtual port
                logger.info("No MIDI outputs found, attempting to create virtual port")
                try:
                    self.midi_out = mido.open_output('iPad MIDI Bridge Diagnostic', virtual=True)
                    logger.info("✅ Created virtual MIDI port: iPad MIDI Bridge Diagnostic")
                except Exception as e:
                    logger.error(f"❌ Failed to create virtual MIDI port: {e}")
                    logger.info("💡 Enable Network Session in iOS Settings > General > AirPlay & Handoff")
                    
        except Exception as e:
            logger.error(f"❌ MIDI setup failed: {e}")
            logger.info("💡 Make sure Network Session is enabled in iOS Settings")
    
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
            time.sleep(0.1)
            
            # Send test note off
            test_msg = mido.Message('note_off', channel=0, note=60, velocity=0)
            self.midi_out.send(test_msg)
            logger.info("✅ Test Note Off sent")
            
            logger.info("🧪 MIDI test completed - check iOS MIDI apps for received notes")
            
        except Exception as e:
            logger.error(f"❌ MIDI test failed: {e}")
    
    async def handle_websocket(self, websocket):
        """Handle WebSocket connections from web browser"""
        self.clients.add(websocket)
        client_ip = websocket.remote_address[0] if websocket.remote_address else "unknown"
        logger.info(f"🌐 WebSocket client connected from {client_ip}")
        logger.info(f"Total clients: {len(self.clients)}")
        
        try:
            async for message in websocket:
                self.message_count += 1
                try:
                    data = json.loads(message)
                    logger.info(f"📨 Received message #{self.message_count}: {data}")
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
        """Process incoming messages with enhanced diagnostics"""
        logger.info(f"🔍 Processing message: {data}")
        
        if not self.midi_out and HAS_MIDO:
            logger.warning("⚠️ No MIDI output available - running in simulation mode")
            
        try:
            address = data.get('address', '')
            args = data.get('args', [])
            
            if address == '/midi/note' and len(args) >= 3:
                channel = max(0, min(15, args[0]['value'] - 1))  # Convert to 0-15
                note = max(0, min(127, args[1]['value']))
                velocity = max(0, min(127, args[2]['value']))
                
                self.midi_message_count += 1
                
                logger.info(f"🎵 MIDI Note Message #{self.midi_message_count}:")
                logger.info(f"   Channel: {channel} (0-15) / {channel+1} (1-16)")
                logger.info(f"   Note: {note} ({self.note_name(note)})")
                logger.info(f"   Velocity: {velocity}")
                logger.info(f"   Type: {'Note On' if velocity > 0 else 'Note Off'}")
                
                if HAS_MIDO and self.midi_out:
                    try:
                        if velocity > 0:
                            # Note on
                            msg = mido.Message('note_on', channel=channel, note=note, velocity=velocity)
                            logger.info(f"📤 Sending MIDI: {msg}")
                        else:
                            # Note off
                            msg = mido.Message('note_off', channel=channel, note=note, velocity=0)
                            logger.info(f"📤 Sending MIDI: {msg}")
                        
                        self.midi_out.send(msg)
                        logger.info("✅ MIDI message sent successfully")
                        
                        # Additional verification
                        logger.info(f"🔍 MIDI port status: {type(self.midi_out)}")
                        logger.info(f"🔍 MIDI port name: {getattr(self.midi_out, 'name', 'unknown')}")
                        
                    except Exception as e:
                        logger.error(f"❌ MIDI send failed: {e}")
                        logger.error(f"❌ Exception type: {type(e)}")
                        
                else:
                    # Simulation mode
                    if velocity > 0:
                        logger.info(f"🎵 [SIMULATION] Note On: Ch{channel+1} Note{note} ({self.note_name(note)}) Vel{velocity}")
                    else:
                        logger.info(f"🎵 [SIMULATION] Note Off: Ch{channel+1} Note{note} ({self.note_name(note)})")
                
            elif address == '/midi/cc' and len(args) >= 3:
                # Control change
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
                logger.warning(f"❓ Unhandled message: {address}")
                
        except Exception as e:
            logger.error(f"❌ Error processing message: {e}")
            logger.error(f"❌ Exception type: {type(e)}")
            import traceback
            logger.error(f"❌ Traceback: {traceback.format_exc()}")
    
    def note_name(self, note_number):
        """Convert MIDI note number to note name"""
        note_names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
        octave = (note_number // 12) - 1
        note = note_names[note_number % 12]
        return f"{note}{octave}"
    
    async def start_server(self):
        """Start WebSocket server with diagnostics"""
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
            logger.info("iPad MIDI Bridge - DIAGNOSTIC MODE")
            logger.info("=" * 60)
            logger.info(f"📱 iPad IP Address: {self.ipad_ip}")
            logger.info(f"🌐 WebSocket URL: ws://{self.ipad_ip}:{self.websocket_port}")
            logger.info(f"🐍 Platform: {platform.system()} {platform.release()}")
            logger.info("")
            
            if HAS_MIDO and self.midi_out:
                logger.info("✅ MIDI Output: ACTIVE")
                logger.info(f"   Connected to: {getattr(self.midi_out, 'name', 'unknown')}")
            else:
                logger.info("⚠️  MIDI Output: SIMULATION ONLY")
                
            logger.info("")
            logger.info("🎹 iOS Network Session Setup:")
            logger.info("1. Open iOS Settings")
            logger.info("2. Go to General > AirPlay & Handoff")
            logger.info("3. Enable 'Network MIDI'")
            logger.info("4. Or: Settings > MIDI > Enable Network Session")
            logger.info("5. Verify 'Network Session 1' appears in MIDI apps")
            logger.info("")
            logger.info("🧪 Test with iOS MIDI Apps:")
            logger.info("- GarageBand: Check if it receives MIDI")
            logger.info("- AUM: Monitor MIDI input")
            logger.info("- MIDI apps: Look for 'Network Session 1' input")
            logger.info("")
            logger.info("📊 Monitoring:")
            logger.info("- All WebSocket messages will be logged")
            logger.info("- MIDI send attempts will be detailed")
            logger.info("- Errors will include full traceback")
            logger.info(f"- Log file: ipad-midi-bridge.log")
            logger.info("")
            logger.info("Ready! Press Ctrl+C to stop")
            logger.info("=" * 60)
            
            # Periodic status updates
            asyncio.create_task(self.status_monitor())
            
            # Keep running
            await asyncio.Future()
    
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
        logger.info("🛑 iPad MIDI Bridge Diagnostic closed")

def print_intro():
    """Print introduction and setup instructions"""
    print("iPad MIDI Bridge - DIAGNOSTIC VERSION")
    print("=" * 60)
    print()
    print("This enhanced version provides detailed diagnostics to help")
    print("identify why MIDI messages might not reach iOS Network Session.")
    print()
    print("Features:")
    print("✅ Comprehensive MIDI environment diagnosis")
    print("✅ Detailed message logging and tracing")
    print("✅ MIDI connection testing")
    print("✅ Real-time status monitoring")
    print("✅ Enhanced error reporting")
    print("✅ Log file output for analysis")
    print()
    print("Requirements:")
    print("📱 iPad/iPhone with iOS 12+")
    print("🐍 Python 3.7+ with websockets, mido, python-rtmidi")
    print("🎹 iOS Network Session enabled in Settings")
    print()

def main():
    """Main entry point"""
    print_intro()
    
    bridge = iPadMIDIBridgeDiagnostic()
    
    try:
        asyncio.run(bridge.start_server())
    except KeyboardInterrupt:
        print("\n🛑 Stopping iPad MIDI bridge diagnostic...")
    finally:
        bridge.close()

if __name__ == "__main__":
    main()