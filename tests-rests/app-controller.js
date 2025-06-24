/**
 * App Controller Module (app-controller.js)
 * Main application orchestrator for the Rhythm Pattern Explorer
 * 
 * Features:
 * - EnhancedPatternApp class - Central application controller
 * - Event handling setup and management
 * - Workflow coordination between all modules
 * - UI state management and updates
 * - Application initialization and error handling
 * 
 * Dependencies: ALL other modules
 * - math-core.js (MathUtils)
 * - pattern-generators.js (RegularPolygonGenerator, EuclideanGenerator)
 * - pattern-analysis.js (PerfectBalanceAnalyzer, PatternAnalyzer, CenterOfGravityCalculator)
 * - pattern-processing.js (AdvancedPatternCombiner, UnifiedPatternParser, PatternConverter)
 * - pattern-exploration.js (SystematicExplorer)
 * - pattern-database.js (PatternDatabase)
 * - ui-components.js (UIComponents)
 */

// Import dependencies
import { MathUtils } from './math-core.js';
import { RegularPolygonGenerator, EuclideanGenerator } from './pattern-generators.js';
import { PerfectBalanceAnalyzer, PatternAnalyzer, CenterOfGravityCalculator } from './pattern-analysis.js';
import { AdvancedPatternCombiner, UnifiedPatternParser, PatternConverter } from './pattern-processing.js';
import { SystematicExplorer } from './pattern-exploration.js';
import { PatternDatabase } from './pattern-database.js';
import { UIComponents } from './ui-components.js';

/**
 * Enhanced Pattern Application Controller
 * 
 * Main orchestrator class that coordinates all subsystems:
 * - Pattern processing and analysis
 * - Database operations and rendering
 * - Perfect balance exploration
 * - User interface management
 * - Event handling and workflow coordination
 */
export class EnhancedPatternApp {
    constructor() {
        // Initialize core subsystems
        this.database = new PatternDatabase();
        this.explorer = new SystematicExplorer();
        
        // Application state
        this.currentPattern = null;
        this.sortByDate = true;
        this.explorationResults = [];
        
        // Setup application
        this.setupEventListeners();
        this.initialize();
    }
    
    /**
     * Initialize application
     * - Load database from storage
     * - Render initial UI state
     * - Update statistics
     * - Show help text and generate sample pattern
     */
    initialize() {
        this.database.load();
        this.renderDatabase();
        this.updateStats();
        
        // Show initial help
        this.updateAnalysisDisplay(null);
        
        // Generate a sample pattern
        this.parseUniversalInput('P(3,1)+P(5,0)+P(2,5)', false);
    }
    
    // === UNIVERSAL PATTERN PROCESSING ===
    
    /**
     * Parse universal input and process pattern
     * Handles all pattern formats: polygon, euclidean, binary, hex, decimal, combinations
     * 
     * @param {string} input - Pattern input string
     * @param {boolean} interactive - Whether to show UI feedback
     * @returns {boolean} Success status
     */
    parseUniversalInput(input, interactive = true) {
        try {
            const parsed = UnifiedPatternParser.parse(input || document.getElementById('universalInput').value);
            
            if (parsed.type === 'advanced_combination') {
                // Handle advanced combination with subtraction
                this.currentPattern = parsed.combined;
                this.currentPattern.expression = parsed.expression;
            } else if (parsed.type === 'combination') {
                // Handle regular combination
                this.currentPattern = parsed.combined;
            } else {
                // Handle single pattern
                this.currentPattern = parsed.pattern;
            }
            
            if (interactive) {
                this.clearErrors();
                this.showCompactOutput(parsed);
                this.updateAnalysisDisplay(this.currentPattern);
            }
            
            return true;
        } catch (error) {
            if (interactive) {
                alert(error.message);
                this.showError();
            }
            console.error('Parse error:', error);
            return false;
        }
    }
    
    /**
     * Display compact pattern output with formatted representations
     * @param {Object} parsed - Parsed pattern object
     */
    showCompactOutput(parsed) {
        const output = document.getElementById('compactOutput');
        const result = document.getElementById('compactResult');
        
        if (parsed.type === 'advanced_combination') {
            const combinedCompact = UnifiedPatternParser.formatCompact(parsed.combined);
            result.innerHTML = `
                <div><strong>Advanced Expression:</strong> ${parsed.expression}</div>
                <div style="margin-top: 8px;"><strong>Result:</strong> ${combinedCompact}</div>
            `;
        } else if (parsed.type === 'combination') {
            if (parsed.multiPattern) {
                // Multi-pattern combination
                let html = '';
                parsed.patterns.forEach((pattern, index) => {
                    const patternCompact = UnifiedPatternParser.formatCompact(pattern);
                    html += `<div style="margin-bottom: 8px;"><strong>Pattern ${index + 1}:</strong> ${patternCompact}</div>`;
                });
                
                const combinedCompact = UnifiedPatternParser.formatCompact(parsed.combined);
                html += `<div style="margin-bottom: 8px; padding-top: 8px; border-top: 1px solid #dee2e6;"><strong>Combined (${parsed.patterns.length} patterns):</strong> ${combinedCompact}</div>`;
                
                result.innerHTML = html;
            } else {
                // Two-pattern combination
                const pattern1Compact = UnifiedPatternParser.formatCompact(parsed.patterns[0]);
                const pattern2Compact = UnifiedPatternParser.formatCompact(parsed.patterns[1]);
                const combinedCompact = UnifiedPatternParser.formatCompact(parsed.combined);
                
                result.innerHTML = `
                    <div style="margin-bottom: 8px;"><strong>Pattern 1:</strong> ${pattern1Compact}</div>
                    <div style="margin-bottom: 8px;"><strong>Pattern 2:</strong> ${pattern2Compact}</div>
                    <div style="margin-bottom: 8px; padding-top: 8px; border-top: 1px solid #dee2e6;"><strong>Combined:</strong> ${combinedCompact}</div>
                `;
            }
        } else {
            const patternCompact = UnifiedPatternParser.formatCompact(parsed.pattern);
            result.innerHTML = `<div>${patternCompact}</div>`;
        }
        
        output.style.display = 'block';
    }
    
    /**
     * Clear input error styling
     */
    clearErrors() {
        document.querySelectorAll('.input.error').forEach(input => {
            input.classList.remove('error');
        });
    }
    
