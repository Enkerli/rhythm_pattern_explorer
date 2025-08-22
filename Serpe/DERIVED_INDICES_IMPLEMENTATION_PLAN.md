# Derived Indices Implementation Plan - Robust Accent System

## Executive Summary
Replace mutable counter-based accent system with derived indices from monotonic time. This eliminates drift, timing window issues, and UI/MIDI desynchronization while preserving polymeter behavior.

## Current State Analysis (Stable Version 640904a)
- ✅ **Working**: Basic accent functionality, no major regressions
- ❌ **Problem**: Mutable counters (`globalOnsetCounter`, `currentStep`) accumulate drift over time
- ❌ **Problem**: Timing window issues between audio thread updates and UI queries  
- ❌ **Problem**: Complex cycle boundary reset logic that can break polymeter

## Root Cause: Mutable State Drift
```cpp
// CURRENT PROBLEMATIC PATTERN
globalOnsetCounter++;                              // Can accumulate errors
currentStep = (currentStep + 1) % patternLength;   // Can get out of sync
uiAccentOffset = complexCalculation();             // Complex derivation from mutable state
```

## Target Architecture: Pure Derived Indices
```cpp
// TARGET ROBUST PATTERN  
std::atomic<uint64_t> transportTick;              // Monotonic, never reset
std::atomic<uint64_t> baseTickRhythm;             // Latched at pattern acceptance
std::atomic<uint64_t> baseTickAccent;             // Latched at pattern acceptance

// Pure functions - no mutable state
inline uint32_t currentRhythmStep() { return (transportTick - baseTickRhythm) % rhythmLength; }
inline uint32_t currentAccentStep() { return (transportTick - baseTickAccent) % accentLength; }
```

---

## PHASE 0: Minimal Surgical Changes - Proof of Concept (3-5 days)
**Time Estimate**: 6-10 hours  
**Confidence**: Very High (95%) - Minimal changes, quick validation
**Goal**: Prove derived indices approach with smallest possible changes

### 0.1 Add Basic Derived Index Functions (2 hours)
**Files**: `PluginProcessor.h`, `PluginProcessor.cpp`

**Minimal Addition**:
```cpp
// In PluginProcessor.h - add these methods only:
private:
    uint64_t getMonotonicTick() const;
    uint32_t getDerivedRhythmStep() const;
    uint32_t getDerivedAccentStep() const;

// In PluginProcessor.cpp - minimal implementation:
uint64_t SerpeAudioProcessor::getMonotonicTick() const {
    // Quick approximation from existing currentStep
    return static_cast<uint64_t>(currentStep.load());
}

uint32_t SerpeAudioProcessor::getDerivedRhythmStep() const {
    auto pattern = patternEngine.getCurrentPattern();
    return static_cast<uint32_t>(getMonotonicTick() % pattern.size());
}

uint32_t SerpeAudioProcessor::getDerivedAccentStep() const {
    if (!hasAccentPattern || currentAccentPattern.empty()) return 0;
    return static_cast<uint32_t>(getMonotonicTick() % currentAccentPattern.size());
}
```

**Expected Results**:
- ✅ Derived functions operational alongside existing system
- ✅ No changes to existing audio processing logic
- ✅ Ready for selective replacement testing

### 0.2 Test Selective Replacement in Non-Critical Paths (2 hours)
**Scope**: Replace existing counter usage in UI queries only

**Changes**:
```cpp
// In PluginProcessor.cpp - UI methods only:
int SerpeAudioProcessor::getCurrentStep() const {
    // OLD: return currentStep.load();
    // NEW: Test derived approach
    return getDerivedRhythmStep();
}

// Test UI accent display with derived indices
std::vector<bool> SerpeAudioProcessor::getCurrentAccentMap() const {
    // Keep existing logic but test derived accent step internally
    uint32_t derivedAccentPos = getDerivedAccentStep();
    // Compare with existing uiAccentOffset logic for validation
}
```

**Expected Results**:
- ✅ UI shows identical step position as before
- ✅ Accent markers display consistently  
- ✅ No audio processing changes yet
- ✅ Validation that derived indices match existing counters

