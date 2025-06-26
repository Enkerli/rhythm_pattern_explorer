// === MODULE 3: PATTERN ANALYSIS ===
class PerfectBalanceAnalyzer {
    static calculateBalance(steps, stepCount) {
        const onsetPositions = [];
        for (let i = 0; i < stepCount; i++) {
            if (steps[i]) {
                onsetPositions.push(i);
            }
        }
        
        if (onsetPositions.length === 0) {
            return {
                magnitude: 0,
                normalizedMagnitude: 0,
                isPerfectlyBalanced: false,
                balanceScore: 'empty',
                complexSum: { real: 0, imag: 0 },
                onsetCount: 0,
                onsetPositions: [],
                geometricBalance: "No onsets",
                cogPosition: { x: 0, y: 0 }
            };
        }
        
        let realSum = 0;
        let imagSum = 0;
        
        for (const position of onsetPositions) {
            const angle = (2 * Math.PI * position) / stepCount;
            realSum += Math.cos(angle);
            imagSum += Math.sin(angle);
        }
        
        const magnitude = Math.sqrt(realSum * realSum + imagSum * imagSum);
        const normalizedMagnitude = magnitude / onsetPositions.length;
        
        let balanceScore = 'poor';
        let isPerfectlyBalanced = false;
        
        if (magnitude < 0.001) {
            balanceScore = 'perfect';
            isPerfectlyBalanced = true;
        } else if (normalizedMagnitude < 0.05) {
            balanceScore = 'excellent';
        } else if (normalizedMagnitude < 0.15) {
            balanceScore = 'good';
        } else if (normalizedMagnitude < 0.4) {
            balanceScore = 'fair';
        }
        
        const isGeometricallyBalanced = normalizedMagnitude < 0.1;
        const geometricBalance = normalizedMagnitude < 0.1 ? "Geometrically Centered" :
                                normalizedMagnitude < 0.3 ? "Well Distributed" :
                                normalizedMagnitude < 0.7 ? "Moderately Spread" : "Highly Clustered";
        
        return {
            magnitude,
            normalizedMagnitude,
            isPerfectlyBalanced,
            balanceScore,
            complexSum: { real: realSum, imag: imagSum },
            onsetCount: onsetPositions.length,
            onsetPositions,
            isGeometricallyBalanced,
            geometricBalance,
            cogPosition: { x: realSum / onsetPositions.length, y: imagSum / onsetPositions.length }
        };
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
            // Use same angle convention as visualization (position 0 at top)
            const angle = (position / actualStepCount) * 2 * Math.PI - Math.PI / 2;
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
    
    static calculateMultipleCoG(patterns) {
        if (!Array.isArray(patterns)) {
            throw new Error('Patterns must be an array');
        }
        
        return patterns.map((pattern, index) => {
            const steps = pattern.steps || pattern;
            const stepCount = pattern.stepCount || steps.length;
            
            return {
                index,
                pattern: pattern,
                centerOfGravity: this.calculateCenterOfGravity(steps, stepCount)
            };
        });
    }
}

class PatternAnalyzer {
    static detectRepetition(steps, stepCount) {
        if (stepCount < 2) return null;
        
        for (let unitLength = 1; unitLength <= stepCount / 2; unitLength++) {
            if (stepCount % unitLength === 0) {
                const repetitions = stepCount / unitLength;
                const unit = steps.slice(0, unitLength);
                
                let isRepeating = true;
                for (let rep = 1; rep < repetitions; rep++) {
                    const start = rep * unitLength;
                    for (let i = 0; i < unitLength; i++) {
                        if (steps[start + i] !== unit[i]) {
                            isRepeating = false;
                            break;
                        }
                    }
                    if (!isRepeating) break;
                }
                
                if (isRepeating) {
                    const unitBinary = unit.map(s => s ? '1' : '0').join('');
                    const unitDecimal = parseInt(unitBinary, 2);
                    const unitHex = '0x' + unitDecimal.toString(16).toUpperCase();
                    
                    return {
                        unitLength,
                        repetitions,
                        unit,
                        unitBinary,
                        unitDecimal,
                        unitHex,
                        compressionRatio: stepCount / unitLength
                    };
                }
            }
        }
        
        return null;
    }
    
