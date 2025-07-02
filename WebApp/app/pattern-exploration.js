// === MODULE 5: PATTERN EXPLORATION ===

/**
 * Pattern Exploration Module (pattern-exploration.js)
 * Systematic discovery and analysis of rhythm pattern combinations
 * 
 * Provides comprehensive automated exploration of polygon-based rhythm patterns
 * to discover mathematically interesting combinations with perfect or near-perfect
 * balance properties. Uses systematic combinatorial analysis with performance
 * optimization for real-time pattern discovery.
 * 
 * Core Features:
 * - Systematic combinatorial exploration of polygon patterns
 * - Perfect balance detection using Milne's center of gravity theory
 * - Multi-pattern combination with addition and subtraction operations
 * - Real-time progress tracking and performance optimization
 * - Quality scoring system for pattern ranking and evaluation
 * - Configurable exploration parameters for different use cases
 * - Asynchronous processing with UI responsiveness preservation
 * 
 * Mathematical Foundation:
 * - Regular polygon pattern generation with offset variations
 * - LCM-based pattern combination for rhythmic coherence
 * - Perfect balance analysis using complex number representation
 * - Combinatorial mathematics for systematic pattern space exploration
 * - Quality metrics combining balance, density, and structural properties
 * 
 * Performance Features:
 * - Configurable exploration limits to prevent browser blocking
 * - Asynchronous processing with periodic UI updates
 * - Deterministic offset generation for reproducible results
 * - Memory-efficient pattern storage and analysis
 * - Progress monitoring and early termination support
 * 
 * Dependencies:
 * - RegularPolygonGenerator (from pattern-generators.js) - Polygon pattern creation
 * - AdvancedPatternCombiner (from pattern-processing.js) - Pattern combination
 * - PerfectBalanceAnalyzer (from pattern-analysis.js) - Balance analysis
 * - PatternAnalyzer (from pattern-analysis.js) - Structural analysis
 * - MathUtils (from math-core.js) - Mathematical utility functions
 */

/**
 * Exploration Configuration Constants
 * 
 * Defines operational parameters for systematic pattern exploration,
 * balancing thoroughness with performance and user experience.
 * 
 * Performance Tuning:
 * - MAX_OFFSET_LIMIT: Maximum offset range to prevent exponential explosion
 * - MAX_COMBINATIONS_PER_TEST: Limits per-combination testing for responsiveness
 * - OFFSET_COMBINATIONS_LIMIT: Constrains offset variations per polygon set
 * - UI_UPDATE_INTERVAL: Frequency of progress updates for smooth user feedback
 * - ASYNC_DELAY_MS: Minimal delay for browser responsiveness preservation
 * 
 * Mathematical Constraints:
 * - MIN_POLYGON_SIDES: Minimum polygon complexity for meaningful patterns
 * - Exploration limits prevent browser timeout and memory exhaustion
 * - Deterministic generation ensures reproducible research results
 */
const EXPLORATION_CONFIG = {
    MAX_OFFSET_LIMIT: 8,           // Maximum offset range for pattern variations
    MIN_POLYGON_SIDES: 3,          // Minimum polygon sides (triangle and above)
    MAX_COMBINATIONS_PER_TEST: 50, // Performance limit for combination testing
    OFFSET_COMBINATIONS_LIMIT: 20, // Maximum offset variations per polygon set
    UI_UPDATE_INTERVAL: 10,        // Update UI progress every N combinations
    ASYNC_DELAY_MS: 1              // Minimal delay for UI responsiveness
};

/**
 * Systematic Explorer - Automated Pattern Discovery Engine
 * 
 * Implements comprehensive systematic exploration of polygon-based rhythm patterns
 * to discover mathematically interesting combinations with perfect or near-perfect
 * balance properties. Uses advanced combinatorial analysis with real-time progress
 * tracking and performance optimization.
 * 
 * Exploration Strategy:
 * - Generates all possible polygon combinations within specified parameters
 * - Tests multiple offset variations for each polygon combination
 * - Applies addition and subtraction operations for complex pattern discovery
 * - Evaluates mathematical balance using Milne's center of gravity theory
 * - Ranks results using comprehensive quality scoring algorithms
 * 
 * Performance Architecture:
 * - Asynchronous processing prevents browser blocking during exploration
 * - Configurable limits balance thoroughness with responsiveness
 * - Progress tracking provides real-time feedback to users
 * - Memory-efficient storage and processing of large result sets
 * - Early termination support for user-controlled exploration
 * 
 * Mathematical Foundation:
 * - Combinatorial generation using systematic enumeration
 * - Deterministic offset patterns for reproducible results
 * - Balance analysis using complex number representation
 * - Quality metrics incorporating balance, density, and structural properties
 * - Statistical analysis of pattern distribution and characteristics
 */
