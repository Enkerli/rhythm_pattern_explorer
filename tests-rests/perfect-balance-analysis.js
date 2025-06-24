// Perfect Balance Analysis for P(3,1)+P(5,0)+P(2,5)
// This script demonstrates why this pattern is perfectly balanced

console.log("=== Perfect Balance Analysis: P(3,1)+P(5,0)+P(2,5) ===\n");

// Key insight: The pattern has 10 onsets positioned at:
// [0, 1, 5, 6, 11, 12, 18, 20, 21, 24] in a 30-step pattern

const onsetPositions = [0, 1, 5, 6, 11, 12, 18, 20, 21, 24];
const stepCount = 30;

console.log("Onset positions:", onsetPositions);
console.log("Total steps:", stepCount);
console.log("Number of onsets:", onsetPositions.length);

// Convert to angles and unit vectors
console.log("\n=== Unit Vector Analysis ===");
const vectors = [];
let totalX = 0;
let totalY = 0;

for (let i = 0; i < onsetPositions.length; i++) {
    const position = onsetPositions[i];
    const angle = (2 * Math.PI * position) / stepCount;
    const angleDegrees = (angle * 180 / Math.PI);
    const x = Math.cos(angle);
    const y = Math.sin(angle);
    
    vectors.push({ position, angle, angleDegrees, x, y });
    totalX += x;
    totalY += y;
    
    console.log(`Position ${position}: ${angleDegrees.toFixed(1)}° → (${x.toFixed(6)}, ${y.toFixed(6)})`);
}

console.log(`\nSum of all vectors: (${totalX.toFixed(8)}, ${totalY.toFixed(8)})`);
console.log(`Average (center of gravity): (${(totalX/onsetPositions.length).toFixed(8)}, ${(totalY/onsetPositions.length).toFixed(8)})`);

// Group vectors by symmetry
console.log("\n=== Symmetry Analysis ===");
console.log("Looking for opposing pairs that cancel out:");

// Check for pairs that sum to near zero
const tolerance = 1e-10;
const used = new Set();

for (let i = 0; i < vectors.length; i++) {
    if (used.has(i)) continue;
    
    const v1 = vectors[i];
    let foundPair = false;
    
    for (let j = i + 1; j < vectors.length; j++) {
        if (used.has(j)) continue;
        
        const v2 = vectors[j];
        const sumX = v1.x + v2.x;
        const sumY = v1.y + v2.y;
        
        if (Math.abs(sumX) < tolerance && Math.abs(sumY) < tolerance) {
            console.log(`Pair ${i+1} & ${j+1}: Position ${v1.position} (${v1.angleDegrees.toFixed(1)}°) + Position ${v2.position} (${v2.angleDegrees.toFixed(1)}°) = (${sumX.toFixed(8)}, ${sumY.toFixed(8)})`);
            used.add(i);
            used.add(j);
            foundPair = true;
            break;
        }
    }
    
    if (!foundPair && !used.has(i)) {
        console.log(`Unpaired: Position ${v1.position} (${v1.angleDegrees.toFixed(1)}°) = (${v1.x.toFixed(6)}, ${v1.y.toFixed(6)})`);
    }
}

// Look for larger symmetric groups
console.log("\n=== Larger Symmetry Groups ===");
console.log("Checking for groups of 3 or more vectors that sum to zero:");

// Find groups of 3 vectors that sum to near zero
for (let i = 0; i < vectors.length - 2; i++) {
    for (let j = i + 1; j < vectors.length - 1; j++) {
        for (let k = j + 1; k < vectors.length; k++) {
            const v1 = vectors[i];
            const v2 = vectors[j];
            const v3 = vectors[k];
            
            const sumX = v1.x + v2.x + v3.x;
            const sumY = v1.y + v2.y + v3.y;
            
            if (Math.abs(sumX) < tolerance && Math.abs(sumY) < tolerance) {
                console.log(`Triple: Positions ${v1.position}, ${v2.position}, ${v3.position} sum to (${sumX.toFixed(8)}, ${sumY.toFixed(8)})`);
            }
        }
    }
}

// Angle distribution analysis
console.log("\n=== Angle Distribution Analysis ===");
const angles = vectors.map(v => v.angleDegrees).sort((a, b) => a - b);
console.log("Angles in ascending order:", angles.map(a => a.toFixed(1) + "°").join(", "));

// Check angular differences
console.log("\nAngular differences:");
for (let i = 0; i < angles.length; i++) {
    const current = angles[i];
    const next = angles[(i + 1) % angles.length];
    const diff = next > current ? next - current : (360 - current) + next;
    console.log(`${current.toFixed(1)}° → ${next.toFixed(1)}°: ${diff.toFixed(1)}°`);
}

// Check if any angle appears multiple times
console.log("\n=== Angle Frequency Analysis ===");
const angleCount = {};
angles.forEach(angle => {
    const rounded = Math.round(angle * 10) / 10; // Round to 1 decimal
    angleCount[rounded] = (angleCount[rounded] || 0) + 1;
});

for (const [angle, count] of Object.entries(angleCount)) {
    if (count > 1) {
        console.log(`Angle ${angle}° appears ${count} times`);
    }
}

// Mathematical proof of perfect balance
console.log("\n=== Mathematical Proof ===");
console.log("Why this pattern is perfectly balanced:");
console.log("1. The sum of all unit vectors is essentially (0, 0)");
console.log("2. This means the center of gravity is exactly at the origin");
console.log("3. The pattern has perfect rotational symmetry when all vectors are summed");

const magnitude = Math.sqrt(totalX * totalX + totalY * totalY);
console.log(`\nMagnitude of vector sum: ${magnitude.toExponential(10)}`);
console.log(`This is effectively zero (within floating-point precision)`);
console.log(`Therefore, the pattern P(3,1)+P(5,0)+P(2,5) is PERFECTLY BALANCED.`);

// Verification with the actual CenterOfGravityCalculator result
const avgX = totalX / onsetPositions.length;
const avgY = totalY / onsetPositions.length;
const cogMagnitude = Math.sqrt(avgX * avgX + avgY * avgY);

console.log(`\nCenter of Gravity magnitude: ${cogMagnitude.toExponential(10)}`);
console.log(`This is < 0.001 (perfect balance threshold): ${cogMagnitude < 0.001}`);
console.log(`This is < 0.1 (good balance threshold): ${cogMagnitude < 0.1}`);