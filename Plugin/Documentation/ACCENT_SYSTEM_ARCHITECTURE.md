# Accent System Architecture - Definitive Implementation Guide

## Executive Summary

This document provides an ironclad blueprint for implementing the single source of truth accent system in the Rhythm Pattern Explorer plugin. This system successfully provides stable visual accent feedback while maintaining perfect synchronization between UI visualization and MIDI output.

**Critical Warning**: This implementation was achieved after extensive trial and error, multiple complete rewrites, and several epic regressions. The architectural decisions documented here are the result of hard-learned lessons and must be preserved exactly as specified.

## Core Architectural Principles

### 1. Single Source of Truth with Dual Perspectives

The system maintains **one authoritative onset counter** (`globalOnsetCounter`) but provides **two different views** of accent state:

- **Real-time perspective**: For MIDI output (updates on every onset)
- **Stable perspective**: For UI visualization (updates only at rhythm cycle boundaries)

**Key Insight**: This is NOT a dual-offset system. It's a single counter with two viewing windows into the accent pattern.

### 2. Layered Cycles Concept

The fundamental insight that solved the polyrhythmic accent problem:

```
Rhythm Pattern: [1 0 0 1 0 0 1 0] (8 steps, cycles every 8 steps)
Accent Pattern: [1 0 1]           (3 steps, cycles every 3 steps)

Like two people counting independently:
- Person A: "1-2-3-4-5-6-7-8, 1-2-3-4-5-6-7-8, ..." (rhythm cycle)
- Person B: "1-2-3, 1-2-3, 1-2-3, 1-2-3, ..." (accent cycle)

They align differently each rhythm cycle, creating polyrhythmic patterns.
```

### 3. Windowing Effect for UI Display

**Critical Realization**: The complete accent map for all onsets is deterministic and known in advance. The UI only displays a "window" into this infinite sequence.

```
Complete accent sequence: [1,0,1,1,0,1,1,0,1,1,0,1,...]
Rhythm cycle 1 window:    [1,0,1] (steps 0,3,6 of rhythm)
Rhythm cycle 2 window:    [1,0,1] (steps 0,3,6 of rhythm, but different accents)
```

The window content changes not because the accent logic changes, but because we're viewing different portions of the infinite accent sequence.

## Implementation Architecture

### Core Data Structures

```cpp
// PluginProcessor.h - Accent system variables
bool hasAccentPattern = false;
std::vector<bool> currentAccentPattern;
int globalOnsetCounter = 0;           // Single source of truth: counts all onsets since pattern start
int uiAccentOffset = 0;               // Stable accent offset for UI display (updates only at cycle boundaries)
```

**Critical**: Both counters must be reset simultaneously on manual triggers to maintain synchronization.

### Key Methods and Their Responsibilities

#### 1. Pattern Parsing and Setup (`parseAndApplyUPI`)

```cpp
// Extract accent pattern from UPI input like {101}E(3,8)
if (parseResult.hasAccentPattern) {
    hasAccentPattern = true;
    currentAccentPattern = parseResult.accentPattern;
} else {
    hasAccentPattern = false;
    currentAccentPattern.clear();
}

// CRITICAL: Reset both counters for fresh start
globalOnsetCounter = 0;
uiAccentOffset = 0;
```

#### 2. Real-time MIDI Processing (`processStep`)

```cpp
if (stepToProcess < pattern.size() && pattern[stepToProcess]) {
    // Use real-time counter for immediate MIDI accent decision
    bool isAccented = shouldOnsetBeAccented(globalOnsetCounter);
    
    triggerNote(midiBuffer, samplePosition, isAccented);
    
    // Advance real-time counter
    globalOnsetCounter++;
}

// Update UI counter only at cycle boundaries
int nextStep = (stepToProcess + 1) % static_cast<int>(pattern.size());
if (nextStep == 0) {
    // Update stable UI accent offset at cycle boundaries
    if (hasAccentPattern && !currentAccentPattern.empty()) {
        uiAccentOffset = globalOnsetCounter % static_cast<int>(currentAccentPattern.size());
    }
    patternChanged.store(true); // Notify UI of cycle completion
}
```

#### 3. Accent Decision Logic (`shouldOnsetBeAccented`)

```cpp
bool shouldOnsetBeAccented(int onsetNumber) const {
    if (!hasAccentPattern || currentAccentPattern.empty())
        return false;
    
    // Simple layered cycle: which position in accent pattern cycle?
    int accentPosition = onsetNumber % static_cast<int>(currentAccentPattern.size());
    return currentAccentPattern[accentPosition];
}
```

#### 4. UI Visualization (`getCurrentAccentMap`)

```cpp
std::vector<bool> getCurrentAccentMap() const {
    // Use STABLE UI offset that only updates at cycle boundaries
    int onsetNumber = uiAccentOffset; // NOT globalOnsetCounter!
    
    for (int stepInCycle = 0; stepInCycle < currentPattern.size(); ++stepInCycle) {
        if (currentPattern[stepInCycle]) {
            // This step will have an onset - will it be accented?
            accentMap[stepInCycle] = shouldOnsetBeAccented(onsetNumber);
            onsetNumber++;
        }
    }
    return accentMap;
}
```

## Critical Implementation Details

### 1. When Counters Update

| Event | globalOnsetCounter | uiAccentOffset | UI Refresh |
|-------|-------------------|----------------|------------|
| Onset occurs | ✓ Increments | No change | No |
| Cycle boundary | No change | ✓ Updates | ✓ Yes |
| Manual trigger | ✓ Reset to 0 | ✓ Reset to 0 | ✓ Yes |
| Pattern change | ✓ Reset to 0 | ✓ Reset to 0 | ✓ Yes |

