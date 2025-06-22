# Angle Calculation Analysis for Pattern "11100000"

## Summary

You identified a discrepancy in angle calculations between the center of gravity (CoG) calculation and the visualization. After thorough analysis, **there is no actual discrepancy** - the difference is purely cosmetic for display purposes.

## The Two Angle Calculations

### 1. Center of Gravity Calculation (pattern-analysis.js, line 103)
```javascript
const angle = (2 * Math.PI * position) / stepCount;
```

### 2. Visualization Calculation (ui-components.js, line 40)
```javascript
const angle = (position / stepCount) * 2 * Math.PI - Math.PI / 2;
```

## Key Difference

The visualization applies a **-π/2 radians (-90°) offset** to rotate the coordinate system:
- **Purpose**: Makes position 0 appear at the top (12 o'clock) instead of the right (3 o'clock)
- **Effect**: Purely cosmetic - rotates the entire display by 90° counterclockwise

## Verification for Pattern "11100000"

### Pattern Details
- Pattern: `11100000`
- Onsets at positions: 0, 1, 2 (out of 8 steps)

### Angle Calculations

| Position | CoG Angle (rad) | CoG Angle (deg) | Viz Angle (rad) | Viz Angle (deg) | Difference |
|----------|-----------------|-----------------|-----------------|-----------------|------------|
| 0        | 0.000000        | 0°              | -1.570796       | 270°            | -π/2       |
| 1        | 0.785398        | 45°             | -0.785398       | 315°            | -π/2       |
| 2        | 1.570796        | 90°             | 0.000000        | 0°              | -π/2       |

### Center of Gravity Results

Both methods produce the **same center of gravity magnitude**:

#### CoG Method
- Coordinates: (0.569036, 0.569036)
- Magnitude: 0.804738

#### Visualization Method  
- Coordinates: (0.569036, -0.569036)
- Magnitude: 0.804738

**Note**: The coordinates differ due to the rotation, but the magnitude (distance from origin) is identical.

## Mathematical Explanation

### Unit Vectors for Each Position

**CoG Method (standard angles):**
- Position 0: (cos(0°), sin(0°)) = (1, 0)
- Position 1: (cos(45°), sin(45°)) = (0.707107, 0.707107)
- Position 2: (cos(90°), sin(90°)) = (0, 1)

**Visualization Method (rotated -90°):**
- Position 0: (cos(270°), sin(270°)) = (0, -1)
- Position 1: (cos(315°), sin(315°)) = (0.707107, -0.707107)
- Position 2: (cos(0°), sin(0°)) = (1, 0)

### Center of Gravity Calculation

1. **Sum of unit vectors:**
   - CoG: (1.707107, 1.707107)
   - Viz: (1.707107, -1.707107)

2. **Average (÷ 3):**
   - CoG: (0.569036, 0.569036)
   - Viz: (0.569036, -0.569036)

3. **Magnitude:**
   - CoG: √(0.569036² + 0.569036²) = 0.804738
   - Viz: √(0.569036² + (-0.569036)²) = 0.804738

## Conclusion

### Your Analysis Was Correct
- ✅ Pattern "11100000" has onsets at positions 0, 1, 2
- ✅ CoG angles are 0°, 45°, 90° respectively
- ✅ Manual calculation gives magnitude 0.804738
- ✅ You correctly identified the -π/2 offset difference

### The "Discrepancy" Explained
- ✅ No actual discrepancy exists in the calculations
- ✅ The -π/2 offset is purely for visual presentation
- ✅ Both methods produce identical center of gravity magnitudes
- ✅ The rotation only affects the coordinate orientation, not the balance properties

### Key Insight
The visualization offset is a **display rotation** that makes the rhythm pattern appear with position 0 at the top of the circle (like a clock face), which is more intuitive for musical rhythm visualization. This rotation preserves all mathematical properties of the center of gravity calculation while improving the visual presentation.

## Files Used in Analysis
- `/Users/alex/Downloads/Rhythmic Pattern Explorer Integration Modules/pattern-analysis.js` - CoG calculation
- `/Users/alex/Downloads/Rhythmic Pattern Explorer Integration Modules/ui-components.js` - Visualization
- `/Users/alex/Downloads/Rhythmic Pattern Explorer Integration Modules/debug-angle-calculation.html` - Interactive analysis
- `/Users/alex/Downloads/Rhythmic Pattern Explorer Integration Modules/verify-cog-calculation.js` - Verification script