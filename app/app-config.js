/**
 * App Configuration Module (app-config.js)
 * Centralized configuration management for the Rhythm Pattern Explorer
 * 
 * Features:
 * - Default parameters and constants
 * - UI text, labels, and messages
 * - Mathematical constants and thresholds
 * - Feature flags and toggles
 * - Color schemes and styling constants
 * - Performance settings and limits
 * - Validation rules and constraints
 * 
 * Dependencies: None
 */

/**
 * Main Application Configuration Object
 * All constants, settings, and configuration values used throughout the application
 */
const AppConfig = {
    
    // ========================================
    // UI CONFIGURATION
    // ========================================
    UI: {
        
        /**
         * CSS Class Names used throughout the application
         */
        CLASSES: {
            // Analysis display classes
            ANALYSIS: {
                TITLE: 'analysis-title',
                CONTENT: 'analysis-content',
                ITEM: 'analysis-item',
                FORMULA: 'mathematical-formula'
            },
            
            // Balance score classes
            BALANCE: {
                SCORE: 'balance-score',
                PERFECT: 'balance-perfect',
                EXCELLENT: 'balance-excellent',
                GOOD: 'balance-good',
                FAIR: 'balance-fair',
                POOR: 'balance-poor'
            },
            
            // Pattern entry classes
            PATTERN: {
                ENTRY: 'pattern-entry',
                HEADER: 'pattern-header',
                NAME: 'pattern-name',
                ACTIONS: 'pattern-actions',
                STAR: 'pattern-star',
                REPR: 'pattern-repr',
                INFO: 'pattern-info',
                REPRESENTATIONS: 'pattern-representations'
            },
            
            // Modal classes
            MODAL: {
                CONTAINER: 'modal',
                CONTENT: 'modal-content',
                HEADER: 'modal-header',
                BODY: 'modal-body',
                CLOSE: 'modal-close'
            },
            
            // Button classes
            BUTTON: {
                BASE: 'btn',
                PRIMARY: 'btn-primary',
                SUCCESS: 'btn-success',
                WARNING: 'btn-warning',
                DANGER: 'btn-danger',
                SECONDARY: 'btn-secondary'
            },
            
            // Input classes
            INPUT: {
                BASE: 'input',
                ERROR: 'error',
                INPUT_ERROR: 'input-error'
            },
            
            // Special feature classes
            SPECIAL: {
                PERFECT_BALANCE: 'perfect-balance',
                PERFECT_BALANCE_BADGE: 'perfect-balance-badge',
                POLYGON_TYPE: 'polygon-type',
                EUCLIDEAN_TYPE: 'euclidean-type',
                COMBINED_TYPE: 'combined-type',
                COG_VALUE: 'cog-value',
                CLICKABLE_UNIT: 'clickable-unit'
            }
        },
        
        /**
         * Color Schemes and Values
         */
        COLORS: {
            // Balance score colors
            BALANCE_SCORES: {
                PERFECT: '#4CAF50',     // Green
                EXCELLENT: '#8BC34A',   // Light Green
                GOOD: '#FFEB3B',        // Yellow
                FAIR: '#FF9800',        // Orange
                POOR: '#F44336'         // Red
            },
            
            // UI state colors
            UI_STATES: {
                SUCCESS: '#28a745',
                ERROR: '#f44336',
                WARNING: '#ffc107',
                INFO: '#17a2b8',
                PRIMARY: '#007bff',
                SECONDARY: '#6c757d'
            },
            
            // Background colors
            BACKGROUNDS: {
                LIGHT: '#f8f9fa',
                WHITE: '#ffffff',
                GRAY_LIGHT: '#f0f0f0',
                GRAY: '#6c757d',
                DARK: '#343a40'
            },
            
            // Border colors
            BORDERS: {
                LIGHT: '#dee2e6',
                GRAY: '#ced4da',
                DARK: '#495057'
            }
        },
        
        /**
         * UI Text and Labels
         */
        TEXT: {
            // Analysis section titles
            ANALYSIS_TITLES: {
                MAIN: 'Mathematical Pattern Analysis',
                BASIC_PROPERTIES: 'Basic Properties',
                PERFECT_BALANCE: '🎼 Perfect Balance Analysis (Milne)',
                PATTERN_COMBINATION: '🎯 Pattern Combination',
                ADVANCED_COMBINATION: '➖ Advanced Combination with Subtraction',
                PATTERN_ROTATION: '🔄 Pattern Rotation Applied',
                POLYGON_DETECTED: '🔺 Regular Polygon Detected',
                EUCLIDEAN_DETECTED: '🌀 Euclidean Rhythm Detected',
                CENTER_OF_GRAVITY: '📐 Center of Gravity Analysis',
                PATTERN_REPETITION: '🔄 Pattern Repetition Detected'
            },
            
            // Button labels
            BUTTON_LABELS: {
                PARSE: 'Parse Pattern',
                ADD_TO_DB: 'Add to Database',
                COPY_HEX: 'Copy Hex',
                LOAD: 'Load',
                DELETE: 'Delete',
                EXPORT: 'Export',
                IMPORT: 'Import',
                CLEAR_DB: 'Clear Database',
                FIND_PERFECT: 'Find Perfect',
                FIND_NEAR: 'Find Near Perfect',
                STOP: 'Stop',
                SORT_BY_DATE: 'Sort by Date',
                SORT_BY_STEPS: 'Sort by Steps',
                RUN_TESTS: 'Run Tests',
                COPY_ALL: 'Copy All',
                CLOSE: 'Close',
                CANCEL: 'Cancel'
            },
            
            // Section headers
            SECTION_HEADERS: {
                UNIVERSAL_INPUT: 'Universal Pattern Input',
                COMPACT_OUTPUT: 'Pattern Output',
                ANALYSIS_DISPLAY: 'Mathematical Analysis',
                PERFECT_BALANCE_EXPLORER: 'Perfect Balance Explorer',
                PATTERN_DATABASE: 'Pattern Database',
                DATABASE_STATISTICS: 'Database Statistics'
            },
            
            // Status messages
            STATUS: {
                PERFECTLY_BALANCED: '✨ PERFECTLY BALANCED - Center of gravity at origin!',
                NOT_PERFECTLY_BALANCED: 'Not perfectly balanced',
                WELL_FORMED: '⚖️ Well-formed',
                PERFECTLY_BALANCED_SHORT: '🎯 Perfectly balanced',
                COPRIME: '🔢 Coprime',
                REPETITIVE: '🔄 Repetitive pattern'
            }
        },
        
        /**
         * Placeholder Text
         */
        PLACEHOLDERS: {
            UNIVERSAL_INPUT: 'P(3,1)+P(5,0)+P(2,5), P(3,0)+P(5,1)-P(2,0), E(5,8,0), 0x92@3, 0o452, etc.',
            SEARCH_INPUT: 'Search patterns...',
            PATTERN_NAME: 'Enter pattern name',
            IMPORT_DATA: 'Paste your JSON pattern database here...',
            EXPORT_FILENAME: 'enhanced-perfect-balance-database',
            NO_NAME: 'Click to name'
        },
        
        /**
         * Help and Example Text
         */
        HELP_TEXT: {
            EXAMPLES: {
                TRIPLE_COMBINATION: 'P(3,1)+P(5,0)+P(2,5) - Fixed triple combination',
                PERFECT_BALANCE: 'P(3,0)+P(5,1)-P(2,0) - Perfect balance via subtraction',
                POLYGON_EXPANDED: 'P(3,1,4) - Triangle×4',
                EUCLIDEAN: 'E(5,8,0) - Quintillo',
                HEX: '0x92 - Tresillo',
                OCTAL: '0o452 - Octal pattern for ternary rhythms',
                BINARY: 'b101010 - Binary pattern',
                DECIMAL: '146 - Decimal pattern',
                ROTATION: '0x92@3 - Tresillo rotated 3 steps'
            },
            
            INITIAL_HELP: 'Parse a pattern using the universal input above to see comprehensive mathematical analysis.'
        }
    },
    
    // ========================================
    // MATHEMATICAL CONFIGURATION
    // ========================================
    MATH: {
        
        /**
         * Analysis Thresholds
         */
        THRESHOLDS: {
            // Perfect balance analysis thresholds
            BALANCE: {
                PERFECT: 0.001,         // Perfect balance threshold
                EXCELLENT: 0.05,        // Excellent balance threshold
                GOOD: 0.15,             // Good balance threshold
                FAIR: 0.4               // Fair balance threshold
            },
            
            // Center of gravity thresholds
            CENTER_OF_GRAVITY: {
                GEOMETRICALLY_BALANCED: 0.1,
                WELL_DISTRIBUTED: 0.3,
                MODERATELY_SPREAD: 0.7
            }
        },
        
        /**
         * Mathematical Limits
         */
        LIMITS: {
            // Pattern generation limits
            PATTERNS: {
                MIN_VERTICES: 2,        // Minimum polygon vertices
                MAX_VERTICES: 32,       // Maximum polygon vertices
                MIN_EXPANSION: 1,       // Minimum expansion factor
                MAX_EXPANSION: 21,      // Maximum expansion factor
                MAX_STEPS: 64,          // Maximum total steps
                MIN_STEPS: 1            // Minimum steps
            },
            
            // Exploration limits
            EXPLORATION: {
                MAX_OFFSET_LIMIT: 8,
                MIN_POLYGON_SIDES: 3,
                MAX_COMBINATIONS_PER_TEST: 50,
                OFFSET_COMBINATIONS_LIMIT: 20
            }
        },
        
        /**
         * Mathematical Constants
         */
        CONSTANTS: {
            TWO_PI: Math.PI * 2,
            PI_HALF: Math.PI / 2,
            EULER: Math.E,
            GOLDEN_RATIO: (1 + Math.sqrt(5)) / 2
        }
    },
    
    // ========================================
    // PATTERN CONFIGURATION
    // ========================================
    PATTERNS: {
        
        /**
         * Polygon Names
         */
        POLYGON_NAMES: {
            2: 'Digon',
            3: 'Triangle',
            4: 'Square',
            5: 'Pentagon',
            6: 'Hexagon',
            7: 'Heptagon',
            8: 'Octagon',
            9: 'Enneagon',
            10: 'Decagon',
            11: 'Hendecagon',
            12: 'Dodecagon',
            13: 'Tridecagon',
            14: 'Tetradecagon',
            15: 'Pentadecagon',
            16: 'Hexadecagon',
            17: 'Heptadecagon',
            18: 'Octadecagon'
        },
        
        /**
         * Famous Rhythms
         */
        FAMOUS_RHYTHMS: {
            TRESILLO: {
                name: 'Tresillo',
                hex: '0x92',
                description: 'Cuban tresillo rhythm'
            },
            CINQUILLO: {
                name: 'Cinquillo',
                hex: '0x96',
                description: 'Cuban cinquillo rhythm'
            },
            SON: {
                name: 'Son',
                hex: '0xA5',
                description: 'Cuban son rhythm'
            }
        }
    },
    
    // ========================================
    // DATABASE CONFIGURATION
    // ========================================
    DATABASE: {
        
        /**
         * Storage Configuration
         */
        STORAGE_KEY: 'enhancedPerfectBalancePatternDatabase',
        
        /**
         * Database Limits
         */
        LIMITS: {
            MAX_PATTERNS: 10000,        // Maximum patterns in database
            MAX_NAME_LENGTH: 100,       // Maximum pattern name length
            MAX_EXPORT_SIZE: 50 * 1024 * 1024  // 50MB max export size
        },
        
        /**
         * Filter Types
         */
        FILTER_TYPES: {
            ALL: 'all',
            FAVORITES: 'favorites',
            PERFECT_BALANCE: 'perfect_balance',
            POLYGONS: 'polygons',
            EUCLIDEAN: 'euclidean',
            COMBINED: 'combined',
            WELL_FORMED: 'well_formed'
        },
        
        /**
         * Sort Options
         */
        SORT_OPTIONS: {
            DATE: 'date',
            STEPS: 'steps',
            NAME: 'name',
            BALANCE: 'balance'
        }
    },
    
    // ========================================
    // EXPLORATION CONFIGURATION
    // ========================================
    EXPLORATION: {
        
        /**
         * Search Targets
         */
        TARGETS: {
            PERFECT: 'perfect',
            NEAR: 'near'
        },
        
        /**
         * Performance Settings
         */
        PERFORMANCE: {
            UI_UPDATE_INTERVAL: 10,     // Update UI every N combinations
            ASYNC_DELAY_MS: 1,          // Async delay between operations
            PROGRESS_UPDATE_THRESHOLD: 100  // Update progress every N operations
        }
    },
    
    // ========================================
    // MESSAGES CONFIGURATION
    // ========================================
    MESSAGES: {
        
        /**
         * Error Messages
         */
        ERRORS: {
            NO_PATTERN: 'No pattern to copy. Please parse a pattern first.',
            NO_PATTERN_ADD: 'No pattern to add. Please parse a pattern first.',
            VERTICES_RANGE: 'Vertices must be between 2 and 32',
            EXPANSION_RANGE: 'Expansion factor must be between 1 and 21',
            MAX_STEPS: 'Total steps cannot exceed 64',
            EMPTY_INPUT: 'Input must be a non-empty array of numbers',
            MIN_PATTERNS: 'Need at least 2 patterns to combine',
            COMBINATION_SYNTAX: 'Combination syntax must be: pattern1 + pattern2 [+ pattern3 ...]',
            UNRECOGNIZED_FORMAT: 'Unrecognized pattern format',
            IMPORT_FAILED: 'Failed to import database: Invalid file format',
            IMPORT_JSON_FAILED: 'Failed to import database: Invalid JSON format'
        },
        
        /**
         * Alert Messages
         */
        ALERTS: {
            MIN_MAX_SIDES: 'Min sides must be ≤ max sides',
            PATTERN_ADDED: 'Pattern added to database!',
            PATTERN_EXISTS: 'Pattern already exists in database.',
            NO_PERFECT_BALANCE: 'No perfect balance patterns found in database. Run exploration first.',
            CLEAR_CONFIRM: 'Clear all patterns from database?',
            EXPLORATION_COMPLETE: 'Exploration complete',
            PERFECT_PATTERNS_FOUND: 'perfect balance patterns found!'
        },
        
        /**
         * Success Messages
         */
        SUCCESS: {
            DATABASE_IMPORTED: 'Enhanced pattern database imported successfully!',
            COPIED: 'Copied!',
            EXPORT_COMPLETE: 'Database exported successfully',
            TESTS_PASSED: 'All tests passed!'
        },
        
        /**
         * Info Messages
         */
        INFO: {
            PARSING_PATTERN: 'Parsing pattern...',
            LOADING_DATABASE: 'Loading database...',
            SEARCHING_PATTERNS: 'Searching patterns...',
            GENERATING_REPORT: 'Generating report...'
        }
    },
    
    // ========================================
    // DEFAULT VALUES
    // ========================================
    DEFAULTS: {
        
        /**
         * Exploration Defaults
         */
        EXPLORATION: {
            MIN_SIDES: 3,
            MAX_SIDES: 7,
            MAX_COMBINATIONS: 3
        },
        
        /**
         * Pattern Input Defaults
         */
        PATTERN_INPUT: {
            SAMPLE_PATTERN: 'P(3,1)+P(5,0)+P(2,5)',
            DEFAULT_VERTICES: 3,
            DEFAULT_EXPANSION: 1,
            DEFAULT_OFFSET: 0
        },
        
        /**
         * UI Defaults
         */
        UI: {
            SORT_BY_DATE: true,
            SHOW_ANALYSIS: true,
            AUTO_SAVE: true
        },
        
        /**
         * Visualization Defaults
         */
        VISUALIZATION: {
            COG_RADIUS: 80,
            COG_CENTER_X: 100,
            COG_CENTER_Y: 100,
            CIRCLE_RADIUS: 30,
            POINT_RADIUS: 3
        }
    },
    
    // ========================================
    // PERFORMANCE CONFIGURATION
    // ========================================
    PERFORMANCE: {
        
        /**
         * Timeout Values (in milliseconds)
         */
        TIMEOUTS: {
            PARSE_TIMEOUT: 5000,        // Pattern parsing timeout
            ANALYSIS_TIMEOUT: 10000,    // Analysis timeout
            DATABASE_TIMEOUT: 3000,     // Database operation timeout
            EXPLORATION_TIMEOUT: 300000, // 5 minutes exploration timeout
            COPY_FEEDBACK: 2000,        // Copy feedback display time
            MODAL_TRANSITION: 200       // Modal transition time
        },
        
        /**
         * Performance Limits
         */
        LIMITS: {
            MAX_RENDER_ITEMS: 1000,     // Maximum items to render at once
            DEBOUNCE_DELAY: 300,        // Input debounce delay
            THROTTLE_DELAY: 100,        // Scroll throttle delay
            MAX_HISTORY_SIZE: 100       // Maximum undo/redo history
        },
        
        /**
         * Performance Thresholds
         */
        THRESHOLDS: {
            FAST_OPERATION: 50,         // Operations under 50ms are "fast"
            SLOW_OPERATION: 500,        // Operations over 500ms are "slow"
            CRITICAL_OPERATION: 1000    // Operations over 1000ms are "critical"
        }
    },
    
    // ========================================
    // VALIDATION CONFIGURATION
    // ========================================
    VALIDATION: {
        
        /**
         * Pattern Validation Rules
         */
        PATTERNS: {
            MIN_STEPS: 1,
            MAX_STEPS: 64,
            MIN_BEATS: 0,
            MAX_BEATS: 64,
            MIN_VERTICES: 2,
            MAX_VERTICES: 32
        },
        
        /**
         * Input Validation Rules
         */
        INPUT: {
            MAX_INPUT_LENGTH: 1000,
            ALLOWED_CHARACTERS: /^[0-9a-fA-F+\-*().,\s:@PEbxo]+$/,
            PATTERN_FORMATS: [
                'polygon',      // P(v,o) or P(v,o,e)
                'euclidean',    // E(b,s,o)
                'binary',       // b101010
                'hexadecimal',  // 0xAB or 0xAB:8
                'octal',        // 0o452 or 0o452:9
                'decimal',      // 123 or 123:8
                'rotation',     // pattern@steps
                'combination'   // pattern+pattern
            ]
        },
        
        /**
         * Database Validation Rules
         */
        DATABASE: {
            MAX_PATTERN_NAME: 100,
            MIN_PATTERN_NAME: 1,
            REQUIRED_FIELDS: ['id', 'stepCount', 'binary'],
            OPTIONAL_FIELDS: ['name', 'favorite', 'timestamp']
        }
    },
    
    // ========================================
    // FEATURE FLAGS
    // ========================================
    FEATURES: {
        ENABLE_PERFECT_BALANCE_EXPLORER: true,
        ENABLE_PATTERN_DATABASE: true,
        ENABLE_ADVANCED_COMBINATIONS: true,
        ENABLE_PATTERN_ROTATION: true,
        ENABLE_EUCLIDEAN_DETECTION: true,
        ENABLE_POLYGON_DETECTION: true,
        ENABLE_REPETITION_ANALYSIS: true,
        ENABLE_IMPORT_EXPORT: true,
        ENABLE_FAVORITES: true,
        ENABLE_SEARCH_FILTERING: true,
        ENABLE_PERFORMANCE_MONITORING: false,
        ENABLE_DEBUG_LOGGING: false,
        ENABLE_EXPERIMENTAL_FEATURES: false
    },
    
    // ========================================
    // DEVELOPMENT CONFIGURATION
    // ========================================
    DEV: {
        
        /**
         * Debug Settings
         */
        DEBUG: {
            LOG_LEVEL: 'info',          // 'debug', 'info', 'warn', 'error'
            SHOW_PERFORMANCE: false,
            SHOW_MEMORY_USAGE: false,
            ENABLE_CONSOLE_COMMANDS: false
        },
        
        /**
         * Test Configuration
         */
        TESTING: {
            ENABLE_TEST_PATTERNS: true,
            TEST_TIMEOUT: 5000,
            MOCK_SLOW_OPERATIONS: false
        }
    }
};

