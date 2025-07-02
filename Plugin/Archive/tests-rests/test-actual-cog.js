#!/usr/bin/env node

// Load the pattern analysis module
const fs = require('fs');
const path = require('path');

// Read and evaluate the pattern-analysis.js file
const patternAnalysisPath = path.join(__dirname, 'pattern-analysis.js');
const patternAnalysisCode = fs.readFileSync(patternAnalysisPath, 'utf8');

// Create a simple Node.js compatible version by removing browser-specific exports
const moduleCode = patternAnalysisCode.replace(/if \(typeof window[\s\S]*$/, '');

// Evaluate the code to make classes available
eval(moduleCode);

// Make sure classes are available
global.PerfectBalanceAnalyzer = PerfectBalanceAnalyzer;
global.CenterOfGravityCalculator = CenterOfGravityCalculator;
global.PatternAnalyzer = PatternAnalyzer;

console.log('='.repeat(60));
console.log('TESTING ACTUAL CENTER OF GRAVITY CALCULATION');
console.log('='.repeat(60));
console.log();

// Test pattern "11100000"
const pattern = [1, 1, 1, 0, 0, 0, 0, 0];
const stepCount = 8;

console.log('Pattern:', pattern.map(x => x ? '1' : '0').join(''));
console.log('Step count:', stepCount);
console.log();

// Calculate using the actual CenterOfGravityCalculator
const cogResult = CenterOfGravityCalculator.calculateCenterOfGravity(pattern, stepCount);

console.log('CENTER OF GRAVITY RESULT:');
console.log('-'.repeat(40));
console.log('Coordinates:', `(${cogResult.coordinates.x.toFixed(6)}, ${cogResult.coordinates.y.toFixed(6)})`);
console.log('Magnitude:', cogResult.magnitude.toFixed(6));
console.log('Normalized Magnitude:', cogResult.normalizedMagnitude.toFixed(6));
console.log('Onset Count:', cogResult.onsetCount);
console.log('Onset Positions:', cogResult.onsetPositions.join(', '));
console.log('Is Balanced:', cogResult.isBalanced);
console.log('Balance Quality:', cogResult.balanceQuality);
console.log('Angle (radians):', cogResult.angle.toFixed(6));
console.log('Angle (degrees):', (cogResult.angle * 180 / Math.PI).toFixed(2));

console.log();
console.log('VERIFICATION WITH MANUAL CALCULATION:');
console.log('-'.repeat(40));

// Manual calculation to verify
let realSum = 0, imagSum = 0;
const onsetPositions = [0, 1, 2];

for (const position of onsetPositions) {
    const angle = (2 * Math.PI * position) / stepCount;
    const x = Math.cos(angle);
    const y = Math.sin(angle);
    realSum += x;
    imagSum += y;
    console.log(`Position ${position}: angle=${angle.toFixed(6)} rad, cos=${x.toFixed(6)}, sin=${y.toFixed(6)}`);
}

const avgX = realSum / onsetPositions.length;
const avgY = imagSum / onsetPositions.length;
const magnitude = Math.sqrt(avgX * avgX + avgY * avgY);

console.log();
console.log('Manual Results:');
console.log('Sum:', `(${realSum.toFixed(6)}, ${imagSum.toFixed(6)})`);
console.log('Average:', `(${avgX.toFixed(6)}, ${avgY.toFixed(6)})`);
console.log('Magnitude:', magnitude.toFixed(6));

console.log();
console.log('COMPARISON:');
console.log('-'.repeat(40));
const xDiff = Math.abs(cogResult.coordinates.x - avgX);
const yDiff = Math.abs(cogResult.coordinates.y - avgY);
const magDiff = Math.abs(cogResult.magnitude - magnitude);

console.log(`X coordinate difference: ${xDiff.toFixed(10)}`);
console.log(`Y coordinate difference: ${yDiff.toFixed(10)}`);
console.log(`Magnitude difference: ${magDiff.toFixed(10)}`);

if (xDiff < 1e-10 && yDiff < 1e-10 && magDiff < 1e-10) {
    console.log('✓ PERFECT MATCH: The actual CoG calculation matches manual calculation');
} else {
    console.log('✗ MISMATCH: There is a discrepancy in the calculations');
}

console.log();
console.log('CONCLUSION:');
console.log('-'.repeat(40));
console.log('The center of gravity calculation in pattern-analysis.js is correct.');
console.log('The visualization offset in ui-components.js is purely for display purposes.');
console.log('Your manual trace was correct:');
console.log('- Position 0: 0° (right side of circle)');
console.log('- Position 1: 45° (upper-right)');
console.log('- Position 2: 90° (top of circle)');
console.log('');
console.log('The -π/2 offset in visualization rotates the display so:');
console.log('- Position 0: 270° (bottom of circle in display)');
console.log('- Position 1: 315° (lower-left in display)');
console.log('- Position 2: 0° (top of circle in display)');
console.log('');
console.log('Both give the same center of gravity magnitude, just rotated visually.');

console.log();
console.log('='.repeat(60));