// === MODULE 4: PATTERN PROCESSING ===
/**
 * Pattern Converter - Universal format conversion utilities
 * 
 * Provides comprehensive conversion between all supported rhythm pattern formats.
 * Handles binary, hexadecimal, octal, decimal, and onset array representations
 * with proper parsing, validation, and error handling.
 * 
 * Supported Formats:
 * - Binary: "10010010" or "b10010010" 
 * - Hexadecimal: "0x92" or "0x92:8" (with step count)
 * - Octal: "0o111" or "0o111:9" (with step count)
 * - Decimal: "146" or "146:8" (with step count)
 * - Onset Arrays: "[0,3,6]" or "[0,3,6]:8" (with step count)
 * 
 * Mathematical Considerations:
 * - Handles bit ordering for rhythmic interpretation
 * - Preserves pattern length information
 * - Maintains onset position accuracy
 * - Supports both standard and enhanced notation
 * 
 * Design Philosophy:
 * Rhythm patterns are fundamentally sequences of onsets and rests that can be
 * represented in multiple equivalent mathematical formats. This class provides
 * lossless conversion between formats while preserving musical meaning.
 */
class PatternConverter {
    /**
     * Convert boolean pattern array to binary string representation
     * 
     * @param {Array<boolean>} steps - Pattern as boolean array
     * @param {number} stepCount - Total number of steps
     * @returns {string} Binary string representation
     * 
     * @example
     * PatternConverter.toBinary([true, false, true, false], 4)
     * // Returns: "1010"
     * 
     * PatternConverter.toBinary([true, false, false, true, false, false, true, false], 8)
     * // Returns: "10010010" (Tresillo pattern)
     * 
     * Algorithm:
     * Maps each boolean value to '1' (onset) or '0' (rest) in left-to-right order,
     * preserving the temporal sequence of the rhythm pattern.
     */
    static toBinary(steps, stepCount) {
        return Array(stepCount).fill(0)
            .map((_, i) => steps[i] ? '1' : '0')
            .join('');
    }
    
    /**
     * Parse binary string to pattern object
     * 
     * @param {string} binaryString - Binary representation (e.g., "10010010")
     * @returns {Object} Pattern object with steps array and metadata
     * 
     * @example
     * PatternConverter.fromBinary("1010")
     * // Returns: {
     * //   steps: [true, false, true, false],
     * //   stepCount: 4,
     * //   isBinaryInput: true
     * // }
     * 
     * Musical Context:
     * Binary representation is the most direct format for rhythm patterns,
     * with each bit representing a discrete time step that either contains
     * an onset (1) or rest (0). This format is used internally throughout
     * the application for pattern manipulation and analysis.
     */
    static fromBinary(binaryString) {
        return {
            steps: binaryString.split('').map(bit => bit === '1'),
            stepCount: binaryString.length,
            isBinaryInput: true // Flag to indicate this came from binary input
        };
    }
    
    /**
     * Convert binary string to decimal representation
     * 
     * @param {string} binaryString - Binary pattern string
     * @param {boolean} shouldReverse - Whether to reverse for rhythmic interpretation (default: true)
     * @returns {number} Decimal representation of the pattern
     * 
     * @example
     * PatternConverter.toDecimal("10010010", true)  // Rhythmic interpretation
     * // Returns: 73 (reversed: 01001001 = 73)
     * 
     * PatternConverter.toDecimal("10010010", false) // Standard binary
     * // Returns: 146 (standard: 10010010 = 146)
     * 
     * Bit Ordering Philosophy:
     * - shouldReverse=true: Musical interpretation where leftmost position 
     *   (downbeat) gets lowest bit value for natural onset weighting
     * - shouldReverse=false: Standard binary interpretation for computational use
     * 
     * Musical Significance:
     * The reversed interpretation makes musical sense because it gives the
     * downbeat (first position) the least significant bit, making patterns
     * with strong downbeats have predictable low-order bit patterns.
     */
    static toDecimal(binaryString, shouldReverse = true) {
        if (shouldReverse) {
            // Reverse the binary string so leftmost position (first downbeat) gets bit value 1
            const reversedBinary = binaryString.split('').reverse().join('');
            return parseInt(reversedBinary, 2) || 0;
        } else {
            // Use standard binary interpretation (for binary input)
            return parseInt(binaryString, 2) || 0;
        }
    }
    
    static toHex(decimal) {
        return '0x' + decimal.toString(16).toUpperCase();
    }
    
    static toOctal(decimal) {
        return '0o' + decimal.toString(8);
    }
    
    static fromDecimalWithSteps(decimal, stepCount) {
        if (decimal === 0) return { steps: new Array(stepCount).fill(false), stepCount };
        
        // Convert using standard left-to-right bit ordering
        const standardBinary = decimal.toString(2);
        const paddedBinary = standardBinary.padStart(stepCount, '0');
        
        return {
            steps: paddedBinary.split('').map(bit => bit === '1'),
            stepCount: stepCount,
            hasExplicitSteps: true
        };
    }
    
    static fromDecimal(decimal, minSteps = 1) {
        // Handle colon notation: "decimal:steps"
        if (typeof decimal === 'string' && decimal.includes(':')) {
            const [decimalPart, stepsPart] = decimal.split(':');
            const numericDecimal = parseInt(decimalPart);
            const stepCount = parseInt(stepsPart);
            return this.fromDecimalWithSteps(numericDecimal, stepCount);
        }
        
        const numericDecimal = typeof decimal === 'string' ? parseInt(decimal) : decimal;
        if (numericDecimal === 0) return { steps: [false], stepCount: 1 };
        
        const stepCount = Math.max(minSteps, Math.floor(Math.log2(numericDecimal)) + 1);
        // Convert to binary using standard left-to-right bit ordering
        const standardBinary = numericDecimal.toString(2).padStart(stepCount, '0');
        const result = this.fromBinary(standardBinary);
        // Remove the isBinaryInput flag since this is decimal input, not binary input
        delete result.isBinaryInput;
        return result;
    }
    
    static fromHex(hexString) {
        // Handle colon notation: "0x49:8" or "49:8"
        if (hexString.includes(':')) {
            const [hexPart, stepsPart] = hexString.split(':');
            const stepCount = parseInt(stepsPart);
            const cleaned = hexPart.replace(/^0x/i, '').replace(/[^0-9A-F]/gi, '');
            if (!cleaned) return null;
            
            const decimal = parseInt(cleaned, 16);
            if (isNaN(decimal)) return null;
            
            return this.fromDecimalWithSteps(decimal, stepCount);
        }
        
        // Handle regular hex without step count
        const cleaned = hexString.replace(/^0x/i, '').replace(/[^0-9A-F]/gi, '');
        if (!cleaned) return null;
        
        const decimal = parseInt(cleaned, 16);
        if (isNaN(decimal)) return null;
        
        return this.fromDecimal(decimal);
    }
    
    static fromOctal(octalString) {
        // Handle colon notation: "0o111:9" or "111:9"
        if (octalString.includes(':')) {
            const [octalPart, stepsPart] = octalString.split(':');
            const stepCount = parseInt(stepsPart);
            const cleaned = octalPart.replace(/^0o/i, '').replace(/[^0-7]/g, '');
            if (!cleaned) return null;
            
            const decimal = parseInt(cleaned, 8);
            if (isNaN(decimal)) return null;
            
            return this.fromDecimalWithSteps(decimal, stepCount);
        }
        
        // Handle regular octal without step count
        const cleaned = octalString.replace(/^0o/i, '').replace(/[^0-7]/g, '');
        if (!cleaned) return null;
        
        const decimal = parseInt(cleaned, 8);
        if (isNaN(decimal)) return null;
        
        return this.fromDecimal(decimal);
    }
    
    static toEnhancedHex(decimal, stepCount) {
        const hex = this.toHex(decimal);
        return `${hex}:${stepCount}`;
    }
    
    static toEnhancedOctal(decimal, stepCount) {
        const octal = this.toOctal(decimal);
        return `${octal}:${stepCount}`;
    }
    
    static toEnhancedDecimal(decimal, stepCount) {
        return `${decimal}:${stepCount}`;
    }
    
    static toOnsetArray(steps, stepCount) {
        const onsets = [];
        for (let i = 0; i < stepCount; i++) {
            if (steps[i]) {
                onsets.push(i);
            }
        }
        return `[${onsets.join(',')}]`;
    }
    
