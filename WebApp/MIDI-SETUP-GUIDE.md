# MIDI Setup Guide for Rhythm Pattern Explorer

## 🎹 **MIDI Output Overview**

The Rhythm Pattern Explorer includes real-time MIDI output capability, allowing you to send rhythm patterns directly to DAWs, hardware synthesizers, and MIDI-enabled applications.

## ✅ **Supported Browsers & Methods**

| Browser | Platform | Method | Status | Setup Required |
|---------|----------|---------|---------|----------------|
| **Brave** | Mac/Windows | WebMIDI | ✅ **Recommended** | None |
| **Chrome** | Mac/Windows | WebMIDI | ✅ Working | None |
| **Edge** | Windows | WebMIDI | ✅ Working | None |
| **Safari** | Mac | WebSocket Bridge | ✅ Working | Python script |
| **Safari** | iOS/iPad | None | ❌ Limited | Use native apps |
| **Firefox** | All | OSC Bridge | 🟡 Experimental | Python script |

## 🚀 **Quick Start (Recommended)**

### **Method 1: Brave Browser (Easiest)**

1. **Open Brave Browser**
2. **Navigate to**: [Rhythm Pattern Explorer](https://enkerli.github.io/rhythm_pattern_explorer/app/)
3. **Click the MIDI toggle** in the app interface
4. **Grant MIDI permission** when prompted
5. **Done!** MIDI will output to your default MIDI device

**Advantages:**
- No additional software required
- Direct MIDI output
- Perfect timing
- Hardware and software compatible

## 🛠️ **Detailed Setup Instructions**

### **For Brave/Chrome/Edge (WebMIDI)**

#### **Step 1: Configure MIDI Output**

**On Mac:**
1. **Open Audio MIDI Setup** (`/Applications/Utilities/Audio MIDI Setup.app`)
2. **Window** → **Show MIDI Studio**
3. **Enable IAC Driver**:
   - Double-click **IAC Driver**
   - Check **Device is online**
   - Set **Ports**: at least 1 port
   - Click **Apply**

**On Windows:**
1. **Install loopMIDI** (free virtual MIDI port software)
2. **Create virtual port** for DAW communication
3. **Configure your DAW** to receive from virtual port

#### **Step 2: Connect to DAW**

**GarageBand (Mac):**
1. **Create new project** → **Software Instrument**
2. **Track** → **Configure Track Header**
3. **Record Enable** → **Input**: IAC Driver Bus 1
4. **Choose instrument** (drums, synth, etc.)

**Logic Pro (Mac):**
1. **Create Software Instrument track**
2. **Set Input**: IAC Driver Bus 1
3. **Load instrument** and **record-enable track**

**Ableton Live:**
1. **Preferences** → **MIDI**
2. **Control Surface**: IAC Driver Bus 1
3. **Create MIDI track** → **Set input to IAC Driver**

#### **Step 3: Test MIDI Output**

1. **Open Rhythm Pattern Explorer** in Brave/Chrome
2. **Click MIDI toggle** (🎹 icon)
3. **Grant permission** when browser asks
4. **Parse a pattern**: Enter `E(3,8)` and press Enter
5. **Play pattern**: Click ▶️ button
6. **Check DAW**: Should see MIDI notes and hear audio

### **For Safari (WebSocket Bridge)**

#### **Step 1: Set Up Python Bridge**

1. **Download the repository** or get the bridge file:
   ```bash
   curl -O https://raw.githubusercontent.com/enkerli/rhythm_pattern_explorer/main/minimal-midi-bridge.py
   ```

2. **Install Python dependencies**:
   ```bash
   pip install websockets mido python-rtmidi
   ```

3. **Run the MIDI bridge**:
   ```bash
   python3 minimal-midi-bridge.py
   ```
   
   You should see:
   ```
   MIDI Bridge for Safari - WebSocket to MIDI
   ==========================================
   ✅ MIDI connected to IAC Driver Bus 1
   🌐 WebSocket server ready on ws://localhost:8080
   Ready! Connect from Safari and test MIDI output
   ```

#### **Step 2: Serve App Locally**

1. **Download the app** or serve locally:
   ```bash
   python3 serve-app.py --port 3000
   ```

2. **Open in Safari**:
   ```
   http://localhost:3000/app/
   ```

#### **Step 3: Connect MIDI**

1. **Click MIDI toggle** in the app
2. **Should connect** to Python bridge automatically
3. **Test with pattern**: `E(3,8)` → ▶️
4. **Verify in DAW**: MIDI notes should appear

## 🎵 **MIDI Configuration**

### **Default Settings**
- **MIDI Channel**: 1
- **Base Note**: 36 (C2 - kick drum)
- **Velocity**: 100
- **Note Duration**: 100ms

### **Customizing MIDI Output**

The MIDI settings are configurable in the code:

```javascript
// In midi-output.js, MIDIOutputController constructor:
this.settings = {
    channel: 1,        // MIDI channel (1-16)
    baseNote: 36,      // Base MIDI note (C2 = kick)
    velocity: 100,     // Note velocity (1-127)
    // ... other settings
};
```

**Common MIDI Note Assignments:**
- **36 (C2)**: Kick drum (default)
- **38 (D2)**: Snare drum
- **42 (F#2)**: Hi-hat
- **60 (C4)**: Middle C
- **69 (A4)**: A440 reference

## 🔧 **Troubleshooting**

### **No MIDI Permission in Brave/Chrome**

**Problem**: MIDI toggle shows "Connecting" but no permission prompt
**Solution**: 
1. Make sure you **click the MIDI toggle**
2. Try **starting pattern playback first** (audio context requirement)
3. Check **Site Settings** → **MIDI** → Allow

### **Safari Bridge Not Connecting**

**Problem**: "OSC WebSocket connection failed"
**Solutions**:
1. **Check Python bridge is running**: Should show "server listening" message
2. **Verify localhost access**: Try `http://localhost:3000/app/`
3. **Check port conflicts**: Bridge uses port 8080, app server uses 3000
4. **Restart bridge**: Ctrl+C and restart `python3 minimal-midi-bridge.py`

### **No MIDI in DAW**

**Problem**: App shows MIDI connected but DAW receives no notes
**Solutions**:
1. **Check IAC Driver enabled** (Mac) or virtual MIDI port (Windows)
2. **Verify DAW input settings**: Set to IAC Driver Bus 1
3. **Record-enable track** in DAW
4. **Test MIDI monitor**: Use MIDI monitoring software to verify output

### **Timing Issues**

**Problem**: MIDI notes are late or jittery
**Solutions**:
1. **Use Brave/Chrome** for best timing (WebMIDI is more accurate)
2. **Close other applications** to free up CPU
3. **Check buffer settings** in your DAW
4. **Use lower BPM** for testing (60-80 BPM)

## 🎛️ **Advanced Configuration**

### **Multiple MIDI Outputs**

To send to multiple devices:

1. **Mac**: Create multiple IAC Driver buses
2. **Route in DAW**: Use DAW's MIDI routing to send to multiple instruments
3. **Hardware**: Connect multiple MIDI devices via USB hub

### **Custom MIDI Mapping**

For different note mappings (e.g., different drum kits):

```javascript
// Modify settings in midi-output.js
baseNote: 60,  // Change to C4 for melodic instruments
channel: 10,   // Channel 10 for drums (GM standard)
```

### **Velocity Sensitivity**

To vary velocity based on pattern analysis:

```javascript
// In noteOn method, could add:
const velocityFromBalance = Math.floor(pattern.balance.normalizedMagnitude * 127);
```

## 📊 **Performance Tips**

### **Optimal Setup**
- **Use Brave browser** for best MIDI performance
- **Enable IAC Driver** with 1-2 buses (Mac)
- **Set audio buffer** to 256 or 512 samples in DAW
- **Close unnecessary applications** during use

### **Latency Reduction**
- **Use WebMIDI browsers** (Brave/Chrome) for lowest latency
- **Avoid Firefox** for timing-critical applications
- **Use wired connections** for MIDI hardware
- **Optimize DAW settings** for low latency

## 🎹 **Integration Examples**

### **GarageBand Drum Kit**
1. **Create Drummer track**
2. **Set input**: IAC Driver Bus 1
3. **Select drum kit**: Brooklyn, Logan, etc.
4. **Record-enable and play patterns**

### **Logic Pro Sculpture**
1. **Create Software Instrument**
2. **Load Sculpture** synthesizer
3. **Set MIDI input**: IAC Driver
4. **Play melodic patterns** with different base notes

### **Ableton Live Drum Rack**
1. **Create MIDI track**
2. **Add Drum Rack** device
3. **Set track input**: IAC Driver
4. **Map MIDI notes** to different drum sounds

### **Hardware Synthesizer**
1. **Connect MIDI interface** to computer
2. **Set MIDI channel** on synthesizer
3. **Route through MIDI interface** in DAW
4. **Play patterns** directly to hardware

---

## 📝 **Summary**

The MIDI output system provides professional-grade timing and connectivity for integrating rhythm patterns with your music production workflow. The **Brave browser method is recommended** for the best experience, with the **Safari bridge as a reliable alternative** for macOS users.

**Quick Start**: Open in Brave → Click MIDI toggle → Grant permission → Done!