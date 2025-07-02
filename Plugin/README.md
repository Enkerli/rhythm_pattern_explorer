# Rhythm Pattern Explorer - Plugin Development

This directory contains all files related to the **Audio Plugin** implementation of Rhythm Pattern Explorer.

## Directory Structure

```
Plugin/
├── Source/                     # Core plugin source code
│   ├── PluginProcessor.cpp/h   # Main plugin implementation  
│   ├── PluginEditor.cpp/h      # User interface
│   ├── PatternEngine.cpp/h     # Pattern generation engine
│   └── UPIParser.cpp/h         # Pattern notation parser
├── RhythmPatternExplorer.jucer # JUCE project file
├── Builds/                     # JUCE-generated build files
│   ├── MacOSX/                 # macOS builds (AU, VST3, Standalone)
│   └── iOS/                    # iOS builds (AUv3 - experimental)
├── JuceLibraryCode/            # JUCE framework integration
├── Documentation/              # Plugin-specific documentation
├── Tests/                      # Comprehensive test suites
├── Archive/                    # Historical development files
└── INSTALLATION_SUCCESS.md     # Latest installation status
```

## Plugin Formats

### ✅ Working (macOS)
- **Audio Unit (AU)**: `~/Library/Audio/Plug-Ins/Components/`
- **VST3**: `~/Library/Audio/Plug-Ins/VST3/`
- **Standalone**: Independent application

### ❌ Experimental (iOS)
- **AUv3**: JUCE framework limitations (archived)

## Development Workflow

### Building the Plugin
```bash
# Open JUCE project
open RhythmPatternExplorer.jucer

# Or build from command line
cd Builds/MacOSX
xcodebuild -project "Rhythm Pattern Explorer.xcodeproj" -scheme "Rhythm Pattern Explorer - All" -configuration Release
```

### Testing
```bash
# Run automated tests
cd Tests
./TestRunner.sh

# Manual testing procedures
# See Tests/AU_StressTest.md and Tests/VST3_StressTest.md
```

### Installation
```bash
# Install to user library (automatic with Xcode build)
cp -R "Builds/MacOSX/build/Release/Rhythm Pattern Explorer.component" ~/Library/Audio/Plug-Ins/Components/
cp -R "Builds/MacOSX/build/Release/Rhythm Pattern Explorer.vst3" ~/Library/Audio/Plug-Ins/VST3/
```

## Core Features

### Pattern Generation
- **Euclidean Patterns**: Even distribution algorithms
- **Polygon Patterns**: Geometric rhythm generation
- **Random Patterns**: Controlled randomization
- **Binary Patterns**: Direct binary representation

### Progressive Transformations
- **Barlow (B)**: Indispensability-based evolution
- **Euclidean (E)**: Progressive onset addition
- **Wolrab (W)**: Anti-Barlow transformation
- **Dilcue (D)**: Anti-Euclidean transformation

### UPI Notation System
```
E(3,8)         # Basic Euclidean
E(1,8)E>8      # Progressive transformation
E(8,8)B>1      # Barlow dilution
P(5,12)+E(3,8) # Pattern combination
```

## Technical Specifications

- **Framework**: JUCE 8.0.1
- **Language**: C++17
- **Architecture**: Universal (Intel + Apple Silicon)
- **Plugin Category**: MIDI Effect
- **Real-time**: Thread-safe, low-latency
- **Host Integration**: Full transport sync, automation

## Plugin Version History

- **v2.8**: Current stable release
  - Progressive transformations working
  - Thread-safe implementation
  - Comprehensive testing framework
  - UI improvements and stability

## Dependencies

- **JUCE Framework**: Audio plugin development
- **Xcode**: macOS/iOS development environment
- **macOS 10.13+**: Minimum deployment target

## Contributing

### Code Organization
- Keep plugin logic in `Source/`
- Update documentation in `Documentation/`
- Add tests to `Tests/` directory
- Archive old experiments in `Archive/`

### Testing Requirements
- All changes must pass `TestRunner.sh`
- Manual testing in Logic Pro required
- Performance benchmarks must be maintained

## Related Projects

- **WebApp Implementation**: `../WebApp/` directory
- **Reference Algorithms**: See WebApp for mathematical implementations
- **Documentation**: Shared docs in WebApp directory

## Contact

- **Repository**: https://github.com/enkerli/rhythm_pattern_explorer
- **Issues**: Report plugin-specific issues via GitHub
- **Email**: support@rhythmpatternexplorer.com