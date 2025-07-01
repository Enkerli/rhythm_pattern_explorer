#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
iPad TouchOSC Bridge - WebSocket to TouchOSC OSC bridge for iPad
Receives WebSocket messages from browser and sends OSC to TouchOSC

For iPad/iOS usage:
1. Install Pythonista 3 or a-Shell on iPad
2. Copy this script to the app
3. Run the script
4. Configure TouchOSC to connect to this iPad's IP address

Requirements (install in Pythonista or a-Shell):
- websockets: pip install websockets
- python-osc: pip install python-osc

Usage:
python3 ipad-touchosc-bridge.py
"""

import asyncio
import json
import logging
import websockets
import socket
import struct
from datetime import datetime

# Try to import OSC libraries with fallback
try:
    from pythonosc import udp_client
    HAS_PYTHONOSC = True
except ImportError:
    HAS_PYTHONOSC = False
    print("python-osc not available, using built-in OSC encoding")

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class SimpleOSCEncoder:
    """Simple OSC message encoder for when python-osc is not available"""
    
    @staticmethod
    def encode_string(s):
        """Encode string with null termination and 4-byte padding"""
        data = s.encode('utf-8') + b'\x00'
        padding = (4 - (len(data) % 4)) % 4
        return data + b'\x00' * padding
    
    @staticmethod
    def encode_int(i):
        """Encode 32-bit big-endian integer"""
        return struct.pack('>i', i)
    
    @staticmethod
    def encode_float(f):
        """Encode 32-bit big-endian float"""
        return struct.pack('>f', f)
    
    @classmethod
    def create_message(cls, address, args=None):
        """Create OSC message"""
        if args is None:
            args = []
        
        # Address pattern
        addr_data = cls.encode_string(address)
        
        # Type tag string
        type_tag = ','
        for arg in args:
            if isinstance(arg, int):
                type_tag += 'i'
            elif isinstance(arg, float):
                type_tag += 'f'
            else:
                type_tag += 's'
        
        type_data = cls.encode_string(type_tag)
        
        # Arguments
        arg_data = b''
        for arg in args:
            if isinstance(arg, int):
                arg_data += cls.encode_int(arg)
            elif isinstance(arg, float):
                arg_data += cls.encode_float(arg)
            else:
                arg_data += cls.encode_string(str(arg))
        
        return addr_data + type_data + arg_data

class iPadTouchOSCBridge:
    def __init__(self, websocket_port=8080, touchosc_host='127.0.0.1', touchosc_port=8000):
        self.websocket_port = websocket_port
        self.touchosc_host = touchosc_host
        self.touchosc_port = touchosc_port
        self.clients = set()
        
        # Create UDP socket for OSC
        self.osc_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        # Set up OSC client
        if HAS_PYTHONOSC:
            self.osc_client = udp_client.SimpleUDPClient(touchosc_host, touchosc_port)
            logger.info("Using python-osc for OSC messages")
        else:
            self.osc_client = None
            logger.info("Using built-in OSC encoder")
        
        # Get iPad's IP address
        self.ipad_ip = self.get_local_ip()
        
        logger.info("iPad TouchOSC Bridge initialized")
        logger.info(f"WebSocket server will run on: {self.ipad_ip}:{websocket_port}")
        logger.info(f"OSC target: {touchosc_host}:{touchosc_port}")
    
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
    
    async def handle_websocket(self, websocket):
        """Handle WebSocket connections from web browser"""
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
        """Process incoming messages from web browser"""
        try:
            address = data.get('address', '')
            args = data.get('args', [])
            
            if address == '/midi/note' and len(args) >= 3:
                channel = max(0, min(15, args[0]['value'] - 1))  # Convert to 0-15
                note = max(0, min(127, args[1]['value']))
                velocity = max(0, min(127, args[2]['value']))
                
                # Send OSC messages for TouchOSC
                if velocity > 0:
                    # Note on
                    await self.send_osc('/note/on', [channel + 1, note, velocity])
                    await self.send_osc(f'/1/push{(note % 16) + 1}', [1.0])  # TouchOSC button
                    await self.send_osc('/trigger', [note])
                    logger.info(f"Note On: Ch{channel+1} Note{note} Vel{velocity}")
                else:
                    # Note off
                    await self.send_osc('/note/off', [channel + 1, note])
                    await self.send_osc(f'/1/push{(note % 16) + 1}', [0.0])  # TouchOSC button
                    logger.info(f"Note Off: Ch{channel+1} Note{note}")
                
            elif address == '/midi/cc' and len(args) >= 3:
                channel = max(0, min(15, args[0]['value'] - 1))
                control = max(0, min(127, args[1]['value']))
                value = max(0, min(127, args[2]['value']))
                
                await self.send_osc('/cc', [channel + 1, control, value])
                await self.send_osc(f'/1/fader{(control % 8) + 1}', [value / 127.0])  # TouchOSC fader
                logger.info(f"CC: Ch{channel+1} CC{control} Val{value}")
            
            else:
                logger.warning(f"Unhandled message: {address}")
                
        except Exception as e:
            logger.error(f"Error processing message: {e}")
    
    async def send_osc(self, address, args=None):
        """Send OSC message to TouchOSC"""
        if args is None:
            args = []
        
        try:
            if HAS_PYTHONOSC and self.osc_client:
                # Use python-osc
                if len(args) == 0:
                    self.osc_client.send_message(address, None)
                else:
                    self.osc_client.send_message(address, args)
            else:
                # Use built-in encoder
                message = SimpleOSCEncoder.create_message(address, args)
                self.osc_socket.sendto(message, (self.touchosc_host, self.touchosc_port))
            
            # logger.info(f"OSC sent: {address} {args}")
            
        except Exception as e:
            logger.error(f"OSC send error: {e}")
    
    async def start_server(self):
        """Start WebSocket server"""
        logger.info(f"Starting WebSocket server on {self.ipad_ip}:{self.websocket_port}")
        
        async with websockets.serve(
            self.handle_websocket,
            "0.0.0.0",  # Listen on all interfaces
            self.websocket_port,
            ping_interval=30,
            ping_timeout=10
        ):
            logger.info("WebSocket server ready")
            logger.info("")
            logger.info("Configuration Instructions:")
            logger.info("=" * 50)
            logger.info("1. TouchOSC Setup:")
            logger.info("   - Connection: UDP Server")
            logger.info(f"   - Incoming Port: {self.touchosc_port}")
            logger.info("   - Outgoing Port: 9000")
            logger.info("   - Enable OSC")
            logger.info("")
            logger.info("2. Web Browser Setup:")
            logger.info(f"   - Update MIDI output settings to use: ws://{self.ipad_ip}:{self.websocket_port}")
            logger.info(f"   - Or run on same device: ws://localhost:{self.websocket_port}")
            logger.info("")
            logger.info("3. Network:")
            logger.info(f"   - iPad IP: {self.ipad_ip}")
            logger.info("   - Make sure devices are on same WiFi network")
            logger.info("")
            logger.info("Ready! Press Ctrl+C to stop")
            logger.info("=" * 50)
            
            # Keep running
            await asyncio.Future()
    
    def close(self):
        """Clean up resources"""
        if self.osc_socket:
            self.osc_socket.close()
        logger.info("Bridge closed")

def print_intro():
    """Print introduction and setup instructions"""
    print("iPad TouchOSC Bridge")
    print("=" * 40)
    print()
    print("This script creates a bridge between web browsers and TouchOSC on iPad.")
    print("It receives MIDI messages via WebSocket and forwards them as OSC to TouchOSC.")
    print()
    print("Setup Requirements:")
    print("1. iPad and computer must be on same WiFi network")
    print("2. TouchOSC app installed on iPad")
    print("3. Python environment with websockets (pip install websockets)")
    print("4. Optional: python-osc for better OSC support (pip install python-osc)")
    print()

def main():
    """Main entry point"""
    print_intro()
    
    # Check if we're likely running on iPad/iOS
    import platform
    system = platform.system()
    if system == "Darwin" and "iPad" in platform.platform():
        print("Detected iPad/iOS environment")
    elif system == "Darwin":
        print("Detected macOS environment")
    else:
        print(f"Detected {system} environment")
    
    print()
    
    bridge = iPadTouchOSCBridge()
    
    try:
        asyncio.run(bridge.start_server())
    except KeyboardInterrupt:
        print("\nStopping bridge...")
    finally:
        bridge.close()

if __name__ == "__main__":
    main()