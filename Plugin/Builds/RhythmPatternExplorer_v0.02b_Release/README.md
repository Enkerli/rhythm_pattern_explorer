# Rhythm Pattern Explorer v0.02b - Major Timing Fixes Release

## 🎉 What's Included

This release package contains all plugin formats for macOS:

- **`Rhythm Pattern Explorer.component`** - Audio Unit (AU) format
- **`Rhythm Pattern Explorer.vst3`** - VST3 format  
- **`Rhythm Pattern Explorer.app`** - Standalone application

## 🚀 Quick Installation

### 1. Choose Your Format
- **AU**: For Logic Pro, GarageBand, MainStage
- **VST3**: For Bitwig Studio, Cubase, Reaper, Ableton Live
- **Standalone**: Independent application

### 2. Install Plugins
```bash
# Audio Unit (AU)
cp -R "Rhythm Pattern Explorer.component" ~/Library/Audio/Plug-Ins/Components/

# VST3
cp -R "Rhythm Pattern Explorer.vst3" ~/Library/Audio/Plug-Ins/VST3/

# Standalone App (optional)
cp -R "Rhythm Pattern Explorer.app" /Applications/
```

### 3. Restart Your DAW
Restart your DAW to scan for the new plugins.

### 4. Load Plugin
- Load as **MIDI Effect** (not instrument)
- Route MIDI output to drum/percussion instrument

## 🎼 Quick Start

1. **Enter Pattern**: Type `E(3,8)` in UPI field
2. **Parse**: Click Parse button or press Enter  
3. **Play**: Start DAW transport
4. **Enjoy**: Pattern triggers with visual animation

## 🔧 What's Fixed in v0.02b

### ✅ **Critical Timing Issues Resolved**
- Fixed step advancement problems at all BPM ranges
- Eliminated step advance/back behavior at low BPMs
- Consistent animation timing across tempo changes

### 🎯 **Expanded BPM Support** 
- **Previous**: 60-180 BPM limit
- **Now**: 60-1000 BPM range
- **Tested**: 20-666 BPM in Bitwig Studio

### 🖥️ **Clean Interface**
- Streamlined UPI pattern input
- Real-time pie chart visualization
- Focused workflow for music production

## 📋 System Requirements

- **macOS**: 10.15 Catalina or later
- **Architecture**: Universal (Intel + Apple Silicon)
- **DAW**: AU/VST3 compatible host

## 🧪 Tested With

- ✅ **Bitwig Studio 5.3.12** (VST3, extensively tested)
- ✅ **Logic Pro** (AU, validated)
- 🔍 Other DAWs (basic compatibility testing)

## 📖 Pattern Examples

```
E(3,8)          # Basic Euclidean rhythm
E(5,13)         # Complex distribution  
E(3,8)>E(5,13)  # Progressive pattern
P(7,12,2)       # Polygon rhythm
R(4,8,42)       # Random with seed
```

## 📞 Support

- **Documentation**: See included INSTALLATION.md and CHANGELOG.md
- **Issues**: https://github.com/enkerli/rhythm_pattern_explorer/issues
- **Community**: GitHub Discussions

## ⚠️ Important Notes

### **Debug Version**
This version includes debug logging for troubleshooting. Logs are written to `/tmp/bitwig_debug.log`. This will be removed in future production releases.

### **Plugin vs Instrument**
Always load as **MIDI Effect**, not instrument. Route the MIDI output to your drum/percussion instrument.

### **Performance**
Tested extensively at extreme BPMs (20-666 BPM). If you experience issues, try lower buffer sizes or report specific scenarios.

---

**Version**: v0.02b.240703.2245-CLEAN  
**Release Date**: July 3, 2024  
**Compatibility**: macOS 10.15+, AU/VST3  
**Build**: Universal (Intel + Apple Silicon)