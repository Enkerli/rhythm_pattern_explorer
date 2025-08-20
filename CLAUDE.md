
# Claude Code Memory

## Project Configuration
- **Plugin Installation**: Install to user library (`~/Library/Audio/Plug-Ins/`) NOT system library (`/Library/Audio/Plug-Ins/`)
- **NO STANDALONE APP**: User explicitly does NOT want a standalone application - plugins only (AU/VST3)
  - Remove any `.app` files that get built accidentally
  - Focus on plugin formats only: AU Component and VST3
- **Licensing**: CC0 1.0 Universal (public domain)
- **JUCE Version**: 8.0.8 with WebBrowserComponent support

## Project Development Guidelines
- Before getting "carried away", make sure that was requested fully works as expected.

## CRITICAL: Accent Swirling Prevention Principles

### The Problem
**Accent Swirling**: Visual accent markers in the UI shifting continuously during pattern playback, causing UI/MIDI desynchronization. This was particularly severe with polymetric patterns like {10}E(5,8) where accent patterns (length 2) and rhythm patterns (length 8) have different cycle lengths.

### Root Cause Analysis
The original implementation used **event-based counters** that were incremented during audio processing:
- `globalOnsetCounter++` in processStep() 
- `currentStep.store(value)` throughout the codebase
- UI accent calculation using continuously changing values

This created **counter drift** between systems and **rapid UI updates** causing visual "swirling".

### Architectural Solution: Derived Indices
**Principle**: Replace all mutable counters with **mathematical derivation** from a single monotonic source.

#### Core Components:
1. **Monotonic Transport Tick**: `std::atomic<uint64_t> transportTick` - never decreases during playback
2. **Base Reference Points**: `baseTickRhythm` and `baseTickAccent` for pattern alignment
3. **Derived Calculations**: All positions calculated as `(transportTick - base) % patternSize`

#### Implementation Rules:
- **NEVER** manually increment step/onset counters during audio processing
- **ALWAYS** derive current positions from transportTick mathematically  
- **UI updates** only at cycle boundaries, not every audio buffer
- **MIDI timing** uses real-time derived values for sample-accurate triggering

### UI/MIDI Synchronization Strategy
**Problem**: UI needs stability, MIDI needs real-time accuracy.

**Solution**: Independent but synchronized calculations:
- **MIDI** (processStep): Uses `getCurrentOnsetCount()` for immediate accuracy
- **UI** (getCurrentAccentMap): Uses `uiAccentOffset` updated only at cycle boundaries
- **Both** derive from same `transportTick` ensuring perfect synchronization

### Critical Code Patterns:

#### ✅ CORRECT - Derived calculation:
```cpp
uint32_t getCurrentRhythmStep() const {
    uint64_t tick = transportTick.load();
    uint64_t base = baseTickRhythm.load();
    return static_cast<uint32_t>((tick - base) % pattern.size());
}
```

#### ❌ WRONG - Manual counter increment:
```cpp
// NEVER DO THIS - causes drift and swirling
globalOnsetCounter++;
currentStep.store(newValue);
```

#### ✅ CORRECT - UI accent offset update:
```cpp
// Only update at cycle boundaries (nextStep == 0)
if (nextStep == 0) {
    uiAccentOffset = static_cast<int>(getCurrentOnsetCount() % accentPatternSize);
}
```

### Testing Requirements
Any changes to accent/timing systems MUST be tested with:
- **{10}E(5,8)** - The Quintillo (primary test case)
- **{10100011000}110110111101110** - The Culprit (complex polymetric)
- **Multiple tempo changes** (60-180 BPM range)
- **UI/MIDI synchronization verification** (visual markers match audio output)
- **Extended playback** (10+ pattern cycles without swirling)

### Prevention Guidelines
- **Before modifying timing code**: Understand the derived indices architecture
- **When adding counters**: Ask "Can this be derived from transportTick instead?"
- **For UI updates**: Only update accent displays at appropriate boundaries
- **During debugging**: Use debug logs to verify counter consistency, never assume synchronization

## MIDI Note Duration Debugging Guidelines
**CRITICAL: Always test on both macOS AND iPadOS - platforms can behave differently**

### Required Tools for MIDI Debugging:
1. **MIDI Monitor AUv3** (iPadOS) - Essential for seeing actual MIDI output
2. **Desktop MIDI Monitor** (macOS) - Console apps or DAW MIDI monitors
3. **Internal Debug Counters** - UI-based debugging for AUv3 (console debugging doesn't work)

### Note Duration Issue Diagnostic Sequence:
1. **Check MIDI Monitor Output First** - Shows actual MIDI messages reaching the host
   - Look for note-on (0x90) AND note-off (0x80) messages
   - WARNING FLAG: Note-offs appearing before note-ons indicates timing issue
   - WARNING FLAG: Only note-ons with no note-offs indicates note-offs not reaching output

2. **Check Internal Debug Counters** - Shows if logic is executing
   - Triggers: How many notes have been triggered
   - Active: Current notes in tracking system
   - Offs: How many note-offs have been sent internally
   - WARNING FLAG: High "Offs" count but no note-offs in MIDI monitor = timing issue

3. **Verify MIDI Processing Order** - Critical sequence in processBlock:
   ```cpp
   // CORRECT ORDER:
   // 1. Pattern processing (adds note-ons)
   triggerNote(midiBuffer, samplePosition);
   // 2. Note tracking (sends note-offs)  
   processActiveNotes(midiMessages, bufferSize);
   // 3. Update sample position
   absoluteSamplePosition += bufferSize;
   ```

4. **Cross-Platform Testing** - Desktop can mask timing issues that break iPadOS
   - Test identical patterns on both platforms
   - Compare MIDI monitor output between platforms
   - iPadOS is more sensitive to incorrect MIDI buffer timing

### Common Note Duration Issues:
- **Wrong processBlock order**: Note-offs processed before note-ons → note-offs sent in future buffers
- **Invalid buffer positions**: Note-offs at negative or out-of-bounds positions → messages dropped
- **Zero velocity note-offs**: Some hosts/instruments ignore zero velocity → use 0.5f
- **Platform differences**: Desktop may work with incorrect timing that breaks iPadOS

[Rest of the file remains unchanged...]
- User disapproves of premature celebration and self-praise. Emphasizes thoughtful work.