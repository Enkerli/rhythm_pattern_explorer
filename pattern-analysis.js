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