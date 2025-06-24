// === MODULE 2: PATTERN GENERATORS ===
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

class EuclideanGenerator {
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