    /**
     * Show input error styling
     */
    showError() {
        document.getElementById('universalInput').classList.add('error');
        document.getElementById('compactOutput').style.display = 'none';
    }
    
    /**
     * Copy current pattern hex value to clipboard
     */
    copyCurrentHex() {
        if (!this.currentPattern) {
            alert('No pattern to copy. Please parse a pattern first.');
            return;
        }
        
        const binary = PatternConverter.toBinary(this.currentPattern.steps, this.currentPattern.stepCount);
        const decimal = PatternConverter.toDecimal(binary);
        const hex = PatternConverter.toHex(decimal);
        
        this.copyToClipboard(hex);
    }
    
    // === PERFECT BALANCE EXPLORER ===
    
    /**
     * Start systematic exploration for perfect or near-perfect balance patterns
     * @param {string} targetBalance - 'perfect' or 'near'
     */
    async startSystematicExploration(targetBalance) {
        const minSides = parseInt(document.getElementById('minSides').value) || 3;
        const maxSides = parseInt(document.getElementById('maxSides').value) || 7;
        const maxCombinations = parseInt(document.getElementById('maxCombinations').value) || 3;
        
        if (minSides > maxSides) {
            alert('Min sides must be ≤ max sides');
            return;
        }
        
        // Show progress
        document.getElementById('progressContainer').style.display = 'block';
        document.getElementById('findPerfectBtn').disabled = true;
        document.getElementById('findNearBtn').disabled = true;
        document.getElementById('stopBtn').style.display = 'inline-block';
        
        try {
            console.log(`Starting ${targetBalance} balance exploration...`);
            const results = await this.explorer.exploreAllCombinations(minSides, maxSides, maxCombinations, targetBalance);
            
            // Sort results by balance quality
            results.sort((a, b) => {
                if (a.balance.isPerfectlyBalanced && !b.balance.isPerfectlyBalanced) return -1;
                if (!a.balance.isPerfectlyBalanced && b.balance.isPerfectlyBalanced) return 1;
                return a.balance.normalizedMagnitude - b.balance.normalizedMagnitude;
            });
            
            this.explorationResults = results;
            
            // Add all perfect balance results to database
            let addedCount = 0;
            for (const result of results.filter(r => r.balance.isPerfectlyBalanced)) {
                if (this.addExplorationResultToDatabase(result)) {
                    addedCount++;
                }
            }
            
            console.log(`Exploration complete: ${results.length} results found, ${addedCount} perfect balance patterns added to database`);
            
            if (addedCount > 0) {
                this.renderDatabase();
                this.updateStats();
                alert(`Found ${results.filter(r => r.balance.isPerfectlyBalanced).length} perfect balance patterns! ${addedCount} new patterns added to database.`);
            } else {
                alert(`Exploration complete: ${results.length} results found. ${results.filter(r => r.balance.isPerfectlyBalanced).length} perfect balance patterns (may already be in database).`);
            }
            
        } catch (error) {
            console.error('Exploration error:', error);
        } finally {
            this.stopExploration();
        }
    }
    
    /**
     * Stop exploration and reset UI
     */
    stopExploration() {
        this.explorer.stop();
        document.getElementById('progressContainer').style.display = 'none';
        document.getElementById('findPerfectBtn').disabled = false;
        document.getElementById('findNearBtn').disabled = false;
        document.getElementById('stopBtn').style.display = 'none';
    }
    
    /**
     * Update exploration statistics during progress
     * @param {Array} results - Current exploration results
     */
    updateExplorationStats(results) {
        const perfectCount = results.filter(r => r.balance.isPerfectlyBalanced).length;
        const nearCount = results.filter(r => r.balance.balanceScore === 'excellent' || r.balance.balanceScore === 'good').length;
        const avgBalance = results.length > 0 ? 
            (results.reduce((sum, r) => sum + r.balance.normalizedMagnitude, 0) / results.length).toFixed(3) : '0.000';
        
        // Update temporary stats display in progress text
        const progressText = document.getElementById('progressText');
        if (progressText && results.length > 0) {
            progressText.innerHTML += `<br>Found: ${perfectCount} perfect, ${nearCount} near-perfect`;
        }
    }
    
    /**
     * Add exploration result to database as pattern
     * @param {Object} result - Exploration result object
     * @returns {boolean} Success status
     */
    addExplorationResultToDatabase(result) {
        const { steps, stepCount } = result.combined;
        const binaryString = PatternConverter.toBinary(steps, stepCount);
        const decimal = PatternConverter.toDecimal(binaryString);
        
        // Create pattern name based on expression
        const defaultName = `Perfect Balance: ${result.expression}`;
        
        const pattern = {
            id: Date.now().toString(36) + Math.random().toString(36).substr(2),
            name: defaultName,
            favorite: false,
            stepCount,
            binary: binaryString,
            hex: PatternConverter.toHex(decimal),
            decimal,
            expression: result.expression,
            // Perfect balance information
            perfectBalance: {
                isPerfectlyBalanced: result.balance.isPerfectlyBalanced,
                magnitude: result.balance.magnitude,
                normalizedMagnitude: result.balance.normalizedMagnitude,
                complexSum: result.balance.complexSum,
                balanceScore: result.balance.balanceScore
            },
            // Pattern type information
            polygon: null, // Will be detected if needed
            euclidean: null,
            combined: {
                expression: result.expression,
                vertices: result.vertices,
                offsets: result.offsets,
                hasSubtraction: result.combined.hasSubtraction,
                lcmUsed: result.combined.stepCount
            },
            hasSubtraction: result.combined.hasSubtraction,
            // Coprime analysis
            coprime: result.coprime,
            // Geometric Center of Gravity analysis
            cogAnalysis: {
                magnitude: result.balance.magnitude,
                normalizedMagnitude: result.balance.normalizedMagnitude,
                geometricBalance: result.balance.geometricBalance || 'Unknown',
                isGeometricallyBalanced: result.balance.normalizedMagnitude < 0.1,
                cogPosition: result.balance.cogPosition || { x: 0, y: 0 }
            },
            timestamp: Date.now(),
            isFromExploration: true
        };
        
        return this.database.add(pattern);
    }
    
