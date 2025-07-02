// === MODULE 2: PATTERN GENERATORS ===
/**
 * Regular Polygon Generator - Creates geometric rhythm patterns
 * 
 * Generates rhythm patterns based on regular polygon vertex positions mapped to a circle.
 * This creates mathematically precise rhythmic patterns with geometric foundations.
 * 
 * Musical Applications:
 * - Triangle (3 vertices): Creates ternary rhythms, often found in waltzes
 * - Square (4 vertices): Standard 4/4 patterns, rock and pop foundations  
 * - Pentagon (5 vertices): Complex polyrhythmic patterns, jazz applications
 * - Higher polygons: Experimental rhythms for contemporary music
 * 
 * Mathematical Foundation:
 * Vertices are positioned at angles 2πk/n around a unit circle, then mapped
 * to discrete time steps. The expansion factor allows for pattern lengthening
 * while maintaining the geometric relationships.
 */
class RegularPolygonGenerator {
    /**
     * Generate a regular polygon rhythm pattern
     * 
     * @param {number} vertices - Number of polygon vertices (2-32)
     * @param {number} offset - Rotation offset in steps (default: 0)
     * @param {number|null} expansion - Expansion factor (default: 1, range: 1-21)
     * @returns {Object} Generated polygon pattern with metadata
     * @throws {Error} If vertices not in range 2-32
     * @throws {Error} If expansion not in range 1-21
     * @throws {Error} If total steps exceed 64
     * 
     * @example
     * // Basic triangle pattern
     * const triangle = RegularPolygonGenerator.generate(3);
     * // Returns: P(3,0) with 3 evenly spaced onsets
     * 
     * // Offset pentagon for syncopation
     * const pentagon = RegularPolygonGenerator.generate(5, 1);
     * // Returns: P(5,1) with pentagon vertices offset by 1 step
     * 
     * // Expanded square for longer patterns
     * const expandedSquare = RegularPolygonGenerator.generate(4, 0, 3);
     * // Returns: P(4,0,3) with 12 total steps (4 × 3)
     * 
     * Algorithm:
     * 1. Calculate vertex positions: position = round(vertex × totalSteps / vertices)
     * 2. Apply expansion factor: totalSteps = vertices × expansion
     * 3. Apply offset rotation if specified
     * 4. Return pattern with comprehensive metadata
     * 
     * Musical Theory:
     * Regular polygons create maximally even distributions similar to Euclidean
     * rhythms but with exact geometric positioning. This often results in
     * patterns that feel both mathematical and musically natural.
     */
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
    
    /**
     * Get the proper geometric name for a polygon with given vertices
     * 
     * @param {number} vertices - Number of vertices in the polygon
     * @returns {string} Standard geometric name or fallback format
     * 
     * @example
     * RegularPolygonGenerator.getPolygonName(3)  // "Triangle"
     * RegularPolygonGenerator.getPolygonName(5)  // "Pentagon" 
     * RegularPolygonGenerator.getPolygonName(12) // "12-gon"
     * 
     * Standard Names:
     * - 2: Bipoint (degenerate case)
     * - 3: Triangle (most common in music)
     * - 4: Square (standard 4/4 time)
     * - 5: Pentagon (complex polyrhythms)
     * - 6: Hexagon (compound meters)
     * - 7: Heptagon (experimental rhythms)
     * - 8: Octagon (extended patterns)
     * - 9: Nonagon (complex time signatures)
     * - 10: Decagon (decimal rhythms)
     * - 11+: Generic "{n}-gon" format
     */
    static getPolygonName(vertices) {
        const names = {
            2: 'Bipoint', 3: 'Triangle', 4: 'Square', 5: 'Pentagon', 6: 'Hexagon',
            7: 'Heptagon', 8: 'Octagon', 9: 'Nonagon', 10: 'Decagon'
        };
        return names[vertices] || `${vertices}-gon`;
    }
}

/**
 * Euclidean Generator - Creates maximally even rhythm distributions
 * 
 * Implements Bjorklund's algorithm to generate Euclidean rhythms, which are
 * mathematically optimal distributions of beats within a given time cycle.
 * These patterns appear frequently in traditional music worldwide.
 * 
 * Theoretical Foundation:
 * Based on the Euclidean algorithm for finding the greatest common divisor,
 * this approach creates the most evenly distributed rhythm possible for any
 * combination of beats and steps. The algorithm recursively divides time
 * intervals to achieve maximal evenness.
 * 
 * Cultural Significance:
 * - E(3,8): Cuban tresillo, West African bell patterns
 * - E(5,8): Cuban cinquillo, Brazilian rhythms
 * - E(2,5): Korean traditional rhythms  
 * - E(3,7): Tuareg rhythms from Libya
 * - E(4,9): Turkish aksak rhythms
 * - E(5,12): Afro-Cuban and West African patterns
 * 
 * Mathematical Properties:
 * - Creates patterns with minimal "clustering" of beats
 * - Achieves optimal temporal balance
 * - Generates fractal-like recursive structures
 * - Preserves rotational and translational symmetries
 */
