# Derived Indexing Architecture Plan

## Overview
Replace the mutable counter-based system with a **derived indexing** approach using:
- **Monotonic transport tick** (never reset)
- **Phase-locked pattern acceptance** (bases latched at acceptance time)
- **Immutable rhythm/accent masks** (pure function lookups)
- **Audio thread-owned state** (no mixed ownership)

This eliminates all timing race conditions and morphing issues by making every decision a pure function of `(tick, base, masks)`.

## Core Principle
```cpp
// Instead of: globalOnsetCounter++ (mutable, can drift)
// Use: pure derived indices from monotonic clock
inline uint32_t rhythmIndex(uint64_t tick) { 
    return (uint32_t)((tick - baseTickRhythm.load()) % rhythmPeriod); 
}
inline uint32_t accentIndex(uint64_t tick) { 
    return (uint32_t)((tick - baseTickAccent.load()) % accentPeriod); 
}
```

## Implementation Phases

### Phase 1: Core Infrastructure
**File**: `PluginProcessor.h`
```cpp
// Replace existing counter variables with:
std::atomic<uint64_t> transportTick{0};      // Monotonic, audio-thread only
std::atomic<uint64_t> baseTickRhythm{0};     // Phase-locked at pattern acceptance
std::atomic<uint64_t> baseTickAccent{0};     // Phase-locked at accent acceptance

// Immutable pattern data (audio thread owned)
struct PatternMasks {
    std::vector<bool> rhythmMask;    // [0..nR-1] step-indexed rhythm
    std::vector<bool> accentMask;    // [0..nA-1] accent pattern
    std::vector<uint32_t> onsetSteps; // precomputed onset positions
    std::vector<uint32_t> onsetIndexForStep; // inverse mapping
    uint32_t rhythmPeriod;
    uint32_t accentPeriod;
    bool useOnsetIndexedAccents;     // mode toggle
};

std::atomic<PatternMasks*> currentMasks{nullptr};  // Atomic pointer swap
```

### Phase 2: Pattern Acceptance System
**File**: `PluginProcessor.h`
```cpp
// Lock-free communication UI → Audio
struct PendingPatternChange {
    std::unique_ptr<PatternMasks> newMasks;
    uint64_t applyAtTick;            // Set by audio thread
    bool resetPhase;                 // User pressed Enter/Tick
};

// SPSC queue for thread-safe communication
juce::AbstractFifo patternChangeQueue{16};
std::array<PendingPatternChange, 16> patternChangeBuffer;
```

### Phase 3: Derived Index Functions
**File**: `PluginProcessor.cpp`
```cpp
inline uint32_t getCurrentRhythmIndex() const {
    uint64_t tick = transportTick.load();
    uint64_t base = baseTickRhythm.load();
    auto* masks = currentMasks.load();
    return masks ? (uint32_t)((tick - base) % masks->rhythmPeriod) : 0;
}

inline uint32_t getCurrentAccentIndex() const {
    uint64_t tick = transportTick.load();
    uint64_t base = baseTickAccent.load();
    auto* masks = currentMasks.load();
    return masks ? (uint32_t)((tick - base) % masks->accentPeriod) : 0;
}

inline bool shouldCurrentOnsetBeAccented() const {
    auto* masks = currentMasks.load();
    if (!masks) return false;
    
    uint32_t rhythmIdx = getCurrentRhythmIndex();
    if (!masks->rhythmMask[rhythmIdx]) return false; // No onset at this step
    
    if (masks->useOnsetIndexedAccents) {
        // Accent follows onset order
        uint32_t onsetIdx = masks->onsetIndexForStep[rhythmIdx];
        uint32_t accentIdx = getCurrentAccentIndex();
        return masks->accentMask[(onsetIdx + accentIdx) % masks->accentPeriod];
    } else {
        // Accent follows absolute step position
        uint32_t accentIdx = getCurrentAccentIndex();
        return masks->accentMask[accentIdx];
    }
}
```

### Phase 4: Monotonic Tick Management
**File**: `PluginProcessor.cpp` - in `processBlock()`
```cpp
void updateTransportTick(int numSamples) {
    // Deterministic tick advancement based on sample position
    static double samplesPerStep = (sampleRate * 60.0 / bpm) / stepsPerBeat;
    static double sampleAccumulator = 0.0;
    
    sampleAccumulator += numSamples;
    while (sampleAccumulator >= samplesPerStep) {
        transportTick.fetch_add(1);
        sampleAccumulator -= samplesPerStep;
        
        // Process any pending pattern changes at tick boundaries
        processPatternChangeQueue();
    }
}

void processPatternChangeQueue() {
    while (patternChangeQueue.getNumReady() > 0) {
        int readIndex = patternChangeQueue.getReadPosition();
        auto& change = patternChangeBuffer[readIndex];
        
        // Apply new pattern masks
        auto* oldMasks = currentMasks.exchange(change.newMasks.release());
        delete oldMasks;
        
        // Phase-lock at current tick (not reset!)
        uint64_t currentTick = transportTick.load();
        baseTickRhythm.store(currentTick);
        baseTickAccent.store(currentTick); // + potential offset
        
        patternChangeQueue.finishedRead(1);
    }
}
```

