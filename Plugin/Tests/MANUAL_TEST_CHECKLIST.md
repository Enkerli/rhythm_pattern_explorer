# Comprehensive Manual Test Checklist

## Plugin Parameter Interface Tests
**Status**: ⏳ Ready to Test

### 1. Parameter Count Check
- [ ] Load plugin in DAW and verify only **3 parameters** are exposed:
  - [ ] "Use Host Transport" (boolean)
  - [ ] "MIDI Note" (integer, 0-127, default 36)
  - [ ] "Tick" (boolean)
- [ ] Verify NO legacy parameters appear: BPM, Pattern Type, Onsets, Steps, Playing

### 2. Trigger Parameter Functionality
- [ ] **Host Automation Test**: 
  - [ ] Automate Tick parameter ON/OFF rapidly (4-8 times per measure)
  - [ ] Verify each trigger advances pattern to next scene/offset
  - [ ] Verify parameter automatically resets to OFF after triggering
  - [ ] Verify multiple consecutive triggers work reliably

- [ ] **Edge Detection Test**:
  - [ ] Set Tick to ON and hold - should trigger once only
  - [ ] Set Tick to OFF then ON again - should trigger again
  - [ ] Rapid ON/OFF/ON sequences should work

### 3. MIDI Note Parameter UI Sync
- [ ] **Host to UI Sync**:
  - [ ] Change MIDI Note parameter from host (automation, controller)
  - [ ] Verify UI spinner updates to show correct value
  - [ ] Test with values: 0, 36, 60, 127

- [ ] **UI to Host Sync**:
  - [ ] Change spinner in plugin UI
  - [ ] Verify host shows updated parameter value
  - [ ] Verify MIDI input responds to new note number

## Pattern Parsing Tests
**Status**: ⏳ Ready to Test

### 4. Polygon Combination Tests (Critical Fix)
- [ ] **P(7,0)+P(11,0)**: Should produce 77-step pattern, NOT 7-step "1111111"
  - [ ] Enter pattern in plugin UI
  - [ ] Verify pattern length in display
  - [ ] Verify complex hex output (not 0x7F)
  - [ ] Compare with webapp output: `0x102240C10A0508000000:77`

- [ ] **P(3,1)+P(5,0)**: Should produce 15-step pattern (LCM of 3,5)
- [ ] **P(3,1)+P(5,0)+P(2,0)**: Should produce 30-step pattern (LCM of 3,5,2)
- [ ] **P(4,0)+P(6,0)**: Should produce 12-step pattern (LCM of 4,6)

### 5. Progressive Offset Tests (Ensure No Interference)
- [ ] **P(7,0)+2**: Should be detected as progressive offset, NOT combination
- [ ] **E(3,8)+1**: Should advance offset on each trigger
- [ ] **P(5,0)+3**: Should work with step size 3

### 6. Scene Cycling Tests
- [ ] **Basic Scenes**: `P(3,0)|E(5,8)|P(7,1)`
  - [ ] First trigger: P(3,0) pattern
  - [ ] Second trigger: E(5,8) pattern  
  - [ ] Third trigger: P(7,1) pattern
  - [ ] Fourth trigger: cycles back to P(3,0)

- [ ] **Progressive Scenes**: `P(3,0)+1|P(5,0)+2|E(3,8)+1`
  - [ ] Each scene has its own progressive offset counter
  - [ ] Verify offsets advance independently per scene

- [ ] **Combination Scenes**: `P(3,0)+P(5,0)|E(3,8)|P(7,0)+P(11,0)`
  - [ ] First scene: 15-step combination
  - [ ] Third scene: 77-step combination (the critical test)

### 7. Legacy Pattern Compatibility
- [ ] **Euclidean**: E(3,8), E(5,8), E(7,16)
- [ ] **Single Polygons**: P(3,0), P(5,1), P(7,0,2)
- [ ] **Hex Patterns**: 0x92:8, 0xB5:8
- [ ] **Binary Patterns**: 10010010, 101
- [ ] **Onset Arrays**: [0,3,6]:8

## Host Integration Tests
**Status**: ⏳ Ready to Test

### 8. DAW Compatibility Tests

#### Bitwig Studio
- [ ] **Plugin Loading**: Loads without errors
- [ ] **Parameter Automation**: All 3 parameters automatable
- [ ] **Host Transport**: Sync works correctly when enabled
- [ ] **High BPM Stability**: Test at 180+ BPM with debug logging
- [ ] **MIDI Routing**: Input/output routing works correctly

