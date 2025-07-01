#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Enhanced MIDI Bridge - WebSocket to MIDI/OSC bridge for Mac
Supports both MIDI output (IAC Driver) and OSC output (TouchOSC)

Features:
- MIDI output to IAC Driver for DAWs
- OSC output to TouchOSC (local or network)
- Dual output mode (both MIDI and OSC simultaneously)
- WebSocket interface for web browsers

Requirements:
pip install websockets mido python-rtmidi python-osc

Usage:
python3 enhanced-midi-bridge.py
"""

import asyncio
import json
import logging
import websockets
import argparse

try:
    import mido
    HAS_MIDO = True
except ImportError:
    HAS_MIDO = False
    print("mido not available - MIDI output disabled")

try:
    from pythonosc import udp_client
    HAS_OSC = True
except ImportError:
    HAS_OSC = False
    print("python-osc not available - OSC output disabled")

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class EnhancedMIDIBridge:
    def __init__(self, websocket_port=8080, touchosc_host='localhost', touchosc_port=8000, enable_midi=True, enable_osc=True):
        self.websocket_port = websocket_port
        self.touchosc_host = touchosc_host
        self.touchosc_port = touchosc_port
        self.enable_midi = enable_midi and HAS_MIDO
        self.enable_osc = enable_osc and HAS_OSC
        self.clients = set()
        
        # MIDI setup
        self.midi_out = None
        if self.enable_midi:
            self.setup_midi()
        
        # OSC setup
        self.osc_client = None
        if self.enable_osc:
            self.setup_osc()
        
        logger.info(f"Enhanced MIDI Bridge initialized")
        logger.info(f"MIDI output: {'enabled' if self.enable_midi else 'disabled'}")
        logger.info(f"OSC output: {'enabled' if self.enable_osc else 'disabled'}")
    
    def setup_midi(self):
        """Setup MIDI output"""
        try:
            output_names = mido.get_output_names()
            logger.info(f"Available MIDI outputs: {output_names}")
            
            # Look for IAC Driver first
            iac_ports = [name for name in output_names if 'IAC' in name or 'Bus' in name]
            if iac_ports:
                self.midi_out = mido.open_output(iac_ports[0])
                logger.info(f"MIDI connected to IAC: {iac_ports[0]}")
            elif output_names:
                self.midi_out = mido.open_output(output_names[0])
                logger.info(f"MIDI connected to: {output_names[0]}")
            else:
                # Create virtual port
                self.midi_out = mido.open_output('Enhanced MIDI Bridge', virtual=True)
                logger.info("Created virtual MIDI port: Enhanced MIDI Bridge")
        except Exception as e:
            logger.error(f"MIDI setup failed: {e}")
            self.enable_midi = False
    
    def setup_osc(self):
        """Setup OSC output"""
        try:
            self.osc_client = udp_client.SimpleUDPClient(self.touchosc_host, self.touchosc_port)
            logger.info(f"OSC client ready for {self.touchosc_host}:{self.touchosc_port}")
        except Exception as e:
            logger.error(f"OSC setup failed: {e}")
            self.enable_osc = False
    
    async def handle_websocket(self, websocket):
        """Handle WebSocket connections"""
        self.clients.add(websocket)
        client_ip = websocket.remote_address[0] if websocket.remote_address else "unknown"
        logger.info(f"WebSocket client connected from {client_ip}")
        
        try:
            async for message in websocket:
                try:
                    data = json.loads(message)
                    await self.handle_message(data)
                except json.JSONDecodeError:
                    logger.error(f"Invalid JSON: {message}")
                except Exception as e:
                    logger.error(f"Message error: {e}")
        except websockets.exceptions.ConnectionClosed:
            logger.info(f"WebSocket client {client_ip} disconnected")
        finally:
            self.clients.discard(websocket)
    
    async def handle_message(self, data):
        """Process incoming messages"""
        try:
            address = data.get('address', '')
            args = data.get('args', [])
            
            if address == '/midi/note' and len(args) >= 3:
                channel = max(0, min(15, args[0]['value'] - 1))  # Convert to 0-15
                note = max(0, min(127, args[1]['value']))
                velocity = max(0, min(127, args[2]['value']))
                
                # Send MIDI
                if self.enable_midi:
                    self.send_midi_note(channel, note, velocity)
                
                # Send OSC for TouchOSC
                if self.enable_osc:
                    await self.send_osc_note(channel, note, velocity)
                
            elif address == '/midi/cc' and len(args) >= 3:
                channel = max(0, min(15, args[0]['value'] - 1))
                control = max(0, min(127, args[1]['value']))
                value = max(0, min(127, args[2]['value']))
                
                # Send MIDI CC
                if self.enable_midi:
                    self.send_midi_cc(channel, control, value)
                
                # Send OSC CC
                if self.enable_osc:
                    await self.send_osc_cc(channel, control, value)
            
            else:
                logger.warning(f"Unhandled message: {address}")
                
        except Exception as e:
            logger.error(f"Error processing message: {e}")
    
    def send_midi_note(self, channel, note, velocity):
        """Send MIDI note"""
        if not self.midi_out:
            return
            
        try:
            if velocity > 0:
                msg = mido.Message('note_on', channel=channel, note=note, velocity=velocity)
                logger.info(f"MIDI Note On: Ch{channel+1} Note{note} Vel{velocity}")
            else:
                msg = mido.Message('note_off', channel=channel, note=note, velocity=0)
                logger.info(f"MIDI Note Off: Ch{channel+1} Note{note}")
            
            self.midi_out.send(msg)
        except Exception as e:
            logger.error(f"MIDI send error: {e}")
    
    def send_midi_cc(self, channel, control, value):
        """Send MIDI control change"""
        if not self.midi_out:
            return
            
        try:
            msg = mido.Message('control_change', channel=channel, control=control, value=value)
            self.midi_out.send(msg)
            logger.info(f"MIDI CC: Ch{channel+1} CC{control} Val{value}")
        except Exception as e:
            logger.error(f"MIDI CC error: {e}")
    
    async def send_osc_note(self, channel, note, velocity):
        """Send OSC note for TouchOSC"""
        if not self.osc_client:
            return
        
        try:
            if velocity > 0:
                # Note on - multiple TouchOSC formats
                self.osc_client.send_message('/note/on', [channel + 1, note, velocity])
                self.osc_client.send_message(f'/1/push{(note % 16) + 1}', 1.0)  # TouchOSC button
                self.osc_client.send_message('/trigger', note)
                logger.info(f"OSC Note On: Ch{channel+1} Note{note} Vel{velocity}")
            else:
                # Note off
                self.osc_client.send_message('/note/off', [channel + 1, note])
                self.osc_client.send_message(f'/1/push{(note % 16) + 1}', 0.0)  # TouchOSC button
                logger.info(f"OSC Note Off: Ch{channel+1} Note{note}")
                
        except Exception as e:
            logger.error(f"OSC send error: {e}")
    
    async def send_osc_cc(self, channel, control, value):
        """Send OSC control change for TouchOSC"""
        if not self.osc_client:
            return
        
        try:
            # Send control change
            self.osc_client.send_message('/cc', [channel + 1, control, value])
            self.osc_client.send_message(f'/1/fader{(control % 8) + 1}', value / 127.0)  # TouchOSC fader
            logger.info(f"OSC CC: Ch{channel+1} CC{control} Val{value}")
        except Exception as e:
            logger.error(f"OSC CC error: {e}")
    
    async def start_server(self):
        """Start WebSocket server"""
        logger.info(f"Starting WebSocket server on port {self.websocket_port}")
        
        async with websockets.serve(
            self.handle_websocket,
            "localhost",
            self.websocket_port,
            ping_interval=30,
            ping_timeout=10
        ):
            logger.info("WebSocket server ready on ws://localhost:{}".format(self.websocket_port))
            logger.info("")
            logger.info("Configuration:")
            logger.info("=" * 40)
            if self.enable_midi:
                logger.info("✅ MIDI output: Enabled (for DAWs via IAC Driver)")
            else:
                logger.info("❌ MIDI output: Disabled")
            
            if self.enable_osc:
                logger.info(f"✅ OSC output: Enabled (TouchOSC at {self.touchosc_host}:{self.touchosc_port})")
            else:
                logger.info("❌ OSC output: Disabled")
            
            logger.info("")
            logger.info("TouchOSC Setup (if OSC enabled):")
            logger.info("- Connection: UDP Server")
            logger.info(f"- Incoming Port: {self.touchosc_port}")
            logger.info("- Outgoing Port: 9000")
            logger.info("- Enable OSC")
            logger.info("")
            logger.info("Ready! Press Ctrl+C to stop")
            logger.info("=" * 40)
            
            # Keep running
            await asyncio.Future()
    
    def close(self):
        """Clean up"""
        if self.midi_out:
            self.midi_out.close()
        logger.info("Bridge closed")

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description='Enhanced MIDI Bridge - WebSocket to MIDI/OSC')
    parser.add_argument('--port', type=int, default=8080, help='WebSocket port (default: 8080)')
    parser.add_argument('--touchosc-host', default='localhost', help='TouchOSC host (default: localhost)')
    parser.add_argument('--touchosc-port', type=int, default=8000, help='TouchOSC port (default: 8000)')
    parser.add_argument('--no-midi', action='store_true', help='Disable MIDI output')
    parser.add_argument('--no-osc', action='store_true', help='Disable OSC output')
    
    args = parser.parse_args()
    
    print("Enhanced MIDI Bridge")
    print("=" * 40)
    print("Supports both MIDI (IAC Driver) and OSC (TouchOSC) output")
    print()
    
    bridge = EnhancedMIDIBridge(
        websocket_port=args.port,
        touchosc_host=args.touchosc_host,
        touchosc_port=args.touchosc_port,
        enable_midi=not args.no_midi,
        enable_osc=not args.no_osc
    )
    
    try:
        asyncio.run(bridge.start_server())
    except KeyboardInterrupt:
        print("\nStopping bridge...")
    finally:
        bridge.close()

if __name__ == "__main__":
    main()