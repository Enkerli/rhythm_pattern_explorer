# Advanced Host Sync - Interactive DAW Testing Guide

## Phase 3: Manual Testing Procedures

This guide provides comprehensive testing procedures for the newly implemented Advanced Host Sync features. Follow these tests in your preferred DAW to verify functionality.

## Prerequisites

- ✅ Plugin installed: `~/Library/Audio/Plug-Ins/Components/Rhythm Pattern Explorer.component`  
- ✅ Unit tests passed: 17/17 tests successful
- ✅ Clean Release build completed
- **DAW Required**: Logic Pro, Pro Tools, Ableton Live, or similar with AU support

---

## Test 1: Automatic Pattern Length Adjustment

### Setup
1. Create new DAW project at 120 BPM
2. Add MIDI track with Rhythm Pattern Explorer AU
3. Open plugin interface

### Test Procedure

**Test 1A: Basic Auto Mode**
1. Set Pattern Length Unit to "Auto" (4th option)
2. Enter pattern: `E(3,8)` 
3. **Expected**: Pattern duration should be ~2 beats (verify with DAW's bar/beat display)
4. Enter pattern: `E(1,16)`
5. **Expected**: Pattern duration should be ~8 beats (sparse pattern gets longer)
6. Enter pattern: `E(11,16)`  
7. **Expected**: Pattern duration should be ~2-3 beats (dense pattern gets shorter)

**Test 1B: Auto vs Manual Comparison**
1. Pattern: `E(3,8)`
2. Set to "Auto" → Note the duration
3. Set to "Beats" with value "4" → Compare timing difference
4. Set back to "Auto" → Should return to intelligent duration
5. **Expected**: Auto mode provides musically appropriate timing, Beats mode uses fixed duration

**Test 1C: Special Pattern Recognition**
1. Pattern: `E(3,7)` in Auto mode → Should get ~2 beats
2. Pattern: `E(3,5)` in Auto mode → Should get ~2.5 beats  
3. Pattern: `E(4,5)` in Auto mode → Should get ~1.25 beats
4. **Expected**: Quintillo and Afro-Cuban patterns get appropriate musical durations

### Verification Criteria
- [ ] Auto mode produces different durations for different patterns
- [ ] Sparse patterns get longer durations than dense patterns
- [ ] Common mathematical patterns (tresillo, quintillo) get musically standard durations
- [ ] Auto mode can be switched back and forth with manual modes without issues

---

## Test 2: Host Loop Point Synchronization

### Setup
1. Create 4-bar loop region in DAW
2. Set DAW to loop mode
3. Add Rhythm Pattern Explorer with pattern `E(3,8)`
4. Set Pattern Length Unit to "Beats" with value "2"

### Test Procedure

**Test 2A: Basic Loop Sync**
1. Start playback in loop mode
2. Let loop play several cycles
3. **Expected**: Pattern stays in sync with loop boundaries
4. **Expected**: Pattern resets to step 1 at each loop restart

**Test 2B: Loop Restart Detection**
1. Start playback mid-loop (not at loop start)
2. Let it loop around to beginning
3. **Expected**: Pattern should reset when DAW jumps back to loop start
4. Try manual loop restart (jump to loop beginning while playing)
5. **Expected**: Pattern should immediately reset to step 1

**Test 2C: Accent Pattern Loop Sync**
1. Use pattern: `{100}E(3,8)` (accented tresillo)
2. Play in loop mode
3. **Expected**: Red accent marker should reset to first onset on each loop restart
4. **Expected**: Accent pattern should not "drift" during playback

**Test 2D: Loop Sync Disable Test**
1. Pattern should have option to disable loop sync (if exposed in UI)
2. With loop sync disabled, pattern should continue across loop boundaries
3. With loop sync enabled, pattern should reset on loop restart

### Verification Criteria
- [ ] Pattern resets to step 1 on each loop restart
- [ ] Accent markers reset and stay stable during loop playback
- [ ] Pattern timing stays synchronized with host loop boundaries
- [ ] Manual loop jumps trigger immediate pattern reset
- [ ] Step indicator and visual feedback reset properly

---

## Test 3: Progressive Patterns with Loop Sync

### Setup
1. Create 8-bar loop region
2. Use progressive pattern: `E(1,8)B>8`

### Test Procedure

**Test 3A: Progressive + Loop Integration**
1. Start playback in loop mode
2. **Expected**: Progressive transformation advances on manual triggers (Enter key)
3. **Expected**: Pattern resets to step 1 on each loop restart, but maintains current progressive state
4. Trigger several progressive steps, then let loop restart
5. **Expected**: Progressive state preserved, but step position resets

**Test 3B: Scene Patterns + Loop Integration**
1. Use scene pattern: `E(3,8)|B(5,13)|E(7,16)`
2. Start playback in loop mode  
3. Use Enter or MIDI trigger to advance scenes
4. **Expected**: Scene changes don't interfere with loop restart behavior
5. **Expected**: Each scene resets to step 1 on loop restart

### Verification Criteria
- [ ] Progressive transformations work normally within loops
- [ ] Loop restart doesn't break progressive/scene state
- [ ] Manual triggers still advance progressive/scene patterns
- [ ] Visual feedback (step indicator, accent markers) reset correctly

---

## Test 4: Edge Cases and Stress Testing

### Test 4A: Rapid Loop Changes
1. Create very short loop (1 beat)
2. Pattern: `E(3,8)` with "Auto" length
3. **Expected**: Plugin should handle rapid loop restarts without crashes
4. Try loops of different lengths: 1 bar, 2 bars, 3 bars, 7 bars
5. **Expected**: Pattern should adapt and reset appropriately

### Test 4B: Transport Control Stress
1. Rapid start/stop of transport
2. Jump to different positions in timeline
3. Switch between loop and normal playback
4. **Expected**: No crashes, glitches, or stuck notes

### Test 4C: Parameter Changes During Playback
1. Change Pattern Length Unit from "Auto" to "Beats" during playback
2. Change pattern text during playback  
3. **Expected**: Smooth transitions without audio dropouts

### Verification Criteria
- [ ] No crashes or audio dropouts during stress testing
- [ ] Plugin handles rapid transport changes gracefully
- [ ] Parameter changes during playback work smoothly
- [ ] Memory usage remains stable during extended testing

---

## Test 5: Cross-DAW Compatibility

Test these features in multiple DAWs if available:

### Logic Pro X/Pro
- Test AU version with Logic's loop functionality
- Verify integration with Logic's transport controls

### Pro Tools
- Test with Pro Tools loop recording and playback
- Verify MIDI output timing accuracy

### Ableton Live
- Test with Live's loop functionality and scene launching
- Verify integration with Live's unique transport model

### Verification Criteria
- [ ] Consistent behavior across different DAW hosts
- [ ] Host-specific loop implementations work correctly  
- [ ] No DAW-specific crashes or incompatibilities

---

## Expected Results Summary

After completing all tests, the Advanced Host Sync features should demonstrate:

✅ **Automatic Pattern Length Adjustment**
- Intelligent duration calculation based on pattern complexity
- Musical heuristics for common mathematical patterns
- Density-based timing adjustment (sparse = longer, dense = shorter)
- Smooth integration with existing manual timing modes

✅ **Host Loop Point Synchronization**  
- Reliable detection of loop restart via backward position jumps
- Complete pattern state reset on loop restart (step, accents, counters)
- Preserved progressive/scene state across loop boundaries
- Stable visual feedback during loop playback

✅ **System Integration**
- Clean compilation and installation
- No crashes or audio dropouts during normal use
- Backward compatibility with existing patterns and features
- Consistent behavior across different DAW hosts

---

## Reporting Issues

If any test fails or produces unexpected behavior:

1. **Document the specific test case** that failed
2. **Record the DAW version and OS version** being used  
3. **Note the exact pattern and settings** that caused the issue
4. **Describe the expected vs actual behavior**
5. **Check if the issue reproduces consistently**

## Next Steps

Upon successful completion of interactive testing:
- Update CLAUDE.md with any discovered issues
- Merge feature branch to main if all tests pass
- Update version documentation
- Consider additional DAW-specific optimizations based on testing results

---

**Testing completed by**: [Your name]  
**Date**: [Test date]  
**DAW(s) tested**: [List DAWs used]  
**Overall result**: [Pass/Fail with notes]