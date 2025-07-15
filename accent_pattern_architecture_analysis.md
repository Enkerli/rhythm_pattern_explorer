# Accent Pattern System Architecture Analysis

## Overview
This document provides a comprehensive technical analysis of the accent pattern system in the Rhythm Pattern Explorer JUCE plugin, focusing on the data flow from parsing to playback to display, and identifying potential issues with scene switching and UI synchronization.

## System Architecture

### 1. Data Structures and Storage

#### Core Accent Data (PluginProcessor.h, lines 218-222)
```cpp
bool hasAccentPattern = false;
std::vector<bool> currentAccentPattern;
juce::String currentAccentPatternName;
int globalAccentPosition = 0;  // Global accent position counter (persists across pattern cycles)
```

**Key Design Decisions:**
- **Global Accent Position**: Persists across pattern cycles for polyrhythmic cycling
- **Separate from Rhythm Pattern**: Accent patterns are independent overlay system
- **Boolean Vector**: Simple binary representation matching rhythm pattern structure

#### Per-Scene Accent Storage
The system does NOT store accent patterns per-scene. This is a critical architectural limitation:
- `scenePatterns` (line 235) stores full UPI strings including accent syntax
- No dedicated `sceneAccentPatterns` vector
- Each scene re-parses accent information when activated

### 2. Parsing and Extraction (UPIParser.cpp)

#### Accent Pattern Detection (lines 1105-1108)
```cpp
bool UPIParser::hasAccentPattern(const juce::String& input)
{
    return input.contains("{") && input.contains("}");
}
```

#### Extraction Process (lines 1119-1130)
```cpp
juce::String UPIParser::extractAccentPattern(const juce::String& input)
{
    int start = input.indexOfChar('{');
    int end = input.indexOfChar('}');
    
    if (start >= 0 && end > start)
    {
        return input.substring(start + 1, end);
    }
    
    return "";
}
```

#### Pattern Removal (lines 1132-1144)
```cpp
juce::String UPIParser::removeAccentPattern(const juce::String& input)
{
    int start = input.indexOfChar('{');
    int end = input.indexOfChar('}');
    
    if (start >= 0 && end > start)
    {
        // Remove the entire {accent} portion
        return input.substring(0, start) + input.substring(end + 1);
    }
    
    return input;
}
```

**Critical Flow Issue**: The parsing happens in sequence:
1. Extract accent pattern → Parse accent
2. Remove accent from base → Parse rhythm pattern
3. Attach accent to result

### 3. Accent Pattern Parsing (UPIParser.cpp, lines 1160-1190)

Accent patterns support full UPI syntax recursively:
- **Binary patterns**: `{100}`, `{10010}`, `{101}`
- **Algorithmic patterns**: `{E(2,5)}`, `{P(3,0)}`, `{B(3,8)}`
- **Numeric patterns**: `{0x5:8}`, `{42:8}`, `{[0,2]:8}`

**Design Decision**: Accent patterns are independent and can be different lengths than rhythm patterns, creating polyrhythmic accent cycling.

### 4. Scene Switching Logic (PluginProcessor.cpp)

#### Scene Detection and Storage (lines 745-850)
```cpp
if (isScenes)
{
    // Handle scene cycling: pattern1|pattern2|pattern3
    // CRITICAL: Split scenes BEFORE any UPI parsing to preserve individual accent patterns
    auto scenes = juce::StringArray::fromTokens(pattern, "|", "");
    
    // ...initialization logic...
    
    // CRITICAL: Store original scene patterns with accent syntax intact
    for (const auto& scene : scenes) {
        juce::String scenePattern = scene.trim();
        scenePatterns.add(scenePattern);
        // ...per-scene progressive state tracking...
    }
}
```

#### Scene Application (lines 1299-1342)
```cpp
void RhythmPatternExplorerAudioProcessor::applyCurrentScenePattern()
{
    // Apply the current scene pattern using per-scene progressive state
    if (currentSceneIndex >= static_cast<int>(sceneBasePatterns.size())) {
        return; // Safety check
    }
    
    juce::String basePattern = sceneBasePatterns[currentSceneIndex];
    
    // Parse the base pattern first
    parseAndApplyUPI(basePattern);  // ← POTENTIAL ISSUE: Re-parses accent
}
```

### 5. Global Accent Position Management

#### Reset Logic (PluginProcessor.cpp, lines 724-726, 961-963)
```cpp
// In setUPIInput():
globalAccentPosition = 0;

// In parseAndApplyUPI():
if (resetAccentPosition) {
    globalAccentPosition = 0;
}
```

#### Advancement Logic (lines 579-585)
```cpp
if (hasAccentPattern && !currentAccentPattern.empty())
{
    // Use global accent position (persists across pattern cycles)
    int accentStep = globalAccentPosition % currentAccentPattern.size();
    shouldAccent = currentAccentPattern[accentStep];
    
    // Increment global accent position for next onset
    globalAccentPosition++;
}
```

### 6. UI Display Logic (PluginEditor.cpp)

#### Accent Visualization (lines 627-662)
```cpp
// Check if this step should be accented - stable per cycle display
bool isAccented = false;
if (audioProcessor.getHasAccentPattern() && !audioProcessor.getCurrentAccentPattern().empty())
{
    // Find which onset number this step is (count onsets up to current step)
    int onsetIndex = 0;
    for (int j = 0; j < i; ++j)
    {
        if (j < pattern.size() && pattern[j])
        {
            onsetIndex++;
        }
    }
    
    // SIMPLIFIED: Use global accent position directly for stable cycle-based display
    int globalAccentPosition = audioProcessor.getGlobalAccentPosition();
    
    // Complex cycle calculation follows...
}
```

