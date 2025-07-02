#!/usr/bin/env python3
"""
Local App Server - Serves the rhythm pattern explorer app over HTTP
This bypasses mixed content restrictions when connecting to local bridges

Usage:
python3 serve-app.py

Then open: http://localhost:3000
"""

import http.server
import socketserver
import os
import threading
import webbrowser
from pathlib import Path

class CORSHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    """HTTP handler with CORS headers for local development"""
    
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', '*')
        super().end_headers()
    
    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()
    
    def log_message(self, format, *args):
        """Suppress default HTTP logging for cleaner output"""
        return

def serve_app(port=3000):
    """Serve the app directory"""
    
    # Change to app directory
    app_dir = Path(__file__).parent / "app"
    if not app_dir.exists():
        print(f"❌ App directory not found: {app_dir}")
        print("Make sure you're running this from the rhythm_pattern_explorer directory")
        return
    
    os.chdir(app_dir)
    
    # Create server
    with socketserver.TCPServer(("", port), CORSHTTPRequestHandler) as httpd:
        print("Local Rhythm Pattern Explorer Server")
        print("=" * 40)
        print(f"📱 Serving app on: http://localhost:{port}")
        print(f"🌐 Network access: http://[your-ip]:{port}")
        print(f"📁 Directory: {app_dir}")
        print()
        print("Benefits of local serving:")
        print("✅ No mixed content restrictions")
        print("✅ Can connect to local WebSocket bridges")
        print("✅ Can connect to local HTTP services")
        print("✅ Works on iPad Safari")
        print()
        print("Usage:")
        print("1. Open the URL above in your browser")
        print("2. MIDI bridges will now work without security blocks")
        print("3. Press Ctrl+C to stop server")
        print()
        print("Ready!")
        print("=" * 40)
        
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n🛑 Stopping server...")

def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description='Local App Server for Rhythm Pattern Explorer')
    parser.add_argument('--port', type=int, default=3000, help='Server port (default: 3000)')
    parser.add_argument('--open', action='store_true', help='Auto-open browser')
    
    args = parser.parse_args()
    
    if args.open:
        # Open browser after short delay
        def open_browser():
            import time
            time.sleep(1)
            webbrowser.open(f'http://localhost:{args.port}')
        
        threading.Thread(target=open_browser, daemon=True).start()
    
    serve_app(args.port)

if __name__ == "__main__":
    main()