    static detectEuclideanPattern(steps, stepCount) {
        // Test various Euclidean rhythm parameters to see if this pattern matches
        const onsetCount = steps.filter(s => s).length;
        
        if (onsetCount === 0) {
            return null; // All silence isn't Euclidean
        }
        
        // Test different offsets for the given beats and steps
        for (let offset = 0; offset < stepCount; offset++) {
            try {
                const euclideanSteps = EuclideanGenerator.generate(onsetCount, stepCount, offset);
                
                // Check if generated pattern matches our pattern
                if (euclideanSteps.length === steps.length) {
                    let matches = true;
                    for (let i = 0; i < steps.length; i++) {
                        if (steps[i] !== euclideanSteps[i]) {
                            matches = false;
                            break;
                        }
                    }
                    
                    if (matches) {
                        return {
                            beats: onsetCount,
                            steps: stepCount,
                            offset: offset,
                            formula: offset === 0 ? `E(${onsetCount},${stepCount})` : `E(${onsetCount},${stepCount},${offset})`
                        };
                    }
                }
            } catch (error) {
                // Skip invalid Euclidean parameters
                continue;
            }
        }
        
        return null; // Not a Euclidean pattern
    }
    
    static analyzeStructure(steps, stepCount) {
        const analysis = {
            stepCount,
            onsetCount: steps.filter(s => s).length,
            density: 0,
            maxSilenceGap: 0,
            avgOnsetInterval: 0
        };
        
        analysis.density = (analysis.onsetCount / stepCount) * 100;
        
        if (analysis.onsetCount > 0) {
            const onsetPositions = [];
            for (let i = 0; i < stepCount; i++) {
                if (steps[i]) onsetPositions.push(i);
            }
            
            let maxGap = 0;
            for (let i = 0; i < onsetPositions.length; i++) {
                const nextPos = onsetPositions[(i + 1) % onsetPositions.length];
                const gap = nextPos > onsetPositions[i] ? 
                            nextPos - onsetPositions[i] - 1 :
                            (stepCount - onsetPositions[i] - 1) + nextPos;
                maxGap = Math.max(maxGap, gap);
            }
            
            analysis.maxSilenceGap = maxGap;
            analysis.avgOnsetInterval = stepCount / analysis.onsetCount;
        }
        
        return analysis;
    }
}

class LongShortAnalyzer {
    /**
     * Analyze inter-onset intervals to determine long-short patterns (Morse-like)
     * @param {Array} steps - Boolean array representing the pattern
     * @param {number} stepCount - Total number of steps
     * @returns {Object} Long-short analysis with prosody descriptors
     */
    static analyzeLongShort(steps, stepCount) {
        const onsetPositions = [];
        for (let i = 0; i < stepCount; i++) {
            if (steps[i]) {
                onsetPositions.push(i);
            }
        }
        
        if (onsetPositions.length < 2) {
            return {
                intervals: [],
                longShortPattern: '',
                prosodyType: 'none',
                description: onsetPositions.length === 0 ? 'No onsets' : 'Single onset',
                intervalCounts: {},
                shortInterval: null,
                longInterval: null,
                intervalTypes: []
            };
        }
        
        // Calculate inter-onset intervals (IOIs)
        const intervals = [];
        for (let i = 0; i < onsetPositions.length; i++) {
            const currentPos = onsetPositions[i];
            const nextPos = onsetPositions[(i + 1) % onsetPositions.length];
            
            const interval = nextPos > currentPos ? 
                nextPos - currentPos : 
                (stepCount - currentPos) + nextPos;
            
            intervals.push(interval);
        }
        
        // Get unique interval values and their counts
        const intervalCounts = {};
        intervals.forEach(interval => {
            intervalCounts[interval] = (intervalCounts[interval] || 0) + 1;
        });
        
        const uniqueIntervals = Object.keys(intervalCounts).map(Number).sort((a, b) => a - b);
        
        // Determine long vs short classification
        let shortInterval, longInterval;
        let intervalTypes = [];
        
        if (uniqueIntervals.length === 1) {
            // All intervals the same
            shortInterval = longInterval = uniqueIntervals[0];
            intervalTypes = new Array(intervals.length).fill('equal');
        } else if (uniqueIntervals.length === 2) {
            // Classic long-short pattern
            shortInterval = uniqueIntervals[0];
            longInterval = uniqueIntervals[1];
            intervalTypes = intervals.map(interval => interval === shortInterval ? 'short' : 'long');
        } else {
            // More than 2 interval sizes - shortest vs others
            shortInterval = uniqueIntervals[0];
            longInterval = uniqueIntervals[uniqueIntervals.length - 1]; // Use longest as representative
            intervalTypes = intervals.map(interval => interval === shortInterval ? 'short' : 'long');
        }
        
        // Create long-short pattern string
        const longShortPattern = intervalTypes.map(type => {
            if (type === 'short') return 'S';
            if (type === 'long') return 'L';
            return 'E'; // Equal
        }).join('');
        
        // Create Morse-like notation (dots and dashes)
        const morseNotation = intervalTypes.map(type => {
            if (type === 'short') return '.';
            if (type === 'long') return '-';
            return '='; // Equal intervals
        }).join('');
        
        // Determine prosody type
        const prosodyType = this.determineProsodyType(longShortPattern);
        
        // Generate description
        const description = this.generateDescription(intervals, intervalTypes, prosodyType, shortInterval, longInterval);
        
        // Try to detect Morse characters
        const morseCharacter = this.detectMorseCharacter(morseNotation);
        
        return {
            intervals,
            longShortPattern,
            morseNotation,
            morseCharacter,
            prosodyType,
            description,
            intervalCounts,
            shortInterval,
            longInterval,
            intervalTypes,
            uniqueIntervals
        };
    }
    