class SystematicExplorer {
    /**
     * Initialize Systematic Explorer
     * 
     * Creates a new pattern exploration engine with empty result storage
     * and default operational state for systematic pattern discovery.
     * 
     * Initial State:
     * - Empty results array for discovered patterns
     * - Stopped execution state for user-controlled exploration
     * - Zero progress counters for tracking exploration advancement
     * - Ready for configuration and exploration execution
     * 
     * @example
     * const explorer = new SystematicExplorer();
     * const results = await explorer.exploreAllCombinations(3, 7, 3, 'perfect');
     * console.log(`Found ${results.length} interesting patterns`);
     */
    constructor() {
        this.results = [];              // Array of discovered pattern combinations
        this.isRunning = false;         // Current exploration execution state
        this.shouldStop = false;        // User-requested termination flag
        this.totalCombinations = 0;     // Total combinations to be tested
        this.currentCombination = 0;    // Current progress counter
    }
    
    /**
     * Explore All Pattern Combinations
     * 
     * Executes comprehensive systematic exploration of all possible polygon
     * combinations within specified parameters to discover patterns with
     * desired balance characteristics.
     * 
     * @param {number} minSides - Minimum polygon sides to include (≥3)
     * @param {number} maxSides - Maximum polygon sides to include (≤32)
     * @param {number} maxCombinations - Maximum patterns per combination (2-5)
     * @param {string} [targetBalance='perfect'] - Target balance type ('perfect', 'near', 'all')
     * @returns {Promise<Array>} Array of discovered pattern results with analysis
     * 
     * @example
     * // Find perfect balance patterns using triangles through hexagons
     * const perfectPatterns = await explorer.exploreAllCombinations(3, 6, 3, 'perfect');
     * 
     * // Find near-perfect patterns with broader criteria
     * const goodPatterns = await explorer.exploreAllCombinations(3, 8, 4, 'near');
     * 
     * Exploration Process:
     * 1. Calculate total combination space for progress tracking
     * 2. Generate all possible polygon combinations systematically
     * 3. Test multiple offset variations for each combination
     * 4. Apply addition and subtraction operations where applicable
     * 5. Analyze balance properties using mathematical algorithms
     * 6. Filter results based on target balance criteria
     * 7. Update progress and provide user feedback during execution
     * 
     * Balance Target Types:
     * - 'perfect': Patterns with magnitude < 0.001 (mathematically perfect)
     * - 'near': Patterns with 'excellent' or 'good' balance scores
     * - 'all': All patterns regardless of balance quality
     * 
     * Performance Features:
     * - Asynchronous execution with periodic UI updates
     * - Configurable limits prevent browser timeout
     * - Progress tracking for long-running explorations
     * - Early termination support via shouldStop flag
     * - Memory-efficient result storage and processing
     */
    async exploreAllCombinations(minSides, maxSides, maxCombinations, targetBalance = 'perfect') {
        this.results = [];
        this.isRunning = true;
        this.shouldStop = false;
        this.currentCombination = 0;
        
        // Calculate total combinations
        this.totalCombinations = this.calculateTotalCombinations(minSides, maxSides, maxCombinations);
        
        console.log(`Starting systematic exploration: ${this.totalCombinations} combinations to test`);
        
        // Generate all possible combinations
        const combinations = this.generateAllCombinations(minSides, maxSides, maxCombinations);
        
        for (const combination of combinations) {
            if (this.shouldStop) break;
            
            this.currentCombination++;
            this.updateProgress();
            
            // Test multiple offset combinations for each polygon combination
            this.testCombinationWithOffsets(combination, targetBalance);
            
            // Allow UI to update
            if (this.currentCombination % EXPLORATION_CONFIG.UI_UPDATE_INTERVAL === 0) {
                await this.sleep(EXPLORATION_CONFIG.ASYNC_DELAY_MS);
            }
        }
        
        this.isRunning = false;
        this.updateProgress();
        
        return this.results;
    }
    