#### Visual Rendering (lines 664-730)
Accented onsets are displayed as split slices:
- **Inner part**: Regular green color (innerRadius to markerRadius)
- **Outer part**: Inverted background color (markerRadius to outerRadius)

## Critical Issues Identified

### 1. Scene Switching Accent Display Problem

**Root Cause**: The UI accent display calculation is overly complex and makes assumptions about cycle alignment that break during scene switches.

**Specific Issue** (PluginEditor.cpp, lines 640-661):
```cpp
// Calculate which cycle we're in based on global accent position
int currentAccentCycle = 0;
if (onsetsInPattern > 0) {
    currentAccentCycle = globalAccentPosition / onsetsInPattern;
}

// Calculate the accent offset for this cycle
int displayAccentOffset = (currentAccentCycle * onsetsInPattern) % audioProcessor.getCurrentAccentPattern().size();
```

**Problem**: When switching scenes:
1. New scene may have different rhythm pattern (different `onsetsInPattern`)
2. `globalAccentPosition` continues from previous scene
3. Cycle calculation becomes invalid
4. Display shows incorrect accent positions

### 2. Global Accent Position Reset Inconsistency

**Issue**: `globalAccentPosition` is reset in different scenarios:
- **Always reset**: New UPI input (`setUPIInput`, line 726)
- **Conditionally reset**: Re-parsing (`parseAndApplyUPI`, lines 961-963)
- **Not reset**: Scene advancement (`applyCurrentScenePattern`)

**Consequence**: Scene switches preserve accent position but UI display assumes fresh start.

### 3. Scene Accent Pattern Re-parsing

**Issue**: Each scene switch calls `parseAndApplyUPI(basePattern)` which:
1. Re-extracts accent pattern from stored UPI string
2. Re-parses accent pattern structure
3. Updates `currentAccentPattern` and related fields

**Potential Problem**: If accent parsing fails or differs slightly, accent display becomes inconsistent.

### 4. UI Update Timing

**Issue**: The UI accent display calculation happens during `drawPatternCircle` (paint cycle) but depends on:
- `audioProcessor.getGlobalAccentPosition()` (audio thread)
- `audioProcessor.getCurrentAccentPattern()` (audio thread)
- Complex cycle math that can be expensive

**Consequence**: UI performance impact and potential thread safety issues.

## Recommended Solutions

### 1. Simplify UI Accent Display

Replace complex cycle calculation with direct onset-based lookup:

```cpp
// Simplified approach
bool isAccented = false;
if (audioProcessor.getHasAccentPattern() && !audioProcessor.getCurrentAccentPattern().empty())
{
    // Count onsets up to this position to get onset index
    int onsetIndex = 0;
    for (int j = 0; j < i; ++j) {
        if (j < pattern.size() && pattern[j]) {
            onsetIndex++;
        }
    }
    
    // Use current cycle's accent position directly
    int cycleAccentStart = audioProcessor.getCurrentCycleAccentStart();
    int accentStep = (cycleAccentStart + onsetIndex) % audioProcessor.getCurrentAccentPattern().size();
    isAccented = audioProcessor.getCurrentAccentPattern()[accentStep];
}
```

### 2. Add Per-Scene Accent State Caching

Store parsed accent patterns per scene to avoid re-parsing:

```cpp
// In PluginProcessor.h
std::vector<std::vector<bool>> sceneAccentPatterns;
std::vector<juce::String> sceneAccentPatternNames;
std::vector<bool> sceneHasAccent;
```

### 3. Implement Accent Position Scene Management

Add scene-aware accent position management:

```cpp
// Reset accent position only for new scene sequences, not scene advancement
if (isSameSequence && !scenePatterns.isEmpty())
{
    // Same sequence - advance to next scene
    advanceScene();
    // DON'T reset globalAccentPosition - let it continue
}
else
{
    // New scene sequence - reset accent state
    globalAccentPosition = 0;
    // ... existing reset logic
}
```

### 4. Add UI State Validation

Add defensive checks in accent display:

```cpp
// Validate state before complex calculations
if (!audioProcessor.getHasAccentPattern() || 
    audioProcessor.getCurrentAccentPattern().empty() ||
    pattern.empty()) {
    isAccented = false;
    continue;
}

// Ensure globalAccentPosition is reasonable
int globalPos = audioProcessor.getGlobalAccentPosition();
if (globalPos < 0) {
    isAccented = false;
    continue;
}
```

## Summary

The accent pattern system is architecturally sound but has implementation issues around scene switching. The core problem is that the UI display logic makes assumptions about accent position alignment that break when scenes have different rhythm structures. The solution involves simplifying the UI calculation and adding better state management for scene transitions.

The system correctly implements:
- ✅ Accent pattern parsing and storage
- ✅ Global accent position advancement
- ✅ MIDI accent output (pitch offset and velocity)
- ✅ Basic UI accent visualization

The system needs improvement in:
- ❌ Scene switching accent display synchronization
- ❌ UI accent position calculation complexity
- ❌ Per-scene accent state caching
- ❌ Thread safety between audio and UI threads