    /**
     * Detect Morse code character from morse notation
     * @param {string} morseNotation - Morse notation (dots and dashes)
     * @returns {string|null} Morse character or null if not found
     */
    static detectMorseCharacter(morseNotation) {
        if (!morseNotation || morseNotation === 'N/A' || morseNotation === '=') return null;
        
        // Morse code lookup table (reverse lookup)
        const morseToChar = {
            '.-': 'A', '-...': 'B', '-.-.': 'C', '-..': 'D', '.': 'E', '..-.': 'F',
            '--.': 'G', '....': 'H', '..': 'I', '.---': 'J', '-.-': 'K', '.-..': 'L',
            '--': 'M', '-.': 'N', '---': 'O', '.--.': 'P', '--.-': 'Q', '.-.': 'R',
            '...': 'S', '-': 'T', '..-': 'U', '...-': 'V', '.--': 'W', '-..-': 'X',
            '-.--': 'Y', '--..': 'Z', '-----': '0', '.----': '1', '..---': '2',
            '...--': '3', '....-': '4', '.....': '5', '-....': '6', '--...': '7',
            '---..': '8', '----.': '9'
        };
        
        // For multi-character patterns, try to decode as multiple letters
        if (morseNotation.includes(' ')) {
            const letters = morseNotation.split(' ');
            const chars = letters.map(letter => morseToChar[letter]).filter(char => char);
            return chars.length > 0 ? chars.join('') : null;
        }
        
        return morseToChar[morseNotation] || null;
    }
    