    /**
     * Generate perfect balance analysis report
     */
    generatePerfectBalanceReport() {
        const perfectBalancePatterns = this.database.patterns.filter(p => 
            p.perfectBalance && p.perfectBalance.isPerfectlyBalanced
        );
        
        if (perfectBalancePatterns.length === 0) {
            alert('No perfect balance patterns found in database. Run exploration first.');
            return;
        }
        
        let report = `# Perfect Balance Pattern Analysis Report\n\n`;
        report += `Generated: ${new Date().toLocaleString()}\n`;
        report += `Perfect balance patterns in database: ${perfectBalancePatterns.length}\n\n`;
        
        report += `## Perfect Balance Patterns\n\n`;
        perfectBalancePatterns.forEach((pattern, index) => {
            report += `### ${index + 1}. ${pattern.expression || pattern.name}\n`;
            report += `- Pattern: ${pattern.hex} (${pattern.stepCount} steps, ${pattern.perfectBalance.magnitude.toFixed(8)} balance magnitude)\n`;
            
            if (pattern.combined && pattern.combined.vertices) {
                report += `- Vertices: [${pattern.combined.vertices.join(', ')}]`;
                if (pattern.coprime) {
                    report += ` - ${pattern.coprime.areCoprimes ? 'Coprimes' : 'Not coprimes'}\n`;
                } else {
                    report += '\n';
                }
            }
            
            if (pattern.perfectBalance.complexSum) {
                const cs = pattern.perfectBalance.complexSum;
                report += `- Complex sum: (${cs.real.toFixed(6)}, ${cs.imag.toFixed(6)}i)\n`;
            }
            
            if (pattern.hasSubtraction) {
                report += `- Uses subtraction operation\n`;
            }
            
            report += `\n`;
        });
        
        // Show report in modal
        this.showReportModal(report);
    }
    
    // === ANALYSIS DISPLAY ===
    
    /**
     * Update analysis display with pattern information
     * @param {Object} pattern - Pattern object to analyze
     */
    updateAnalysisDisplay(pattern) {
        if (!pattern) {
            document.getElementById('analysisDisplay').innerHTML = `
                <div class="analysis-title">Mathematical Pattern Analysis</div>
                <div class="analysis-content">
                    Parse a pattern using the universal input above to see comprehensive mathematical analysis.<br><br>
                    <strong>Try these examples:</strong><br>
                    • P(3,1)+P(5,0)+P(2,5) - Fixed triple combination<br>
                    • P(3,0)+P(5,1)-P(2,0) - Perfect balance via subtraction<br>
                    • P(3,1,4) - Triangle×4<br>
                    • E(5,8,0) - Quintillo<br>
                    • 0x92 - Tresillo<br>
                    • 0x92>>3 - Tresillo rotated 3 steps<br>
                    • b101010 - Binary pattern<br>
                    • 146 - Decimal pattern
                </div>
            `;
            return;
        }
        
        this.analyzeCurrentPattern();
    }
    
