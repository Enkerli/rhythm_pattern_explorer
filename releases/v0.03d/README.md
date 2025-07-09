# Rhythm Pattern Explorer v0.03d - Release Notes

**Complete Documentation & Bug Fixes Release**  
*Released: January 2025*

## Installation

### Quick Installation
1. **AU Plugin**: Copy `Rhythm Pattern Explorer.component` to `~/Library/Audio/Plug-Ins/Components/`
2. **VST3 Plugin**: Copy `Rhythm Pattern Explorer.vst3` to `~/Library/Audio/Plug-Ins/VST3/`

### System Requirements
- **macOS**: 10.13 or later
- **Architecture**: Universal (Intel + Apple Silicon)
- **Plugin Formats**: Audio Unit (AU), VST3
- **DAW Compatibility**: Logic Pro, GarageBand, Bitwig Studio, Cubase, Reaper, Ableton Live, and more

## What's New in v0.03d

### 🐛 Bug Fixes
- **Fixed off-by-one error in progressive transformations**: E(1,7)E>7 now correctly cycles 1-7 instead of 2-8
- **Fixed UI step counter display**: Removed duplicate +1 that was causing incorrect step numbers
- **Improved progressive transformation looping**: Patterns now cycle smoothly from base to target and back

### 📚 Complete Documentation Overhaul
- **Comprehensive README**: Updated with accurate feature descriptions and complete UPI syntax reference
- **In-Plugin Documentation**: Enhanced WebView documentation with detailed usage examples
- **Source Code Comments**: Added extensive comments explaining algorithm decisions and implementation details
- **Technical Documentation**: Created detailed progressive transformation logic documentation

### 🔧 Enhanced Features
- **Accent Pattern System**: Suprasegmental accent layer with curly bracket notation `{accent}pattern`
- **Progressive Transformations**: Manual control with `>` syntax vs automatic with `@#` syntax
- **Complete Pattern Language**: B(n,s), W(n,s), D(n,s) notations for Barlow, Wolrab, and Dilcue patterns
- **Polyrhythmic Accent Cycling**: Accents cycle when different lengths from rhythm patterns
- **MIDI Accent Enhancement**: Accented notes have pitch offset and higher velocity
- **Visual Accent Indication**: Red for accented onsets, green for regular onsets

## UPI Syntax Reference

### Basic Pattern Generation
```
E(3,8)                    # Euclidean: 3 onsets in 8 steps → 10010010
B(3,8)                    # Barlow indispensability: 3 onsets in 8 steps → 10001010
W(3,8)                    # Wolrab (anti-Barlow): 3 onsets in 8 steps → 01100100
D(3,8)                    # Dilcue (anti-Euclidean): 3 onsets in 8 steps → 01101001
P(5,12,0)                 # Pentagon rhythm: 5 vertices in 12 steps with 0 offset
R(4,8,42)                 # Random: 4 onsets in 8 steps with seed 42
```

### Progressive Transformations
```
E(1,8)E>8                 # Progressive Euclidean: 1→8 onsets (manual trigger)
E(8,8)B>1                 # Progressive Barlow dilution: 8→1 onsets (manual trigger)
B(3,13)W>11               # Progressive Barlow to Wolrab: 3→11 onsets
E(1,8)@3                  # Progressive offset: rotation by 3 steps per trigger
```

### Accent Patterns
```
{100}E(3,8)               # Accented tresillo: accent on first onset
{10010}E(5,8)             # Quintillo with accents on 1st and 4th onsets
{10}E(4,8)                # Alternating strong/weak accents (polyrhythmic)
{E(2,5)}E(3,8)            # Euclidean accent pattern over Euclidean rhythm
{P(3,0)}B(5,13)           # All-accent pattern cycling over Barlow rhythm
```

### Scene Cycling
```
E(3,8)|P(5,12)            # Scene cycling: alternate between tresillo and pentagon
100|110|101               # Scene cycling: alternate between binary patterns
{100}E(3,8)|{010}B(5,13)  # Scene cycling with different accent patterns
```

### Pattern Combinations
```
E(3,8)+P(5,12)            # Pattern combination: union of onsets
E(3,8)-P(5,12)            # Pattern subtraction: remove overlapping onsets
P(3,0)+P(5,0)-P(2,0)      # Complex combination for perfect balance
```

### Binary and Numeric Patterns
```
101010                    # Binary pattern: direct onset specification
0x2A:8                    # Hexadecimal: 42 in hex spread over 8 steps
0o52:8                    # Octal: 42 in octal spread over 8 steps
42:8                      # Decimal: 42 spread over 8 steps
[0,2,4,6]:8               # Onset array: specific positions in 8 steps
```

## Getting Started

1. **Install the plugin** in your DAW
2. **Open the plugin** in your DAW's track
3. **Enter a pattern** in the UPI field (try `E(3,8)` for tresillo)
4. **Click the Documentation button** to access the complete reference
5. **Experiment with progressions** like `E(1,8)E>8` and use the trigger button

## Technical Features

- **Manual Progressive Control**: Use `>` syntax for precise control over pattern evolution
- **Accent System**: Independent accent patterns that cycle polyrhythmically
- **Scene Cycling**: Chain multiple patterns for complex sequences
- **MIDI Integration**: Full MIDI input/output with accent enhancements
- **Visual Feedback**: Clear indication of accented vs regular onsets
- **Streamlined Interface**: Only essential parameters exposed to host

## Support

- **Documentation**: Access comprehensive docs via the plugin's Documentation button
- **Issues**: Report issues at https://github.com/enkerli/rhythm_pattern_explorer/issues
- **Project**: https://github.com/enkerli/rhythm_pattern_explorer

## Credits

Based on extensive research in:
- Clarence Barlow's Indispensability Theory
- Euclidean Rhythm Algorithms (Bjorklund)
- Mathematical Music Theory
- Algorithmic Composition Techniques

## License

CC0 1.0 Universal (Public Domain)