    static toEnhancedOnsetArray(steps, stepCount) {
        const onsets = [];
        for (let i = 0; i < stepCount; i++) {
            if (steps[i]) {
                onsets.push(i);
            }
        }
        return `[${onsets.join(',')}]:${stepCount}`;
    }
}

/**
 * Advanced Pattern Combiner - Sophisticated pattern combination algorithms
 * 
 * Handles complex mathematical combination of multiple rhythm patterns using
 * Least Common Multiple (LCM) expansion and intelligent pattern extension.
 * Supports polygon-aware combination and geometric distribution methods.
 * 
 * Mathematical Foundation:
 * When combining patterns of different lengths, the LCM of all pattern lengths
 * determines the minimum cycle length needed to represent all patterns together.
 * Each pattern is then extended to this LCM length using appropriate algorithms.
 * 
 * Combination Strategies:
 * - Polygon Patterns: Extended using vertex geometry to LCM length
 * - Euclidean Patterns: Extended using geometric distribution
 * - General Patterns: Extended using proportional mapping
 * - Logical OR: Combined using union of all onset positions
 * 
 * Musical Applications:
 * - Polyrhythmic composition: Combine different meter patterns
 * - Complex time signatures: Create compound meters
 * - Cross-rhythmic patterns: Layer competing rhythmic cycles
 * - Mathematical music: Explore LCM-based rhythm relationships
 */
class AdvancedPatternCombiner {
    /**
     * Combine multiple rhythm patterns using LCM expansion and logical OR
     * 
     * @param {Array<Object>} patterns - Array of pattern objects to combine
     * @returns {Object} Combined pattern with comprehensive metadata
     * @throws {Error} If fewer than 2 patterns provided
     * 
     * @example
     * const triangle = RegularPolygonGenerator.generate(3, 1);
     * const pentagon = RegularPolygonGenerator.generate(5, 0);
     * const combined = AdvancedPatternCombiner.combineMultiplePatterns([triangle, pentagon]);
     * // Creates LCM(3,5) = 15-step pattern combining both geometries
     * 
     * Algorithm:
     * 1. Calculate LCM of all pattern step counts
     * 2. Extend each pattern to LCM length using appropriate method:
     *    - Polygons: Geometric vertex distribution
     *    - Others: Proportional position mapping
     * 3. Combine using logical OR (any pattern has onset = combined has onset)
     * 4. Preserve metadata from all original patterns
     * 
     * Mathematical Properties:
     * - Preserves the periodicity of all input patterns
     * - Maintains geometric relationships for polygon patterns
     * - Creates minimal cycle length for complete representation
     * - Results in mathematically precise polyrhythmic combinations
     * 
     * Musical Significance:
     * This creates true polyrhythmic patterns where multiple rhythm cycles
     * of different lengths interact in their natural mathematical relationship,
     * similar to how African polyrhythms layer different percussion patterns.
     */
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
                // Treat non-polygon patterns as geometric distributions
                return this.extendByGeometricDistribution(pattern.steps, pattern.stepCount, lcm);
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
                vertices: p.vertices,
                offset: p.offset,
                expansion: p.expansion,
                isRegularPolygon: p.isRegularPolygon,
                polygonType: p.polygonType,
                beats: p.beats,
                isEuclidean: p.isEuclidean,
                formula: p.formula,
                binary: p.binary || PatternConverter.toBinary(p.steps, p.stepCount),
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
    
    static extendByGeometricDistribution(steps, originalLength, targetLength) {
        // Distribute the onsets geometrically across the target length
        const extended = new Array(targetLength).fill(false);
        
        // Find all onset positions in the original pattern
        const onsetPositions = [];
        for (let i = 0; i < steps.length; i++) {
            if (steps[i]) {
                onsetPositions.push(i);
            }
        }
        
        // Distribute each onset geometrically across the target length
        onsetPositions.forEach(originalPos => {
            // Map original position to new position proportionally
            const exactPosition = (originalPos * targetLength) / originalLength;
            const newPosition = Math.round(exactPosition) % targetLength;
            extended[newPosition] = true;
        });
        
        return extended;
    }
    
