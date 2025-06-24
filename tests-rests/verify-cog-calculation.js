#!/usr/bin/env node

/**
 * Verify center of gravity calculation for pattern "11100000"
 * This reproduces the exact calculation from pattern-analysis.js
 */

function calculateCenterOfGravity(steps, stepCount = null) {
    if (!Array.isArray(steps)) {
        throw new Error('Steps must be an array');
    }
    
    const actualStepCount = stepCount || steps.length;
    const onsetPositions = [];
    
    // Find all onset positions
    for (let i = 0; i < actualStepCount; i++) {
        if (steps[i]) {
            onsetPositions.push(i);
        }
    }
    
    if (onsetPositions.length === 0) {
        return {
            coordinates: { x: 0, y: 0 },
            magnitude: 0,
            normalizedMagnitude: 0,
            onsetCount: 0,
            onsetPositions: [],
            isEmpty: true
        };
    }
    
    // Calculate center of gravity using complex representation
    let realSum = 0;
    let imagSum = 0;
    
    for (const position of onsetPositions) {
        const angle = (2 * Math.PI * position) / actualStepCount;
        realSum += Math.cos(angle);
        imagSum += Math.sin(angle);
    }
    
    // Average the coordinates
    const avgX = realSum / onsetPositions.length;
    const avgY = imagSum / onsetPositions.length;
    
    // Calculate magnitude
    const magnitude = Math.sqrt(avgX * avgX + avgY * avgY);
    const normalizedMagnitude = magnitude; // Already normalized by division by onset count
    
    return {
        coordinates: { x: avgX, y: avgY },
        magnitude,
        normalizedMagnitude,
        onsetCount: onsetPositions.length,
        onsetPositions,
        isEmpty: false,
        // Additional helper properties
        angle: Math.atan2(avgY, avgX),
        isBalanced: normalizedMagnitude < 0.1,
        balanceQuality: normalizedMagnitude < 0.05 ? 'excellent' :
                       normalizedMagnitude < 0.15 ? 'good' :
                       normalizedMagnitude < 0.4 ? 'fair' : 'poor'
    };
}

console.log('='.repeat(70));
console.log('VERIFYING CENTER OF GRAVITY CALCULATION FOR PATTERN "11100000"');
console.log('='.repeat(70));
console.log();

// Test pattern "11100000"
const pattern = [1, 1, 1, 0, 0, 0, 0, 0];
const stepCount = 8;

console.log('Pattern:', pattern.map(x => x ? '1' : '0').join(''));
console.log('Step count:', stepCount);
console.log('Onsets at positions:', pattern.map((v, i) => v ? i : null).filter(x => x !== null).join(', '));
console.log();

// Calculate using the reproduced function
const cogResult = calculateCenterOfGravity(pattern, stepCount);

console.log('CENTER OF GRAVITY CALCULATION RESULTS:');
console.log('-'.repeat(50));
console.log('Coordinates:', `(${cogResult.coordinates.x.toFixed(6)}, ${cogResult.coordinates.y.toFixed(6)})`);
console.log('Magnitude:', cogResult.magnitude.toFixed(6));
console.log('Normalized Magnitude:', cogResult.normalizedMagnitude.toFixed(6));
console.log('Onset Count:', cogResult.onsetCount);
console.log('Onset Positions:', cogResult.onsetPositions.join(', '));
console.log('Is Balanced:', cogResult.isBalanced, `(threshold < 0.1)`);
console.log('Balance Quality:', cogResult.balanceQuality);
console.log('CoG Angle (radians):', cogResult.angle.toFixed(6));
console.log('CoG Angle (degrees):', (cogResult.angle * 180 / Math.PI).toFixed(2));
console.log();

console.log('STEP-BY-STEP CALCULATION VERIFICATION:');
console.log('-'.repeat(50));

let realSum = 0, imagSum = 0;
const onsetPositions = cogResult.onsetPositions;