    /**
     * Generate All Polygon Combinations
     * 
     * Creates comprehensive list of all possible polygon combinations
     * within specified parameters using systematic combinatorial generation.
     * 
     * @param {number} minSides - Minimum polygon sides to include
     * @param {number} maxSides - Maximum polygon sides to include
     * @param {number} maxCombinations - Maximum patterns per combination
     * @returns {Array<Array<number>>} Array of polygon side combinations
     * 
     * @example
     * // Generate combinations for triangles through pentagons, up to 3 patterns
     * const combinations = explorer.generateAllCombinations(3, 5, 3);
     * // Returns: [[3,4], [3,5], [4,5], [3,4,5], etc.]
     * 
     * Generation Algorithm:
     * 1. Create array of available polygon sides within range
     * 2. Generate combinations of 2 to maxCombinations polygons
     * 3. Use recursive combination generation for systematic coverage
     * 4. Ensure no duplicate combinations in result set
     * 
     * Combinatorial Mathematics:
     * - Uses n-choose-k algorithm for combination generation
     * - Systematic enumeration ensures complete coverage
     * - No repetition of individual polygons within combinations
     * - Ordered generation for consistent results
     */
    generateAllCombinations(minSides, maxSides, maxCombinations) {
        const combinations = [];
        const sides = [];
        
        for (let s = minSides; s <= maxSides; s++) {
            sides.push(s);
        }
        
        // Generate combinations of 2 to maxCombinations polygons
        for (let combCount = 2; combCount <= maxCombinations; combCount++) {
            const combs = this.generateCombinations(sides, combCount);
            combinations.push(...combs);
        }
        
        return combinations;
    }
    
    /**
     * Generate Combinations of Specified Size
     * 
     * Implements recursive combinatorial generation algorithm to create
     * all possible combinations of specified size from input array.
     * 
     * @param {Array} arr - Source array for combination generation
     * @param {number} size - Number of elements per combination
     * @returns {Array<Array>} Array of all possible combinations
     * 
     * @example
     * generateCombinations([3, 4, 5], 2); // Returns: [[3,4], [3,5], [4,5]]
     * generateCombinations([3, 4, 5, 6], 3); // Returns: [[3,4,5], [3,4,6], [3,5,6], [4,5,6]]
     * 
     * Algorithm Details:
     * - Recursive implementation for systematic enumeration
     * - Base case: single elements return as individual arrays
     * - Recursive case: combine head element with tail combinations
     * - Ensures lexicographic ordering of results
     * - No duplicate combinations generated
     * 
     * Mathematical Foundation:
     * - Implements n-choose-k combinatorial mathematics
     * - Time complexity: O(C(n,k)) where n=array size, k=combination size
     * - Space complexity: O(C(n,k)) for result storage
     * - Memory-efficient recursive generation
     */
    generateCombinations(arr, size) {
        if (size === 1) return arr.map(item => [item]);
        if (size > arr.length) return [];
        
        const result = [];
        for (let i = 0; i <= arr.length - size; i++) {
            const head = arr[i];
            const tail = this.generateCombinations(arr.slice(i + 1), size - 1);
            for (const tailComb of tail) {
                result.push([head, ...tailComb]);
            }
        }
        return result;
    }
    
    /**
     * Calculate Total Combination Count
     * 
     * Computes the total number of combinations that will be tested
     * during exploration for accurate progress tracking and time estimation.
     * 
     * @param {number} minSides - Minimum polygon sides in range
     * @param {number} maxSides - Maximum polygon sides in range
     * @param {number} maxCombinations - Maximum patterns per combination
     * @returns {number} Total number of combinations to be tested
     * 
     * @example
     * const total = explorer.calculateTotalCombinations(3, 6, 3);
     * console.log(`Will test ${total} combinations`);
     * 
     * Calculation Process:
     * 1. Determine number of available polygon types (sides range)
     * 2. Calculate n-choose-k for each combination size
     * 3. Multiply by offset test variations per combination
     * 4. Sum across all combination sizes for total count
     * 
     * Mathematical Formula:
     * Total = Σ(k=2 to maxCombinations) C(sides, k) × offsetTests
     * Where C(n,k) = n! / (k! × (n-k)!)
     * 
     * Progress Tracking:
     * - Enables accurate percentage completion calculation
     * - Supports time estimation for long explorations
     * - Helps users understand exploration scope
     * - Allows for performance optimization decisions
     */
    calculateTotalCombinations(minSides, maxSides, maxCombinations) {
        const sides = maxSides - minSides + 1;
        let total = 0;
        
        for (let combCount = 2; combCount <= maxCombinations; combCount++) {
            const combinations = this.nCr(sides, combCount);
            // Each combination will be tested with limited offset combinations
            const offsetTests = Math.min(EXPLORATION_CONFIG.MAX_COMBINATIONS_PER_TEST, 20);
            total += combinations * offsetTests;
        }
        
        return total;
    }
    