    /**
     * Analyze current pattern and display comprehensive analysis
     */
    analyzeCurrentPattern() {
        if (!this.currentPattern) {
            this.updateAnalysisDisplay(null);
            return;
        }
        
        const { steps, stepCount } = this.currentPattern;
        const cogAnalysis = CenterOfGravityCalculator.calculateCircularStatistics(steps, stepCount);
        const repetition = PatternAnalyzer.findRepetition(steps, stepCount);
        const milne = PatternAnalyzer.analyzeMilneCriteria(steps, stepCount);
        const perfectBalance = PerfectBalanceAnalyzer.calculateBalance(steps, stepCount);
        
        // Detect pattern types
        const polygonDetection = RegularPolygonGenerator.detectPolygon(steps, stepCount);
        const euclideanDetection = UnifiedPatternParser.detectEuclideanPattern(steps, stepCount);
        
        let analysisHTML = '<div class="analysis-title">Mathematical Pattern Analysis</div>';
        
        // Basic info with integrated Milne information
        const beatCount = steps.filter(s => s).length;
        const density = ((beatCount / stepCount) * 100).toFixed(1);
        
        let basicInfo = `
            ${stepCount} steps, ${beatCount} beats (${density}% density)<br>
            Pattern: ${steps.map(s => s ? '●' : '∅').join('')}
        `;
        
        // Add onset positions and intervals if available
        if (milne.onsets && milne.onsets.length > 0) {
            basicInfo += `<br>Onsets: [${milne.onsets.join(', ')}]`;
            if (milne.intervals && milne.intervals.length > 0) {
                basicInfo += `<br>Intervals: [${milne.intervals.join(', ')}]`;
                if (milne.analysis) {
                    basicInfo += ` → ${milne.analysis}`;
                }
            }
            
            // Add well-formed/balanced status
            if (milne.wellformed || milne.perfectlyBalanced) {
                const statusParts = [];
                if (milne.wellformed) statusParts.push('⚖️ Well-formed');
                if (milne.perfectlyBalanced) statusParts.push('🎯 Perfectly balanced');
                if (statusParts.length > 0) {
                    basicInfo += `<br><strong>${statusParts.join(' & ')}</strong>`;
                }
            }
        }
        
        analysisHTML += `
            <div class="analysis-item">
                <div class="analysis-title">Basic Properties</div>
                <div class="analysis-content">
                    ${basicInfo}
                </div>
            </div>
        `;
        
        // Perfect Balance Analysis (NEW FEATURE)
        if (perfectBalance.onsetCount > 0) {
            const balanceClass = perfectBalance.isPerfectlyBalanced ? 'perfect-balance' : 'center-gravity';
            analysisHTML += `
                <div class="analysis-item ${balanceClass}">
                    <div class="analysis-title">🎼 Perfect Balance Analysis (Milne)</div>
                    <div class="analysis-content">
                        <div class="mathematical-formula">
                            |∑(e^(i2πkⱼ/n))| / onsets = ${perfectBalance.normalizedMagnitude.toFixed(6)}
                            <br>Complex sum: (${perfectBalance.complexSum.real.toFixed(3)}, ${perfectBalance.complexSum.imag.toFixed(3)}i)
                        </div>
                        <strong>Balance Quality:</strong> 
                        <span class="balance-score balance-${perfectBalance.balanceScore}">${perfectBalance.balanceScore.toUpperCase()}</span><br>
                        <strong>Status:</strong> ${perfectBalance.isPerfectlyBalanced ? 
                            '✨ PERFECTLY BALANCED - Center of gravity at origin!' : 
                            `Not perfectly balanced (magnitude: ${perfectBalance.magnitude.toFixed(6)})`}<br>
                        <strong>Geometric Balance:</strong> ${perfectBalance.geometricBalance || cogAnalysis.geometricBalance}
                        
                        ${perfectBalance.isPerfectlyBalanced ? 
                            '<div style="background: #e8f5e8; padding: 8px; border-radius: 4px; margin-top: 8px;"><strong>🎼 Milne Perfect Balance Achieved!</strong> This pattern satisfies the mathematical definition where the complex exponential sum equals zero, placing the center of gravity exactly at the origin.</div>' : 
                            ''
                        }
                    </div>
                </div>
            `;
        }
        
        // Advanced combination analysis
        if (this.currentPattern.isCombined || this.currentPattern.hasSubtraction) {
            const lcmUsed = this.currentPattern.lcmUsed || this.currentPattern.stepCount;
            
            analysisHTML += `
                <div class="analysis-item combined">
                    <div class="analysis-title">${this.currentPattern.hasSubtraction ? '➖ Advanced Combination with Subtraction' : '🎯 Pattern Combination'}</div>
                    <div class="analysis-content">
                        <div class="mathematical-formula">
                            ${this.currentPattern.expression ? 
                                `Expression: ${this.currentPattern.expression}` :
                                `LCM Combination: ${this.currentPattern.originalPatterns ? 
                                    'LCM(' + this.currentPattern.originalPatterns.map(p => p.stepCount).join(',') + ') = ' + lcmUsed :
                                    'Combined pattern'}`
                            }
                        </div>
                        ${this.currentPattern.expression ? 
                            `<strong>Expression:</strong> ${this.currentPattern.expression}<br>` : ''}
                        ${this.currentPattern.hasSubtraction ? 
                            '<strong>Uses Subtraction:</strong> Logical AND NOT operation for perfect balance<br>' : ''}
                        <strong>Result:</strong> ${stepCount} steps with logical operations<br>
                        ${this.currentPattern.vertices ? 
                            `<strong>Polygon vertices:</strong> [${this.currentPattern.vertices.join(', ')}]<br>` : ''}
                        ${this.currentPattern.offsets ? 
                            `<strong>Offsets:</strong> [${this.currentPattern.offsets.join(', ')}]<br>` : ''}
                        <small>This pattern represents ${this.currentPattern.hasSubtraction ? 'the subtraction-based combination' : 'the union'} of source patterns${this.currentPattern.hasSubtraction ? ', strategically designed for perfect balance' : ''}</small>
                    </div>
                </div>
            `;
        }
        
        // Rotation analysis
        if (this.currentPattern.isRotated) {
            analysisHTML += `
                <div class="analysis-item">
                    <div class="analysis-title">🔄 Pattern Rotation Applied</div>
                    <div class="analysis-content">
                        <strong>Rotated by ${this.currentPattern.appliedRotation} steps</strong><br>
                        Original offset: ${this.currentPattern.originalOffset}<br>
                        Total offset: ${this.currentPattern.offset}<br>
                        <small>Pattern has been rotated clockwise by ${this.currentPattern.appliedRotation} step${this.currentPattern.appliedRotation !== 1 ? 's' : ''}</small>
                    </div>
                </div>
            `;
        }
        
        // Polygon detection
        if (polygonDetection) {
            analysisHTML += `
                <div class="analysis-item polygon">
                    <div class="analysis-title">🔺 Regular Polygon Detected</div>
                    <div class="analysis-content">
                        <div class="mathematical-formula">
                            ${polygonDetection.formula}: ${polygonDetection.polygonType}
                        </div>
                        <strong>${polygonDetection.polygonType}</strong> with ${polygonDetection.vertices} vertices<br>
                        Expansion factor: ×${polygonDetection.expansion}<br>
                        Rotational offset: ${polygonDetection.offset} steps<br>
                        <small>This pattern maintains perfect ${polygonDetection.vertices}-fold rotational symmetry around the circle</small>
                    </div>
                </div>
            `;
        }
        
        // Euclidean detection
        if (euclideanDetection) {
            analysisHTML += `
                <div class="analysis-item euclidean">
                    <div class="analysis-title">🌀 Euclidean Rhythm Detected</div>
                    <div class="analysis-content">
                        <div class="mathematical-formula">
                            ${euclideanDetection.formula}: Bjorklund's Algorithm
                        </div>
                        <strong>Euclidean Rhythm</strong> distributing ${euclideanDetection.beats} beats across ${stepCount} steps<br>
                        Offset: ${euclideanDetection.offset} steps<br>
                        <small>This pattern distributes beats as evenly as possible using Bjorklund's algorithm</small>
                    </div>
                </div>
            `;
        }
        
        // Center of Gravity Analysis
        if (cogAnalysis.onsetCount > 0) {
            analysisHTML += `
                <div class="analysis-item center-gravity">
                    <div class="analysis-title">📐 Center of Gravity Analysis</div>
                    <div class="analysis-content">
                        <div class="mathematical-formula">
                            CoG = |Σ(e^(i2πk_j/n))| / onsets = ${cogAnalysis.normalizedMagnitude.toFixed(3)}
                        </div>
                        <div class="cog-visualization">
                            <div class="cog-circle">
                                ${this.renderCogVisualization(cogAnalysis, stepCount)}
                            </div>
                            <div>
                                <strong>${cogAnalysis.geometricBalance}</strong><br>
                                <small>Milne mathematical balance: ${cogAnalysis.isPerfectlyBalanced ? '✓' : '✗'}</small>
                            </div>
                        </div>
                    </div>
                </div>
            `;
        }
        
        // Repetition analysis
        if (repetition) {
            analysisHTML += `
                <div class="analysis-item repetition">
                    <div class="analysis-title">🔄 Pattern Repetition Detected</div>
                    <div class="analysis-content">
                        This pattern repeats <strong>${repetition.repetitions}×</strong> the unit: 
                        <span class="clickable-unit" data-binary="${repetition.unitBinary}">${repetition.unitBinary}</span>
                        <span class="clickable-unit" data-binary="${repetition.unitBinary}">${repetition.unitHex}</span>
                        <span class="clickable-unit" data-binary="${repetition.unitBinary}">${repetition.unitDecimal}</span><br>
                        Compression ratio: ${repetition.compressionRatio}:1 (${repetition.unitLength} → ${stepCount} steps)
                    </div>
                </div>
            `;
        }
        
        document.getElementById('analysisDisplay').innerHTML = analysisHTML;
        
        // Add click handlers for repetition units
        document.querySelectorAll('.clickable-unit').forEach(element => {
            element.addEventListener('click', () => {
                const binary = element.dataset.binary;
                document.getElementById('universalInput').value = `b${binary}`;
                this.parseUniversalInput();
            });
        });
    }
    
