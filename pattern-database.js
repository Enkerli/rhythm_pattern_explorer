// === MODULE 6: PATTERN DATABASE ===

// Database storage key constant
const DATABASE_STORAGE_KEY = 'enhancedPerfectBalancePatternDatabase';

/**
 * Database Manager for Pattern Storage and Retrieval
 * Handles pattern persistence, search, filtering, import/export
 */
class PatternDatabase {
    constructor(autoLoad = true) {
        this.patterns = [];
        this.useLocalStorage = this.testLocalStorage();
        if (autoLoad) {
            this.load(); // Automatically load existing patterns
        }
    }
    
    /**
     * Test if localStorage is available and functional
     * @returns {boolean} true if localStorage is available
     */
    testLocalStorage() {
        try {
            const test = 'localStorage-test';
            localStorage.setItem(test, test);
            localStorage.removeItem(test);
            return true;
        } catch (e) {
            console.warn('localStorage not available, using in-memory storage only');
            return false;
        }
    }
    
    /**
     * Add a new pattern to the database
     * @param {Object} pattern - Pattern object to add
     * @returns {string|null} Pattern ID if added, null if duplicate
     */
    add(pattern) {
        // Auto-fill missing required fields for resilience
        if (!pattern.steps) {
            pattern.steps = [true]; // Default to single step
        }
        
        if (!pattern.stepCount) {
            pattern.stepCount = Array.isArray(pattern.steps) ? pattern.steps.length : 1;
        }
        
        if (!pattern.binary) {
            pattern.binary = Array.isArray(pattern.steps) ? 
                PatternConverter.toBinary(pattern.steps, pattern.stepCount) : '1';
        }
        
        // Check for duplicates based on binary representation and step count
        const exists = this.patterns.some(p => 
            p.binary === pattern.binary && p.stepCount === pattern.stepCount
        );
        
        if (!exists) {
            // Generate unique ID if not provided
            if (!pattern.id) {
                pattern.id = this.generateId();
            }
            
            // Add timestamp if not provided
            if (!pattern.timestamp) {
                pattern.timestamp = Date.now();
            }
            
            this.patterns.unshift(pattern);
            this.save();
            return pattern.id;
        }
        return null;
    }
    
    /**
     * Remove a pattern by ID
     * @param {string} id - Pattern ID to remove
     * @returns {boolean} true if pattern was removed
     */
    remove(id) {
        const initialLength = this.patterns.length;
        this.patterns = this.patterns.filter(p => p.id !== id);
        
        if (this.patterns.length < initialLength) {
            this.save();
            return true;
        }
        return false;
    }
    
    /**
     * Update a pattern by ID
     * @param {string} id - Pattern ID to update
     * @param {Object} updates - Properties to update
     * @returns {boolean} true if pattern was updated
     */
    update(id, updates) {
        const pattern = this.patterns.find(p => p.id === id);
        if (pattern) {
            Object.assign(pattern, updates);
            this.save();
            return true;
        }
        return false;
    }
    
    /**
     * Load patterns from localStorage
     */
    load() {
        if (!this.useLocalStorage) return;
        
        try {
            const saved = localStorage.getItem(DATABASE_STORAGE_KEY);
            if (saved) {
                this.patterns = JSON.parse(saved);
            }
        } catch (e) {
            console.warn('Failed to load pattern database:', e);
            this.patterns = [];
        }
    }
    
    /**
     * Save patterns to localStorage
     */
    save() {
        if (!this.useLocalStorage) return;
        
        try {
            localStorage.setItem(DATABASE_STORAGE_KEY, JSON.stringify(this.patterns));
        } catch (e) {
            console.warn('Failed to save pattern database:', e);
        }
    }
    
    /**
     * Export all patterns as JSON string
     * @returns {string} JSON representation of all patterns
     */
    export() {
        return JSON.stringify(this.patterns, null, 2);
    }
    
    /**
     * Import patterns from JSON data
     * @param {string} jsonData - JSON data to import
     * @returns {boolean} true if import successful
     */
    import(jsonData) {
        try {
            const imported = JSON.parse(jsonData);
            if (Array.isArray(imported)) {
                // Clear current patterns and replace with imported ones
                this.patterns = [];
                
                // Add imported patterns, avoiding duplicates based on binary+stepCount
                imported.forEach(pattern => {
                    if (!pattern.id) {
                        pattern.id = this.generateId();
                    }
                    
                    // Check for duplicates based on binary representation and step count
                    const exists = this.patterns.some(p => 
                        p.binary === pattern.binary && p.stepCount === pattern.stepCount
                    );
                    
                    if (!exists) {
                        // Add timestamp if not provided
                        if (!pattern.timestamp) {
                            pattern.timestamp = Date.now();
                        }
                        this.patterns.unshift(pattern);
                    }
                });
                
                this.save();
                return true;
            }
        } catch (e) {
            console.error('Import failed:', e);
        }
        return false;
    }
    
