# MIDI Note Duration Debugging Guide

## Overview
This document provides a comprehensive guide for debugging MIDI note duration issues in the Serpe rhythm pattern explorer plugin. These guidelines were developed after solving a complex cross-platform timing issue where notes had proper durations on macOS but failed to terminate correctly on iPadOS.

## Critical Insights

### Platform Differences Matter
**ALWAYS test on both macOS AND iPadOS** - Desktop can mask timing issues that break iPadOS:
- macOS (AU/VST3) may be more forgiving of incorrect MIDI buffer timing
- iPadOS (AUv3) is more sensitive to proper MIDI message ordering
- A fix that works on desktop may still be broken on iPadOS

### The Hidden Timing Issue
Internal logic can be perfect, but if the **processBlock order is wrong**, MIDI output will be broken:
```cpp
// WRONG ORDER (what was happening):
processActiveNotes(midiBuffer);  // ← Note-offs processed first
// ... pattern processing ...
triggerNote(midiBuffer);         // ← Note-ons added later
// Result: Note-offs sent in future buffers, after recording stops

// CORRECT ORDER (what fixed it):
// ... pattern processing ...
triggerNote(midiBuffer);         // ← Note-ons added first  
processActiveNotes(midiBuffer);  // ← Note-offs processed last
// Result: Both messages in same buffer, proper note durations
```

## Required Debugging Tools

### 1. MIDI Monitor Applications
- **iPadOS**: MIDI Monitor AUv3 plugin - Essential for seeing actual MIDI output
- **macOS**: Console MIDI monitors or DAW built-in MIDI monitoring
- **Purpose**: Shows the actual MIDI messages reaching the host/DAW

### 2. Internal Debug Counters
Since console debugging (DBG) doesn't work in AUv3, use UI-based debug display:
```cpp
// In UI timer callback:
juce::String debugText = "Triggers: " + juce::String(processor.getDebugTriggerCount()) +
                        " | Active: " + juce::String(processor.getDebugActiveNotesCount()) +
                        " | Offs: " + juce::String(processor.getDebugNoteOffsSent());
```

## Diagnostic Sequence

### Step 1: Check MIDI Monitor Output FIRST
Look for the actual MIDI messages in external monitoring:
- **Expected**: Both note-on (0x90) AND note-off (0x80) messages
- **🚨 WARNING FLAG**: Note-offs appearing before note-ons = timing issue
- **🚨 WARNING FLAG**: Only note-ons with no note-offs = note-offs not reaching output

### Step 2: Check Internal Debug Counters
Verify the internal logic is executing:
- **Triggers**: Number of notes triggered (should match note-on count)
- **Active**: Current notes in tracking system (should accumulate then decrease)  
- **Offs**: Note-offs sent internally (should match triggers eventually)
- **🚨 WARNING FLAG**: High "Offs" count but no note-offs in MIDI monitor = timing issue

### Step 3: Verify processBlock Order
Ensure correct sequence in the main audio processing method:
```cpp
void SerpeAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    // ... transport and timing logic ...
    
    // STEP 1: Pattern processing (adds note-ons to buffer)
    if (shouldTriggerStep) {
        processStep(midiMessages, samplePosition, currentStep);
            // ↳ calls triggerNote(midiMessages, position, isAccented)
    }
    
    // STEP 2: Note tracking (sends note-offs for expired notes)
    processActiveNotes(midiMessages, bufferSize);
    
    // STEP 3: Update timing for next buffer
    absoluteSamplePosition += bufferSize;
}
```

### Step 4: Cross-Platform Verification
Test identical patterns on both platforms:
1. Use same UPI pattern (e.g., `{E(1,3)}E(5,8)`)
2. Record MIDI output on both platforms
3. Compare note durations and MIDI monitor output
4. iPadOS issues often reveal timing problems masked on desktop

## Common Issues and Solutions

### Issue: Wrong processBlock Order
**Symptoms**: 
- Internal counters show note-offs being sent
- MIDI monitor shows only note-ons
- Notes have extremely long durations

**Solution**: Move `processActiveNotes()` to END of processBlock

### Issue: Invalid Buffer Positions  
**Symptoms**:
- Some note-offs missing randomly
- Debug counter shows fewer offs than expected

**Solution**: Validate buffer positions before sending:
```cpp
if (bufferPosition >= 0 && bufferPosition < bufferSize) {
    midiBuffer.addEvent(noteOff, bufferPosition);
}
```

### Issue: Zero Velocity Note-offs
**Symptoms**:
- Note-offs appear in MIDI monitor but notes don't stop
- Instruments ignore the note-off messages

**Solution**: Use non-zero velocity for note-offs:
```cpp
// WRONG: Some hosts/instruments ignore zero velocity
juce::MidiMessage::noteOff(channel, noteNumber, 0.0f);

// CORRECT: Use moderate velocity
juce::MidiMessage::noteOff(channel, noteNumber, 0.5f);
```

## Warning Signs to Watch For

### Red Flags in MIDI Monitor:
- ✅ Note-offs before note-ons (wrong timing order)
- ✅ Missing note-offs entirely (timing or validation issue)
- ✅ All note-offs at same timing position (buffer position bug)

### Red Flags in Debug Counters:
- ✅ High "Offs" count with no MIDI note-offs (timing issue)
- ✅ "Active" count keeps growing (notes not being marked inactive)
- ✅ "Triggers" increasing but no MIDI note-ons (different issue)

### Red Flags in Behavior:
- ✅ Works on desktop but not iPadOS (timing sensitivity)
- ✅ Notes stop when transport stops (note-offs sent too late)
- ✅ Extremely long note durations (>1000ms) (missing note-offs)

## Prevention Guidelines

1. **Always implement UI-based debugging** for AUv3 plugins
2. **Test on both platforms early** in development cycle  
3. **Use MIDI monitoring tools** from the beginning
4. **Validate all MIDI buffer operations** with bounds checking
5. **Document processBlock order** with clear comments
6. **Never assume desktop behavior** translates to iPadOS

## Example Debug Session

A real debugging session that led to this documentation:

1. **Problem**: Notes had long durations on iPadOS, worked on macOS
2. **First attempt**: Assumed tracking system was broken
3. **Debug evidence**: 53 triggers, 52 offs counted internally
4. **MIDI monitor**: Only note-ons, zero note-offs
5. **Key insight**: Note-offs were being sent, but not appearing in output
6. **Root cause**: processActiveNotes() called before triggerNote()
7. **Solution**: Move processActiveNotes() to end of processBlock
8. **Result**: Proper note durations on both platforms

This sequence shows how multiple debugging tools were needed to identify the real issue.