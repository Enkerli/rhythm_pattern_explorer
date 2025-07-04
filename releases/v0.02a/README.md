# Rhythm Pattern Explorer v0.02a

Mathematical rhythm pattern generator and analyzer for macOS.

## 🎯 New in v0.02a

### Major Features
- **Hierarchical Indispensability Engine** - Fixes sequential filling in prime patterns
- **New Pattern Notations** - B(n,s), W(n,s), D(n,s) complement E(n,s)
- **Enhanced Morse Code** - Any text support with natural length preservation
- **Improved Progressive Transformations** - Better musical results for prime patterns

### Technical Improvements
- **Prime Pattern Fix** - 7, 11, 13 step patterns now use hierarchical placement
- **Natural Length Support** - `M:A` → `110` (3 steps) instead of artificial padding
- **Webapp Compatibility** - Perfect sync with rhythm-pattern-explorer.app

## 📦 Installation

### VST3 Plugin
1. Copy `Rhythm Pattern Explorer.vst3` to:
   - **System-wide**: `/Library/Audio/Plug-Ins/VST3/`
   - **User-only**: `~/Library/Audio/Plug-Ins/VST3/`
2. Scan plugins in your DAW
3. Load as MIDI effect

### AU Plugin
1. Copy `Rhythm Pattern Explorer.component` to:
   - **System-wide**: `/Library/Audio/Plug-Ins/Components/`
   - **User-only**: `~/Library/Audio/Plug-Ins/Components/`
2. Scan plugins in your DAW
3. Load as MIDI effect

## 🎼 Quick Start

1. **Load plugin** as MIDI effect on any track
2. **Enter pattern** using UPI syntax: `E(3,8)`, `B(4,7)`, `M:ALEX`
3. **Click Parse** to generate rhythm
4. **Play track** to hear MIDI pattern
5. **Experiment** with progressive transformations: `E(3,8)B>6`

## 📚 Pattern Syntax (UPI)

### Core Patterns
- **E(3,8)** - Euclidean: 3 onsets in 8 steps
- **B(4,7)** - Barlow: Hierarchical indispensability
- **W(2,11)** - Wolrab: Anti-Barlow (syncopated)
- **D(5,13)** - Dilcue: Anti-Euclidean
- **P(5,0,16)** - Polygon: 5-sided shape in 16 steps
- **M:ALEX** - Morse: Text to rhythm pattern
- **R(4,8)** - Random: 4 random hits in 8 steps

### Transformations
- **~E(3,8)** - Invert pattern
- **rev P(5,0)** - Reverse pattern
- **comp B(4,7)** - Complement pattern

### Combinations
- **E(3,8) + P(4,0)** - OR combination
- **B(5,12) - E(2,12)** - Difference

### Progressive Evolution
- **E(3,8)B>7** - Euclidean evolving via Barlow to 7 onsets
- **P(4,0)W>2** - Polygon reducing via Wolrab to 2 onsets

## 🌐 Web Integration

Perfect compatibility with [rhythm-pattern-explorer.app](https://enkerli.github.io/rhythm_pattern_explorer/app/):
1. **Experiment online** with patterns
2. **Copy syntax** to plugin
3. **Generate MIDI** in your DAW

## 💻 System Requirements

- **macOS**: 10.13 or later
- **Architecture**: Universal Binary (Intel + Apple Silicon)
- **DAW**: Any VST3 or AU compatible host
- **Memory**: Minimal footprint
- **CPU**: Real-time pattern generation

## 🎵 Compatible DAWs

- Logic Pro
- Ableton Live
- Cubase
- Pro Tools
- Bitwig Studio
- Reaper
- Studio One
- And more...

## 🔧 Troubleshooting

### Plugin Not Appearing
1. Verify plugin is in correct folder
2. Rescan plugins in DAW
3. Check DAW supports MIDI effects

### No MIDI Output
1. Ensure plugin loaded as MIDI effect
2. Check MIDI routing in DAW
3. Verify track is armed/enabled

### Pattern Not Generating
1. Check UPI syntax spelling
2. Try simpler pattern first: `E(3,8)`
3. Click Parse after entering pattern

## 📋 Release Details

- **Version**: v0.02a
- **Build Date**: July 4, 2025
- **File Size**: ~2MB per format
- **Formats**: VST3, AU
- **Code Signing**: Local development signature

## 🏗️ For Developers

Built with:
- **JUCE 8.0.1** - Audio plugin framework
- **C++17** - Modern C++ standard
- **Xcode 15** - macOS development
- **Universal Binary** - Intel + Apple Silicon

## 📝 License

© 2024 Rhythm Pattern Explorer
Free for personal and commercial use.

## 🔗 Links

- **Web App**: https://enkerli.github.io/rhythm_pattern_explorer/app/
- **Source Code**: https://github.com/enkerli/rhythm_pattern_explorer
- **Documentation**: Built into web application

---

**Rhythm Pattern Explorer v0.02a** - Bringing mathematical rhythm generation to your creative workflow.