    /**
     * Search patterns by query string
     * @param {string} query - Search query
     * @returns {Array} Matching patterns
     */
    search(query) {
        if (!query || query.trim() === '') return [...this.patterns];
        
        const lowerQuery = query.toLowerCase();
        return this.patterns.filter(p => 
            (p.name && p.name.toLowerCase().includes(lowerQuery)) ||
            (p.binary && p.binary.includes(query)) ||
            (p.hex && p.hex.toLowerCase().includes(lowerQuery)) ||
            (p.decimal && p.decimal.toString().includes(query)) ||
            (p.polygonType && p.polygonType.toLowerCase().includes(lowerQuery)) ||
            (p.euclidean && p.euclidean.toLowerCase().includes(lowerQuery)) ||
            (p.expression && p.expression.toLowerCase().includes(lowerQuery))
        );
    }
    
    /**
     * Filter patterns by type
     * @param {string} type - Filter type
     * @returns {Array} Filtered patterns
     */
    filter(type) {
        switch (type) {
            case 'favorites':
                return this.patterns.filter(p => p.favorite);
            case 'perfect_balance':
                return this.patterns.filter(p => p.perfectBalance && p.perfectBalance.isPerfectlyBalanced);
            case 'polygons':
                return this.patterns.filter(p => p.polygon || p.isRegularPolygon);
            case 'euclidean':
                return this.patterns.filter(p => p.euclidean || p.isEuclidean);
            case 'combined':
                return this.patterns.filter(p => p.combined || p.isCombined);
            case 'subtraction':
                return this.patterns.filter(p => p.hasSubtraction);
            case 'repetitive':
                return this.patterns.filter(p => p.repetition);
            case 'wellformed':
                return this.patterns.filter(p => p.milne && p.milne.wellformed);
            case 'balanced':
                return this.patterns.filter(p => 
                    (p.milne && p.milne.perfectlyBalanced) || 
                    (p.perfectBalance && ['perfect', 'excellent'].includes(p.perfectBalance.balanceScore))
                );
            case 'geometric_centered':
                return this.patterns.filter(p => p.cogAnalysis && p.cogAnalysis.normalizedMagnitude < 0.1);
            case 'coprime':
                return this.patterns.filter(p => p.coprime && p.coprime.areCoprimes);
            default:
                return [...this.patterns];
        }
    }
    
    /**
     * Get database statistics
     * @returns {Object} Statistics about the database
     */
    getStatistics() {
        const total = this.patterns.length;
        const favorites = this.filter('favorites').length;
        const perfectBalance = this.filter('perfect_balance').length;
        const polygons = this.filter('polygons').length;
        const euclidean = this.filter('euclidean').length;
        const combined = this.filter('combined').length;
        
        return {
            totalPatterns: total,
            favorites,
            perfectBalance,
            polygons,
            euclidean,
            combined,
            storageType: this.useLocalStorage ? 'localStorage' : 'memory'
        };
    }
    
    /**
     * Generate unique pattern ID
     * @returns {string} Unique ID
     */
    generateId() {
        return Date.now().toString(36) + Math.random().toString(36).substr(2);
    }
    
    /**
     * Clear all patterns from database
     */
    clear() {
        this.patterns = [];
        this.save();
    }
    
    /**
     * Get pattern by ID
     * @param {string} id - Pattern ID
     * @returns {Object|null} Pattern object or null if not found
     */
    getById(id) {
        return this.patterns.find(p => p.id === id) || null;
    }
    
    /**
     * Get patterns sorted by specified criteria
     * @param {string} sortBy - Sort criteria ('date', 'steps', 'name', 'balance')
     * @returns {Array} Sorted patterns
     */
    getSorted(sortBy = 'date') {
        const sorted = [...this.patterns];
        switch (sortBy) {
            case 'date':
                return sorted.sort((a, b) => (b.timestamp || 0) - (a.timestamp || 0));
            case 'steps':
                return sorted.sort((a, b) => a.stepCount - b.stepCount || a.decimal - b.decimal);
            case 'name':
                return sorted.sort((a, b) => (a.name || '').localeCompare(b.name || ''));
            case 'balance':
                return sorted.sort((a, b) => {
                    const balanceA = a.perfectBalance ? a.perfectBalance.magnitude : 1;
                    const balanceB = b.perfectBalance ? b.perfectBalance.magnitude : 1;
                    return balanceA - balanceB;
                });
            default:
                return sorted;
        }
    }
}

