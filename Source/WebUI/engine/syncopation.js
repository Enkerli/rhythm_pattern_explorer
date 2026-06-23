// syncopation.js — ported verbatim from the original webapp's
// SyncopationAnalyzer (WebApp/app/pattern-analysis.js). Six musicological
// measures of rhythmic syncopation + a weighted overall score, level and
// description. Self-contained; no DOM. analyzeSyncopation(steps, stepCount).

export function analyzeSyncopation(steps, stepCount) {
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
            barlowIndispensability: 0,
            overallSyncopation: 0,
            description: 'No syncopation (no onsets)',
            level: 'none'
        };
    }
    
    // Calculate each syncopation measure
    const weightedNoteToBeats = calculateWeightedNoteToBeats(onsetPositions, stepCount);
    const offBeatRatio = calculateOffBeatRatio(onsetPositions, stepCount);
    const expectancyViolation = calculateExpectancyViolation(onsetPositions, stepCount);
    const rhythmicDisplacement = calculateRhythmicDisplacement(onsetPositions, stepCount);
    const crossRhythmic = calculateCrossRhythmic(steps, stepCount);
    const barlowIndispensability = calculateBarlowIndispensability(onsetPositions, stepCount);
    
    // Calculate overall syncopation (weighted average)
    const overallSyncopation = (
        weightedNoteToBeats * 0.2 +
        offBeatRatio * 0.15 +
        expectancyViolation * 0.2 +
        rhythmicDisplacement * 0.15 +
        crossRhythmic * 0.15 +
        barlowIndispensability * 0.15
    );
    
    const level = getSyncopationLevel(overallSyncopation);
    const description = getSyncopationDescription(overallSyncopation, level);
    
    return {
        weightedNoteToBeats: Math.round(weightedNoteToBeats * 1000) / 1000,
        offBeatRatio: Math.round(offBeatRatio * 1000) / 1000,
        expectancyViolation: Math.round(expectancyViolation * 1000) / 1000,
        rhythmicDisplacement: Math.round(rhythmicDisplacement * 1000) / 1000,
        crossRhythmic: Math.round(crossRhythmic * 1000) / 1000,
        barlowIndispensability: Math.round(barlowIndispensability * 1000) / 1000,
        overallSyncopation: Math.round(overallSyncopation * 1000) / 1000,
        description,
        level
    };
}

/**
 * Weighted Note-to-Beat Distance (Longuet-Higgins & Lee)
 * Measures distance from onsets to strong metric positions
 */
function calculateWeightedNoteToBeats(onsetPositions, stepCount) {
    let totalWeight = 0;
    let totalDistance = 0;
    
    for (const position of onsetPositions) {
        const metricWeight = getMetricWeight(position, stepCount);
        const distanceToStrongBeat = getDistanceToStrongBeat(position, stepCount);
        
        totalWeight += metricWeight;
        totalDistance += distanceToStrongBeat * (1 - metricWeight); // Invert weight for syncopation
    }
    
    return totalWeight > 0 ? totalDistance / onsetPositions.length : 0;
}

/**
 * Off-Beat Onset Ratio
 * Percentage of onsets on weak beats vs strong beats
 */
