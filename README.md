# Rhythm Pattern Explorer

**Mathematical Rhythm Pattern Generator and Analyzer**

A comprehensive system for generating, analyzing, and manipulating rhythmic patterns using advanced mathematical algorithms including Euclidean rhythms, Barlow's indispensability theory, and progressive transformations.

## Project Structure

This repository is organized into two main components:

> **Note**: For backward compatibility, `app/` in the project root is a symbolic link to `WebApp/app/`

### 🎛️ Plugin/ - Audio Plugin Implementation
```
Plugin/
├── Source/                  # C++ plugin source code
├── RhythmPatternExplorer.jucer  # JUCE project file
├── Builds/                  # Xcode build files
├── Tests/                   # Comprehensive test suites
├── Documentation/           # Plugin-specific docs
└── README.md               # Plugin development guide
```

**Formats**: Audio Unit (AU), VST3, Standalone  
**Platform**: macOS (iOS experimental)  
**Status**: ✅ Production ready

### 🌐 WebApp/ - Web Application (Reference Implementation)
```
WebApp/
├── app/                     # Web application files
├── serve-app.py            # Development server
├── minimal-midi-bridge.py  # MIDI bridge utility
├── *.md                    # Shared documentation
└── WEBAPP_README.md        # WebApp guide
```

**Platform**: Any modern web browser  
**Features**: Full algorithm reference, interactive UI  
**Status**: ✅ Stable reference implementation

## Quick Start

### Using the Plugin (Recommended for Music Production)
```bash
# Navigate to plugin directory
cd Plugin/

# Open JUCE project
open RhythmPatternExplorer.jucer

# Build and install (see Plugin/README.md for details)
```

### Using the Web App (For Development/Reference)
```bash
# Option 1: Use WebApp directory (recommended)
cd WebApp/
python3 serve-app.py

# Option 2: Use legacy app/ link (backward compatibility)
cd app/
python3 -m http.server 8000

# Open browser to http://localhost:8080 or http://localhost:8000
```

## Core Features

### Pattern Generation Algorithms
- **Euclidean Patterns**: Even distribution using Bjorklund algorithm
- **Polygon Patterns**: Geometric rhythm generation
- **Random Patterns**: Controlled randomization with seeds
- **Binary Patterns**: Direct binary representation
- **Accent Patterns**: Suprasegmental accent layer with curly bracket notation

### Progressive Transformations
- **Barlow (B)**: Indispensability-based pattern evolution
- **Euclidean (E)**: Progressive onset addition/removal
- **Wolrab (W)**: Anti-Barlow transformations
- **Dilcue (D)**: Anti-Euclidean transformations

### Accent Pattern System (v0.03c+)
- **Curly Bracket Notation**: `{accent}pattern` or `pattern{accent}`
- **Polyrhythmic Cycling**: Automatic cycling when accent/rhythm lengths differ
- **MIDI Enhancement**: Accented notes have +5 semitones pitch and higher velocity
- **Visual Feedback**: Red for accented onsets, green for regular onsets
- **Universal Support**: Works with all pattern types (E, P, R, B, W, D)

### UPI (Universal Pattern Input) Notation

#### Basic Pattern Generation
```
E(3,8)                    # Euclidean: 3 onsets in 8 steps → 10010010
B(3,8)                    # Barlow indispensability: 3 onsets in 8 steps → 10001010
W(3,8)                    # Wolrab (anti-Barlow): 3 onsets in 8 steps → 01100100
D(3,8)                    # Dilcue (anti-Euclidean): 3 onsets in 8 steps → 01101001
P(5,12,0)                 # Pentagon rhythm: 5 vertices in 12 steps with 0 offset
R(4,8,42)                 # Random: 4 onsets in 8 steps with seed 42
```

