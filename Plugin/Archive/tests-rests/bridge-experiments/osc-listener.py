#!/usr/bin/env python3
"""
OSC Listener - Test if OSC messages are being sent correctly
"""

import socket
import struct

def decode_osc_string(data, offset):
    """Decode OSC string from data"""
    end = data.find(b'\x00', offset)
    if end == -1:
        return "", len(data)
    
    string = data[offset:end].decode('utf-8')
    # Advance to next 4-byte boundary
    next_offset = ((end + 4) // 4) * 4
    return string, next_offset

def decode_osc_int(data, offset):
    """Decode OSC 32-bit integer"""
    value = struct.unpack('>i', data[offset:offset+4])[0]
    return value, offset + 4

def decode_osc_message(data):
    """Decode OSC message"""
    try:
        # Address pattern
        address, offset = decode_osc_string(data, 0)
        
        # Type tag string
        type_tag, offset = decode_osc_string(data, offset)
        
        # Arguments
        args = []
        for tag in type_tag[1:]:  # Skip the comma
            if tag == 'i':
                value, offset = decode_osc_int(data, offset)
                args.append(value)
            elif tag == 's':
                value, offset = decode_osc_string(data, offset)
                args.append(value)
        
        return address, args
    except Exception as e:
        return f"Parse error: {e}", []

def listen_for_osc(port=8000):
    """Listen for OSC messages on specified port"""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(('0.0.0.0', port))
    
    print(f"🎧 Listening for OSC messages on port {port}")
    print("This simulates TouchOSC receiving messages...")
    print("Press Ctrl+C to stop")
    print()
    
    try:
        while True:
            data, addr = sock.recvfrom(1024)
            address, args = decode_osc_message(data)
            print(f"📥 OSC from {addr}: {address} {args}")
            
    except KeyboardInterrupt:
        print("\n👋 Stopping OSC listener")
    finally:
        sock.close()

if __name__ == "__main__":
    listen_for_osc()