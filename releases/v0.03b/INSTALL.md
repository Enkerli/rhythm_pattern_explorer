# Installation Instructions - Rhythm Pattern Explorer v0.03b

## Quick Install

1. **Extract the downloaded archive**
2. **Copy plugins to your user library** (NOT system library):
   ```bash
   # AU Component
   cp -R "Rhythm Pattern Explorer.component" ~/Library/Audio/Plug-Ins/Components/
   
   # VST3 Plugin  
   cp -R "Rhythm Pattern Explorer.vst3" ~/Library/Audio/Plug-Ins/VST3/
   ```
3. **Restart your DAW**
4. **Load the plugin** - Look for "Rhythm Pattern Explorer" in your DAW's MIDI effects

## Detailed Installation

### Step 1: Download and Extract
- Download the v0.03b release archive
- Extract to a temporary location
- You should see the `Plugins/` folder containing both plugin formats

### Step 2: Install Plugins

#### Option A: Manual Copy (Recommended)
Open Terminal and run:
```bash
# Navigate to the extracted folder
cd /path/to/extracted/v0.03b/Plugins

# Copy AU Component
cp -R "Rhythm Pattern Explorer.component" ~/Library/Audio/Plug-Ins/Components/

# Copy VST3 Plugin
cp -R "Rhythm Pattern Explorer.vst3" ~/Library/Audio/Plug-Ins/VST3/
```

#### Option B: Finder Copy
1. Open Finder and navigate to the extracted `Plugins/` folder
2. Press `Cmd+Shift+G` and type `~/Library/Audio/Plug-Ins/Components/`
3. Drag `Rhythm Pattern Explorer.component` to this folder
4. Press `Cmd+Shift+G` and type `~/Library/Audio/Plug-Ins/VST3/`
5. Drag `Rhythm Pattern Explorer.vst3` to this folder

### Step 3: Verify Installation
Check that plugins are in the correct locations:
```bash
# Verify AU Component
ls ~/Library/Audio/Plug-Ins/Components/ | grep "Rhythm Pattern Explorer"

# Verify VST3 Plugin
ls ~/Library/Audio/Plug-Ins/VST3/ | grep "Rhythm Pattern Explorer"
```

### Step 4: DAW Setup
1. **Restart your DAW** completely
2. **Rescan plugins** if necessary (some DAWs auto-detect)
3. **Load as MIDI Effect** - Rhythm Pattern Explorer is a MIDI processor, not an instrument

## DAW-Specific Notes

### Logic Pro
- Insert on a MIDI track or Software Instrument track
- Found under **MIDI FX** → **Rhythm Pattern Explorer**
- Use **MIDI Note** parameter to set trigger note

### Ableton Live  
- Insert on a MIDI track
- Found under **MIDI Effects** → **Rhythm Pattern Explorer**
- Route MIDI output to your drum rack or instrument

### Bitwig Studio
- Insert as **MIDI FX** on any track
- Excellent for modulating the **Trigger** parameter with automation
- Works great with Bitwig's **Note FX Grid**

### Cubase/Nuendo
- Insert on MIDI track or Instrument track
- Found under **MIDI Effects** → **Rhythm Pattern Explorer**
- Use **MIDI Inserts** section

### Pro Tools
- Insert on MIDI or Instrument track  
- Found under **MIDI** → **Rhythm Pattern Explorer**
- Requires Pro Tools 2020.1+ for VST3 support

## System Requirements

- **macOS**: 10.13 (High Sierra) or newer
- **Architecture**: Universal Binary (Intel + Apple Silicon native)
- **Memory**: 8MB+ available RAM
- **Disk Space**: 20MB for installation

## Plugin Parameters

The streamlined interface exposes only 3 essential parameters:

1. **Use Host Transport** (Boolean) - Sync with DAW transport
2. **MIDI Note** (0-127) - Note number that triggers patterns  
3. **Trigger** (Boolean) - Manual trigger for pattern updates (automatable)

All pattern editing is done through the plugin's UI, not host parameters.

## Troubleshooting

### Plugin Not Appearing
- Ensure plugins are in **user** library, not system library
- Check paths: `~/Library/Audio/Plug-Ins/` (note the tilde ~)
- Restart DAW completely after installation
- Try rescanning plugins in DAW preferences

### Permission Issues
```bash
# Fix permissions if needed
chmod -R 755 ~/Library/Audio/Plug-Ins/Components/"Rhythm Pattern Explorer.component"
chmod -R 755 ~/Library/Audio/Plug-Ins/VST3/"Rhythm Pattern Explorer.vst3"
```

### No Sound Output
- Rhythm Pattern Explorer is a **MIDI processor**, not a sound generator
- Route its MIDI output to a drum machine, sampler, or instrument
- Check that MIDI is flowing through the plugin

### macOS Security
If you get security warnings:
1. Go to **System Preferences** → **Security & Privacy**
2. Click **Allow** for Rhythm Pattern Explorer
3. Or run: `xattr -cr ~/Library/Audio/Plug-Ins/Components/"Rhythm Pattern Explorer.component"`

## Getting Started

1. **Load the plugin** on a MIDI track
2. **Set MIDI Note** parameter (default: 36 / C1)
3. **Enter a pattern** in the UPI field (try: `E(3,8)`)
4. **Press Parse** or use the **Trigger** parameter
5. **Route MIDI output** to your drum machine/sampler
6. **Play your DAW** and trigger patterns with the specified MIDI note

## Pattern Examples to Try

```
E(3,8)          # Basic Euclidean
B(3,8)          # Barlow indispensability  
W(3,8)          # Wolrab (anti-Barlow)
D(3,8)          # Dilcue (anti-Euclidean)
P(5,0)          # Polygon pattern
E(3,8)B>5       # Progressive Barlow transformation
B(2,7)|W(3,11)  # Scene cycling
```

## Support

- **Documentation**: See included UPI syntax guide
- **Examples**: Check the WebApp for interactive pattern exploration
- **Issues**: Report at project repository
- **License**: CC0 1.0 Universal (Public Domain)

---

**Installation complete!** 🎵 Start exploring mathematical rhythm patterns in your DAW.