#### Progressive Transformations
```
E(1,8)E>8                 # Progressive Euclidean: 1→8 onsets (manual trigger)
E(8,8)B>1                 # Progressive Barlow dilution: 8→1 onsets (manual trigger)
B(3,13)W>11               # Progressive Barlow to Wolrab: 3→11 onsets
E(1,8)@3                  # Progressive offset: rotation by 3 steps per trigger
E(1,8)@#5                 # Auto-advancing offset: rotation by 5 steps per beat
```

#### Scene Cycling
```
E(3,8)|P(5,12)           # Scene cycling: alternate between tresillo and pentagon
100|110|101              # Scene cycling: alternate between binary patterns
{100}E(3,8)|{010}B(5,13) # Scene cycling with different accent patterns
```

#### Accent Patterns (v0.03c+)
```
{100}E(3,8)              # Accented tresillo: accent on first onset
{10010}E(5,8)            # Quintillo with accents on 1st and 4th onsets
{10}E(4,8)               # Alternating strong/weak accents (polyrhythmic)
{E(2,5)}E(3,8)           # Euclidean accent pattern over Euclidean rhythm
{P(3,0)}B(5,13)          # All-accent pattern cycling over Barlow rhythm
{101}E(1,8)B>8           # Progressive transformation with accents
```

#### Pattern Combinations
```
E(3,8)+P(5,12)           # Pattern combination: union of onsets
E(3,8)-P(5,12)           # Pattern subtraction: remove overlapping onsets
P(3,0)+P(5,0)-P(2,0)     # Complex combination for perfect balance
```

#### Binary and Numeric Patterns
```
101010                   # Binary pattern: direct onset specification
0x2A:8                   # Hexadecimal: 42 in hex spread over 8 steps
0o52:8                   # Octal: 42 in octal spread over 8 steps
42:8                     # Decimal: 42 spread over 8 steps
[0,2,4,6]:8              # Onset array: specific positions in 8 steps
```

## Implementation Relationship

```
WebApp (JavaScript)          Plugin (C++)
      ↓                          ↑
 Reference Algorithms    ←    Ported Algorithms
      ↓                          ↑
   Browser Testing       ←    DAW Integration
      ↓                          ↑
 Algorithm Validation    ←    Performance Testing
```

The **WebApp** serves as the **reference implementation** - all algorithms are first developed and validated in the browser, then ported to the **Plugin** for professional music production use.

## Documentation

### Shared Documentation (in WebApp/)
- **[Algorithm Reference](WebApp/PROGRESSIVE_TRANSFORMATION_REFERENCE.md)**: Mathematical specifications
- **[MIDI Setup Guide](WebApp/MIDI-SETUP-GUIDE.md)**: Configuration instructions
- **[Working Solutions](WebApp/WORKING-MIDI-SOLUTIONS.md)**: Troubleshooting guide

### Plugin-Specific (in Plugin/)
- **[Plugin Status](Plugin/Documentation/PLUGIN_STATUS.md)**: Current state and features
- **[Installation Guide](Plugin/INSTALLATION_SUCCESS.md)**: Build and install instructions
- **[Test Suites](Plugin/Tests/)**: Comprehensive testing procedures

### WebApp-Specific (in WebApp/)
- **[WebApp Guide](WebApp/WEBAPP_README.md)**: Web application documentation

## Development Workflow

### Algorithm Development
1. **Prototype** in WebApp (JavaScript)
2. **Validate** algorithms and edge cases
3. **Port** to Plugin (C++)
4. **Cross-validate** outputs match
5. **Test** in production environments

### Testing Strategy
1. **WebApp**: Browser-based algorithm validation
2. **Plugin**: DAW integration testing
3. **Cross-validation**: Ensure identical outputs
4. **Performance**: Stress testing and benchmarks

## Version History