class EuclideanGenerator {
    /**
     * Generate a Euclidean rhythm pattern using Bjorklund's algorithm
     * 
     * @param {number} beats - Number of beats (onsets) to distribute
     * @param {number} steps - Total number of time steps
     * @param {number} offset - Rotation offset (default: 0)
     * @returns {Array<boolean>} Boolean array representing the Euclidean rhythm
     * 
     * @example
     * // Cuban tresillo
     * const tresillo = EuclideanGenerator.generate(3, 8);
     * // Returns: [true, false, false, true, false, false, true, false]
     * 
     * // Brazilian cinquillo
     * const cinquillo = EuclideanGenerator.generate(5, 8);
     * // Returns: [true, false, true, true, false, true, true, false]
     * 
     * // Standard 4/4 beat with offset
     * const offsetBeat = EuclideanGenerator.generate(4, 4, 1);
     * // Returns 4/4 pattern rotated by 1 step
     * 
     * Algorithm (Bjorklund):
     * 1. Initialize with (steps - beats) zeros and beats ones
     * 2. Apply Euclidean algorithm recursively:
     *    - Group patterns into pairs
     *    - Handle remainders at each level
     *    - Build final pattern through recursive construction
     * 3. Normalize to start with first beat
     * 4. Apply rotation offset if specified
     * 
     * Time Complexity: O(log(min(beats, steps)))
     * Space Complexity: O(steps)
     * 
     * Musical Applications:
     * - World music rhythm generation
     * - Polyrhythmic composition
     * - Metric modulation sequences
     * - Contemporary music experimentation
     */
    static generate(beats, steps, offset = 0) {
        if (beats > steps) beats = steps;
        if (beats <= 0) return new Array(steps).fill(false);
        
        let pattern = [];
        let counts = [];
        let remainders = [];
        
        let divisor = steps - beats;
        remainders[0] = beats;
        let level = 0;
        
        do {
            counts[level] = Math.floor(divisor / remainders[level]);
            remainders[level + 1] = divisor % remainders[level];
            divisor = remainders[level];
            level++;
        } while (remainders[level] > 1);
        
        counts[level] = divisor;
        
        function build(level) {
            if (level == -1) {
                pattern.push(false);
            } else if (level == -2) {
                pattern.push(true);
            } else {
                for (let i = 0; i < counts[level]; i++) {
                    build(level - 1);
                }
                if (remainders[level] != 0) {
                    build(level - 2);
                }
            }
        }
        
        build(level);
        
        while (pattern.length < steps) {
            pattern.push(false);
        }
        
        const firstBeatIndex = pattern.findIndex(beat => beat);
        if (firstBeatIndex > 0) {
            pattern = pattern.slice(firstBeatIndex).concat(pattern.slice(0, firstBeatIndex));
        }
        
        if (offset !== 0) {
            offset = ((offset % steps) + steps) % steps;
            // Apply offset by rotating RIGHT (same direction as @ rotation operator)
            const offsetPattern = new Array(steps);
            for (let i = 0; i < steps; i++) {
                offsetPattern[i] = pattern[(i - offset + steps) % steps];
            }
            pattern = offsetPattern;
        }
        
        return pattern;
    }
    
    /**
     * Generate Euclidean complement pattern
     * E(3,8) complement is E(5,8) - fills the remaining positions with Euclidean distribution
     * @param {number} beats - Original number of beats
     * @param {number} steps - Total number of steps  
     * @param {number} offset - Rotation offset (default: 0)
     * @returns {Array} Complement pattern as boolean array
     */
    static generateComplement(beats, steps, offset = 0) {
        const complementBeats = steps - beats;
        return this.generate(complementBeats, steps, offset);
    }
}

// Export to global scope for browser compatibility
if (typeof window !== 'undefined') {
    window.RegularPolygonGenerator = RegularPolygonGenerator;
    window.EuclideanGenerator = EuclideanGenerator;
}