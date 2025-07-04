// Test current vs improved indispensability for prime patterns

// Current implementation (from pattern-analysis.js)
function calculatePositionIndispensability_Current(position, length) {
    if (position === 0) return 1; // Downbeat is most indispensable
    
    if (position === length - 1) {
        return 0.75; // High indispensability for pickup beat
    }
    
    // Current stratified approach
    const stratification = getPrimeFactors(length);
    let totalIndispensability = 0;
    
    for (let level = 0; level < stratification.length; level++) {
        const primeAtLevel = stratification[level];
        const levelSize = length / Math.pow(primeAtLevel, level + 1);
        
        if (levelSize > 0 && position % levelSize === 0) {
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

// Evanstein's hierarchical approach (based on SCAMP implementation)
function calculatePositionIndispensability_Evanstein(position, length) {
    if (position === 0) return 1; // Downbeat is most indispensable
    
    // Decompose length into additive meters (2s and 3s)
    const decomposition = decomposeToTwosAndThrees(length);
    
    // Calculate hierarchical indispensability
    const strata = [decomposition]; // Single stratum for now
    const backwardPriorities = getBackwardBeatPriorities(strata, length);
    const indispensabilityArray = generateIndispensabilityArray(backwardPriorities);
    
    return indispensabilityArray[position];
}

// Helper functions for Evanstein's approach
function decomposeToTwosAndThrees(n) {
    const result = [];
    
    // For prime numbers > 3, decompose as largest multiple of 2 or 3 + remainder
    if (isPrime(n) && n > 3) {
        // For 7: 4+3 (prefer 2s first, then 3s)
        // For 11: 6+3+2 or 9+2 (prefer larger groups of 3s)
        // For 13: 9+2+2 or 12+1 -> 9+4 -> 9+2+2
        
        let remaining = n;
        
        // Try to use 3s first for larger groups
        while (remaining >= 6) {
            result.push(3);
            remaining -= 3;
        }
        
        // Handle remainder
        if (remaining === 4) {
            result.push(2);
            result.push(2);
        } else if (remaining === 5) {
            result.push(2);
            result.push(3);
        } else if (remaining === 2) {
            result.push(2);
        } else if (remaining === 3) {
            result.push(3);
        }
    } else {
        // For composite numbers, use prime factorization
        const factors = getPrimeFactors(n);
        for (const factor of factors) {
            if (factor === 2 || factor === 3) {
                result.push(factor);
            } else {
                // Recursively decompose larger primes
                const subDecomp = decomposeToTwosAndThrees(factor);
                result.push(...subDecomp);
            }
        }
    }
    
    return result.reverse(); // Reverse to match Evanstein's convention
}

function getBackwardBeatPriorities(strata, totalLength) {
    const priorities = [];
    
    for (let pos = 0; pos < totalLength; pos++) {
        let priority = 0;
        
        // Calculate priority based on position alignment with hierarchical structure
        const decomp = strata[0]; // Single stratum for now
        let currentPos = pos;
        let level = 0;
        
        for (const groupSize of decomp) {
            const groupIndex = Math.floor(currentPos / groupSize);
            const posInGroup = currentPos % groupSize;
            
            if (posInGroup === 0) {
                // This position aligns with this hierarchical level
                priority += Math.pow(2, decomp.length - level - 1);
            }
            
            currentPos = groupIndex;
            level++;
        }
        
        priorities.push(priority);
    }
    
    return priorities;
}

function generateIndispensabilityArray(backwardPriorities) {
    const length = backwardPriorities.length;
    const maxPriority = Math.max(...backwardPriorities);
    
    return backwardPriorities.map(p => p / maxPriority);
}

// Utility functions
function isPrime(n) {
    if (n < 2) return false;
    for (let i = 2; i <= Math.sqrt(n); i++) {
        if (n % i === 0) return false;
    }
    return true;
}

function getPrimeFactors(n) {
    const factors = [];
    for (let i = 2; i <= n; i++) {
        while (n % i === 0) {
            factors.push(i);
            n /= i;
        }
    }
    return factors;
}

function greatestCommonDivisor(a, b) {
    return b === 0 ? a : greatestCommonDivisor(b, a % b);
}

// Test with 7-step pattern
console.log("=== Testing 7-step pattern ===");
console.log("Current approach:");
for (let i = 0; i < 7; i++) {
    const indisp = calculatePositionIndispensability_Current(i, 7);
    console.log(`Position ${i}: ${indisp.toFixed(3)}`);
}

console.log("\nEvanstein approach:");
for (let i = 0; i < 7; i++) {
    const indisp = calculatePositionIndispensability_Evanstein(i, 7);
    console.log(`Position ${i}: ${indisp.toFixed(3)}`);
}

console.log("\nDecomposition of 7:", decomposeToTwosAndThrees(7));
console.log("Decomposition of 11:", decomposeToTwosAndThrees(11));
console.log("Decomposition of 13:", decomposeToTwosAndThrees(13));