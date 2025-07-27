# Rhythm Pattern Explorer - Complete State Inventory

## Analysis Date: 2025-01-27
## Purpose: Comprehensive documentation of all state variables requiring persistence

---

## Currently Saved State (Basic XML)

### ✅ Basic Parameters (4 variables)
- `currentBPM` (float) - Internal BPM override
- `internalPlaying` (bool) - Internal play state
- `useHostTransport` (bool) - Host transport sync mode  
- `midiNote` (int) - MIDI note number for triggers

### ✅ Basic Pattern Data (2 variables)
- Binary pattern data (std::vector<bool>) - Serialized as string "10110100"
- `currentUPIInput` (String) - Current UPI pattern string

---

## Missing Critical State (Causes Pattern Loss/Corruption)

### 🔴 HIGH PRIORITY - Pattern Core (8 variables)
- `originalUPIInput` (String) - **CRITICAL**: Preserves progressive/scene syntax for tick/MIDI triggers
- `lastParsedUPI` (String) - Last successfully parsed pattern
- `currentProgressivePatternKey` (String) - Progressive pattern tracking key
- `basePattern` (String) - Base pattern without progressive syntax
- `baseLengthPattern` (std::vector<bool>) - Original pattern for lengthening operations
- Pattern timing parameters:
  - `patternLengthUnitParam->getIndex()` (int) - Steps/Beats/Bars mode
  - `patternLengthValueParam->getIndex()` (int) - Pattern length value  
  - `subdivisionParam->getIndex()` (int) - Subdivision selection

### 🔴 HIGH PRIORITY - Progressive Transformation State (Complex Structure)
**ProgressiveManager::progressiveStates** (std::map<String, ProgressiveState>)
- Map key: pattern string (e.g., "E(1,8)>8")
- Per-pattern state:
  - `basePattern` (String) - Original pattern
  - Progressive Offset (+N syntax):
    - `hasOffset` (bool)
    - `currentOffset` (int)
    - `offsetStep` (int)
  - Progressive Lengthening (*N syntax):
    - `hasLengthening` (bool) 
    - `lengtheningStep` (int)
    - `baseLengthPattern` (std::vector<bool>)
    - `currentLengthenedPattern` (std::vector<bool>)
  - Progressive Transformation (>N syntax):
    - `hasTransformation` (bool)
    - `transformationType` (String) - "B", "W", "E", "D"
    - `targetOnsets` (int)
    - `transformationStep` (int)
    - `currentTransformedPattern` (std::vector<bool>)
  - LRU metadata:
    - `accessCount` (int)
    - `lastAccessed` (juce::Time)

### 🔴 HIGH PRIORITY - Scene Management State (Complex Structure)
**SceneManager** (if using new system) OR **Legacy Scene Arrays**:
- `scenePatterns` (StringArray) - List of scene patterns
- `currentSceneIndex` (int) - Current scene position
- Per-scene progressive state (parallel arrays):
  - `sceneProgressiveOffsets` (std::vector<int>) - Current offset for each scene
  - `sceneProgressiveSteps` (std::vector<int>) - Step size for each scene
  - `sceneBasePatterns` (std::vector<String>) - Base pattern for each scene
  - `sceneProgressiveLengthening` (std::vector<int>) - Current lengthening for each scene
  - `sceneBaseLengthPatterns` (std::vector<std::vector<bool>>) - Base patterns for lengthening

### 🔴 HIGH PRIORITY - Accent Pattern State (6 variables)
- `hasAccentPattern` (bool) - Whether accent pattern is active
- `currentAccentPattern` (std::vector<bool>) - Binary accent pattern
- `globalOnsetCounter` (int) - Onset counter for accent positioning
- `uiAccentOffset` (int) - Stable UI accent offset  
- `accentPatternManuallyModified` (bool) - Prevents auto-cycling after manual edits
- Accent parameters:
  - `accentVelocityParam->get()` (float) - Accent velocity multiplier

### 🟡 MEDIUM PRIORITY - Quantization State (6 variables)
- `hasQuantization` (bool) - Whether pattern is quantized
- `originalStepCount` (int) - Original pattern step count
- `quantizedStepCount` (int) - Quantized pattern step count  
- `quantizationClockwise` (bool) - Quantization direction
- `originalOnsetCount` (int) - Original onset count
- `quantizedOnsetCount` (int) - Quantized onset count

### 🟡 MEDIUM PRIORITY - Manual Modification State (4 variables)
- `patternManuallyModified` (bool) - Pattern manually edited (suspension mode)
- `suspendedRhythmPattern` (std::vector<bool>) - Manually modified rhythm pattern
- `suspendedAccentPattern` (std::vector<bool>) - Manually modified accent pattern
- `accentPatternManuallyModified` (bool) - Accent manually modified flag

### 🟢 LOW PRIORITY - Runtime State (Not Critical for Persistence)
- `currentStep` (atomic<int>) - Current step indicator (resets on play)
- `progressiveOffset` (int) - **LEGACY**: Being replaced by ProgressiveManager
- `progressiveStep` (int) - **LEGACY**: Being replaced by ProgressiveManager  
- `progressiveLengthening` (int) - **LEGACY**: Being replaced by ProgressiveManager
- Timing state (recalculated):
  - `samplesPerStep`, `currentSample`, `wasPlaying`
  - `lastHostPosition`, `hostIsPlaying`

---

## State Complexity Analysis

### Most Complex State Structures:
1. **ProgressiveManager::progressiveStates** - Nested map with complex per-pattern state
2. **Scene management** - Multiple parallel arrays with per-scene progressive states
3. **Accent patterns** - Timing-sensitive with UI synchronization requirements

### State Interdependencies:
- `originalUPIInput` → Progressive/Scene pattern parsing
- Progressive states → Pattern generation timing
- Accent patterns → Step timing and UI display
- Quantization → Pattern step counting and display

### Estimated Serialization Complexity:
- **Simple state**: 18 variables (basic types)
- **Complex state**: 3 major structures (nested serialization required)
- **Total state variables**: ~50+ individual pieces of data

---

## Risk Assessment

### 🔴 Critical Failure Points:
1. **Progressive state loss** → User loses transformation progress
2. **Scene state corruption** → Multi-scene performances break  
3. **Original UPI loss** → Tick/MIDI triggers stop working
4. **Accent timing desync** → Rhythmic feel completely lost

### 🟡 Moderate Impact:
1. **Quantization loss** → Visual display issues, pattern still plays
2. **Manual modification loss** → User edits lost but pattern functions

### 🟢 Low Impact:
1. **Runtime state loss** → Playback position resets (normal behavior)
2. **Timing parameter loss** → Reverts to defaults (recoverable)

This inventory provides the foundation for implementing comprehensive state saving in the subsequent phases.