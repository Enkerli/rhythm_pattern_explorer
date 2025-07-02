#!/usr/bin/env python3
"""
Simple TouchOSC Bridge - WebSocket to OSC/MIDI
For TouchOSC integration with rhythm pattern explorer
"""

import asyncio
import json
import logging
import websockets
import socket
import threading

try:
    import mido
    from pythonosc import udp_client
except ImportError:
    print("Missing dependencies. Please install:")
    print("pip install websockets python-osc mido python-rtmidi")
    exit(1)

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class SimpleTouchOSCBridge:
    def __init__(self, websocket_port=8080, osc_send_port=8000, osc_receive_port=9000):
        self.websocket_port = websocket_port
        self.osc_send_port = osc_send_port
        self.osc_receive_port = osc_receive_port
        self.clients = set()
        
        # MIDI setup
        self.setup_midi()
        
        # OSC setup
        self.osc_client = udp_client.SimpleUDPClient("127.0.0.1", osc_send_port)
        logger.info(f"OSC client sending to localhost:{osc_send_port}")
        
        # Simple UDP server for receiving OSC
        self.osc_socket = None
        
    def setup_midi(self):
        """Setup MIDI output"""
        self.midi_out = None
        try:
            output_names = mido.get_output_names()
            logger.info(f"Available MIDI outputs: {output_names}")
            
            # Look for IAC Driver first
            iac_ports = [name for name in output_names if 'IAC' in name or 'Bus' in name]
            if iac_ports:
                self.midi_out = mido.open_output(iac_ports[0])
                logger.info(f"MIDI connected to: {iac_ports[0]}")
            elif output_names:
                self.midi_out = mido.open_output(output_names[0])
                logger.info(f"MIDI connected to: {output_names[0]}")
            else:
                self.midi_out = mido.open_output('TouchOSC Bridge', virtual=True)
                logger.info("Created virtual MIDI port: TouchOSC Bridge")
        except Exception as e:
            logger.error(f"MIDI setup failed: {e}")
    
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
                    logger.error(f"Invalid JSON: {message}")
                except Exception as e:
                    logger.error(f"WebSocket error: {e}")
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
                
        except Exception as e:
            logger.error(f"Error processing web message: {e}")
    
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
                logger.info(f"MIDI Note On: Ch{channel+1} Note{note} Vel{velocity}")
            else:
                msg = mido.Message('note_off', channel=channel, note=note, velocity=0)
                logger.info(f"MIDI Note Off: Ch{channel+1} Note{note}")
            
            self.midi_out.send(msg)
        except Exception as e:
            logger.error(f"MIDI send error: {e}")
    
    def send_osc_note(self, channel, note, velocity):
        """Send OSC note to TouchOSC"""
        try:
            if velocity > 0:
                # Note on - send to multiple TouchOSC addresses
                self.osc_client.send_message("/note/on", [channel, note, velocity])
                self.osc_client.send_message(f"/note/{note}/on", velocity)
                self.osc_client.send_message("/trigger", note)
                logger.info(f"OSC Note On: Ch{channel} Note{note} Vel{velocity}")
            else:
                # Note off
                self.osc_client.send_message("/note/off", [channel, note])
                self.osc_client.send_message(f"/note/{note}/off", 0)
                logger.info(f"OSC Note Off: Ch{channel} Note{note}")
                
        except Exception as e:
            logger.error(f"OSC send error: {e}")
    
    def start_osc_receiver(self):
        """Start simple UDP receiver for TouchOSC"""
        def run_receiver():
            try:
                self.osc_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                self.osc_socket.bind(('0.0.0.0', self.osc_receive_port))
                logger.info(f"OSC receiver listening on port {self.osc_receive_port}")
                
                while True:
                    try:
                        data, addr = self.osc_socket.recvfrom(1024)
                        logger.info(f"OSC received from {addr}: {len(data)} bytes")
                        # Could parse OSC data here if needed
                    except Exception as e:
                        logger.error(f"OSC receive error: {e}")
                        break
                        
            except Exception as e:
                logger.error(f"OSC receiver setup error: {e}")
        
        thread = threading.Thread(target=run_receiver, daemon=True)
        thread.start()
        return thread
    
    async def start_websocket_server(self):
        """Start WebSocket server"""
        logger.info(f"Starting WebSocket server on port {self.websocket_port}")
        
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
        # Start OSC receiver in background
        self.start_osc_receiver()
        
        # Start WebSocket server (blocks)
        await self.start_websocket_server()
    
    def close(self):
        """Clean up resources"""
        if self.midi_out:
            self.midi_out.close()
        if self.osc_socket:
            self.osc_socket.close()
        logger.info("Bridge closed")

def main():
    """Main entry point"""
    print("Simple TouchOSC Bridge Server")
    print("=" * 40)
    
    bridge = SimpleTouchOSCBridge()
    
    print(f"""
TouchOSC Configuration:
======================
1. Open TouchOSC app
2. Go to Settings > OSC
3. Set Host: [your computer's IP address]
4. Set Send Port: {bridge.osc_receive_port}
5. Set Receive Port: {bridge.osc_send_port}
6. Enable OSC

Web App:
========
WebSocket: ws://localhost:{bridge.websocket_port}

Ready! Press Ctrl+C to stop
""")
    
    try:
        asyncio.run(bridge.start_servers())
    except KeyboardInterrupt:
        print("\nStopping bridge...")
    finally:
        bridge.close()

if __name__ == "__main__":
    main()