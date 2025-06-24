// Test script to analyze the Center of Gravity for P(3,1)+P(5,0)+P(2,5)
// Load required modules

// First, let's recreate the necessary classes inline for testing
class MathUtils {
    static gcd(a, b) {
        a = Math.abs(a);
        b = Math.abs(b);
        while (b !== 0) {
            [a, b] = [b, a % b];
        }
        return a;
    }
    
    static lcm(a, b) {
        if (a === 0 || b === 0) return 0;
        return Math.abs(a * b) / this.gcd(a, b);
    }
    
    static lcmArray(numbers) {
        if (!Array.isArray(numbers) || numbers.length === 0) {
            throw new Error('Input must be a non-empty array of numbers');
        }
        const validNumbers = numbers.filter(n => n !== 0);
        if (validNumbers.length === 0) return 0;
        return validNumbers.reduce((lcm, num) => this.lcm(lcm, num), 1);
    }
}

class RegularPolygonGenerator {
    static generate(vertices, offset = 0, expansion = null) {
        if (vertices < 2 || vertices > 32) {
            throw new Error('Vertices must be between 2 and 32');
        }
        
        if (expansion === null) {
            expansion = 1;
        }
        
        if (expansion < 1 || expansion > 21) {
            throw new Error('Expansion factor must be between 1 and 21');
        }
        
        const totalSteps = vertices * expansion;
        if (totalSteps > 64) {
            throw new Error('Total steps cannot exceed 64');
        }
        
        const pattern = new Array(totalSteps).fill(false);
        
        for (let v = 0; v < vertices; v++) {
            const exactPosition = (v * totalSteps) / vertices;
            const position = Math.round(exactPosition) % totalSteps;
            pattern[position] = true;
        }
        
        if (offset !== 0) {
            const offsetPattern = new Array(totalSteps).fill(false);
            for (let i = 0; i < totalSteps; i++) {
                const sourceIndex = ((i - offset) % totalSteps + totalSteps) % totalSteps;
                offsetPattern[i] = pattern[sourceIndex];
            }
            return {
                steps: offsetPattern,
                stepCount: totalSteps,
                vertices,
                expansion,
                offset,
                polygonType: this.getPolygonName(vertices),
                isRegularPolygon: true,
                formula: expansion === 1 ? `P(${vertices},${offset})` : `P(${vertices},${offset},${expansion})`
            };
        }
        
        return {
            steps: pattern,
            stepCount: totalSteps,
            vertices,
            expansion,
            offset,
            polygonType: this.getPolygonName(vertices),
            isRegularPolygon: true,
            formula: expansion === 1 ? `P(${vertices},${offset})` : `P(${vertices},${offset},${expansion})`
        };
    }
    
    static getPolygonName(vertices) {
        const names = {
            2: 'Bipoint', 3: 'Triangle', 4: 'Square', 5: 'Pentagon', 6: 'Hexagon',
            7: 'Heptagon', 8: 'Octagon', 9: 'Nonagon', 10: 'Decagon'
        };
        return names[vertices] || `${vertices}-gon`;
    }
}

class AdvancedPatternCombiner {
    static combineMultiplePatterns(patterns) {
        if (patterns.length < 2) {
            throw new Error('Need at least 2 patterns to combine');
        }
        
        const stepCounts = patterns.map(p => p.stepCount);
        const lcm = MathUtils.lcmArray(stepCounts);
        
        const extendedPatterns = patterns.map((pattern, index) => {
            if (pattern.isRegularPolygon) {
                return this.extendPolygonToLCM(pattern, lcm);
            } else {
                return this.extendByRepetition(pattern.steps, pattern.stepCount, lcm);
            }
        });
        
        const combinedSteps = new Array(lcm).fill(false);
        for (let i = 0; i < lcm; i++) {
            for (const extendedPattern of extendedPatterns) {
                combinedSteps[i] = combinedSteps[i] || extendedPattern[i];
            }
        }
        
        return {
            steps: combinedSteps,
            stepCount: lcm,
            isCombined: true,
            originalPatterns: patterns.map(p => ({
                steps: p.steps,
                stepCount: p.stepCount,
                description: this.getPatternDescription(p)
            })),
            lcmUsed: lcm,
            combinationType: 'logical_or_multi'
        };
    }
    
    static extendPolygonToLCM(polygon, lcm) {
        const extended = new Array(lcm).fill(false);
        const vertices = polygon.vertices;
        const offset = polygon.offset || 0;
        
        for (let v = 0; v < vertices; v++) {
            const exactPosition = (v * lcm) / vertices;
            const position = Math.round(exactPosition) % lcm;
            const finalPosition = (position + offset) % lcm;
            extended[finalPosition] = true;
        }
        
        return extended;
    }
    
    static extendByRepetition(steps, originalLength, targetLength) {
        const extended = [];
        const repetitions = targetLength / originalLength;
        
        for (let rep = 0; rep < repetitions; rep++) {
            extended.push(...steps);
        }
        
        return extended;
    }
    
