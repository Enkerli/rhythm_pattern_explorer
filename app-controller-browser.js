/**
 * App Controller Module (app-controller-browser.js)
 * Browser-compatible version - Main application orchestrator for the Rhythm Pattern Explorer
 * 
 * Features:
 * - EnhancedPatternApp class - Central application controller
 * - Event handling setup and management
 * - Workflow coordination between all modules
 * - UI state management and updates
 * - Application initialization and error handling
 * 
 * Dependencies: ALL other modules (loaded as global classes via script tags)
 * - MathUtils (from math-core.js)
 * - RegularPolygonGenerator, EuclideanGenerator (from pattern-generators.js)
 * - PerfectBalanceAnalyzer, PatternAnalyzer, CenterOfGravityCalculator (from pattern-analysis.js)
 * - AdvancedPatternCombiner, UnifiedPatternParser, PatternConverter (from pattern-processing.js)
 * - SystematicExplorer (from pattern-exploration.js)
 * - PatternDatabase (from pattern-database.js)
 * - UIComponents (from ui-components.js)
 * - AppConfig (from app-config.js)
 */

/**
 * Enhanced Pattern Application Controller
 * Main orchestrator class that coordinates all modules and manages application state
 */
class EnhancedPatternApp {
    constructor() {
        console.log('🎼 Initializing Enhanced Pattern Application...');
        
        // Verify dependencies are available
        this.verifyDependencies();
        
        // Initialize core components
        this.database = new PatternDatabase();
        this.explorer = new SystematicExplorer();
        
        // Application state
        this.currentPattern = null;
        this.sortByDate = true;
        this.explorationResults = [];
        this.isExploring = false;
        
        // Initialize the application
        this.setupEventListeners();
        this.initialize();
        
        console.log('✅ Enhanced Pattern Application initialized successfully');
    }
    
    /**
     * Verify all required dependencies are available
     */
    verifyDependencies() {
        const requiredClasses = [
            'MathUtils', 'RegularPolygonGenerator', 'EuclideanGenerator',
            'PerfectBalanceAnalyzer', 'PatternAnalyzer', 'CenterOfGravityCalculator',
            'AdvancedPatternCombiner', 'UnifiedPatternParser', 'PatternConverter',
            'SystematicExplorer', 'PatternDatabase', 'UIComponents', 'AppConfig'
        ];
        
        const missingClasses = requiredClasses.filter(className => 
            typeof window[className] === 'undefined'
        );
        
        if (missingClasses.length > 0) {
            throw new Error(`Missing required dependencies: ${missingClasses.join(', ')}`);
        }
        
        console.log('✅ All dependencies verified');
    }
    
    /**
     * Initialize the application
     */
    initialize() {
        // Load existing patterns from database
        this.updatePatternList();
        this.updateDatabaseStats();
        
        // Set default values
        this.resetExplorationControls();
        
        // Update UI state
        this.updateSortButton();
        
        console.log('🎯 Application initialization complete');
    }
    
    /**
     * Setup all event listeners for the application
     */
    setupEventListeners() {
        console.log('🔧 Setting up event listeners...');
        
        // Universal input controls
        this.setupUniversalInputEvents();
        
        // Perfect balance explorer events
        this.setupExplorationEvents();
        
        // Database events
        this.setupDatabaseEvents();
        
        // Search and filter events
        this.setupSearchEvents();
        
        console.log('✅ Event listeners configured');
    }
    
    /**
     * Setup universal pattern input event listeners
     */
    setupUniversalInputEvents() {
        const parseBtn = document.getElementById('parseBtn');
        const addBtn = document.getElementById('addUniversalBtn');
        const copyBtn = document.getElementById('copyHexUniversalBtn');
        const testBtn = document.getElementById('testBtn');
        const universalInput = document.getElementById('universalInput');
        
        if (parseBtn) {
            parseBtn.addEventListener('click', () => this.parseUniversalInput());
        }
        
        if (addBtn) {
            addBtn.addEventListener('click', () => this.addCurrentPatternToDatabase());
        }
        
        if (copyBtn) {
            copyBtn.addEventListener('click', () => this.copyCurrentPatternHex());
        }
        
        if (testBtn) {
            testBtn.addEventListener('click', () => this.runTestExamples());
        }
        
        if (universalInput) {
            universalInput.addEventListener('keypress', (e) => {
                if (e.key === 'Enter') {
                    this.parseUniversalInput();
                }
            });
        }
    }
    