### Phase 5: Pattern Canonicalization
**File**: `UPIParser.cpp`
```cpp
PatternMasks* createCanonicalMasks(const std::string& upiInput) {
    auto masks = std::make_unique<PatternMasks>();
    
    // Parse and normalize pattern to canonical form
    auto parsedPattern = parseUPI(upiInput);
    normalizeToCanonical(parsedPattern); // Always start at first onset
    
    // Build immutable rhythm mask
    masks->rhythmPeriod = parsedPattern.rhythm.size();
    masks->rhythmMask.resize(masks->rhythmPeriod);
    for (size_t i = 0; i < masks->rhythmPeriod; ++i) {
        masks->rhythmMask[i] = parsedPattern.rhythm[i];
    }
    
    // Precompute onset positions and inverse mapping
    for (uint32_t step = 0; step < masks->rhythmPeriod; ++step) {
        if (masks->rhythmMask[step]) {
            masks->onsetIndexForStep[step] = masks->onsetSteps.size();
            masks->onsetSteps.push_back(step);
        }
    }
    
    // Build accent mask (step-indexed or onset-indexed)
    buildAccentMask(*masks, parsedPattern.accents);
    
    return masks.release();
}
```

### Phase 6: UI Thread Integration
**File**: `PluginProcessor.cpp`
```cpp
void setUPIInput(const std::string& input) override {
    // Build new masks on UI thread (safe - immutable data)
    auto newMasks = std::unique_ptr<PatternMasks>(createCanonicalMasks(input));
    
    // Enqueue for audio thread application
    if (patternChangeQueue.getFreeSpace() > 0) {
        int writeIndex = patternChangeQueue.getWritePosition();
        patternChangeBuffer[writeIndex] = {
            std::move(newMasks),
            0, // applyAtTick will be set by audio thread
            true // resetPhase = true (user action)
        };
        patternChangeQueue.finishedWrite(1);
    }
}

void triggerReset() {
    // "Reset to step 0" becomes phase re-latch, not counter clear
    if (patternChangeQueue.getFreeSpace() > 0) {
        int writeIndex = patternChangeQueue.getWritePosition();
        patternChangeBuffer[writeIndex] = {
            nullptr, // Keep current masks
            0,
            true // resetPhase = true
        };
        patternChangeQueue.finishedWrite(1);
    }
}
```

## Key Benefits

### 1. **Eliminates All Morphing Sources**
- No mutable counters to drift or get out of sync
- No timing race conditions between flags
- No state variables with mixed thread ownership

### 2. **Preserves Polymeter Behavior**
- Phase bases are latched, not reset to zero
- Different period lengths create natural polymetric patterns
- No cycle boundary interference

### 3. **Thread Safety**
- Audio thread owns all playback state
- UI thread only enqueues immutable data
- Atomic pointer swaps for pattern changes

### 4. **Deterministic & Testable**
- Pure functions: `f(tick, base, masks) → (rhythm, accent)`
- Identical inputs always produce identical outputs
- Easy to unit test cross-cycle consistency

### 5. **Performance Optimized**
- No locks in audio thread hot path
- Precomputed lookup tables
- Cache-friendly immutable data structures

## Migration Strategy

### Minimal Changes First (Phase 1-2)
1. Add transportTick and derived index functions
2. Replace `globalOnsetCounter` usage with `getCurrentRhythmIndex()`
3. Replace accent calculation with `shouldCurrentOnsetBeAccented()`
4. Keep existing UI temporarily

### Full Architecture (Phase 3-6)
1. Implement pattern canonicalization
2. Add SPSC queue for pattern changes
3. Move all state ownership to audio thread
4. Add property tests for consistency

## Success Criteria

✅ **Cross-cycle consistency**: Cycles 1, 2, 3... produce identical MIDI patterns
✅ **Pattern equivalence**: M:Alex and {[0,2,6,7]:11}M:Alex produce identical output
✅ **Phase-locked resets**: Enter/Tick synchronizes without breaking polymeter
✅ **Thread safety**: No race conditions or mixed ownership
✅ **Performance**: No audio dropouts or timing jitter

This architecture addresses every issue identified in the failed experiments by eliminating the fundamental source of non-determinism: mutable, incrementally-updated counters.