    /**
     * Determine prosody type based on long-short pattern
     * @param {string} pattern - Pattern string (e.g., "LSL", "SLS")
     * @returns {string} Prosody type
     */
    static determineProsodyType(pattern) {
        if (!pattern || pattern.length < 2) return 'none';
        
        // Remove equal intervals for prosody analysis
        const cleanPattern = pattern.replace(/E/g, '');
        
        if (cleanPattern.length < 2) return 'uniform';
        
        // Check for common prosodic patterns
        switch (cleanPattern) {
            case 'LS':
            case 'LSLS':
            case 'LSLSLS':
                return 'trochaic'; // Strong-weak (long-short)
            
            case 'SL':
            case 'SLSL':
            case 'SLSLSL':
                return 'iambic'; // Weak-strong (short-long)
            
            case 'LSL':
            case 'LSLSL':
                return 'dactylic'; // Strong-weak-weak (long-short-long)
            
            case 'SLS':
            case 'SLSLS':
                return 'amphibrachic'; // Weak-strong-weak (short-long-short)
            
            case 'SSL':
            case 'SSLSSL':
                return 'anapestic'; // Weak-weak-strong (short-short-long)
            
            case 'LLS':
            case 'LLSLLS':
                return 'anti-dactylic'; // Strong-strong-weak (long-long-short)
                
            case 'LL':
            case 'LLLL':
            case 'LLLLLL':
                return 'spondaic'; // Strong-strong (long-long)
                
            case 'SS':
            case 'SSSS':
            case 'SSSSSS':
                return 'pyrrhic'; // Weak-weak (short-short)
                
            case 'SLL':
            case 'SLLSLL':
                return 'amphibrachic'; // Weak-strong-strong (short-long-long) - variant
                
            case 'LSS':
            case 'LSSLSS':
                return 'bacchic'; // Strong-weak-weak (long-short-short)
                
            case 'SSL':
            case 'SSLSSL':
                return 'anapestic'; // Weak-weak-strong (short-short-long)
                
            case 'SLS':
            case 'SLSSLS':
                return 'amphibrachic'; // Weak-strong-weak (short-long-short)
                
            case 'LLL':
            case 'LLLLLL':
                return 'molossic'; // Strong-strong-strong (long-long-long)
                
            case 'SSS':
            case 'SSSSSS':
                return 'tribrach'; // Weak-weak-weak (short-short-short)
                
            case 'SSLL':
            case 'SSLLSSLL':
                return 'ionic_minor'; // Weak-weak-strong-strong (short-short-long-long)
                
            case 'LLSS':
            case 'LLSSLLSS':
                return 'ionic_major'; // Strong-strong-weak-weak (long-long-short-short)
                
            // Paeon patterns (4-syllable feet with 1 long, 3 short)
            case 'LSSS':
            case 'LSSSLSSS':
                return 'primus_paeon'; // Strong-weak-weak-weak (long-short-short-short)
                
            case 'SLSS':
            case 'SLSSSLSS':
                return 'secundus_paeon'; // Weak-strong-weak-weak (short-long-short-short)
                
            case 'SSLS':
            case 'SSLSSSLS':
                return 'tertius_paeon'; // Weak-weak-strong-weak (short-short-long-short)
                
            case 'SSSL':
            case 'SSSLOSSSL':
                return 'quartus_paeon'; // Weak-weak-weak-strong (short-short-short-long)
                
            // Dimeter patterns
            case 'LSLS':
            case 'LSLSLSLS':
                return 'ditrochee'; // Strong-weak-strong-weak (long-short-long-short)
                
            case 'SLSL':
            case 'SLSLSLSL':
                return 'diiamb'; // Weak-strong-weak-strong (short-long-short-long)
                
            case 'LSSL':
            case 'LSSLLSSL':
                return 'choriamb'; // Strong-weak-weak-strong (long-short-short-long)
                
            case 'SLLS':
            case 'SLLSSLLS':
                return 'antispast'; // Weak-strong-strong-weak (short-long-long-short)
                
            // Epitrite patterns (4-syllable feet with 3 long, 1 short)
            case 'SLLL':
            case 'SLLLSLLL':
                return 'first_epitrite'; // Weak-strong-strong-strong (short-long-long-long)
                
            case 'LSLL':
            case 'LSLLLSLL':
                return 'second_epitrite'; // Strong-weak-strong-strong (long-short-long-long)
                
            case 'LLSL':
            case 'LLSLLLSL':
                return 'third_epitrite'; // Strong-strong-weak-strong (long-long-short-long)
                
            case 'LLLS':
            case 'LLLSLLLS':
                return 'fourth_epitrite'; // Strong-strong-strong-weak (long-long-long-short)
        }
        
        // Check for predominantly long or short
        const longCount = (cleanPattern.match(/L/g) || []).length;
        const shortCount = (cleanPattern.match(/S/g) || []).length;
        
        if (longCount > shortCount * 1.5) return 'predominantly_long';
        if (shortCount > longCount * 1.5) return 'predominantly_short';
        
        // Check for alternating patterns
        if (/^(LS)+$/.test(cleanPattern)) return 'trochaic';
        if (/^(SL)+$/.test(cleanPattern)) return 'iambic';
        
        return 'complex';
    }
    
