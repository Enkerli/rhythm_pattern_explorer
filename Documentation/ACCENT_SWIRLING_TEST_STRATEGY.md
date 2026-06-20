# Accent Swirling Prevention - Testing Strategy

## Overview
This document outlines comprehensive testing to verify the resolution of accent pattern swirling issues through the derived indices architecture implementation.

## Test Categories

### 1. Polymetric Accent Patterns (Primary Target)
These patterns create polymetric relationships between rhythm and accent patterns, which historically triggered swirling.

#### Core Test Cases
- **{10}E(5,8)** - The Quintillo: 8-step rhythm with 2-step accent (LCM=10)
- **{10100011000}110110111101110** - The Culprit: Complex polymetric relationship
- **{101}E(3,8)** - Simple 3-accent on 8-step pattern (LCM=24)
- **{1010}E(7,16)** - 4-accent on 16-step pattern (LCM=16)
- **{110}E(5,13)** - Prime-length pattern with 3-accent (LCM=39)

#### Mathematical Relationships to Test
- **Short accent, long rhythm**: {10}E(11,16) - accent repeats before rhythm
- **Long accent, short rhythm**: {101010101}E(3,4) - rhythm repeats before accent  
- **Prime relationships**: {11}E(7,13) - both pattern lengths are prime
- **Power-of-2 vs odd**: {1010}E(9,16) - even rhythm, odd onset count

### 2. Temporal Stability Tests
Verify accent patterns remain stable across various timing scenarios.

#### Tempo Change Tests
- **Gradual tempo ramping**: 60 BPM → 180 BPM over 30 seconds
- **Sudden tempo jumps**: 120 BPM → 80 BPM → 160 BPM
- **Extreme tempos**: Test at 40 BPM and 200 BPM boundaries

#### DAW Integration Tests
- **Loop boundary crossing**: Verify accents maintain integrity across loop points
- **Transport start/stop**: Multiple play/pause cycles
- **Position jumping**: DAW cursor jumps to arbitrary positions
- **Sample rate changes**: 44.1kHz → 48kHz → 96kHz

### 3. UI/MIDI Synchronization Verification
Ensure visual accent markers always match audible MIDI accents.

#### Visual Verification Tests
- **Real-time observation**: Watch accent markers during 10+ pattern cycles
- **Cycle boundary alignment**: Verify UI updates only at rhythm cycle boundaries
- **Pattern modification**: Toggle steps and verify accent markers adjust correctly
- **Multiple UI instances**: Test with multiple plugin instances

#### MIDI Verification Tests
- **MIDI capture analysis**: Record MIDI output and verify accent timing
- **Audio rendering**: Export audio and verify accent velocity differences
- **Cross-reference**: Compare UI display against actual MIDI output

### 4. Edge Case Robustness
Test scenarios that historically caused issues.

#### Pattern Complexity Tests
- **Maximum length patterns**: Test with 64-step rhythms
- **Single-step patterns**: E(1,1) with accents
- **All-rest patterns**: E(0,8) edge case
- **Dense patterns**: E(15,16) nearly full patterns

#### Progressive Transformation Tests
- **Progressive offset**: {10}E(5,8)%1, %2, %3...
- **Progressive lengthening**: E(3,8)*1, *2, *3...
- **Scene cycling**: Multiple patterns with different accent structures
- **Combined transformations**: Progressive + scene cycling

### 5. Performance Stress Tests
Verify the derived indices system performs well under load.

#### High-Frequency Tests
- **Multiple instances**: 8+ plugin instances simultaneously
- **Complex patterns**: All instances running different polymetric patterns
- **Rapid pattern changes**: UPI updates every few seconds
- **Memory monitoring**: Check for memory leaks during extended operation

## Success Criteria

### Primary Success Indicators
1. **No Visual Swirling**: Accent markers remain stationary during pattern playback
2. **UI/MIDI Synchronization**: Visual accents always match audible accents
3. **Cycle Integrity**: Accent patterns repeat correctly at LCM boundaries
4. **Temporal Stability**: Accent patterns stable across tempo/timing changes

### Performance Benchmarks
- **CPU Usage**: No significant increase from legacy implementation
- **Memory Usage**: Stable memory footprint during extended testing  
- **Timing Accuracy**: ±1 sample accuracy for accent placement
- **UI Responsiveness**: No noticeable lag in accent marker updates

## Testing Methodology

### Automated Testing
```cpp
// Example verification code
bool verifyAccentPatternStability(const std::string& pattern, int cycles) {
    // Load pattern and run for specified cycles
    // Capture UI accent maps at each cycle boundary
    // Verify identical accent placement across cycles
    // Return true if stable, false if swirling detected
}
```

### Manual Testing Protocol
1. **Load test pattern** in fresh DAW project
2. **Set tempo** to 120 BPM initially
3. **Play for 10 cycles** minimum, observing UI accent markers
4. **Verify synchronization** between visual and audible accents
5. **Test tempo changes** while monitoring for swirling
6. **Document results** with pattern name, tempo, and observations

### Regression Testing
- **Daily builds**: Run core test suite on every commit
- **Release validation**: Full test suite before any release
- **Performance benchmarking**: Weekly performance regression checks
- **Platform testing**: macOS, Windows, and iOS validation

## Expected Outcomes

### Immediate Verification
- **The Quintillo** ({10}E(5,8)): Stable 2-accent pattern every 10 onsets
- **The Culprit**: Complex pattern maintains accent structure integrity
- **Tempo independence**: Accent patterns stable from 60-180 BPM
- **UI consistency**: Visual markers match MIDI output 100% of time

### Long-term Validation
- **Extended sessions**: 4+ hour sessions without swirling
- **Complex projects**: Projects with multiple polymetric patterns
- **Live performance**: Real-time pattern manipulation stability
- **Cross-platform**: Identical behavior across all supported platforms

## Documentation Requirements

### Test Results Documentation
- **Pattern name and notation**
- **Test duration and cycle count**
- **Tempo range tested**
- **Any observed anomalies**
- **Platform and build information**

### Issue Reporting Format
```
Pattern: {accent}rhythm_notation
Issue: [Brief description]
Reproduction: [Step-by-step instructions]
Expected: [What should happen]
Actual: [What actually happened]
Platform: [OS, DAW, sample rate]
Build: [Git commit hash]
```

This comprehensive testing strategy ensures the derived indices architecture successfully prevents accent swirling across all use cases and edge conditions.