/**
 * Configuration Helper Functions
 */
AppConfig.getBalanceScoreColor = function(score) {
    const colors = this.UI.COLORS.BALANCE_SCORES;
    switch (score.toLowerCase()) {
        case 'perfect': return colors.PERFECT;
        case 'excellent': return colors.EXCELLENT;
        case 'good': return colors.GOOD;
        case 'fair': return colors.FAIR;
        case 'poor': return colors.POOR;
        default: return colors.POOR;
    }
};

AppConfig.getPolygonName = function(vertices) {
    return this.PATTERNS.POLYGON_NAMES[vertices] || `${vertices}-gon`;
};

AppConfig.isFeatureEnabled = function(featureName) {
    return this.FEATURES[featureName] === true;
};

AppConfig.getPerformanceClass = function(duration) {
    const thresholds = this.PERFORMANCE.THRESHOLDS;
    if (duration < thresholds.FAST_OPERATION) return 'fast';
    if (duration < thresholds.SLOW_OPERATION) return 'normal';
    if (duration < thresholds.CRITICAL_OPERATION) return 'slow';
    return 'critical';
};

AppConfig.validatePattern = function(pattern) {
    const rules = this.VALIDATION.PATTERNS;
    if (!pattern || !pattern.steps) return false;
    if (pattern.stepCount < rules.MIN_STEPS || pattern.stepCount > rules.MAX_STEPS) return false;
    return true;
};

