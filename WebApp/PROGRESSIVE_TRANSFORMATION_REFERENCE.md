# Progressive Transformation Reference

## Overview
This document provides comprehensive test results for implementing progressive transformations in the Rhythm Pattern Explorer plugin. It compares all 8 progressive transformations with the webapp implementation to ensure accurate plugin behavior.

## Test Patterns

### 1. E(8,8)B>1 - Barlow Concentration: 8 to 1 onsets
**Algorithm:** Barlow Indispensability  
**Mode:** Concentration (removing onsets)  

| Step | Onsets | Pattern  | Description |
|------|--------|----------|-------------|
| Base | 8      | 11111111 | All beats active |
| 1    | 7      | 10111111 | Remove position 1 (least indispensable) |
| 2    | 6      | 10101111 | Remove position 3 (least indispensable) |
| 3    | 5      | 10101011 | Remove position 5 (least indispensable) |
| 4    | 4      | 10101010 | Remove position 7 (least indispensable) |
| 5    | 3      | 10001010 | Remove position 2 (medium indispensable) |
| 6    | 2      | 10001000 | Remove position 6 (medium indispensable) |
| 7    | 1      | 10000000 | Remove position 4, keep downbeat |

### 2. E(1,8)B>8 - Barlow Dilution: 1 to 8 onsets  
**Algorithm:** Barlow Indispensability  
**Mode:** Dilution (adding onsets)  

| Step | Onsets | Pattern  | Description |
|------|--------|----------|-------------|
| Base | 1      | 10000000 | Only downbeat |
| 1    | 2      | 10001000 | Add position 4 (highest indispensability) |
| 2    | 3      | 10101000 | Add position 2 (medium indispensability) |
| 3    | 4      | 10101010 | Add position 6 (medium indispensability) |
| 4    | 5      | 11101010 | Add position 1 (least indispensable) |
| 5    | 6      | 11111010 | Add position 3 (least indispensable) |
| 6    | 7      | 11111110 | Add position 5 (least indispensable) |
| 7    | 8      | 11111111 | Add position 7 (least indispensable) |

### 3. E(8,8)W>1 - Wolrab Concentration: 8 to 1 onsets
**Algorithm:** Wolrab (Anti-Barlow)  
**Mode:** Concentration (removing onsets)  

| Step | Onsets | Pattern  | Description |
|------|--------|----------|-------------|
| Base | 8      | 11111111 | All beats active |
| 1    | 7      | 01111111 | Remove position 0 (most indispensable - downbeat!) |
| 2    | 6      | 01110111 | Remove position 4 (highest indispensability) |
| 3    | 5      | 01010111 | Remove position 2 (medium indispensable) |
| 4    | 4      | 01010101 | Remove position 6 (medium indispensable) |
| 5    | 3      | 00010101 | Remove position 1 (least indispensable) |
| 6    | 2      | 00000101 | Remove position 3 (least indispensable) |
| 7    | 1      | 00000001 | Remove position 5, keep position 7 |

### 4. E(1,8)W>8 - Wolrab Dilution: 1 to 8 onsets
**Algorithm:** Wolrab (Anti-Barlow)  
**Mode:** Dilution (adding onsets)  

| Step | Onsets | Pattern  | Description |
|------|--------|----------|-------------|
| Base | 1      | 10000000 | Only downbeat |
| 1    | 2      | 11000000 | Add position 1 (least indispensable) |
| 2    | 3      | 11010000 | Add position 3 (least indispensable) |
| 3    | 4      | 11010100 | Add position 5 (least indispensable) |
| 4    | 5      | 11010101 | Add position 7 (least indispensable) |
| 5    | 6      | 11110101 | Add position 2 (medium indispensable) |
| 6    | 7      | 11110111 | Add position 6 (medium indispensable) |
| 7    | 8      | 11111111 | Add position 4 (highest indispensability) |

### 5. E(8,8)E>1 - Euclidean Concentration: 8 to 1 onsets
**Algorithm:** Euclidean (Bjorklund)  
**Mode:** Concentration  

| Step | Onsets | Pattern  | Description |
|------|--------|----------|-------------|
| Base | 8      | 11111111 | All beats active |
| 1    | 7      | 11111011 | E(7,8) - maximally even distribution |
| 2    | 6      | 11011011 | E(6,8) - maximally even distribution |
| 3    | 5      | 10110110 | E(5,8) - maximally even distribution |
| 4    | 4      | 10101010 | E(4,8) - maximally even distribution |
| 5    | 3      | 10010010 | E(3,8) - maximally even distribution |
| 6    | 2      | 10010000 | E(2,8) - maximally even distribution |
| 7    | 1      | 10000000 | E(1,8) - single downbeat |

### 6. E(1,8)E>8 - Euclidean Dilution: 1 to 8 onsets
**Algorithm:** Euclidean (Bjorklund)  
**Mode:** Dilution  

| Step | Onsets | Pattern  | Description |
|------|--------|----------|-------------|
| Base | 1      | 10000000 | E(1,8) - single downbeat |
| 1    | 2      | 10010000 | E(2,8) - maximally even distribution |
| 2    | 3      | 10010010 | E(3,8) - maximally even distribution |
| 3    | 4      | 10101010 | E(4,8) - maximally even distribution |
| 4    | 5      | 10110110 | E(5,8) - maximally even distribution |
| 5    | 6      | 11011011 | E(6,8) - maximally even distribution |
| 6    | 7      | 11111011 | E(7,8) - maximally even distribution |
| 7    | 8      | 11111111 | E(8,8) - all beats active |