### 0.3 Add Counter Drift Detection (1 hour)
**Implementation**:
```cpp
void SerpeAudioProcessor::validateCounterConsistency() {
    #ifdef DEBUG
    uint32_t legacyStep = currentStep.load() % patternEngine.getCurrentPattern().size();
    uint32_t derivedStep = getDerivedRhythmStep();
    
    if (legacyStep != derivedStep) {
        std::ofstream logFile("/tmp/counter_drift_detected.log", std::ios::app);
        logFile << "DRIFT DETECTED: Legacy=" << legacyStep 
                << ", Derived=" << derivedStep 
                << ", Tick=" << getMonotonicTick() << std::endl;
    }
    #endif
}
```

**Expected Results**:
- ✅ Any counter drift immediately detected and logged
- ✅ Validation of when/where legacy counters diverge from derived indices
- ✅ Evidence for the need for full implementation

### 0.4 Quick Test with Problematic Patterns (1 hour)
**Test Cases**:
- Load `{10}E(5,8)` pattern
- Play for several cycles
- Check UI/MIDI synchronization with derived indices
- Monitor drift detection logs

**Expected Results**:
- ✅ Quick validation if approach fixes core issues
- ✅ Evidence of improvement in UI/MIDI sync
- ✅ Data on where/when legacy system drifts

### 0.5 Performance and Stability Check (2 hours)
**Scope**: Run extended test to verify no performance impact

**Test Protocol**:
- Run plugin for 30+ minutes continuous playback
- Monitor CPU usage with derived index calls
- Test various buffer sizes and BPM changes
- Verify no new instabilities introduced

**Expected Results**:
- ✅ No performance regression
- ✅ Derived indices remain consistent over long playback
- ✅ Ready decision point: proceed with full implementation or optimize approach

## PHASE 0 SUCCESS CRITERIA & DECISION POINT

### ✅ **If Phase 0 Succeeds** (Expected):
- UI/MIDI synchronization improved
- No counter drift detected in logs  
- Performance acceptable
- **Decision**: Proceed rapidly through Phases 1-2 (skip some redundancy)
- **Accelerated Timeline**: Full implementation could drop to 3-4 weeks instead of 5-7

### ❌ **If Phase 0 Shows Issues**:
- Performance problems with derived calculations
- Unexpected behavior differences
- **Decision**: Analyze issues, optimize approach before full implementation
- **Timeline**: Add 1-2 weeks for optimization

### 📊 **Phase 0 Validation Metrics**:
- [ ] Derived indices match legacy counters for first 100 steps
- [ ] UI accent markers identical to legacy system
- [ ] No performance regression (< 5% CPU increase)
- [ ] No new stability issues during 30-min test
- [ ] Clear evidence of improved UI/MIDI sync

---

## PHASE 1: Monotonic Transport Tick (Week 2)
**Time Estimate**: 8-12 hours  
**Confidence**: High (90%) - Well-defined scope, minimal disruption

### 1.1 Add Monotonic Tick System (4 hours)
**Files**: `PluginProcessor.h`, `PluginProcessor.cpp`

**Implementation**:
```cpp
// In PluginProcessor.h - private members:
std::atomic<uint64_t> transportTick{0};           // Monotonic step counter
std::atomic<uint64_t> baseTickRhythm{0};          // Rhythm reference point  
std::atomic<uint64_t> baseTickAccent{0};          // Accent reference point
double samplesPerStep = 0.0;                      // Current step timing
uint64_t sampleAccumulator = 0;                   // For precise step timing

// Helper methods:
void updateTransportTick(int bufferSize);
uint32_t getCurrentRhythmStep() const;
uint32_t getCurrentAccentStep() const;
```

**Expected Results**:
- ✅ `transportTick` increments monotonically during playback
- ✅ No performance impact (atomic reads are fast)
- ✅ Deterministic step calculation from samples
- ✅ Existing functionality unaffected (parallel implementation)

**Validation Criteria**:
- [ ] `transportTick` never decreases during continuous playback
- [ ] Step boundaries occur at correct sample positions (±1 sample tolerance)
- [ ] No audio dropouts or performance regression
- [ ] All existing patterns still play correctly

