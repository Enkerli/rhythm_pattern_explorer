// mutate.js — ported from the original webapp's RhythmMutator
// (WebApp/app/pattern-analysis.js). Move each onset by a style + amount:
// balanced, groove, syncopate, straighten, swing, shuffle. Returns
// { mutated, mutatedOnsets, ... }. Self-contained, no DOM.

export function mutatePattern(originalPattern, mutationAmount = 0.5, options = {}) {
    const {
        preserveOnsetCount = true,    // Keep same number of hits
        allowMicroTiming = false,     // Allow sub-step displacement (future feature)
        mutationStyle = 'balanced',   // 'balanced', 'groove', 'syncopate', 'straighten'
        constrainToGrid = true,       // Keep onsets on grid positions
        respectMeter = true           // Bias toward metrically strong positions
    } = options;
    
    if (!Array.isArray(originalPattern) || originalPattern.length === 0) {
        throw new Error('Invalid original pattern for mutation');
    }
    
    const stepCount = originalPattern.length;
    const originalOnsets = getOnsetPositions(originalPattern);
    
    if (originalOnsets.length === 0) {
        return {
            pattern: [...originalPattern],
            mutated: [...originalPattern],
            originalOnsets: [],
            mutatedOnsets: [],
            mutationAmount: 0,
            displacement: 0
        };
    }
    
    // Generate mutated onset positions
    const mutatedOnsets = generateMutatedOnsets(
        originalOnsets, 
        stepCount, 
        mutationAmount, 
        mutationStyle,
        options
    );
    
    // Create new pattern from mutated onsets
    const mutatedPattern = new Array(stepCount).fill(false);
    mutatedOnsets.forEach(pos => {
        if (pos >= 0 && pos < stepCount) {
            mutatedPattern[pos] = true;
        }
    });
    
    // Calculate displacement metrics
    const displacement = calculateDisplacement(originalOnsets, mutatedOnsets, stepCount);
    
    return {
        pattern: [...originalPattern],
        mutated: mutatedPattern,
        originalOnsets: originalOnsets,
        mutatedOnsets: mutatedOnsets,
        mutationAmount: mutationAmount,
        displacement: displacement,
        mutationStyle: mutationStyle,
        description: `${mutationStyle} mutation (${Math.round(mutationAmount*100)}% displacement)`
    };
}

/**
 * Generate mutated onset positions
 */
function generateMutatedOnsets(originalOnsets, stepCount, mutationAmount, mutationStyle, options) {
    const mutatedOnsets = [];
    
    for (const originalPos of originalOnsets) {
        let newPos = originalPos;
        
        switch (mutationStyle) {
            case 'balanced':
                newPos = mutateBalanced(originalPos, stepCount, mutationAmount);
                break;
                
            case 'groove':
                newPos = mutateToGroove(originalPos, stepCount, mutationAmount);
                break;
                
            case 'syncopate':
                newPos = mutateSyncopate(originalPos, stepCount, mutationAmount);
                break;
                
            case 'straighten':
                newPos = mutateStraighten(originalPos, stepCount, mutationAmount);
                break;
                
            case 'swing':
                newPos = mutateSwing(originalPos, stepCount, mutationAmount);
                break;
                
            case 'shuffle':
                newPos = mutateShuffle(originalPos, stepCount, mutationAmount);
                break;
        }
        
        // Ensure position is within bounds and not duplicate
        newPos = Math.round(newPos) % stepCount;
        if (newPos < 0) newPos += stepCount;
        
        // Avoid duplicates at same position
        if (!mutatedOnsets.includes(newPos)) {
            mutatedOnsets.push(newPos);
        } else {
            // Find nearest available position
            const nearestPos = findNearestAvailable(newPos, mutatedOnsets, stepCount);
            if (nearestPos !== -1) {
                mutatedOnsets.push(nearestPos);
            }
        }
    }
    
    return mutatedOnsets.sort((a, b) => a - b);
}

/**
 * Balanced mutation - random displacement in both directions
 */
function mutateBalanced(position, stepCount, amount) {
    const maxDisplacement = Math.floor(stepCount * 0.25); // Max 25% of pattern length
    const displacement = (Math.random() - 0.5) * 2 * maxDisplacement * amount;
    return position + displacement;
}

/**
 * Groove mutation - bias toward groove positions
 */
function mutateToGroove(position, stepCount, amount) {
    const groovePositions = getGroovePositions(stepCount);
    const nearestGroove = findNearestGroovePosition(position, groovePositions);
    const displacement = (nearestGroove - position) * amount;
    return position + displacement;
}

/**
 * Syncopation mutation - move toward off-beat positions
 */