### Current: v0.03d (July 2025) - Complete Documentation & Bug Fixes Release
- ✅ **Accent Pattern System**: Suprasegmental accent layer with curly bracket notation `{accent}pattern`
- ✅ **Progressive Transformations**: Manual control with `>` syntax vs automatic with `@#` syntax
- ✅ **B(n,s), W(n,s), D(n,s) Notations**: Complete pattern language with Barlow, Wolrab, and Dilcue
- ✅ **Off-by-One Fixes**: Progressive transformations now correctly cycle 1-7 instead of 2-8
- ✅ **Auto-Advancement Control**: Users can choose manual triggers or automatic progression
- ✅ **Polyrhythmic Accent Cycling**: Accents cycle when different lengths from rhythm patterns
- ✅ **MIDI Accent Enhancement**: Accented notes have pitch offset and higher velocity
- ✅ **Visual Accent Indication**: Red for accented onsets, green for regular onsets
- ✅ **Streamlined UI**: Only essential parameters exposed to hosts (Use Host Transport, MIDI Note, Trigger)

### Key Milestones
- **v0.01**: Initial stable plugin release with core pattern engine
- **v0.02a**: Enhanced UI with pie chart visualization, UPI input, and major timing fixes

## Requirements

### Plugin Development
- **macOS**: 10.13+ (plugin target)
- **Xcode**: Latest version
- **JUCE**: 8.0.1+ framework
- **Architecture**: Universal (Intel + Apple Silicon)

### WebApp Development
- **Browser**: Chrome, Firefox, Safari (modern versions)
- **Server**: Python 3.6+ (development)
- **APIs**: Web Audio, Web MIDI (optional)

## Installation

### For Music Producers (Plugin)
```bash
# Quick install (if already built)
cp -R "Plugin/Builds/MacOSX/build/Release/Rhythm Pattern Explorer.component" ~/Library/Audio/Plug-Ins/Components/
cp -R "Plugin/Builds/MacOSX/build/Release/Rhythm Pattern Explorer.vst3" ~/Library/Audio/Plug-Ins/VST3/
```

### For Developers (WebApp)
```bash
cd WebApp/app/
python3 -m http.server 8000
# Open http://localhost:8000
```

## Compatibility

### Plugin Hosts (Tested)
- ✅ **Bitwig Studio**: 5.3.12 VST3 (extensively tested 20-666 BPM)
- ✅ **Logic Pro**: AU support (validated)
- 🔍 **GarageBand**: AU support (basic testing)
- 🔍 **MainStage**: AU support (basic testing)
- 🔍 **Cubase**: VST3 support (basic testing)
- 🔍 **Reaper**: AU/VST3 support (basic testing)
- 🔍 **Ableton Live**: VST3 support (basic testing)

### Browsers (WebApp)
- ✅ **Chrome/Edge**: Full feature support
- ✅ **Firefox**: Full feature support
- ✅ **Safari**: Full feature support

## Contributing

### Code Organization
- **WebApp changes**: Test in browser first
- **Plugin changes**: Ensure algorithm parity with WebApp
- **Documentation**: Update both implementations
- **Testing**: Use provided test suites

### Algorithm Development
1. Implement in WebApp first (easier debugging)
2. Validate with comprehensive test cases
3. Port to Plugin with identical behavior
4. Cross-validate outputs between implementations

## Research Foundation

Based on extensive research in:
- **Clarence Barlow's Indispensability Theory**
- **Euclidean Rhythm Algorithms (Bjorklund)**
- **Mathematical Music Theory**
- **Algorithmic Composition Techniques**
- **Real-time Audio Processing**

## Contact

- **Repository**: https://github.com/enkerli/rhythm_pattern_explorer
- **Issues**: Report via GitHub Issues
- **Email**: support@rhythmpatternexplorer.com

## License

[License information to be added]

---

**Choose Your Platform:**
- **Music Production**: Use the [Plugin](Plugin/) for professional DAW integration
- **Research/Development**: Use the [WebApp](WebApp/) for algorithm exploration
- **Both**: Both implementations share the same core algorithms and notation system