/**
 * Configuration Freezing (prevents accidental modification)
 */
if (typeof Object.freeze === 'function') {
    // Recursively freeze all nested objects
    function deepFreeze(obj) {
        Object.freeze(obj);
        Object.getOwnPropertyNames(obj).forEach(function(prop) {
            if (obj[prop] !== null && typeof obj[prop] === 'object') {
                deepFreeze(obj[prop]);
            }
        });
        return obj;
    }
    
    // Only freeze in production (allow modifications in development)
    if (!AppConfig.DEV.DEBUG.ENABLE_CONSOLE_COMMANDS) {
        deepFreeze(AppConfig);
    }
}

// Export configuration (make globally available)
if (typeof window !== 'undefined') {
    window.AppConfig = AppConfig;
}

// Also support module exports for Node.js environments
if (typeof module !== 'undefined' && module.exports) {
    module.exports = AppConfig;
}

// Console logging for development
if (typeof console !== 'undefined' && AppConfig.DEV.DEBUG.LOG_LEVEL === 'debug') {
    console.log('⚙️ AppConfig loaded successfully');
    console.log('📊 Configuration sections:', Object.keys(AppConfig));
    console.log('🎨 UI color schemes loaded:', Object.keys(AppConfig.UI.COLORS));
    console.log('📐 Mathematical thresholds configured:', Object.keys(AppConfig.MATH.THRESHOLDS));
    console.log('💾 Database configuration ready');
    console.log('🔍 Validation rules established');
    console.log('🚀 Performance settings configured');
}