#### Logic Pro (if available)
- [ ] **AU Plugin Loading**: Component loads in Audio Units
- [ ] **Parameter Control**: Host controls work
- [ ] **Automation**: Parameter automation recording/playback
- [ ] **Transport Sync**: Logic transport controls plugin

### 9. Performance Tests
- [ ] **High BPM Stress Test**:
  - [ ] Set BPM to 200+ and verify stable triggering
  - [ ] Monitor /tmp/bitwig_debug.log for timing issues
  - [ ] Verify no missed steps or double triggers

- [ ] **Rapid Pattern Changes**:
  - [ ] Automate Tick parameter at 16th note intervals
  - [ ] Test with complex scenes and combinations
  - [ ] Monitor CPU usage and audio dropouts

- [ ] **Memory Usage**:
  - [ ] Load plugin in multiple tracks
  - [ ] Test with large pattern combinations (P(31,0)+P(29,0))
  - [ ] Verify no memory leaks during extended use

## Webapp vs Plugin Consistency Tests
**Status**: ⏳ Ready to Test

### 10. Pattern Output Verification
For each test pattern, compare webapp output with plugin output:

- [ ] **E(3,8)**: Webapp: `0x92:8` → Plugin: `0x92`
- [ ] **E(5,8)**: Webapp: `0xB5:8` → Plugin: `0xB5`
- [ ] **P(3,0)**: Webapp: `0x5:3` → Plugin: `0x5`
- [ ] **P(7,0)**: Webapp: `0x7F:7` → Plugin: `0x7F`
- [ ] **P(7,0)+P(11,0)**: Webapp: `0x102240C10A0508000000:77` → Plugin: Should match
- [ ] **P(5,0,3)**: Webapp: `0x421:15` → Plugin: Should match

### 11. Advanced Pattern Tests
- [ ] **Transformations**: ~P(3,0), rev P(3,0), comp E(3,8)
- [ ] **Rotations**: P(3,0)@1, E(3,8)@2
- [ ] **Complex Combinations**: P(3,1)+P(5,2)+P(7,0)
- [ ] **Progressive Lengthening**: P(3,0)*2, E(5,8)*3

## Error Handling Tests
**Status**: ⏳ Ready to Test

### 12. Input Validation
- [ ] **Empty Input**: Should handle gracefully
- [ ] **Invalid Syntax**: P(3,0, P(, ), etc.
- [ ] **Out of Range**: P(1,0), P(65,0), E(0,8), E(9,8)
- [ ] **Memory Limits**: Very large patterns near 64-step limit

### 13. State Persistence
- [ ] **DAW Save/Load**: Pattern state preserved across sessions
- [ ] **Parameter Values**: All 3 parameters save/restore correctly  
- [ ] **Progressive State**: Scene index and offset counters preserved

## Test Results Summary

### ✅ Completed Tests
- [ ] Parameter interface (3 params only)
- [ ] Polygon combinations work correctly
- [ ] Progressive offsets don't interfere
- [ ] Scene cycling functionality
- [ ] Trigger parameter automation
- [ ] MIDI note UI sync
- [ ] Host transport sync
- [ ] Webapp consistency
- [ ] Performance stability
- [ ] Error handling

### ❌ Failed Tests
Document any failures here with:
- Test description
- Expected behavior
- Actual behavior
- Severity (Critical/High/Medium/Low)

### 📋 Test Environment
- Plugin Version: v0.03a4 (post-polygon-fix)
- DAW: _______________
- OS: macOS 14.6.0
- Test Date: _______________
- Tester: _______________

## Quick Test Commands

### Essential Pattern Tests (Copy/Paste Ready)
```
P(7,0)+P(11,0)
P(3,1)+P(5,0)
P(3,0)|E(5,8)|P(7,1)
P(3,0)+1|P(5,0)+2
E(3,8)
P(7,0)+2
0x92:8
[0,3,6]:8
```

### Expected Outputs
- `P(7,0)+P(11,0)` → 77 steps, complex pattern
- `P(3,1)+P(5,0)` → 15 steps
- `E(3,8)` → 0x92 (tresillo)
- `P(7,0)+2` → progressive offset behavior