    /**
     * Generate human-readable description
     * @param {Array} intervals - Array of interval values
     * @param {Array} types - Array of interval types
     * @param {string} prosodyType - Prosody classification
     * @param {number} shortInterval - Short interval value
     * @param {number} longInterval - Long interval value
     * @returns {string} Description
     */
    static generateDescription(intervals, types, prosodyType, shortInterval, longInterval) {
        if (intervals.length === 0) return 'No rhythmic intervals';
        if (intervals.length === 1) return 'Single interval';
        
        const prosodyDescriptions = {
            'trochaic': 'Trochaic (strong-weak, long-short)',
            'iambic': 'Iambic (weak-strong, short-long)', 
            'dactylic': 'Dactylic (strong-weak-weak, long-short-long)',
            'amphibrachic': 'Amphibrachic (weak-strong-weak, short-long-short)',
            'anapestic': 'Anapestic (weak-weak-strong, short-short-long)',
            'anti-dactylic': 'Anti-dactylic (strong-strong-weak, long-long-short)',
            'spondaic': 'Spondaic (strong-strong, long-long)',
            'pyrrhic': 'Pyrrhic (weak-weak, short-short)',
            'bacchic': 'Bacchic (strong-weak-weak, long-short-short)',
            'molossic': 'Molossic (strong-strong-strong, long-long-long)',
            'tribrach': 'Tribrach (weak-weak-weak, short-short-short)',
            'ionic_minor': 'Ionic Minor (weak-weak-strong-strong, short-short-long-long)',
            'ionic_major': 'Ionic Major (strong-strong-weak-weak, long-long-short-short)',
            'primus_paeon': 'Primus Paeon (strong-weak-weak-weak, long-short-short-short)',
            'secundus_paeon': 'Secundus Paeon (weak-strong-weak-weak, short-long-short-short)',
            'tertius_paeon': 'Tertius Paeon (weak-weak-strong-weak, short-short-long-short)',
            'quartus_paeon': 'Quartus Paeon (weak-weak-weak-strong, short-short-short-long)',
            'ditrochee': 'Ditrochee (strong-weak-strong-weak, long-short-long-short)',
            'diiamb': 'Diiamb (weak-strong-weak-strong, short-long-short-long)',
            'choriamb': 'Choriamb (strong-weak-weak-strong, long-short-short-long)',
            'antispast': 'Antispast (weak-strong-strong-weak, short-long-long-short)',
            'first_epitrite': 'First Epitrite (weak-strong-strong-strong, short-long-long-long)',
            'second_epitrite': 'Second Epitrite (strong-weak-strong-strong, long-short-long-long)',
            'third_epitrite': 'Third Epitrite (strong-strong-weak-strong, long-long-short-long)',
            'fourth_epitrite': 'Fourth Epitrite (strong-strong-strong-weak, long-long-long-short)',
            'predominantly_long': 'Predominantly long intervals',
            'predominantly_short': 'Predominantly short intervals',
            'uniform': 'Uniform intervals',
            'complex': 'Complex rhythmic pattern',
            'none': 'No prosodic pattern'
        };
        
        let description = prosodyDescriptions[prosodyType] || 'Unknown pattern';
        
        if (shortInterval !== longInterval) {
            description += ` (${shortInterval}:${longInterval})`;
        } else {
            description += ` (${shortInterval})`;
        }
        
        return description;
    }
}

/**
 * Syncopation Analysis based on multiple musicological measures
 * Implements 5 complementary approaches to measuring rhythmic syncopation
 */
class SyncopationAnalyzer {
    /**
     * Comprehensive syncopation analysis using multiple measures
     * @param {Array} steps - Boolean array representing the pattern
     * @param {number} stepCount - Total number of steps in the pattern
     * @returns {Object} Syncopation analysis with multiple measures
     */
    static analyzeSyncopation(steps, stepCount) {
        const onsetPositions = [];
        for (let i = 0; i < stepCount; i++) {
            if (steps[i]) {
                onsetPositions.push(i);
            }
        }
        
        if (onsetPositions.length === 0) {
            return {
                weightedNoteToBeats: 0,
                offBeatRatio: 0,
                expectancyViolation: 0,
                rhythmicDisplacement: 0,
                crossRhythmic: 0,
                overallSyncopation: 0,
                description: 'No syncopation (no onsets)',
                level: 'none'
            };
        }
        
        // Calculate each syncopation measure
        const weightedNoteToBeats = this.calculateWeightedNoteToBeats(onsetPositions, stepCount);
        const offBeatRatio = this.calculateOffBeatRatio(onsetPositions, stepCount);
        const expectancyViolation = this.calculateExpectancyViolation(onsetPositions, stepCount);
        const rhythmicDisplacement = this.calculateRhythmicDisplacement(onsetPositions, stepCount);
        const crossRhythmic = this.calculateCrossRhythmic(steps, stepCount);
        
        // Calculate overall syncopation (weighted average)
        const overallSyncopation = (
            weightedNoteToBeats * 0.25 +
            offBeatRatio * 0.2 +
            expectancyViolation * 0.25 +
            rhythmicDisplacement * 0.15 +
            crossRhythmic * 0.15
        );
        
        const level = this.getSyncopationLevel(overallSyncopation);
        const description = this.getSyncopationDescription(overallSyncopation, level);
        
        return {
            weightedNoteToBeats: Math.round(weightedNoteToBeats * 1000) / 1000,
            offBeatRatio: Math.round(offBeatRatio * 1000) / 1000,
            expectancyViolation: Math.round(expectancyViolation * 1000) / 1000,
            rhythmicDisplacement: Math.round(rhythmicDisplacement * 1000) / 1000,
            crossRhythmic: Math.round(crossRhythmic * 1000) / 1000,
            overallSyncopation: Math.round(overallSyncopation * 1000) / 1000,
            description,
            level
        };
    }
    
