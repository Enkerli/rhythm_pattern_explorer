# Accent Morphing Failed Experiments Documentation

## Problem Statement
**Issue**: Accent positions morph/shift during live pattern changes in Serpe JUCE audio plugin
**Manifestation**: When switching between equivalent patterns (e.g., M:Alex ↔ M:Alexa, or different notations like `{[0,2,6,7]:11}110110111101110` vs `{[0,2,6,7]:11}M:Alex`), accent positions drift across cycles instead of maintaining synchronization
**User Requirement**: "Any time Enter is pressed, or the Tick mechanism is used, whether or not there's a new pattern, there should be a reset to step 0 for the rhythm pattern with an associated reset of the accent structure"
**Critical Constraint**: Must preserve polymeter behavior (no cycle boundary resets)

## Root Cause Hypotheses Tested

### Hypothesis 1: Pattern Change Detection Issues
**Theory**: Reset mechanism not triggering consistently on all pattern entry methods
**Test**: Modified `setUPIInput()` to trigger universal reset regardless of string comparison
**Result**: FAILED - Reset triggered but morphing persisted
**Evidence**: Debug logs showed reset occurring but subsequent accents still displaced

### Hypothesis 2: Timing Race Condition
**Theory**: `globalOnsetCounter` increments before reset takes effect due to audio thread timing
**Test**: Added `pendingPatternChange` flag to block counter increments during reset
**Result**: FAILED - Displacement continued despite blocked increments
**Evidence**: Debug logs showed increment blocking working but accents still wrong positions

### Hypothesis 3: Reset Counter Mechanism
**Theory**: Need separate counter for first accent calculation after pattern change
**Test**: Implemented `patternChangeResetCounter` to force position 0 usage
**Result**: FAILED - First accent correct (position 0) but subsequent accents displaced
**Evidence**: MIDI logs show F1 (accented) at first note as expected, but pattern shifts in later cycles

### Hypothesis 4: Flag Clearance Timing
**Theory**: `pendingPatternChange` and `patternChangeResetCounter` cleared at different times creating timing window
**Test**: Modified logic to clear both flags simultaneously after reset counter usage
**Result**: FAILED - Latest MIDI log still shows morphing patterns across cycles
**Evidence**: Current MIDI output shows inconsistent accent patterns between cycles

## Technical Approaches Attempted

### Attempt 1: Universal Reset Mechanism
```cpp
// In setUPIInput() - always reset regardless of pattern equivalence
globalOnsetCounter = 0;
uiAccentOffset = 0;
currentStep.store(0);
absoluteSamplePosition = 0;
activeNotes.clear();
```
**Outcome**: Triggered properly but didn't prevent morphing

### Attempt 2: Nuclear Reset Approach
```cpp
// Complete audio state clearing
globalOnsetCounter = 0;
uiAccentOffset = 0;
currentStep.store(0);
absoluteSamplePosition = 0;
activeNotes.clear();
patternEngineTransformationCount = 0;
// + reset all pattern state
```
**Outcome**: Eliminated some state but morphing persisted

### Attempt 3: Reset Counter with Delayed Flag Clearance
```cpp
// Use reset counter for first accent calculation
if (patternChangeResetCounter >= 0) {
    accentPosition = patternChangeResetCounter;
    patternChangeResetCounter = -1;
    // Keep pendingPatternChange active longer
} else {
    accentPosition = globalOnsetCounter;
}

// Later: clear pendingPatternChange after first audio cycle
if (!pendingPatternChange) {
    globalOnsetCounter++;
} else {
    pendingPatternChange = false; // Clear after blocking one increment
}
```
**Outcome**: Prevented early increments but displacement continued

### Attempt 4: Simultaneous Flag Clearance
```cpp
// Clear both flags together after reset counter used
if (patternChangeResetCounter >= 0) {
    accentPosition = patternChangeResetCounter;
    patternChangeResetCounter = -1;    // Disable reset counter
    pendingPatternChange = false;      // Allow increment again
}
```
**Outcome**: Clean flag management but morphing still occurs

## Evidence of Failure