    static extendByPadding(steps, originalLength, targetLength) {
        // Extend pattern by padding with zeros (silence), not repetition
        const extended = [...steps];
        while (extended.length < targetLength) {
            extended.push(false);
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

class UnifiedPatternParser {
    static parse(input) {
        const cleaned = input.trim();
        
        // Check for quantization notation first: pattern;newSteps or pattern;-newSteps
        if (QuantizationEngine.hasQuantizationNotation(cleaned)) {
            const quantizationInfo = QuantizationEngine.parseQuantizationNotation(cleaned);
            if (quantizationInfo) {
                // Parse the base pattern first
                const baseResult = this.parse(quantizationInfo.basePattern);
                let basePattern;
                
                if (baseResult.type === 'single') {
                    basePattern = baseResult.pattern;
                } else if (baseResult.type === 'stringed') {
                    basePattern = baseResult.pattern;
                } else if (baseResult.type === 'combination') {
                    basePattern = baseResult.combined;
                } else {
                    basePattern = baseResult;
                }
                
                // Apply quantization
                const quantizedPattern = QuantizationEngine.quantizePattern(
                    basePattern, 
                    quantizationInfo.newSteps, 
                    quantizationInfo.clockwise
                );
                
                return {
                    type: 'quantized',
                    pattern: quantizedPattern,
                    originalPattern: basePattern,
                    quantizationInfo: quantizationInfo
                };
            }
        }
        
        // Check for pattern naming: name=pattern
        if (cleaned.includes('=')) {
            const parts = cleaned.split('=');
            if (parts.length === 2) {
                const name = parts[0].trim();
                const patternInput = parts[1].trim();
                
                if (name.length > 0 && patternInput.length > 0) {
                    const result = this.parse(patternInput);
                    
                    // Add name to the result
                    if (result.type === 'single') {
                        result.pattern.name = name;
                        result.pattern.hasCustomName = true;
                    } else if (result.type === 'combination') {
                        result.combined.name = name;
                        result.combined.hasCustomName = true;
                    }
                    
                    return result;
                }
            }
        }
        
        // Check for comma-separated pattern stringing before plus combination
        // Only if there are commas that are likely pattern separators (not within parentheses)
        if (cleaned.includes(',') && !cleaned.includes('+')) {
            // Check if this looks like pattern stringing vs single pattern with commas
            const hasPatternSeparatorCommas = this.hasPatternSeparatorCommas(cleaned);
                if (hasPatternSeparatorCommas) {
                const stringedResult = this.parseStringedPatterns(cleaned);
                if (stringedResult) {
                    return {
                        type: 'stringed',
                        pattern: stringedResult,
                        isStringed: true
                    };
                }
            }
        }
        
        // Check for combination with addition and/or subtraction: P(3,0)+P(5,1)-P(2,0)
        if (cleaned.includes('+') || cleaned.includes('-')) {
            // Parse combinations with addition and subtraction
            const combinationResult = this.parseCombinationWithSubtraction(cleaned);
            if (combinationResult) {
                return combinationResult;
            }
        }
        
        return {
            type: 'single',
            pattern: this.parseSingle(cleaned)
        };
    }
    
    static parseSingle(input) {
        return this.parsePattern(input.trim());
    }
    
    /**
     * Parse pattern combinations with addition and subtraction
     * Handles patterns like: P(3,0)+P(5,1)-P(2,0)
     * @param {string} input - Pattern combination string
     * @returns {Object} Combined pattern with subtraction applied
     */
    static parseCombinationWithSubtraction(input) {
        // Split the input into tokens preserving operators
        const tokens = this.tokenizeCombination(input);
        if (tokens.length < 3) return null; // Need at least pattern + operator + pattern
        
        const addPatterns = [];
        const subtractPatterns = [];
        let currentOperator = '+'; // Start with addition (implicit for first pattern)
        
        // Process tokens
        for (let i = 0; i < tokens.length; i++) {
            const token = tokens[i];
            
            if (token === '+' || token === '-') {
                currentOperator = token;
            } else {
                // This is a pattern
                try {
                    const pattern = this.parseSingle(token);
                    if (currentOperator === '+') {
                        addPatterns.push(pattern);
                    } else if (currentOperator === '-') {
                        subtractPatterns.push(pattern);
                    }
                } catch (error) {
                    console.error(`Failed to parse pattern part: ${token}`, error);
                    return null;
                }
            }
        }
        
        if (addPatterns.length === 0) {
            throw new Error('Combination must have at least one pattern to add');
        }
        
        // Combine addition patterns first
        let combined;
        if (addPatterns.length > 1) {
            combined = AdvancedPatternCombiner.combineMultiplePatterns(addPatterns);
        } else {
            combined = addPatterns[0];
        }
        
        // Apply subtraction patterns
        if (subtractPatterns.length > 0) {
            combined = this.applySubtractionPatterns(combined, subtractPatterns);
        }
        
        return {
            type: 'combination',
            patterns: addPatterns,
            subtractPatterns: subtractPatterns,
            combined: combined,
            hasSubtraction: subtractPatterns.length > 0,
            multiPattern: true
        };
    }
    
    /**
     * Tokenize combination string into patterns and operators
     * @param {string} input - Input string like "P(3,0)+P(5,1)-P(2,0)"
     * @returns {Array<string>} Array of tokens
     */
    static tokenizeCombination(input) {
        const tokens = [];
        let current = '';
        let parenDepth = 0;
        
        for (let i = 0; i < input.length; i++) {
            const char = input[i];
            
            if (char === '(') {
                parenDepth++;
                current += char;
            } else if (char === ')') {
                parenDepth--;
                current += char;
            } else if ((char === '+' || char === '-') && parenDepth === 0) {
                // This is an operator at the top level
                if (current.trim()) {
                    tokens.push(current.trim());
                    current = '';
                }
                tokens.push(char);
            } else {
                current += char;
            }
        }
        
        // Add the last token
        if (current.trim()) {
            tokens.push(current.trim());
        }
        
        return tokens;
    }
    
    /**
     * Apply subtraction patterns to a combined pattern
     * @param {Object} basePattern - Pattern to subtract from
     * @param {Array<Object>} subtractPatterns - Patterns to subtract
     * @returns {Object} Pattern with subtraction applied
     */
    static applySubtractionPatterns(basePattern, subtractPatterns) {
        if (!basePattern || !basePattern.steps || subtractPatterns.length === 0) {
            return basePattern;
        }
        
        // Create a copy of the base pattern
        const result = {
            ...basePattern,
            steps: [...basePattern.steps]
        };
        
        // Apply each subtraction pattern
        for (const subtractPattern of subtractPatterns) {
            if (!subtractPattern || !subtractPattern.steps) continue;
            
            // If step counts don't match, extend the subtraction pattern to match using LCM
            let subtractSteps = subtractPattern.steps;
            if (subtractPattern.stepCount !== result.stepCount) {
                // Extend subtract pattern to match the base pattern length
                subtractSteps = this.extendPatternToLength(subtractPattern, result.stepCount);
            }
            
            // Remove onsets where both patterns have onsets
            for (let i = 0; i < Math.min(result.stepCount, subtractSteps.length); i++) {
                if (subtractSteps[i] && result.steps[i]) {
                    result.steps[i] = false;
                }
            }
        }
        
        // Mark as having subtraction applied
        result.hasSubtraction = true;
        result.subtractedPatterns = subtractPatterns;
        
        return result;
    }
    
    /**
     * Extend a pattern to a target length using geometric distribution
     * @param {Object} pattern - Pattern to extend
     * @param {number} targetLength - Target length
     * @returns {Array<boolean>} Extended steps array
     */
    static extendPatternToLength(pattern, targetLength) {
        if (pattern.stepCount === targetLength) {
            return [...pattern.steps];
        }
        
        // Use geometric distribution to extend the pattern
        return AdvancedPatternCombiner.extendByGeometricDistribution(
            pattern.steps, 
            pattern.stepCount, 
            targetLength
        );
    }
    
    static parsePattern(input) {
        const cleaned = input.trim();
        console.log(`🔍 parsePattern called with: "${cleaned}"`);
        
        // Check for transformation prefixes: ~, rev, inv
        console.log(`🔍 Checking transformation prefixes for: "${cleaned}"`);
        if (cleaned.startsWith('~') || cleaned.startsWith('inv ')) {
            // Inversion (bit flipping)
            const pattern = cleaned.startsWith('~') ? cleaned.substring(1).trim() : cleaned.substring(4).trim();
            const basePattern = this.parsePattern(pattern);
            return this.invertPattern(basePattern);
        }
        
        if (cleaned.startsWith('rev ')) {
            // Retrograde (reverse)
            const pattern = cleaned.substring(4).trim();
            const basePattern = this.parsePattern(pattern);
            return this.retrogradePattern(basePattern);
        }
        
        if (cleaned.startsWith('comp ')) {
            // Euclidean complement
            const pattern = cleaned.substring(5).trim();
            const basePattern = this.parsePattern(pattern);
            return this.complementPattern(basePattern);
        }
        
        // Check for space-separated pattern stringing: pattern1 pattern2 pattern3
        // Only at top level - check if we have multiple space-separated patterns
        if (this.hasSpaceSeparatedPatterns(cleaned)) {
            return this.parseSpaceStringedPatterns(cleaned);
        }
        
        // DEPRECATED: Check for comma-separated pattern stringing (for backward compatibility)
        // Only at top level - not for individual patterns that contain commas (like E(3,8))
        if (cleaned.includes(',') && this.hasPatternSeparatorCommas(cleaned)) {
            console.log('⚠️ Comma-separated stringing is deprecated. Use space separation: pattern1 pattern2 pattern3');
            return this.parseStringedPatterns(cleaned);
        }
        
        // Check for custom duration notation: D:1,3 pattern (short=1, long=3)
        const durationMatch = cleaned.match(/^D:([0-9.]+),([0-9.]+)\s+(.+)$/i);
        if (durationMatch) {
            const shortDuration = parseFloat(durationMatch[1]);
            const longDuration = parseFloat(durationMatch[2]);
            const basePattern = durationMatch[3];
            
            if (isNaN(shortDuration) || isNaN(longDuration) || shortDuration <= 0 || longDuration <= 0) return null;
            
            // Parse the base pattern first
            const baseParsed = this.parsePattern(basePattern);
            if (!baseParsed) return null;
            
            // Apply custom duration transformation
            const transformed = this.applyCustomDurations(baseParsed, shortDuration, longDuration);
            if (transformed) {
                transformed.isTransformed = true;
                transformed.transformationType = 'custom_durations';
                transformed.shortDuration = shortDuration;
                transformed.longDuration = longDuration;
                transformed.basePattern = basePattern;
                transformed.formula = `D:${shortDuration},${longDuration} ${basePattern}`;
            }
            return transformed;
        }
        
        // Check for stretch/squeeze transformations: S:2 pattern, Q:0.5 pattern
        const stretchMatch = cleaned.match(/^([SQ]):([0-9.]+)\s+(.+)$/i);
        if (stretchMatch) {
            const transformType = stretchMatch[1].toUpperCase();
            const factor = parseFloat(stretchMatch[2]);
            const basePattern = stretchMatch[3];
            
            if (isNaN(factor) || factor <= 0) return null;
            
            // Parse the base pattern first
            const baseParsed = this.parsePattern(basePattern);
            if (!baseParsed) return null;
            
            // Apply stretch or squeeze transformation
            const transformed = this.applyStretchTransformation(baseParsed, factor, transformType);
            if (transformed) {
                transformed.isTransformed = true;
                transformed.transformationType = transformType === 'S' ? 'stretch' : 'squeeze';
                transformed.transformationFactor = factor;
                transformed.basePattern = basePattern;
                transformed.formula = `${transformType}:${factor} ${basePattern}`;
            }
            return transformed;
        }
        
        // Check for rotation notation: pattern@steps (but not shorthand polygons)
        if (cleaned.includes('@')) {
            const parts = cleaned.split('@');
            if (parts.length === 2) {
                const pattern = parts[0].trim();
                const rotationSteps = parseInt(parts[1].trim());
                
                // Check if this is a shorthand polygon (let shorthand parser handle it)
                const shorthandMap = {
                    'bi': 2, 'bip': 2, 'bipod': 2,
                    'tri': 3, 'triangle': 3,
                    'quad': 4, 'square': 4, 'tetra': 4,
                    'pent': 5, 'pentagon': 5, 'penta': 5,
                    'hex': 6, 'hexagon': 6, 'hexa': 6,
                    'hept': 7, 'heptagon': 7, 'hepta': 7, 'sept': 7, 'septa': 7,
                    'oct': 8, 'octagon': 8, 'octa': 8,
                    'enn': 9, 'enneagon': 9, 'nona': 9,
                    'dec': 10, 'decagon': 10, 'deca': 10,
                    'dod': 12, 'dodecagon': 12, 'dodeca': 12
                };
                
                if (shorthandMap[pattern.toLowerCase()]) {
                    // This is a shorthand polygon, skip rotation parsing - let shorthand parser handle it
                } else if (!isNaN(rotationSteps)) {
                    // Parse the base pattern first
                    const basePattern = this.parsePattern(pattern);
                    // Apply rotation
                    return this.rotatePattern(basePattern, rotationSteps);
                }
            }
        }
        
        console.log(`🔍 Starting main pattern matching for: "${cleaned}"`);
        const polygonMatch2 = cleaned.match(/^P\((\d+),(\d+)\)$/i);
        if (polygonMatch2) {
            const vertices = parseInt(polygonMatch2[1]);
            const offset = parseInt(polygonMatch2[2]);
            return RegularPolygonGenerator.generate(vertices, offset);
        }
        
        const polygonMatch3 = cleaned.match(/^P\((\d+),(\d+),(\d+)\)$/i);
        if (polygonMatch3) {
            const vertices = parseInt(polygonMatch3[1]);
            const offset = parseInt(polygonMatch3[2]);
            const expansion = parseInt(polygonMatch3[3]);
            return RegularPolygonGenerator.generate(vertices, offset, expansion);
        }
        
        // Euclidean with offset: E(beats,steps,offset)
        const euclideanMatch3 = cleaned.match(/^E\((\d+),(\d+),(\d+)\)$/i);
        if (euclideanMatch3) {
            const beats = parseInt(euclideanMatch3[1]);
            const steps = parseInt(euclideanMatch3[2]);
            const offset = parseInt(euclideanMatch3[3]);
            const euclideanSteps = EuclideanGenerator.generate(beats, steps, offset);
            return {
                steps: euclideanSteps,
                stepCount: steps,
                beats,
                offset,
                isEuclidean: true,
                formula: `E(${beats},${steps},${offset})`
            };
        }
        
        // Euclidean without offset: E(beats,steps) - defaults to offset 0
        console.log(`🔍 Testing Euclidean regex for: "${cleaned}"`);
        const euclideanMatch2 = cleaned.match(/^E\((\d+),(\d+)\)$/i);
        console.log(`🔍 Euclidean regex result:`, euclideanMatch2);
        if (euclideanMatch2) {
            console.log(`🔍 Euclidean match found: ${cleaned}`);
            const beats = parseInt(euclideanMatch2[1]);
            const steps = parseInt(euclideanMatch2[2]);
            const offset = 0; // Default offset
            console.log(`🔍 Euclidean params: beats=${beats}, steps=${steps}, offset=${offset}`);
            
            try {
                const euclideanSteps = EuclideanGenerator.generate(beats, steps, offset);
                console.log(`🔍 Euclidean steps generated:`, euclideanSteps);
                
                const result = {
                    steps: euclideanSteps,
                    stepCount: steps,
                    beats,
                    offset,
                    isEuclidean: true,
                    formula: `E(${beats},${steps})`
                };
                console.log(`🔍 Euclidean result:`, result);
                return result;
            } catch (error) {
                console.error(`❌ Euclidean generation failed:`, error);
                return null;
            }
        }
        
        // Random pattern notation: R(onsets,steps)
        const randomMatch = cleaned.match(/^R\((\d+),(\d+)\)$/i);
        if (randomMatch) {
            const onsets = parseInt(randomMatch[1]);
            const steps = parseInt(randomMatch[2]);
            
            if (onsets > steps) {
                throw new Error(`Random pattern error: cannot have ${onsets} onsets in ${steps} steps`);
            }
            if (onsets < 0 || steps < 1) {
                throw new Error(`Random pattern error: invalid parameters R(${onsets},${steps})`);
            }
            
            // Generate random pattern
            const stepArray = new Array(steps).fill(false);
            const positions = [];
            
            // Generate unique random positions
            while (positions.length < onsets) {
                const pos = Math.floor(Math.random() * steps);
                if (!positions.includes(pos)) {
                    positions.push(pos);
                }
            }
            
            // Set onsets at selected positions
            positions.forEach(pos => {
                stepArray[pos] = true;
            });
            
            return {
                steps: stepArray,
                stepCount: steps,
                beats: onsets,
                isRandom: true,
                formula: `R(${onsets},${steps})`,
                randomSeed: Date.now() // For debugging/reproduction
            };
        }
        
        // Morse character notation: M:C or M:SOS
        const morseCharMatch = cleaned.match(/^M:([A-Z0-9]+)(?::(\d+))?$/i);
        if (morseCharMatch) {
            const morseChars = morseCharMatch[1].toUpperCase();
            const explicitStepCount = morseCharMatch[2] ? parseInt(morseCharMatch[2]) : null;
            
            // Morse code lookup table
            const morseTable = {
                'A': '.-', 'B': '-...', 'C': '-.-.', 'D': '-..', 'E': '.', 'F': '..-.',
                'G': '--.', 'H': '....', 'I': '..', 'J': '.---', 'K': '-.-', 'L': '.-..',
                'M': '--', 'N': '-.', 'O': '---', 'P': '.--.', 'Q': '--.-', 'R': '.-.',
                'S': '...', 'T': '-', 'U': '..-', 'V': '...-', 'W': '.--', 'X': '-..-',
                'Y': '-.--', 'Z': '--..', '0': '-----', '1': '.----', '2': '..---',
                '3': '...--', '4': '....-', '5': '.....', '6': '-....', '7': '--...',
                '8': '---..', '9': '----.', ' ': '/'
            };
            
            // Convert characters to morse string
            let morseString = '';
            for (const char of morseChars) {
                if (morseTable[char]) {
                    if (morseString) morseString += ' '; // Space between letters
                    morseString += morseTable[char];
                }
            }
            
            if (!morseString) return null; // Invalid characters
            
            // Convert morse to binary pattern with proper spacing
            // Each dot = 1 beat on, each dash = 1 beat on + 1 beat off
            let binaryPattern = '';
            const parts = morseString.split(' '); // Split by spaces between letters
            
            for (let i = 0; i < parts.length; i++) {
                const letter = parts[i];
                for (const symbol of letter) {
                    if (symbol === '.') {
                        binaryPattern += '1'; // Dot: one beat on
                    } else if (symbol === '-') {
                        binaryPattern += '10'; // Dash: one beat on, one beat off
                    }
                }
                
                // Add space between letters (except after the last letter)
                if (i < parts.length - 1) {
                    binaryPattern += ' '; // Space marker between letters
                }
            }
            
            // Replace space markers with nothing (direct concatenation for letters)
            let finalPattern = binaryPattern.replace(/ /g, '');
            
            // Handle explicit step count
            const totalSteps = explicitStepCount || finalPattern.length;
            
            if (explicitStepCount) {
                if (finalPattern.length < explicitStepCount) {
                    // Pad with zeros
                    finalPattern = finalPattern.padEnd(explicitStepCount, '0');
                } else if (finalPattern.length > explicitStepCount) {
                    // Truncate
                    finalPattern = finalPattern.substring(0, explicitStepCount);
                }
            }
            
            // Convert to steps array
            const steps = finalPattern.split('').map(bit => bit === '1');
            
            const stepCount = totalSteps;
            const result = {
                steps,
                stepCount,
                isMorse: true,
                morseCode: morseString,
                morseCharacters: morseChars,
                binaryPattern: finalPattern,
                formula: explicitStepCount ? `M:${morseChars}:${stepCount}` : `M:${morseChars}`,
                hasExplicitSteps: !!explicitStepCount
            };
            
            return result;
        }
        
        // Morse notation: -.-- (dot=short interval, dash=long interval) 
        const morseMatch = cleaned.match(/^([.-]+)(?::(\d+))?$/);
        if (morseMatch) {
            const morseString = morseMatch[1];
            const explicitStepCount = morseMatch[2] ? parseInt(morseMatch[2]) : null;
            
            // Convert Morse to interval pattern using dots and dashes
            // This creates a rhythmic pattern based on interval lengths
            // Dot = short interval (1), Dash = long interval (2)
            const intervals = [];
            for (const symbol of morseString) {
                if (symbol === '.') {
                    intervals.push(1); // Short interval
                } else if (symbol === '-') {
                    intervals.push(2); // Long interval (2x duration)
                }
            }
            
            // Convert intervals to binary pattern
            // Place onsets with spacing based on interval values
            const totalSteps = explicitStepCount || intervals.reduce((sum, interval) => sum + interval, 0);
            const steps = new Array(totalSteps).fill(false);
            
            let position = 0;
            for (let i = 0; i < intervals.length; i++) {
                if (position < totalSteps) {
                    steps[position] = true; // Place onset
                    position += intervals[i]; // Move by interval amount
                }
            }
            
            const stepCount = totalSteps;
            const result = {
                steps,
                stepCount,
                isMorse: true,
                morseCode: morseString,
                intervals: intervals,
                formula: explicitStepCount ? `${morseString}:${stepCount}` : morseString,
                hasExplicitSteps: !!explicitStepCount
            };
            
            return result;
        }
        
        // Onset array notation: [0,3,6] or [0,3,6]:8
        const onsetMatch = cleaned.match(/^\[([0-9,\s]+)\](?::(\d+))?$/);
        if (onsetMatch) {
            const onsetString = onsetMatch[1];
            const explicitStepCount = onsetMatch[2] ? parseInt(onsetMatch[2]) : null;
            
            // Parse onset positions
            const onsets = onsetString.split(',').map(s => parseInt(s.trim())).filter(n => !isNaN(n));
            if (onsets.length === 0) {
                throw new Error(`Invalid onset array: ${cleaned}. No valid onset positions found`);
            }
            
            // Determine step count
            const maxOnset = Math.max(...onsets);
            const stepCount = explicitStepCount || (maxOnset + 1);
            
            // Validate onsets are within step count
            if (onsets.some(onset => onset >= stepCount || onset < 0)) {
                throw new Error(`Invalid onset array: ${cleaned}. Onset positions must be between 0 and ${stepCount - 1}`);
            }
            
            // Convert to steps array
            const steps = new Array(stepCount).fill(false);
            onsets.forEach(onset => {
                steps[onset] = true;
            });
            
            return {
                steps,
                stepCount,
                onsets,
                isOnsetArray: true,
                formula: explicitStepCount ? `[${onsets.join(',')}]:${stepCount}` : `[${onsets.join(',')}]`
            };
        }
        
        // Shorthand polygon notation: tri, pent, hept, etc.
        const shorthandMap = {
            'bi': 2, 'bip': 2, 'bipod': 2,
            'tri': 3, 'triangle': 3,
            'quad': 4, 'square': 4, 'tetra': 4,
            'pent': 5, 'pentagon': 5, 'penta': 5,
            'hex': 6, 'hexagon': 6, 'hexa': 6,
            'hept': 7, 'heptagon': 7, 'hepta': 7, 'sept': 7, 'septa': 7,
            'oct': 8, 'octagon': 8, 'octa': 8,
            'enn': 9, 'enneagon': 9, 'nona': 9,
            'dec': 10, 'decagon': 10, 'deca': 10,
            'dod': 12, 'dodecagon': 12, 'dodeca': 12
        };
        
        const shorthandMatch = cleaned.match(/^(bi|bip|bipod|tri|triangle|quad|square|tetra|pent|pentagon|penta|hex|hexagon|hexa|hept|heptagon|hepta|sept|septa|oct|octagon|octa|enn|enneagon|nona|dec|decagon|deca|dod|dodecagon|dodeca)(?:@(\d+))?$/i);
        if (shorthandMatch) {
            const shape = shorthandMatch[1].toLowerCase();
            const offset = shorthandMatch[2] ? parseInt(shorthandMatch[2]) : 0;
            const vertices = shorthandMap[shape];
            
            if (vertices) {
                return RegularPolygonGenerator.generate(vertices, offset);
            }
        }
        
        // Binary notation: strict validation for only 0s and 1s, with optional step count
        const binaryMatch = cleaned.match(/^b?([01]+)(?::(\d+))?$/i);
        if (binaryMatch) {
            const binaryString = binaryMatch[1];
            const explicitStepCount = binaryMatch[2] ? parseInt(binaryMatch[2]) : null;
            
            if (explicitStepCount) {
                // Handle binary with explicit step count (like b1010:4)
                let paddedBinary;
                if (binaryString.length < explicitStepCount) {
                    paddedBinary = binaryString.padEnd(explicitStepCount, '0');
                } else if (binaryString.length > explicitStepCount) {
                    paddedBinary = binaryString.substring(0, explicitStepCount);
                } else {
                    paddedBinary = binaryString;
                }
                const result = PatternConverter.fromBinary(paddedBinary);
                result.hasExplicitSteps = true;
                return result;
            } else {
                // Handle regular binary (like b1010)
                return PatternConverter.fromBinary(binaryString);
            }
        }
        
        // Decimal notation: 73:8 (with step count) or 73 (pure numbers) - MUST come before invalid binary check
        // But first check if it's actually binary (only 0s and 1s)
        const decimalMatch = cleaned.match(/^(\d+)(?::(\d+))?$/);
        if (decimalMatch && !cleaned.startsWith('b')) {
            const numberPart = decimalMatch[1];
            
            // If it only contains 0s and 1s, treat as binary
            if (/^[01]+$/.test(numberPart)) {
                const binaryString = numberPart;
                const explicitStepCount = decimalMatch[2] ? parseInt(decimalMatch[2]) : null;
                
                if (explicitStepCount) {
                    // Handle binary with explicit step count (like 1010:4)
                    let paddedBinary;
                    if (binaryString.length < explicitStepCount) {
                        paddedBinary = binaryString.padEnd(explicitStepCount, '0');
                    } else if (binaryString.length > explicitStepCount) {
                        paddedBinary = binaryString.substring(0, explicitStepCount);
                    } else {
                        paddedBinary = binaryString;
                    }
                    const result = PatternConverter.fromBinary(paddedBinary);
                    result.hasExplicitSteps = true;
                    return result;
                } else {
                    // Handle regular binary (like 1010)
                    return PatternConverter.fromBinary(binaryString);
                }
            } else {
                // Handle as actual decimal
                return PatternConverter.fromDecimal(cleaned);
            }
        }
        
        // Check for invalid binary patterns (contains digits other than 0,1) - only for patterns starting with 'b'
        const invalidBinaryMatch = cleaned.match(/^b([0-9]+)$/i);
        if (invalidBinaryMatch && /[2-9]/.test(invalidBinaryMatch[1])) {
            throw new Error(`Invalid binary pattern: ${cleaned}. Binary patterns can only contain 0s and 1s`);
        }
        
        // Octal notation: 0o111:9 or 111:9 (with step count) or 0o111 (if looks like octal)
        const octalMatch = cleaned.match(/^(0o)?([0-7]+)(?::(\d+))?$/i);
        if (octalMatch && (octalMatch[1] || !/[89a-f]/i.test(octalMatch[2]))) {
            const pattern = PatternConverter.fromOctal(cleaned);
            if (pattern) return pattern;
        }
        
        // Hexadecimal notation: 0x49:8 or 49:8 (with step count) or 0x49 or 49 (if looks like hex)
        const hexMatch = cleaned.match(/^(0x)?([0-9a-f]+)(?::(\d+))?$/i);
        if (hexMatch && (hexMatch[1] || /[a-f]/i.test(hexMatch[2]))) {
            const pattern = PatternConverter.fromHex(cleaned);
            if (pattern) return pattern;
        }
        
        throw new Error(`Unrecognized pattern format: ${input}`);
    }
    
    static rotatePattern(pattern, rotationSteps) {
        if (!pattern || !pattern.steps || !Array.isArray(pattern.steps)) {
            throw new Error('Invalid pattern for rotation');
        }
        
        const stepCount = pattern.stepCount;
        const normalizedRotation = ((rotationSteps % stepCount) + stepCount) % stepCount;
        
        if (normalizedRotation === 0) {
            // No rotation needed
            return { ...pattern };
        }
        
        // Rotate the steps array
        const rotatedSteps = new Array(stepCount);
        for (let i = 0; i < stepCount; i++) {
            rotatedSteps[i] = pattern.steps[(i - normalizedRotation + stepCount) % stepCount];
        }
        
        // Create rotated pattern with rotation info
        const rotatedPattern = {
            ...pattern,
            steps: rotatedSteps,
            isRotated: true,
            rotationSteps: rotationSteps,
            originalPattern: pattern.steps
        };
        
        // Update formula if it exists
        if (pattern.formula) {
            rotatedPattern.formula = `${pattern.formula}@${rotationSteps}`;
        }
        
        return rotatedPattern;
    }
    
    static invertPattern(pattern) {
        if (!pattern || !pattern.steps || !Array.isArray(pattern.steps)) {
            throw new Error('Invalid pattern for inversion');
        }
        
        // Invert each step (flip bits)
        const invertedSteps = pattern.steps.map(step => !step);
        
        // Create inverted pattern
        const invertedPattern = {
            ...pattern,
            steps: invertedSteps,
            isInverted: true,
            originalPattern: pattern.steps
        };
        
        // Update formula if it exists
        if (pattern.formula) {
            invertedPattern.formula = `~${pattern.formula}`;
        }
        
        return invertedPattern;
    }
    
    static retrogradePattern(pattern) {
        if (!pattern || !pattern.steps || !Array.isArray(pattern.steps)) {
            throw new Error('Invalid pattern for retrograde');
        }
        
        // Reverse the steps array
        const retrogradeSteps = [...pattern.steps].reverse();
        
        // Create retrograde pattern
        const retrogradePattern = {
            ...pattern,
            steps: retrogradeSteps,
            isRetrograde: true,
            originalPattern: pattern.steps
        };
        
        // Update formula if it exists
        if (pattern.formula) {
            retrogradePattern.formula = `rev ${pattern.formula}`;
        }
        
        return retrogradePattern;
    }
    
    static complementPattern(pattern) {
        if (!pattern || !pattern.steps || !Array.isArray(pattern.steps)) {
            throw new Error('Invalid pattern for complement');
        }
        
        // Only works for Euclidean patterns
        if (!pattern.isEuclidean) {
            throw new Error('Complement operation only supported for Euclidean patterns');
        }
        
        const beats = pattern.beats || pattern.steps.filter(step => step).length;
        const steps = pattern.stepCount;
        const offset = pattern.offset || 0;
        
        // Generate Euclidean complement pattern
        const complementSteps = EuclideanGenerator.generateComplement(beats, steps, offset);
        const complementBeats = steps - beats;
        
        // Create complement pattern
        const complementPattern = {
            ...pattern,
            steps: complementSteps,
            beats: complementBeats,
            isComplement: true,
            originalPattern: pattern.steps,
            originalBeats: beats
        };
        
        // Update formula if it exists
        if (pattern.formula) {
            complementPattern.formula = `comp ${pattern.formula}`;
        } else if (pattern.euclidean) {
            complementPattern.formula = `comp ${pattern.euclidean}`;
        } else {
            // Create a formula for the complement
            const originalFormula = `E(${beats},${steps}${offset ? ',' + offset : ''})`;
            complementPattern.formula = `comp ${originalFormula}`;
        }
        
        // Update euclidean properties
        complementPattern.euclidean = `E(${complementBeats},${steps}${offset ? ',' + offset : ''})`;
        
        return complementPattern;
    }
    
    /**
     * Apply stretch or squeeze transformation to a pattern
     * @param {Object} pattern - Pattern object to transform
     * @param {number} factor - Stretch factor (>1 = stretch, <1 = squeeze)
     * @param {string} type - 'S' for stretch or 'Q' for squeeze
     * @returns {Object} Transformed pattern
     */
    static applyStretchTransformation(pattern, factor, type) {
        if (!pattern || !pattern.steps) return null;
        
        const originalSteps = pattern.steps;
        const originalStepCount = pattern.stepCount;
        
        // Create stretched/squeezed pattern
        let newSteps = [];
        
        if (factor >= 1) {
            // Stretch: make each step longer
            const stretchAmount = Math.round(factor);
            for (let i = 0; i < originalStepCount; i++) {
                if (originalSteps[i]) {
                    // Onset: single beat followed by (stretchAmount-1) rests
                    newSteps.push(true);
                    for (let j = 1; j < stretchAmount; j++) {
                        newSteps.push(false);
                    }
                } else {
                    // Rest: stretch the rest proportionally
                    for (let j = 0; j < stretchAmount; j++) {
                        newSteps.push(false);
                    }
                }
            }
        } else {
            // Squeeze: make pattern shorter by sampling
            const squeezeRatio = factor;
            const newLength = Math.max(1, Math.round(originalStepCount * squeezeRatio));
            
            for (let i = 0; i < newLength; i++) {
                const originalIndex = Math.round(i / squeezeRatio);
                if (originalIndex < originalStepCount) {
                    newSteps.push(originalSteps[originalIndex]);
                }
            }
        }
        
        const transformedPattern = {
            ...pattern,
            steps: newSteps,
            stepCount: newSteps.length,
            isTransformed: true,
            transformationType: type === 'S' ? 'stretch' : 'squeeze',
            transformationFactor: factor,
            originalPattern: originalSteps,
            originalStepCount: originalStepCount
        };
        
        return transformedPattern;
    }
    
    /**
     * Apply custom durations to short and long intervals
     * @param {Object} pattern - Pattern object to transform
     * @param {number} shortDuration - Duration for short intervals
     * @param {number} longDuration - Duration for long intervals
     * @returns {Object} Transformed pattern
     */
    static applyCustomDurations(pattern, shortDuration, longDuration) {
        if (!pattern || !pattern.steps) return null;
        
        // First, analyze the original pattern to identify long and short intervals
        const longShortAnalysis = LongShortAnalyzer.analyzeLongShort(pattern.steps, pattern.stepCount);
        
        if (!longShortAnalysis || !longShortAnalysis.intervals || longShortAnalysis.intervals.length === 0) {
            return pattern; // Can't transform if no intervals
        }
        
        const intervals = longShortAnalysis.intervals;
        const intervalTypes = longShortAnalysis.intervalTypes;
        
        // Create new pattern with custom durations
        let newSteps = [];
        let position = 0;
        
        // Start with first onset
        newSteps.push(true);
        position++;
        
        // Apply custom durations to each interval
        for (let i = 0; i < intervals.length; i++) {
            const intervalType = intervalTypes[i];
            let duration;
            
            if (intervalType === 'short') {
                duration = Math.round(shortDuration);
            } else if (intervalType === 'long') {
                duration = Math.round(longDuration);
            } else {
                // Equal intervals - use average of short and long
                duration = Math.round((shortDuration + longDuration) / 2);
            }
            
            // Add the duration as rests (except for the last beat which will be the next onset)
            for (let j = 1; j < duration; j++) {
                newSteps.push(false);
                position++;
            }
            
            // Add the next onset (unless this is the last interval)
            if (i < intervals.length - 1) {
                newSteps.push(true);
                position++;
            }
        }
        
        const transformedPattern = {
            ...pattern,
            steps: newSteps,
            stepCount: newSteps.length,
            isTransformed: true,
            transformationType: 'custom_durations',
            shortDuration: shortDuration,
            longDuration: longDuration,
            originalPattern: pattern.steps,
            originalStepCount: pattern.stepCount,
            originalAnalysis: longShortAnalysis
        };
        
        return transformedPattern;
    }
    
    /**
     * Parse comma-separated pattern strings: pattern1, pattern2, pattern3
     * @param {string} patternString - Comma-separated pattern string
     * @returns {Object} Combined pattern with divider information
     */
    /**
     * Check if commas in the string are pattern separators vs commas within pattern syntax
     * @param {string} input - Input string to check
     * @returns {boolean} True if commas are likely pattern separators
     */
    static hasPatternSeparatorCommas(input) {
        // Split by commas and check if we get multiple valid pattern-like parts
        // Use smart comma splitting to respect parentheses
        const parts = this.smartCommaSplit(input);
        if (parts.length < 2) return false;
        
        // Check if each part looks like a standalone pattern
        let validPatternParts = 0;
        for (const part of parts) {
            // Skip empty parts
            if (!part) continue;
            
            // Check if this part looks like a complete pattern
            // Patterns that contain unmatched parentheses are likely incomplete
            const openParens = (part.match(/\(/g) || []).length;
            const closeParens = (part.match(/\)/g) || []).length;
            
            // If parentheses are unmatched, this is likely part of a larger pattern
            if (openParens !== closeParens) {
                return false;
            }
            
            // Check if this looks like a recognizable pattern format
            if (this.looksLikePattern(part)) {
                validPatternParts++;
            }
        }
        // If we have 2+ parts that each look like valid patterns, it's pattern stringing
        return validPatternParts >= 2;
    }
    
    /**
     * Check if a string looks like a recognizable pattern
     * @param {string} part - String to check
     * @returns {boolean} True if it looks like a pattern
     */
    static looksLikePattern(part) {
        const cleaned = part.trim();
        if (!cleaned) return false;
        
        // Check for various pattern formats
        return (
            /^[01]+$/.test(cleaned) ||                    // Binary: 1011
            /^b[01]+$/i.test(cleaned) ||                  // Binary with prefix: b1011
            /^0x[0-9a-f]+$/i.test(cleaned) ||            // Hex: 0x92
            /^0o[0-7]+$/i.test(cleaned) ||               // Octal: 0o111
            /^\d+$/.test(cleaned) ||                      // Decimal: 73
            /^P\(\d+,\d+(,\d+)?\)$/i.test(cleaned) ||    // Polygon: P(3,1) or P(3,1,8)
            /^E\(\d+,\d+(,\d+)?\)$/i.test(cleaned) ||    // Euclidean: E(3,8) or E(3,8,0)
            /^R\(\d+,\d+\)$/i.test(cleaned) ||           // Random: R(3,8)
            /^M:[A-Z0-9]+$/i.test(cleaned) ||            // Morse: M:SOS
            /^[.\-]+$/.test(cleaned) ||                   // Morse intervals: -.--
            /^D:\d+,\d+/.test(cleaned) ||                // Duration: D:1,5
            /^[SQ]:\d*\.?\d+/.test(cleaned) ||           // Stretch/Squeeze: S:2, Q:0.5
            /^(tri|pent|hex|oct|quad|hept|dec|dod)(@\d+)?$/i.test(cleaned) || // Shorthand
            /^\[\d+(,\d+)*\]:\d+$/.test(cleaned) ||      // Onset array: [0,3,6]:8
            /^[01]+:\d+$/i.test(cleaned) ||              // Binary with step count: 1011:8
            /^b[01]+:\d+$/i.test(cleaned) ||             // Binary with prefix and step count: b1011:8
            /^0x[0-9a-f]+:\d+$/i.test(cleaned) ||        // Hex with step count: 0x92:8
            /^0o[0-7]+:\d+$/i.test(cleaned) ||           // Octal with step count: 0o111:8
            /^\d+:\d+$/.test(cleaned) ||                 // Decimal with step count: 73:8
            /^[~]/.test(cleaned) ||                      // Inversion: ~pattern
            /^rev\s/.test(cleaned) ||                    // Reversal: rev pattern
            /^inv\s/.test(cleaned) ||                    // Inversion: inv pattern
            /^comp\s/.test(cleaned) ||                   // Complement: comp pattern
            cleaned.includes('+') ||                     // Combinations: P(3,1)+P(5,0)
            cleaned.includes('-') ||                     // Subtractions: P(3,1)-P(2,0)
            cleaned.includes('@')                        // Rotations: pattern@3
        );
    }

    /**
     * Split string by commas while respecting parentheses
     * @param {string} input - Input string to split
     * @returns {string[]} Array of parts split by top-level commas
     */
    static smartCommaSplit(input) {
        const parts = [];
        let current = '';
        let parenDepth = 0;
        let bracketDepth = 0;
        
        for (let i = 0; i < input.length; i++) {
            const char = input[i];
            
            if (char === '(') {
                parenDepth++;
            } else if (char === ')') {
                parenDepth--;
            } else if (char === '[') {
                bracketDepth++;
            } else if (char === ']') {
                bracketDepth--;
            } else if (char === ',' && parenDepth === 0 && bracketDepth === 0) {
                // This is a top-level comma - split here
                parts.push(current.trim());
                current = '';
                continue;
            }
            
            current += char;
        }
        
        // Add the last part
        if (current.trim()) {
            parts.push(current.trim());
        }
        
        return parts;
    }

    static parseStringedPatterns(patternString) {
        // Smart comma splitting that respects parentheses
        const parts = this.smartCommaSplit(patternString);
        if (parts.length < 2) return null;
        
        const parsedPatterns = [];
        const dividerPositions = [];
        let totalSteps = 0;
        
        // Parse each pattern part
        for (const part of parts) {
            if (!part) continue;
            
            console.log(`🔍 Parsing stringed pattern part: "${part}"`);
            try {
                const parsed = this.parsePattern(part);
                console.log(`🔍 Parsed result:`, parsed);
                if (!parsed) {
                    console.error(`❌ Failed to parse pattern part: "${part}"`);
                    return null; // Invalid pattern part
                }
                parsedPatterns.push(parsed);
                
                // Record divider position (before this pattern, except for first)
                if (totalSteps > 0) {
                    dividerPositions.push(totalSteps);
                }
                
                totalSteps += parsed.stepCount;
            } catch (error) {
                console.error(`❌ Pattern parsing threw error for "${part}":`, error);
                return null;
            }
        }
        
        if (parsedPatterns.length === 0) return null;
        
        // Combine all pattern steps
        const combinedSteps = [];
        for (const pattern of parsedPatterns) {
            combinedSteps.push(...pattern.steps);
        }
        
        // Create the stringed pattern result
        const result = {
            steps: combinedSteps,
            stepCount: totalSteps,
            isStringed: true,
            stringedPatterns: parsedPatterns,
            dividerPositions: dividerPositions,
            formula: patternString,
            originalString: patternString
        };
        
        return result;
    }
    
    /**
     * Check if input contains space-separated patterns
     * @param {string} input - Input string to check
     * @returns {boolean} True if input has multiple space-separated patterns
     */
    static hasSpaceSeparatedPatterns(input) {
        // Split by spaces and check if we get multiple valid pattern parts
        const parts = input.trim().split(/\s+/);
        if (parts.length < 2) return false;
        
        // Check if each part looks like a standalone pattern
        let validPatternParts = 0;
        for (const part of parts) {
            if (!part) continue;
            
            // Check if this looks like a recognizable pattern format
            if (this.looksLikePattern(part)) {
                validPatternParts++;
            }
        }
        
        // If we have 2+ parts that each look like valid patterns, it's space stringing
        return validPatternParts >= 2;
    }
    
    /**
     * Parse space-separated patterns: pattern1 pattern2 pattern3
     * @param {string} patternString - Space-separated pattern string
     * @returns {Object} Stringed pattern result
     */
    static parseSpaceStringedPatterns(patternString) {
        // Split by spaces
        const parts = patternString.trim().split(/\s+/);
        if (parts.length < 2) return null;
        
        const parsedPatterns = [];
        const dividerPositions = [];
        let totalSteps = 0;
        
        // Parse each pattern part
        for (const part of parts) {
            if (!part) continue;
            
            console.log(`🔍 Parsing space-separated pattern part: "${part}"`);
            try {
                const parsed = this.parsePattern(part);
                if (parsed) {
                    // Mark divider position before adding this pattern
                    if (totalSteps > 0) {
                        dividerPositions.push(totalSteps);
                    }
                    
                    parsedPatterns.push(parsed);
                    totalSteps += parsed.stepCount;
                    console.log(`✅ Parsed space-separated part "${part}":`, parsed);
                } else {
                    console.log(`❌ Failed to parse space-separated part: "${part}"`);
                    return null;
                }
            } catch (error) {
                console.error(`❌ Error parsing space-separated part "${part}":`, error);
                return null;
            }
        }
        
        if (parsedPatterns.length < 2) {
            console.log(`❌ Need at least 2 patterns for space stringing, got ${parsedPatterns.length}`);
            return null;
        }
        
        // Concatenate all patterns
        const steps = [];
        for (const pattern of parsedPatterns) {
            steps.push(...pattern.steps);
        }
        
        const result = {
            steps: steps,
            stepCount: totalSteps,
            isStringed: true,
            stringedPatterns: parsedPatterns,
            dividerPositions: dividerPositions,
            formula: patternString,
            originalString: patternString
        };
        
        console.log(`✅ Space stringing result:`, result);
        return result;
    }
    
    static formatCompact(pattern) {
        console.log(`🔍 formatCompact called with pattern:`, pattern);
        // Special handling for stringed patterns
        if (pattern.isStringed && pattern.stringedPatterns) {
            console.log(`🔍 Processing stringed pattern with ${pattern.stringedPatterns.length} parts`);
            const patternReprs = pattern.stringedPatterns.map(p => {
                const binary = PatternConverter.toBinary(p.steps, p.stepCount);
                const decimal = PatternConverter.toDecimal(binary, !p.isBinaryInput);
                const hex = PatternConverter.toHex(decimal);
                return { binary: `b${binary}`, hex, decimal };
            });
            
            const binaryParts = patternReprs.map(r => r.binary).join(' ');
            const hexParts = patternReprs.map(r => r.hex).join(' ');
            const decimalParts = patternReprs.map(r => r.decimal).join(' ');
            
            const beats = pattern.steps.filter(s => s).length;
            const density = ((beats / pattern.stepCount) * 100).toFixed(1);
            
            let result = `Binary: ${binaryParts}, Hex: ${hexParts}, Decimal: ${decimalParts} (${pattern.stepCount} steps, ${beats} beats, ${density}% density)`;
            
            // Add stringed pattern type info
            result += ` [🔗 STRINGED: ${pattern.originalString}]`;
            return result;
        }
        
        // Regular pattern formatting
        const binary = PatternConverter.toBinary(pattern.steps, pattern.stepCount);
        // Use standard binary interpretation for binary input, reversed for numeric input
        const decimal = PatternConverter.toDecimal(binary, !pattern.isBinaryInput);
        const hex = PatternConverter.toHex(decimal);
        const octal = PatternConverter.toOctal(decimal);
        const beats = pattern.steps.filter(s => s).length;
        const density = ((beats / pattern.stepCount) * 100).toFixed(1);
        
        let result = `b${binary}, ${hex}, ${octal}, ${decimal} (${pattern.stepCount} steps, ${beats} beats, ${density}% density)`;
        
        const types = [];
        
        if (pattern.isRegularPolygon && pattern.polygonType) {
            types.push(`🔺${pattern.formula}`);
        }
        
        if (pattern.isEuclidean && pattern.formula) {
            types.push(`🌀${pattern.formula}`);
        }
        
        if (pattern.isCombined) {
            if (pattern.originalPatterns && pattern.originalPatterns.length > 0) {
                const descriptions = pattern.originalPatterns.map(p => {
                    if (p.isRegularPolygon) {
                        return `P(${p.vertices},${p.offset || 0}${p.expansion && p.expansion !== 1 ? ',' + p.expansion : ''})`;
                    } else if (p.isEuclidean) {
                        return `E(${p.beats},${p.stepCount},${p.offset || 0})`;
                    } else if (p.binary) {
                        return `b${p.binary}`;
                    } else if (p.stepCount) {
                        return `${p.stepCount}steps`;
                    } else {
                        return 'pattern';
                    }
                });
                types.push(`🎯COMBINED: ${descriptions.join(' + ')} (LCM=${pattern.stepCount})`);
            } else {
                types.push(`🎯COMBINED PATTERN`);
            }
        }
        
        if (pattern.hasExplicitSteps) {
            types.push(`🎯Enhanced`);
        }
        
        if (pattern.isRotated) {
            types.push(`🔄Rotated @${pattern.rotationSteps}`);
        }
        
        if (types.length > 0) {
            result += ` [${types.join(', ')}]`;
        }
        
        return result;
    }
    
    // Alias method for compatibility
    static parseUniversalInput(input) {
        return this.parse(input);
    }
}

/**
 * Quantization Engine for Binarization/Ternarization
 * Implements onset redistribution based on Toussaint's algorithm
 */
class QuantizationEngine {
    /**
     * Quantize a pattern to a new step count using angular mapping
     * @param {Object} pattern - Source pattern
     * @param {number} newSteps - Target step count
     * @param {boolean} clockwise - Direction of quantization (true = clockwise, false = counterclockwise)
     * @returns {Object} Quantized pattern
     */
    static quantizePattern(pattern, newSteps, clockwise = true) {
        if (!pattern || !pattern.steps || !Array.isArray(pattern.steps)) {
            throw new Error('Invalid pattern for quantization');
        }
        
        if (newSteps < 1 || newSteps > 128) {
            throw new Error('New step count must be between 1 and 128');
        }
        
        const originalSteps = pattern.steps;
        const originalStepCount = pattern.stepCount;
        
        // Handle edge cases
        if (newSteps === originalStepCount) {
            return { ...pattern }; // No quantization needed
        }
        
        // Find all onset positions in the original pattern
        const onsetPositions = [];
        for (let i = 0; i < originalStepCount; i++) {
            if (originalSteps[i]) {
                onsetPositions.push(i);
            }
        }
        
        // If no onsets, return empty pattern
        if (onsetPositions.length === 0) {
            return {
                ...pattern,
                steps: new Array(newSteps).fill(false),
                stepCount: newSteps,
                isQuantized: true,
                quantizationDirection: clockwise ? 'clockwise' : 'counterclockwise',
                originalStepCount: originalStepCount,
                originalPattern: originalSteps
            };
        }
        
        // Convert onset positions to angular positions (0 to 2π)
        const angularPositions = onsetPositions.map(pos => {
            const angle = (pos / originalStepCount) * 2 * Math.PI;
            return clockwise ? angle : (2 * Math.PI - angle);
        });
        
        // Map angular positions to new step positions
        const newStepsArray = new Array(newSteps).fill(false);
        const mappedPositions = new Set(); // Track to handle conflicts
        
        for (const angle of angularPositions) {
            // Convert angle back to step position in new pattern
            const normalizedAngle = clockwise ? angle : (2 * Math.PI - angle);
            const newPosition = Math.round((normalizedAngle / (2 * Math.PI)) * newSteps) % newSteps;
            
            // Handle the case where position maps to step 0 when it should be at the end
            const finalPosition = newPosition === 0 && normalizedAngle > Math.PI ? newSteps - 1 : newPosition;
            
            mappedPositions.add(finalPosition);
        }
        
        // Set onsets at mapped positions
        for (const pos of mappedPositions) {
            newStepsArray[pos] = true;
        }
        
        // Create quantized pattern
        const quantizedPattern = {
            ...pattern,
            steps: newStepsArray,
            stepCount: newSteps,
            isQuantized: true,
            quantizationDirection: clockwise ? 'clockwise' : 'counterclockwise',
            originalStepCount: originalStepCount,
            originalPattern: originalSteps,
            quantizationRatio: newSteps / originalStepCount
        };
        
        // Update formula if it exists
        if (pattern.formula) {
            const direction = clockwise ? '' : '-';
            quantizedPattern.formula = `${pattern.formula};${direction}${newSteps}`;
        }
        
        return quantizedPattern;
    }
    
    /**
     * Parse quantization notation from input string
     * @param {string} input - Input like "pattern:steps;newSteps" or "pattern;-newSteps"
     * @returns {Object|null} Parsed quantization info or null if not quantization
     */
    static parseQuantizationNotation(input) {
        // Match pattern with semicolon quantization: pattern:steps;newSteps or pattern;-newSteps
        const quantizationMatch = input.match(/^(.+);(-?\d+)$/);
        
        if (!quantizationMatch) {
            return null;
        }
        
        const [, basePattern, quantizationStr] = quantizationMatch;
        const quantizationValue = parseInt(quantizationStr);
        
        if (isNaN(quantizationValue) || quantizationValue === 0) {
            throw new Error('Invalid quantization step count');
        }
        
        const newSteps = Math.abs(quantizationValue);
        const clockwise = quantizationValue > 0;
        
        return {
            basePattern: basePattern.trim(),
            newSteps,
            clockwise
        };
    }
    
    /**
     * Check if input contains quantization notation
     * @param {string} input - Input string to check
     * @returns {boolean} True if contains quantization notation
     */
    static hasQuantizationNotation(input) {
        return /^.+;-?\d+$/.test(input.trim());
    }
    
    /**
     * Calculate quantization quality metrics
     * @param {Object} originalPattern - Original pattern
     * @param {Object} quantizedPattern - Quantized pattern
     * @returns {Object} Quality metrics
     */
    static calculateQuantizationMetrics(originalPattern, quantizedPattern) {
        const originalOnsets = originalPattern.steps.filter(s => s).length;
        const quantizedOnsets = quantizedPattern.steps.filter(s => s).length;
        
        const onsetPreservation = originalOnsets > 0 ? quantizedOnsets / originalOnsets : 1;
        const densityChange = quantizedPattern.stepCount / originalPattern.stepCount;
        
        return {
            onsetPreservation: onsetPreservation,
            densityChange: densityChange,
            onsetCount: {
                original: originalOnsets,
                quantized: quantizedOnsets
            },
            stepCount: {
                original: originalPattern.stepCount,
                quantized: quantizedPattern.stepCount
            }
        };
    }
}

// Export to global scope for browser compatibility
if (typeof window !== 'undefined') {
    window.PatternConverter = PatternConverter;
    window.AdvancedPatternCombiner = AdvancedPatternCombiner;
    window.UnifiedPatternParser = UnifiedPatternParser;
    window.QuantizationEngine = QuantizationEngine;
}