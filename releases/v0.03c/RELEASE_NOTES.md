# Rhythm Pattern Explorer v0.03c Release Notes

## 🎵 Major New Feature: Accent Pattern System

### Revolutionary Suprasegmental Accent Layer
v0.03c introduces a **groundbreaking accent pattern system** that adds sophisticated rhythmic expression while maintaining the elegance of binary patterns. This creates a second rhythmic dimension that transforms musical possibilities.

#### **Curly Bracket Notation**
- **Flexible Placement**: `{accent}pattern` or `pattern{accent}` work identically
- **Example**: `{100}E(3,8)` or `E(3,8){100}` - accents first onset of tresillo
- **Complex Patterns**: Use any UPI notation for accents: `{E(2,5)}`, `{P(3,0)}`, `{B(3,7)}`

#### **Polyrhythmic Cycling**
When accent and rhythm patterns have different lengths, accents automatically cycle through creating sophisticated polyrhythmic interactions:

- **`{10}E(4,8)`** - Alternating accented/unaccented onsets
- **`{100100}E(3,4)`** - 6-step accent pattern cycling over 4-step rhythm
- **`{E(2,5)}E(3,8)`** - Euclidean accent pattern over Euclidean rhythm

#### **Enhanced MIDI Output**
Accented notes are distinguished by:
- **Higher Pitch**: +5 semitones above base note
- **Higher Velocity**: Maximum velocity (1.0) vs regular notes (0.8)
- **Clear Musical Distinction**: Immediately audible accent placement

#### **Visual Representation**
- **Color Coding**: Accented onsets display in **red**, regular onsets in **green**
- **Real-time Updates**: Visual feedback updates as patterns play
- **Pattern Circle**: Clear visual distinction of accent placement

### **Accent Pattern Examples**

```
# Basic accent patterns
{100}E(3,8)      # Accented tresillo - first onset emphasized
{10010}E(5,8)    # Quintillo with accents on onsets 1 and 4
{10}E(4,8)       # Alternating strong/weak feel

# Complex polyrhythmic patterns
{E(2,5)}E(3,8)   # 5-step Euclidean accents over 8-step Euclidean rhythm
{P(3,0)}E(5,8)   # All-accent pattern cycling over quintillo
{B(2,7)}W(3,11)  # Barlow accents over Wolrab rhythm

# Progressive patterns with accents
{101}E(1,8)B>8   # Progressive Barlow transformation with accent pattern
{10}P(3,0)|E(5,8){100}  # Scene cycling with different accent approaches

# Advanced combinations
{1010}E(3,8)B>5  # Progressive Barlow with alternating accents
{E(3,7)}B(5,13)|W(2,11)  # Complex scene cycling with Euclidean accents
```

## 🔧 Technical Improvements

### **Complete Pattern Language**
v0.03c includes the full spectrum of rhythm pattern notations:
- **E(n,s)** - Euclidean patterns
- **P(n,s)** - Polygon patterns  
- **R(n,s)** - Random patterns
- **B(n,s)** - Barlow indispensability patterns
- **W(n,s)** - Wolrab (anti-Barlow) patterns
- **D(n,s)** - Dilcue (anti-Euclidean) patterns
- **{accent}** - Accent patterns using any of the above

### **Code Quality Enhancements**
- **Pattern Recognition Refactor**: Table-driven approach reduces boilerplate
- **Progressive State Cleanup**: LRU mechanism prevents memory growth
- **Pattern Function Centralization**: Eliminated code duplication
- **Enhanced Debug Logging**: Category-based routing with performance optimization

### **Fixed Issues**
- **Accent Detection Logic**: Corrected to use onset positions rather than step positions
- **Progressive Transformation Looping**: Fixed transformations getting stuck at target
- **Barlow Universal Algorithm**: Proper hierarchical indispensability for all step counts
- **Hex Notation**: Strict left-to-right bit ordering implementation

## 📦 What's Included

### **Plugin Formats**
- **AU Component**: `Rhythm Pattern Explorer.component` (~8.4MB)
- **VST3 Plugin**: `Rhythm Pattern Explorer.vst3` (~8.4MB)
- **Universal Binary**: Native support for Intel and Apple Silicon Macs

