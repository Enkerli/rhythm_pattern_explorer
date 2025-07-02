#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Minimal MIDI Bridge - Works with mido library
Simple WebSocket to MIDI bridge using mido (more reliable)

Requirements:
pip install websockets mido python-rtmidi

Usage:
python minimal-midi-bridge.py
"""

import asyncio
import json
import logging
import websockets

try:
    import mido
except ImportError:
    print("Missing mido library. Please install:")
    print("pip install mido python-rtmidi")
    exit(1)

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class MinimalMIDIBridge:
    def __init__(self, websocket_port=8080):
        self.websocket_port = websocket_port
        self.clients = set()
        
        # Get MIDI output
        self.midi_out = None
        try:
            output_names = mido.get_output_names()
            logger.info("Available MIDI outputs: {}".format(output_names))
            
            # Look for IAC Driver first
            iac_ports = [name for name in output_names if 'IAC' in name or 'Bus' in name]
            if iac_ports:
                self.midi_out = mido.open_output(iac_ports[0])
                logger.info("MIDI output connected to IAC: {}".format(iac_ports[0]))
            elif output_names:
                self.midi_out = mido.open_output(output_names[0])
                logger.info("MIDI output connected to: {}".format(output_names[0]))
            else:
                # Create virtual port
                self.midi_out = mido.open_output('Rhythm Explorer MIDI', virtual=True)
                logger.info("Created virtual MIDI port: Rhythm Explorer MIDI")
                logger.info("Connect this to your DAW or enable IAC Driver in Audio MIDI Setup")
        except Exception as e:
            logger.error("Failed to open MIDI output: {}".format(e))
    
    async def handle_websocket(self, websocket):
        """Handle WebSocket connections"""
        self.clients.add(websocket)
        logger.info("WebSocket client connected")
        
        try:
            async for message in websocket:
                try:
                    data = json.loads(message)
                    await self.handle_message(data)
                except json.JSONDecodeError:
                    logger.error("Invalid JSON: {}".format(message))
                except Exception as e:
                    logger.error("Message error: {}".format(e))
        except websockets.exceptions.ConnectionClosed:
            logger.info("WebSocket client disconnected")
        finally:
            self.clients.discard(websocket)
    
    async def handle_message(self, data):
        """Process incoming messages"""
        if not self.midi_out:
            logger.warning("No MIDI output available")
            return
            
        try:
            address = data.get('address', '')
            args = data.get('args', [])
            
            if address == '/midi/note' and len(args) >= 3:
                channel = max(0, min(15, args[0]['value'] - 1))  # Convert to 0-15
                note = max(0, min(127, args[1]['value']))
                velocity = max(0, min(127, args[2]['value']))
                
                if velocity > 0:
                    # Note on
                    msg = mido.Message('note_on', channel=channel, note=note, velocity=velocity)
                    logger.info("Note On: Ch{} Note{} Vel{}".format(channel+1, note, velocity))
                else:
                    # Note off
                    msg = mido.Message('note_off', channel=channel, note=note, velocity=0)
                    logger.info("Note Off: Ch{} Note{}".format(channel+1, note))
                
                self.midi_out.send(msg)
                
            elif address == '/midi/cc' and len(args) >= 3:
                # Control change
                channel = max(0, min(15, args[0]['value'] - 1))
                control = max(0, min(127, args[1]['value']))
                value = max(0, min(127, args[2]['value']))
                
                msg = mido.Message('control_change', channel=channel, control=control, value=value)
                self.midi_out.send(msg)
                logger.info("CC: Ch{} CC{} Val{}".format(channel+1, control, value))
            
            else:
                logger.warning("Unhandled message: {}".format(address))
                
        except Exception as e:
            logger.error("Error processing message: {}".format(e))
    
    async def start_server(self):
        """Start WebSocket server"""
        logger.info("Starting WebSocket server on port {}".format(self.websocket_port))
        
        async with websockets.serve(
            self.handle_websocket,
            "localhost",
            self.websocket_port,
            ping_interval=30,
            ping_timeout=10
        ):
            logger.info("WebSocket server ready on ws://localhost:{}".format(self.websocket_port))
            print("""
Minimal MIDI Bridge Server
=========================
WebSocket: ws://localhost:{}
MIDI output: {}

Ready for connections!
Press Ctrl+C to stop
""".format(
                self.websocket_port,
                "Virtual Port" if not mido.get_output_names() else mido.get_output_names()[0]
            ))
            
            # Keep running
            await asyncio.Future()
    
    def close(self):
        """Clean up"""
        if self.midi_out:
            self.midi_out.close()
        logger.info("Bridge closed")

def main():
    """Main entry point"""
    print("Minimal MIDI Bridge Server")
    print("=" * 40)
    
    output_names = mido.get_output_names()
    print("Available MIDI outputs:")
    if output_names:
        for i, name in enumerate(output_names):
            print("  {}: {}".format(i, name))
        
        # Check for IAC Driver
        iac_found = any('IAC' in name or 'Bus' in name for name in output_names)
        if not iac_found:
            print("\nTo route MIDI to your DAW:")
            print("1. Open 'Audio MIDI Setup' (in Applications/Utilities)")
            print("2. Go to Window > Show MIDI Studio")
            print("3. Double-click 'IAC Driver'")
            print("4. Check 'Device is online'")
            print("5. Restart this bridge")
    else:
        print("  No MIDI outputs found")
        print("\nTo enable MIDI routing:")
        print("1. Open 'Audio MIDI Setup'")
        print("2. Enable IAC Driver as above")
        print("3. Or connect to your DAW's virtual MIDI port")
    
    bridge = MinimalMIDIBridge()
    
    try:
        asyncio.run(bridge.start_server())
    except KeyboardInterrupt:
        print("\nStopping server...")
    finally:
        bridge.close()

if __name__ == "__main__":
    main()