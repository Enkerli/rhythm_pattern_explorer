# Quantization Interactive Test Plan
## Lascabettes-Style Quantization Testing for Host DAWs

**Debug Build Version**: v0.03f with debug logging  
**Test Date**: July 2025  
**Debug Log Location**: `~/Desktop/RhythmPatternExplorer_Debug.log`

---

### Pre-Test Setup

#### 1. Build Configuration
- [ ] Ensure debug build is compiled with `-DDEBUG` flag
- [ ] Verify DebugLogger infrastructure is active
- [ ] Confirm debug log file is created at plugin initialization

#### 2. Host DAW Setup
- [ ] Load fresh instance of Rhythm Pattern Explorer plugin
- [ ] Set host transport BPM to 120 (standard tempo)
- [ ] Enable host transport synchronization in plugin
- [ ] Create MIDI track to receive plugin output

#### 3. Debug Log Verification
- [ ] Check desktop for `RhythmPatternExplorer_Debug.log` file
- [ ] Verify initial session header appears in log:
  ```
  ================================================================================
  DEBUG SESSION STARTED: [timestamp]
  Rhythm Pattern Explorer - Quantization Debug Logging
  ================================================================================
  ```

---

### Test Cases

#### **Test Category 1: Basic Quantization Functionality**

##### Test 1.1: Simple Euclidean Quantization
**Input**: `E(3,8);12`  
**Expected**: 3 onsets from 8-step tresillo quantized to 12 steps  
**Debug Log Check**: Look for:
```
[QUANT] Quantization: 'E(3,8);12' | 8→12 steps | Direction: CW
```
**Manual Verification**:
- [ ] Pattern displays as 12 steps instead of 8
- [ ] Visual direction indicator shows "CW" 
- [ ] Original pattern still has tresillo-like spacing
- [ ] Play pattern and verify timing feels natural

##### Test 1.2: Counterclockwise Quantization  
**Input**: `E(5,13);-16`  
**Expected**: 5 onsets from 13-step pattern quantized to 16 steps counterclockwise  
**Debug Log Check**: Look for:
```
[QUANT] Quantization: 'E(5,13);-16' | 13→16 steps | Direction: CCW
```
**Manual Verification**:
- [ ] Pattern displays as 16 steps
- [ ] Direction indicator shows "CCW"
- [ ] Different onset placement than clockwise version
- [ ] Play and compare feel vs clockwise equivalent

##### Test 1.3: Edge Case - Same Step Count
**Input**: `E(4,16);16`  
**Expected**: No actual quantization (16→16), but notation processed  
**Debug Log Check**: Look for:
```
[QUANT] Quantization: 'E(4,16);16' | 16→16 steps | Direction: CW
```
**Manual Verification**:
- [ ] Pattern identical to `E(4,16)` without quantization
- [ ] Quantization info still displays correctly
- [ ] No performance degradation

#### **Test Category 2: Complex Pattern Integration**

##### Test 2.1: Quantization with Accents
**Input**: `{101}E(3,8);13`  
**Expected**: Accented tresillo quantized to 13 steps with preserved accent pattern  
**Debug Log Check**: Look for both:
```
[QUANT] Quantization: '{101}E(3,8);13' | 8→13 steps | Direction: CW
[MIDI ] MIDI Trigger: '{101}E(3,8);13' | Velocity: 127 | Accented: YES
[MIDI ] MIDI Trigger: '{101}E(3,8);13' | Velocity: 102 | Accented: NO
```
**Manual Verification**:
- [ ] First onset is accented (higher velocity/pitch)
- [ ] Accent pattern cycles properly with quantized rhythm
- [ ] Visual accent indicators (red dots) appear correctly

##### Test 2.2: Progressive Quantization  
**Input**: `E(1,8);12B>12`  
**Expected**: Progressive Barlow transformation of quantized pattern  
**Debug Log Check**: Look for quantization followed by progressive updates:
```
[QUANT] Quantization: 'E(1,8);12B>12' | 8→12 steps | Direction: CW
[UI   ] Pattern Input: User entered: 'E(1,8);12B>12'
```
**Manual Verification**:
- [ ] Initial pattern shows 1 onset in 12 steps 
- [ ] Manual triggers (Enter/MIDI) advance progressive transformation
- [ ] Final pattern reaches full Barlow(12,12) configuration
- [ ] Step indicator resets properly on manual triggers

##### Test 2.3: Scene Cycling with Quantization
**Input**: `E(3,8);12|E(5,13);16|E(2,7);11`  
**Expected**: Three scenes with different quantizations that cycle properly  
**Debug Log Check**: Look for scene cycling with different quantization:
```
[QUANT] Quantization: 'E(3,8);12' | 8→12 steps | Direction: CW
[UI   ] Pattern Input: User entered: 'E(3,8);12|E(5,13);16|E(2,7);11'
[QUANT] Quantization: 'E(5,13);16' | 13→16 steps | Direction: CW
```
**Manual Verification**:
- [ ] Scene 1: Tresillo → 12 steps
- [ ] Scene 2: 5-onset → 16 steps  
- [ ] Scene 3: 2-onset → 11 steps
- [ ] Manual triggers cycle through scenes correctly
- [ ] Display updates show current scene quantization info

#### **Test Category 3: Host DAW Integration**