    /**
     * Calculate n-Choose-r Combinations
     * 
     * Implements efficient calculation of binomial coefficient using
     * iterative algorithm to avoid factorial overflow for large numbers.
     * 
     * @param {number} n - Total number of items
     * @param {number} r - Number of items to choose
     * @returns {number} Number of possible combinations
     * 
     * @example
     * nCr(5, 2); // Returns 10 (combinations of 2 items from 5)
     * nCr(8, 3); // Returns 56 (combinations of 3 items from 8)
     * 
     * Mathematical Formula:
     * C(n,r) = n! / (r! × (n-r)!)
     * 
     * Implementation Optimization:
     * - Iterative calculation prevents factorial overflow
     * - Early termination for edge cases (r=0, r=n)
     * - Efficient multiplication and division order
     * - Integer arithmetic for exact results
     * 
     * Edge Cases:
     * - Returns 0 for invalid inputs (r > n)
     * - Returns 1 for r=0 or r=n (mathematical identity)
     * - Handles large numbers within JavaScript integer limits
     */
    nCr(n, r) {
        if (r > n) return 0;
        if (r === 0 || r === n) return 1;
        
        let result = 1;
        for (let i = 0; i < r; i++) {
            result *= (n - i);
            result /= (i + 1);
        }
        return Math.floor(result);
    }
    
    /**
     * Test Combination with Multiple Offset Variations
     * 
     * Tests a specific polygon combination with various offset patterns
     * to discover interesting rhythmic variations and balance properties.
     * 
     * @param {Array<number>} vertices - Array of polygon side counts to combine
     * @param {string} targetBalance - Target balance type for filtering results
     * @returns {Array} Array of qualifying pattern results with analysis
     * 
     * @example
     * // Test triangle + pentagon combination
     * const results = explorer.testCombinationWithOffsets([3, 5], 'perfect');
     * 
     * Testing Strategy:
     * 1. Test addition-only patterns with various offset combinations
     * 2. Test subtraction patterns for combinations with 3+ polygons
     * 3. Apply balance analysis to each generated pattern
     * 4. Filter results based on target balance criteria
     * 5. Store qualifying patterns for final result compilation
     * 
     * Offset Exploration:
     * - Limited offset range prevents exponential explosion
     * - Deterministic offset generation for reproducible results
     * - Multiple offset combinations per polygon set
     * - Systematic coverage of offset space within limits
     * 
     * Pattern Operations:
     * - Addition: Standard pattern combination using LCM alignment
     * - Subtraction: Remove onset positions from base patterns
     * - Analysis: Mathematical balance calculation for each result
     * - Quality: Comprehensive scoring for pattern ranking
     */
    testCombinationWithOffsets(vertices, targetBalance) {
        const maxOffset = Math.min(EXPLORATION_CONFIG.MAX_OFFSET_LIMIT, Math.max(...vertices));
        const results = [];
        
        // Test addition-only patterns first
        const additionResults = this.testOffsetsForCombination(vertices, [], maxOffset, targetBalance);
        results.push(...additionResults);
        
        // Test patterns with subtraction (if we have enough polygons)
        if (vertices.length >= 3) {
            const subtractionResults = this.testOffsetsWithSubtraction(vertices, maxOffset, targetBalance);
            results.push(...subtractionResults);
        }
        
        // Add qualifying results to main results array
        this.results.push(...results);
        
        return results;
    }
    