    /**
     * Setup perfect balance exploration event listeners
     */
    setupExplorationEvents() {
        const findPerfectBtn = document.getElementById('findPerfectBtn');
        const findNearBtn = document.getElementById('findNearBtn');
        const generateReportBtn = document.getElementById('generateReportBtn');
        const stopBtn = document.getElementById('stopBtn');
        
        if (findPerfectBtn) {
            findPerfectBtn.addEventListener('click', () => this.startPerfectBalanceExploration());
        }
        
        if (findNearBtn) {
            findNearBtn.addEventListener('click', () => this.startNearPerfectExploration());
        }
        
        if (generateReportBtn) {
            generateReportBtn.addEventListener('click', () => this.generatePerfectBalanceReport());
        }
        
        if (stopBtn) {
            stopBtn.addEventListener('click', () => this.stopExploration());
        }
    }
    
    /**
     * Setup database management event listeners
     */
    setupDatabaseEvents() {
        const exportBtn = document.getElementById('exportBtn');
        const importBtn = document.getElementById('importBtn');
        const clearBtn = document.getElementById('clearDbBtn');
        
        if (exportBtn) {
            exportBtn.addEventListener('click', () => this.exportDatabase());
        }
        
        if (importBtn) {
            importBtn.addEventListener('click', () => this.importDatabase());
        }
        
        if (clearBtn) {
            clearBtn.addEventListener('click', () => this.clearDatabase());
        }
    }
    
    /**
     * Setup search and filter event listeners
     */
    setupSearchEvents() {
        const searchInput = document.getElementById('searchInput');
        const filterSelect = document.getElementById('filterSelect');
        const sortBtn = document.getElementById('sortBtn');
        
        if (searchInput) {
            searchInput.addEventListener('input', () => this.handleSearch());
        }
        
        if (filterSelect) {
            filterSelect.addEventListener('change', () => this.handleFilter());
        }
        
        if (sortBtn) {
            sortBtn.addEventListener('click', () => this.toggleSort());
        }
    }
    
    /**
     * Parse universal pattern input
     */
    parseUniversalInput() {
        const input = document.getElementById('universalInput');
        if (!input) return;
        
        const inputValue = input.value.trim();
        if (!inputValue) {
            alert('Please enter a pattern to parse');
            return;
        }
        
        try {
            console.log(`🎵 Parsing pattern: ${inputValue}`);
            
            // Use UnifiedPatternParser to parse the input
            const result = UnifiedPatternParser.parse(inputValue);
            
            if (result.type === 'single') {
                this.currentPattern = result.pattern;
                this.displayPatternAnalysis(this.currentPattern);
                this.showCompactOutput(this.currentPattern);
                console.log('✅ Pattern parsed successfully');
            } else if (result.type === 'combination') {
                this.currentPattern = result.combined;
                this.displayPatternAnalysis(this.currentPattern);
                this.showCompactOutput(this.currentPattern);
                console.log('✅ Combined pattern parsed successfully');
            } else {
                alert('Failed to parse pattern: Unknown result type');
                console.error('❌ Pattern parsing failed: Unknown result type');
            }
            
        } catch (error) {
            console.error('❌ Pattern parsing error:', error);
            alert('Error parsing pattern: ' + error.message);
        }
    }
    
    /**
     * Display comprehensive pattern analysis
     */
    displayPatternAnalysis(pattern) {
        const analysisDisplay = document.getElementById('analysisDisplay');
        if (!analysisDisplay) return;
        
        try {
            // Generate comprehensive analysis
            const analysis = this.generateComprehensiveAnalysis(pattern);
            
            // Update analysis display
            analysisDisplay.innerHTML = `
                <div class="analysis-title">${analysis.title}</div>
                <div class="analysis-content">${analysis.content}</div>
            `;
            
        } catch (error) {
            console.error('❌ Analysis display error:', error);
            analysisDisplay.innerHTML = `
                <div class="analysis-title">Analysis Error</div>
                <div class="analysis-content">Failed to generate analysis: ${error.message}</div>
            `;
        }
    }
    