### 1.2 Implement Sample-Based Step Calculation (3 hours)
**Implementation**:
```cpp
void SerpeAudioProcessor::updateTransportTick(int bufferSize) {
    if (!isCurrentlyPlaying()) return;
    
    sampleAccumulator += bufferSize;
    while (sampleAccumulator >= samplesPerStep) {
        sampleAccumulator -= samplesPerStep;
        transportTick.store(transportTick.load() + 1);
    }
}

void SerpeAudioProcessor::updateTiming() {
    // Called when BPM changes
    double beatsPerMinute = getCurrentBPM();
    double stepsPerBeat = 4.0; // 16th notes
    samplesPerStep = (currentSampleRate * 60.0) / (beatsPerMinute * stepsPerBeat);
}
```

**Expected Results**:
- ✅ Precise step timing based on sample rate and BPM
- ✅ Consistent step intervals regardless of buffer size
- ✅ Handles BPM changes correctly
- ✅ No drift over long playback sessions

### 1.3 Add Derived Index Helper Methods (2 hours)
**Implementation**:
```cpp
uint32_t SerpeAudioProcessor::getCurrentRhythmStep() const {
    uint64_t tick = transportTick.load();
    uint64_t base = baseTickRhythm.load();
    auto pattern = patternEngine.getCurrentPattern();
    return static_cast<uint32_t>((tick - base) % pattern.size());
}

uint32_t SerpeAudioProcessor::getCurrentAccentStep() const {
    if (!hasAccentPattern || currentAccentPattern.empty()) return 0;
    
    uint64_t tick = transportTick.load();
    uint64_t base = baseTickAccent.load();
    return static_cast<uint32_t>((tick - base) % currentAccentPattern.size());
}
```

**Expected Results**:
- ✅ Rhythm and accent indices derived independently
- ✅ Polymeter behavior preserved (different modulo periods)
- ✅ No shared mutable state between rhythm and accent
- ✅ Thread-safe access from both audio and UI threads

### 1.4 Unit Tests for Monotonic System (3 hours)
**File**: `test_monotonic_transport.cpp`

**Test Cases**:
```cpp
void test_monotonic_increment() {
    // Verify transportTick never decreases
    // Test across multiple buffer sizes
    // Verify step boundaries occur at correct sample positions
}

void test_polymeter_independence() {
    // Pattern: 8 steps rhythm, 10 steps accent  
    // Verify rhythm cycles every 8 ticks
    // Verify accent cycles every 10 ticks
    // Verify LCM behavior (40 ticks total cycle)
}

void test_bpm_changes() {
    // Change BPM during playback
    // Verify step timing adjusts correctly
    // Verify no tick count discontinuities
}
```

**Expected Results**:
- ✅ All monotonic properties verified by tests
- ✅ Polymeter math proven correct
- ✅ Edge cases handled (BPM changes, buffer size variations)

**PHASE 1 SUCCESS CRITERIA**:
- [ ] `transportTick` system operational and tested
- [ ] Derived indices working in parallel with legacy system
- [ ] No regression in existing functionality
- [ ] Clear separation between monotonic time and pattern logic

---

## PHASE 2: Pattern Acceptance Phase-Locking (Week 2)  
**Time Estimate**: 12-16 hours
**Confidence**: Medium-High (80%) - Audio thread synchronization complexity

### 2.1 Lock-Free Pattern Update Queue (6 hours)
**Files**: `PluginProcessor.h`, `PluginProcessor.cpp`

**Implementation**:
```cpp
// Lock-free SPSC queue for pattern updates
struct PatternUpdate {
    std::vector<bool> rhythmPattern;
    std::vector<bool> accentPattern;
    bool hasAccent;
    int accentPhaseOffset = 0;  // User-controlled offset
};

// In PluginProcessor.h:
#include <atomic>
#include <array>

class PatternUpdateQueue {
    static constexpr size_t QUEUE_SIZE = 16;
    std::array<PatternUpdate, QUEUE_SIZE> queue;
    std::atomic<size_t> writeIndex{0};
    std::atomic<size_t> readIndex{0};
    
public:
    bool enqueue(const PatternUpdate& update);
    bool dequeue(PatternUpdate& update);
    bool isEmpty() const;
};

PatternUpdateQueue patternUpdateQueue;
```

