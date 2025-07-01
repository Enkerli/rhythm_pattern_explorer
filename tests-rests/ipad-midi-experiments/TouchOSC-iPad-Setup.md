# TouchOSC iPad Setup Guide

## 🎯 Goal: Standalone iPad MIDI from Rhythm Pattern Explorer

This setup enables the Rhythm Pattern Explorer webapp to send MIDI directly to iOS apps like GarageBand **on the same iPad** without requiring any external devices.

## 📱 Step 1: Install TouchOSC

1. **Download TouchOSC** from the iPad App Store (~$10)
2. **Install and open** TouchOSC on your iPad

## ⚙️ Step 2: Configure TouchOSC

### Connection Settings:
1. **Open TouchOSC** on iPad
2. **Tap the "•••" button** (settings)
3. **Go to "Connections"**
4. **Set the following:**
   - **Connection Type:** `OSC`
   - **Protocol:** `UDP`
   - **Local Port:** `8000` (This is where webapp will send)
   - **Remote Host:** `127.0.0.1` (localhost - same device)
   - **Remote Port:** `9000` (TouchOSC will send here)
   - **Auto Connect:** `ON`

### MIDI Output Settings:
1. **Still in TouchOSC settings**
2. **Go to "MIDI"**
3. **Enable "MIDI Output"**
4. **Set:**
   - **Output Destination:** `Network Session 1` (or any iOS MIDI destination)
   - **Channel:** `1` (or your preferred channel)
   - **Note Range:** `0-127` (full range)

### OSC Routing (Important):
1. **In TouchOSC settings**
2. **Go to "OSC"**
3. **Enable "Route OSC to MIDI"** (if available)
4. **Or set up OSC→MIDI mapping:**
   - **OSC Address:** `/note/on` → **MIDI:** Note On
   - **OSC Address:** `/note/off` → **MIDI:** Note Off

## 🎹 Step 3: Enable iOS Network Session

1. **iPad Settings** → **General** → **AirPlay & Handoff**
2. **Enable "Network MIDI"**
3. **Verify "Network Session 1" appears** in music apps

## 🌐 Step 4: Test the Connection

### Option A: Use Built-in Test Page
1. **Open Safari on iPad**
2. **Navigate to:** `[your-app-url]/ipad-midi-test.html`
3. **Keep TouchOSC running in background**
4. **Click "Connect to Mac"** (but enter `localhost`)
5. **Test MIDI output**

### Option B: Use Main App
1. **Open the Rhythm Pattern Explorer** in Safari
2. **Keep TouchOSC running in background**
3. **Enable MIDI output** in the app
4. **Play patterns** - should send MIDI to TouchOSC

## 🧪 Step 5: Verify MIDI Reception

### In TouchOSC:
- **Watch for activity indicators** when webapp sends MIDI
- **Check OSC message log** for incoming `/note/on` and `/note/off`
- **Verify MIDI output is active**

### In GarageBand (or other MIDI app):
1. **Open GarageBand**
2. **Create new project** → **Instruments**
3. **Choose any instrument**
4. **Tap track settings** (gear icon)
5. **Input:** Select `Network Session 1` or `TouchOSC`
6. **Record-enable the track**
7. **Play rhythm patterns** - should hear audio and see MIDI notes

## 🔧 Troubleshooting

### TouchOSC Not Receiving Messages:
- **Check port 8000** is correct in both TouchOSC and webapp
- **Verify UDP Server mode** is enabled in TouchOSC
- **Try force-closing and reopening** TouchOSC
- **Check Safari console** for connection errors

### No MIDI Output from TouchOSC:
- **Verify MIDI Output is enabled** in TouchOSC settings
- **Check Network Session 1** is available (iOS Settings)
- **Try different MIDI destination** in TouchOSC
- **Restart iPad** to refresh MIDI system

### App Says "No MIDI Available":
- **Make sure touchosc-ipad-bridge.js is loaded** in webapp
- **Check Safari console** for JavaScript errors
- **Verify iPad detection** in browser console
- **Try manually enabling TouchOSC mode**

## 📊 Expected Behavior

When working correctly:
1. **Rhythm Pattern Explorer plays** rhythm patterns
2. **Webapp sends OSC messages** to TouchOSC (port 8000)
3. **TouchOSC converts OSC to MIDI** and outputs to Network Session
4. **GarageBand receives MIDI** from Network Session 1
5. **You hear the rhythm** in GarageBand instruments

## 🎵 Advanced: Custom MIDI Routing

For more complex setups:
- **Use AUM (Audio Mixer)** for advanced MIDI routing
- **Connect multiple iOS music apps** via Network Session
- **Create custom TouchOSC layouts** for visual feedback
- **Route to hardware via USB or Lightning adapters**

## 🚀 Success Confirmation

You'll know it's working when:
- ✅ **TouchOSC shows incoming OSC activity**
- ✅ **TouchOSC MIDI output indicator is active**
- ✅ **GarageBand receives and plays MIDI notes**
- ✅ **Rhythm patterns are audible in real-time**
- ✅ **No external devices required**

This gives you true **standalone iPad MIDI** for the Rhythm Pattern Explorer!