    /**
     * Generate comprehensive mathematical analysis
     */
    generateComprehensiveAnalysis(pattern) {
        let content = '<div class="analysis-section">';
        
        // Basic properties
        content += '<div class="analysis-item">';
        content += '<div class="analysis-title">📊 Basic Properties</div>';
        content += '<div class="analysis-content">';
        const binary = PatternConverter.toBinary(pattern.steps, pattern.stepCount);
        const decimal = PatternConverter.toDecimal(binary);
        const hex = PatternConverter.toHex(decimal);
        content += `<strong>Pattern:</strong> ${binary}<br>`;
        content += `<strong>Hex:</strong> ${hex}<br>`;
        content += `<strong>Decimal:</strong> ${decimal}<br>`;
        content += `<strong>Steps:</strong> ${pattern.stepCount}<br>`;
        content += `<strong>Beats:</strong> ${pattern.steps.filter(step => step).length}<br>`;
        content += `<strong>Density:</strong> ${((pattern.steps.filter(step => step).length / pattern.stepCount) * 100).toFixed(1)}%`;
        content += '</div></div>';
        
        // Structure analysis
        const structureAnalysis = PatternAnalyzer.analyzeStructure(pattern.steps, pattern.stepCount);
        content += '<div class="analysis-item structure">';
        content += '<div class="analysis-title">📐 Structure Analysis</div>';
        content += '<div class="analysis-content">';
        content += `<strong>Step Count:</strong> ${structureAnalysis.stepCount}<br>`;
        content += `<strong>Onset Count:</strong> ${structureAnalysis.onsetCount}<br>`;
        content += `<strong>Density:</strong> ${structureAnalysis.density.toFixed(1)}%<br>`;
        content += `<strong>Maximum Silence Gap:</strong> ${structureAnalysis.maxSilenceGap} steps<br>`;
        content += `<strong>Average Onset Interval:</strong> ${structureAnalysis.avgOnsetInterval.toFixed(2)} steps`;
        content += '</div></div>';
        
        // Repetition analysis
        const repetitionAnalysis = PatternAnalyzer.detectRepetition(pattern.steps, pattern.stepCount);
        if (repetitionAnalysis) {
            content += '<div class="analysis-item repetition">';
            content += '<div class="analysis-title">🔄 Repetition Analysis</div>';
            content += '<div class="analysis-content">';
            content += `<strong>Repetitive Pattern Detected!</strong><br>`;
            content += `<strong>Unit Length:</strong> ${repetitionAnalysis.unitLength} (pattern "${repetitionAnalysis.unitBinary}" repeats ${repetitionAnalysis.repetitions} times)<br>`;
            content += `<strong>Compression Ratio:</strong> ${repetitionAnalysis.compressionRatio}:1<br>`;
            content += `<strong>Unit Pattern:</strong> ${repetitionAnalysis.unitBinary} (${repetitionAnalysis.unitHex}, ${repetitionAnalysis.unitDecimal})<br>`;
            content += `<strong>Regularity Score:</strong> High (perfect repetition)`;
            content += '</div></div>';
        }
        
        // Perfect balance analysis
        const balanceAnalysis = PerfectBalanceAnalyzer.calculateBalance(pattern.steps, pattern.stepCount);
        content += '<div class="analysis-item perfect-balance">';
        content += '<div class="analysis-title">🎼 Perfect Balance Analysis (Milne)</div>';
        content += '<div class="analysis-content">';
        content += `<div class="mathematical-formula">|∑(e^(i2πkⱼ/n))| / onsets = ${balanceAnalysis.normalizedMagnitude.toFixed(6)}</div>`;
        content += `<strong>Balance Quality:</strong> `;
        content += `<span class="balance-score balance-${balanceAnalysis.balanceScore.toLowerCase()}">${balanceAnalysis.balanceScore}</span><br>`;
        content += `<strong>Status:</strong> ${balanceAnalysis.isPerfectlyBalanced ? '✨ PERFECTLY BALANCED - Center of gravity at origin!' : 'Not perfectly balanced'}`;
        content += '</div></div>';
        
        // Pattern type detection
        if (pattern.isRegularPolygon) {
            content += '<div class="analysis-item polygon">';
            content += '<div class="analysis-title">🔺 Regular Polygon Detected</div>';
            content += '<div class="analysis-content">';
            content += `<div class="mathematical-formula">P(${pattern.vertices},${pattern.offset || 0}): ${pattern.polygonType}</div>`;
            content += `<strong>${pattern.polygonType}</strong> with ${pattern.vertices} vertices<br>`;
            if (pattern.expansion && pattern.expansion > 1) {
                content += `Expansion factor: ×${pattern.expansion}<br>`;
            }
            if (pattern.offset) {
                content += `Rotational offset: ${pattern.offset} steps`;
            }
            content += '</div></div>';
        }
        
        if (pattern.isEuclidean) {
            content += '<div class="analysis-item euclidean">';
            content += '<div class="analysis-title">🌀 Euclidean Rhythm Detected</div>';
            content += '<div class="analysis-content">';
            content += `<div class="mathematical-formula">${pattern.formula}: Bjorklund's Algorithm</div>`;
            content += `<strong>Euclidean Rhythm</strong> distributing ${pattern.beats} beats across ${pattern.steps} steps<br>`;
            if (pattern.offset) {
                content += `Offset: ${pattern.offset} steps`;
            }
            content += '</div></div>';
        }
        
        // Center of gravity analysis
        const cogAnalysis = CenterOfGravityCalculator.calculateCenterOfGravity(pattern.steps);
        content += '<div class="analysis-item center-gravity">';
        content += '<div class="analysis-title">📐 Center of Gravity Analysis</div>';
        content += '<div class="analysis-content">';
        content += UIComponents.renderCogVisualization(cogAnalysis, pattern.stepCount);
        content += '</div></div>';
        
        content += '</div>';
        
        return {
            title: 'Mathematical Pattern Analysis',
            content: content
        };
    }
    