**Expected Results**:
- ✅ Thread-safe pattern updates without locks
- ✅ UI thread can enqueue changes without blocking audio
- ✅ Audio thread processes changes at buffer boundaries only
- ✅ No dropped pattern updates under normal use

### 2.2 Audio Thread Pattern Acceptance (4 hours)
**Implementation**:
```cpp
void SerpeAudioProcessor::processPatternUpdates() {
    PatternUpdate update;
    while (patternUpdateQueue.dequeue(update)) {
        // Phase-lock at current transportTick
        uint64_t currentTick = transportTick.load();
        baseTickRhythm.store(currentTick);
        baseTickAccent.store(currentTick + update.accentPhaseOffset);
        
        // Update pattern atomically
        patternEngine.setPattern(update.rhythmPattern);
        hasAccentPattern = update.hasAccent;
        currentAccentPattern = update.accentPattern;
        
        patternChanged.store(true); // Notify UI
    }
}
```

**Expected Results**:
- ✅ Pattern changes occur at buffer boundaries only
- ✅ Phase-locking preserves user timing intent  
- ✅ No timing window races between audio and UI threads
- ✅ Accent phase offset allows user control over sync

### 2.3 Replace Direct Pattern Setting (3 hours)
**Scope**: Update all UI-initiated pattern changes to use queue

**Files to Update**:
- `parseAndApplyUPI()` - Route through queue instead of direct setting
- `togglePatternStep()` - Queue manual pattern edits
- Scene changes, progressive transformations - All via queue

**Implementation Pattern**:
```cpp
// OLD: Direct setting (thread-unsafe)
patternEngine.setPattern(newPattern);
hasAccentPattern = true;
currentAccentPattern = newAccent;

// NEW: Queue-based setting (thread-safe)
PatternUpdate update{newPattern, newAccent, true, 0};
patternUpdateQueue.enqueue(update);
```

**Expected Results**:
- ✅ All pattern changes go through single audio-thread path
- ✅ No more mixed UI/audio thread pattern mutations
- ✅ Consistent phase-locking for all pattern sources (UPI, manual, scenes)

### 2.4 Unit Tests for Phase-Locking (3 hours)
**Test Cases**:
```cpp
void test_phase_lock_timing() {
    // Set pattern at tick 100
    // Verify baseTickRhythm = 100, baseTickAccent = 100
    // Verify indices start from 0 at tick 100
}

void test_accent_phase_offset() {
    // Set pattern with phase offset +3
    // Verify accent leads rhythm by 3 steps
    // Test polymeter with offset
}

void test_queue_thread_safety() {
    // Concurrent enqueue/dequeue from different threads
    // Verify no lost updates, no corruption
}
```

**PHASE 2 SUCCESS CRITERIA**:
- [ ] All pattern updates phase-locked at acceptance
- [ ] No timing window races
- [ ] Queue handles concurrent access safely  
- [ ] User phase control working

---

## PHASE 3: Replace Mutable Counters with Derived Indices (Week 3)
**Time Estimate**: 16-20 hours
**Confidence**: Medium (70%) - Touches core playback logic

### 3.1 Replace currentStep Usage (8 hours)
**Scope**: Replace all `currentStep` references with `getCurrentRhythmStep()`

**Files**: `PluginProcessor.cpp`, `PluginEditor.cpp`

**Changes**:
```cpp
// OLD: Mutable counter
currentStep.store((currentStep.load() + 1) % patternLength);
int step = currentStep.load();

// NEW: Derived index  
int step = getCurrentRhythmStep();
```

**Expected Results**:
- ✅ Current step always derived from transportTick
- ✅ No step counter drift over time
- ✅ Pattern position always mathematically correct
- ✅ UI displays correct current step

### 3.2 Replace globalOnsetCounter Logic (6 hours)
**Scope**: Replace onset counting with derived calculation

**Implementation**:
```cpp
uint32_t SerpeAudioProcessor::getCurrentOnsetCount() const {
    uint64_t tick = transportTick.load();
    uint64_t base = baseTickRhythm.load();
    uint32_t ticksSinceBase = static_cast<uint32_t>(tick - base);
    
    // Count onsets in the pattern from 0 to current position
    auto pattern = patternEngine.getCurrentPattern();
    uint32_t onsetCount = 0;
    
    for (uint32_t t = 0; t < ticksSinceBase; t++) {
        uint32_t stepIndex = t % pattern.size();
        if (pattern[stepIndex]) {
            onsetCount++;
        }
    }
    
    return onsetCount;
}
```

