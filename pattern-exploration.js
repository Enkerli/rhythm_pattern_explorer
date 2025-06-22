// === MODULE 5: PATTERN EXPLORATION ===

// Exploration configuration constants
const EXPLORATION_CONFIG = {
    MAX_OFFSET_LIMIT: 8,
    MIN_POLYGON_SIDES: 3,
    MAX_COMBINATIONS_PER_TEST: 50,
    OFFSET_COMBINATIONS_LIMIT: 20,
    UI_UPDATE_INTERVAL: 10, // Update UI every 10 combinations
    ASYNC_DELAY_MS: 1 // Delay for UI updates
};

class SystematicExplorer {
    constructor() {
        this.results = [];
        this.isRunning = false;
        this.shouldStop = false;
        this.totalCombinations = 0;
        this.currentCombination = 0;
    }
    
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
    
    getProgress() {
        if (this.totalCombinations === 0) return 0;
        return (this.currentCombination / this.totalCombinations) * 100;
    }
    
    getPerfectBalanceResults() {
        return this.results.filter(r => r.balance.balanceScore === 'perfect');
    }
    
    getNearPerfectResults() {
        return this.results.filter(r => ['perfect', 'excellent', 'good'].includes(r.balance.balanceScore));
    }
    
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
    
    reset() {
        this.results = [];
        this.isRunning = false;
        this.shouldStop = false;
        this.totalCombinations = 0;
        this.currentCombination = 0;
    }
    
    stop() {
        this.shouldStop = true;
        this.isRunning = false;
    }
    
    sleep(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
}

// Export to global scope for browser compatibility
if (typeof window !== 'undefined') {
    window.EXPLORATION_CONFIG = EXPLORATION_CONFIG;
    window.SystematicExplorer = SystematicExplorer;
}