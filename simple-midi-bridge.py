#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Simple MIDI Bridge Server (Python 2.7+ compatible)
WebSocket to MIDI bridge for Safari and Firefox MIDI output

Requirements:
pip install tornado rtmidi

Usage:
python simple-midi-bridge.py

Then in the web app, connect to ws://localhost:8080
"""

import json
import logging
import threading
import time
import sys

try:
    import tornado.ioloop
    import tornado.web
    import tornado.websocket
    import rtmidi
except ImportError as e:
    print("Missing dependencies. Please install:")
    print("pip install tornado rtmidi")
    sys.exit(1)

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class MIDIWebSocketHandler(tornado.websocket.WebSocketHandler):
    """WebSocket handler for MIDI messages"""
    
    def initialize(self, midi_bridge):
        self.midi_bridge = midi_bridge
    
    def check_origin(self, origin):
        """Allow connections from any origin"""
        return True
    
    def open(self):
        """Called when WebSocket connection is opened"""
        logger.info("WebSocket client connected from {}".format(self.request.remote_ip))
        self.midi_bridge.add_client(self)
    
    def on_message(self, message):
        """Handle incoming WebSocket messages"""
        try:
            data = json.loads(message)
            self.midi_bridge.handle_osc_message(data)
        except ValueError:
            logger.error("Invalid JSON received: {}".format(message))
        except Exception as e:
            logger.error("Error handling message: {}".format(e))
    
    def on_close(self):
        """Called when WebSocket connection is closed"""
        logger.info("WebSocket client disconnected")
        self.midi_bridge.remove_client(self)

class SimpleMIDIBridge:
    """Simple MIDI bridge using Tornado WebSocket server"""
    
    def __init__(self, port=8080):
        self.port = port
        self.clients = set()
        
        # Initialize MIDI
        self.midi_out = rtmidi.RtMidiOut()
        
        # Get available ports
        port_count = self.midi_out.getPortCount()
        self.midi_ports = []
        for i in range(port_count):
            self.midi_ports.append(self.midi_out.getPortName(i))
        
        if self.midi_ports:
            self.midi_out.openPort(0)
            logger.info("MIDI output connected to: {}".format(self.midi_ports[0]))
        else:
            self.midi_out.openVirtualPort("MIDI Bridge")
            logger.info("Created virtual MIDI port: MIDI Bridge")
    
    def add_client(self, client):
        """Add WebSocket client"""
        self.clients.add(client)
    
    def remove_client(self, client):
        """Remove WebSocket client"""
        self.clients.discard(client)
    
    def handle_osc_message(self, data):
        """Convert OSC-style message to MIDI"""
        try:
            address = data.get('address', '')
            args = data.get('args', [])
            
            if address == '/midi/note' and len(args) >= 3:
                channel = args[0]['value'] - 1  # Convert to 0-15
                note = args[1]['value']
                velocity = args[2]['value']
                
                # Ensure values are in valid MIDI range
                channel = max(0, min(15, channel))
                note = max(0, min(127, note))
                velocity = max(0, min(127, velocity))
                
                if velocity > 0:
                    # Note on
                    status = 0x90 + channel
                    logger.info("Note On: Ch{} Note{} Vel{}".format(channel+1, note, velocity))
                else:
                    # Note off
                    status = 0x80 + channel
                    velocity = 0
                    logger.info("Note Off: Ch{} Note{}".format(channel+1, note))
                
                # Send MIDI message
                try:
                    message_bytes = bytes([status, note, velocity])
                    self.midi_out.sendMessage(message_bytes)
                except Exception as e1:
                    try:
                        message_array = bytearray([status, note, velocity])
                        self.midi_out.sendMessage(message_array)
                    except Exception as e2:
                        try:
                            self.midi_out.sendMessage([status, note, velocity])
                        except Exception as e3:
                            logger.error("Failed to send MIDI: {} / {} / {}".format(e1, e2, e3))
                
            elif address == '/midi/cc' and len(args) >= 3:
                # Control change support
                channel = args[0]['value'] - 1
                controller = args[1]['value']
                value = args[2]['value']
                
                # Ensure values are in valid MIDI range
                channel = max(0, min(15, channel))
                controller = max(0, min(127, controller))
                value = max(0, min(127, value))
                
                status = 0xB0 + channel
                logger.info("CC: Ch{} CC{} Val{}".format(channel+1, controller, value))
                
                try:
                    message_bytes = bytes([status, controller, value])
                    self.midi_out.sendMessage(message_bytes)
                except Exception as e1:
                    try:
                        message_array = bytearray([status, controller, value])
                        self.midi_out.sendMessage(message_array)
                    except Exception as e2:
                        try:
                            self.midi_out.sendMessage([status, controller, value])
                        except Exception as e3:
                            logger.error("Failed to send MIDI CC: {} / {} / {}".format(e1, e2, e3))
            
            else:
                logger.warning("Unhandled OSC message: {}".format(address))
                
        except Exception as e:
            logger.error("Error processing OSC message: {}".format(e))
    
    def list_midi_ports(self):
        """List available MIDI ports"""
        logger.info("Available MIDI ports:")
        if self.midi_ports:
            for i, port in enumerate(self.midi_ports):
                logger.info("  {}: {}".format(i, port))
        else:
            logger.info("  No MIDI ports found - using virtual port")
        return self.midi_ports
    
    def start_server(self):
        """Start the WebSocket server"""
        app = tornado.web.Application([
            (r"/", MIDIWebSocketHandler, {"midi_bridge": self}),
        ])
        
        app.listen(self.port)
        logger.info("WebSocket server started on port {}".format(self.port))
        
        print("""
WebSocket server running on ws://localhost:{}
MIDI output ready
Configure your web app to connect to: ws://localhost:{}

For Safari users:
1. Start this bridge server
2. Open the Rhythm Pattern Explorer in Safari
3. MIDI should automatically connect

Press Ctrl+C to stop the server
""".format(self.port, self.port))
        
        try:
            tornado.ioloop.IOLoop.current().start()
        except KeyboardInterrupt:
            print("\nServer stopped by user")
            self.close()
    
    def close(self):
        """Clean up resources"""
        if self.midi_out:
            self.midi_out.closePort()
        logger.info("Bridge closed")

def main():
    """Main entry point"""
    print("Simple MIDI Bridge Server")
    print("=" * 40)
    
    # Check Python version
    if sys.version_info < (2, 7):
        print("Error: Python 2.7 or higher required")
        sys.exit(1)
    
    bridge = SimpleMIDIBridge()
    
    # List available MIDI ports
    bridge.list_midi_ports()
    
    # Start server
    bridge.start_server()

if __name__ == "__main__":
    main()