    /**
     * Show compact pattern output
     */
    showCompactOutput(pattern) {
        const compactOutput = document.getElementById('compactOutput');
        const compactResult = document.getElementById('compactResult');
        
        if (!compactOutput || !compactResult) return;
        
        const compact = UnifiedPatternParser.formatCompact(pattern);
        compactResult.innerHTML = compact;
        compactOutput.style.display = 'block';
    }
    
    /**
     * Add current pattern to database
     */
    addCurrentPatternToDatabase() {
        if (!this.currentPattern) {
            alert(AppConfig.MESSAGES.ERRORS.NO_PATTERN_ADD);
            return;
        }
        
        try {
            // Calculate perfect balance analysis for database storage
            const perfectBalance = PerfectBalanceAnalyzer.calculateBalance(this.currentPattern.steps, this.currentPattern.stepCount);
            
            // Create database pattern with analysis
            const databasePattern = createDatabasePattern(this.currentPattern, { perfectBalance });
            
            const patternId = this.database.add(databasePattern);
            console.log(`✅ Pattern added to database with ID: ${patternId}`);
            alert(`✅ Pattern added to database with ID: ${patternId}`);
            
            // Update UI
            this.updatePatternList();
            this.updateDatabaseStats();
            
        } catch (error) {
            console.error('❌ Failed to add pattern:', error);
            alert('Failed to add pattern: ' + error.message);
        }
    }
    
    /**
     * Copy current pattern hex to clipboard
     */
    copyCurrentPatternHex() {
        if (!this.currentPattern) {
            alert(AppConfig.MESSAGES.ERRORS.NO_PATTERN);
            return;
        }
        
        const binary = PatternConverter.toBinary(this.currentPattern.steps, this.currentPattern.stepCount);
        const decimal = PatternConverter.toDecimal(binary);
        const hex = PatternConverter.toHex(decimal);
        UIComponents.copyToClipboard(hex);
    }
    
    /**
     * Run test examples
     */
    runTestExamples() {
        const examples = [
            'P(3,1)+P(5,0)+P(2,5)',
            'P(3,0)+P(5,1)-P(2,0)',
            'E(5,8,0)',
            '0x92',
            'b101010'
        ];
        
        const randomExample = examples[Math.floor(Math.random() * examples.length)];
        const input = document.getElementById('universalInput');
        if (input) {
            input.value = randomExample;
            this.parseUniversalInput();
        }
    }
    
    /**
     * Start perfect balance exploration
     */
    async startPerfectBalanceExploration() {
        const minSides = parseInt(document.getElementById('minSides')?.value || '3');
        const maxSides = parseInt(document.getElementById('maxSides')?.value || '7');
        const maxCombinations = parseInt(document.getElementById('maxCombinations')?.value || '3');
        
        if (minSides > maxSides) {
            alert(AppConfig.MESSAGES.ALERTS.MIN_MAX_SIDES);
            return;
        }
        
        try {
            this.isExploring = true;
            this.showExplorationProgress();
            
            console.log(`🔍 Starting perfect balance exploration: sides(${minSides}-${maxSides}), combinations(${maxCombinations})`);
            
            const results = await this.explorer.exploreAllCombinations(
                minSides, maxSides, maxCombinations, 'perfect'
            );
            
            this.explorationResults = results;
            console.log(`✅ Exploration complete: ${results.length} perfect balance patterns found`);
            
            // Add results to database
            let addedCount = 0;
            results.forEach(result => {
                try {
                    // Create database pattern with perfect balance analysis
                    const databasePattern = createDatabasePattern(result.pattern, { perfectBalance: result.balance });
                    const patternId = this.database.add(databasePattern);
                    if (patternId) {
                        addedCount++;
                    }
                } catch (error) {
                    // Pattern might already exist, that's okay
                }
            });
            
            this.updatePatternList();
            this.updateDatabaseStats();
            this.hideExplorationProgress();
            
            alert(`✅ Exploration complete: ${results.length} perfect balance patterns found, ${addedCount} new patterns added to database`);
            
        } catch (error) {
            console.error('❌ Exploration failed:', error);
            alert('Exploration failed: ' + error.message);
        } finally {
            this.isExploring = false;
            this.hideExplorationProgress();
        }
    }
    