    static getPatternDescription(pattern) {
        if (pattern.isRegularPolygon) {
            return `${pattern.polygonType}(${pattern.vertices},${pattern.offset || 0})`;
        } else if (pattern.isEuclidean) {
            return `Euclidean ${pattern.formula}`;
        } else {
            return `Manual(${pattern.stepCount})`;
        }
    }
}

class CenterOfGravityCalculator {
    static calculateCenterOfGravity(steps, stepCount = null) {
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
}

// Test the pattern P(3,1)+P(5,0)+P(2,5)
console.log("=== Testing Pattern P(3,1)+P(5,0)+P(2,5) ===\n");

// Generate individual patterns
const p1 = RegularPolygonGenerator.generate(3, 1);  // P(3,1)
const p2 = RegularPolygonGenerator.generate(5, 0);  // P(5,0)
const p3 = RegularPolygonGenerator.generate(2, 5);  // P(2,5)

console.log("Individual patterns:");
console.log("P(3,1):", {
    steps: p1.steps,
    stepCount: p1.stepCount,
    binary: p1.steps.map(s => s ? '1' : '0').join(''),
    onsetPositions: p1.steps.map((s, i) => s ? i : null).filter(x => x !== null)
});

console.log("P(5,0):", {
    steps: p2.steps,
    stepCount: p2.stepCount,
    binary: p2.steps.map(s => s ? '1' : '0').join(''),
    onsetPositions: p2.steps.map((s, i) => s ? i : null).filter(x => x !== null)
});

console.log("P(2,5):", {
    steps: p3.steps,
    stepCount: p3.stepCount,
    binary: p3.steps.map(s => s ? '1' : '0').join(''),
    onsetPositions: p3.steps.map((s, i) => s ? i : null).filter(x => x !== null)
});

// Combine patterns
const combined = AdvancedPatternCombiner.combineMultiplePatterns([p1, p2, p3]);

console.log("\nCombined pattern:");
console.log("LCM used:", combined.lcmUsed);
console.log("Combined steps:", combined.steps);
console.log("Binary:", combined.steps.map(s => s ? '1' : '0').join(''));
console.log("Step count:", combined.stepCount);
console.log("Onset positions:", combined.steps.map((s, i) => s ? i : null).filter(x => x !== null));

// Calculate center of gravity
const cog = CenterOfGravityCalculator.calculateCenterOfGravity(combined.steps, combined.stepCount);

console.log("\nCenter of Gravity Analysis:");
console.log("Coordinates:", cog.coordinates);
console.log("X coordinate:", cog.coordinates.x);
console.log("Y coordinate:", cog.coordinates.y);
console.log("Magnitude:", cog.magnitude);
console.log("Normalized magnitude:", cog.normalizedMagnitude);
console.log("Onset count:", cog.onsetCount);
console.log("Onset positions:", cog.onsetPositions);
console.log("Is balanced?", cog.isBalanced);
console.log("Balance quality:", cog.balanceQuality);
console.log("Angle (radians):", cog.angle);
console.log("Angle (degrees):", (cog.angle * 180 / Math.PI).toFixed(2));

console.log("\n=== Detailed Analysis ===");

// Show the detailed calculation for each onset
console.log("Detailed calculation for each onset:");
for (let i = 0; i < cog.onsetPositions.length; i++) {
    const position = cog.onsetPositions[i];
    const angle = (2 * Math.PI * position) / combined.stepCount;
    const x = Math.cos(angle);
    const y = Math.sin(angle);
    console.log(`Onset ${i+1} at position ${position}: angle = ${(angle * 180 / Math.PI).toFixed(2)}°, x = ${x.toFixed(6)}, y = ${y.toFixed(6)}`);
}

// Show the sum calculation
let realSum = 0;
let imagSum = 0;
for (const position of cog.onsetPositions) {
    const angle = (2 * Math.PI * position) / combined.stepCount;
    realSum += Math.cos(angle);
    imagSum += Math.sin(angle);
}
console.log(`\nSum before averaging: realSum = ${realSum.toFixed(6)}, imagSum = ${imagSum.toFixed(6)}`);
console.log(`After averaging by onset count (${cog.onsetCount}): x = ${(realSum/cog.onsetCount).toFixed(6)}, y = ${(imagSum/cog.onsetCount).toFixed(6)}`);

// Check if this is close to (0,0)
const distanceFromOrigin = Math.sqrt(cog.coordinates.x * cog.coordinates.x + cog.coordinates.y * cog.coordinates.y);
console.log(`\nDistance from origin: ${distanceFromOrigin.toFixed(8)}`);
console.log(`Is practically at origin (< 0.001)? ${distanceFromOrigin < 0.001}`);
console.log(`Is well balanced (< 0.1)? ${distanceFromOrigin < 0.1}`);

// Show pattern visualization
console.log("\n=== Pattern Visualization ===");
const stepDisplay = [];
for (let i = 0; i < combined.stepCount; i++) {
    stepDisplay.push(combined.steps[i] ? 'X' : '.');
}
console.log("Pattern:", stepDisplay.join(''));
console.log("Positions:", Array.from({length: combined.stepCount}, (_, i) => i.toString().padStart(2, ' ')).join(''));