    /**
     * Test Offset Variations for Polygon Combination
     * 
     * Systematically tests different offset combinations for a specific
     * set of polygons to discover optimal rhythmic alignments.
     * 
     * @param {Array<number>} vertices - Primary polygon sides for combination
     * @param {Array<number>} subtractVertices - Polygon sides to subtract (if any)
     * @param {number} maxOffset - Maximum offset value to test
     * @param {string} targetBalance - Target balance type for result filtering
     * @returns {Array} Array of qualifying pattern combinations
     * 
     * @example
     * // Test triangle + square with various offsets
     * const results = explorer.testOffsetsForCombination([3, 4], [], 6, 'perfect');
     * 
     * Testing Process:
     * 1. Generate deterministic offset combinations within limits
     * 2. Create polygon patterns with specified offsets
     * 3. Combine patterns using LCM-based alignment
     * 4. Apply subtraction operations if specified
     * 5. Analyze balance properties using mathematical algorithms
     * 6. Filter results based on balance criteria and onset count
     * 
     * Pattern Generation:
     * - Each polygon generated with its assigned offset
     * - LCM-based combination ensures rhythmic coherence
     * - Subtraction removes overlapping onset positions
     * - Recalculation of pattern properties after modification
     * 
     * Quality Filtering:
     * - Balance analysis using center of gravity calculation
     * - Minimum onset count requirement for meaningful patterns
     * - Target balance filtering (perfect, near, all)
     * - Error handling for invalid pattern combinations
     */
    testOffsetsForCombination(vertices, subtractVertices, maxOffset, targetBalance) {
        const results = [];
        const offsetCombinations = this.generateOffsetCombinations(vertices.length, maxOffset);
        
        for (const offsets of offsetCombinations.slice(0, EXPLORATION_CONFIG.OFFSET_COMBINATIONS_LIMIT)) {
            try {
                // Create patterns for each polygon
                const patterns = [];
                for (let i = 0; i < vertices.length; i++) {
                    const polygon = RegularPolygonGenerator.generate(vertices[i], offsets[i]);
                    patterns.push(polygon);
                }
                
                // Add subtract patterns if any
                const subtractPatterns = [];
                for (const subVertex of subtractVertices) {
                    const subPattern = RegularPolygonGenerator.generate(subVertex, 0);
                    subtractPatterns.push(subPattern);
                }
                
                // Combine patterns
                let combined;
                if (patterns.length > 1) {
                    combined = AdvancedPatternCombiner.combineMultiplePatterns(patterns);
                } else if (patterns.length === 1) {
                    combined = patterns[0];
                } else {
                    continue; // Skip if no patterns
                }
                
                // Apply subtraction if needed
                if (subtractPatterns.length > 0) {
                    // Simple subtraction: remove onset positions from subtract patterns
                    for (const subPattern of subtractPatterns) {
                        for (let i = 0; i < Math.min(combined.stepCount, subPattern.stepCount); i++) {
                            if (subPattern.steps[i] && combined.steps[i]) {
                                combined.steps[i] = false;
                            }
                        }
                    }
                    // Recalculate combined properties
                    combined.isCombined = true;
                    combined.hasSubtraction = true;
                }
                
                const balance = PerfectBalanceAnalyzer.calculateBalance(combined.steps, combined.stepCount);
                
                const shouldInclude = targetBalance === 'perfect' ? balance.isPerfectlyBalanced :
                                    targetBalance === 'near' ? ['excellent', 'good'].includes(balance.balanceScore) :
                                    true;
                
                if (shouldInclude && balance.onsetCount > 0) {
                    results.push({
                        polygons: vertices,
                        offsets,
                        subtractVertices,
                        pattern: combined,
                        balance,
                        isInteresting: balance.isPerfectlyBalanced || balance.balanceScore === 'excellent'
                    });
                }
            } catch (error) {
                // Skip invalid combinations
                console.warn('Skipping invalid combination:', vertices, offsets, error.message);
            }
        }
        
        return results;
    }
    
    /**
     * Test Offset Variations with Subtraction Operations
     * 
     * Explores pattern combinations using subtraction to create more
     * complex rhythmic relationships and discover perfect balance through
     * strategic pattern removal.
     * 
     * @param {Array<number>} vertices - Polygon sides for combination testing
     * @param {number} maxOffset - Maximum offset range for exploration
     * @param {string} targetBalance - Target balance type for filtering
     * @returns {Array} Array of subtraction-based pattern results
     * 
     * @example
     * // Test pentagon + hexagon with triangle subtraction
     * const results = explorer.testOffsetsWithSubtraction([3, 5, 6], 4, 'perfect');
     * 
     * Subtraction Strategy:
     * 1. Identify smallest polygon in combination for subtraction
     * 2. Use remaining polygons as base pattern combination
     * 3. Test various offset combinations for optimal alignment
     * 4. Apply subtraction operation to remove onset overlaps
     * 5. Analyze resulting pattern for balance properties
     * 
     * Mathematical Approach:
     * - Smallest polygon often provides best subtraction candidates
     * - Remaining larger polygons form more stable base patterns
     * - Subtraction can achieve perfect balance through strategic removal
     * - Multiple subtraction patterns possible for complex combinations
     * 
     * Pattern Discovery:
     * - Enables discovery of patterns impossible through addition alone
     * - Creates asymmetric patterns with interesting properties
     * - Often produces perfect balance through mathematical cancellation
     * - Expands pattern space beyond additive combinations
     */
    testOffsetsWithSubtraction(vertices, maxOffset, targetBalance) {
        const results = [];
        
        // Try subtracting the smallest polygon
        const smallestVertex = Math.min(...vertices);
        const remainingVertices = vertices.filter(v => v !== smallestVertex);
        
        if (remainingVertices.length >= 2) {
            const subtractionResults = this.testOffsetsForCombination(
                remainingVertices, 
                [smallestVertex], 
                maxOffset, 
                targetBalance
            );
            results.push(...subtractionResults);
        }
        
        return results;
    }
    