### 7. E(8,8)D>1 - Dilcue Concentration: 8 to 1 onsets
**Algorithm:** Dilcue (Anti-Euclidean)  
**Mode:** Concentration  

| Step | Onsets | Pattern  | Description |
|------|--------|----------|-------------|
| Base | 8      | 11111111 | All beats active |
| 1    | 7      | 01111111 | Complement of E(1,8) |
| 2    | 6      | 01101111 | Complement of E(2,8) |
| 3    | 5      | 01101101 | Complement of E(3,8) |
| 4    | 4      | 01010101 | Complement of E(4,8) |
| 5    | 3      | 01001001 | Complement of E(5,8) |
| 6    | 2      | 00100100 | Complement of E(6,8) |
| 7    | 1      | 00000100 | Complement of E(7,8) |

### 8. E(1,8)D>8 - Dilcue Dilution: 1 to 8 onsets
**Algorithm:** Dilcue (Anti-Euclidean)  
**Mode:** Dilution  

| Step | Onsets | Pattern  | Description |
|------|--------|----------|-------------|
| Base | 1      | 10000000 | Single downbeat |
| 1    | 2      | 00100100 | Complement of E(6,8) |
| 2    | 3      | 01001001 | Complement of E(5,8) |
| 3    | 4      | 01010101 | Complement of E(4,8) |
| 4    | 5      | 01101101 | Complement of E(3,8) |
| 5    | 6      | 01101111 | Complement of E(2,8) |
| 6    | 7      | 01111111 | Complement of E(1,8) |
| 7    | 8      | 11111111 | All beats active |

## Algorithm Implementation Details

### Barlow Indispensability Algorithm
Based on Clarence Barlow's concept of indispensability values for different metrical positions.

**Indispensability Table for 8 steps:**
```
Position 0: 1.000 (downbeat - most indispensable)
Position 1: 0.333 (least indispensable)
Position 2: 0.666 (medium indispensable)
Position 3: 0.333 (least indispensable)
Position 4: 0.833 (high indispensable - halfway point)
Position 5: 0.333 (least indispensable)
Position 6: 0.666 (medium indispensable)
Position 7: 0.333 (least indispensable)
```

**Transformation Rules:**
- **Concentration (removing onsets):** Remove least indispensable onsets first, preserve downbeat
- **Dilution (adding onsets):** Add onsets to most indispensable empty positions first

### Wolrab (Anti-Barlow) Algorithm
Inverts the Barlow logic to create anti-metrical patterns.

**Transformation Rules:**
- **Concentration (removing onsets):** Remove most indispensable onsets first (including downbeat!)
- **Dilution (adding onsets):** Add onsets to least indispensable empty positions first

### Euclidean Algorithm
Uses the Bjorklund algorithm to create maximally even distributions.

**Transformation Rules:**
- Always generates the standard Euclidean pattern E(k,n) for k onsets in n steps
- Creates the most evenly spaced distribution possible

### Dilcue (Anti-Euclidean) Algorithm
Generates the complement of Euclidean patterns.

**Transformation Rules:**
- For k onsets in n steps, generates the complement of E(n-k,n)
- Creates maximally uneven distributions (the opposite of Euclidean)

## Plugin Implementation Checklist

### Base Pattern Generation
- [ ] E(8,8) generates "11111111" (all 8 positions active)
- [ ] E(1,8) generates "10000000" (only downbeat active)

### Barlow Transformer
- [ ] Implements indispensability table calculation for any step count
- [ ] Concentration mode removes least indispensable onsets first
- [ ] Preserves downbeat (position 0) during concentration
- [ ] Dilution mode adds onsets to most indispensable empty positions
- [ ] Progressive stepping works for any start/end onset count

### Wolrab Transformer  
- [ ] Uses same indispensability table as Barlow but inverts logic
- [ ] Concentration mode removes most indispensable onsets first (including downbeat)
- [ ] Dilution mode adds onsets to least indispensable empty positions
- [ ] Progressive stepping works for any start/end onset count

### Euclidean Transformer
- [ ] Implements Bjorklund algorithm for any (k,n) combination
- [ ] Progressive stepping generates E(k,n) for each intermediate k
- [ ] Handles edge cases (k=0, k=n)

### Dilcue Transformer
- [ ] Generates complement patterns correctly
- [ ] For k onsets, uses complement of E(n-k,n) pattern
- [ ] Progressive stepping works through complement sequence

### Progressive UPI Notation
- [ ] Parses patterns like "E(8,8)B>1" correctly
- [ ] Supports all 4 transformer types (B, W, E, D)
- [ ] Enables step-by-step progression with Enter key
- [ ] Displays transformation progress clearly

## Test Validation

Use the patterns above to validate your plugin implementation:

1. **Generate base patterns** and verify they match expected binary strings
2. **Run progressive transformations** and compare each step with the expected output
3. **Test edge cases** like same source/target onset counts
4. **Verify algorithm behavior** matches the webapp's mathematical approach

The test file `/Users/alex/Documents/Coding/rhythm_pattern_explorer/progressive_test_simple.js` can be run to regenerate these results for verification.

## Key Differences Between Algorithms

| Algorithm | Concentration Strategy | Dilution Strategy | Downbeat Treatment |
|-----------|----------------------|-------------------|-------------------|
| Barlow (B) | Remove least indispensable | Add to most indispensable | Always preserved |
| Wolrab (W) | Remove most indispensable | Add to least indispensable | Can be removed |
| Euclidean (E) | Generate E(k,n) | Generate E(k,n) | Depends on math |
| Dilcue (D) | Generate complement | Generate complement | Depends on math |

This comprehensive reference should enable accurate implementation of all 8 progressive transformations in the plugin, with exact pattern matching to the webapp behavior.