##### Test 3.1: Transport Synchronization
**Setup**: Set host transport to different BPMs (80, 120, 160, 200)  
**Input**: `E(5,17);13` (complex quantization)  
**Debug Log Check**: Look for timing correlation:
```
[TIME ] Timing: Position 4.000 | Step 2 | Playing: YES
[QUANT] Quantization: 'E(5,17);13' | 17→13 steps | Direction: CW
```
**Manual Verification**:
- [ ] Pattern timing stays locked to host transport at all BPMs
- [ ] Step indicator advances properly with host playback
- [ ] No timing drift over extended playback (test 32+ bars)
- [ ] Quantized patterns maintain musical feel at different tempos

##### Test 3.2: MIDI Input Triggering
**Setup**: Send MIDI notes to plugin while host plays  
**Input**: `{110}E(3,8);12B>12` (accents + quantization + progressive)  
**Debug Log Check**: Look for MIDI trigger responses:
```
[MIDI ] MIDI Trigger: '{110}E(3,8);12B>12' | Velocity: 127 | Accented: YES
[QUANT] Quantization: 'E(3,8);12B>12' | 8→12 steps | Direction: CW
```
**Manual Verification**:
- [ ] MIDI input advances progressive transformation
- [ ] Quantized pattern maintains proper progressive behavior
- [ ] Accent patterns stay synchronized with rhythm changes
- [ ] No double-triggering or missed triggers

##### Test 3.3: Parameter Automation
**Setup**: Automate plugin parameters while using quantized patterns  
**Input**: `E(7,16);20` with automated MIDI note parameter  
**Debug Log Check**: Monitor parameter changes don't affect quantization:
```
[QUANT] Quantization: 'E(7,16);20' | 16→20 steps | Direction: CW
[TIME ] Timing: Position 8.000 | Step 4 | Playing: YES
```
**Manual Verification**:
- [ ] MIDI note changes work correctly with quantized patterns
- [ ] Transport enable/disable doesn't break quantization
- [ ] Velocity parameters affect quantized notes properly
- [ ] No audio glitches during parameter automation

#### **Test Category 4: Edge Cases and Error Handling**

##### Test 4.1: Invalid Quantization Syntax
**Input**: `E(3,8);` (missing target steps)  
**Expected**: Pattern processes as normal E(3,8), ignores incomplete quantization  
**Debug Log Check**: Should NOT show quantization log entry  
**Manual Verification**:
- [ ] Pattern falls back to standard E(3,8) 
- [ ] No quantization indicators in UI
- [ ] No crashes or unexpected behavior

##### Test 4.2: Extreme Quantization Values
**Input**: `E(2,5);100` (very large quantization)  
**Expected**: Large pattern with proper angular spacing  
**Debug Log Check**: Look for:
```
[QUANT] Quantization: 'E(2,5);100' | 5→100 steps | Direction: CW
```
**Manual Verification**:
- [ ] Pattern displays correctly (may need scrolling/zooming)
- [ ] Performance remains acceptable
- [ ] Angular mapping preserves musical relationships

##### Test 4.3: Unicode and Special Characters  
**Input**: `E(3,8);12` with various copy/paste scenarios  
**Expected**: Robust parsing regardless of input method  
**Debug Log Check**: Verify clean pattern strings in logs  
**Manual Verification**:
- [ ] Copy/paste from documentation works
- [ ] Type directly in plugin text field works
- [ ] No encoding issues affect parsing

---

### Performance Testing

#### Load Testing
**Setup**: Multiple instances of plugin with quantized patterns  
**Test**: Load 8+ instances each with complex quantization like `{E(3,7)}E(5,13);17B>17`  
**Monitor**: CPU usage, audio dropouts, log file performance  
**Success Criteria**:
- [ ] CPU usage remains reasonable (< 10% per instance)
- [ ] No audio dropouts or timing issues
- [ ] Debug logging doesn't cause performance problems

#### Memory Testing  
**Setup**: Extended session with pattern switching  
**Test**: Cycle through 50+ different quantized patterns over 30 minutes  
**Monitor**: Memory usage, log file size, plugin stability  
**Success Criteria**:
- [ ] Memory usage stays stable (no leaks)
- [ ] Log file grows at predictable rate
- [ ] Plugin remains responsive throughout test

---

### Expected Debug Log Structure

A successful test session should show logs like:
```
[GENER] Plugin initialized - Debug logging active
[UI   ] Pattern Input: User entered: 'E(3,8);12'
[QUANT] Quantization: 'E(3,8);12' | 8→12 steps | Direction: CW
[MIDI ] MIDI Trigger: 'E(3,8);12' | Velocity: 102 | Accented: NO
[TIME ] Timing: Position 2.000 | Step 1 | Playing: YES
[GENER] Plugin shutting down - Debug logging cleanup
```

---

### Cleanup and Analysis

#### Post-Test Actions
- [ ] Stop host transport and plugin playback
- [ ] Save/export debug log file for analysis
- [ ] Note any unexpected behaviors or crashes
- [ ] Document performance characteristics observed

#### Success Criteria Summary
- [ ] All quantization test cases pass
- [ ] Debug logging captures all expected events
- [ ] No crashes or audio dropouts during testing
- [ ] Host DAW integration works smoothly
- [ ] Performance remains acceptable under load
- [ ] Angular mapping produces musically useful results

#### Final Report
- Create summary of test results
- Identify any issues requiring fixes before release
- Recommend production build configuration
- Archive debug logs for future reference

---

**IMPORTANT**: This is a debug test build. Before distribution:
1. Remove all DEBUG_LOG statements
2. Disable DebugLogger initialization  
3. Compile release build without debug flags
4. Verify no performance impact from debug infrastructure