#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
OSC-to-MIDI Bridge Server
Simple WebSocket to MIDI bridge for Safari and Firefox MIDI output

Requirements:
pip install websockets rtmidi

Usage:
python osc-midi-bridge.py

Then in the web app, connect to ws://localhost:8080
"""

import asyncio
import json
import logging
import websockets
import rtmidi

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class OSCMIDIBridge:
    def __init__(self, websocket_port=8080, osc_port=5005):
        self.websocket_port = websocket_port
        self.osc_port = osc_port
        
        # MIDI setup
        self.midi_out = rtmidi.RtMidiOut()
        
        # Get available ports
        port_count = self.midi_out.getPortCount()
        self.midi_ports = []
        for i in range(port_count):
            self.midi_ports.append(self.midi_out.getPortName(i))
        
        if self.midi_ports:
            self.midi_out.openPort(0)  # Open first available port
            logger.info("MIDI output connected to: {}".format(self.midi_ports[0]))
        else:
            self.midi_out.openVirtualPort("OSC-MIDI Bridge")
            logger.info("Created virtual MIDI port: OSC-MIDI Bridge")
        
        # WebSocket clients
        self.clients = set()
        
    async def handle_websocket(self, websocket):
        """Handle WebSocket connections from web browsers"""
        self.clients.add(websocket)
        logger.info("WebSocket client connected from {}".format(websocket.remote_address))
        
        try:
            async for message in websocket:
                try:
                    data = json.loads(message)
                    await self.handle_osc_message(data)
                except json.JSONDecodeError:
                    logger.error("Invalid JSON received: {}".format(message))
                except Exception as e:
                    logger.error("Error handling message: {}".format(e))
        except websockets.exceptions.ConnectionClosed:
            logger.info("WebSocket client disconnected")
        finally:
            self.clients.discard(websocket)
    
    async def handle_osc_message(self, data):
        """Convert OSC message to MIDI"""
        try:
            address = data.get('address', '')
            args = data.get('args', [])
            
            if address == '/midi/note' and len(args) >= 3:
                channel = args[0]['value'] - 1  # Convert to 0-15
                note = args[1]['value']
                velocity = args[2]['value']
                
                if velocity > 0:
                    # Note on
                    status = 0x90 + channel
                    logger.info("Note On: Ch{} Note{} Vel{}".format(channel+1, note, velocity))
                else:
                    # Note off
                    status = 0x80 + channel
                    velocity = 0
                    logger.info("Note Off: Ch{} Note{}".format(channel+1, note))
                
                # Send MIDI message using different rtmidi approaches
                try:
                    # Method 1: Use rtmidi.MidiMessage if available
                    if hasattr(rtmidi, 'MidiMessage'):
                        msg = rtmidi.MidiMessage()
                        msg.setMessage(status, note, velocity)
                        self.midi_out.sendMessage(msg)
                    else:
                        raise AttributeError("MidiMessage not available")
                except Exception as e1:
                    try:
                        # Method 2: Use alternative python-rtmidi API
                        import rtmidi.midiconstants
                        self.midi_out.send_message([status, note, velocity])
                    except Exception as e2:
                        try:
                            # Method 3: Try mido compatibility layer
                            import mido
                            if velocity > 0:
                                msg = mido.Message('note_on', channel=channel, note=note, velocity=velocity)
                            else:
                                msg = mido.Message('note_off', channel=channel, note=note, velocity=0)
                            # Convert mido message to bytes and send
                            self.midi_out.sendMessage(msg.bytes())
                        except Exception as e3:
                            logger.error("All MIDI methods failed: {} / {} / {}".format(e1, e2, e3))
                            logger.error("Raw values: status={}, note={}, velocity={}".format(status, note, velocity))
            
            elif address == '/midi/cc' and len(args) >= 3:
                # Control change support
                channel = args[0]['value'] - 1
                controller = args[1]['value']
                value = args[2]['value']
                
                status = 0xB0 + channel
                logger.info("CC: Ch{} CC{} Val{}".format(channel+1, controller, value))
                
                # Send MIDI CC message
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
    
    async def start_websocket_server(self):
        """Start WebSocket server for browser connections"""
        logger.info("Starting WebSocket server on port {}".format(self.websocket_port))
        
        async with websockets.serve(
            self.handle_websocket, 
            "localhost", 
            self.websocket_port,
            ping_interval=30,
            ping_timeout=10
        ):
            logger.info("WebSocket server ready")
            # Keep the server running
            await asyncio.Future()  # Run forever
    
    def list_midi_ports(self):
        """List available MIDI ports"""
        logger.info("Available MIDI ports:")
        for i, port in enumerate(self.midi_ports):
            logger.info("  {}: {}".format(i, port))
        return self.midi_ports
    
    def close(self):
        """Clean up resources"""
        if self.midi_out:
            self.midi_out.closePort()
        logger.info("Bridge closed")

def main():
    """Main entry point"""
    print("OSC-to-MIDI Bridge Server")
    print("=" * 50)
    
    bridge = OSCMIDIBridge()
    
    # List available MIDI ports
    bridge.list_midi_ports()
    
    print("""
WebSocket server starting on ws://localhost:{}
MIDI output ready
Configure your web app to connect to: ws://localhost:{}

For Safari users:
1. Start this bridge server
2. Open the Rhythm Pattern Explorer in Safari
3. MIDI should automatically connect via OSC

Press Ctrl+C to stop the server
""".format(bridge.websocket_port, bridge.websocket_port))
    
    try:
        # Start WebSocket server
        asyncio.run(bridge.start_websocket_server())
    except KeyboardInterrupt:
        print("\nServer stopped by user")
    finally:
        bridge.close()

if __name__ == "__main__":
    main()