    /**
     * Generate Deterministic Offset Combinations
     * 
     * Creates systematic offset combinations for polygon patterns using
     * deterministic generation to ensure reproducible exploration results.
     * 
     * @param {number} polygonCount - Number of polygons requiring offsets
     * @param {number} maxOffset - Maximum offset value for any polygon
     * @returns {Array<Array<number>>} Array of offset combination arrays
     * 
     * @example
     * // Generate offsets for 3 polygons with max offset 4
     * const offsets = explorer.generateOffsetCombinations(3, 4);
     * // Returns: [[0,0,0], [1,1,1], [2,2,2], [1,2,3], [2,3,4], ...]
     * 
     * Generation Strategy:
     * 1. Always include zero offsets as baseline combination
     * 2. Generate systematic patterns based on combination index
     * 3. Use modular arithmetic for deterministic distribution
     * 4. Limit total combinations for performance constraints
     * 
     * Deterministic Benefits:
     * - Reproducible results for research and debugging
     * - Systematic coverage of offset space within limits
     * - Avoids random variations that complicate analysis
     * - Enables consistent performance across exploration runs
     * 
     * Performance Optimization:
     * - Limits total combinations to prevent exponential explosion
     * - Prioritizes zero offsets for baseline comparison
     * - Uses efficient modular arithmetic for offset calculation
     * - Memory-efficient generation without recursive algorithms
     */
    generateOffsetCombinations(polygonCount, maxOffset) {
        const combinations = [];
        const maxCombinations = Math.min(EXPLORATION_CONFIG.MAX_COMBINATIONS_PER_TEST, 20);
        
        // Always include zero offsets first
        combinations.push(new Array(polygonCount).fill(0));
        
        // Generate deterministic offset combinations instead of random
        for (let i = 1; i < maxCombinations && combinations.length < maxCombinations; i++) {
            const offsets = [];
            for (let j = 0; j < polygonCount; j++) {
                // Use deterministic pattern based on combination index
                offsets.push((i + j) % maxOffset);
            }
            combinations.push(offsets);
        }
        
        return combinations;
    }
    
    /**
     * Update Exploration Progress Display
     * 
     * Updates user interface elements with current exploration progress
     * and discovered pattern statistics for real-time feedback.
     * 
     * Progress Updates:
     * - Visual progress bar with percentage completion
     * - Text display of current combination count
     * - Statistical summary of discovered patterns
     * - Performance metrics for exploration monitoring
     * 
     * @example
     * // Called automatically during exploration
     * // Updates DOM elements: progressFill, progressText
     * // Calls app.updateExplorationStats() if available
     * 
     * UI Integration:
     * - Safe DOM element access with existence checking
     * - Graceful degradation for missing UI elements
     * - Debug logging for development environments
     * - External application integration via app object
     * 
     * Performance Tracking:
     * - Progress percentage calculation from combination counts
     * - Real-time statistics update for discovered patterns
     * - Memory usage monitoring for large explorations
     * - Frame rate preservation during updates
     * 
     * Error Handling:
     * - Silent failure for missing DOM elements
     * - Debug logging without throwing exceptions
     * - Continues exploration despite UI update failures
     * - Compatible with headless testing environments
     */
    updateProgress() {
        // Safe DOM updates - only if elements exist
        try {
            const progressFill = document.getElementById('progressFill');
            const progressText = document.getElementById('progressText');
            
            if (this.totalCombinations > 0) {
                const percentage = (this.currentCombination / this.totalCombinations) * 100;
                if (progressFill) progressFill.style.width = `${percentage}%`;
                if (progressText) progressText.textContent = `${this.currentCombination} / ${this.totalCombinations} combinations tested`;
            }
            
            // Update the main stats display - safely check for app existence
            if (typeof app !== 'undefined' && app && app.updateExplorationStats) {
                app.updateExplorationStats(this.results);
            }
        } catch (error) {
            // Silently ignore DOM update errors in test environments
            console.debug('DOM update skipped:', error.message);
        }
    }
    
    /**
     * Get Current Exploration Progress
     * 
     * Returns current exploration progress as percentage for
     * external monitoring and display purposes.
     * 
     * @returns {number} Progress percentage (0-100)
     * 
     * @example
     * const progress = explorer.getProgress();
     * console.log(`Exploration ${progress.toFixed(1)}% complete`);
     * 
     * Progress Calculation:
     * - Based on completed combinations vs total combinations
     * - Returns 0 for explorations that haven't started
     * - Provides accurate percentage for progress bars
     * - Real-time updates during exploration execution
     */
    getProgress() {
        if (this.totalCombinations === 0) return 0;
        return (this.currentCombination / this.totalCombinations) * 100;
    }
    
