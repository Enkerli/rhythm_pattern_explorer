#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
TouchOSC Bridge Server
WebSocket to OSC UDP bridge for TouchOSC integration

Requirements:
pip install websockets python-osc mido python-rtmidi

Usage:
python touchosc-bridge.py

Then configure TouchOSC to connect to localhost:8000 (OSC)
Web app connects to ws://localhost:8080 (WebSocket)
"""

import asyncio
import json
import logging
import websockets
import threading
import time

try:
    import mido
    from pythonosc import udp_client
    from pythonosc.dispatcher import Dispatcher
    
    # Try different server import patterns for compatibility
    osc_udp_server = None
    try:
        from pythonosc.server import osc_udp_server
        server_available = True
    except ImportError:
        try:
            from pythonosc import server
            osc_udp_server = server
            server_available = True
        except ImportError:
            print("No OSC server module found - will use basic UDP")
            server_available = False
except ImportError as e:
    print(f"Missing dependencies: {e}")
    print("Please install: pip install websockets python-osc mido python-rtmidi")
    exit(1)

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class TouchOSCBridge:
    def __init__(self, websocket_port=8080, osc_send_port=8000, osc_receive_port=9000):
        self.websocket_port = websocket_port
        self.osc_send_port = osc_send_port
        self.osc_receive_port = osc_receive_port
        self.clients = set()
        
        # MIDI setup
        self.setup_midi()
        
        # OSC setup
        self.osc_client = udp_client.SimpleUDPClient("127.0.0.1", osc_send_port)
        logger.info("OSC client sending to localhost:{}".format(osc_send_port))
        
        # OSC server for receiving from TouchOSC
        self.dispatcher = Dispatcher()
        self.dispatcher.set_default_handler(self.handle_osc_message)
        self.osc_server = None
        self.use_simple_server = not server_available
        
    def setup_midi(self):
        """Setup MIDI output"""
        self.midi_out = None
        try:
            output_names = mido.get_output_names()
            logger.info("Available MIDI outputs: {}".format(output_names))
            
            # Look for IAC Driver first
            iac_ports = [name for name in output_names if 'IAC' in name or 'Bus' in name]
            if iac_ports:
                self.midi_out = mido.open_output(iac_ports[0])
                logger.info("MIDI connected to: {}".format(iac_ports[0]))
            elif output_names:
                self.midi_out = mido.open_output(output_names[0])
                logger.info("MIDI connected to: {}".format(output_names[0]))
            else:
                self.midi_out = mido.open_output('TouchOSC Bridge', virtual=True)
                logger.info("Created virtual MIDI port: TouchOSC Bridge")
        except Exception as e:
            logger.error("MIDI setup failed: {}".format(e))
    
    async def handle_websocket(self, websocket):
        """Handle WebSocket connections from web browser"""
        self.clients.add(websocket)
        logger.info("WebSocket client connected")
        
        try:
            async for message in websocket:
                try:
                    data = json.loads(message)
                    await self.handle_web_message(data)
                except json.JSONDecodeError:
                    logger.error("Invalid JSON: {}".format(message))
                except Exception as e:
                    logger.error("WebSocket error: {}".format(e))
        except websockets.exceptions.ConnectionClosed:
            logger.info("WebSocket client disconnected")
        finally:
            self.clients.discard(websocket)
    
    async def handle_web_message(self, data):
        """Process messages from web app"""
        try:
            address = data.get('address', '')
            args = data.get('args', [])
            
            if address == '/midi/note' and len(args) >= 3:
                channel = args[0]['value']
                note = args[1]['value']
                velocity = args[2]['value']
                
                # Send MIDI
                self.send_midi_note(channel, note, velocity)
                
                # Send OSC to TouchOSC
                self.send_osc_note(channel, note, velocity)
                
            elif address == '/midi/cc' and len(args) >= 3:
                channel = args[0]['value']
                control = args[1]['value']
                value = args[2]['value']
                
                # Send MIDI CC
                self.send_midi_cc(channel, control, value)
                
                # Send OSC CC
                self.osc_client.send_message("/midi/cc", [channel, control, value])
                
        except Exception as e:
            logger.error("Error processing web message: {}".format(e))
    
    def send_midi_note(self, channel, note, velocity):
        """Send MIDI note"""
        if not self.midi_out:
            return
            
        try:
            channel = max(0, min(15, channel - 1))  # Convert to 0-15
            note = max(0, min(127, note))
            velocity = max(0, min(127, velocity))
            
            if velocity > 0:
                msg = mido.Message('note_on', channel=channel, note=note, velocity=velocity)
                logger.info("MIDI Note On: Ch{} Note{} Vel{}".format(channel+1, note, velocity))
            else:
                msg = mido.Message('note_off', channel=channel, note=note, velocity=0)
                logger.info("MIDI Note Off: Ch{} Note{}".format(channel+1, note))
            
            self.midi_out.send(msg)
        except Exception as e:
            logger.error("MIDI send error: {}".format(e))
    
    def send_midi_cc(self, channel, control, value):
        """Send MIDI control change"""
        if not self.midi_out:
            return
            
        try:
            channel = max(0, min(15, channel - 1))
            control = max(0, min(127, control))
            value = max(0, min(127, value))
            
            msg = mido.Message('control_change', channel=channel, control=control, value=value)
            self.midi_out.send(msg)
            logger.info("MIDI CC: Ch{} CC{} Val{}".format(channel+1, control, value))
        except Exception as e:
            logger.error("MIDI CC error: {}".format(e))
    
    def send_osc_note(self, channel, note, velocity):
        """Send OSC note to TouchOSC"""
        try:
            if velocity > 0:
                # Note on
                self.osc_client.send_message("/note/on", [channel, note, velocity])
                self.osc_client.send_message("/note/{}/on".format(note), velocity)
                logger.info("OSC Note On: Ch{} Note{} Vel{}".format(channel, note, velocity))
            else:
                # Note off
                self.osc_client.send_message("/note/off", [channel, note])
                self.osc_client.send_message("/note/{}/off".format(note), 0)
                logger.info("OSC Note Off: Ch{} Note{}".format(channel, note))
                
            # Send trigger for TouchOSC visualization
            self.osc_client.send_message("/trigger", note)
            
        except Exception as e:
            logger.error("OSC send error: {}".format(e))
    
    def handle_osc_message(self, address, *args):
        """Handle incoming OSC from TouchOSC"""
        logger.info("OSC received: {} {}".format(address, args))
        
        # Forward interesting OSC messages back to web clients
        try:
            message = {
                'type': 'osc_input',
                'address': address,
                'args': list(args)
            }
            
            # Broadcast to all web clients
            if self.clients:
                asyncio.create_task(self.broadcast_to_web(json.dumps(message)))
                
        except Exception as e:
            logger.error("Error handling OSC input: {}".format(e))
    
    async def broadcast_to_web(self, message):
        """Send message to all connected web clients"""
        if self.clients:
            await asyncio.gather(
                *[client.send(message) for client in self.clients.copy()],
                return_exceptions=True
            )
    
    def start_osc_server(self):
        """Start OSC server in background thread"""
        def run_osc_server():
            try:
                logger.info("Starting OSC server on port {}".format(self.osc_receive_port))
                
                if self.use_simple_server:
                    # Fallback for newer python-osc versions
                    import socketserver
                    import socket
                    
                    class OSCRequestHandler(socketserver.BaseRequestHandler):
                        def handle(self):
                            try:
                                data = self.request[0]
                                osc.osc_packet.ParsePacket(data)
                                logger.info("OSC packet received")
                            except Exception as e:
                                logger.error("OSC parse error: {}".format(e))
                    
                    self.osc_server = socketserver.UDPServer(
                        ("0.0.0.0", self.osc_receive_port), OSCRequestHandler
                    )
                    logger.info("Simple OSC server ready on port {}".format(self.osc_receive_port))
                else:
                    # Use standard OSC server
                    self.osc_server = osc_udp_server.ThreadingOSCUDPServer(
                        ("0.0.0.0", self.osc_receive_port), self.dispatcher
                    )
                    logger.info("OSC server ready on port {}".format(self.osc_receive_port))
                
                self.osc_server.serve_forever()
            except Exception as e:
                logger.error("OSC server error: {}".format(e))
        
        thread = threading.Thread(target=run_osc_server, daemon=True)
        thread.start()
        return thread
    
    async def start_websocket_server(self):
        """Start WebSocket server"""
        logger.info("Starting WebSocket server on port {}".format(self.websocket_port))
        
        async with websockets.serve(
            self.handle_websocket,
            "localhost",
            self.websocket_port,
            ping_interval=30,
            ping_timeout=10
        ):
            logger.info("WebSocket server ready")
            await asyncio.Future()  # Run forever
    
    async def start_servers(self):
        """Start both servers"""
        # Start OSC server in background
        self.start_osc_server()
        
        # Start WebSocket server (blocks)
        await self.start_websocket_server()
    
    def close(self):
        """Clean up resources"""
        if self.midi_out:
            self.midi_out.close()
        if self.osc_server:
            self.osc_server.shutdown()
        logger.info("Bridge closed")

def main():
    """Main entry point"""
    print("TouchOSC Bridge Server")
    print("=" * 40)
    
    bridge = TouchOSCBridge()
    
    print("""
TouchOSC Configuration:
======================
1. Open TouchOSC app
2. Go to Settings > OSC
3. Set Host: {} (your computer's IP)
4. Set Send Port: {}
5. Set Receive Port: {}
6. Enable OSC

Web App:
========
WebSocket: ws://localhost:{}

Ready! Press Ctrl+C to stop
""".format(
        "192.168.1.xxx",  # User needs to set their IP
        bridge.osc_receive_port,
        bridge.osc_send_port,
        bridge.websocket_port
    ))
    
    try:
        asyncio.run(bridge.start_servers())
    except KeyboardInterrupt:
        print("\nStopping bridge...")
    finally:
        bridge.close()

if __name__ == "__main__":
    main()