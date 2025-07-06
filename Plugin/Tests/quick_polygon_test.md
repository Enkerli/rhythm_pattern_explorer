# Quick Polygon Combination Test

## Test Cases for Manual Verification

### 1. Single Polygons (Should Work)
- `P(7,0)` → Expected: 7 steps, binary `1111111`, hex `0x7F`
- `P(11,0)` → Expected: 11 steps, binary `11111111111`, hex `0x7FF`

### 2. Polygon Combinations (The Fix)
- `P(7,0)+P(11,0)` → Expected: 77 steps (LCM of 7,11), complex pattern
  - Should NOT be `1111111` (7 steps)
  - Should be 77-step pattern with combined vertices

### 3. Progressive Offsets (Should Still Work)
- `P(7,0)+2` → Expected: Progressive offset by 2 steps each trigger
- `P(5,0)+1` → Expected: Progressive offset by 1 step each trigger

### 4. Scene Combinations (Should Work)
- `P(3,0)|P(7,0)+P(11,0)|E(5,8)` → 
  - Scene 1: P(3,0) - 3 steps
  - Scene 2: P(7,0)+P(11,0) - 77 steps (the critical test)
  - Scene 3: E(5,8) - 8 steps

## Mathematical Verification

### LCM Calculations
- LCM(7,11) = 77 (both are prime)
- LCM(3,5) = 15 
- LCM(4,6) = 12
- LCM(3,5,2) = 30

### Polygon Vertex Mapping
For P(7,0)+P(11,0) projected onto 77 steps:
- P(7,0): vertices at positions 0, 11, 22, 33, 44, 55, 66 (every 77/7 = 11 steps)
- P(11,0): vertices at positions 0, 7, 14, 21, 28, 35, 42, 49, 56, 63, 70 (every 77/11 = 7 steps)
- Combined: OR of both patterns = complex 77-step pattern

## Quick Test Protocol

1. **Load Plugin**: Open in Bitwig/Logic
2. **Check Parameters**: Verify only 3 params (Use Host Transport, MIDI Note, Tick)
3. **Test P(7,0)**: Enter, verify 7 steps, pattern shows `1111111`
4. **Test P(7,0)+P(11,0)**: Enter, verify 77 steps, pattern is NOT `1111111`
5. **Test Progressive**: Enter `P(7,0)+2`, trigger multiple times, verify offset advancement
6. **Test Scenes**: Enter `P(3,0)|P(7,0)+P(11,0)`, trigger twice, verify scene 2 is 77 steps

## Expected Results

### Before Fix (Broken)
- `P(7,0)+P(11,0)` would be treated as progressive offset
- Result: 7-step pattern `1111111`
- Wrong because it ignores the second polygon

### After Fix (Correct)
- `P(7,0)+P(11,0)` recognized as polygon combination
- Result: 77-step complex pattern
- LCM calculation and vertex projection working correctly

## Plugin UI Verification

In the plugin interface:
1. Pattern Display should show 77 steps for `P(7,0)+P(11,0)`
2. Hex output should be complex, not `0x7F`
3. Step count indicator should read 77
4. Pattern visualization should show sparse hits across 77 positions

## Debug Log Verification

Check `/tmp/rhythm_progressive_debug.log` for:
- Progressive offset detection: Should be FALSE for `P(7,0)+P(11,0)`
- Polygon combination detection: Should be TRUE for `P(7,0)+P(11,0)`
- LCM calculation: Should show LCM(7,11) = 77