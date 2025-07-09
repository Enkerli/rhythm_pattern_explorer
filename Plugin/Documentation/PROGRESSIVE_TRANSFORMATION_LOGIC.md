# Progressive Transformation Logic

## Overview

Progressive transformations in the Rhythm Pattern Explorer allow users to step through algorithmic changes to rhythm patterns. This document explains the implementation details, design decisions, and user interface behavior.

## Core Concepts

### Manual vs Automatic Control

The system supports two progressive transformation syntaxes:

1. **Manual Control (`>` syntax)**: Used in the audio plugin
   - `E(1,8)E>8` - Progressive Euclidean from 1 to 8 onsets
   - `E(8,8)B>1` - Progressive Barlow dilution from 8 to 1 onsets
   - Requires explicit triggers via button clicks or MIDI input

2. **Automatic Control (`@#` syntax)**: Used in the web application
   - `E(1,8)@#5` - Auto-advancing offset rotation
   - `E(1,8)E@#8` - Auto-advancing progression
   - Advances automatically on each beat/cycle

### Design Philosophy

The manual control system was implemented to give users precise control over pattern evolution, essential for live performance and composition. The automatic system provides generative capabilities for ambient and algorithmic music.

## Implementation Details

### State Management

```cpp
// Progressive transformation state tracking
std::map<juce::String, std::vector<bool>> progressivePatterns;
std::map<juce::String, int> progressiveStepCount;
std::map<juce::String, int> progressiveAccessCount;
```

#### Pattern Key Generation

Each progressive transformation uses a unique key for state management:
- Format: `basePattern + transformerType + targetOnsets`
- Example: `"10000000e8"` for `E(1,8)E>8`
- Ensures different transformations maintain separate state

#### LRU Cleanup System

To prevent unbounded memory growth, the system implements LRU (Least Recently Used) cleanup:
- Maximum 100 concurrent progressive patterns
- Removes oldest patterns when limit exceeded
- Tracks access count for each pattern

### Transformation Sequence

1. **First Call**: Returns base pattern unchanged
   - Sets `progressiveStepCount[key] = 1`
   - UI displays step "1"
   - Stores base pattern for next iteration

2. **Subsequent Calls**: Apply transformation algorithm
   - Calculate direction: +1 (concentration) or -1 (dilution)
   - Apply algorithm-specific transformation
   - Increment step counter
   - Store result for next iteration

3. **Target Reached**: Loop back to base pattern
   - Reset `progressiveStepCount[key] = 1`
   - Store base pattern
   - Creates continuous cycling behavior

### Transformation Algorithms

#### Euclidean Transformation (`E>`)
- Uses Bjorklund algorithm for even distribution
- Maintains rhythmic regularity during transformation
- Good for creating smooth transitions

#### Barlow Transformation (`B>`)
- Uses indispensability theory
- Higher indispensability positions preferred for onsets
- Creates musically intelligent onset placement

#### Wolrab Transformation (`W>`)
- Anti-Barlow: inverts indispensability logic
- Lower indispensability positions preferred
- Creates groove-oriented, anti-metrical patterns

#### Dilcue Transformation (`D>`)
- Anti-Euclidean: inverts distribution logic
- Creates more irregular, syncopated patterns
- Good for adding rhythmic complexity

## UI Integration

### Step Counter Display

The step counter shows the current position in the transformation sequence:
- Step 1: Base pattern
- Step 2: First transformation
- Step N: (N-1)th transformation
- Cycles back to Step 1 when target reached

### Button Behavior

The trigger button behavior depends on pattern type:
- **Progressive Transformations**: Shows step number (1-7)
- **Scene Cycling**: Shows scene number (1-3)
- **Regular Patterns**: Shows current step in pattern cycle

### MIDI Integration

Progressive transformations can be triggered via:
- Plugin button clicks
- MIDI note input
- MIDI CC input
- Host automation

## Algorithm Implementation

### Pattern State Tracking

```cpp
if (progressivePatterns.find(patternKey) == progressivePatterns.end())
{
    // First time - return base pattern
    progressivePatterns[patternKey] = basePattern;
    progressiveStepCount[patternKey] = 1;
    return basePattern;
}
else
{
    // Continue from stored state
    currentPattern = progressivePatterns[patternKey];
}
```

### Direction Calculation

```cpp
// Direction: +1 for concentration, -1 for dilution
int direction = (targetOnsets > currentOnsets) ? 1 : -1;
int nextOnsets = currentOnsets + direction;

// Clamp to prevent overshooting
if (direction > 0)
    nextOnsets = std::min(nextOnsets, targetOnsets);
else
    nextOnsets = std::max(nextOnsets, targetOnsets);
```

### Target Reached Detection

```cpp
if (currentOnsets == targetOnsets)
{
    // Loop back to base pattern
    progressivePatterns[patternKey] = basePattern;
    progressiveStepCount[patternKey] = 1;
    return basePattern;
}
```

## User Experience

### Manual Control Benefits

1. **Precise Control**: Users decide when patterns advance
2. **Performance Ready**: Suitable for live performance
3. **Composition Tool**: Build complex sequences step-by-step
4. **Predictable**: No surprise pattern changes

### Continuous Cycling

When the target is reached, the pattern loops back to the base pattern:
- `E(1,8)E>8`: 1→2→3→4→5→6→7→8→1→2→...
- `E(8,8)B>1`: 8→7→6→5→4→3→2→1→8→7→...

This creates musically useful cycling behavior for live performance.

## Technical Considerations

### Memory Management

The LRU cleanup system prevents memory leaks:
- Maximum 100 patterns stored simultaneously
- Cleanup triggered on each transformation call
- Frequently used patterns kept longer

### Thread Safety

The implementation is designed for single-threaded audio processing:
- All state modifications happen on the audio thread
- No mutex needed for current implementation
- UI updates happen via parameter changes

### Performance

The system is optimized for real-time audio processing:
- Fast key lookups using `std::map`
- Minimal memory allocation during transformation
- Efficient binary pattern representation

## Future Enhancements

### Potential Improvements

1. **Custom Step Sizes**: Allow step sizes other than 1
2. **Bidirectional Cycling**: Option to reverse direction at limits
3. **Pattern Interpolation**: Smooth transitions between patterns
4. **Save/Load States**: Persist progressive states between sessions

### Compatibility

The system is designed to be compatible with:
- All accent pattern notations
- Scene cycling combinations
- Pattern combinations (+, -)
- All numeric pattern formats

## Debugging

### Debug Output

The system provides comprehensive debug logging:
- Pattern key generation
- State lookup results
- Direction calculation
- Algorithm selection
- Final pattern results

### Common Issues

1. **Off-by-One Errors**: Fixed by careful step counter management
2. **Memory Leaks**: Prevented by LRU cleanup system
3. **State Corruption**: Avoided by unique key generation
4. **UI Synchronization**: Ensured by consistent step counting

## Conclusion

The progressive transformation system provides a powerful tool for rhythmic exploration while maintaining predictable behavior for live performance. The manual control system gives users precise control over pattern evolution, essential for musical applications.