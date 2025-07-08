# Accent Pattern Tests

## Test Cases for Accent Pattern Implementation

### Basic Accent Patterns

1. **Simple Binary Accent**
   - Input: `{100}E(3,8)`
   - Expected: 
     - Rhythm: `10010010` (tresillo)
     - Accent: `100` (cycles: `10010010`)
     - Result: First onset is accented, others are not

2. **Accent After Pattern**
   - Input: `E(3,8){100}`
   - Expected: Same as above (accent position shouldn't matter)

3. **Complex Accent Cycling**
   - Input: `{10010}E(5,8)`
   - Expected:
     - Rhythm: `10101010` (quintillo)
     - Accent: `10010` (cycles as needed)
     - Result: Steps 0 and 3 are accented

### Pattern Length Mismatches (Polyrhythmic Feel)

4. **Short Accent on Long Pattern**
   - Input: `{10}E(4,8)`
   - Expected:
     - Rhythm: `10101010`
     - Accent: `10` (cycles: `10101010`)
     - Result: Alternating accented/unaccented

5. **Long Accent on Short Pattern**
   - Input: `{100100}E(3,4)`
   - Expected:
     - Rhythm: `1011`
     - Accent: `100100` (cycles through pattern)
     - Result: Accents cycle creating polyrhythmic effect

### Advanced Accent Patterns

6. **Euclidean Accent Pattern**
   - Input: `{E(2,5)}E(3,8)`
   - Expected:
     - Rhythm: `10010010`
     - Accent: `10101` (from E(2,5))
     - Result: Complex polyrhythmic accents

7. **Polygon Accent Pattern**
   - Input: `{P(3,0)}E(5,8)`
   - Expected:
     - Rhythm: `10101010`
     - Accent: `111` (from P(3,0))
     - Result: All onsets accented in groups of 3

### MIDI Output Expectations

For each accented note:
- **Pitch**: Base note + 5 semitones
- **Velocity**: 1.0 (vs 0.8 for regular notes)
- **Visual/Debug**: Clear indication of accent in logs

## Implementation Status

✅ Basic parsing with curly bracket notation  
✅ Binary accent patterns (`{100}`, `{10010}`)  
✅ Recursive parsing for complex patterns (`{E(2,5)}`)  
✅ Pattern cycling for mismatched lengths  
✅ MIDI output with higher pitch and volume  
⏳ UI visual representation  
⏳ Comprehensive testing  

## Test Commands

```
# Basic tests
{100}E(3,8)
E(3,8){100}
{10010}E(5,8)

# Polyrhythmic tests  
{10}E(4,8)
{100100}E(3,4)

# Advanced tests
{E(2,5)}E(3,8)
{P(3,0)}E(5,8)
```