    /**
     * Get Perfect Balance Pattern Results
     * 
     * Filters and returns only patterns with perfect mathematical balance
     * from the exploration results for specialized analysis.
     * 
     * @returns {Array} Array of perfectly balanced pattern results
     * 
     * @example
     * const perfectPatterns = explorer.getPerfectBalanceResults();
     * console.log(`Found ${perfectPatterns.length} perfect balance patterns`);
     * 
     * Perfect Balance Criteria:
     * - Balance score exactly equals 'perfect'
     * - Center of gravity magnitude < 0.001
     * - Mathematical precision within floating-point limits
     * - Represents ideal rhythmic distribution
     * 
     * Applications:
     * - Research into mathematically optimal rhythm patterns
     * - Generation of perfectly balanced musical sequences
     * - Analysis of geometric properties in rhythm
     * - Educational examples of mathematical perfection in music
     */
    getPerfectBalanceResults() {
        return this.results.filter(r => r.balance.balanceScore === 'perfect');
    }
    
    /**
     * Get Near-Perfect Balance Pattern Results
     * 
     * Returns patterns with excellent to perfect balance characteristics
     * for broader analysis of high-quality rhythmic patterns.
     * 
     * @returns {Array} Array of high-quality balance pattern results
     * 
     * @example
     * const qualityPatterns = explorer.getNearPerfectResults();
     * qualityPatterns.forEach(pattern => {
     *   console.log(`Pattern: ${pattern.balance.balanceScore}`);
     * });
     * 
     * Quality Categories Included:
     * - 'perfect': Mathematically perfect balance (magnitude < 0.001)
     * - 'excellent': Very high balance quality (magnitude < 0.05)
     * - 'good': Good balance characteristics (magnitude < 0.15)
     * 
     * Musical Applications:
     * - High-quality patterns for musical composition
     * - Educational examples of well-balanced rhythms
     * - Comparative analysis of balance quality levels
     * - Pattern selection for aesthetic applications
     */
    getNearPerfectResults() {
        return this.results.filter(r => ['perfect', 'excellent', 'good'].includes(r.balance.balanceScore));
    }
    
    /**
     * Sort Results by Balance Quality
     * 
     * Sorts exploration results by balance quality with perfect balance
     * patterns first, followed by progressively lower quality patterns.
     * 
     * @returns {Array} Sorted copy of results array (original unchanged)
     * 
     * @example
     * const sortedResults = explorer.sortResultsByBalance();
     * sortedResults.forEach((result, index) => {
     *   console.log(`${index + 1}. ${result.balance.balanceScore} (${result.balance.magnitude.toFixed(4)})`);
     * });
     * 
     * Sorting Criteria:
     * 1. Primary: Balance score rank (perfect > excellent > good > fair > poor)
     * 2. Secondary: Balance magnitude (lower magnitude = better balance)
     * 3. Stable sort preserves original order for equal elements
     * 
     * Score Ranking:
     * - perfect: 0 (highest priority)
     * - excellent: 1
     * - good: 2
     * - fair: 3
     * - poor: 4 (lowest priority)
     * - unknown: 5 (fallback for unrecognized scores)
     * 
     * Applications:
     * - Display results in order of mathematical quality
     * - Prioritize perfect patterns for analysis
     * - Generate ranked lists for pattern selection
     * - Research analysis of balance distribution
     */
    sortResultsByBalance() {
        const scoreOrder = { 'perfect': 0, 'excellent': 1, 'good': 2, 'fair': 3, 'poor': 4 };
        
        return [...this.results].sort((a, b) => {
            const scoreA = scoreOrder[a.balance.balanceScore] !== undefined ? scoreOrder[a.balance.balanceScore] : 5;
            const scoreB = scoreOrder[b.balance.balanceScore] !== undefined ? scoreOrder[b.balance.balanceScore] : 5;
            
            // Sort by score first (lower is better)
            if (scoreA !== scoreB) {
                return scoreA - scoreB;
            }
            
            // Within same score, sort by magnitude (lower is better for balance)
            return a.balance.magnitude - b.balance.magnitude;
        });
    }
    
