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
        this.sequencer = null;
        
        // Application state
        this.currentPattern = null;
        this.sortByDate = true;
        this.explorationResults = [];
        this.isExploring = false;
        this.isUpdatingFromSequencer = false;
        
        // Initialize the application
        this.setupEventListeners();
        this.initialize();
        this.initializeSequencer();
        
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
            'SystematicExplorer', 'PatternDatabase', 'UIComponents', 'AppConfig',
            'SequencerController', 'SequencerIntegration', 'SyncopationAnalyzer'
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
        this.updateButtonStates();
        
        console.log('🎯 Application initialization complete');
    }
    
    /**
     * Initialize the sequencer controller
     */
    async initializeSequencer() {
        try {
            console.log('🎵 Initializing sequencer...');
            
            // Create sequencer controller
            this.sequencer = new SequencerController('patternSequencerCanvas', 'balanceSection', {
                autoPlay: false,
                defaultTempo: 120,
                defaultVolume: 0.5,
                enableAudio: true,
                enableVisual: true
            });
            
            // Setup sequencer event listeners
            this.setupSequencerEvents();
            
            console.log('✅ Sequencer initialized successfully');
            
        } catch (error) {
            console.error('❌ Failed to initialize sequencer:', error);
            // Continue without sequencer if initialization fails
            this.sequencer = null;
        }
    }
    
    /**
     * Setup sequencer event listeners
     */
    setupSequencerEvents() {
        if (!this.sequencer) return;
        
        // Sequencer control buttons
        const playBtn = document.getElementById('sequencerPlayBtn');
        const pauseBtn = document.getElementById('sequencerPauseBtn');
        const stopBtn = document.getElementById('sequencerStopBtn');
        const tempoSlider = document.getElementById('sequencerTempoSlider');
        const volumeSlider = document.getElementById('sequencerVolumeSlider');
        const tempoDisplay = document.getElementById('sequencerTempoDisplay');
        const volumeDisplay = document.getElementById('sequencerVolumeDisplay');
        const statusDisplay = document.getElementById('sequencerStatus');
        
        if (playBtn) {
            playBtn.addEventListener('click', () => this.sequencer.play());
        }
        
        if (pauseBtn) {
            pauseBtn.addEventListener('click', () => this.sequencer.pause());
        }
        
        if (stopBtn) {
            stopBtn.addEventListener('click', () => this.sequencer.stop());
        }
        
        if (tempoSlider && tempoDisplay) {
            tempoSlider.addEventListener('input', (e) => {
                const tempo = parseInt(e.target.value);
                this.sequencer.setTempo(tempo);
                tempoDisplay.textContent = tempo;
            });
        }
        
        if (volumeSlider && volumeDisplay) {
            volumeSlider.addEventListener('input', (e) => {
                const volume = parseInt(e.target.value) / 100;
                this.sequencer.updateAudioSettings({ volume });
                volumeDisplay.textContent = `${e.target.value}%`;
            });
        }
        
        // Listen for sequencer events
        this.sequencer.on('onPlaybackChange', (data) => {
            if (statusDisplay) {
                statusDisplay.textContent = data.isPlaying ? 'Playing' : 
                                          data.isPaused ? 'Paused' : 'Ready';
            }
        });
        
        this.sequencer.on('onPatternChange', (data) => {
            // Prevent infinite loop - ignore updates from sequencer when we're already updating
            if (this.isUpdatingFromSequencer) {
                return;
            }
            
            // Ignore pattern changes from controller (these are from loadPatternIntoSequencer calls)
            // Only respond to visual interaction changes
            if (data.source === 'controller') {
                return;
            }
            
            // Update current pattern with the modified steps
            if (this.currentPattern && data.pattern) {
                try {
                    this.isUpdatingFromSequencer = true;
                    
                    this.currentPattern.steps = [...data.pattern.steps];
                    this.currentPattern.stepCount = data.pattern.stepCount;
                    
                    // Regenerate analysis with full sequencer integration (loop is prevented by source filtering)
                    this.displayPatternAnalysis(this.currentPattern);
                    
                    // Update the universal input to reflect the new pattern
                    this.updateUniversalInputFromPattern(this.currentPattern);
                    
                    // Update button states
                    this.updateButtonStates();
                    
                } catch (error) {
                    console.error('❌ Error updating pattern from sequencer:', error);
                    showNotification('Error updating pattern from sequencer', 'error', 3000);
                } finally {
                    this.isUpdatingFromSequencer = false;
                }
            } else {
                console.warn('⚠️ Pattern update skipped: no current pattern or invalid data', {
                    hasCurrentPattern: !!this.currentPattern,
                    hasDataPattern: !!(data && data.pattern)
                });
            }
        });
        
        this.sequencer.on('onError', (error) => {
            console.error('Sequencer error:', error);
            if (statusDisplay) {
                statusDisplay.textContent = 'Error';
            }
        });
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
        
        // Stochastic generator events
        this.setupStochasticEvents();
        
        console.log('✅ Event listeners configured');
    }
    
    /**
     * Setup universal pattern input event listeners
     */
    setupUniversalInputEvents() {
        const parseBtn = document.getElementById('parseBtn');
        const addBtn = document.getElementById('addToDbBtn');
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
                    if (e.metaKey || e.ctrlKey) {
                        // Cmd+Enter or Ctrl+Enter: Parse and add to database
                        this.parseUniversalInput();
                        setTimeout(() => {
                            if (this.currentPattern) {
                                this.addCurrentPatternToDatabase();
                            }
                        }, 100);
                    } else {
                        // Just Enter: Parse only
                        this.parseUniversalInput();
                    }
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
        const minStepCountInput = document.getElementById('minStepCount');
        const maxStepCountInput = document.getElementById('maxStepCount');
        
        if (searchInput) {
            searchInput.addEventListener('input', () => this.handleSearch());
        }
        
        if (filterSelect) {
            filterSelect.addEventListener('change', () => this.handleFilter());
        }
        
        if (sortBtn) {
            sortBtn.addEventListener('click', () => this.toggleSort());
        }
        
        // Step count filter event listeners
        if (minStepCountInput) {
            minStepCountInput.addEventListener('keypress', (e) => {
                if (e.key === 'Enter') {
                    this.updatePatternList();
                }
            });
            minStepCountInput.addEventListener('input', () => {
                // Optional: Add debounced filtering on input change
                clearTimeout(this.stepCountFilterTimeout);
                this.stepCountFilterTimeout = setTimeout(() => {
                    this.updatePatternList();
                }, 500);
            });
        }
        
        if (maxStepCountInput) {
            maxStepCountInput.addEventListener('keypress', (e) => {
                if (e.key === 'Enter') {
                    this.updatePatternList();
                }
            });
            maxStepCountInput.addEventListener('input', () => {
                // Optional: Add debounced filtering on input change
                clearTimeout(this.stepCountFilterTimeout);
                this.stepCountFilterTimeout = setTimeout(() => {
                    this.updatePatternList();
                }, 500);
            });
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
            showNotification('Please enter a pattern to parse', 'warning');
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
                this.updateButtonStates();
                console.log('✅ Pattern parsed successfully');
            } else if (result.type === 'combination') {
                this.currentPattern = result.combined;
                this.displayPatternAnalysis(this.currentPattern);
                this.showCompactOutput(this.currentPattern);
                this.updateButtonStates();
                console.log('✅ Combined pattern parsed successfully');
            } else if (result.type === 'stringed') {
                console.log('🔍 Stringed pattern result:', result);
                this.currentPattern = result.pattern;
                console.log('🔍 Current pattern after assignment:', this.currentPattern);
                this.displayPatternAnalysis(this.currentPattern);
                this.showCompactOutput(this.currentPattern);
                this.updateButtonStates();
                console.log('✅ Stringed pattern parsed successfully');
            } else if (result.type === 'quantized') {
                console.log('🔍 Quantized pattern result:', result);
                this.currentPattern = result.pattern;
                console.log('🔍 Current pattern after assignment:', this.currentPattern);
                this.displayPatternAnalysis(this.currentPattern);
                this.showCompactOutput(this.currentPattern);
                this.updateButtonStates();
                console.log('✅ Quantized pattern parsed successfully');
            } else {
                showNotification('Failed to parse pattern: Unknown result type', 'error');
                console.error('❌ Pattern parsing failed: Unknown result type');
            }
            
        } catch (error) {
            console.error('❌ Pattern parsing error:', error);
            showNotification('Error parsing pattern\n' + error.message, 'error');
        }
    }
    
    /**
     * Display comprehensive pattern analysis
     */
    displayPatternAnalysis(pattern) {
        try {
            // Generate comprehensive analysis
            const analysis = this.generateComprehensiveAnalysis(pattern);
            
            // Hide placeholder
            const placeholder = document.getElementById('analysisPlaceholder');
            if (placeholder) placeholder.style.display = 'none';
            
            // Show and populate pattern output line
            const patternOutputLine = document.getElementById('patternOutputLine');
            if (patternOutputLine) {
                patternOutputLine.style.display = 'flex';
                patternOutputLine.innerHTML = analysis.patternOutputItems.map(item => 
                    `<div class="output-item">${item}</div>`
                ).join('');
            }
            
            // Show and populate balance section
            const balanceSection = document.getElementById('balanceSection');
            if (balanceSection) {
                balanceSection.style.display = 'grid';
                
                const balanceInfo = balanceSection.querySelector('.balance-info-compact');
                const balanceViz = balanceSection.querySelector('.balance-visualization-large');
                
                if (balanceInfo) {
                    balanceInfo.innerHTML = `
                        <div class="analysis-box balance-box">
                            <div class="analysis-box-header">
                                <span class="analysis-box-icon">⚖️</span>
                                <span class="analysis-box-title">Perfect Balance</span>
                                <button class="info-btn-inline" onclick="toggleInfoBox('balance-equation')" title="Show mathematical formula">🧮</button>
                            </div>
                            <div class="analysis-box-content">
                                <div class="balance-score-display balance-${analysis.balanceAnalysis.balanceScore.toLowerCase()}">
                                    ${analysis.balanceAnalysis.balanceScore}
                                </div>
                                <div class="balance-score-value">
                                    Score: ${analysis.balanceAnalysis.normalizedMagnitude.toFixed(4)}
                                </div>
                                <div class="balance-score-status">
                                    ${analysis.balanceAnalysis.isPerfectlyBalanced ? '✨ PERFECTLY BALANCED' : 'Not perfectly balanced'}
                                </div>
                            </div>
                            <div class="info-box" id="balance-equation" style="display: none; margin-top: 8px; font-size: 12px; background: #f8f9fa; padding: 6px; border-radius: 4px; border-left: 3px solid #28a745;">
                                <strong>Milne's Formula:</strong> |∑(e^(i2πkⱼ/n))| / onsets = ${analysis.balanceAnalysis.normalizedMagnitude.toFixed(6)}
                            </div>
                        </div>
                        <div class="analysis-box cog-box" style="margin-top: 8px;">
                            <div class="analysis-box-header">
                                <span class="analysis-box-icon">🎯</span>
                                <span class="analysis-box-title">Center of Gravity</span>
                            </div>
                            <div class="analysis-box-content">
                                <div class="cog-distance-value">
                                    Distance: ${analysis.cogAnalysis?.normalizedMagnitude?.toFixed(4) || analysis.cogAnalysis?.distance?.toFixed(4) || 'N/A'}
                                </div>
                                <div class="cog-angle-value">
                                    Angle: ${analysis.cogAnalysis?.angle?.toFixed(1) || 
                                            (analysis.cogAnalysis?.coordinates ? 
                                                Math.atan2(analysis.cogAnalysis.coordinates.y, analysis.cogAnalysis.coordinates.x) * (180 / Math.PI) : 0).toFixed(1)}°
                                </div>
                                <div class="cog-description">
                                    ${(analysis.cogAnalysis?.normalizedMagnitude || analysis.cogAnalysis?.distance || 1) < 0.1 ? '🎯 Geometrically Centered' : 'Off-center pattern'}
                                </div>
                            </div>
                        </div>
${(() => {
                            // Skip entire prosody display for generic interval descriptions
                            const skipEntireDisplay = analysis.longShortAnalysis?.description && 
                                (analysis.longShortAnalysis.description.includes('Predominantly') || 
                                 analysis.longShortAnalysis.description.includes('intervals') ||
                                 analysis.longShortAnalysis.description.includes('Complex rhythmic pattern') ||
                                 analysis.longShortAnalysis.description.includes('No prosodic pattern'));

                            // Only show if there's meaningful analysis
                            if (!analysis.longShortAnalysis?.description || 
                                analysis.longShortAnalysis.description === 'No prosodic pattern' || 
                                skipEntireDisplay) {
                                return '';
                            }

                            return `
                                <div class="analysis-box longshort-box" style="margin-top: 8px;">
                                    <div class="analysis-box-header">
                                        <span class="analysis-box-icon">🎼</span>
                                        <span class="analysis-box-title">Rhythmic Prosody</span>
                                    </div>
                                    <div class="analysis-box-content">
                                        <div class="prosody-type">
                                            ${analysis.longShortAnalysis.description}
                                        </div>
                                        ${analysis.longShortAnalysis.morseNotation ? 
                                            `<div class="morse-notation" style="font-family: monospace; margin-top: 4px; font-size: 16px;">
                                                Morse: ${analysis.longShortAnalysis.morseNotation}${analysis.longShortAnalysis.morseCharacter ? ` (${analysis.longShortAnalysis.morseCharacter})` : ''}
                                            </div>` : ''
                                        }
                                        <div class="interval-details" style="font-size: 12px; color: #666; margin-top: 4px;">
                                            ${analysis.longShortAnalysis.intervals?.length > 0 ? 
                                                `Intervals: [${analysis.longShortAnalysis.intervals.join(', ')}]` : 
                                                'No intervals'}
                                        </div>
                                    </div>
                                </div>
                            `;
                        })()}
                        
                        <!-- SYNCOPATION ANALYSIS BOX -->
                        <div class="analysis-box syncopation-box" style="margin-top: 8px;">
                            <div class="analysis-box-header">
                                <span class="analysis-box-icon">🎵</span>
                                <span class="analysis-box-title">Syncopation Analysis</span>
                                <button class="info-btn" onclick="toggleInfoBox('syncopation-info')" style="margin-left: auto;">ℹ️</button>
                            </div>
                            <div class="analysis-box-content">
                                <div class="syncopation-overall">
                                    ${analysis.syncopationAnalysis.description} (${(analysis.syncopationAnalysis.overallSyncopation * 100).toFixed(1)}%)
                                </div>
                                <div class="syncopation-measures" style="font-size: 12px; color: #666; margin-top: 6px; display: grid; grid-template-columns: 1fr 1fr; gap: 4px;">
                                    <div>Note-to-Beat: ${(analysis.syncopationAnalysis.weightedNoteToBeats * 100).toFixed(0)}%</div>
                                    <div>Off-Beat Ratio: ${(analysis.syncopationAnalysis.offBeatRatio * 100).toFixed(0)}%</div>
                                    <div>Expectancy: ${(analysis.syncopationAnalysis.expectancyViolation * 100).toFixed(0)}%</div>
                                    <div>Displacement: ${(analysis.syncopationAnalysis.rhythmicDisplacement * 100).toFixed(0)}%</div>
                                    <div>Cross-Rhythmic: ${(analysis.syncopationAnalysis.crossRhythmic * 100).toFixed(0)}%</div>
                                    <div>Barlow Index: ${(analysis.syncopationAnalysis.barlowIndispensability * 100).toFixed(0)}%</div>
                                </div>
                            </div>
                            
                            <!-- SYNCOPATION INFORMATION BOX -->
                            <div class="info-box" id="syncopation-info" style="display: none; margin-top: 8px;">
                                <strong>Syncopation Measures:</strong><br>
                                • <strong>Note-to-Beat:</strong> Distance from onsets to strong beats (Longuet-Higgins & Lee)<br>
                                • <strong>Off-Beat Ratio:</strong> Percentage of onsets on weak beats vs strong beats<br>
                                • <strong>Expectancy:</strong> Cognitive expectancy violations (Huron) - surprise factor<br>
                                • <strong>Displacement:</strong> How far onsets are displaced from expected positions<br>
                                • <strong>Cross-Rhythmic:</strong> Conflict between surface rhythm and meter (Temperley)<br>
                                • <strong>Barlow Index:</strong> Prime factorization-based indispensability (Clarence Barlow)<br><br>
                                <strong>Overall Score:</strong> Weighted combination of all measures<br>
                                • <strong>0-10%:</strong> No syncopation - strong metric alignment<br>
                                • <strong>10-30%:</strong> Low syncopation - mostly on-beat<br>
                                • <strong>30-50%:</strong> Moderate syncopation - balanced off-beat elements<br>
                                • <strong>50-70%:</strong> High syncopation - significant rhythmic tension<br>
                                • <strong>70%+:</strong> Extreme syncopation - heavily off-beat emphasis
                            </div>
                        </div>
                    `;
                }
            }
            
            // Show and populate stochastic section
            const stochasticSection = document.getElementById('stochasticSection');
            if (stochasticSection) {
                stochasticSection.style.display = 'block';
                // Reset any previous generation state
                this.resetStochasticControls();
            }
            
            // Load pattern into sequencer
            this.loadPatternIntoSequencer(pattern, analysis);
            
        } catch (error) {
            console.error('❌ Analysis display error:', error);
            
            // Hide other elements and show error
            const placeholder = document.getElementById('analysisPlaceholder');
            if (placeholder) {
                placeholder.style.display = 'block';
                placeholder.innerHTML = `
                    <div class="analysis-error">
                        <strong>Analysis Error</strong><br>
                        Failed to generate analysis: ${error.message}
                    </div>
                `;
            }
            
            const patternOutputLine = document.getElementById('patternOutputLine');
            if (patternOutputLine) patternOutputLine.style.display = 'none';
            
            const balanceSection = document.getElementById('balanceSection');
            if (balanceSection) balanceSection.style.display = 'none';
        }
    }
    
    /**
     * Generate comprehensive mathematical analysis
     */
    generateComprehensiveAnalysis(pattern) {
        console.log('🔍 Generating analysis for pattern:', pattern);
        
        if (!pattern || !pattern.steps) {
            throw new Error('Invalid pattern: missing steps array');
        }
        
        // Generate pattern output line data
        const structureAnalysis = PatternAnalyzer.analyzeStructure(pattern.steps, pattern.stepCount);
        const binary = PatternConverter.toBinary(pattern.steps, pattern.stepCount);
        const decimal = PatternConverter.toDecimal(binary, false);
        const hex = PatternConverter.toHex(decimal);
        const octal = PatternConverter.toOctal(decimal);
        const onsetCount = pattern.steps.filter(step => step).length;
        const density = ((onsetCount / pattern.stepCount) * 100).toFixed(1);
        
        // Generate pattern types with colored polygon info
        let patternTypes = [];
        
        // Check for Euclidean patterns (both input and detected)
        if (pattern.isEuclidean) {
            const euclideanLabel = pattern.detectedEuclidean ? 'Detected Euclidean' : 'Euclidean';
            const formula = pattern.formula || pattern.euclidean || (pattern.euclideanData && pattern.euclideanData.formula);
            patternTypes.push(`<span class="pattern-type euclidean">🌀 ${euclideanLabel}: ${formula}</span>`);
        } else {
            // Check if this pattern matches a Euclidean rhythm (for real-time detection)
            const euclideanAnalysis = PatternAnalyzer.detectEuclideanPattern(pattern.steps, pattern.stepCount);
            if (euclideanAnalysis) {
                patternTypes.push(`<span class="pattern-type euclidean detected">🌀 Detected Euclidean: ${euclideanAnalysis.formula}</span>`);
            }
        }
        if (pattern.isRegularPolygon) {
            const polygonName = pattern.polygonType || `${pattern.vertices}-gon`;
            patternTypes.push(`<span class="pattern-type polygon">🔺 ${polygonName}</span>`);
        }
        if (pattern.isCombined && pattern.polygonComponents && pattern.polygonComponents.length > 0) {
            const polygonLabels = pattern.polygonComponents.map(comp => 
                `🔺 ${comp.polygonType || comp.type || 'Polygon'}`
            ).join(' + ');
            patternTypes.push(`<span class="pattern-type polygon-combined">${polygonLabels}</span>`);
        }
        const repetitionAnalysis = PatternAnalyzer.detectRepetition(pattern.steps, pattern.stepCount);
        if (repetitionAnalysis) {
            patternTypes.push(`<span class="pattern-type repetition">🔄 Repetition: ${repetitionAnalysis.unitBinary}×${repetitionAnalysis.repetitions}</span>`);
        }
        if (pattern.isQuantized) {
            const direction = pattern.quantizationDirection === 'clockwise' ? '↻' : '↺';
            const ratio = pattern.quantizationRatio;
            const ratioText = ratio > 1 ? `expanded ${ratio.toFixed(2)}×` : `compressed ${ratio.toFixed(2)}×`;
            patternTypes.push(`<span class="pattern-type quantization">⚡ Quantized ${direction}: ${pattern.originalStepCount}→${pattern.stepCount} (${ratioText})</span>`);
        }
        
        // Pattern output line - special handling for stringed patterns
        let patternOutputItems;
        if (pattern.isStringed && pattern.stringedPatterns) {
            // Generate spaced representations for stringed patterns
            console.log(`🔍 Stringed patterns order:`, pattern.stringedPatterns.map((p, i) => `${i}: ${p.formula || 'unknown'} (${p.stepCount} steps)`));
            const patternReprs = pattern.stringedPatterns.map(p => {
                const pBinary = PatternConverter.toBinary(p.steps, p.stepCount);
                // Use consistent binary interpretation (false = standard left-to-right)
                const pDecimal = PatternConverter.toDecimal(pBinary, false);
                const pHex = PatternConverter.toHex(pDecimal);
                const pOct = PatternConverter.toOctal(pDecimal);
                console.log(`🔍 Pattern ${p.formula || 'unknown'}: Steps=${p.steps}, Binary=${pBinary}, Decimal=${pDecimal}, Hex=${pHex}`);
                return { binary: pBinary, hex: pHex, oct: pOct, decimal: pDecimal };
            });
            
            const binaryParts = patternReprs.map(r => r.binary).join(' ');
            const hexParts = patternReprs.map(r => r.hex).join(' ');
            const octParts = patternReprs.map(r => r.oct).join(' ');
            const decParts = patternReprs.map(r => r.decimal).join(' ');
            
            patternOutputItems = [
                `Binary: ${binaryParts}`,
                `Hex: ${hexParts}`,
                `Oct: ${octParts}`,
                `Dec: ${decParts}`,
                `Steps: ${pattern.stepCount}`,
                `Beats: ${onsetCount}`,
                `Density: ${density}%`,
                `Max Gap: ${structureAnalysis.maxSilenceGap}`,
                `Avg Interval: ${structureAnalysis.avgOnsetInterval.toFixed(1)}`,
                `🔗 Stringed: ${pattern.originalString}`,
                ...patternTypes
            ];
        } else {
            // Regular pattern formatting
            patternOutputItems = [
                `Binary: ${binary}`,
                `Hex: ${hex}`,
                `Oct: ${octal}`,
                `Dec: ${decimal}`,
                `Steps: ${pattern.stepCount}`,
                `Beats: ${onsetCount}`,
                `Density: ${density}%`,
                `Max Gap: ${structureAnalysis.maxSilenceGap}`,
                `Avg Interval: ${structureAnalysis.avgOnsetInterval.toFixed(1)}`,
                ...patternTypes
            ];
        }
        
        // Balance analysis
        const balanceAnalysis = PerfectBalanceAnalyzer.calculateBalance(pattern.steps, pattern.stepCount);
        const cogAnalysis = CenterOfGravityCalculator.calculateCenterOfGravity(pattern.steps);
        
        // Long-short analysis
        let longShortAnalysis = LongShortAnalyzer.analyzeLongShort(pattern.steps, pattern.stepCount);
        
        // If this is a Morse pattern, add Morse information while preserving prosodic analysis
        if (pattern.isMorse && pattern.morseCode) {
            const morseDescription = pattern.morseCharacters ? 
                `Morse Code: ${pattern.morseCharacters} (${pattern.morseCode})` : 
                `Morse Pattern (${pattern.morseCode})`;
            
            longShortAnalysis = {
                ...longShortAnalysis,
                morseNotation: pattern.morseCode,
                morseCharacter: pattern.morseCharacters || LongShortAnalyzer.detectMorseCharacter(pattern.morseCode),
                // Combine prosodic and Morse descriptions if both exist
                description: longShortAnalysis.description && longShortAnalysis.description !== 'No prosodic pattern' ?
                    `${longShortAnalysis.description} | ${morseDescription}` :
                    morseDescription
            };
        } else {
            // For non-Morse patterns, still try to detect Morse character if 4 or fewer onsets
            const onsetCount = pattern.steps.filter(s => s).length;
            if (onsetCount <= 4 && longShortAnalysis.morseNotation) {
                const detectedMorse = LongShortAnalyzer.detectMorseCharacter(longShortAnalysis.morseNotation);
                if (detectedMorse) {
                    longShortAnalysis.morseCharacter = detectedMorse;
                    // Combine prosodic and detected Morse if both exist
                    if (longShortAnalysis.description && longShortAnalysis.description !== 'No prosodic pattern') {
                        longShortAnalysis.description = `${longShortAnalysis.description} | Morse: ${detectedMorse}`;
                    }
                }
            }
        }
        
        // Syncopation analysis
        const syncopationAnalysis = SyncopationAnalyzer.analyzeSyncopation(pattern.steps, pattern.stepCount);
        
        return {
            title: 'Mathematical Pattern Analysis',
            patternOutputItems: patternOutputItems,
            balanceAnalysis: balanceAnalysis,
            cogAnalysis: cogAnalysis,
            longShortAnalysis: longShortAnalysis,
            syncopationAnalysis: syncopationAnalysis,
            stepCount: pattern.stepCount
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
            showNotification('No pattern selected to add to database', 'warning');
            return;
        }
        
        try {
            // Calculate perfect balance analysis for database storage
            const perfectBalance = PerfectBalanceAnalyzer.calculateBalance(this.currentPattern.steps, this.currentPattern.stepCount);
            
            // Calculate repetition analysis for database storage
            const repetitionAnalysis = PatternAnalyzer.detectRepetition(this.currentPattern.steps, this.currentPattern.stepCount);
            
            // Calculate Euclidean analysis for database storage
            const euclideanAnalysis = PatternAnalyzer.detectEuclideanPattern(this.currentPattern.steps, this.currentPattern.stepCount);
            
            // Calculate long-short analysis for database storage
            const longShortAnalysis = LongShortAnalyzer.analyzeLongShort(this.currentPattern.steps, this.currentPattern.stepCount);
            
            // Calculate syncopation analysis for database storage
            const syncopationAnalysis = SyncopationAnalyzer.analyzeSyncopation(this.currentPattern.steps, this.currentPattern.stepCount);
            
            // Create database pattern with analysis
            const databasePattern = createDatabasePattern(this.currentPattern, { 
                perfectBalance,
                repetition: repetitionAnalysis,
                euclidean: euclideanAnalysis,
                longShort: longShortAnalysis,
                syncopation: syncopationAnalysis
            });
            
            const patternId = this.database.add(databasePattern);
            console.log(`✅ Pattern added to database with ID: ${patternId}`);
            showNotification(`Pattern added to database\nID: ${patternId}`, 'success');
            
            // Update UI
            this.updatePatternList();
            this.updateDatabaseStats();
            
        } catch (error) {
            console.error('❌ Failed to add pattern:', error);
            showNotification('Failed to add pattern\n' + error.message, 'error');
        }
    }
    
    /**
     * Update UI button states based on current pattern
     */
    updateButtonStates() {
        const addBtn = document.getElementById('addToDbBtn');
        const copyBtn = document.getElementById('copyHexUniversalBtn');
        
        const hasPattern = !!this.currentPattern;
        
        if (addBtn) {
            addBtn.disabled = !hasPattern;
            addBtn.style.opacity = hasPattern ? '1' : '0.5';
        }
        
        if (copyBtn) {
            copyBtn.disabled = !hasPattern;
            copyBtn.style.opacity = hasPattern ? '1' : '0.5';
        }
    }
    
    /**
     * Copy current pattern hex to clipboard
     */
    copyCurrentPatternHex() {
        if (!this.currentPattern) {
            showNotification('No pattern selected to copy', 'warning');
            return;
        }
        
        const binary = PatternConverter.toBinary(this.currentPattern.steps, this.currentPattern.stepCount);
        const decimal = PatternConverter.toDecimal(binary, false);
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
            '0x49',
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
    /**
     * Parse comma-separated exploration parameters
     * @returns {Object} Parsed parameters or null if invalid
     */
    parseExplorationParams() {
        try {
            const paramInput = document.getElementById('explorerParams')?.value || '3,7,3';
            const parts = paramInput.split(',').map(part => part.trim());
            
            if (parts.length !== 3) {
                throw new Error('Expected format: min sides, max sides, max patterns');
            }
            
            const minSides = parseInt(parts[0]);
            const maxSides = parseInt(parts[1]);
            const maxCombinations = parseInt(parts[2]);
            
            if (isNaN(minSides) || isNaN(maxSides) || isNaN(maxCombinations)) {
                throw new Error('All parameters must be valid numbers');
            }
            
            if (minSides < 2 || maxSides < 2 || maxCombinations < 2) {
                throw new Error('All parameters must be at least 2');
            }
            
            if (minSides > maxSides) {
                throw new Error('Minimum sides cannot be greater than maximum sides');
            }
            
            return { minSides, maxSides, maxCombinations };
        } catch (error) {
            showNotification(`Invalid parameters: ${error.message}`, 'warning');
            return null;
        }
    }
    
    async startPerfectBalanceExploration() {
        const params = this.parseExplorationParams();
        if (!params) return;
        
        const { minSides, maxSides, maxCombinations } = params;
        
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
            let duplicateCount = 0;
            results.forEach(result => {
                try {
                    // Create database pattern with comprehensive analysis
                    const longShortAnalysis = LongShortAnalyzer.analyzeLongShort(result.pattern.steps, result.pattern.stepCount);
                    const databasePattern = createDatabasePattern(result.pattern, { 
                        perfectBalance: result.balance,
                        longShort: longShortAnalysis
                    });
                    const patternId = this.database.add(databasePattern);
                    if (patternId) {
                        addedCount++;
                    } else {
                        duplicateCount++;
                    }
                } catch (error) {
                    // Pattern might already exist, that's okay
                    duplicateCount++;
                }
            });
            
            this.updatePatternList();
            this.updateDatabaseStats();
            this.hideExplorationProgress();
            
            // Create more informative message
            let message = `✅ Exploration complete: ${results.length} perfect balance patterns discovered\n`;
            message += `📊 Results: ${addedCount} new patterns added to database`;
            if (duplicateCount > 0) {
                message += `, ${duplicateCount} duplicates skipped`;
            }
            
            showNotification(message, 'success', 8000);
            
        } catch (error) {
            console.error('❌ Exploration failed:', error);
            showNotification('Exploration failed\n' + error.message, 'error');
        } finally {
            this.isExploring = false;
            this.hideExplorationProgress();
        }
    }
    
    /**
     * Start near perfect balance exploration
     */
    async startNearPerfectExploration() {
        const params = this.parseExplorationParams();
        if (!params) return;
        
        const { minSides, maxSides, maxCombinations } = params;
        
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
            let duplicateCount = 0;
            results.forEach(result => {
                try {
                    // Create database pattern with comprehensive analysis
                    const longShortAnalysis = LongShortAnalyzer.analyzeLongShort(result.pattern.steps, result.pattern.stepCount);
                    const databasePattern = createDatabasePattern(result.pattern, { 
                        perfectBalance: result.balance,
                        longShort: longShortAnalysis
                    });
                    const patternId = this.database.add(databasePattern);
                    if (patternId) {
                        addedCount++;
                    } else {
                        duplicateCount++;
                    }
                } catch (error) {
                    // Pattern might already exist, that's okay
                    duplicateCount++;
                }
            });
            
            this.updatePatternList();
            this.updateDatabaseStats();
            this.hideExplorationProgress();
            
            // Create more informative message
            let message = `✅ Exploration complete: ${results.length} near-perfect balance patterns discovered\n`;
            message += `📊 Results: ${addedCount} new patterns added to database`;
            if (duplicateCount > 0) {
                message += `, ${duplicateCount} duplicates skipped`;
            }
            
            showNotification(message, 'success', 8000);
            
        } catch (error) {
            console.error('❌ Exploration failed:', error);
            showNotification('Exploration failed\n' + error.message, 'error');
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
            showNotification('No perfect balance patterns found in database', 'info');
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

        // Download report as text file
        const timestamp = new Date().toISOString().slice(0, 19).replace(/:/g, '-');
        const filename = `perfect-balance-report-${timestamp}.txt`;
        
        const success = downloadFile(report, filename, 'text/plain');
        if (success) {
            showNotification(`Report generated and downloaded\nFile: ${filename}`, 'success');
            console.log('📄 Report generated and downloaded:', filename);
        } else {
            showNotification('Failed to download report', 'error');
        }
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
            showNotification('Export failed\n' + error.message, 'error');
        }
    }
    
    /**
     * Import database
     */
    async importDatabase() {
        try {
            const files = await selectFile('.json');
            if (files.length === 0) return;
            
            const file = files[0];
            const reader = new FileReader();
            
            reader.onload = (e) => {
                try {
                    const importData = e.target.result;
                    this.database.import(importData);
                    this.updatePatternList();
                    this.updateDatabaseStats();
                    showNotification(`Database imported successfully\nLoaded from: ${file.name}`, 'success');
                    console.log('📥 Database imported successfully from file:', file.name);
                } catch (error) {
                    console.error('❌ Import failed:', error);
                    showNotification(`Import failed\n${error.message}`, 'error');
                }
            };
            
            reader.onerror = () => {
                showNotification('Failed to read file', 'error');
            };
            
            reader.readAsText(file);
        } catch (error) {
            console.error('❌ File selection failed:', error);
            showNotification('File selection failed', 'error');
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
            const minStepCountInput = document.getElementById('minStepCount')?.value;
            const maxStepCountInput = document.getElementById('maxStepCount')?.value;
            
            // Parse step count range values
            const minSteps = minStepCountInput && minStepCountInput.trim() ? parseInt(minStepCountInput) : null;
            const maxSteps = maxStepCountInput && maxStepCountInput.trim() ? parseInt(maxStepCountInput) : null;
            
            // Use combined filtering approach
            const filters = {
                type: filterType,
                search: searchTerm,
                minSteps: minSteps,
                maxSteps: maxSteps
            };
            
            const patterns = this.database.filterCombined(filters);
            
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
        
        // Binary representation with explicit step count
        const binary = PatternConverter.toBinary(pattern.steps, pattern.stepCount);
        representations.push(`b${binary}:${pattern.stepCount}`);
        
        // Decimal representation with explicit step count
        const decimal = PatternConverter.toDecimal(binary, false);
        representations.push(`${decimal}:${pattern.stepCount}`);
        
        // Hex representation with explicit step count
        const hex = PatternConverter.toHex(decimal);
        representations.push(`${hex}:${pattern.stepCount}`);
        
        // Octal representation with explicit step count
        const octal = PatternConverter.toOctal(decimal);
        representations.push(`${octal}:${pattern.stepCount}`);
        
        // Onset array representation with explicit step count
        const onsetArray = PatternConverter.toEnhancedOnsetArray(pattern.steps, pattern.stepCount);
        representations.push(onsetArray);
        
        // Morse notation representation if available
        if (pattern.longShort && pattern.longShort.morseNotation && 
            pattern.longShort.morseNotation.length > 0 && 
            pattern.longShort.morseNotation !== '=' && 
            pattern.longShort.morseNotation !== 'N/A') {
            const morseDisplay = pattern.longShort.morseCharacter ? 
                `${pattern.longShort.morseNotation} (${pattern.longShort.morseCharacter})` : 
                pattern.longShort.morseNotation;
            representations.push(`Morse: ${morseDisplay}`);
        }
        
        return representations;
    }

    /**
     * Create HTML for pattern entry
     */
    createPatternEntryHTML(pattern) {
        const analysis = PerfectBalanceAnalyzer.calculateBalance(pattern.steps, pattern.stepCount);
        const representations = this.getPatternRepresentations(pattern);
        
        const stepCountClass = pattern.stepCount <= 8 ? 'small' : pattern.stepCount <= 16 ? 'medium' : 'large';
        
        // Get polygon labels for combined patterns - check multiple possible locations
        let polygonLabels = '';
        if (pattern.isCombined) {
            if (pattern.polygonComponents && pattern.polygonComponents.length > 0) {
                polygonLabels = pattern.polygonComponents.map(comp => `🔺 ${comp.polygonType || comp.type || 'Polygon'}`).join(' ');
            } else if (pattern.combined && pattern.combined.originalPatterns && pattern.combined.originalPatterns.length > 0) {
                // Check combined.originalPatterns for polygon data
                const polygonComps = pattern.combined.originalPatterns.filter(comp => 
                    comp.isRegularPolygon || comp.type === 'polygon' || (comp.vertices && comp.vertices > 0)
                );
                if (polygonComps.length > 0) {
                    const polygonNames = polygonComps.map(comp => {
                        if (comp.polygonType) return `🔺 ${comp.polygonType}`;
                        if (comp.vertices) {
                            const sides = comp.vertices;
                            const name = sides === 3 ? 'Triangle' : 
                                       sides === 4 ? 'Square' : 
                                       sides === 5 ? 'Pentagon' : 
                                       sides === 6 ? 'Hexagon' : 
                                       sides === 7 ? 'Heptagon' : 
                                       sides === 8 ? 'Octagon' : 
                                       `${sides}-gon`;
                            return `🔺 ${name}`;
                        }
                        return `🔺 ${comp.name || comp.type || 'Polygon'}`;
                    });
                    polygonLabels = polygonNames.join(' ');
                }
            } else if (pattern.components && pattern.components.length > 0) {
                // Check if any components are polygons
                const polygonComps = pattern.components.filter(comp => comp.type === 'polygon' || comp.isRegularPolygon);
                if (polygonComps.length > 0) {
                    polygonLabels = polygonComps.map(comp => `🔺 ${comp.polygonType || comp.name || 'Polygon'}`).join(' ');
                }
            } else if (pattern.expression && pattern.expression.includes('P(')) {
                // Extract polygon information from expression as fallback
                const polygonMatches = pattern.expression.match(/P\((\d+),(\d+)\)/g);
                if (polygonMatches) {
                    const polygonNames = polygonMatches.map(match => {
                        const [, vertices] = match.match(/P\((\d+),\d+\)/);
                        const sides = parseInt(vertices);
                        const name = sides === 3 ? 'Triangle' : 
                                   sides === 4 ? 'Square' : 
                                   sides === 5 ? 'Pentagon' : 
                                   sides === 6 ? 'Hexagon' : 
                                   sides === 7 ? 'Heptagon' : 
                                   sides === 8 ? 'Octagon' : 
                                   `${sides}-gon`;
                        return `🔺 ${name}`;
                    });
                    polygonLabels = polygonNames.join(' ');
                }
            } else if (pattern.formula && pattern.formula.includes('P(')) {
                // Extract polygon information from formula as fallback
                const polygonMatches = pattern.formula.match(/P\((\d+),(\d+)\)/g);
                if (polygonMatches) {
                    const polygonNames = polygonMatches.map(match => {
                        const [, vertices] = match.match(/P\((\d+),\d+\)/);
                        const sides = parseInt(vertices);
                        const name = sides === 3 ? 'Triangle' : 
                                   sides === 4 ? 'Square' : 
                                   sides === 5 ? 'Pentagon' : 
                                   sides === 6 ? 'Hexagon' : 
                                   sides === 7 ? 'Heptagon' : 
                                   sides === 8 ? 'Octagon' : 
                                   `${sides}-gon`;
                        return `🔺 ${name}`;
                    });
                    polygonLabels = polygonNames.join(' ');
                }
            }
        }

        // Get safe polygon type with proper names - check both direct and nested properties
        let polygonType = 'Polygon';
        if (pattern.polygonType) {
            polygonType = pattern.polygonType;
        } else if (pattern.polygon && pattern.polygon.polygonType) {
            polygonType = pattern.polygon.polygonType;
        } else if (pattern.vertices) {
            const sides = pattern.vertices;
            polygonType = sides === 3 ? 'Triangle' : 
                         sides === 4 ? 'Square' : 
                         sides === 5 ? 'Pentagon' : 
                         sides === 6 ? 'Hexagon' : 
                         sides === 7 ? 'Heptagon' : 
                         sides === 8 ? 'Octagon' : 
                         `${sides}-gon`;
        } else if (pattern.polygon && pattern.polygon.vertices) {
            const sides = pattern.polygon.vertices;
            polygonType = sides === 3 ? 'Triangle' : 
                         sides === 4 ? 'Square' : 
                         sides === 5 ? 'Pentagon' : 
                         sides === 6 ? 'Hexagon' : 
                         sides === 7 ? 'Heptagon' : 
                         sides === 8 ? 'Octagon' : 
                         `${sides}-gon`;
        } else if (pattern.type && pattern.type !== 'polygon') {
            polygonType = pattern.type;
        }
        
        // Check for Euclidean patterns (both input and detected)
        let isEuclideanPattern = pattern.isEuclidean;
        let euclideanFormula = 'Euclidean';
        let euclideanLabel = 'Euclidean';
        
        if (pattern.isEuclidean) {
            // Pattern was originally entered as Euclidean or detected and stored
            euclideanLabel = pattern.detectedEuclidean ? 'Detected Euclidean' : 'Euclidean';
            if (pattern.formula) {
                euclideanFormula = pattern.formula;
            } else if (pattern.euclidean && typeof pattern.euclidean === 'string') {
                euclideanFormula = pattern.euclidean;
            } else if (pattern.euclideanData && pattern.euclideanData.formula) {
                euclideanFormula = pattern.euclideanData.formula;
            } else if (pattern.beats && pattern.steps) {
                euclideanFormula = `E(${pattern.beats},${pattern.steps},${pattern.offset || 0})`;
            } else if (pattern.euclideanParams) {
                euclideanFormula = `E(${pattern.euclideanParams.beats || '?'},${pattern.euclideanParams.steps || '?'},${pattern.euclideanParams.offset || 0})`;
            }
        } else {
            // Check if this pattern matches a Euclidean rhythm (for real-time detection)
            const euclideanAnalysis = PatternAnalyzer.detectEuclideanPattern(pattern.steps, pattern.stepCount);
            if (euclideanAnalysis) {
                isEuclideanPattern = true;
                euclideanLabel = 'Detected Euclidean';
                euclideanFormula = euclideanAnalysis.formula;
            }
        }

        return `
            <div class="pattern-entry ${analysis.isPerfectlyBalanced ? 'perfect-balance' : ''} ${pattern.favorite ? 'favorite' : ''}" data-step-count="${pattern.stepCount}" data-step-count-size="${stepCountClass}">
                <span class="pattern-star ${pattern.favorite ? 'active' : ''}" onclick="app.toggleFavorite('${pattern.id}')">${pattern.favorite ? '★' : '☆'}</span>
                <div class="pattern-actions">
                    <button class="btn btn-sm success" onclick="app.loadPattern('${pattern.id}')">Load</button>
                    <button class="btn btn-sm danger" onclick="app.deletePattern('${pattern.id}')">Delete</button>
                </div>
                <div class="pattern-info">
                    <div class="pattern-header">
                        <div class="pattern-name ${!pattern.name ? 'unnamed' : ''}" onclick="app.editPatternName('${pattern.id}')">
                            ${pattern.name || AppConfig.UI.PLACEHOLDERS.NO_NAME}
                        </div>
                    </div>
                    <div class="pattern-badges">
                        <span class="step-count-badge">${pattern.stepCount} steps</span>
                        ${analysis.isPerfectlyBalanced ? '<span class="pattern-repr perfect-balance-badge">✨ Perfect Balance</span>' : ''}
                        ${pattern.isCombined ? '<span class="pattern-repr combined-type">🎯 Combined</span>' : ''}
                        ${pattern.isRegularPolygon ? `<span class="pattern-repr polygon-type">🔺 ${polygonType}</span>` : ''}
                        ${polygonLabels ? `<span class="pattern-repr polygon-components">${polygonLabels}</span>` : ''}
                        ${isEuclideanPattern ? `<span class="pattern-repr euclidean-type">🌀 ${euclideanLabel}: ${euclideanFormula}</span>` : ''}
                        ${pattern.isQuantized ? `<span class="pattern-repr quantization-type">⚡ Quantized ${pattern.quantizationDirection === 'clockwise' ? '↻' : '↺'}: ${pattern.originalStepCount}→${pattern.stepCount}</span>` : ''}
                        ${(!pattern.isRegularPolygon && !polygonLabels && ((pattern.formula && pattern.formula.includes('P(')) || (pattern.expression && pattern.expression.includes('P(')))) ? `<span class="pattern-repr polygon-type">🔺 Contains Polygons</span>` : ''}
                    </div>
                    ${this.generateLongShortDisplayForDB(pattern)}
                    ${this.generateSyncopationDisplayForDB(pattern)}
                    <div class="pattern-representations">
                        ${representations.map(repr => `<span class="pattern-repr" title="${repr}" onclick="togglePatternRepr(this)">${repr}</span>`).join('')}
                        ${pattern.isRotated ? `<span class="pattern-repr rotation-type" style="background: #fff3e0; color: #f57c00; font-weight: bold;">🔄 Rotated @${pattern.rotationSteps}</span>` : ''}
                    </div>
                </div>
            </div>
        `;
    }
    
    /**
     * Generate long-short analysis display for database entries
     */
    generateLongShortDisplayForDB(pattern) {
        // Perform the same enhanced analysis as the main display
        let longShortAnalysis = LongShortAnalyzer.analyzeLongShort(pattern.steps, pattern.stepCount);
        const onsetCount = pattern.steps.filter(s => s).length;
        
        // Add Morse detection for patterns with 4 or fewer onsets
        if (onsetCount <= 4 && longShortAnalysis.morseNotation) {
            const detectedMorse = LongShortAnalyzer.detectMorseCharacter(longShortAnalysis.morseNotation);
            if (detectedMorse) {
                longShortAnalysis.morseCharacter = detectedMorse;
                // Combine prosodic and detected Morse if both exist
                if (longShortAnalysis.description && longShortAnalysis.description !== 'No prosodic pattern') {
                    longShortAnalysis.description = `${longShortAnalysis.description} | Morse: ${detectedMorse}`;
                }
            }
        }
        
        // Handle explicit Morse patterns
        if (pattern.isMorse && pattern.morseCode) {
            const morseDescription = pattern.morseCharacters ? 
                `Morse Code: ${pattern.morseCharacters} (${pattern.morseCode})` : 
                `Morse Pattern (${pattern.morseCode})`;
            
            longShortAnalysis = {
                ...longShortAnalysis,
                morseNotation: pattern.morseCode,
                morseCharacter: pattern.morseCharacters || LongShortAnalyzer.detectMorseCharacter(pattern.morseCode),
                description: longShortAnalysis.description && longShortAnalysis.description !== 'No prosodic pattern' ?
                    `${longShortAnalysis.description} | ${morseDescription}` :
                    morseDescription
            };
        }
        
        // Skip entire display for generic interval descriptions
        const skipEntireDisplay = longShortAnalysis.description && 
            (longShortAnalysis.description.includes('Predominantly') || 
             longShortAnalysis.description.includes('intervals') ||
             longShortAnalysis.description.includes('Complex rhythmic pattern') ||
             longShortAnalysis.description.includes('No prosodic pattern'));

        // Only show if there's meaningful analysis
        if (!longShortAnalysis.description || longShortAnalysis.description === 'No prosodic pattern' || skipEntireDisplay) {
            return '';
        }

        return `
            <div class="pattern-longshort-db" style="margin: 6px 0; padding: 6px; background: #f8f9fa; border-radius: 4px; font-size: 13px;">
                <div style="font-weight: 500; color: #495057;">
                    📝 ${longShortAnalysis.description}
                </div>
                ${longShortAnalysis.morseNotation ? 
                    `<div style="font-family: monospace; color: #6c757d; margin-top: 2px; font-size: 14px;">
                        Morse: ${longShortAnalysis.morseNotation}${longShortAnalysis.morseCharacter ? ` (${longShortAnalysis.morseCharacter})` : ''}
                    </div>` : ''
                }
            </div>
        `;
    }
    
    /**
     * Generate syncopation display for database entries
     * @param {Object} pattern - Pattern object from database
     * @returns {string} HTML for syncopation display
     */
    generateSyncopationDisplayForDB(pattern) {
        // Check if syncopation analysis exists
        if (!pattern.syncopation) {
            return '';
        }
        
        const syncopation = pattern.syncopation;
        
        return `
            <div class="pattern-syncopation-db" style="margin: 6px 0; padding: 6px; background: #f0f8ff; border-radius: 4px; font-size: 13px;">
                <div style="font-weight: 500; color: #495057;">
                    🎵 ${syncopation.description} (${(syncopation.overallSyncopation * 100).toFixed(1)}%)
                </div>
                <div style="font-size: 11px; color: #6c757d; margin-top: 2px; display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 4px;">
                    <div>Note-Beat: ${(syncopation.weightedNoteToBeats * 100).toFixed(0)}%</div>
                    <div>Off-Beat: ${(syncopation.offBeatRatio * 100).toFixed(0)}%</div>
                    <div>Barlow: ${(syncopation.barlowIndispensability * 100).toFixed(0)}%</div>
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
            // Use explicit step count notation for loading
            const binary = PatternConverter.toBinary(pattern.steps, pattern.stepCount);
            const decimal = PatternConverter.toDecimal(binary, false);
            const hex = PatternConverter.toHex(decimal);
            
            // Always include explicit step count when loading
            input.value = `${hex}:${pattern.stepCount}`;
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
    
    /**
     * Update universal input field from pattern
     */
    updateUniversalInputFromPattern(pattern) {
        const input = document.getElementById('universalInput');
        if (!input || !pattern) return;
        
        try {
            // Convert pattern to hex representation with explicit step count
            const binary = PatternConverter.toBinary(pattern.steps, pattern.stepCount);
            const decimal = PatternConverter.toDecimal(binary, false);
            const hex = PatternConverter.toHex(decimal);
            
            // Update input with hex:stepCount format
            input.value = `${hex}:${pattern.stepCount}`;
            
        } catch (error) {
            console.error('❌ Failed to update universal input:', error);
        }
    }
    
    /**
     * Load pattern into sequencer for playback and visualization
     */
    loadPatternIntoSequencer(pattern, analysis) {
        if (!this.sequencer) {
            console.warn('⚠️ Sequencer not available');
            return;
        }
        
        try {
            // Create sequencer-compatible pattern object
            const sequencerPattern = SequencerIntegration.convertPatternResult(pattern);
            
            // Add CoG data for visualization
            if (analysis && analysis.cogAnalysis) {
                sequencerPattern.cogData = analysis.cogAnalysis;
            }
            
            // Load pattern into sequencer
            this.sequencer.updatePattern(sequencerPattern);
            
            console.log(`🎵 Pattern loaded into sequencer: ${sequencerPattern.name}`);
            
        } catch (error) {
            console.error('❌ Failed to load pattern into sequencer:', error);
        }
    }
    
    /**
     * Setup stochastic rhythm generator event listeners
     */
    setupStochasticEvents() {
        // Parameter slider events
        const densitySlider = document.getElementById('densitySlider');
        const metricStrengthSlider = document.getElementById('metricStrengthSlider');
        const syncopationSlider = document.getElementById('syncopationSlider');
        const densityValue = document.getElementById('densityValue');
        const metricStrengthValue = document.getElementById('metricStrengthValue');
        const syncopationValue = document.getElementById('syncopationValue');
        
        if (densitySlider && densityValue) {
            densitySlider.addEventListener('input', (e) => {
                densityValue.textContent = e.target.value + '%';
            });
        }
        
        if (metricStrengthSlider && metricStrengthValue) {
            metricStrengthSlider.addEventListener('input', (e) => {
                metricStrengthValue.textContent = e.target.value + '%';
            });
        }
        
        if (syncopationSlider && syncopationValue) {
            syncopationSlider.addEventListener('input', (e) => {
                syncopationValue.textContent = e.target.value + '%';
            });
        }
        
        // Generation button events
        const generateBtn = document.getElementById('generateStochasticBtn');
        const generateMultipleBtn = document.getElementById('generateMultipleBtn');
        const addVariationBtn = document.getElementById('addVariationBtn');
        
        if (generateBtn) {
            generateBtn.addEventListener('click', () => this.generateSingleStochasticVariation());
        }
        
        if (generateMultipleBtn) {
            generateMultipleBtn.addEventListener('click', () => this.generateMultipleStochasticVariations());
        }
        
        if (addVariationBtn) {
            addVariationBtn.addEventListener('click', () => this.addStochasticVariationToDatabase());
        }
    }
    
    /**
     * Reset stochastic controls to default state
     */
    resetStochasticControls() {
        const resultsDiv = document.getElementById('stochasticResults');
        if (resultsDiv) {
            resultsDiv.style.display = 'none';
            resultsDiv.innerHTML = '';
        }
        
        const addBtn = document.getElementById('addVariationBtn');
        if (addBtn) {
            addBtn.disabled = true;
        }
        
        this.selectedStochasticVariation = null;
    }
    
    /**
     * Get current stochastic parameters from UI and map to algorithm values
     */
    getStochasticParameters() {
        // Get percentage values from UI
        const densityPercent = parseFloat(document.getElementById('densitySlider')?.value || 50);
        const beatFocusPercent = parseFloat(document.getElementById('metricStrengthSlider')?.value || 70);
        const syncopationPercent = parseFloat(document.getElementById('syncopationSlider')?.value || 20);
        
        // Map intuitive percentages to algorithm parameters
        // Density: 0-100% maps to different probability scaling
        const baseProbability = densityPercent / 100; // 0.0 to 1.0
        
        // Beat Focus: 0-100% maps to R parameter (inverted for intuitive control)
        // Higher beat focus = lower R = more emphasis on strong beats
        const density = 1 - (beatFocusPercent / 100); // 1.0 to 0.0
        
        // Metrical strength: derived from density to create musical results
        // Lower values = higher probability = denser patterns
        const metricalStrength = (1 - baseProbability) * 2 + 0.1; // 0.1 to 2.1
        
        return {
            density: density,
            metricalStrength: metricalStrength,
            syncopationAmount: syncopationPercent / 100,
            variationMode: document.getElementById('variationMode')?.value || 'stable',
            // Store original UI values for display
            uiDensity: densityPercent,
            uiBeatFocus: beatFocusPercent,
            uiSyncopation: syncopationPercent
        };
    }
    
    /**
     * Generate a single stochastic variation
     */
    generateSingleStochasticVariation() {
        if (!this.currentPattern) {
            showNotification('No pattern loaded. Parse a pattern first.', 'warning');
            return;
        }
        
        try {
            const params = this.getStochasticParameters();
            const performance = StochasticRhythmGenerator.generatePerformance(
                this.currentPattern.steps,
                this.currentPattern.stepCount,
                params
            );
            
            this.displayStochasticResults([performance], 'Single Variation');
            
        } catch (error) {
            console.error('❌ Stochastic generation error:', error);
            showNotification('Error generating stochastic variation: ' + error.message, 'error');
        }
    }
    
    /**
     * Generate multiple stochastic variations with different parameters
     */
    generateMultipleStochasticVariations() {
        if (!this.currentPattern) {
            showNotification('No pattern loaded. Parse a pattern first.', 'warning');
            return;
        }
        
        try {
            const baseParams = this.getStochasticParameters();
            const variations = [];
            
            // Generate 3 variations with different parameter combinations
            const parameterSets = [
                { ...baseParams, density: Math.max(0, baseParams.density - 0.2), name: 'Sparse' },
                { ...baseParams, name: 'Current Settings' },
                { ...baseParams, syncopationAmount: Math.min(1, baseParams.syncopationAmount + 0.3), name: 'High Syncopation' }
            ];
            
            for (const params of parameterSets) {
                const performance = StochasticRhythmGenerator.generatePerformance(
                    this.currentPattern.steps,
                    this.currentPattern.stepCount,
                    params
                );
                performance.name = params.name;
                variations.push(performance);
            }
            
            this.displayStochasticResults(variations, 'Multiple Variations');
            
        } catch (error) {
            console.error('❌ Multiple stochastic generation error:', error);
            showNotification('Error generating variations: ' + error.message, 'error');
        }
    }
    
    /**
     * Display stochastic generation results (compact version)
     */
    displayStochasticResults(performances, title) {
        const resultsDiv = document.getElementById('stochasticResults');
        if (!resultsDiv) return;
        
        resultsDiv.style.display = 'block';
        
        const html = `
            <div class="stochastic-results-header">
                <h4>${title}</h4>
            </div>
            <div class="stochastic-variations">
                ${performances.map((perf, index) => this.renderStochasticVariationCompact(perf, index)).join('')}
            </div>
        `;
        
        resultsDiv.innerHTML = html;
        
        // Add click handlers for selection
        performances.forEach((perf, index) => {
            const variationDiv = resultsDiv.querySelector(`[data-variation-index="${index}"]`);
            if (variationDiv) {
                variationDiv.addEventListener('click', () => this.selectStochasticVariation(perf, index));
            }
        });
        
        // Auto-select first variation
        if (performances.length > 0) {
            this.selectStochasticVariation(performances[0], 0);
        }
    }
    
    /**
     * Render a single stochastic variation (compact version)
     */
    renderStochasticVariationCompact(performance, index) {
        const binary = PatternConverter.toBinary(performance.performedPattern, performance.performedPattern.length);
        const complexity = performance.complexity;
        
        return `
            <div class="stochastic-variation" data-variation-index="${index}">
                <div class="variation-header">
                    <span class="variation-name">${performance.name || `Var ${index + 1}`}</span>
                    <span class="variation-complexity">${(complexity.overall * 100).toFixed(0)}%</span>
                </div>
                <div class="variation-pattern">
                    <span class="pattern-binary">${binary}</span>
                </div>
                <div class="variation-stats">
                    <span>D:${(complexity.density * 100).toFixed(0)}%</span>
                    <span>S:${(complexity.syncopation * 100).toFixed(0)}%</span>
                </div>
            </div>
        `;
    }
    
    /**
     * Render a single stochastic variation (legacy method for compatibility)
     */
    renderStochasticVariation(performance, index) {
        return this.renderStochasticVariationCompact(performance, index);
    }
    
    /**
     * Select a stochastic variation
     */
    selectStochasticVariation(performance, index) {
        // Update UI selection
        const resultsDiv = document.getElementById('stochasticResults');
        if (resultsDiv) {
            resultsDiv.querySelectorAll('.stochastic-variation').forEach(div => {
                div.classList.remove('selected');
            });
            
            const selectedDiv = resultsDiv.querySelector(`[data-variation-index="${index}"]`);
            if (selectedDiv) {
                selectedDiv.classList.add('selected');
            }
        }
        
        // Store selection
        this.selectedStochasticVariation = performance;
        
        // Enable add to database button
        const addBtn = document.getElementById('addVariationBtn');
        if (addBtn) {
            addBtn.disabled = false;
        }
        
        // Load into sequencer for preview
        this.loadStochasticVariationIntoSequencer(performance);
    }
    
    /**
     * Load stochastic variation into sequencer for preview
     */
    loadStochasticVariationIntoSequencer(performance) {
        if (!this.sequencer) return;
        
        try {
            const variationPattern = {
                steps: performance.performedPattern,
                stepCount: performance.performedPattern.length,
                name: `Stochastic: ${performance.name || 'Variation'}`,
                isStochasticVariation: true
            };
            
            const sequencerPattern = SequencerIntegration.convertPatternResult(variationPattern);
            this.sequencer.updatePattern(sequencerPattern);
            
        } catch (error) {
            console.error('❌ Failed to load variation into sequencer:', error);
        }
    }
    
    /**
     * Add selected stochastic variation to database
     */
    addStochasticVariationToDatabase() {
        if (!this.selectedStochasticVariation) {
            showNotification('No variation selected', 'warning');
            return;
        }
        
        try {
            const performance = this.selectedStochasticVariation;
            
            // Create pattern object for database
            const patternData = {
                steps: performance.performedPattern,
                stepCount: performance.performedPattern.length,
                name: `Stochastic Variation: ${performance.name || 'Generated'}`,
                isStochasticGenerated: true,
                originalPattern: this.currentPattern,
                stochasticParameters: performance.parameters,
                complexity: performance.complexity
            };
            
            // Generate analyses for the new pattern
            const analyses = this.generateComprehensiveAnalysis(patternData);
            
            // Create database pattern
            const dbPattern = createDatabasePattern(patternData, {
                perfectBalance: analyses.balanceAnalysis,
                syncopation: analyses.syncopationAnalysis
            });
            
            // Add to database
            const patternId = this.database.add(dbPattern);
            if (patternId) {
                showNotification('Stochastic variation added to database!', 'success');
                this.updatePatternList();
                this.updateDatabaseStats();
                
                // Reset selection
                this.resetStochasticControls();
            } else {
                showNotification('Pattern already exists in database', 'info');
            }
            
        } catch (error) {
            console.error('❌ Error adding stochastic variation:', error);
            showNotification('Error adding variation to database: ' + error.message, 'error');
        }
    }
}

// Global utility functions
function togglePatternRepr(element) {
    try {
        console.log('Toggling pattern repr:', element);
        element.classList.toggle('expanded');
        
        // Temporarily select the text when expanded
        if (element.classList.contains('expanded')) {
            setTimeout(() => {
                try {
                    const range = document.createRange();
                    range.selectNodeContents(element);
                    const selection = window.getSelection();
                    selection.removeAllRanges();
                    selection.addRange(range);
                } catch (e) {
                    console.log('Selection failed:', e);
                }
            }, 50);
        }
    } catch (error) {
        console.error('Toggle error:', error);
    }
}

// Export to global scope
window.togglePatternRepr = togglePatternRepr;

// Export for global access
window.EnhancedPatternApp = EnhancedPatternApp;

console.log('✅ Enhanced Pattern App Controller loaded (browser-compatible version)');