    /**
     * Render center of gravity visualization
     * @param {Object} cogAnalysis - Center of gravity analysis data
     * @param {number} stepCount - Total step count
     * @returns {string} HTML for visualization
     */
    renderCogVisualization(cogAnalysis, stepCount) {
        if (cogAnalysis.onsetCount === 0) return '';
        
        let html = '';
        
        // Render onset points around the circle
        for (const position of cogAnalysis.onsetPositions) {
            const angle = (2 * Math.PI * position / stepCount) - (Math.PI / 2);
            const x = 40 + 30 * Math.cos(angle);
            const y = 40 + 30 * Math.sin(angle);
            html += `<div class="onset-point" style="left: ${x}px; top: ${y}px;"></div>`;
        }
        
        // Render center of gravity point
        const cogX = 40 + 25 * cogAnalysis.cogPosition.x;
        const cogY = 40 + 25 * cogAnalysis.cogPosition.y;
        html += `<div class="cog-point" style="left: ${cogX}px; top: ${cogY}px;" title="Center of Gravity"></div>`;
        
        return html;
    }
    
    /**
     * Run test patterns to verify parser functionality
     */
    runTests() {
        const tests = [
            'P(3,1)+P(5,0)+P(2,5)',
            'P(3,0)+P(5,1)-P(2,0)',
            'P(3,1,4)',
            'P(4,0)+P(6,2)',
            'E(5,8,0)>>2',
            '0x92@3',
            'b101010',
            '146'
        ];
        
        let results = '<strong>Test Results:</strong><br><br>';
        
        tests.forEach(test => {
            try {
                const parsed = UnifiedPatternParser.parse(test);
                if (parsed.type === 'advanced_combination') {
                    results += `<strong>${test}</strong> → ${parsed.combined.stepCount} steps with subtraction ✓<br>`;
                } else if (parsed.type === 'combination') {
                    results += `<strong>${test}</strong> → LCM(${parsed.patterns.map(p => p.stepCount).join(',')}) = ${parsed.combined.stepCount} steps ✓<br>`;
                } else {
                    results += `<strong>${test}</strong> → ${parsed.pattern.stepCount} steps ✓<br>`;
                }
            } catch (error) {
                results += `<strong>${test}</strong> → ERROR: ${error.message} ✗<br>`;
            }
        });
        
        document.getElementById('compactResult').innerHTML = results;
        document.getElementById('compactOutput').style.display = 'block';
    }
    
    // === DATABASE OPERATIONS ===
    
    /**
     * Add current pattern to database with complete analysis
     */
    addCurrentToDatabase() {
        if (!this.currentPattern) {
            alert('No pattern to add. Please parse a pattern first.');
            return;
        }
        
        const { steps, stepCount } = this.currentPattern;
        const binaryString = PatternConverter.toBinary(steps, stepCount);
        const decimal = PatternConverter.toDecimal(binaryString);
        
        // Calculate all analyses
        const cogAnalysis = CenterOfGravityCalculator.calculateCircularStatistics(steps, stepCount);
        const repetition = PatternAnalyzer.findRepetition(steps, stepCount);
        const milne = PatternAnalyzer.analyzeMilneCriteria(steps, stepCount);
        const perfectBalance = PerfectBalanceAnalyzer.calculateBalance(steps, stepCount);
        const polygonDetection = RegularPolygonGenerator.detectPolygon(steps, stepCount);
        const euclideanDetection = UnifiedPatternParser.detectEuclideanPattern(steps, stepCount);
        
        // Analyze coprime properties if pattern has vertices
        let coprimeAnalysis = null;
        if (this.currentPattern.vertices && this.currentPattern.vertices.length > 0) {
            coprimeAnalysis = PerfectBalanceAnalyzer.analyzeCoprimeProperties(this.currentPattern.vertices);
        }
        
        // Create pattern name based on type
        let defaultName = '';
        if (this.currentPattern.expression) {
            defaultName = `Expression: ${this.currentPattern.expression}`;
        } else if (this.currentPattern.isCombined) {
            defaultName = `Combined (${this.currentPattern.originalPatterns ? this.currentPattern.originalPatterns.length : 'multi'} patterns)`;
        } else if (this.currentPattern.isRegularPolygon) {
            defaultName = `${this.currentPattern.polygonType} (${this.currentPattern.vertices},${this.currentPattern.expansion},${this.currentPattern.offset})`;
        } else if (this.currentPattern.isEuclidean) {
            defaultName = `Euclidean (${this.currentPattern.beats},${stepCount},${this.currentPattern.offset})`;
        } else if (polygonDetection) {
            defaultName = `${polygonDetection.polygonType} (${polygonDetection.vertices},${polygonDetection.expansion},${polygonDetection.offset})`;
        } else if (euclideanDetection) {
            defaultName = `Euclidean (${euclideanDetection.beats},${stepCount},${euclideanDetection.offset})`;
        }
        
        if (perfectBalance.isPerfectlyBalanced) {
            defaultName = `Perfect Balance: ${defaultName || 'Custom Pattern'}`;
        }
        
        const pattern = {
            id: Date.now().toString(36) + Math.random().toString(36).substr(2),
            name: defaultName,
            favorite: false,
            stepCount,
            binary: binaryString,
            hex: PatternConverter.toHex(decimal),
            decimal,
            expression: this.currentPattern.expression || null,
            // Perfect Balance Analysis
            perfectBalance: {
                isPerfectlyBalanced: perfectBalance.isPerfectlyBalanced,
                magnitude: perfectBalance.magnitude,
                normalizedMagnitude: perfectBalance.normalizedMagnitude,
                balanceScore: perfectBalance.balanceScore,
                complexSum: perfectBalance.complexSum,
                geometricBalance: perfectBalance.geometricBalance
            },
            // Pattern type information
            polygon: polygonDetection ? {
                vertices: polygonDetection.vertices,
                expansion: polygonDetection.expansion,
                offset: polygonDetection.offset,
                polygonType: polygonDetection.polygonType,
                formula: polygonDetection.formula
            } : null,
            isRegularPolygon: this.currentPattern.isRegularPolygon || (polygonDetection ? true : false),
            euclidean: euclideanDetection ? euclideanDetection.formula : null,
            isEuclidean: this.currentPattern.isEuclidean || (euclideanDetection ? true : false),
            combined: this.currentPattern.isCombined ? {
                originalPatterns: this.currentPattern.originalPatterns,
                lcmUsed: this.currentPattern.stepCount,
                combinationType: this.currentPattern.combinationType || 'logical_or',
                hasSubtraction: this.currentPattern.hasSubtraction || false,
                vertices: this.currentPattern.vertices || null,
                offsets: this.currentPattern.offsets || null
            } : null,
            isCombined: this.currentPattern.isCombined || false,
            hasSubtraction: this.currentPattern.hasSubtraction || false,
            // Coprime analysis
            coprime: coprimeAnalysis,
            // Geometric Center of Gravity analysis
            cogAnalysis: {
                magnitude: cogAnalysis.magnitude,
                normalizedMagnitude: cogAnalysis.normalizedMagnitude,
                geometricBalance: cogAnalysis.geometricBalance,
                isGeometricallyBalanced: cogAnalysis.isGeometricallyBalanced,
                cogPosition: cogAnalysis.cogPosition
            },
            repetition: repetition ? {
                unitBinary: repetition.unitBinary,
                unitHex: repetition.unitHex,
                unitDecimal: repetition.unitDecimal,
                repetitions: repetition.repetitions,
                compressionRatio: repetition.compressionRatio
            } : null,
            milne: milne.wellformed || milne.perfectlyBalanced ? {
                wellformed: milne.wellformed,
                perfectlyBalanced: milne.perfectlyBalanced,
                intervals: milne.intervals,
                onsets: milne.onsets,
                analysis: milne.analysis
            } : null,
            timestamp: Date.now()
        };
        
        if (this.database.add(pattern)) {
            this.renderDatabase();
            this.updateStats();
            alert(`Pattern added to database!`);
        } else {
            alert('Pattern already exists in database.');
        }
    }
    
