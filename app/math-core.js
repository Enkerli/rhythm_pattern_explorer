// === MODULE 1: MATH CORE ===
/**
 * Mathematical utility functions for rhythm pattern analysis
 * Provides fundamental mathematical operations needed throughout the application
 */
class MathUtils {
    /**
     * Calculate Greatest Common Divisor using Euclidean algorithm
     * Essential for rhythm analysis, polygon combinations, and meter relationships
     * 
     * @param {number} a - First number
     * @param {number} b - Second number
     * @returns {number} Greatest common divisor of a and b
     * 
     * @example
     * MathUtils.gcd(12, 8) // returns 4
     * MathUtils.gcd(15, 25) // returns 5
     * 
     * Mathematical Background:
     * The Euclidean algorithm is fundamental to many rhythm generation techniques,
     * particularly Euclidean rhythms where the GCD determines the pattern structure.
     */
    static gcd(a, b) {
        a = Math.abs(a);
        b = Math.abs(b);
        while (b !== 0) {
            [a, b] = [b, a % b];
        }
        return a;
    }
    
    /**
     * Calculate Least Common Multiple
     * Critical for polygon pattern combinations where different polygon cycles
     * must be synchronized to a common period
     * 
     * @param {number} a - First number
     * @param {number} b - Second number
     * @returns {number} Least common multiple of a and b, or 0 if either input is 0
     * 
     * @example
     * MathUtils.lcm(4, 6) // returns 12
     * MathUtils.lcm(3, 5) // returns 15 (for P(3,x)+P(5,y) combinations)
     * 
     * Musical Application:
     * When combining P(3,1) + P(5,0), the LCM(3,5) = 15 determines the
     * total steps needed to represent both polygon patterns together.
     */
    static lcm(a, b) {
        if (a === 0 || b === 0) return 0;
        return Math.abs(a * b) / this.gcd(a, b);
    }
    
    /**
     * Calculate LCM of multiple numbers
     * Used for complex polygon combinations like P(3,1)+P(5,0)+P(7,2)
     * 
     * @param {number[]} numbers - Array of numbers to find LCM for
     * @returns {number} LCM of all numbers, or 0 if all numbers are 0
     * @throws {Error} If input is not a non-empty array
     * 
     * @example
     * MathUtils.lcmArray([3, 5, 7]) // returns 105
     * MathUtils.lcmArray([4, 6, 8]) // returns 24
     * 
     * Algorithm:
     * Uses reduce() to iteratively calculate LCM of pairs, building up
     * to the final LCM of all numbers. Zero values are filtered out.
     */
    static lcmArray(numbers) {
        if (!Array.isArray(numbers) || numbers.length === 0) {
            throw new Error('Input must be a non-empty array of numbers');
        }
        const validNumbers = numbers.filter(n => n !== 0);
        if (validNumbers.length === 0) return 0;
        return validNumbers.reduce((lcm, num) => this.lcm(lcm, num), 1);
    }
    
    /**
     * Test if a number is prime
     * Used in Barlow indispensability calculations for meter analysis
     * 
     * @param {number} n - Number to test for primality
     * @returns {boolean} True if n is prime, false otherwise
     * 
     * @example
     * MathUtils.isPrime(7) // returns true
     * MathUtils.isPrime(12) // returns false
     * 
     * Musical Context:
     * Prime numbers play a crucial role in Clarence Barlow's indispensability theory.
     * The prime factorization of meter positions determines their rhythmic importance.
     * 
     * Algorithm:
     * Uses trial division with optimizations:
     * - Handle special cases (< 2, == 2, even numbers)
     * - Only test odd divisors up to √n
     */
    static isPrime(n) {
        if (n < 2) return false;
        if (n === 2) return true;
        if (n % 2 === 0) return false;
        for (let i = 3; i <= Math.sqrt(n); i += 2) {
            if (n % i === 0) return false;
        }
        return true;
    }
    
    /**
     * Test if two numbers are coprime (relatively prime)
     * Important for polygon balance analysis and pattern combinations
     * 
     * @param {number} a - First number
     * @param {number} b - Second number
     * @returns {boolean} True if gcd(a,b) = 1, false otherwise
     * 
     * @example
     * MathUtils.areCoprimes(9, 16) // returns true (gcd = 1)
     * MathUtils.areCoprimes(6, 9) // returns false (gcd = 3)
     * 
     * Musical Significance:
     * Coprime polygon combinations often produce interesting rhythmic patterns
     * with complex but mathematically elegant structures.
     */
    static areCoprimes(a, b) {
        return this.gcd(a, b) === 1;
    }
    