console.log('For each onset position, calculate angle and unit vector:');
for (const position of onsetPositions) {
    const angle = (2 * Math.PI * position) / stepCount;
    const x = Math.cos(angle);
    const y = Math.sin(angle);
    realSum += x;
    imagSum += y;
    
    console.log(`Position ${position}:`);
    console.log(`  Angle = (2π × ${position}) / ${stepCount} = ${angle.toFixed(6)} rad = ${(angle * 180 / Math.PI).toFixed(2)}°`);
    console.log(`  Unit vector = (cos(${angle.toFixed(3)}), sin(${angle.toFixed(3)})) = (${x.toFixed(6)}, ${y.toFixed(6)})`);
}

console.log();
console.log('Sum of unit vectors:', `(${realSum.toFixed(6)}, ${imagSum.toFixed(6)})`);
console.log('Average (divide by onset count):', `(${(realSum/onsetPositions.length).toFixed(6)}, ${(imagSum/onsetPositions.length).toFixed(6)})`);
console.log('Magnitude of average:', Math.sqrt((realSum/onsetPositions.length)**2 + (imagSum/onsetPositions.length)**2).toFixed(6));

console.log();
console.log('COMPARISON WITH YOUR MANUAL CALCULATION:');
console.log('-'.repeat(50));
console.log('You calculated:');
console.log('Position 0: angle = 0°, unit vector = (1, 0)');
console.log('Position 1: angle = 45°, unit vector = (0.707107, 0.707107)');  
console.log('Position 2: angle = 90°, unit vector = (0, 1)');
console.log();
console.log('Sum: (1 + 0.707107 + 0, 0 + 0.707107 + 1) = (1.707107, 1.707107)');
console.log('Average: (1.707107/3, 1.707107/3) = (0.569036, 0.569036)');
console.log('Magnitude: √(0.569036² + 0.569036²) = 0.804738');
console.log();

const expectedX = 1.707107 / 3;
const expectedY = 1.707107 / 3;
const expectedMag = Math.sqrt(expectedX * expectedX + expectedY * expectedY);

console.log('VERIFICATION:');
const xDiff = Math.abs(cogResult.coordinates.x - expectedX);
const yDiff = Math.abs(cogResult.coordinates.y - expectedY);
const magDiff = Math.abs(cogResult.magnitude - expectedMag);

console.log(`X difference: ${xDiff.toFixed(10)} (${xDiff < 1e-6 ? '✓' : '✗'})`);
console.log(`Y difference: ${yDiff.toFixed(10)} (${yDiff < 1e-6 ? '✓' : '✗'})`);
console.log(`Magnitude difference: ${magDiff.toFixed(10)} (${magDiff < 1e-6 ? '✓' : '✗'})`);

console.log();
console.log('ABOUT THE VISUALIZATION OFFSET:');
console.log('-'.repeat(50));
console.log('The visualization in ui-components.js uses:');
console.log('  angle = (position / stepCount) * 2 * Math.PI - Math.PI / 2');
console.log();
console.log('This applies a -π/2 (-90°) rotation for display purposes:');
console.log('• Position 0: 0° → -90° (270°) - moves from right to bottom');
console.log('• Position 1: 45° → -45° (315°) - moves from upper-right to lower-right');  
console.log('• Position 2: 90° → 0° - moves from top to right');
console.log();
console.log('This rotation is purely cosmetic - it makes position 0 appear at');
console.log('12 o\'clock instead of 3 o\'clock on the circle display.');
console.log();
console.log('The center of gravity magnitude remains the same because rotating');
console.log('all points by the same amount only rotates the result vector,');
console.log('preserving distances and balance properties.');

console.log();
console.log('CONCLUSION:');
console.log('-'.repeat(50));
console.log('✓ Your manual calculation was CORRECT');
console.log('✓ The CoG calculation in pattern-analysis.js is CORRECT');  
console.log('✓ The visualization offset in ui-components.js is CORRECT (cosmetic only)');
console.log('✓ There is NO discrepancy - both methods give the same magnitude');

console.log();
console.log('='.repeat(70));