    // === DATABASE RENDERING ===
    
    /**
     * Render database patterns with search and filtering
     */
    renderDatabase() {
        const container = document.getElementById('patternList');
        const searchQuery = document.getElementById('searchInput').value;
        const filterType = document.getElementById('filterSelect').value;
        
        let patterns = this.database.patterns;
        
        if (searchQuery) {
            patterns = this.database.search(searchQuery);
        }
        
        patterns = this.database.filter(filterType).filter(p => 
            !searchQuery || this.database.search(searchQuery).includes(p)
        );
        
        if (this.sortByDate) {
            patterns.sort((a, b) => b.timestamp - a.timestamp);
        } else {
            patterns.sort((a, b) => a.stepCount - b.stepCount || a.decimal - b.decimal);
        }
        
        container.innerHTML = '';
        
        patterns.forEach(pattern => {
            // Use the ui-components createPatternEntry function which has combination info display
            const entry = createPatternEntry(pattern);
            container.appendChild(entry);
        });
        
        if (patterns.length === 0) {
            container.innerHTML = '<div style="padding: 20px; text-align: center; color: #666;">No patterns found</div>';
        }
    }
    
    // NOTE: Removed duplicate createPatternEntry method - now using the one from ui-components.js
    // which properly displays combination information with blue boxes
    
    // === UTILITY FUNCTIONS ===
    
    /**
     * Copy text to clipboard with fallback
     * @param {string} text - Text to copy
     */
    copyToClipboard(text) {
        if (navigator.clipboard && navigator.clipboard.writeText) {
            navigator.clipboard.writeText(text).then(() => {
                this.showCopyFeedback();
            }).catch(() => {
                this.fallbackCopy(text);
            });
        } else {
            this.fallbackCopy(text);
        }
    }
    
    /**
     * Show copy feedback animation
     */
    showCopyFeedback() {
        const btn = document.getElementById('copyHexUniversalBtn');
        const originalText = btn.textContent;
        btn.textContent = 'Copied!';
        btn.classList.add('success');
        setTimeout(() => {
            btn.textContent = originalText;
            btn.classList.remove('success');
        }, 2000);
    }
    
    /**
     * Fallback copy method for older browsers
     * @param {string} text - Text to copy
     */
    fallbackCopy(text) {
        const textArea = document.createElement('textarea');
        textArea.value = text;
        document.body.appendChild(textArea);
        textArea.select();
        try {
            document.execCommand('copy');
            this.showCopyFeedback();
        } catch (err) {
            console.warn('Copy failed');
        }
        document.body.removeChild(textArea);
    }
    
    /**
     * Update database statistics display
     */
    updateStats() {
        const cogValues = this.database.patterns
            .filter(p => p.cogAnalysis)
            .map(p => p.cogAnalysis.normalizedMagnitude);
        
        const avgCog = cogValues.length > 0 
            ? (cogValues.reduce((sum, val) => sum + val, 0) / cogValues.length).toFixed(3)
            : '0.000';
        
        const stats = {
            total: this.database.patterns.length,
            perfectBalance: this.database.patterns.filter(p => p.perfectBalance && p.perfectBalance.isPerfectlyBalanced).length,
            favorites: this.database.patterns.filter(p => p.favorite).length,
            polygons: this.database.patterns.filter(p => p.polygon).length,
            euclidean: this.database.patterns.filter(p => p.euclidean).length,
            combined: this.database.patterns.filter(p => p.combined).length,
            wellformed: this.database.patterns.filter(p => p.milne && p.milne.wellformed).length,
            avgCog
        };
        
        document.getElementById('totalPatterns').textContent = stats.total;
        document.getElementById('perfectBalanceCount').textContent = stats.perfectBalance;
        document.getElementById('favoriteCount').textContent = stats.favorites;
        document.getElementById('polygonCount').textContent = stats.polygons;
        document.getElementById('euclideanCount').textContent = stats.euclidean;
        document.getElementById('combinedCount').textContent = stats.combined;
        document.getElementById('wellformedCount').textContent = stats.wellformed;
        document.getElementById('avgCogValue').textContent = stats.avgCog;
    }
    
    /**
     * Show report modal with copy functionality
     * @param {string} report - Report content
     */
    showReportModal(report) {
        const modal = document.createElement('div');
        modal.className = 'modal';
        
        modal.innerHTML = `
            <div class="modal-header">
                <h3>Perfect Balance Analysis Report</h3>
                <button class="btn danger" id="closeReportBtn">Close</button>
            </div>
            <p>Copy the report below:</p>
            <div style="margin-bottom: 15px;">
                <button class="btn primary" id="copyReportBtn">Copy Report</button>
            </div>
            <textarea readonly>${report}</textarea>
        `;
        
        document.body.appendChild(modal);
        
        modal.querySelector('#copyReportBtn').addEventListener('click', () => {
            const textarea = modal.querySelector('textarea');
            textarea.select();
            this.copyToClipboard(report);
        });
        
        modal.querySelector('#closeReportBtn').addEventListener('click', () => {
            document.body.removeChild(modal);
        });
    }
    