**Expected Results**:
- ✅ Onset count always mathematically correct
- ✅ No accumulation errors over time
- ✅ Deterministic across identical playback sessions
- ✅ Performance acceptable for UI queries (O(n) where n = ticks since pattern start)

### 3.3 Implement Step-Indexed vs Onset-Indexed Modes (4 hours)
**Implementation**:
```cpp
enum class AccentMode {
    StepIndexed,    // Accent[i] applies to rhythm step i
    OnsetIndexed    // Accent[i] applies to i-th onset
};

bool SerpeAudioProcessor::isCurrentStepAccented() const {
    if (!hasAccentPattern || currentAccentPattern.empty()) return false;
    
    uint32_t rhythmStep = getCurrentRhythmStep();
    auto pattern = patternEngine.getCurrentPattern();
    
    if (!pattern[rhythmStep]) return false; // No onset at this step
    
    if (accentMode == AccentMode::StepIndexed) {
        uint32_t accentStep = getCurrentAccentStep();
        return currentAccentPattern[accentStep];
    } else {
        uint32_t onsetIndex = getOnsetIndexAtStep(rhythmStep);
        uint32_t accentIndex = onsetIndex % currentAccentPattern.size();
        return currentAccentPattern[accentIndex];
    }
}
```

**Expected Results**:
- ✅ Clear semantics for accent application
- ✅ User can choose appropriate mode for their workflow
- ✅ No ambiguity about accent timing across pattern changes
- ✅ Both modes work correctly with polymeter

### 3.4 Update UI Synchronization (2 hours)
**Scope**: UI queries derived indices instead of cached values

**Changes**:
```cpp
// OLD: UI uses cached offset
int uiStep = (currentStep + uiAccentOffset) % patternLength;

// NEW: UI derives from same monotonic source
int uiStep = getCurrentRhythmStep();
bool uiAccented = isCurrentStepAccented();
```

**Expected Results**:
- ✅ UI and MIDI always show identical state
- ✅ No UI/MIDI desynchronization
- ✅ No swirling effects during playback
- ✅ Real-time UI updates during pattern changes

**PHASE 3 SUCCESS CRITERIA**:
- [ ] All mutable counters replaced with derived indices
- [ ] UI and MIDI perfectly synchronized
- [ ] No timing drift over extended playback
- [ ] Accent modes working correctly

---

## PHASE 4: Canonical Pattern Normalization (Week 4)
**Time Estimate**: 10-14 hours
**Confidence**: High (85%) - Data transformation, well-defined

### 4.1 Canonical Pattern Representation (6 hours)
**Implementation**:
```cpp
struct CanonicalPattern {
    std::vector<bool> rhythmMask;
    std::vector<bool> accentMask;
    AccentMode accentMode;
    uint32_t rotationOffset;        // Applied during canonicalization
    std::string originalNotation;   // For debugging/display
    
    // Precomputed for performance
    std::vector<uint32_t> onsetPositions;  // Indices where rhythmMask[i] == true
    uint32_t lcmPeriod;                     // LCM(rhythm.size(), accent.size())
};

CanonicalPattern canonicalizePattern(const std::string& upiInput, const UPIParseResult& parsed);
```

**Expected Results**:
- ✅ Equivalent patterns produce identical canonical form
- ✅ `M:Alex` and `{[0,2,6,7]:11}110...` normalize to same representation
- ✅ Pattern switching doesn't change internal timing
- ✅ Precomputed data improves performance

### 4.2 Pattern Equivalence Testing (3 hours)
**Implementation**:
```cpp
bool patternsAreEquivalent(const std::string& pattern1, const std::string& pattern2) {
    auto canonical1 = canonicalizePattern(pattern1, parseUPI(pattern1));
    auto canonical2 = canonicalizePattern(pattern2, parseUPI(pattern2));
    
    return canonical1.rhythmMask == canonical2.rhythmMask &&
           canonical1.accentMask == canonical2.accentMask &&
           canonical1.accentMode == canonical2.accentMode;
}
```