### **Documentation**
- Complete installation instructions
- UPI syntax reference with accent pattern examples
- Pattern transformation examples
- DAW integration guides

## 🎼 Musical Impact

The accent system creates **sophisticated rhythmic expression**:

### **Traditional Rhythms Enhanced**
- **Tresillo**: `{100}E(3,8)` - Classic pattern with accent on first beat
- **Cinquillo**: `{10010}E(5,8)` - Cuban rhythm with traditional accent placement
- **Polyrhythms**: `{10}E(3,8)` - Creates cross-rhythm accent patterns

### **Advanced Rhythmic Concepts**
- **Metric Modulation**: Different accent/rhythm cycle lengths
- **Hierarchical Accents**: Using Barlow/Wolrab patterns for accent placement
- **Progressive Accents**: Accent patterns that evolve with transformations

### **Live Performance Ready**
- **Scene Cycling**: Different accent patterns per scene
- **Host Automation**: Trigger parameter for real-time pattern changes
- **Visual Feedback**: Immediate understanding of accent placement

## 🔄 Upgrade Notes

### **From v0.02a/v0.03b**
- **Full Compatibility**: All existing patterns continue to work unchanged
- **New Syntax**: Accent patterns are purely additive - no breaking changes
- **Enhanced Expression**: Add accents to existing patterns instantly

### **Installation**
1. Copy plugins to user library (NOT system library):
   - AU: `~/Library/Audio/Plug-Ins/Components/`
   - VST3: `~/Library/Audio/Plug-Ins/VST3/`
2. Restart your DAW
3. Explore accent patterns with familiar rhythm patterns!

## 🎯 Use Cases

### **Music Production**
- **Drum Programming**: Create dynamic, expressive drum patterns
- **Rhythmic Sketching**: Quickly prototype complex rhythmic ideas
- **Pattern Variation**: Add accent layers to existing compositions

### **Live Performance**
- **Real-time Control**: Host automation of accent patterns
- **Visual Feedback**: Clear indication of pattern structure
- **Scene Management**: Pre-programmed accent/rhythm combinations

### **Music Education**
- **Rhythm Analysis**: Visual representation of accent placement
- **Polyrhythm Study**: Explore complex rhythmic relationships
- **Cultural Rhythms**: Implement traditional accent patterns

## 🚀 Performance

- **Optimized MIDI Output**: Minimal latency for accent detection
- **Memory Management**: Bounded progressive state storage
- **Visual Efficiency**: Smooth real-time pattern display
- **Thread Safety**: Proper synchronization between audio and UI threads

## 🎛️ DAW Compatibility

Tested and compatible with:
- Logic Pro X/Pro (excellent AU support)
- Ableton Live (full VST3 integration)
- Bitwig Studio (advanced automation support)
- Cubase/Nuendo (professional workflow integration)
- Pro Tools (VST3 support in 2020.1+)
- Reaper (flexible routing capabilities)
- Studio One (drag-and-drop workflow)

## 📅 Build Information

- **Build Date**: July 7, 2025
- **Framework**: JUCE 8.0.8
- **License**: CC0 1.0 Universal (Public Domain)
- **macOS**: 10.13+ (High Sierra and newer)
- **Architecture**: Universal Binary (Intel + Apple Silicon)

## 🎵 Getting Started

1. **Load the plugin** as a MIDI effect in your DAW
2. **Enter a basic pattern**: `E(3,8)` (tresillo)
3. **Add accents**: `{100}E(3,8)` (accent first onset)
4. **Route MIDI output** to your drum machine/sampler
5. **Experiment with polyrhythms**: `{10}E(3,8)` (alternating accents)

## 🌟 What's Next

The accent system opens up infinite rhythmic possibilities. Experiment with:
- Complex accent cycling patterns
- Progressive transformations with accent layers
- Scene-based accent/rhythm combinations
- Traditional world music accent patterns

---

**Download Size**: ~16MB total
**Installation Time**: < 1 minute  
**Learning Curve**: Immediate for existing users, comprehensive for newcomers

**Rhythm Pattern Explorer v0.03c** - Where mathematical precision meets musical expression! 🎵✨