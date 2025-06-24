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
    
    static toEnhancedHex(decimal, stepCount) {
        const hex = this.toHex(decimal);
        return `${hex}:${stepCount}`;
    }
    
    static toEnhancedDecimal(decimal, stepCount) {
        return `${decimal}:${stepCount}`;
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
        
        const euclideanMatch = cleaned.match(/^E\((\d+),(\d+),(\d+)\)$/i);
        if (euclideanMatch) {
            const beats = parseInt(euclideanMatch[1]);
            const steps = parseInt(euclideanMatch[2]);
            const offset = parseInt(euclideanMatch[3]);
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
    
    static formatCompact(pattern) {
        const binary = PatternConverter.toBinary(pattern.steps, pattern.stepCount);
        const decimal = PatternConverter.toDecimal(binary);
        const hex = PatternConverter.toHex(decimal);
        const beats = pattern.steps.filter(s => s).length;
        const density = ((beats / pattern.stepCount) * 100).toFixed(1);
        
        let result = `b${binary}, ${hex}, ${decimal} (${pattern.stepCount} steps, ${beats} beats, ${density}% density)`;
        
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