// === MODULE 4: PATTERN PROCESSING ===
class PatternConverter {
    static toBinary(steps, stepCount) {
        return Array(stepCount).fill(0)
            .map((_, i) => steps[i] ? '1' : '0')
            .join('');
    }
    
    static fromBinary(binaryString) {
        return {
            steps: binaryString.split('').map(bit => bit === '1'),
            stepCount: binaryString.length
        };
    }
    
    static toDecimal(binaryString) {
        return parseInt(binaryString, 2) || 0;
    }
    
    static toHex(decimal) {
        return '0x' + decimal.toString(16).toUpperCase();
    }
    
    static toOctal(decimal) {
        return '0o' + decimal.toString(8);
    }
    
    static fromDecimalWithSteps(decimal, stepCount) {
        if (decimal === 0) return { steps: new Array(stepCount).fill(false), stepCount };
        
        const binaryString = decimal.toString(2).padStart(stepCount, '0');
        return {
            steps: binaryString.split('').map(bit => bit === '1'),
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
        const binaryString = numericDecimal.toString(2).padStart(stepCount, '0');
        return this.fromBinary(binaryString);
    }
    
    static fromHex(hexString) {
        // Handle colon notation: "0x92:8" or "92:8"
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
        // Handle colon notation: "0o452:9" or "452:9"
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
        
        // Check for pattern naming: name=pattern
        if (cleaned.includes('=') && !cleaned.includes('P(') && !cleaned.includes('E(')) {
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
        
        if (cleaned.includes('+')) {
            const parts = cleaned.split('+').map(p => p.trim());
            if (parts.length < 2) {
                throw new Error('Combination syntax must be: pattern1 + pattern2 [+ pattern3 ...]');
            }
            
            const patterns = parts.map(part => this.parseSingle(part));
            const combined = AdvancedPatternCombiner.combineMultiplePatterns(patterns);
            
            return {
                type: 'combination',
                patterns: patterns,
                combined: combined,
                multiPattern: true
            };
        }
        
        return {
            type: 'single',
            pattern: this.parseSingle(cleaned)
        };
    }
    
    static parseSingle(input) {
        return this.parsePattern(input.trim());
    }
    
    static parsePattern(input) {
        const cleaned = input.trim();
        
        // Check for transformation prefixes: ~, rev, inv
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
        
        // Check for rotation notation: pattern@steps
        if (cleaned.includes('@')) {
            const parts = cleaned.split('@');
            if (parts.length === 2) {
                const pattern = parts[0].trim();
                const rotationSteps = parseInt(parts[1].trim());
                
                if (!isNaN(rotationSteps)) {
                    // Parse the base pattern first
                    const basePattern = this.parsePattern(pattern);
                    // Apply rotation
                    return this.rotatePattern(basePattern, rotationSteps);
                }
            }
        }
        
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
        const euclideanMatch2 = cleaned.match(/^E\((\d+),(\d+)\)$/i);
        if (euclideanMatch2) {
            const beats = parseInt(euclideanMatch2[1]);
            const steps = parseInt(euclideanMatch2[2]);
            const offset = 0; // Default offset
            const euclideanSteps = EuclideanGenerator.generate(beats, steps, offset);
            return {
                steps: euclideanSteps,
                stepCount: steps,
                beats,
                offset,
                isEuclidean: true,
                formula: `E(${beats},${steps})`
            };
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
        
        const shorthandMatch = cleaned.match(/^(bi|bip|bipod|tri|triangle|quad|square|tetra|pent|pentagon|penta|hex|hexagon|hexa|hept|heptagon|hepta|sept|septa|oct|octagon|octa|enn|enneagon|nona|dec|decagon|deca|dod|dodecagon|dodeca)(?:\+(\d+))?$/i);
        if (shorthandMatch) {
            const shape = shorthandMatch[1].toLowerCase();
            const offset = shorthandMatch[2] ? parseInt(shorthandMatch[2]) : 0;
            const vertices = shorthandMap[shape];
            
            if (vertices) {
                return RegularPolygonGenerator.generate(vertices, offset);
            }
        }
        
        // Binary notation: strict validation for only 0s and 1s
        const binaryMatch = cleaned.match(/^b?([01]+)$/i);
        if (binaryMatch) {
            return PatternConverter.fromBinary(binaryMatch[1]);
        }
        
        // Check for invalid binary patterns (contains digits other than 0,1)
        const invalidBinaryMatch = cleaned.match(/^b?([0-9]+)$/i);
        if (invalidBinaryMatch && /[2-9]/.test(invalidBinaryMatch[1])) {
            throw new Error(`Invalid binary pattern: ${cleaned}. Binary patterns can only contain 0s and 1s`);
        }
        
        // Octal notation: 0o452:9 or 452:9 (with step count) or 0o452 (if looks like octal)
        const octalMatch = cleaned.match(/^(0o)?([0-7]+)(?::(\d+))?$/i);
        if (octalMatch && (octalMatch[1] || !/[89a-f]/i.test(octalMatch[2]))) {
            const pattern = PatternConverter.fromOctal(cleaned);
            if (pattern) return pattern;
        }
        
        // Hexadecimal notation: 0x92:8 or 92:8 (with step count) or 0x92 or 92 (if looks like hex)
        const hexMatch = cleaned.match(/^(0x)?([0-9a-f]+)(?::(\d+))?$/i);
        if (hexMatch && (hexMatch[1] || /[a-f]/i.test(hexMatch[2]))) {
            const pattern = PatternConverter.fromHex(cleaned);
            if (pattern) return pattern;
        }
        
        // Decimal notation: 146:8 (with step count) or 146 (pure numbers)
        const decimalMatch = cleaned.match(/^(\d+)(?::(\d+))?$/);
        if (decimalMatch) {
            return PatternConverter.fromDecimal(cleaned);
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
    
    static formatCompact(pattern) {
        const binary = PatternConverter.toBinary(pattern.steps, pattern.stepCount);
        const decimal = PatternConverter.toDecimal(binary);
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

// Export to global scope for browser compatibility
if (typeof window !== 'undefined') {
    window.PatternConverter = PatternConverter;
    window.AdvancedPatternCombiner = AdvancedPatternCombiner;
    window.UnifiedPatternParser = UnifiedPatternParser;
}