    /**
     * Weighted Note-to-Beat Distance (Longuet-Higgins & Lee)
     * Measures distance from onsets to strong metric positions
     */
    static calculateWeightedNoteToBeats(onsetPositions, stepCount) {
        let totalWeight = 0;
        let totalDistance = 0;
        
        for (const position of onsetPositions) {
            const metricWeight = this.getMetricWeight(position, stepCount);
            const distanceToStrongBeat = this.getDistanceToStrongBeat(position, stepCount);
            
            totalWeight += metricWeight;
            totalDistance += distanceToStrongBeat * (1 - metricWeight); // Invert weight for syncopation
        }
        
        return totalWeight > 0 ? totalDistance / onsetPositions.length : 0;
    }
    
    /**
     * Off-Beat Onset Ratio
     * Percentage of onsets on weak beats vs strong beats
     */
    static calculateOffBeatRatio(onsetPositions, stepCount) {
        let offBeatCount = 0;
        
        for (const position of onsetPositions) {
            const metricWeight = this.getMetricWeight(position, stepCount);
            if (metricWeight < 0.5) { // Weak beat threshold
                offBeatCount++;
            }
        }
        
        return onsetPositions.length > 0 ? offBeatCount / onsetPositions.length : 0;
    }
    
    /**
     * Expectancy Violation (Huron)
     * Based on cognitive expectancy of strong vs weak positions
     */
    static calculateExpectancyViolation(onsetPositions, stepCount) {
        let totalViolation = 0;
        
        for (const position of onsetPositions) {
            const expectedProbability = this.getExpectedOnsetProbability(position, stepCount);
            const violation = 1 - expectedProbability; // Higher for unexpected positions
            totalViolation += violation;
        }
        
        return onsetPositions.length > 0 ? totalViolation / onsetPositions.length : 0;
    }
    
    /**
     * Rhythmic Displacement Index
     * How far onsets are displaced from expected positions
     */
    static calculateRhythmicDisplacement(onsetPositions, stepCount) {
        let totalDisplacement = 0;
        
        for (const position of onsetPositions) {
            const nearestStrongBeat = this.getNearestStrongBeat(position, stepCount);
            const displacement = Math.abs(position - nearestStrongBeat);
            const normalizedDisplacement = displacement / (stepCount / 4); // Normalize by quarter note
            totalDisplacement += Math.min(normalizedDisplacement, 1); // Cap at 1
        }
        
        return onsetPositions.length > 0 ? totalDisplacement / onsetPositions.length : 0;
    }
    
    /**
     * Cross-Rhythmic Syncopation (Temperley)
     * Measures conflict between surface rhythm and implied meter
     */
    static calculateCrossRhythmic(steps, stepCount) {
        // Use auto-correlation to find competing periodicities
        const periodicities = [2, 3, 4, 6, 8]; // Common metric divisions
        let maxConflict = 0;
        
        for (const period of periodicities) {
            if (period >= stepCount) continue;
            
            const correlation = this.calculateAutoCorrelation(steps, period);
            const expectedCorrelation = this.getExpectedCorrelation(stepCount, period);
            const conflict = Math.abs(correlation - expectedCorrelation);
            maxConflict = Math.max(maxConflict, conflict);
        }
        
        return Math.min(maxConflict, 1); // Normalize to 0-1
    }
    
