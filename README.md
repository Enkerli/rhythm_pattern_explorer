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

### Progressive Transformations
- **Barlow (B)**: Indispensability-based pattern evolution
- **Euclidean (E)**: Progressive onset addition/removal
- **Wolrab (W)**: Anti-Barlow transformations
- **Dilcue (D)**: Anti-Euclidean transformations

### UPI (Universal Pattern Input) Notation
```
E(3,8)                    # Basic Euclidean: 10010010
E(1,8)E>8                 # Progressive: 1→11111111
E(8,8)B>1                 # Barlow dilution: 11111111→10000000
P(5,12,0)                 # Pentagon rhythm
E(3,8)+P(5,12)           # Pattern combination
E(3,8)*P(5,12)           # Pattern multiplication
R(4,8,42)                # Random with seed
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

### Current: v0.02a (July 2024) - Major Timing Fixes Release
- ✅ **Timing Issues Resolved**: Fixed step advancement and animation problems
- ✅ **BPM Range Expanded**: 20-666 BPM support (previously limited to 180 BPM)
- ✅ **Performance Validated**: Bitwig Studio 5.3.12 stress tested
- ✅ **Position Sync Fixed**: Eliminated host timeline interference
- ✅ **Clean Interface**: Streamlined UPI-focused design

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