    /**
     * Start near perfect balance exploration
     */
    async startNearPerfectExploration() {
        const minSides = parseInt(document.getElementById('minSides')?.value || '3');
        const maxSides = parseInt(document.getElementById('maxSides')?.value || '7');
        const maxCombinations = parseInt(document.getElementById('maxCombinations')?.value || '3');
        
        if (minSides > maxSides) {
            alert(AppConfig.MESSAGES.ALERTS.MIN_MAX_SIDES);
            return;
        }
        
        try {
            this.isExploring = true;
            this.showExplorationProgress();
            
            console.log(`🔍 Starting near-perfect balance exploration: sides(${minSides}-${maxSides}), combinations(${maxCombinations})`);
            
            const results = await this.explorer.exploreAllCombinations(
                minSides, maxSides, maxCombinations, 'near'
            );
            
            this.explorationResults = results;
            console.log(`✅ Exploration complete: ${results.length} near-perfect balance patterns found`);
            
            // Add results to database
            let addedCount = 0;
            results.forEach(result => {
                try {
                    // Create database pattern with perfect balance analysis
                    const databasePattern = createDatabasePattern(result.pattern, { perfectBalance: result.balance });
                    const patternId = this.database.add(databasePattern);
                    if (patternId) {
                        addedCount++;
                    }
                } catch (error) {
                    // Pattern might already exist, that's okay
                }
            });
            
            this.updatePatternList();
            this.updateDatabaseStats();
            this.hideExplorationProgress();
            
            alert(`✅ Exploration complete: ${results.length} near-perfect balance patterns found, ${addedCount} new patterns added to database`);
            
        } catch (error) {
            console.error('❌ Exploration failed:', error);
            alert('Exploration failed: ' + error.message);
        } finally {
            this.isExploring = false;
            this.hideExplorationProgress();
        }
    }
    
    /**
     * Generate perfect balance report
     */
    generatePerfectBalanceReport() {
        const perfectBalancePatterns = this.database.patterns.filter(pattern => {
            const analysis = PerfectBalanceAnalyzer.calculateBalance(pattern.steps, pattern.stepCount);
            return analysis.isPerfectlyBalanced;
        });
        
        if (perfectBalancePatterns.length === 0) {
            alert(AppConfig.MESSAGES.ALERTS.NO_PERFECT_BALANCE);
            return;
        }
        
        // Generate and display report
        console.log(`📊 Generating report for ${perfectBalancePatterns.length} perfect balance patterns`);
        
        const stats = this.database.getStatistics();
        
        const report = `
=== PERFECT BALANCE ANALYSIS REPORT ===
Generated: ${new Date().toLocaleString()}

DATABASE STATISTICS:
• Total Patterns: ${stats.totalPatterns}
• Perfect Balance Patterns: ${perfectBalancePatterns.length}
• Favorites: ${stats.favorites}
• Polygon Patterns: ${stats.polygons}
• Euclidean Patterns: ${stats.euclidean}
• Combined Patterns: ${stats.combined}

PERFECT BALANCE PATTERNS FOUND:
${perfectBalancePatterns.map((pattern, index) => {
    const compact = UnifiedPatternParser.formatCompact(pattern);
    return `${index + 1}. ${pattern.name || 'Unnamed'}\n   ${compact}`;
}).join('\n\n')}

=== END REPORT ===
`;

        // Show report in modal
        UIComponents.showReportModal(report);
    }
    
    /**
     * Stop current exploration
     */
    stopExploration() {
        if (this.isExploring) {
            this.explorer.stop();
            this.isExploring = false;
            this.hideExplorationProgress();
            console.log('🛑 Exploration stopped by user');
        }
    }
    