### 2. UI Refresh Timing

**Problem Solved**: UI was refreshing on every onset (16ms timer + onset changes), causing "flashing lights" effect.

**Solution**: UI accent map uses stable `uiAccentOffset` that only changes at cycle boundaries, not the real-time `globalOnsetCounter`.

### 3. Manual Trigger Consistency

All manual triggers MUST reset both counters to maintain synchronization:
- Enter key presses
- MIDI input triggers  
- Tick parameter changes
- Scene cycling
- Progressive pattern advancement

```cpp
// Example from parseAndApplyUPI
globalOnsetCounter = 0;
uiAccentOffset = 0;
patternChanged.store(true);
```

## Supported Accent Pattern Types

The system supports all UPI pattern types within curly braces:

```cpp
{101}E(3,8)           // Binary accent pattern
{E(2,5)}E(3,8)        // Euclidean accent pattern  
{B(3,7)}E(5,13)       // Barlow accent pattern
{W(2,5)}E(4,9)        // Wolrab accent pattern
{.-.}E(3,8)           // Morse code accent pattern
{0x5:4}E(3,8)         // Hex accent pattern
```

## Parameter Integration

Three accent parameters exposed to host:

```cpp
// Parameter declarations
juce::AudioParameterFloat* accentVelocityParam;      // 0.0-1.0, default 1.0
juce::AudioParameterFloat* unaccentedVelocityParam;  // 0.0-1.0, default 0.8  
juce::AudioParameterInt* accentPitchOffsetParam;     // -12 to +12, default +5

// Usage in triggerNote
if (isAccented && hasAccentPattern) {
    velocity = accentVelocityParam->get();
    noteNumber = baseNoteNumber + accentPitchOffsetParam->get();
} else {
    velocity = unaccentedVelocityParam->get();
}
```

## Visual Feedback

UI visualization uses red for accented onsets, green for regular onsets:

```cpp
// In PluginEditor.cpp drawPatternCircle
auto accentMap = audioProcessor.getCurrentAccentMap();
bool isAccented = (i < accentMap.size()) ? accentMap[i] : false;

if (isAccented) {
    g.setColour(juce::Colour(0xffdc143c));  // Red for accented onsets
} else {
    g.setColour(juce::Colour(0xff48bb78));  // Green for regular onsets
}
```

## Common Pitfalls and How They Were Solved

### 1. The "Swirling Effect" (Initial Problem)

**Symptom**: Accent markers moved every step during playback
**Cause**: UI was using real-time `globalOnsetCounter` 
**Solution**: Implemented stable `uiAccentOffset` that only updates at cycle boundaries

### 2. The "Flashing Lights" Effect (Intermediate Problem)  

**Symptom**: UI refreshed on every onset instead of every cycle
**Cause**: UI timer refreshing every 16ms + accent map recalculation on every onset
**Solution**: UI accent map uses stable offset, only changes at cycle boundaries

### 3. MIDI/UI Desynchronization (Multiple Occurrences)

**Symptom**: MIDI accents didn't match visual accent indicators
**Cause**: Different logic paths for MIDI and UI accent calculation
**Solution**: Both use same `shouldOnsetBeAccented()` method, but with different offset inputs

### 4. Manual Trigger Inconsistency (Recurring Issue)

**Symptom**: Manual triggers didn't reset accent system properly
**Cause**: Only resetting one counter instead of both
**Solution**: All manual trigger paths reset both `globalOnsetCounter` and `uiAccentOffset`

## Testing Scenarios

The following scenarios must work flawlessly:

1. **Basic Polyrhythm**: `{10}E(3,8)` - alternating accents on tresillo
2. **Complex Polyrhythm**: `{101}E(5,13)` - 3-step accents on 5/13 pattern
3. **Scene Cycling**: `{10}E(3,8)|{01}E(5,16)` - accent patterns change with scenes
4. **Progressive + Accents**: `{101}E(1,8)>8` - accents maintained during progression
5. **Transport Jumps**: Scrubbing in DAW maintains accent synchronization
6. **Manual Triggers**: Enter, MIDI, Tick all reset accent system consistently

## Preservation Notes

This implementation represents the solution to a complex timing and synchronization problem that took extensive development time and multiple complete rewrites. The key insights include:

1. **Separation of Concerns**: Real-time MIDI vs. stable UI perspectives
2. **Timing Precision**: When exactly counters update matters critically  
3. **Windowing Concept**: UI displays a moving window into infinite accent sequence
4. **Reset Consistency**: All manual triggers must reset both counters
5. **Cycle Boundary Updates**: UI updates only when rhythm cycle completes

**Critical**: Any deviation from this architecture risks reintroducing the timing issues that plagued earlier implementations. The polyrhythmic nature of accents combined with real-time audio processing creates subtle timing dependencies that must be preserved exactly as documented.

## File Locations

- **Core Logic**: `Plugin/Source/PluginProcessor.cpp` (lines 661-669, 1549-1563)
- **Data Structures**: `Plugin/Source/PluginProcessor.h` (lines 255-259)
- **Pattern Parsing**: `Plugin/Source/UPIParser.cpp` (parseAccentPattern method)
- **UI Integration**: `Plugin/Source/PluginEditor.cpp` (drawPatternCircle method)

This architecture successfully provides stable, synchronized accent feedback for complex polyrhythmic patterns while maintaining perfect MIDI timing accuracy.