    // === EVENT HANDLERS ===
    
    /**
     * Setup comprehensive event listeners for all UI interactions
     */
    setupEventListeners() {
        // Universal input
        const universalInput = document.getElementById('universalInput');
        
        universalInput.addEventListener('input', () => {
            universalInput.classList.remove('error');
        });
        
        universalInput.addEventListener('keydown', (e) => {
            if (e.key === 'Enter') {
                e.preventDefault();
                this.parseUniversalInput();
            }
        });
        
        document.getElementById('parseBtn').addEventListener('click', () => {
            this.parseUniversalInput();
        });
        
        document.getElementById('addUniversalBtn').addEventListener('click', () => {
            this.addCurrentToDatabase();
        });
        
        document.getElementById('copyHexUniversalBtn').addEventListener('click', () => {
            this.copyCurrentHex();
        });
        
        document.getElementById('testBtn').addEventListener('click', () => {
            this.runTests();
        });
        
        // Perfect Balance Explorer
        document.getElementById('findPerfectBtn').addEventListener('click', () => {
            this.startSystematicExploration('perfect');
        });
        
        document.getElementById('findNearBtn').addEventListener('click', () => {
            this.startSystematicExploration('near');
        });
        
        document.getElementById('generateReportBtn').addEventListener('click', () => {
            this.generatePerfectBalanceReport();
        });
        
        document.getElementById('stopBtn').addEventListener('click', () => {
            this.stopExploration();
        });
        
        // Database controls
        document.getElementById('searchInput').addEventListener('input', () => {
            this.renderDatabase();
        });
        
        document.getElementById('filterSelect').addEventListener('change', () => {
            this.renderDatabase();
        });
        
        document.getElementById('sortBtn').addEventListener('click', () => {
            this.sortByDate = !this.sortByDate;
            document.getElementById('sortBtn').textContent = 
                this.sortByDate ? 'Sort by Date' : 'Sort by Steps';
            this.renderDatabase();
        });
        
        document.getElementById('exportBtn').addEventListener('click', () => {
            this.exportDatabase();
        });
        
        document.getElementById('importBtn').addEventListener('click', () => {
            this.importDatabase();
        });
        
        document.getElementById('clearDbBtn').addEventListener('click', () => {
            if (confirm('Clear all patterns from database?')) {
                this.database.patterns = [];
                this.database.save();
                this.renderDatabase();
                this.updateStats();
            }
        });
        
        // Pattern list interactions
        document.getElementById('patternList').addEventListener('click', (e) => {
            const id = e.target.dataset.id;
            const action = e.target.dataset.action;
            
            if (e.target.classList.contains('pattern-star')) {
                this.toggleFavorite(id);
            } else if (e.target.classList.contains('pattern-name')) {
                this.editPatternName(e.target, id);
            } else if (e.target.classList.contains('pattern-repr')) {
                const value = e.target.dataset.value;
                this.copyToClipboard(value);
                
                // Visual feedback
                const original = e.target.style.background;
                e.target.style.background = '#28a745';
                e.target.style.color = 'white';
                setTimeout(() => {
                    e.target.style.background = original;
                    e.target.style.color = '';
                }, 200);
            } else if (action === 'load') {
                this.loadPattern(id);
            } else if (action === 'delete') {
                this.database.remove(id);
                this.renderDatabase();
                this.updateStats();
            }
        });
    }
    
    /**
     * Toggle pattern favorite status
     * @param {string} id - Pattern ID
     */
    toggleFavorite(id) {
        const pattern = this.database.patterns.find(p => p.id === id);
        if (pattern) {
            pattern.favorite = !pattern.favorite;
            this.database.save();
            this.renderDatabase();
            this.updateStats();
        }
    }
    
    /**
     * Edit pattern name inline
     * @param {HTMLElement} element - Name element
     * @param {string} id - Pattern ID
     */
    editPatternName(element, id) {
        const pattern = this.database.patterns.find(p => p.id === id);
        if (!pattern) return;
        
        const input = document.createElement('input');
        input.type = 'text';
        input.className = 'input';
        input.style.width = '200px';
        input.style.fontSize = '12px';
        input.value = pattern.name;
        input.placeholder = 'Enter pattern name';
        
        element.replaceWith(input);
        input.focus();
        input.select();
        
        const saveEdit = () => {
            pattern.name = input.value.trim();
            this.database.save();
            this.renderDatabase();
        };
        
        input.addEventListener('blur', saveEdit);
        input.addEventListener('keydown', (e) => {
            if (e.key === 'Enter') {
                saveEdit();
            } else if (e.key === 'Escape') {
                this.renderDatabase();
            }
        });
    }
    
    /**
     * Load pattern from database and restore its properties
     * @param {string} id - Pattern ID
     */
    loadPattern(id) {
        const pattern = this.database.patterns.find(p => p.id === id);
        if (!pattern) return;
        
        const patternData = PatternConverter.fromBinary(pattern.binary);
        
        // Restore pattern properties from database
        if (pattern.expression) {
            patternData.expression = pattern.expression;
            patternData.hasSubtraction = pattern.hasSubtraction;
        }
        
        if (pattern.polygon) {
            patternData.isRegularPolygon = true;
            patternData.vertices = pattern.polygon.vertices;
            patternData.expansion = pattern.polygon.expansion;
            patternData.offset = pattern.polygon.offset;
            patternData.polygonType = pattern.polygon.polygonType;
            patternData.formula = pattern.polygon.formula;
        }
        
        if (pattern.euclidean) {
            patternData.isEuclidean = true;
            const match = pattern.euclidean.match(/E\((\d+),(\d+),(-?\d+)\)/);
            if (match) {
                patternData.beats = parseInt(match[1]);
                patternData.offset = parseInt(match[3]);
                patternData.formula = pattern.euclidean;
            }
        }
        
        if (pattern.combined) {
            patternData.isCombined = true;
            patternData.originalPatterns = pattern.combined.originalPatterns;
            patternData.combinationType = pattern.combined.combinationType;
            patternData.hasSubtraction = pattern.combined.hasSubtraction;
            patternData.vertices = pattern.combined.vertices;
            patternData.offsets = pattern.combined.offsets;
        }
        
        this.currentPattern = patternData;
        this.updateAnalysisDisplay(patternData);
        
        // Update universal input field with appropriate notation
        let inputValue = '';
        if (pattern.expression) {
            inputValue = pattern.expression;
        } else if (pattern.polygon) {
            inputValue = pattern.polygon.formula;
        } else if (pattern.euclidean) {
            inputValue = pattern.euclidean;
        } else if (pattern.combined && pattern.combined.expression) {
            inputValue = pattern.combined.expression;
        } else {
            inputValue = pattern.hex;
        }
        
        document.getElementById('universalInput').value = inputValue;
        
        // Show compact output
        const compactResult = UnifiedPatternParser.formatCompact(patternData);
        document.getElementById('compactResult').innerHTML = `<div>${compactResult}</div>`;
        document.getElementById('compactOutput').style.display = 'block';
    }
    