    /**
     * Get all prime factors of a number
     * Essential for Barlow indispensability calculations
     * 
     * @param {number} n - Number to factorize
     * @returns {number[]} Array of prime factors (with repetition)
     * 
     * @example
     * MathUtils.getPrimeFactors(12) // returns [2, 2, 3]
     * MathUtils.getPrimeFactors(15) // returns [3, 5]
     * 
     * Barlow Context:
     * The indispensability of a metric position is calculated as:
     * 1 / (product of prime factors of (meter_length / gcd(position, meter_length)))
     * 
     * Algorithm:
     * Trial division starting from 2, collecting all prime factors
     * with their multiplicities preserved.
     */
    static getPrimeFactors(n) {
        const factors = [];
        for (let i = 2; i <= n; i++) {
            while (n % i === 0) {
                factors.push(i);
                n /= i;
            }
        }
        return factors;
    }
    
    /**
     * Generate a random integer using normal (bell curve) distribution
     * Used for random pattern generation to avoid extreme values
     * 
     * @param {number} min - Minimum value (inclusive)
     * @param {number} max - Maximum value (inclusive)
     * @param {number} skew - Skewness factor (1 = normal, >1 skews toward min, <1 skews toward max)
     * @returns {number} Random integer between min and max with bell curve distribution
     * 
     * @example
     * MathUtils.randomBellCurve(0, 8) // More likely to return 3, 4, 5 than 0, 1, 7, 8
     * MathUtils.randomBellCurve(0, 16, 0.8) // Slightly skewed toward higher values
     * 
     * Musical Application:
     * For R(r,8), this generates onset counts that favor moderate densities,
     * making more musically interesting patterns by avoiding empty or completely full patterns.
     * 
     * Algorithm:
     * Uses Box-Muller transformation to generate normal distribution,
     * then maps to the desired range and applies optional skewness.
     */
    static randomBellCurve(min, max, skew = 1) {
        // Validate inputs
        if (min > max) [min, max] = [max, min];
        if (min === max) return min;
        
        // Generate normal distribution using Box-Muller transformation
        let u1 = 0, u2 = 0;
        while (u1 === 0) u1 = Math.random(); // Converting [0,1) to (0,1)
        while (u2 === 0) u2 = Math.random();
        
        const z0 = Math.sqrt(-2.0 * Math.log(u1)) * Math.cos(2 * Math.PI * u2);
        
        // Map from standard normal (mean=0, std=1) to our range
        // Use 3 standard deviations to cover ~99.7% of distribution
        const mean = (min + max) / 2;
        const stdDev = (max - min) / 6; // 3 standard deviations span the range
        
        let value = mean + z0 * stdDev;
        
        // Apply skewness if specified
        if (skew !== 1) {
            const normalized = (value - min) / (max - min);
            const skewed = Math.pow(normalized, skew);
            value = min + skewed * (max - min);
        }
        
        // Clamp to range and round to integer
        value = Math.max(min, Math.min(max, value));
        return Math.round(value);
    }
    
    /**
     * Generate optimal random onset count for rhythmic patterns
     * Specifically tuned for musical applications with bell curve distribution
     * 
     * @param {number} steps - Total number of steps in the pattern
     * @param {Object} options - Configuration options
     * @returns {number} Random onset count optimized for musical interest
     * 
     * @example
     * MathUtils.randomOnsetCount(8) // Likely returns 3, 4, or 5
     * MathUtils.randomOnsetCount(16, { density: 'sparse' }) // Favors lower onset counts
     * MathUtils.randomOnsetCount(8, { density: 'dense' }) // Favors higher onset counts
     * 
     * Musical Tuning:
     * - Avoids completely empty (0 onsets) or full (all onsets) patterns
     * - Favors moderate densities that create interesting rhythmic relationships
     * - Provides options for sparse, normal, or dense bias
     */
    static randomOnsetCount(steps, options = {}) {
        const { 
            density = 'normal',  // 'sparse', 'normal', 'dense'
            avoidExtremes = true 
        } = options;
        
        // Define range based on whether to avoid extremes
        const minOnsets = avoidExtremes ? 1 : 0;
        const maxOnsets = avoidExtremes ? Math.max(1, steps - 1) : steps;
        
        // Adjust skewness based on density preference
        let skew = 1; // Normal distribution
        switch (density) {
            case 'sparse':
                skew = 1.5; // Skew toward lower values
                break;
            case 'dense':
                skew = 0.67; // Skew toward higher values
                break;
            default:
                skew = 1; // Normal bell curve
        }
        
        return this.randomBellCurve(minOnsets, maxOnsets, skew);
    }
}

// Export to global scope for browser compatibility
if (typeof window !== 'undefined') {
    window.MathUtils = MathUtils;
}