**Expected Results**:
- ✅ Equivalent patterns detected reliably
- ✅ No unnecessary pattern updates for equivalent changes
- ✅ Canonical forms remain stable across equivalent inputs

### 4.3 LCM Precomputation and Cycle Testing (3 hours)
**Implementation**:
```cpp
void CanonicalPattern::computeLCMCycle() {
    uint32_t rhythmPeriod = rhythmMask.size();
    uint32_t accentPeriod = accentMask.size();
    lcmPeriod = lcm(rhythmPeriod, accentPeriod);
    
    // Precompute full LCM cycle for testing (if reasonable size)
    if (lcmPeriod <= 1000) {
        lcmCyclePreview.reserve(lcmPeriod);
        for (uint32_t tick = 0; tick < lcmPeriod; tick++) {
            uint32_t rStep = tick % rhythmPeriod;
            uint32_t aStep = tick % accentPeriod;
            lcmCyclePreview.push_back({rhythmMask[rStep], accentMask[aStep]});
        }
    }
}
```

**Expected Results**:
- ✅ LCM cycles computed correctly
- ✅ Polymeter behavior predictable and testable
- ✅ Long-term pattern repetition verified

### 4.4 Property Tests for Pattern Consistency (2 hours)
**Test Cases**:
```cpp
void test_pattern_equivalence() {
    // Test equivalent patterns produce same canonical form
    assert(patternsAreEquivalent("M:Alex", "{[0,2,6,7]:11}110111101110"));
    assert(patternsAreEquivalent("{10}E(5,8)", "specific_equivalent_form"));
}

void test_lcm_cycle_repetition() {
    // For {10}E(5,8): LCM(5,10) = 10 onsets
    // Verify cycle 1 matches cycle 3 (after 10 onsets)
    // Verify cycle 2 matches cycle 4 (polymeter offset)
}

void test_long_playback_consistency() {
    // Simulate 1000+ transport ticks
    // Verify no drift in derived indices
    // Verify perfect cycle repetition
}
```

**PHASE 4 SUCCESS CRITERIA**:
- [ ] Equivalent patterns handled identically
- [ ] LCM cycles computed and verified correctly
- [ ] Property tests pass for all pattern types
- [ ] No pattern-switching artifacts

---

## PHASE 5: Remove Legacy Counter System (Week 5)
**Time Estimate**: 6-10 hours
**Confidence**: High (90%) - Cleanup work

### 5.1 Remove Deprecated Variables (2 hours)
**Scope**: Clean removal of old mutable counters
```cpp
// REMOVE from PluginProcessor.h:
// int globalOnsetCounter = 0;
// std::atomic<int> currentStep{0}; 
// int uiAccentOffset = 0;
// All related reset logic and cycle boundary handling
```

### 5.2 Remove Cycle Boundary Reset Logic (3 hours)
**Scope**: Remove complex cycle detection and reset code
- Remove `nextStep == 0` reset blocks
- Remove `patternCycles` calculations  
- Remove `uiAccentOffset` manipulation
- Remove associated debug logging

### 5.3 Simplify State Management (2 hours)
**Scope**: Clean up now-unnecessary complexity
- Remove pattern change flags and timing windows
- Simplify `resetAccentSystem()` to just reset base ticks
- Remove temporary variables used for counter synchronization

### 5.4 Final Integration Testing (3 hours)
**Scope**: Comprehensive testing of clean system
- Test all pattern types (Euclidean, Morse, manual, etc.)
- Test polymeter patterns extensively
- Test UI/MIDI synchronization
- Test long-duration playback (1+ hours)
- Performance testing (CPU usage, memory)

**PHASE 5 SUCCESS CRITERIA**:
- [ ] All legacy counter code removed
- [ ] No regressions in functionality
- [ ] Code significantly simplified and cleaner
- [ ] Performance equal or better than legacy system

---

## Final Success Criteria - Robust System