    // === EXPORT/IMPORT ===
    
    /**
     * Export database to JSON file
     */
    exportDatabase() {
        const data = this.database.export();
        
        if (window.location.protocol === 'blob:' || window.self !== window.top) {
            this.showExportModal(data);
            return;
        }
        
        try {
            const blob = new Blob([data], { type: 'application/json' });
            const url = URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `enhanced-perfect-balance-database-${new Date().toISOString().split('T')[0]}.json`;
            
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            
            setTimeout(() => URL.revokeObjectURL(url), 100);
        } catch (error) {
            this.showExportModal(data);
        }
    }
    
    /**
     * Show export modal with JSON data
     * @param {string} data - JSON data to export
     */
    showExportModal(data) {
        const modal = document.createElement('div');
        modal.className = 'modal';
        
        modal.innerHTML = `
            <div class="modal-header">
                <h3>Export Enhanced Pattern Database</h3>
                <button class="btn danger" id="closeModalBtn">Close</button>
            </div>
            <p>Copy the JSON data below:</p>
            <div style="margin-bottom: 15px;">
                <button class="btn primary" id="copyAllBtn">Copy All</button>
            </div>
            <textarea readonly>${data}</textarea>
        `;
        
        document.body.appendChild(modal);
        
        modal.querySelector('#copyAllBtn').addEventListener('click', () => {
            const textarea = modal.querySelector('textarea');
            textarea.select();
            this.copyToClipboard(data);
        });
        
        modal.querySelector('#closeModalBtn').addEventListener('click', () => {
            document.body.removeChild(modal);
        });
    }
    
    /**
     * Import database from JSON file
     */
    importDatabase() {
        if (window.location.protocol === 'blob:' || window.self !== window.top) {
            this.showImportModal();
            return;
        }
        
        const input = document.createElement('input');
        input.type = 'file';
        input.accept = '.json';
        
        input.addEventListener('change', (e) => {
            const file = e.target.files[0];
            if (!file) return;
            
            const reader = new FileReader();
            reader.onload = (evt) => {
                if (this.database.import(evt.target.result)) {
                    this.renderDatabase();
                    this.updateStats();
                    alert('Enhanced pattern database imported successfully!');
                } else {
                    alert('Failed to import database: Invalid file format');
                }
            };
            reader.readAsText(file);
        });
        
        input.click();
    }
    
    /**
     * Show import modal for pasting JSON data
     */
    showImportModal() {
        const modal = document.createElement('div');
        modal.className = 'modal';
        
        modal.innerHTML = `
            <div class="modal-header">
                <h3>Import Enhanced Pattern Database</h3>
                <button class="btn danger" id="closeImportBtn">Cancel</button>
            </div>
            <p>Paste JSON data below:</p>
            <div style="margin-bottom: 15px;">
                <button class="btn primary" id="importDataBtn">Import Data</button>
            </div>
            <textarea placeholder="Paste your JSON pattern database here..."></textarea>
        `;
        
        document.body.appendChild(modal);
        
        modal.querySelector('#importDataBtn').addEventListener('click', () => {
            const textarea = modal.querySelector('textarea');
            const jsonData = textarea.value.trim();
            
            if (jsonData) {
                if (this.database.import(jsonData)) {
                    this.renderDatabase();
                    this.updateStats();
                    alert('Enhanced pattern database imported successfully!');
                } else {
                    alert('Failed to import database: Invalid JSON format');
                }
            }
            
            document.body.removeChild(modal);
        });
        
        modal.querySelector('#closeImportBtn').addEventListener('click', () => {
            document.body.removeChild(modal);
        });
        
        modal.querySelector('textarea').focus();
    }
}

// === APPLICATION INITIALIZATION UTILITIES ===

/**
 * Initialize application with error handling
 * @returns {EnhancedPatternApp} Application instance
 */
export function initializeApp() {
    // Global error handling
    window.addEventListener('error', function(e) {
        console.error('Enhanced pattern app error:', e.message);
    });

    window.addEventListener('unhandledrejection', function(e) {
        console.error('Unhandled promise rejection:', e.reason);
    });

    try {
        const app = new EnhancedPatternApp();
        console.log('Enhanced Pattern Database with Perfect Balance Explorer initialized successfully');
        console.log('Features available:');
        console.log('- Fixed LCM calculation for proper pattern combinations');
        console.log('- Advanced subtraction operations for perfect balance discovery');
        console.log('- Systematic perfect balance exploration');
        console.log('- Dual polygon notation: P(v,o) for combinations, P(v,o,e) for single');
        console.log('- All pattern formats: Polygon, Euclidean, Binary, Hex, Decimal, Rotation');
        console.log('- Mathematical analysis: CoG, Milne criteria, perfect balance detection');
        console.log('- Enhanced pattern database with perfect balance tracking');
        return app;
    } catch (error) {
        console.error('Failed to initialize enhanced pattern app:', error);
        return null;
    }
}

/**
 * Auto-initialize when DOM is ready
 * Can be disabled by setting window.DISABLE_AUTO_INIT = true before script load
 */
if (typeof window !== 'undefined' && !window.DISABLE_AUTO_INIT) {
    let app;

    function handleDOMReady() {
        app = initializeApp();
        if (app) {
            window.enhancedPatternApp = app; // Make available globally
        }
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', handleDOMReady);
    } else {
        handleDOMReady();
    }
}