/**
 * Create a pattern object for database storage
 * @param {Object} patternData - Pattern data from generators or analysis
 * @param {Object} analyses - Optional analysis results
 * @returns {Object} Pattern object ready for database
 */
function createDatabasePattern(patternData, analyses = {}) {
    const { steps, stepCount } = patternData;
    const binaryString = PatternConverter.toBinary(steps, stepCount);
    const decimal = PatternConverter.toDecimal(binaryString);
    
    // Create pattern name based on type
    let defaultName = '';
    if (patternData.expression) {
        defaultName = `Expression: ${patternData.expression}`;
    } else if (patternData.isCombined) {
        defaultName = `Combined (${patternData.originalPatterns ? patternData.originalPatterns.length : 'multi'} patterns)`;
    } else if (patternData.isRegularPolygon) {
        defaultName = `${patternData.polygonType} (${patternData.vertices},${patternData.expansion || 1},${patternData.offset || 0})`;
    } else if (patternData.isEuclidean) {
        defaultName = `Euclidean (${patternData.beats},${stepCount},${patternData.offset || 0})`;
    } else {
        defaultName = `Custom Pattern (${stepCount} steps)`;
    }
    
    // Add perfect balance prefix if applicable
    if (analyses.perfectBalance && analyses.perfectBalance.isPerfectlyBalanced) {
        defaultName = `Perfect Balance: ${defaultName}`;
    }
    
    const pattern = {
        id: Date.now().toString(36) + Math.random().toString(36).substr(2),
        name: patternData.name || defaultName,
        favorite: false,
        stepCount,
        binary: binaryString,
        hex: PatternConverter.toHex(decimal),
        decimal,
        steps: steps,
        expression: patternData.expression || null,
        timestamp: Date.now()
    };
    
    // Add analysis results if provided
    if (analyses.perfectBalance) {
        pattern.perfectBalance = analyses.perfectBalance;
    }
    
    if (patternData.isRegularPolygon) {
        pattern.polygon = {
            vertices: patternData.vertices,
            expansion: patternData.expansion || 1,
            offset: patternData.offset || 0,
            polygonType: patternData.polygonType,
            formula: patternData.formula
        };
        pattern.isRegularPolygon = true;
    }
    
    if (patternData.isEuclidean) {
        pattern.euclidean = patternData.formula || `E(${patternData.beats},${stepCount},${patternData.offset || 0})`;
        pattern.isEuclidean = true;
    }
    
    if (patternData.isCombined) {
        pattern.combined = {
            originalPatterns: patternData.originalPatterns,
            lcmUsed: patternData.stepCount,
            combinationType: patternData.combinationType || 'logical_or',
            hasSubtraction: patternData.hasSubtraction || false
        };
        pattern.isCombined = true;
        pattern.hasSubtraction = patternData.hasSubtraction || false;
    }
    
    return pattern;
}

/**
 * Database utility functions for pattern management
 */
const DatabaseUtils = {
    /**
     * Generate unique pattern ID
     * @returns {string} Unique ID
     */
    generateId() {
        return Date.now().toString(36) + Math.random().toString(36).substr(2);
    },
    
    /**
     * Validate pattern object structure
     * @param {Object} pattern - Pattern to validate
     * @returns {boolean} true if valid
     */
    validatePattern(pattern) {
        return pattern && 
               typeof pattern.stepCount === 'number' &&
               typeof pattern.binary === 'string' &&
               Array.isArray(pattern.steps);
    },
    
    /**
     * Sort patterns by different criteria
     * @param {Array} patterns - Patterns to sort
     * @param {string} sortBy - Sort criteria ('date', 'steps', 'name', 'balance')
     * @returns {Array} Sorted patterns
     */
    sortPatterns(patterns, sortBy = 'date') {
        const sorted = [...patterns];
        switch (sortBy) {
            case 'date':
                return sorted.sort((a, b) => (b.timestamp || 0) - (a.timestamp || 0));
            case 'steps':
                return sorted.sort((a, b) => a.stepCount - b.stepCount || a.decimal - b.decimal);
            case 'name':
                return sorted.sort((a, b) => (a.name || '').localeCompare(b.name || ''));
            case 'balance':
                return sorted.sort((a, b) => {
                    const balanceA = a.perfectBalance ? a.perfectBalance.magnitude : 1;
                    const balanceB = b.perfectBalance ? b.perfectBalance.magnitude : 1;
                    return balanceA - balanceB;
                });
            default:
                return sorted;
        }
    }
};

// Export to global scope for browser compatibility
if (typeof window !== 'undefined') {
    window.DATABASE_STORAGE_KEY = DATABASE_STORAGE_KEY;
    window.PatternDatabase = PatternDatabase;
    window.createDatabasePattern = createDatabasePattern;
    window.DatabaseUtils = DatabaseUtils;
}