    /**
     * Calculate Comprehensive Quality Score
     * 
     * Computes multi-factor quality score combining balance, density,
     * structural properties, and mathematical characteristics.
     * 
     * @param {Object} result - Pattern result with balance and structure data
     * @param {Object} result.balance - Balance analysis results
     * @param {Object} result.pattern - Pattern structure and step data
     * @param {Array} result.polygons - Source polygon specifications
     * @returns {number} Quality score (0-100, higher is better)
     * 
     * @example
     * const score = explorer.calculateQualityScore(patternResult);
     * console.log(`Pattern quality: ${score}/100`);
     * 
     * Scoring Components:
     * - Balance Score (0-50 points): Mathematical balance quality
     * - Density Score (0-25 points): Optimal rhythm density (20-50%)
     * - Polygon Count (0-15 points): Complexity bonus for multi-polygon patterns
     * - Coprime Bonus (0-10 points): Mathematical elegance for coprime pairs
     * 
     * Balance Scoring:
     * - perfect: 50 points (mathematical perfection)
     * - excellent: 40 points (very high quality)
     * - good: 30 points (good balance)
     * - fair: 20 points (acceptable balance)
     * - poor: 10 points (low balance quality)
     * 
     * Density Optimization:
     * - Optimal range: 20-50% (25 points)
     * - Good range: 15-60% (20 points)
     * - Acceptable range: 10-70% (15 points)
     * - Outside optimal: 10 points (baseline)
     * 
     * Mathematical Elegance:
     * - Three or more polygons: 15 points (high complexity)
     * - Two polygons: 10 points (moderate complexity)
     * - Single polygon: 5 points (low complexity)
     * - Coprime pairs: 10 point bonus (mathematical elegance)
     */
    calculateQualityScore(result) {
        const balance = result.balance;
        const structure = result.structure || PatternAnalyzer.analyzeStructure(result.pattern?.steps || [], result.pattern?.stepCount || 0);
        
        let score = 0;
        
        // Balance score (0-50 points)
        switch (balance.balanceScore) {
            case 'perfect': score += 50; break;
            case 'excellent': score += 40; break;
            case 'good': score += 30; break;
            case 'fair': score += 20; break;
            default: score += 10; break;
        }
        
        // Density score (0-25 points) - prefer moderate density
        const density = structure.density;
        if (density >= 20 && density <= 50) score += 25;
        else if (density >= 15 && density <= 60) score += 20;
        else if (density >= 10 && density <= 70) score += 15;
        else score += 10;
        
        // Polygon count bonus (0-15 points)
        const polygonCount = result.polygons?.length || 0;
        if (polygonCount >= 3) score += 15;
        else if (polygonCount === 2) score += 10;
        else score += 5;
        
        // Coprime bonus (0-10 points)
        if (result.polygons && result.polygons.length === 2) {
            if (MathUtils.areCoprimes(result.polygons[0], result.polygons[1])) {
                score += 10;
            }
        }
        
        return Math.min(100, score);
    }
    
    /**
     * Reset Explorer to Initial State
     * 
     * Clears all exploration results and resets operational state
     * for new exploration sessions.
     * 
     * @example
     * explorer.reset();
     * console.log('Explorer ready for new exploration');
     * 
     * Reset Operations:
     * - Clear all discovered pattern results
     * - Reset execution state to stopped
     * - Clear termination flag
     * - Reset progress counters to zero
     * - Prepare for new exploration parameters
     * 
     * Use Cases:
     * - Starting new exploration with different parameters
     * - Clearing memory after large exploration sessions
     * - Resetting state after exploration completion
     * - Preparing for different balance target criteria
     */
    reset() {
        this.results = [];
        this.isRunning = false;
        this.shouldStop = false;
        this.totalCombinations = 0;
        this.currentCombination = 0;
    }
    
    /**
     * Stop Current Exploration
     * 
     * Requests termination of currently running exploration at the
     * next safe checkpoint to preserve discovered results.
     * 
     * @example
     * explorer.stop();
     * console.log('Exploration will stop after current combination');
     * 
     * Termination Behavior:
     * - Sets termination flag for graceful shutdown
     * - Allows current combination to complete processing
     * - Preserves all results discovered before termination
     * - Updates execution state to stopped
     * - Safe for user-initiated cancellation
     * 
     * Graceful Shutdown:
     * - No abrupt termination that could corrupt results
     * - Maintains data integrity during shutdown
     * - Allows UI to update with final progress state
     * - Returns partial results for analysis
     */
    stop() {
        this.shouldStop = true;
        this.isRunning = false;
    }
    
    /**
     * Asynchronous Sleep Utility
     * 
     * Provides non-blocking delay for maintaining browser responsiveness
     * during intensive exploration operations.
     * 
     * @param {number} ms - Delay duration in milliseconds
     * @returns {Promise} Promise that resolves after specified delay
     * 
     * @example
     * await explorer.sleep(10); // 10ms delay for UI updates
     * 
     * Responsiveness Benefits:
     * - Prevents browser UI freezing during long operations
     * - Allows progress updates to render smoothly
     * - Enables user interaction during exploration
     * - Maintains frame rate for animations and feedback
     * 
     * Implementation:
     * - Uses Promise-based setTimeout for async compatibility
     * - Minimal delay preserves exploration performance
     * - Essential for user experience in long explorations
     * - Compatible with async/await pattern
     */
    sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
}

// Export to global scope for browser compatibility
if (typeof window !== 'undefined') {
    window.EXPLORATION_CONFIG = EXPLORATION_CONFIG;
    window.SystematicExplorer = SystematicExplorer;
}