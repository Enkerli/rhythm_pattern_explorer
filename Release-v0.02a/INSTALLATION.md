# Rhythm Pattern Explorer - Installation Guide

## 🎯 Quick Install

### Download & Install
1. **Download** the latest release from GitHub releases
2. **Extract** the plugin bundle to your plugins folder:
   - **AU**: `~/Library/Audio/Plug-Ins/Components/`
   - **VST3**: `~/Library/Audio/Plug-Ins/VST3/`
   - **AUv3**: Install standalone app, extension auto-registers
3. **Restart** your DAW to scan new plugins
4. **Load** "Rhythm Pattern Explorer" as a MIDI effect

---

## 🎵 DAW Setup Instructions

### Bitwig Studio
1. **Add MIDI Effect Device**
2. **Browse** → **Third Party** → **Rhythm Pattern Explorer**
3. **Set as VST3** for best performance
4. **Route MIDI output** to your drum/percussion instrument

### Logic Pro
1. **Add MIDI Effect** on instrument channel strip
2. **Select AU** → **Rhythm Pattern Explorer**
3. **Set Channel Strip** to receive MIDI from plugin
4. **Connect to** drum kit or sampler instrument

### Other DAWs
- **Load as MIDI Effect** (not instrument)
- **Enable MIDI passthrough** if available
- **Route output** to drum/percussion tracks
- **Sync with host transport** recommended

---

## 🎼 Basic Usage

### Getting Started
1. **Enter Pattern**: Type `E(3,8)` in UPI input field
2. **Press Parse**: Click Parse button or press Enter
3. **Start Playback**: Use DAW transport controls
4. **Adjust Tempo**: Change DAW BPM (20-666 BPM supported)

### Pattern Examples
```
E(3,8)          # Euclidean: 3 hits in 8 steps
E(5,13)         # Euclidean: 5 hits in 13 steps  
P(7,2)          # Polygon: 7-sided polygon, rotation 2
R(4,12)         # Random: 4 hits in 12 steps
B(10101010,8)   # Binary: Custom pattern
E(3,8)>E(5,13)  # Progressive: Evolving pattern
```

### Pattern Types
- **E(onsets,steps)**: Euclidean rhythm distribution
- **P(sides,rotation)**: Polygon-based patterns
- **R(hits,steps)**: Random distribution
- **B(binary,steps)**: Custom binary patterns
- **Progressive**: Chain patterns with `>`

---

## 🔧 Troubleshooting

### Plugin Not Found
- **Check plugin folder** permissions
- **Restart DAW** after installation
- **Verify architecture** (Intel vs Apple Silicon)
- **Try different formats** (AU vs VST3)

### No MIDI Output
- **Set as MIDI Effect**, not instrument
- **Check MIDI routing** in DAW
- **Verify host transport** is playing
- **Test with simple pattern** like `E(4,4)`

### Timing Issues
- **Use host transport** for best sync
- **Avoid very high BPMs** (>666 BPM)
- **Check audio buffer size** if glitchy
- **Restart plugin** if timing drifts

### Animation Problems
- **Resize plugin window** to refresh
- **Check pattern is valid** in text display
- **Restart if animation stops** responding

---

## ⚙️ Advanced Configuration

### Performance Optimization
- **Audio Buffer Size**: 128-512 samples recommended
- **Sample Rate**: 44.1kHz or 48kHz standard
- **CPU Usage**: Minimal impact on most systems
- **Memory**: <10MB typical usage

### BPM Range
- **Minimum**: 20 BPM (very slow patterns)
- **Maximum**: 666 BPM (limited by Bitwig)
- **Optimal**: 60-300 BPM for most musical applications
- **Extreme**: 400+ BPM for electronic/experimental music

### Pattern Complexity
- **Steps**: 4-32 step patterns supported
- **Onsets**: 1-31 onsets per pattern
- **Progressive**: Up to 5 chained patterns
- **Binary**: Custom 32-bit patterns

---

## 🐛 Debug Information

### Debug Logging (Development)
Current version includes debug logging for troubleshooting:
- **Log File**: `/tmp/bitwig_debug.log`
- **Contents**: Transport state, BPM sync, step triggers
- **Usage**: For reporting issues to developers

### Reporting Issues
When reporting problems, include:
1. **DAW name and version**
2. **Plugin format used** (AU/VST3/AUv3)
3. **BPM when issue occurs**
4. **Pattern being used**
5. **Debug log excerpt** if available

### Common Debug Messages
```
BITWIG PROCESS: BPM=120, SamplesPerStep=2756, BufferSize=512
PLAYING DEBUG: isPlaying=TRUE, hostIsPlaying=TRUE
STEP TRIGGER: BPM=180.0, Step 2->3, samplesPerStep=1837
```

---

## 📋 System Requirements

### Minimum Requirements
- **macOS**: 10.15 Catalina or later
- **CPU**: Intel or Apple Silicon
- **RAM**: 4GB available memory
- **DAW**: AU/VST3 compatible host

### Recommended Setup
- **macOS**: 12.0 Monterey or later
- **CPU**: Apple Silicon (M1/M2) for best performance
- **RAM**: 8GB+ for complex projects
- **Audio Interface**: Professional grade for low latency

### Tested Environments
- **Bitwig Studio**: 5.3.12 (VST3 recommended)
- **Logic Pro**: 10.7+ (AU format)
- **Buffer Sizes**: 64-2048 samples
- **Sample Rates**: 44.1, 48, 88.2, 96 kHz

---

## 🔄 Version Management

### Current Stable Version
- **v0.02b.240703.2245-CLEAN**: Latest stable release
- **Major Features**: Fixed timing issues, expanded BPM range
- **Compatibility**: All supported DAWs and formats

### Update Process
1. **Download** new version from releases
2. **Quit DAW** before updating
3. **Replace** old plugin files
4. **Restart DAW** to load new version
5. **Verify** version number in plugin UI

### Backup Recommendations
- **Save project files** before updating
- **Export important patterns** as text
- **Keep previous version** until testing complete

---

## 📞 Support & Resources

### Documentation
- **GitHub Repository**: Source code and issues
- **User Manual**: Comprehensive pattern guide
- **Video Tutorials**: Coming soon
- **Community Forum**: User discussions

### Getting Help
1. **Check troubleshooting** section first
2. **Search existing issues** on GitHub
3. **Create new issue** with debug info
4. **Community support** via discussions

### Contributing
- **Bug Reports**: Use GitHub issues
- **Feature Requests**: Discussions section
- **Code Contributions**: Pull requests welcome
- **Pattern Examples**: Share in community