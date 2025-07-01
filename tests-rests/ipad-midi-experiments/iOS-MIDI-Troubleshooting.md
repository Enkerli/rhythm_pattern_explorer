# iOS Network Session MIDI Troubleshooting Guide

## Problem: iPad MIDI Bridge receives WebSocket messages but MIDI doesn't reach Network Session 1

Based on the current status: "When I press the MIDI button, it connects to the Python script and when I run playback, I get the MIDI note messages... as long as both apps are active. What doesn't work: no MIDI message received on iPad (Network Session 1)."

## Root Cause Analysis

The issue is likely one of the following:

### 1. iOS Network Session Not Properly Enabled
**Most Common Issue** - Network Session must be explicitly enabled in iOS Settings.

**Solution:**
```
iOS Settings Path (varies by iOS version):
- iOS 15+: Settings > General > AirPlay & Handoff > Network MIDI
- iOS 14-: Settings > General > Network > Network Session
- Alternative: Some iOS versions have Settings > MIDI > Network Session
```

**Verification:**
- Look for "Network Session 1" in MIDI apps like GarageBand
- Check Audio MIDI Setup on Mac - should show iPad as network device

### 2. MIDI Port Name Mismatch
**Issue:** The bridge may not be connecting to the correct Network Session port.

**Diagnostic Steps:**
```python
# Run this in the diagnostic bridge to see exact port names:
import mido
print("Available MIDI outputs:")
for i, name in enumerate(mido.get_output_names()):
    print(f"  {i}: {name}")
```

**Expected Results:**
- Should see "Network Session 1" or similar
- May see "iPad-Name Network Session" format
- Could be "Bluetooth MIDI" if using Bluetooth

### 3. iOS MIDI Permission/Security
**Issue:** iOS may be blocking MIDI access from network sources.

**Solutions:**
- Restart the iPad after enabling Network Session
- Try connecting from a different Mac/device first
- Check iOS Privacy settings for MIDI apps

### 4. Network/Firewall Issues
**Issue:** WiFi network or firewall blocking MIDI traffic.

**Diagnostic:**
```bash
# On Mac, check if iPad is visible:
dns-sd -B _apple-midi._tcp

# Should show iPad's MIDI service
```

**Solutions:**
- Ensure both devices on same WiFi network
- Try different WiFi network
- Disable VPN if active
- Check router firewall settings

### 5. Python-rtmidi Backend Issues
**Issue:** The rtmidi library may not support iOS Network Session properly.

**Diagnostic:**
```python
import rtmidi
print("Available MIDI APIs:")
for api in rtmidi.get_compiled_api():
    print(f"  {rtmidi.get_api_display_name(api)}")
```

**Solutions:**
- Try different rtmidi backends
- Use CoreMIDI directly if on macOS
- Consider alternative MIDI libraries

## Comprehensive Testing Steps

### Step 1: Verify iOS Network Session
```
1. iPad Settings > General > AirPlay & Handoff
2. Enable "Network MIDI" (toggle should be ON)
3. Restart iPad
4. Open GarageBand or another MIDI app
5. Look for "Network Session 1" in MIDI input list
```

### Step 2: Test with Mac Audio MIDI Setup
```
1. Mac: Open Audio MIDI Setup (/Applications/Utilities/)
2. Window > Show MIDI Studio
3. Click "Network" button
4. Should see iPad listed as "Online" device
5. Enable iPad in "My Sessions" list
6. Test sending MIDI from Mac to iPad
```

### Step 3: Run Enhanced Diagnostic Bridge
```bash
# Use the new diagnostic version:
python3 ipad-midi-bridge-diagnostic.py

# Check the detailed logs for:
# - MIDI port discovery
# - Connection success/failure
# - Message send confirmation
```

### Step 4: Test with Alternative iOS MIDI Apps
Try these apps to verify Network Session works:
- **GarageBand** (free) - Check track input settings
- **AUM** (paid) - Excellent MIDI monitoring
- **MIDI Wrench** (paid) - MIDI diagnostic tool
- **TouchOSC** - Can receive MIDI and show activity

### Step 5: Alternative Connection Methods

If Network Session fails, try:

#### A. Bluetooth MIDI
```python
# Look for Bluetooth MIDI ports instead:
bluetooth_ports = [name for name in mido.get_output_names() 
                   if 'bluetooth' in name.lower()]
```

#### B. USB MIDI (if iPad supports USB-C)
Connect iPad directly via USB-C and use USB MIDI interface.

#### C. Virtual MIDI Apps
Use iOS apps that create virtual MIDI ports:
- **MIDI Bridge** - Creates network MIDI connections
- **Bluetooth MIDI Connect** - Enhanced Bluetooth MIDI

## Code Modifications for Better Compatibility

### Enhanced Port Discovery
```python
def find_ios_midi_port():
    """Find iOS MIDI port with multiple strategies"""
    output_names = mido.get_output_names()
    
    # Strategy 1: Look for exact Network Session
    for name in output_names:
        if 'Network Session 1' in name:
            return name
    
    # Strategy 2: Look for any Network Session
    for name in output_names:
        if 'Network Session' in name:
            return name
    
    # Strategy 3: Look for iPad-specific names
    for name in output_names:
        if any(term in name.lower() for term in ['ipad', 'network', 'session']):
            return name
    
    # Strategy 4: Look for Bluetooth MIDI
    for name in output_names:
        if 'bluetooth' in name.lower():
            return name
    
    return None
```

### Force CoreMIDI Backend (macOS)
```python
import rtmidi
# Force CoreMIDI backend for better iOS compatibility
midiout = rtmidi.MidiOut(rtmidi.API_MACOSX_CORE)
```

## Alternative: Direct iOS MIDI Bridge App

If Network Session continues to fail, consider using a dedicated iOS MIDI bridge app:

1. **MIDI Network** (iOS App Store)
2. **MIDImux** (iOS App Store) 
3. **MIDI Bridge** (iOS App Store)

These apps create more reliable MIDI connections and often work when Network Session fails.

## Expected Debug Output

When working correctly, you should see:
```
Available MIDI outputs: ['Network Session 1', 'IAC Driver Bus 1']
✅ MIDI connected to Network Session: Network Session 1
🧪 Testing MIDI connection...
✅ Test Note On sent (C4, velocity 64)
✅ Test Note Off sent
📤 Sending MIDI: note_on channel=0 note=36 velocity=100 time=0
✅ MIDI message sent successfully
```

## Last Resort: Hybrid Approach

If iOS Network Session proves unreliable, implement a hybrid approach:

1. **WebSocket to Mac bridge** (current working setup)
2. **Mac runs DAW or MIDI routing software**
3. **DAW sends MIDI to iOS via:**
   - Audio MIDI Setup Network Session
   - Third-party MIDI routing (MIDI Patchbay, OSCRouter)
   - Hardware MIDI interface

This adds one step but provides more reliable MIDI delivery to iOS devices.

## Summary

The most likely issue is that iOS Network Session isn't properly enabled or configured. The diagnostic bridge will help identify exactly where the connection is failing. Try the enhanced diagnostic version first, then work through the troubleshooting steps systematically.