function mutateSyncopate(position, stepCount, amount) {
    const strongBeats = getStrongBeats(stepCount);
    
    // If on strong beat, move to nearby off-beat
    if (strongBeats.includes(position)) {
        const offBeatTargets = getNearbyOffBeats(position, stepCount);
        const target = offBeatTargets[Math.floor(Math.random() * offBeatTargets.length)];
        return position + (target - position) * amount;
    }
    
    // If already off-beat, add some randomness
    return mutateBalanced(position, stepCount, amount * 0.5);
}

/**
 * Straightening mutation - move toward strong beats
 */
function mutateStraighten(position, stepCount, amount) {
    const strongBeats = getStrongBeats(stepCount);
    const nearestStrong = strongBeats.reduce((nearest, beat) => {
        const currentDistance = Math.abs(position - beat);
        const nearestDistance = Math.abs(position - nearest);
        return currentDistance < nearestDistance ? beat : nearest;
    });
    
    const displacement = (nearestStrong - position) * amount;
    return position + displacement;
}

/**
 * Swing mutation - create swing feel
 */
function mutateSwing(position, stepCount, amount) {
    const eighth = stepCount / 8;
    const beatPosition = position % eighth;
    
    // Move off-beats later for swing feel
    if (beatPosition > eighth / 2) {
        const swingDelay = eighth * 0.15 * amount; // Typical swing ratio
        return position + swingDelay;
    }
    
    return position;
}

/**
 * Shuffle mutation - create shuffle rhythm
 */
function mutateShuffle(position, stepCount, amount) {
    const sixteenth = stepCount / 16;
    const beatPosition = position % (sixteenth * 2);
    
    // Shuffle the second sixteenth note of each eighth note pair
    if (beatPosition >= sixteenth && beatPosition < sixteenth * 2) {
        const shuffleDelay = sixteenth * 0.2 * amount;
        return position + shuffleDelay;
    }
    
    return position;
}

// Helper methods
function getOnsetPositions(pattern) {
    return pattern.map((step, index) => step ? index : -1)
                 .filter(index => index !== -1);
}

function getStrongBeats(stepCount) {
    const quarter = Math.floor(stepCount / 4);
    return [0, quarter, quarter * 2, quarter * 3].filter(pos => pos < stepCount);
}

function getGroovePositions(stepCount) {
    const positions = [];
    const sixteenth = stepCount / 16;
    
    // Add groove positions (1, 1+, 2, 2+, 3, 3+, 4, 4+)
    for (let i = 0; i < 16; i++) {
        const pos = Math.floor(i * sixteenth);
        if (pos < stepCount && !positions.includes(pos)) {
            positions.push(pos);
        }
    }
    
    return positions;
}

function findNearestGroovePosition(position, groovePositions) {
    return groovePositions.reduce((nearest, groove) => {
        const currentDistance = Math.abs(position - groove);
        const nearestDistance = Math.abs(position - nearest);
        return currentDistance < nearestDistance ? groove : nearest;
    });
}

function getNearbyOffBeats(position, stepCount) {
    const strongBeats = getStrongBeats(stepCount);
    const quarter = Math.floor(stepCount / 4);
    const eighth = Math.floor(stepCount / 8);
    
    // Return positions that are not on strong beats
    const offBeats = [];
    for (let i = -eighth; i <= eighth; i++) {
        const pos = (position + i + stepCount) % stepCount;
        if (!strongBeats.includes(pos) && pos !== position) {
            offBeats.push(pos);
        }
    }
    
    return offBeats;
}

function findNearestAvailable(targetPos, usedPositions, stepCount) {
    for (let distance = 1; distance < stepCount / 2; distance++) {
        const pos1 = (targetPos + distance) % stepCount;
        const pos2 = (targetPos - distance + stepCount) % stepCount;
        
        if (!usedPositions.includes(pos1)) return pos1;
        if (!usedPositions.includes(pos2)) return pos2;
    }
    return -1; // No available position found
}

function calculateDisplacement(originalOnsets, mutatedOnsets, stepCount) {
    if (originalOnsets.length !== mutatedOnsets.length) {
        return { average: 0, total: 0, normalized: 0 };
    }
    
    let totalDisplacement = 0;
    for (let i = 0; i < originalOnsets.length; i++) {
        const original = originalOnsets[i];
        const mutated = mutatedOnsets[i];
        
        // Calculate circular distance
        const directDistance = Math.abs(mutated - original);
        const wrapDistance = stepCount - directDistance;
        const displacement = Math.min(directDistance, wrapDistance);
        
        totalDisplacement += displacement;
    }
    
    const averageDisplacement = totalDisplacement / originalOnsets.length;
    const normalizedDisplacement = averageDisplacement / (stepCount / 4); // Normalize to quarter note
    
    return {
        total: totalDisplacement,
        average: averageDisplacement,
        normalized: Math.min(1, normalizedDisplacement)
    };
}

/**
 * Create a mutation sequence - interpolate between original and target
 */