function calculateOffBeatRatio(onsetPositions, stepCount) {
    let offBeatCount = 0;
    
    for (const position of onsetPositions) {
        const metricWeight = getMetricWeight(position, stepCount);
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
function calculateExpectancyViolation(onsetPositions, stepCount) {
    let totalViolation = 0;
    
    for (const position of onsetPositions) {
        const expectedProbability = getExpectedOnsetProbability(position, stepCount);
        const violation = 1 - expectedProbability; // Higher for unexpected positions
        totalViolation += violation;
    }
    
    return onsetPositions.length > 0 ? totalViolation / onsetPositions.length : 0;
}

/**
 * Rhythmic Displacement Index
 * How far onsets are displaced from expected positions
 */
function calculateRhythmicDisplacement(onsetPositions, stepCount) {
    let totalDisplacement = 0;
    
    for (const position of onsetPositions) {
        const nearestStrongBeat = getNearestStrongBeat(position, stepCount);
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
function calculateCrossRhythmic(steps, stepCount) {
    // Use auto-correlation to find competing periodicities
    const periodicities = [2, 3, 4, 6, 8]; // Common metric divisions
    let maxConflict = 0;
    
    for (const period of periodicities) {
        if (period >= stepCount) continue;
        
        const correlation = calculateAutoCorrelation(steps, period);
        const expectedCorrelation = getExpectedCorrelation(stepCount, period);
        const conflict = Math.abs(correlation - expectedCorrelation);
        maxConflict = Math.max(maxConflict, conflict);
    }
    
    return Math.min(maxConflict, 1); // Normalize to 0-1
}

/**
 * Barlow Indispensability (Clarence Barlow)
 * Measures syncopation based on prime factorization of beat positions
 */
function calculateBarlowIndispensability(onsetPositions, stepCount) {
    let totalIndispensability = 0;
    const indispensabilityTable = generateBarlowIndispensabilityTable(stepCount);
    
    for (const position of onsetPositions) {
        const indispensability = indispensabilityTable[position];
        // Higher indispensability = less syncopated, so invert for syncopation measure
        const syncopationValue = 1 - (indispensability / Math.max(...indispensabilityTable));
        totalIndispensability += syncopationValue;
    }
    
    return onsetPositions.length > 0 ? totalIndispensability / onsetPositions.length : 0;
}

/**
 * Generate Barlow's indispensability table for a given length
 * Based on prime factorization approach
 */
function generateBarlowIndispensabilityTable(length) {
    const table = new Array(length);
    
    for (let i = 0; i < length; i++) {
        table[i] = calculatePositionIndispensability(i, length);
    }
    
    
    return table;
}

/**
 * Calculate indispensability for a specific position
 * Using enhanced Barlow's method with stratified meter decomposition
 */
function calculatePositionIndispensability(position, length) {
    if (position === 0) return 1; // Downbeat is most indispensable
    
    // Special case: pickup beat (last position) has high indispensability
    // due to its anacrustic function leading to the downbeat
    if (position === length - 1) {
        // Give it high indispensability - not as high as downbeat, but higher than most other positions
        // Make it roughly equivalent to a strong quarter note position
        return 0.75;
    }
    
    // Enhanced approach: Stratified meter decomposition
    const stratification = getStratifiedMeter(length);
    let totalIndispensability = 0;
    
    for (let level = 0; level < stratification.length; level++) {
        const primeAtLevel = stratification[level];
        const levelSize = length / Math.pow(primeAtLevel, level + 1);
        
        if (levelSize > 0 && position % levelSize === 0) {
            // This position aligns with this stratification level
            const levelIndispensability = 1 / Math.pow(primeAtLevel, level + 1);
            totalIndispensability += levelIndispensability;
        }
    }
    
    // Fallback to original method if stratification doesn't apply
    if (totalIndispensability === 0) {
        const gcd = greatestCommonDivisor(position, length);
        const level = length / gcd;
        const primeFactors = getPrimeFactors(level);
        
        totalIndispensability = 1;
        for (const prime of primeFactors) {
            totalIndispensability *= (1 / prime);
        }
    }
    
    return Math.min(totalIndispensability, 1);
}

/**
 * Get stratified meter decomposition (prime factorization hierarchy)
 * Following the document's approach: bar → quarters → eighths → sixteenths
 */
function getStratifiedMeter(length) {
    const primes = getPrimeFactors(length);
    // Sort primes to create hierarchical stratification
    // Smaller primes (2, 3) typically represent higher-level divisions
    return primes.sort((a, b) => a - b);
}

/**
 * Calculate greatest common divisor using Euclidean algorithm
 */
function greatestCommonDivisor(a, b) {
    while (b !== 0) {
        const temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * Get prime factors of a number
 */
function getPrimeFactors(n) {
    const factors = [];
    let divisor = 2;
    
    while (n > 1) {
        while (n % divisor === 0) {
            factors.push(divisor);
            n /= divisor;
        }
        divisor++;
        if (divisor * divisor > n && n > 1) {
            factors.push(n);
            break;
        }
    }
    
    return factors;
}

/**
 * Get metric weight for a position (1 = strongest, 0 = weakest)
 */
function getMetricWeight(position, stepCount) {
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
function getDistanceToStrongBeat(position, stepCount) {
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
function getExpectedOnsetProbability(position, stepCount) {
    const metricWeight = getMetricWeight(position, stepCount);
    return metricWeight * 0.8 + 0.1; // Scale to 0.1-0.9 range
}

/**
 * Get nearest strong beat position
 */
function getNearestStrongBeat(position, stepCount) {
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
function calculateAutoCorrelation(steps, lag) {
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
function getExpectedCorrelation(stepCount, period) {
    // Expected correlation based on typical metric hierarchy
    if (period === stepCount / 2) return 0.7; // Half note correlation
    if (period === stepCount / 4) return 0.5; // Quarter note correlation
    return 0.3; // Other periods
}

/**
 * Get syncopation level description
 */
function getSyncopationLevel(syncopationScore) {
    if (syncopationScore < 0.1) return 'none';
    if (syncopationScore < 0.3) return 'low';
    if (syncopationScore < 0.5) return 'moderate';
    if (syncopationScore < 0.7) return 'high';
    return 'extreme';
}

/**
 * Get descriptive text for syncopation score
 */
function getSyncopationDescription(score, level) {
    const descriptions = {
        'none': 'No syncopation - strong metric alignment',
        'low': 'Minimal syncopation - mostly on-beat',
        'moderate': 'Moderate syncopation - balanced off-beat elements',
        'high': 'High syncopation - significant rhythmic tension',
        'extreme': 'Extreme syncopation - heavily off-beat emphasis'
    };
    
    return descriptions[level] || 'Unknown syncopation level';
}