### Latest MIDI Log Analysis (Post Final Fix)
```
1 1 1 4    Note 1 F1 127    <- First note correctly accented (position 0)
1 1 2 17   Note 1 C1 102    <- Pattern continues...
...
3 3 2 147  Note 1 F1 127    <- Accent pattern shifts
3 3 4 177  Note 1 F1 127    <- Two consecutive accented notes (shouldn't happen)
...
4 1 1 3    Note 1 C1 102    <- Pattern completely different from cycle 1
4 2 1 65   Note 1 F1 127    <- Accents at different positions
```

**Key Observation**: Accent patterns between cycles 1, 3, 4, and 5 show completely different structures, proving morphing is occurring despite all attempted fixes.

## Files Modified During Experiments

### Primary Changes
- `/Source/Platform/PluginProcessor.h`: Added `pendingPatternChange`, `forceAccentAtZero`, `patternChangeResetCounter` flags
- `/Source/Platform/PluginProcessor.cpp`: 
  - Lines 1320-1350: Universal reset in `setUPIInput()`
  - Lines 1025-1048: Reset counter accent calculation logic
  - Lines 1071-1087: Counter increment blocking and flag management
- `/Source/Core/UPIParser.cpp`: Fixed integer precision warnings with `static_cast<int>()`

### Debug Infrastructure
- Added comprehensive logging to `/tmp/serpe_accent_debug.log`
- Created test files: `/tmp/test_timing_fix.cpp`, `/tmp/test_reset_counter_timing.cpp`, `/tmp/test_polymeter_preservation.cpp`

## Preserved Features During Experiments
✅ **Polymeter Behavior**: Successfully maintained different-length accent/rhythm patterns creating polymetric effects
✅ **Pattern Equivalence**: Morse code conversions and binary notations working correctly
✅ **Normal Playback**: Single pattern playback without changes works perfectly
✅ **Universal Reset Triggering**: All pattern entry methods (Enter, Tick, MIDI, presets) trigger resets

## What Doesn't Work
❌ **Cross-Cycle Consistency**: Accent patterns shift between cycles of the same pattern
❌ **Pattern Change Synchronization**: Switching between equivalent patterns causes accent drift
❌ **Live Performance Stability**: Cannot reliably use pattern changes during live performance

## Technical Insights Gained

1. **Reset Mechanisms Work**: The reset logic correctly triggers and sets initial state to position 0
2. **First Accent Correct**: Immediately after pattern change, first accent calculation uses position 0 correctly
3. **Morphing Occurs Later**: The displacement/morphing happens in subsequent cycles, not immediately
4. **Audio Thread Timing**: All attempted timing fixes (blocking increments, flag management) did not resolve the core issue
5. **State Isolation**: The issue appears to be deeper than the variables we've been manipulating

## Failed Hypotheses Summary

1. ❌ Pattern change detection inconsistency
2. ❌ Audio thread timing race conditions  
3. ❌ Counter increment timing issues
4. ❌ Flag clearance synchronization problems
5. ❌ State variable contamination

## Next Steps Recommended

1. **Commit Current Failed State**: Preserve all experimental code and debug infrastructure
2. **Revert to 640904a**: Clean slate for new approach
3. **Seek External Consultation**: The issue may require JUCE audio processing expertise
4. **Consider Alternative Architectures**: 
   - Complete separation of accent and rhythm counters
   - Event-based accent triggering instead of counter-based
   - Accent pattern pre-calculation and lookup tables
5. **Deeper Audio Processing Analysis**: May need to examine JUCE audio buffer processing, sample-accurate timing, or host DAW interaction

## Commit Message for Failed Experiment
```
🔬 EXPERIMENT: Failed accent morphing fixes - comprehensive documentation

Multiple approaches attempted to fix accent position morphing during pattern changes:
- Universal reset mechanisms
- Nuclear state clearing  
- Reset counter with timing controls
- Simultaneous flag clearance
- Audio thread synchronization

All approaches failed to prevent cross-cycle accent pattern drift.
Polymeter behavior preserved throughout experiments.
Ready for revert to 640904a and fresh architectural approach.

See ACCENT_MORPHING_FAILED_EXPERIMENTS.md for complete analysis.
```

---

**Status**: All attempted fixes have failed to resolve accent morphing issue. Ready for branch commit and revert to stable state.