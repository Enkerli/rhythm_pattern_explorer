// Test script for CenterOfGravityCalculator with pattern "11100000"
// Load the pattern analysis module
const fs = require('fs');
const path = require('path');

// Read and execute the pattern-analysis.js file to get the classes
const patternAnalysisCode = fs.readFileSync(path.join(__dirname, 'pattern-analysis.js'), 'utf8');
eval(patternAnalysisCode);

// Test pattern: "11100000" - onsets at positions 0, 1, 2 out of 8 steps
const testPattern = [true, true, true, false, false, false, false, false];
const stepCount = 8;

console.log('Testing CenterOfGravityCalculator with pattern "11100000"');
console.log('Pattern:', testPattern.map(s => s ? '1' : '0').join(''));
console.log('Onsets at positions: 0, 1, 2');
console.log('');

// Calculate center of gravity
const result = CenterOfGravityCalculator.calculateCenterOfGravity(testPattern, stepCount);

console.log('Center of Gravity Results:');
console.log('========================');
console.log('Coordinates:', result.coordinates);
console.log('Magnitude:', result.magnitude);
console.log('Normalized Magnitude:', result.normalizedMagnitude);
console.log('Onset Count:', result.onsetCount);
console.log('Onset Positions:', result.onsetPositions);
console.log('Angle (radians):', result.angle);
console.log('Angle (degrees):', result.angle * 180 / Math.PI);
console.log('Is Balanced:', result.isBalanced);
console.log('Balance Quality:', result.balanceQuality);
console.log('');

// Let's also manually calculate what we expect
console.log('Manual Calculation Verification:');
console.log('===============================');

// For positions 0, 1, 2 in an 8-step pattern:
// Position 0: angle = 0, cos(0) = 1, sin(0) = 0
// Position 1: angle = π/4, cos(π/4) = √2/2, sin(π/4) = √2/2  
// Position 2: angle = π/2, cos(π/2) = 0, sin(π/2) = 1

const angles = [0, Math.PI/4, Math.PI/2];
let manualRealSum = 0;
let manualImagSum = 0;

for (let i = 0; i < 3; i++) {
    const angle = angles[i];
    const cosVal = Math.cos(angle);
    const sinVal = Math.sin(angle);
    manualRealSum += cosVal;
    manualImagSum += sinVal;
    console.log(`Position ${i}: angle=${angle.toFixed(4)}, cos=${cosVal.toFixed(4)}, sin=${sinVal.toFixed(4)}`);
}

const manualAvgX = manualRealSum / 3;
const manualAvgY = manualImagSum / 3;

console.log('');
console.log('Manual sums: real =', manualRealSum.toFixed(4), ', imag =', manualImagSum.toFixed(4));
console.log('Manual averages: x =', manualAvgX.toFixed(4), ', y =', manualAvgY.toFixed(4));
console.log('');

// The center of gravity should be around (0.5, 0.6) approximately
// This points towards the early part of the pattern, not position 3
console.log('Expected behavior: Center of gravity should point towards early part of pattern');
console.log('Expected approximate coordinates: x ≈ 0.5, y ≈ 0.6');
console.log('Actual coordinates: x =', result.coordinates.x.toFixed(4), ', y =', result.coordinates.y.toFixed(4));

// Convert to polar coordinates to see which direction it points
const radius = Math.sqrt(result.coordinates.x * result.coordinates.x + result.coordinates.y * result.coordinates.y);
const angleRad = Math.atan2(result.coordinates.y, result.coordinates.x);
const angleDeg = angleRad * 180 / Math.PI;

console.log('');
console.log('Polar representation:');
console.log('Radius:', radius.toFixed(4));
console.log('Angle:', angleRad.toFixed(4), 'radians =', angleDeg.toFixed(2), 'degrees');

// In a circular representation, 0° is position 0, 45° is position 1, 90° is position 2, etc.
// The center of gravity angle should be between 0° and 90° for this cluster
console.log('');
console.log('Analysis:');
if (angleDeg >= 0 && angleDeg <= 90) {
    console.log('✓ CORRECT: Center of gravity angle is between 0° and 90°, pointing towards the cluster');
} else {
    console.log('✗ ERROR: Center of gravity angle is outside expected range (0° to 90°)');
}

// The center should NOT point towards position 3 (which would be around 135°)
if (angleDeg > 120 && angleDeg < 150) {
    console.log('✗ ERROR: Center of gravity is pointing towards position 3 area (around 135°)');
} else {
    console.log('✓ CORRECT: Center of gravity is NOT pointing towards position 3');
}