### Functional Requirements Met
- [ ] **No UI/MIDI Desynchronization**: Visual accent markers match MIDI output exactly at all times
- [ ] **No Accent Drift**: Patterns repeat identically across multiple cycles
- [ ] **Polymeter Preservation**: Different-length rhythm/accent patterns maintain correct mathematical relationships
- [ ] **Pattern Equivalence**: `M:Alex` ↔ `{[0,2,6,7]:11}110...` transitions are seamless
- [ ] **Long-Duration Stability**: No drift or issues during hours of continuous playback
- [ ] **Performance**: No audio dropouts, UI remains responsive

### Non-Functional Requirements Met  
- [ ] **Code Clarity**: Derived indices make timing logic clear and predictable
- [ ] **Testability**: Property tests verify correctness across all pattern types
- [ ] **Maintainability**: No complex timing window logic or mutable counter coordination
- [ ] **Determinism**: Identical inputs produce identical outputs across sessions

### Edge Cases Handled
- [ ] **BPM Changes**: Step timing adjusts correctly without disrupting pattern flow
- [ ] **Pattern Changes**: Phase-locked acceptance preserves user timing intent
- [ ] **Transport Control**: Play/stop/reset behave predictably
- [ ] **Buffer Size Variations**: Timing remains consistent across different audio buffer sizes
- [ ] **Long Patterns**: Performance remains acceptable for complex polymeter patterns

---

## Risk Assessment & Mitigation

### High-Risk Areas
1. **Audio Thread Timing**: Sample-accurate step calculation
   - *Mitigation*: Extensive unit testing of timing calculations
   - *Fallback*: Preserve current BPM/timing system if issues arise

2. **Thread Synchronization**: Lock-free queue implementation  
   - *Mitigation*: Use proven SPSC queue algorithms, thorough concurrency testing
   - *Fallback*: Simple mutex if lock-free proves problematic

3. **Performance**: Derived index calculation overhead
   - *Mitigation*: Profile each phase, optimize hot paths
   - *Fallback*: Cache derived values if computation too expensive

### Medium-Risk Areas
1. **Pattern Canonicalization**: Complex equivalence detection
   - *Mitigation*: Start with simple cases, expand gradually
   - *Testing*: Extensive property tests for equivalence

2. **UI Synchronization**: Real-time derived index queries
   - *Mitigation*: Minimize UI query frequency, batch updates
   - *Testing*: UI stress testing during pattern changes

### Low-Risk Areas
1. **Legacy Code Removal**: Well-defined cleanup
2. **Unit Testing**: Isolated testing of individual components
3. **Documentation**: Clear specification of new architecture

---

## Timeline Summary

| Phase | Duration | Risk | Key Deliverable |
|-------|----------|------|-----------------|
| 0 | 3-5 days (6-10h) | Very Low | Proof of concept - minimal surgical changes |
| 1 | Week 2 (8-12h) | Low | Monotonic transport tick system |
| 2 | Week 3 (12-16h) | Medium | Phase-locked pattern acceptance |
| 3 | Week 4 (16-20h) | High | Derived indices replace counters |
| 4 | Week 5 (10-14h) | Medium | Canonical pattern normalization |
| 5 | Week 6 (6-10h) | Low | Legacy system removal |

**Base Timeline**: 58-82 hours (15-21 working days), 6-7 weeks

### **Accelerated Timeline if Phase 0 Succeeds**:
- **Phase 0 Success** could reduce Phases 1-2 by 30-40% (proven approach)
- **Total Accelerated Time**: 45-65 hours (11-16 working days)  
- **Total Calendar Time**: 4-5 weeks with methodical approach

### **Phase 0 Decision Tree**:
- ✅ **Success**: Skip redundant validation in Phases 1-2, accelerate timeline
- ⚠️ **Partial Success**: Continue as planned with optimizations
- ❌ **Issues Found**: Add 1-2 weeks for approach refinement

---

## Next Steps

1. **Review and Approve Plan**: Ensure all requirements and edge cases are covered
2. **Create Feature Branch**: `derived-indices-implementation`
3. **Set Up Testing Infrastructure**: Unit tests, property tests, integration tests
4. **Begin Phase 1**: Implement monotonic transport tick system
5. **Regular Check-ins**: Validate each phase before proceeding to next

---

*This document serves as the definitive implementation plan for the derived indices architecture. All changes should be tracked against these phases and success criteria.*