    /**
     * Get metric weight for a position (1 = strongest, 0 = weakest)
     */
    static getMetricWeight(position, stepCount) {
        // Create hierarchical metric structure
        const beatPosition = position % (stepCount / 4); // Assuming 4/4 meter
        
        if (position === 0) return 1.0; // Downbeat
        if (position % (stepCount / 2) === 0) return 0.8; // Half note
        if (position % (stepCount / 4) === 0) return 0.6; // Quarter note
        if (position % (stepCount / 8) === 0) return 0.4; // Eighth note
        if (position % (stepCount / 16) === 0) return 0.2; // Sixteenth note
        
        return 0.1; // Syncopated positions
    }
    
    /**
     * Get distance to nearest strong beat
     */
    static getDistanceToStrongBeat(position, stepCount) {
        const strongBeats = [];
        const quarterNote = stepCount / 4;
        
        for (let i = 0; i < stepCount; i += quarterNote) {
            strongBeats.push(i);
        }
        
        let minDistance = stepCount;
        for (const beat of strongBeats) {
            const distance = Math.min(
                Math.abs(position - beat),
                stepCount - Math.abs(position - beat) // Circular distance
            );
            minDistance = Math.min(minDistance, distance);
        }
        
        return minDistance / (stepCount / 8); // Normalize by eighth note
    }
    
    /**
     * Get expected onset probability for cognitive model
     */
    static getExpectedOnsetProbability(position, stepCount) {
        const metricWeight = this.getMetricWeight(position, stepCount);
        return metricWeight * 0.8 + 0.1; // Scale to 0.1-0.9 range
    }
    
    /**
     * Get nearest strong beat position
     */
    static getNearestStrongBeat(position, stepCount) {
        const quarterNote = stepCount / 4;
        const strongBeats = [];
        
        for (let i = 0; i < stepCount; i += quarterNote) {
            strongBeats.push(i);
        }
        
        let nearestBeat = strongBeats[0];
        let minDistance = stepCount;
        
        for (const beat of strongBeats) {
            const distance = Math.min(
                Math.abs(position - beat),
                stepCount - Math.abs(position - beat)
            );
            if (distance < minDistance) {
                minDistance = distance;
                nearestBeat = beat;
            }
        }
        
        return nearestBeat;
    }
    
    /**
     * Calculate auto-correlation for cross-rhythmic analysis
     */
    static calculateAutoCorrelation(steps, lag) {
        let correlation = 0;
        const n = steps.length;
        
        for (let i = 0; i < n; i++) {
            const j = (i + lag) % n;
            if (steps[i] && steps[j]) {
                correlation += 1;
            }
        }
        
        return correlation / n;
    }
    
    /**
     * Get expected correlation for metric structure
     */
    static getExpectedCorrelation(stepCount, period) {
        // Expected correlation based on typical metric hierarchy
        if (period === stepCount / 2) return 0.7; // Half note correlation
        if (period === stepCount / 4) return 0.5; // Quarter note correlation
        return 0.3; // Other periods
    }
    
    /**
     * Get syncopation level description
     */
    static getSyncopationLevel(syncopationScore) {
        if (syncopationScore < 0.1) return 'none';
        if (syncopationScore < 0.3) return 'low';
        if (syncopationScore < 0.5) return 'moderate';
        if (syncopationScore < 0.7) return 'high';
        return 'extreme';
    }
    
    /**
     * Get descriptive text for syncopation score
     */
    static getSyncopationDescription(score, level) {
        const descriptions = {
            'none': 'No syncopation - strong metric alignment',
            'low': 'Minimal syncopation - mostly on-beat',
            'moderate': 'Moderate syncopation - balanced off-beat elements',
            'high': 'High syncopation - significant rhythmic tension',
            'extreme': 'Extreme syncopation - heavily off-beat emphasis'
        };
        
        return descriptions[level] || 'Unknown syncopation level';
    }
}

// Export to global scope for browser compatibility
if (typeof window !== 'undefined') {
    window.PerfectBalanceAnalyzer = PerfectBalanceAnalyzer;
    window.CenterOfGravityCalculator = CenterOfGravityCalculator;
    window.PatternAnalyzer = PatternAnalyzer;
    window.LongShortAnalyzer = LongShortAnalyzer;
    window.SyncopationAnalyzer = SyncopationAnalyzer;
}