    /**
     * Export database
     */
    exportDatabase() {
        try {
            const exportData = this.database.export();
            const blob = new Blob([exportData], { type: 'application/json' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `pattern-database-${new Date().toISOString().split('T')[0]}.json`;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            URL.revokeObjectURL(url);
            console.log('📤 Database exported successfully');
        } catch (error) {
            console.error('❌ Export failed:', error);
            alert('Export failed: ' + error.message);
        }
    }
    
    /**
     * Import database
     */
    importDatabase() {
        UIComponents.showModal('Import Database', `
            <div class="input-group">
                <label class="input-label">Paste JSON data:</label>
                <textarea class="input" id="importData" placeholder="${AppConfig.UI.PLACEHOLDERS.IMPORT_DATA}" rows="10"></textarea>
            </div>
            <div class="btn-group">
                <button class="btn primary" onclick="app.processImport()">Import</button>
                <button class="btn secondary" onclick="UIComponents.hideModal()">Cancel</button>
            </div>
        `);
    }
    
    /**
     * Process database import
     */
    processImport() {
        const importData = document.getElementById('importData')?.value;
        if (!importData) return;
        
        try {
            this.database.import(importData);
            this.updatePatternList();
            this.updateDatabaseStats();
            UIComponents.hideModal();
            alert(AppConfig.MESSAGES.SUCCESS.DATABASE_IMPORTED);
            console.log('📥 Database imported successfully');
        } catch (error) {
            console.error('❌ Import failed:', error);
            alert(AppConfig.MESSAGES.ERRORS.IMPORT_FAILED + ': ' + error.message);
        }
    }
    
    /**
     * Clear database
     */
    clearDatabase() {
        if (confirm(AppConfig.MESSAGES.ALERTS.CLEAR_CONFIRM)) {
            this.database.clear();
            this.updatePatternList();
            this.updateDatabaseStats();
            console.log('🗑️ Database cleared');
        }
    }
    
    /**
     * Handle search input
     */
    handleSearch() {
        this.updatePatternList();
    }
    
    /**
     * Handle filter change
     */
    handleFilter() {
        this.updatePatternList();
    }
    
    /**
     * Toggle sort order
     */
    toggleSort() {
        this.sortByDate = !this.sortByDate;
        this.updateSortButton();
        this.updatePatternList();
    }
    
    /**
     * Update pattern list display
     */
    updatePatternList() {
        const patternList = document.getElementById('patternList');
        if (!patternList) return;
        
        try {
            const searchTerm = document.getElementById('searchInput')?.value || '';
            const filterType = document.getElementById('filterSelect')?.value || 'all';
            
            let patterns = this.database.patterns || [];
            
            // Apply search filter if there's a search term
            if (searchTerm) {
                patterns = this.database.search(searchTerm);
            }
            
            // Apply type filter if specified
            if (filterType && filterType !== 'all') {
                patterns = this.database.filter(filterType);
                // If we have both search and filter, we need to apply both
                if (searchTerm) {
                    const searchResults = this.database.search(searchTerm);
                    patterns = patterns.filter(p => searchResults.some(sr => sr.id === p.id));
                }
            }
            const sortedPatterns = this.sortByDate ? 
                patterns.sort((a, b) => new Date(b.timestamp) - new Date(a.timestamp)) :
                patterns.sort((a, b) => a.stepCount - b.stepCount);
            
            patternList.innerHTML = sortedPatterns.map(pattern => 
                this.createPatternEntryHTML(pattern)
            ).join('');
            
        } catch (error) {
            console.error('❌ Failed to update pattern list:', error);
            patternList.innerHTML = '<div class="error-box">Failed to load patterns</div>';
        }
    }
    
    /**
     * Get pattern representations for display
     */
    getPatternRepresentations(pattern) {
        const representations = [];
        
        // Binary representation
        const binary = PatternConverter.toBinary(pattern.steps, pattern.stepCount);
        representations.push(`b${binary}`);
        
        // Decimal representation
        const decimal = PatternConverter.toDecimal(binary);
        representations.push(`${decimal}`);
        
        // Hex representation
        const hex = PatternConverter.toHex(decimal);
        representations.push(`${hex}`);
        
        // Enhanced notation if available
        if (pattern.stepCount) {
            representations.push(`${decimal}:${pattern.stepCount}`);
        }
        
        return representations;
    }

    /**
     * Create HTML for pattern entry
     */
    createPatternEntryHTML(pattern) {
        const analysis = PerfectBalanceAnalyzer.calculateBalance(pattern.steps, pattern.stepCount);
        const representations = this.getPatternRepresentations(pattern);
        
        return `
            <div class="pattern-entry ${analysis.isPerfectlyBalanced ? 'perfect-balance' : ''} ${pattern.favorite ? 'favorite' : ''}">
                <span class="pattern-star ${pattern.favorite ? 'active' : ''}" onclick="app.toggleFavorite('${pattern.id}')">★</span>
                <div class="pattern-info">
                    <div class="pattern-name ${!pattern.name ? 'unnamed' : ''}" onclick="app.editPatternName('${pattern.id}')">
                        ${pattern.name || AppConfig.UI.PLACEHOLDERS.NO_NAME}
                    </div>
                    <div class="pattern-representations">
                        ${representations.map(repr => `<span class="pattern-repr">${repr}</span>`).join('')}
                        ${analysis.isPerfectlyBalanced ? '<span class="pattern-repr perfect-balance-badge">✨ Perfect Balance</span>' : ''}
                        ${pattern.isRegularPolygon ? `<span class="pattern-repr polygon-type">🔺 ${pattern.polygonType}</span>` : ''}
                        ${pattern.isEuclidean ? `<span class="pattern-repr euclidean-type">🌀 ${pattern.formula}</span>` : ''}
                        ${(() => {
                            // Get original patterns from either structure
                            const originalPatterns = pattern.isCombined ? 
                                (pattern.combined?.originalPatterns || pattern.originalPatterns) : null;
                            
                            if (!originalPatterns) return '';
                            
                            return originalPatterns.map(p => {
                                if (p.isRegularPolygon) {
                                    return `<span class="pattern-repr polygon-type">🔺 ${p.polygonType}</span>`;
                                } else if (p.isEuclidean) {
                                    return `<span class="pattern-repr euclidean-type">🌀 ${p.formula}</span>`;
                                }
                                return '';
                            }).join('');
                        })()}
                        ${(() => {
                            // Get original patterns from either structure
                            const originalPatterns = pattern.isCombined ? 
                                (pattern.combined?.originalPatterns || pattern.originalPatterns) : null;
                            
                            if (!originalPatterns) return '';
                            
                            const combinationText = originalPatterns.map(p => {
                                if (p.isRegularPolygon) {
                                    return `P(${p.vertices},${p.offset || 0}${p.expansion && p.expansion !== 1 ? ',' + p.expansion : ''})`;
                                } else if (p.isEuclidean) {
                                    return `E(${p.beats},${p.stepCount},${p.offset || 0})`;
                                } else if (p.binary) {
                                    return `b${p.binary}`;
                                } else {
                                    return `${p.stepCount}steps`;
                                }
                            }).join(' + ');
                            
                            return `<span class="pattern-repr combined-type" style="background: #e3f2fd; color: #1976d2; font-weight: bold; border-left: 3px solid #2196f3; padding: 2px 6px;">🎯 COMBINED: ${combinationText}</span>`;
                        })()}
                    </div>
                </div>
                <div class="pattern-actions">
                    <button class="btn btn-sm success" onclick="app.loadPattern('${pattern.id}')">Load</button>
                    <button class="btn btn-sm danger" onclick="app.deletePattern('${pattern.id}')">Delete</button>
                </div>
            </div>
        `;
    }
    
    /**
     * Update database statistics
     */
    updateDatabaseStats() {
        const stats = this.database.getStatistics();
        
        // Calculate additional stats that aren't provided by the database
        const allPatterns = this.database.patterns || [];
        const perfectBalancePatterns = allPatterns.filter(pattern => {
            if (pattern.perfectBalance && pattern.perfectBalance.isPerfectlyBalanced) {
                return true;
            }
            // For patterns without pre-calculated perfect balance, calculate it
            try {
                const analysis = PerfectBalanceAnalyzer.calculateBalance(pattern.steps, pattern.stepCount);
                return analysis.isPerfectlyBalanced;
            } catch (e) {
                return false;
            }
        });
        
        const wellformedPatterns = allPatterns.filter(pattern => {
            try {
                const analysis = PerfectBalanceAnalyzer.calculateBalance(pattern.steps, pattern.stepCount);
                return analysis.balanceScore === 'excellent' || analysis.balanceScore === 'good';
            } catch (e) {
                return false;
            }
        });
        
        const avgCog = allPatterns.length > 0 ? 
            allPatterns.reduce((sum, pattern) => {
                try {
                    const cogAnalysis = CenterOfGravityCalculator.calculateCenterOfGravity(pattern.steps);
                    return sum + cogAnalysis.normalizedMagnitude;
                } catch (e) {
                    return sum;
                }
            }, 0) / allPatterns.length : 0;
        
        // Merge with database stats
        stats.perfectBalance = perfectBalancePatterns.length;
        stats.wellformed = wellformedPatterns.length;
        stats.avgCog = avgCog;
        
        const updateElement = (id, value) => {
            const element = document.getElementById(id);
            if (element) {
                element.textContent = value;
            }
        };
        
        updateElement('totalPatterns', stats.totalPatterns || '0');
        updateElement('perfectBalanceCount', stats.perfectBalance || '0');
        updateElement('favoriteCount', stats.favorites || '0');
        updateElement('polygonCount', stats.polygons || '0');
        updateElement('euclideanCount', stats.euclidean || '0');
        updateElement('combinedCount', stats.combined || '0');
        updateElement('wellformedCount', stats.wellformed || '0');
        updateElement('avgCogValue', (stats.avgCog || 0).toFixed(3));
    }
    
    /**
     * Update sort button text
     */
    updateSortButton() {
        const sortBtn = document.getElementById('sortBtn');
        if (sortBtn) {
            sortBtn.textContent = this.sortByDate ? 
                AppConfig.UI.TEXT.BUTTON_LABELS.SORT_BY_STEPS : 
                AppConfig.UI.TEXT.BUTTON_LABELS.SORT_BY_DATE;
        }
    }
    
    /**
     * Show exploration progress
     */
    showExplorationProgress() {
        const progressContainer = document.getElementById('progressContainer');
        const stopBtn = document.getElementById('stopBtn');
        const findPerfectBtn = document.getElementById('findPerfectBtn');
        const findNearBtn = document.getElementById('findNearBtn');
        
        if (progressContainer) progressContainer.style.display = 'block';
        if (stopBtn) stopBtn.style.display = 'inline-block';
        if (findPerfectBtn) findPerfectBtn.style.display = 'none';
        if (findNearBtn) findNearBtn.style.display = 'none';
    }
    
    /**
     * Hide exploration progress
     */
    hideExplorationProgress() {
        const progressContainer = document.getElementById('progressContainer');
        const stopBtn = document.getElementById('stopBtn');
        const findPerfectBtn = document.getElementById('findPerfectBtn');
        const findNearBtn = document.getElementById('findNearBtn');
        
        if (progressContainer) progressContainer.style.display = 'none';
        if (stopBtn) stopBtn.style.display = 'none';
        if (findPerfectBtn) findPerfectBtn.style.display = 'inline-block';
        if (findNearBtn) findNearBtn.style.display = 'inline-block';
    }
    
    /**
     * Update exploration progress
     */
    updateExplorationProgress(progress) {
        const progressFill = document.getElementById('progressFill');
        const progressText = document.getElementById('progressText');
        
        if (progressFill) {
            progressFill.style.width = `${progress.percentage}%`;
        }
        
        if (progressText) {
            progressText.textContent = progress.message || 'Searching...';
        }
    }
    
    /**
     * Reset exploration controls
     */
    resetExplorationControls() {
        const minSides = document.getElementById('minSides');
        const maxSides = document.getElementById('maxSides');
        const maxCombinations = document.getElementById('maxCombinations');
        
        if (minSides) minSides.value = AppConfig.DEFAULTS.EXPLORATION.MIN_SIDES;
        if (maxSides) maxSides.value = AppConfig.DEFAULTS.EXPLORATION.MAX_SIDES;
        if (maxCombinations) maxCombinations.value = AppConfig.DEFAULTS.EXPLORATION.MAX_COMBINATIONS;
    }
    
    /**
     * Toggle pattern favorite status
     */
    toggleFavorite(patternId) {
        const pattern = this.database.getById(patternId);
        if (pattern) {
            this.database.update(patternId, { favorite: !pattern.favorite });
        }
        this.updatePatternList();
        this.updateDatabaseStats();
    }
    
    /**
     * Edit pattern name
     */
    editPatternName(patternId) {
        const pattern = this.database.getById(patternId);
        if (!pattern) return;
        
        const newName = prompt('Enter pattern name:', pattern.name || '');
        if (newName !== null) {
            this.database.update(patternId, { name: newName });
            this.updatePatternList();
        }
    }
    
    /**
     * Load pattern into input
     */
    loadPattern(patternId) {
        const pattern = this.database.getById(patternId);
        if (!pattern) return;
        
        const input = document.getElementById('universalInput');
        if (input) {
            const binary = PatternConverter.toBinary(pattern.steps, pattern.stepCount);
            const decimal = PatternConverter.toDecimal(binary);
            input.value = PatternConverter.toHex(decimal);
            this.parseUniversalInput();
        }
    }
    
    /**
     * Delete pattern
     */
    deletePattern(patternId) {
        if (confirm('Delete this pattern?')) {
            this.database.remove(patternId);
            this.updatePatternList();
            this.updateDatabaseStats();
        }
    }
}

// Export for global access
window.EnhancedPatternApp = EnhancedPatternApp;

console.log('✅ Enhanced Pattern App Controller loaded (browser-compatible version)');