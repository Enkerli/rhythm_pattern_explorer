#!/usr/bin/env node

/**
 * Debug script to verify angle calculation differences between
 * center of gravity calculation and visualization
 */

// Pattern data for "11100000"
const pattern = [1, 1, 1, 0, 0, 0, 0, 0];
const stepCount = 8;
const onsetPositions = [0, 1, 2];

// Helper functions
function radToDeg(rad) {
    return rad * (180 / Math.PI);
}

function normalizeAngle(deg) {
    return ((deg % 360) + 360) % 360;
}

console.log('='.repeat(60));
console.log('DEBUGGING ANGLE CALCULATION FOR PATTERN "11100000"');
console.log('='.repeat(60));
console.log();

console.log('Pattern:', pattern.map(x => x ? '1' : '0').join(''));
console.log('Onsets at positions:', onsetPositions.join(', '));
console.log('Step count:', stepCount);
console.log();

// Calculate angles for each onset position
console.log('ANGLE CALCULATIONS:');
console.log('-'.repeat(80));
console.log('Pos | CoG Angle (rad) | CoG Angle (deg) | Viz Angle (rad) | Viz Angle (deg) | Difference');
console.log('-'.repeat(80));

const angleResults = [];
for (const position of onsetPositions) {
    // CoG calculation (from pattern-analysis.js line 103)
    const cogAngle = (2 * Math.PI * position) / stepCount;
    
    // Visualization calculation (from ui-components.js line 40)
    const vizAngle = (position / stepCount) * 2 * Math.PI - Math.PI / 2;
    
    const cogAngleDeg = normalizeAngle(radToDeg(cogAngle));
    const vizAngleDeg = normalizeAngle(radToDeg(vizAngle));
    const difference = vizAngle - cogAngle;
    
    console.log(`${position.toString().padStart(3)} | ${cogAngle.toFixed(6).padStart(15)} | ${cogAngleDeg.toFixed(2).padStart(15)} | ${vizAngle.toFixed(6).padStart(15)} | ${vizAngleDeg.toFixed(2).padStart(15)} | ${difference.toFixed(6)}`);
    
    angleResults.push({
        position,
        cogAngle,
        vizAngle,
        cogAngleDeg,
        vizAngleDeg,
        difference
    });
}

console.log();

// Calculate center of gravity using both methods
console.log('CENTER OF GRAVITY CALCULATIONS:');
console.log('-'.repeat(40));

let cogRealSum = 0, cogImagSum = 0;
let vizRealSum = 0, vizImagSum = 0;

console.log('CoG Method (pattern-analysis.js):');
for (const position of onsetPositions) {
    const cogAngle = (2 * Math.PI * position) / stepCount;
    const x = Math.cos(cogAngle);
    const y = Math.sin(cogAngle);
    cogRealSum += x;
    cogImagSum += y;
    console.log(`  Position ${position}: angle=${cogAngle.toFixed(6)} rad, cos=${x.toFixed(6)}, sin=${y.toFixed(6)}`);
}

console.log();
console.log('Visualization Method (ui-components.js):');
for (const position of onsetPositions) {
    const vizAngle = (position / stepCount) * 2 * Math.PI - Math.PI / 2;
    const x = Math.cos(vizAngle);
    const y = Math.sin(vizAngle);
    vizRealSum += x;
    vizImagSum += y;
    console.log(`  Position ${position}: angle=${vizAngle.toFixed(6)} rad, cos=${x.toFixed(6)}, sin=${y.toFixed(6)}`);
}

console.log();

// Calculate average coordinates
const cogAvgX = cogRealSum / onsetPositions.length;
const cogAvgY = cogImagSum / onsetPositions.length;
const vizAvgX = vizRealSum / onsetPositions.length;
const vizAvgY = vizImagSum / onsetPositions.length;

const cogMagnitude = Math.sqrt(cogAvgX * cogAvgX + cogAvgY * cogAvgY);
const vizMagnitude = Math.sqrt(vizAvgX * vizAvgX + vizAvgY * vizAvgY);

console.log('RESULTS:');
console.log('-'.repeat(40));
console.log(`CoG Method - Center: (${cogAvgX.toFixed(6)}, ${cogAvgY.toFixed(6)}), Magnitude: ${cogMagnitude.toFixed(6)}`);
console.log(`Viz Method - Center: (${vizAvgX.toFixed(6)}, ${vizAvgY.toFixed(6)}), Magnitude: ${vizMagnitude.toFixed(6)}`);
console.log(`Magnitude Difference: ${Math.abs(cogMagnitude - vizMagnitude).toFixed(6)}`);

console.log();
console.log('MANUAL VERIFICATION:');
console.log('-'.repeat(40));
console.log('Expected calculation for pattern "11100000":');
console.log('Position 0: angle = (2π × 0) / 8 = 0 rad = 0°');
console.log('Position 1: angle = (2π × 1) / 8 = π/4 rad = 45°');
console.log('Position 2: angle = (2π × 2) / 8 = π/2 rad = 90°');
console.log();
console.log('Expected coordinates:');
console.log('Position 0: (cos(0), sin(0)) = (1, 0)');
console.log('Position 1: (cos(π/4), sin(π/4)) = (0.707107, 0.707107)');
console.log('Position 2: (cos(π/2), sin(π/2)) = (0, 1)');
console.log();
console.log('Sum: (1 + 0.707107 + 0, 0 + 0.707107 + 1) = (1.707107, 1.707107)');
console.log('Average: (1.707107/3, 1.707107/3) = (0.569036, 0.569036)');
console.log('Magnitude: √(0.569036² + 0.569036²) = 0.804738');

console.log();
console.log('ANALYSIS:');
console.log('-'.repeat(40));
const isConsistent = Math.abs(cogMagnitude - vizMagnitude) < 0.001;
if (isConsistent) {
    console.log('✓ CONSISTENT: The visualization offset (-π/2) only affects display orientation.');
    console.log('  The center of gravity magnitude is the same in both calculations.');
    console.log('  This is mathematically correct - rotating all points by the same amount');
    console.log('  only rotates the result, preserving distances and magnitudes.');
} else {
    console.log('✗ INCONSISTENT: There is a discrepancy in the calculations!');
    console.log('  This suggests the angle offset is affecting more than just visualization.');
}

console.log();
console.log('KEY INSIGHT:');
console.log('The -π/2 offset in ui-components.js line 40 is for visualization purposes only.');
console.log('It rotates the display so position 0 appears at the top (12 o\'clock)');
console.log('instead of the right (3 o\'clock). This should not affect CoG calculations.');

console.log();
console.log('='.repeat(60));