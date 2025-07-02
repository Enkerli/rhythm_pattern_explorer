# Rhythm Pattern Explorer Plugin - Current Status

## Version: 2.8
**Last Updated:** July 2, 2025  
**Build Status:** ✅ Working (macOS AU/VST3)

## Current Features

### Core Functionality
- **MIDI Effect Plugin**: Generates rhythmic MIDI patterns without audio processing
- **Real-time Pattern Generation**: Responds to host transport and timing
- **Progressive Transformations**: Advanced algorithmic pattern evolution

### Pattern Types Supported
1. **Euclidean Patterns**: `E(onsets, steps, rotation)`
   - Bjorklund algorithm implementation
   - Even distribution of onsets across steps
   
2. **Polygon Patterns**: `P(sides, vertices, rotation)`
   - Geometric rhythm generation
   - Based on regular polygon vertex positions
   
3. **Random Patterns**: `R(onsets, steps, seed)`
   - Controlled randomization with optional seed
   
4. **Binary Patterns**: `B(binary_value, steps)`
   - Direct binary representation of rhythms

### Progressive Transformations
- **Barlow (B)**: Indispensability-based pattern evolution
  - Uses prime factorization for onset weighting
  - Concentration: adds most important onsets
  - Dilution: removes least important onsets
  
- **Euclidean (E)**: Progressive Euclidean generation
  - Step-by-step approach to even distribution
  
- **Wolrab (W)**: Anti-Barlow transformation
  - Inverse of Barlow indispensability
  
- **Dilcue (D)**: Anti-Euclidean transformation
  - Inverse of Euclidean distribution

### UPI (Universal Pattern Input) Parser
Advanced pattern notation supporting:
- **Basic Patterns**: `E(3,8)`, `P(5,12)`
- **Combinations**: `E(3,8)+P(5,12)`, `E(3,8)*P(5,12)`
- **Progressive Sequences**: `E(1,8)E>8` (1 to 8 onsets progressively)
- **Complex Chains**: `E(8,8)B>1` (8 onsets reduced via Barlow)

## Technical Implementation

### Architecture
- **Plugin Format**: JUCE AudioProcessor
- **Threading**: Thread-safe with critical sections
- **Parameters**: AudioProcessorValueTreeState integration
- **MIDI**: Real-time MIDI generation and timing

### Key Classes
- **`RhythmPatternExplorerAudioProcessor`**: Main plugin class
- **`PatternEngine`**: Core pattern generation and analysis
- **`UPIParser`**: Pattern notation parser and transformer
- **`RhythmPatternExplorerAudioProcessorEditor`**: Plugin UI

### Files Structure
```
Source/
├── PluginProcessor.cpp/h     # Main plugin implementation
├── PluginEditor.cpp/h        # User interface
├── PatternEngine.cpp/h       # Pattern generation core
└── UPIParser.cpp/h          # Pattern notation parser
```

## Plugin Formats

### ✅ Working Formats
- **Audio Unit (AU)**: Fully functional on macOS
- **VST3**: Fully functional on macOS
- **Standalone**: Working standalone application

### ❌ Current Issues
- **AUv3 (iOS)**: JUCE framework issues causing crashes
  - NSException in view constraints
  - Memory management issues
  - Known JUCE v8.0.1 iOS compatibility problems

## Performance Characteristics

### Tested Scenarios
- ✅ Host sync with various DAWs (Logic Pro, Ableton Live, Cubase)
- ✅ Real-time parameter changes
- ✅ Progressive transformations up to 64 steps
- ✅ Complex UPI pattern parsing
- ✅ Multiple plugin instances

### Resource Usage
- **CPU**: Minimal (<1% on modern systems)
- **Memory**: ~10MB typical usage
- **Latency**: <1ms processing time

## Known Working Configurations

### DAW Compatibility
- **Logic Pro X/11**: Full AU/VST3 support
- **Ableton Live**: VST3 support
- **Cubase/Nuendo**: VST3 support
- **Reaper**: AU/VST3 support
- **GarageBand**: AU support

### Pattern Examples (Tested)
```
E(3,8)         # Basic Euclidean: 10010010
E(1,8)E>8      # Progressive Euclidean: 1→8 onsets
E(8,8)B>1      # Barlow dilution: 8→1 onsets
P(5,12,0)      # Pentagon rhythm in 12 steps
E(3,8)+P(3,8)  # Pattern combination
```

## Development History

### Major Milestones
1. **Initial Implementation**: Basic Euclidean and polygon patterns
2. **UPI Parser**: Advanced pattern notation system
3. **Progressive Transformations**: Barlow and anti-patterns
4. **Threading Safety**: Resolved race conditions
5. **UI Improvements**: Resizable interface, circular display
6. **iOS Attempts**: Multiple approaches, JUCE limitations encountered

### Bug Fixes
- ✅ Progressive stepping (was starting on step 2)
- ✅ Barlow ending patterns (32+ step robustness)
- ✅ Thread safety in parameter updates
- ✅ MIDI timing precision
- ✅ Memory leaks in pattern generation

## Future Development

### Planned Improvements
- [ ] iOS implementation (waiting for JUCE updates)
- [ ] Preset system for pattern libraries
- [ ] MIDI CC control for real-time parameter changes
- [ ] Advanced visualization modes
- [ ] Pattern morphing capabilities
- [ ] Swing/groove templates

### iOS Strategy
Given current JUCE AUv3 issues:
1. Monitor JUCE updates for iOS fixes
2. Consider alternative frameworks (AudioKit, native iOS)
3. Standalone iOS app with AudioBus/IAA
4. Web-based solution optimization

## Installation

### macOS Plugin Installation
Plugins are installed to:
- **AU**: `~/Library/Audio/Plug-Ins/Components/`
- **VST3**: `~/Library/Audio/Plug-Ins/VST3/`

### Building from Source
1. Open `RhythmPatternExplorer.jucer` in Projucer
2. Generate Xcode project
3. Build in Xcode (Release configuration recommended)
4. Plugins automatically copied to system directories

## Support

### Documentation
- [UPI Pattern Reference](PROGRESSIVE_TRANSFORMATION_REFERENCE.md)
- [MIDI Setup Guide](MIDI-SETUP-GUIDE.md)
- [Working Solutions](WORKING-MIDI-SOLUTIONS.md)

### Contact
- **Repository**: https://github.com/enkerli/rhythm_pattern_explorer
- **Issues**: Report via GitHub Issues
- **Email**: support@rhythmpatternexplorer.com