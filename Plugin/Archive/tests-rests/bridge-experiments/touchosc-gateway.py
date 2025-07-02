#!/usr/bin/env python3
"""
TouchOSC Gateway - Minimal HTTP to OSC UDP bridge
Allows web browsers to send OSC messages directly to TouchOSC via HTTP requests
"""

import socket
import struct
import threading
from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import urllib.parse

class OSCMessage:
    """Simple OSC message encoder"""
    
    @staticmethod
    def encode_string(s):
        """Encode string with null termination and 4-byte padding"""
        data = s.encode('utf-8') + b'\x00'
        # Pad to 4-byte boundary
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
    def create(cls, address, args=None):
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

class TouchOSCGateway:
    def __init__(self, http_port=8889, touchosc_host='localhost', touchosc_port=8000):
        self.http_port = http_port
        self.touchosc_host = touchosc_host
        self.touchosc_port = touchosc_port
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        print(f"TouchOSC Gateway")
        print(f"================")
        print(f"HTTP Server: http://localhost:{http_port}")
        print(f"TouchOSC Target: {touchosc_host}:{touchosc_port}")
        print()
        print("TouchOSC Setup:")
        print("1. Set Connection: UDP Server")
        print(f"2. Set Incoming Port: {touchosc_port}")
        print("3. Set Outgoing Port: 9000")
        print("4. Enable OSC")
        print()
    
    def send_osc(self, address, args=None):
        """Send OSC message to TouchOSC"""
        try:
            message = OSCMessage.create(address, args or [])
            # Send to both localhost and broadcast for TouchOSC discovery
            targets = [
                (self.touchosc_host, self.touchosc_port),
                ('127.0.0.1', self.touchosc_port),
                ('10.0.0.146', self.touchosc_port),  # Your computer's IP
                ('255.255.255.255', self.touchosc_port)  # Broadcast
            ]
            
            for target in targets:
                try:
                    self.sock.sendto(message, target)
                except Exception as target_error:
                    # Ignore individual target errors (like broadcast permission)
                    pass
            
            print(f"📤 OSC: {address} {args or []} -> {self.touchosc_host}:{self.touchosc_port}")
            return True
        except Exception as e:
            print(f"❌ OSC send error: {e}")
            return False

class OSCHandler(BaseHTTPRequestHandler):
    def do_OPTIONS(self):
        """Handle CORS preflight requests"""
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'POST, GET, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()
    
    def do_POST(self):
        """Handle OSC message requests"""
        try:
            # Set CORS headers
            self.send_response(200)
            self.send_header('Access-Control-Allow-Origin', '*')
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            
            # Parse request
            content_length = int(self.headers.get('Content-Length', 0))
            if content_length > 0:
                body = self.rfile.read(content_length)
                data = json.loads(body.decode('utf-8'))
            else:
                # Handle GET-style parameters in POST
                query = urllib.parse.urlparse(self.path).query
                params = urllib.parse.parse_qs(query)
                data = {k: v[0] if len(v) == 1 else v for k, v in params.items()}
            
            # Extract OSC message components
            address = data.get('address', '/test')
            args = data.get('args', [])
            
            # Convert to TouchOSC-friendly addresses
            if address == '/note/on':
                # Send to TouchOSC button/pad format
                touchosc_address = f'/1/push{args[1] % 16 + 1}' if len(args) >= 2 else '/1/push1'
                success1 = self.server.gateway.send_osc(touchosc_address, [1.0])
                success2 = self.server.gateway.send_osc(address, converted_args)
                success = success1 or success2
            elif address == '/note/off':
                touchosc_address = f'/1/push{args[1] % 16 + 1}' if len(args) >= 2 else '/1/push1'
                success1 = self.server.gateway.send_osc(touchosc_address, [0.0])
                success2 = self.server.gateway.send_osc(address, converted_args)
                success = success1 or success2
            else:
                success = self.server.gateway.send_osc(address, converted_args)
            
            # Convert string numbers to appropriate types
            converted_args = []
            for arg in args:
                if isinstance(arg, str):
                    try:
                        # Try integer first
                        converted_args.append(int(arg))
                    except ValueError:
                        try:
                            # Try float
                            converted_args.append(float(arg))
                        except ValueError:
                            # Keep as string
                            converted_args.append(arg)
                else:
                    converted_args.append(arg)
            
            # Send OSC message (this line is now handled above)
            # success = self.server.gateway.send_osc(address, converted_args)
            
            # Response
            response = {'success': success, 'address': address, 'args': converted_args}
            self.wfile.write(json.dumps(response).encode('utf-8'))
            
        except Exception as e:
            print(f"❌ Handler error: {e}")
            self.send_response(500)
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            self.wfile.write(json.dumps({'error': str(e)}).encode('utf-8'))
    
    def do_GET(self):
        """Handle GET requests for testing"""
        if self.path.startswith('/osc'):
            # Parse query parameters
            query = urllib.parse.urlparse(self.path).query
            params = urllib.parse.parse_qs(query)
            
            address = params.get('address', ['/test'])[0]
            args = []
            
            # Extract numbered arguments (arg0, arg1, etc.)
            i = 0
            while f'arg{i}' in params:
                args.append(params[f'arg{i}'][0])
                i += 1
            
            self.send_response(200)
            self.send_header('Access-Control-Allow-Origin', '*')
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            
            success = self.server.gateway.send_osc(address, args)
            self.wfile.write(f"OSC sent: {success}".encode('utf-8'))
        else:
            # Simple test page
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.end_headers()
            
            html = f"""
            <html><body>
            <h1>TouchOSC Gateway</h1>
            <p>Target: {self.server.gateway.touchosc_host}:{self.server.gateway.touchosc_port}</p>
            <p>Test: <a href="/osc?address=/test&arg0=42">/test 42</a></p>
            <p>Note On: <a href="/osc?address=/note/on&arg0=1&arg1=60&arg2=100">/note/on 1 60 100</a></p>
            </body></html>
            """
            self.wfile.write(html.encode('utf-8'))
    
    def log_message(self, format, *args):
        """Suppress default HTTP logging"""
        pass

def main():
    gateway = TouchOSCGateway()
    
    server = HTTPServer(('localhost', gateway.http_port), OSCHandler)
    server.gateway = gateway
    
    print("Ready! Test with:")
    print(f"curl -X POST http://localhost:{gateway.http_port}/osc -H 'Content-Type: application/json' -d '{{\"address\":\"/test\",\"args\":[42]}}'")
    print()
    print("Press Ctrl+C to stop")
    
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping gateway...")
        server.shutdown()
        gateway.sock.close()

if __name__ == "__main__":
    main()