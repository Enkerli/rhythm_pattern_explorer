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
 * 
 * Main orchestrator class that coordinates all modules and manages application state.
 * Acts as the central hub for the Rhythm Pattern Explorer, handling user interactions,
 * coordinating between modules, and maintaining application state.
 * 
 * Key Responsibilities:
 * - Initialize and coordinate all application modules
 * - Handle user interface events and interactions
 * - Manage pattern parsing, analysis, and transformations
 * - Control database operations and pattern storage
 * - Integrate audio sequencer for pattern playback
 * - Coordinate Barlow transformations and progressive sequences
 * - Manage UI state and provide user feedback
 * 
 * Architecture:
 * This class follows the coordinator pattern, delegating specific tasks to
 * specialized modules while maintaining overall application flow and state.
 * It serves as the bridge between the user interface and the mathematical
 * analysis engines.
 */
class EnhancedPatternApp {
    /**
     * Initialize the Enhanced Pattern Application
     * 
     * Sets up all application components, verifies dependencies, initializes
     * core modules, and establishes event listeners for user interactions.
     * 
     * Initialization Process:
     * 1. Verify all required dependencies are available
     * 2. Initialize core components (database, explorer, sequencer)
     * 3. Set up application state variables
     * 4. Configure event listeners for UI interactions
     * 5. Initialize application with existing data
     * 6. Set up audio sequencer for pattern playback
     * 
     * @throws {Error} If required dependencies are missing
     * 
     * Dependencies Verified:
     * - Mathematical utilities (MathUtils)
     * - Pattern generators (RegularPolygonGenerator, EuclideanGenerator)
     * - Analysis engines (PerfectBalanceAnalyzer, PatternAnalyzer, etc.)
     * - Processing utilities (UnifiedPatternParser, PatternConverter)
     * - Database and UI components
     * - Sequencer and integration modules
     * - Rhythm mutation and Barlow transformation tools
     */
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
        
        // Progressive transformation state
        this.euclideanProgressiveState = null;
        this.progressiveOffsetsState = {
            originalPattern: null,
            currentOffset: 0,
            forwardStep: 1,
            backwardStep: 1
        };
        this.progressiveLengtheningState = {
            originalPattern: null,
            extensionCount: 0,
            stepsToAdd: 4
        };
        
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
            'SequencerController', 'SequencerIntegration', 'SyncopationAnalyzer',
            'IntuitiveRhythmGenerators', 'RhythmMutator', 'BarlowTransformer', 'EuclideanTransformer'
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
        
        // Initialize transformer module
        this.initializeTransformerModule();
        
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
        
        // Progressive offsets events
        this.setupProgressiveOffsetsEvents();
        
        // Progressive lengthening events
        this.setupProgressiveLengtheningEvents();
        
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
            // Standard Enter key handler
            platformUtils.addKeyListener(universalInput, () => {
                this.parseUniversalInput();
            }, { key: 'Enter', requireModifier: false });
            
            // Platform-specific modifier+Enter handler (Cmd+Enter on Mac, Ctrl+Enter elsewhere)
            platformUtils.addKeyListener(universalInput, () => {
                this.parseUniversalInput();
                setTimeout(() => {
                    if (this.currentPattern) {
                        this.addCurrentPatternToDatabase();
                    }
                }, 100);
            }, { key: 'Enter', requireModifier: true });
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
        
        // Step count filter event listeners with cross-platform support
        if (minStepCountInput) {
            platformUtils.addKeyListener(minStepCountInput, () => {
                this.updatePatternList();
            }, { key: 'Enter' });
            
            minStepCountInput.addEventListener('input', () => {
                // Optional: Add debounced filtering on input change
                clearTimeout(this.stepCountFilterTimeout);
                this.stepCountFilterTimeout = setTimeout(() => {
                    this.updatePatternList();
                }, 500);
            });
        }
        
        if (maxStepCountInput) {
            platformUtils.addKeyListener(maxStepCountInput, () => {
                this.updatePatternList();
            }, { key: 'Enter' });
            
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
        
        // Check if we're in UPI progressive mode and should step instead of parse
        const trimmedInputValue = inputValue.trim();
        if (this.upiProgressiveState && this.upiProgressiveState.originalUPIContent === trimmedInputValue) {
            this.stepThroughUPIProgressiveSimple();
            return;
        }
        
        try {
            // console.log(`🎵 Parsing pattern: ${inputValue}`);
            
            // Clear any existing progressive state when parsing a new pattern
            if (this.upiProgressiveState && this.upiProgressiveState.originalUPIContent !== inputValue) {
                this.upiProgressiveState = null;
            }
            
            // Use UnifiedPatternParser to parse the input
            const result = UnifiedPatternParser.parse(inputValue);
            
            if (result.type === 'single') {
                if (result.pattern.isProgressiveTransformer) {
                    // console.log('🔍 Progressive transformer pattern detected:', result.pattern);
                    this.handleProgressiveTransformerPattern(result.pattern);
                    // console.log('✅ Progressive transformer pattern handled successfully');
                } else {
                    this.currentPattern = result.pattern;
                    this.displayPatternAnalysis(this.currentPattern);
                    this.showCompactOutput(this.currentPattern);
                    this.updateButtonStates();
                    this.updateUniversalInputFromPattern(this.currentPattern);
                    // console.log('✅ Pattern parsed successfully');
                }
            } else if (result.type === 'combination') {
                this.currentPattern = result.combined;
                this.displayPatternAnalysis(this.currentPattern);
                this.showCompactOutput(this.currentPattern);
                this.updateButtonStates();
                this.updateUniversalInputFromPattern(this.currentPattern);
                // console.log('✅ Combined pattern parsed successfully');
            } else if (result.type === 'stringed') {
                // console.log('🔍 Stringed pattern result:', result);
                this.currentPattern = result.pattern;
                // console.log('🔍 Current pattern after assignment:', this.currentPattern);
                this.displayPatternAnalysis(this.currentPattern);
                this.showCompactOutput(this.currentPattern);
                this.updateButtonStates();
                this.updateUniversalInputFromPattern(this.currentPattern);
                // console.log('✅ Stringed pattern parsed successfully');
            } else if (result.type === 'quantized') {
                // console.log('🔍 Quantized pattern result:', result);
                this.currentPattern = result.pattern;
                // console.log('🔍 Current pattern after assignment:', this.currentPattern);
                this.displayPatternAnalysis(this.currentPattern);
                this.showCompactOutput(this.currentPattern);
                this.updateButtonStates();
                this.updateUniversalInputFromPattern(this.currentPattern);
                // console.log('✅ Quantized pattern parsed successfully');
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
                
                const balanceInfo = balanceSection.querySelector('.balance-analysis-column');
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
                            <div class="info-box tex2jax_process" id="balance-equation" style="display: none; margin-top: 8px; font-size: 12px; background: #f8f9fa; padding: 6px; border-radius: 4px; border-left: 3px solid #28a745;">
                                <strong>Milne's Formula:</strong><br>
                                $$\\frac{\\left|\\sum_{j=1}^{k} e^{i2\\pi k_j/n}\\right|}{\\text{onsets}}$$
                                <div style="margin-top: 8px;"><strong>Value:</strong> ${analysis.balanceAnalysis.normalizedMagnitude.toFixed(6)}</div>
                            </div>
                        </div>
                        <div class="analysis-box cog-box" style="margin-top: 8px;">
                            <div class="analysis-box-header">
                                <span class="analysis-box-icon">🎯</span>
                                <span class="analysis-box-title">Center of Gravity</span>
                                <button class="info-btn-inline" onclick="toggleInfoBox('cog-equation')" title="Show mathematical formula">🧮</button>
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
                            <div class="info-box tex2jax_process" id="cog-equation" style="display: none; margin-top: 8px; font-size: 12px; background: #f8f9fa; padding: 6px; border-radius: 4px; border-left: 3px solid #17a2b8;">
                                <strong>Center of Gravity:</strong> $$CoG = \\frac{1}{k}\\sum_{j=1}^{k} e^{i2\\pi k_j/n}$$
                                <div class="equation-legend">Angle: $\\theta = \\arctan2(y, x) \\times \\frac{180}{\\pi}$</div>
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
            
            // Show and populate mutator section
            const stochasticSection = document.getElementById('stochasticSection');
            if (stochasticSection) {
                stochasticSection.style.display = 'block';
                // Reset any previous mutation state and update original pattern display
                this.resetStochasticControls();
                this.updateOriginalPatternDisplay(pattern);
            }
            
            // Show generator section
            const generatorSection = document.getElementById('generatorSection');
            if (generatorSection) {
                generatorSection.style.display = 'block';
                // Reset any previous generation state
                this.resetGeneratorControls();
            }
            
            // Show Barlow transformer section
            const barlowSection = document.getElementById('barlowSection');
            if (barlowSection) {
                barlowSection.style.display = 'block';
                // Only reset controls if we're not in the middle of a Barlow transformation
                if (!pattern.isBarlowTransformed || !this.selectedBarlowTransformation) {
                    this.resetBarlowControls();
                }
                this.updateCurrentOnsetsDisplay(pattern);
            }
            
            // Show Euclidean transformer section
            const euclideanSection = document.getElementById('euclideanSection');
            if (euclideanSection) {
                euclideanSection.style.display = 'block';
                // Only reset controls if we're not in the middle of a Euclidean transformation or progressive sequence
                if (!pattern.isEuclideanTransformed || (!this.selectedEuclideanTransformation && !this.euclideanProgressiveState)) {
                    this.resetEuclideanControls();
                }
                this.updateCurrentEuclideanOnsetsDisplay(pattern);
            }
            
            // Show Progressive Offsets section
            const progressiveOffsetsSection = document.getElementById('progressiveOffsetsSection');
            if (progressiveOffsetsSection) {
                progressiveOffsetsSection.style.display = 'block';
                // Reset controls if we're not in the middle of progressive offsets
                if (!pattern.isOffset || !this.progressiveOffsetsState.originalPattern) {
                    this.resetProgressiveOffsetsControls();
                }
                this.updateCurrentOffsetsPatternDisplay(pattern);
            }
            
            // Show Progressive Lengthening section
            const progressiveLengtheningSection = document.getElementById('progressiveLengtheningSection');
            if (progressiveLengtheningSection) {
                progressiveLengtheningSection.style.display = 'block';
                // Reset controls if we're not in the middle of progressive lengthening
                if (!pattern.isExtended || !this.progressiveLengtheningState.originalPattern) {
                    this.resetProgressiveLengtheningControls();
                }
                this.updateCurrentLengtheningPatternDisplay(pattern);
                this.updatePatternLengthDisplay();
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
        // console.log('🔍 Generating analysis for pattern:', pattern);
        
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
            
            // console.log(`🎵 Pattern loaded into sequencer: ${sequencerPattern.name}`);
            
        } catch (error) {
            console.error('❌ Failed to load pattern into sequencer:', error);
        }
    }
    
    /**
     * Setup rhythm mutator event listeners
     */
    setupStochasticEvents() {
        // Mutator parameter slider events
        const mutationAmountSlider = document.getElementById('mutationAmount');
        const mutationValue = document.getElementById('mutationValue');
        
        if (mutationAmountSlider && mutationValue) {
            mutationAmountSlider.addEventListener('input', (e) => {
                mutationValue.textContent = e.target.value + '%';
            });
        }
        
        // Mutator button events
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
        
        // Generator parameter slider events
        const genDensitySlider = document.getElementById('genDensitySlider');
        const genDensityValue = document.getElementById('genDensityValue');
        const genFunkinessSlider = document.getElementById('genFunkinessSlider');
        const genFunkinessValue = document.getElementById('genFunkinessValue');
        
        if (genDensitySlider && genDensityValue) {
            genDensitySlider.addEventListener('input', (e) => {
                genDensityValue.textContent = e.target.value + '%';
            });
        }
        
        if (genFunkinessSlider && genFunkinessValue) {
            genFunkinessSlider.addEventListener('input', (e) => {
                genFunkinessValue.textContent = e.target.value + '%';
            });
        }
        
        // Generator button events
        const generateFunkyBtn = document.getElementById('generateFunkyBtn');
        const generateMultipleFunkyBtn = document.getElementById('generateMultipleFunkyBtn');
        const addGeneratedBtn = document.getElementById('addGeneratedBtn');
        
        if (generateFunkyBtn) {
            generateFunkyBtn.addEventListener('click', () => this.generateSingleFunkyPattern());
        }
        
        if (generateMultipleFunkyBtn) {
            generateMultipleFunkyBtn.addEventListener('click', () => this.generateMultipleFunkyPatterns());
        }
        
        if (addGeneratedBtn) {
            addGeneratedBtn.addEventListener('click', () => this.addGeneratedPatternToDatabase());
        }
        
        // Barlow transformer button events
        const transformBarlowBtn = document.getElementById('transformBarlowBtn');
        const progressiveBarlowBtn = document.getElementById('progressiveBarlowBtn');
        const addBarlowBtn = document.getElementById('addBarlowBtn');
        
        if (transformBarlowBtn) {
            transformBarlowBtn.addEventListener('click', () => this.transformBarlowPattern());
        }
        
        if (progressiveBarlowBtn) {
            progressiveBarlowBtn.addEventListener('click', () => this.progressiveBarlowTransform());
        }
        
        if (addBarlowBtn) {
            addBarlowBtn.addEventListener('click', () => this.addBarlowPatternToDatabase());
        }
        
        // Target onsets input Enter key event - Cross-platform support
        const targetOnsetsInput = document.getElementById('targetOnsets');
        if (targetOnsetsInput) {
            platformUtils.addKeyListener(targetOnsetsInput, () => {
                this.progressiveBarlowTransform();
            }, { key: 'Enter' });
        }
        
        // Euclidean transformer button events
        const transformEuclideanBtn = document.getElementById('transformEuclideanBtn');
        const progressiveEuclideanBtn = document.getElementById('progressiveEuclideanBtn');
        const addEuclideanBtn = document.getElementById('addEuclideanBtn');
        
        if (transformEuclideanBtn) {
            transformEuclideanBtn.addEventListener('click', () => this.transformEuclideanPattern());
        }
        
        if (progressiveEuclideanBtn) {
            progressiveEuclideanBtn.addEventListener('click', () => this.generateProgressiveEuclideanTransformations());
        }
        
        if (addEuclideanBtn) {
            addEuclideanBtn.addEventListener('click', () => this.addEuclideanTransformationToDatabase());
        }
        
        // Target Euclidean onsets input Enter key event - Cross-platform support
        const targetEuclideanOnsetsInput = document.getElementById('targetEuclideanOnsets');
        if (targetEuclideanOnsetsInput) {
            platformUtils.addKeyListener(targetEuclideanOnsetsInput, () => {
                this.generateProgressiveEuclideanTransformations();
            }, { key: 'Enter' });
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
     * Get current mutator parameters from UI
     */
    getStochasticParameters() {
        return {
            mutationStyle: document.getElementById('mutationStyle')?.value || 'balanced',
            mutationAmount: parseFloat(document.getElementById('mutationAmount')?.value || 30) / 100
        };
    }
    
    /**
     * Update the original pattern display
     */
    updateOriginalPatternDisplay(pattern) {
        const originalDisplay = document.getElementById('originalPattern');
        if (originalDisplay && pattern && pattern.steps) {
            const binary = PatternConverter.toBinary(pattern.steps, pattern.stepCount);
            const onsetCount = pattern.steps.filter(step => step).length;
            originalDisplay.textContent = `${binary} (${onsetCount} onsets)`;
            originalDisplay.classList.add('pattern-loaded');
        }
    }
    
    /**
     * Generate a single morphed rhythm variation
     */
    generateSingleStochasticVariation() {
        if (!this.currentPattern) {
            showNotification('No pattern loaded. Parse a pattern first.', 'warning');
            return;
        }
        
        try {
            const params = this.getStochasticParameters();
            
            // Mutate the current pattern
            const result = RhythmMutator.mutatePattern(
                this.currentPattern.steps,
                params.mutationAmount,
                {
                    mutationStyle: params.mutationStyle,
                    preserveOnsetCount: true
                }
            );
            
            // Convert to expected format
            const performance = {
                performedPattern: result.mutated,
                originalPattern: result.pattern,
                parameters: params,
                name: result.description,
                type: 'morphed',
                displacement: result.displacement,
                complexity: this.calculateSimpleComplexity(result.mutated)
            };
            
            this.displayStochasticResults([performance], 'Mutated Pattern');
            
        } catch (error) {
            console.error('❌ Mutation error:', error);
            showNotification('Error mutating rhythm: ' + error.message, 'error');
        }
    }
    
    /**
     * Calculate simple complexity metrics for a pattern
     */
    calculateSimpleComplexity(pattern) {
        const density = pattern.filter(step => step).length / pattern.length;
        const syncopation = this.calculateSimpleSyncopation(pattern);
        const overall = Math.sqrt(Math.pow(density - 0.5, 2) + Math.pow(syncopation - 0.5, 2));
        
        return { density, syncopation, overall };
    }
    
    /**
     * Calculate simple syncopation level
     */
    calculateSimpleSyncopation(pattern) {
        const quarter = Math.floor(pattern.length / 4);
        const strongBeats = [0, quarter, quarter * 2, quarter * 3];
        const onsetPositions = pattern.map((step, index) => step ? index : -1)
                                     .filter(index => index !== -1);
        
        const syncopatedOnsets = onsetPositions.filter(pos => !strongBeats.includes(pos));
        return syncopatedOnsets.length / Math.max(onsetPositions.length, 1);
    }
    
    /**
     * Generate multiple mutated variations with different amounts
     */
    generateMultipleStochasticVariations() {
        if (!this.currentPattern) {
            showNotification('No pattern loaded. Parse a pattern first.', 'warning');
            return;
        }
        
        try {
            const baseParams = this.getStochasticParameters();
            const variations = [];
            
            // Generate 3 variations with different mutation amounts
            const mutationAmounts = [
                { amount: Math.max(0.1, baseParams.mutationAmount - 0.2), name: 'Subtle' },
                { amount: baseParams.mutationAmount, name: 'Current' },
                { amount: Math.min(1, baseParams.mutationAmount + 0.3), name: 'Heavy' }
            ];
            
            for (const mutationConfig of mutationAmounts) {
                const result = RhythmMutator.mutatePattern(
                    this.currentPattern.steps,
                    mutationConfig.amount,
                    {
                        mutationStyle: baseParams.mutationStyle,
                        preserveOnsetCount: true
                    }
                );
                
                const performance = {
                    performedPattern: result.mutated,
                    originalPattern: result.pattern,
                    parameters: { ...baseParams, mutationAmount: mutationConfig.amount },
                    name: mutationConfig.name,
                    type: 'mutated',
                    displacement: result.displacement,
                    complexity: this.calculateSimpleComplexity(result.mutated)
                };
                
                variations.push(performance);
            }
            
            this.displayStochasticResults(variations, 'Multiple Mutations');
            
        } catch (error) {
            console.error('❌ Multiple mutation error:', error);
            showNotification('Error generating mutation variations: ' + error.message, 'error');
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
        
        // Auto-select first variation only if there's just one
        if (performances.length === 1) {
            this.selectStochasticVariation(performances[0], 0);
        }
    }
    
    /**
     * Render a single stochastic variation (compact version)
     */
    renderStochasticVariationCompact(performance, index) {
        const binary = PatternConverter.toBinary(performance.performedPattern, performance.performedPattern.length);
        const complexity = performance.complexity;
        
        // Show displacement info for mutated patterns
        let displacementInfo = '';
        if (performance.displacement && performance.displacement.normalized) {
            const displacementPercent = Math.round(performance.displacement.normalized * 100);
            displacementInfo = `<span>Disp:${displacementPercent}%</span>`;
        }
        
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
                    ${displacementInfo}
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
        
        // Handle different types of variations
        if (performance.type === 'barlow_transform') {
            // For Barlow transformations, load as input pattern
            const transformationData = {
                transformedPattern: performance.performedPattern,
                originalPattern: performance.originalPattern,
                parameters: performance.parameters,
                name: performance.name,
                type: performance.type,
                transformation: performance.transformation,
                metadata: performance.metadata
            };
            this.loadTransformedPatternAsInput(transformationData);
        } else {
            // For regular mutations/generations, load into sequencer for preview
            this.loadStochasticVariationIntoSequencer(performance);
        }
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
     * Add selected mutated variation to database
     */
    addStochasticVariationToDatabase() {
        if (!this.selectedStochasticVariation) {
            showNotification('No variation selected', 'warning');
            return;
        }
        
        try {
            const performance = this.selectedStochasticVariation;
            
            // Handle different types of variations
            if (performance.type === 'barlow_transform') {
                // Use the dedicated Barlow database method
                this.addBarlowPatternToDatabase();
                return;
            }
            
            // Create pattern object for database with proper mutation metadata
            const patternData = {
                steps: performance.performedPattern,
                stepCount: performance.performedPattern.length,
                name: `Mutated: ${performance.name || 'Variation'}`,
                isMutated: true,
                originalPattern: {
                    steps: performance.originalPattern,
                    stepCount: performance.originalPattern.length,
                    binary: PatternConverter.toBinary(performance.originalPattern, performance.originalPattern.length),
                    name: this.currentPattern.name || 'Source Pattern'
                },
                mutationParameters: performance.parameters,
                displacement: performance.displacement,
                complexity: performance.complexity
            };
            
            // Generate analyses for the new pattern
            const analyses = this.generateComprehensiveAnalysis(patternData);
            
            // Create database pattern
            const dbPattern = createDatabasePattern(patternData, {
                perfectBalance: analyses.balanceAnalysis,
                syncopation: analyses.syncopationAnalysis
            });
            
            // Add mutation-specific metadata to database pattern
            dbPattern.isMutated = true;
            dbPattern.originalPattern = patternData.originalPattern;
            dbPattern.mutationParameters = patternData.mutationParameters;
            dbPattern.displacement = patternData.displacement;
            
            // Add to database
            const patternId = this.database.add(dbPattern);
            if (patternId) {
                showNotification('Mutated pattern added to database!', 'success');
                this.updatePatternList();
                this.updateDatabaseStats();
                
                // Reset selection
                this.resetStochasticControls();
            } else {
                showNotification('Pattern already exists in database', 'info');
            }
            
        } catch (error) {
            console.error('❌ Error adding mutated pattern:', error);
            showNotification('Error adding mutated pattern to database: ' + error.message, 'error');
        }
    }
    
    /**
     * Get current generator parameters from UI
     */
    getGeneratorParameters() {
        return {
            generatorType: document.getElementById('generatorType')?.value || 'groove',
            stepCount: parseInt(document.getElementById('stepCountInput')?.value || 16),
            density: parseFloat(document.getElementById('genDensitySlider')?.value || 60) / 100,
            style: document.getElementById('genStyleSelect')?.value || 'funk',
            funkiness: parseFloat(document.getElementById('genFunkinessSlider')?.value || 50) / 100
        };
    }
    
    /**
     * Generate a single funky pattern
     */
    generateSingleFunkyPattern() {
        try {
            const params = this.getGeneratorParameters();
            
            // Generate the pattern using IntuitiveRhythmGenerators
            const result = IntuitiveRhythmGenerators.generate(
                params.generatorType,
                params.stepCount,
                {
                    density: params.density,
                    style: params.style,
                    funkiness: params.funkiness
                }
            );
            
            if (!result || !result.steps) {
                throw new Error('Generator failed to create pattern');
            }
            
            // Convert to expected format
            const performance = {
                performedPattern: result.steps,
                originalPattern: null, // Generated patterns have no original
                parameters: params,
                name: result.description || `${params.generatorType} pattern`,
                type: 'generated',
                complexity: this.calculateSimpleComplexity(result.steps)
            };
            
            this.displayGeneratorResults([performance], 'Generated Pattern');
            
        } catch (error) {
            console.error('❌ Generation error:', error);
            showNotification('Error generating pattern: ' + error.message, 'error');
        }
    }
    
    /**
     * Generate multiple funky patterns with variations
     */
    generateMultipleFunkyPatterns() {
        try {
            const baseParams = this.getGeneratorParameters();
            const variations = [];
            
            // Generate 3 variations with different parameters
            const paramVariations = [
                { ...baseParams, density: Math.max(0.1, baseParams.density - 0.2), name: 'Sparse' },
                { ...baseParams, name: 'Current' },
                { ...baseParams, density: Math.min(1, baseParams.density + 0.2), name: 'Dense' }
            ];
            
            for (const params of paramVariations) {
                const result = IntuitiveRhythmGenerators.generate(
                    params.generatorType,
                    params.stepCount,
                    {
                        density: params.density,
                        style: params.style,
                        funkiness: params.funkiness
                    }
                );
                
                if (result && result.steps) {
                    const performance = {
                        performedPattern: result.steps,
                        originalPattern: null,
                        parameters: params,
                        name: params.name,
                        type: 'generated',
                        complexity: this.calculateSimpleComplexity(result.steps)
                    };
                    
                    variations.push(performance);
                }
            }
            
            this.displayGeneratorResults(variations, 'Generated Variations');
            
        } catch (error) {
            console.error('❌ Multiple generation error:', error);
            showNotification('Error generating pattern variations: ' + error.message, 'error');
        }
    }
    
    /**
     * Display generator results
     */
    displayGeneratorResults(performances, title) {
        const resultsDiv = document.getElementById('generatorResults');
        if (!resultsDiv) return;
        
        resultsDiv.style.display = 'block';
        
        const html = `
            <div class="stochastic-results-header">
                <h4>${title}</h4>
            </div>
            <div class="stochastic-variations">
                ${performances.map((performance, index) => 
                    this.renderGeneratorVariationCompact(performance, index)
                ).join('')}
            </div>
        `;
        
        resultsDiv.innerHTML = html;
        
        // Add click event listeners for selection
        resultsDiv.querySelectorAll('.stochastic-variation').forEach((div, index) => {
            div.addEventListener('click', () => this.selectGeneratedVariation(performances[index], index));
        });
        
        // Auto-select first variation only if there's just one
        if (performances.length === 1) {
            this.selectGeneratedVariation(performances[0], 0);
        }
    }
    
    /**
     * Render a generator variation in compact format
     */
    renderGeneratorVariationCompact(performance, index) {
        const binary = PatternConverter.toBinary(performance.performedPattern, performance.performedPattern.length);
        const complexity = performance.complexity;
        const onsetCount = performance.performedPattern.filter(step => step).length;
        
        return `
            <div class="stochastic-variation" data-variation-index="${index}">
                <div class="variation-header">
                    <span class="variation-name">${performance.name || `Gen ${index + 1}`}</span>
                    <span class="variation-complexity">${(complexity.overall * 100).toFixed(0)}%</span>
                </div>
                <div class="variation-pattern">
                    <span class="pattern-binary">${binary}</span>
                </div>
                <div class="variation-stats">
                    <span>D:${(complexity.density * 100).toFixed(0)}%</span>
                    <span>S:${(complexity.syncopation * 100).toFixed(0)}%</span>
                    <span>O:${onsetCount}</span>
                </div>
            </div>
        `;
    }
    
    /**
     * Select a generated variation
     */
    selectGeneratedVariation(performance, index) {
        // Update UI selection
        const resultsDiv = document.getElementById('generatorResults');
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
        this.selectedGeneratedPattern = performance;
        
        // Enable add to database button
        const addBtn = document.getElementById('addGeneratedBtn');
        if (addBtn) {
            addBtn.disabled = false;
        }
        
        // Load the generated pattern as the new current pattern
        this.loadGeneratedPatternAsInput(performance);
        
        showNotification(`Selected generated pattern: ${performance.name}`, 'info');
    }
    
    /**
     * Add selected generated pattern to database
     */
    addGeneratedPatternToDatabase() {
        if (!this.selectedGeneratedPattern) {
            showNotification('No generated pattern selected', 'warning');
            return;
        }
        
        try {
            const performance = this.selectedGeneratedPattern;
            
            // Create descriptive name for database
            const onsetCount = performance.performedPattern.filter(step => step).length;
            const stepCount = performance.performedPattern.length;
            const params = performance.parameters;
            
            let descriptiveName = '';
            switch (params.generatorType) {
                case 'groove':
                    descriptiveName = `Generated-Groove(${params.style}, ${Math.round(params.density*100)}%, ${stepCount})`;
                    break;
                case 'funky-euclidean':
                    descriptiveName = `Generated-FunkyEuclidean(${Math.round(params.density*100)}%, ${Math.round(params.funkiness*100)}%, ${stepCount})`;
                    break;
                case 'probabilistic':
                    descriptiveName = `Generated-Probabilistic(${Math.round(params.density*100)}%, ${stepCount})`;
                    break;
                case 'polyrhythm':
                    descriptiveName = `Generated-Polyrhythm(${Math.round(params.density*100)}%, ${stepCount})`;
                    break;
                default:
                    descriptiveName = `Generated-${params.generatorType}(${onsetCount}, ${stepCount})`;
            }
            
            // Create pattern object for database
            const patternData = {
                steps: performance.performedPattern,
                stepCount: performance.performedPattern.length,
                name: descriptiveName,
                isGenerated: true,
                generatorType: performance.parameters.generatorType,
                generatorParameters: performance.parameters,
                complexity: performance.complexity
            };
            
            // Generate analyses for the new pattern
            const analyses = this.generateComprehensiveAnalysis(patternData);
            
            // Create database pattern
            const dbPattern = createDatabasePattern(patternData, analyses);
            
            // Add to database
            const patternId = this.database.add(dbPattern);
            
            if (patternId) {
                showNotification(`Generated pattern added to database!`, 'success');
                this.updatePatternList();
                this.updateDatabaseStats();
                
                // Reset generator controls
                this.resetGeneratorControls();
            } else {
                showNotification('Pattern already exists in database', 'info');
            }
            
        } catch (error) {
            console.error('❌ Error adding generated pattern to database:', error);
            showNotification('Error adding pattern to database: ' + error.message, 'error');
        }
    }
    
    /**
     * Reset generator controls to default state
     */
    resetGeneratorControls() {
        const resultsDiv = document.getElementById('generatorResults');
        if (resultsDiv) {
            resultsDiv.style.display = 'none';
            resultsDiv.innerHTML = '';
        }
        
        const addBtn = document.getElementById('addGeneratedBtn');
        if (addBtn) {
            addBtn.disabled = true;
        }
        
        this.selectedGeneratedPattern = null;
    }
    
    /**
     * Update current onsets display for Barlow transformer
     */
    updateCurrentOnsetsDisplay(pattern) {
        const currentOnsetsDisplay = document.getElementById('currentOnsets');
        if (currentOnsetsDisplay && pattern && pattern.steps) {
            const onsetCount = pattern.steps.filter(step => step).length;
            currentOnsetsDisplay.textContent = `${onsetCount} onsets`;
            currentOnsetsDisplay.classList.add('pattern-loaded');
            
            // Update target onsets input max value only
            const targetOnsetsInput = document.getElementById('targetOnsets');
            if (targetOnsetsInput) {
                targetOnsetsInput.max = pattern.stepCount;
                // Don't change the user's target value at all - let them set it
            }
        }
    }
    
    /**
     * Get current Barlow transformer parameters from UI
     */
    getBarlowParameters() {
        return {
            targetOnsets: parseInt(document.getElementById('targetOnsets')?.value || 4),
            operation: document.getElementById('barlowOperation')?.value || 'auto',
            wolrabMode: document.getElementById('wolrabMode')?.checked || false
        };
    }
    
    /**
     * Transform pattern using Barlow indispensability
     */
    transformBarlowPattern() {
        if (!this.currentPattern) {
            showNotification('No pattern loaded. Parse a pattern first.', 'warning');
            return;
        }
        
        try {
            const params = this.getBarlowParameters();
            
            // Apply transformation
            const result = BarlowTransformer.transformPattern(
                this.currentPattern.steps,
                params.targetOnsets,
                {
                    preserveDownbeat: true, // Always preserve downbeat
                    avoidWeakBeats: false,
                    minimumIndispensability: 0.0, // Allow all positions for concentration
                    wolrabMode: params.wolrabMode
                }
            );
            
            // Convert to expected format
            const transformation = {
                transformedPattern: result.pattern,
                originalPattern: result.originalPattern,
                parameters: params,
                name: result.description,
                type: 'barlow_transform',
                transformation: result.transformation,
                metadata: {
                    onsetsRemoved: result.onsetsRemoved || 0,
                    onsetsAdded: result.onsetsAdded || 0,
                    removedPositions: result.removedPositions || [],
                    addedPositions: result.addedPositions || [],
                    indispensabilityRanking: result.indispensabilityRanking
                }
            };
            
            // Convert to the same format as stochastic variations for consistent display
            const variation = {
                performedPattern: transformation.transformedPattern,
                originalPattern: transformation.originalPattern,
                parameters: transformation.parameters,
                name: transformation.name,
                type: transformation.type,
                transformation: transformation.transformation,
                metadata: transformation.metadata,
                complexity: this.calculateSimpleComplexity(transformation.transformedPattern)
            };
            
            this.displayBarlowResults([transformation], 'Barlow Transform');
            
        } catch (error) {
            console.error('❌ Barlow transformation error:', error);
            showNotification('Error transforming pattern: ' + error.message, 'error');
        }
    }
    
    /**
     * Generate progressive Barlow transformations from current to target
     */
    progressiveBarlowTransform() {
        if (!this.currentPattern) {
            showNotification('No pattern loaded. Parse a pattern first.', 'warning');
            return;
        }
        
        try {
            const params = this.getBarlowParameters();
            const currentOnsets = this.currentPattern.steps.filter(step => step).length;
            const targetOnsets = params.targetOnsets;
            
            console.log(`📊 Current onsets: ${currentOnsets}, Target onsets: ${targetOnsets}`);
            
            if (currentOnsets === targetOnsets) {
                showNotification('Current and target onset counts are the same', 'info');
                return;
            }
            
            // Generate sequence from current to target
            const transformations = [];
            const direction = targetOnsets > currentOnsets ? 1 : -1; // 1 for concentration, -1 for dilution
            
            console.log(`🎯 Progressive: ${currentOnsets} → ${targetOnsets} (direction: ${direction})`);
            
            let currentPattern = [...this.currentPattern.steps];
            
            // Generate each step in the sequence
            for (let onsets = currentOnsets + direction; 
                 direction > 0 ? onsets <= targetOnsets : onsets >= targetOnsets; 
                 onsets += direction) {
                
                console.log(`🔄 Generating step: ${onsets} onsets`);
                
                const result = BarlowTransformer.transformPattern(
                    currentPattern,
                    onsets,
                    {
                        preserveDownbeat: true,
                        avoidWeakBeats: false,
                        minimumIndispensability: 0.0,
                        wolrabMode: params.wolrabMode
                    }
                );
                
                const transformation = {
                    transformedPattern: result.pattern,
                    originalPattern: [...this.currentPattern.steps], // Always reference original
                    parameters: params,
                    name: `Step ${onsets} onsets`,
                    type: 'barlow_transform',
                    transformation: result.transformation,
                    stepNumber: onsets,
                    metadata: {
                        onsetsRemoved: result.onsetsRemoved || 0,
                        onsetsAdded: result.onsetsAdded || 0,
                        removedPositions: result.removedPositions || [],
                        addedPositions: result.addedPositions || [],
                        indispensabilityRanking: result.indispensabilityRanking
                    }
                };
                
                transformations.push(transformation);
                
                // Update current pattern for next iteration
                currentPattern = [...result.pattern];
            }
            
            console.log(`✅ Generated ${transformations.length} progressive transformations`);
            
            if (transformations.length === 0) {
                showNotification('No transformations generated. Check target onsets value.', 'warning');
                return;
            }
            
            // Convert to the same format as stochastic variations for consistent display
            const variations = transformations.map(transformation => ({
                performedPattern: transformation.transformedPattern,
                originalPattern: transformation.originalPattern,
                parameters: transformation.parameters,
                name: transformation.name,
                type: transformation.type,
                transformation: transformation.transformation,
                metadata: transformation.metadata,
                complexity: this.calculateSimpleComplexity(transformation.transformedPattern)
            }));
            
            this.displayBarlowResults(transformations, `Progressive: ${currentOnsets} → ${targetOnsets} onsets`);
            
        } catch (error) {
            console.error('❌ Progressive Barlow error:', error);
            showNotification('Error generating progressive transformations: ' + error.message, 'error');
        }
    }
    
    /**
     * Display Barlow transformation results
     */
    displayBarlowResults(transformations, title) {
        console.log(`🖼️ Displaying ${transformations.length} Barlow results with title: ${title}`);
        
        const resultsDiv = document.getElementById('barlowResults');
        if (!resultsDiv) {
            console.error('❌ barlowResults div not found!');
            return;
        }
        
        const html = `
            <div class="stochastic-results-header">
                <h4>${title}</h4>
            </div>
            <div class="stochastic-variations">
                ${transformations.map((transformation, index) => 
                    this.renderBarlowVariationCompact(transformation, index)
                ).join('')}
            </div>
        `;
        
        resultsDiv.innerHTML = html;
        
        // Force display after setting HTML
        resultsDiv.style.display = 'block';
        resultsDiv.style.visibility = 'visible';
        
        // Add click event listeners for selection
        resultsDiv.querySelectorAll('.stochastic-variation').forEach((div, index) => {
            div.addEventListener('click', () => this.selectBarlowVariation(transformations[index], index));
        });
        
        // Auto-select first transformation
        if (transformations.length > 0) {
            // Use setTimeout to ensure DOM has been updated
            setTimeout(() => {
                this.selectBarlowVariation(transformations[0], 0);
            }, 100);
        }
    }
    
    /**
     * Render a Barlow transformation variation in compact format
     */
    renderBarlowVariationCompact(transformation, index) {
        const binary = PatternConverter.toBinary(transformation.transformedPattern, transformation.transformedPattern.length);
        const onsetCount = transformation.transformedPattern.filter(step => step).length;
        const originalOnsets = transformation.originalPattern.filter(step => step).length;
        const changeType = onsetCount > originalOnsets ? '+' : onsetCount < originalOnsets ? '-' : '=';
        const changeAmount = Math.abs(onsetCount - originalOnsets);
        
        return `
            <div class="stochastic-variation" data-variation-index="${index}">
                <div class="variation-header">
                    <span class="variation-name">${transformation.name || `Transform ${index + 1}`}</span>
                    <span class="variation-complexity">${changeType}${changeAmount}</span>
                </div>
                <div class="variation-pattern">
                    <span class="pattern-binary">${binary}</span>
                </div>
                <div class="variation-stats">
                    <span>O:${onsetCount}</span>
                    <span>T:${transformation.transformation}</span>
                    ${transformation.metadata && transformation.metadata.onsetsRemoved > 0 ? `<span>R:${transformation.metadata.onsetsRemoved}</span>` : ''}
                    ${transformation.metadata && transformation.metadata.onsetsAdded > 0 ? `<span>A:${transformation.metadata.onsetsAdded}</span>` : ''}
                </div>
            </div>
        `;
    }
    
    /**
     * Select a Barlow transformation variation
     */
    selectBarlowVariation(transformation, index) {
        // Update UI selection
        const resultsDiv = document.getElementById('barlowResults');
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
        this.selectedBarlowTransformation = transformation;
        
        // Enable add to database button
        const addBtn = document.getElementById('addBarlowBtn');
        if (addBtn) {
            addBtn.disabled = false;
            addBtn.style.opacity = '1';
        }
        
        // Load the transformed pattern as the new current pattern  
        this.loadTransformedPatternAsInput(transformation);
        
        showNotification(`Selected transformation: ${transformation.name}`, 'info');
    }
    
    /**
     * Add selected Barlow transformation to database
     */
    addBarlowPatternToDatabase() {
        const transformation = this.selectedBarlowTransformation;
        
        if (!transformation) {
            showNotification('No transformation selected', 'warning');
            return;
        }
        
        try {
            const transformedPattern = transformation.transformedPattern;
            const originalPattern = transformation.originalPattern;
            
            // Create descriptive name for database
            const onsetCount = transformedPattern.filter(step => step).length;
            const stepCount = transformedPattern.length;
            const originalOnsets = originalPattern.filter(step => step).length;
            
            let descriptiveName = '';
            if (transformation.transformation === 'dilution') {
                descriptiveName = `Barlow-Diluted(${originalOnsets}→${onsetCount}, ${stepCount})`;
            } else if (transformation.transformation === 'concentration') {
                descriptiveName = `Barlow-Concentrated(${originalOnsets}→${onsetCount}, ${stepCount})`;
            } else if (transformation.transformation === 'wolrab-dilution') {
                descriptiveName = `Wolrab-Diluted(${originalOnsets}→${onsetCount}, ${stepCount})`;
            } else if (transformation.transformation === 'wolrab-concentration') {
                descriptiveName = `Wolrab-Concentrated(${originalOnsets}→${onsetCount}, ${stepCount})`;
            } else {
                descriptiveName = `Barlow-Transform(${onsetCount}, ${stepCount})`;
            }
            
            // Create pattern object for database
            const patternData = {
                steps: transformedPattern,
                stepCount: transformedPattern.length,
                name: descriptiveName,
                isBarlowTransformed: true,
                originalPattern: {
                    steps: transformation.originalPattern,
                    stepCount: transformation.originalPattern.length,
                    binary: PatternConverter.toBinary(transformation.originalPattern, transformation.originalPattern.length),
                    name: this.currentPattern.name || 'Source Pattern'
                },
                barlowParameters: transformation.parameters,
                barlowMetadata: transformation.metadata,
                transformation: transformation.transformation
            };
            
            // Generate analyses for the new pattern
            const analyses = this.generateComprehensiveAnalysis(patternData);
            
            // Create database pattern
            const dbPattern = createDatabasePattern(patternData, analyses);
            
            // Add to database
            const patternId = this.database.add(dbPattern);
            
            if (patternId) {
                showNotification(`Barlow transformation added to database!`, 'success');
                this.updatePatternList();
                this.updateDatabaseStats();
                
                // Reset Barlow controls
                this.resetBarlowControls();
            } else {
                showNotification('Pattern already exists in database', 'info');
            }
            
        } catch (error) {
            console.error('❌ Error adding Barlow pattern to database:', error);
            showNotification('Error adding pattern to database: ' + error.message, 'error');
        }
    }
    
    /**
     * Reset Barlow controls to default state
     */
    resetBarlowControls() {
        const resultsDiv = document.getElementById('barlowResults');
        if (resultsDiv) {
            resultsDiv.style.display = 'none';
            resultsDiv.innerHTML = '';
        }
        
        const addBtn = document.getElementById('addBarlowBtn');
        if (addBtn) {
            addBtn.disabled = true;
        }
        
        this.selectedBarlowTransformation = null;
    }
    
    /**
     * Load a transformed pattern as the new input pattern
     */
    loadTransformedPatternAsInput(transformation) {
        try {
            // Create pattern data for the transformed pattern
            const transformedPatternData = {
                steps: transformation.transformedPattern,
                stepCount: transformation.transformedPattern.length,
                name: transformation.name || 'Transformed Pattern',
                expression: this.generateTransformationExpression(transformation),
                isBarlowTransformed: true,
                originalPattern: transformation.originalPattern,
                transformation: transformation.transformation,
                metadata: transformation.metadata
            };
            
            // Update the universal input field
            const universalInput = document.getElementById('universalInput');
            if (universalInput) {
                universalInput.value = transformedPatternData.expression;
            }
            
            // Set as current pattern and trigger full analysis
            this.currentPattern = transformedPatternData;
            
            // Generate comprehensive analysis
            const analysis = this.generateComprehensiveAnalysis(transformedPatternData);
            
            // Update UI components without full analysis (to preserve results display)
            this.updateOriginalPatternDisplay(transformedPatternData);
            this.updateCurrentOnsetsDisplay(transformedPatternData);
            
            // Load into sequencer for preview (if available)
            if (this.sequencer) {
                this.sequencer.updatePattern(transformedPatternData);
            }
            
        } catch (error) {
            console.error('❌ Error loading transformed pattern as input:', error);
            showNotification('Error loading transformed pattern: ' + error.message, 'error');
        }
    }
    
    /**
     * Generate a parsable expression for a transformation
     */
    generateTransformationExpression(transformation) {
        const stepCount = transformation.transformedPattern.length;
        
        // Convert to parsable format (binary with step count)
        const binary = PatternConverter.toBinary(transformation.transformedPattern, stepCount);
        return `b${binary}:${stepCount}`;
    }
    
    /**
     * Load a generated pattern as the new input pattern
     */
    loadGeneratedPatternAsInput(performance) {
        try {
            // Create pattern data for the generated pattern
            const generatedPatternData = {
                steps: performance.performedPattern,
                stepCount: performance.performedPattern.length,
                name: performance.name || 'Generated Pattern',
                expression: this.generateGenerationExpression(performance),
                isGenerated: true,
                generatorType: performance.parameters.generatorType,
                generatorParameters: performance.parameters,
                complexity: performance.complexity
            };
            
            // Update the universal input field
            const universalInput = document.getElementById('universalInput');
            if (universalInput) {
                universalInput.value = generatedPatternData.expression;
            }
            
            // Set as current pattern and trigger full analysis
            this.currentPattern = generatedPatternData;
            
            // Generate comprehensive analysis
            const analysis = this.generateComprehensiveAnalysis(generatedPatternData);
            
            // Update UI components without full analysis (to preserve results display)
            this.updateOriginalPatternDisplay(generatedPatternData);
            this.updateCurrentOnsetsDisplay(generatedPatternData);
            
            // Load into sequencer for preview (if available)
            if (this.sequencer) {
                this.sequencer.updatePattern(generatedPatternData);
            }
            
        } catch (error) {
            console.error('❌ Error loading generated pattern as input:', error);
            showNotification('Error loading generated pattern: ' + error.message, 'error');
        }
    }
    
    /**
     * Update the current Euclidean onsets display
     */
    updateCurrentEuclideanOnsetsDisplay(pattern) {
        const currentEuclideanOnsetsDisplay = document.getElementById('currentEuclideanOnsets');
        if (currentEuclideanOnsetsDisplay && pattern && pattern.steps) {
            const onsetCount = pattern.steps.filter(step => step).length;
            currentEuclideanOnsetsDisplay.textContent = `${onsetCount} onsets`;
            currentEuclideanOnsetsDisplay.classList.add('pattern-loaded');
            
            // Update target onsets input max value only
            const targetEuclideanOnsetsInput = document.getElementById('targetEuclideanOnsets');
            if (targetEuclideanOnsetsInput) {
                targetEuclideanOnsetsInput.max = pattern.stepCount;
                // Don't change the user's target value at all - let them set it
            }
        }
    }
    
    /**
     * Get current Euclidean transformer parameters from UI
     */
    getEuclideanParameters() {
        return {
            targetOnsets: parseInt(document.getElementById('targetEuclideanOnsets')?.value || 4),
            operation: document.getElementById('euclideanOperation')?.value || 'auto',
            mode: document.getElementById('euclideanMode')?.value || 'normal'
        };
    }
    
    /**
     * Transform pattern using Euclidean distribution
     */
    transformEuclideanPattern() {
        if (!this.currentPattern) {
            showNotification('No pattern loaded. Parse a pattern first.', 'warning');
            return;
        }
        
        try {
            const params = this.getEuclideanParameters();
            
            const result = EuclideanTransformer.transform(
                this.currentPattern.steps,
                params.targetOnsets,
                {
                    operation: params.operation,
                    mode: params.mode
                }
            );
            
            const transformation = {
                transformedPattern: result.transformed,
                originalPattern: result.original,
                parameters: params,
                name: result.description,
                type: 'euclidean_transformed',
                metadata: result.metadata
            };
            
            this.displayEuclideanResults([transformation], 'Euclidean Transformation');
            
        } catch (error) {
            console.error('❌ Euclidean transformation error:', error);
            showNotification('Error transforming pattern: ' + error.message, 'error');
        }
    }
    
    /**
     * Generate progressive Euclidean transformations (step-by-step)
     */
    generateProgressiveEuclideanTransformations() {
        if (!this.currentPattern) {
            showNotification('No pattern loaded. Parse a pattern first.', 'warning');
            return;
        }
        
        try {
            const params = this.getEuclideanParameters();
            const currentOnsets = this.currentPattern.steps.filter(step => step).length;
            const targetOnsets = params.targetOnsets;
            
            console.log(`🔍 Progressive state check: target=${targetOnsets}, currentStep=${this.euclideanProgressiveState?.currentStep}, current pattern onsets=${currentOnsets}`);
            
            // Check if we need to initialize progressive state
            const hasState = !!this.euclideanProgressiveState;
            const targetChanged = this.euclideanProgressiveState?.targetOnsets !== targetOnsets;
            const stepCompleted = this.euclideanProgressiveState?.currentStep >= this.euclideanProgressiveState?.transformations?.length;
            
            // Only check pattern change if we're not in the middle of a progressive sequence
            // (pattern changes during progressive sequences are expected)
            const inProgressiveSequence = hasState && this.euclideanProgressiveState.currentStep >= 0;
            const patternChanged = !inProgressiveSequence && 
                JSON.stringify(this.euclideanProgressiveState?.originalPattern) !== JSON.stringify(this.currentPattern.steps);
            
            console.log(`🔍 State checks: hasState=${hasState}, targetChanged=${targetChanged}, stepCompleted=${stepCompleted}, inProgressiveSequence=${inProgressiveSequence}, patternChanged=${patternChanged}`);
            
            const needsNewSequence = !hasState || targetChanged || stepCompleted || patternChanged;
                
            if (needsNewSequence) {
                console.log(`🆕 Creating new progressive sequence from ${currentOnsets} to ${targetOnsets} onsets`);
                
                // For progressive transformations, use current pattern as starting point
                const startingPattern = this.currentPattern.steps;
                const startingOnsets = startingPattern.filter(step => step).length;
                
                // Initialize new progressive sequence
                this.euclideanProgressiveState = {
                    originalPattern: [...startingPattern],
                    targetOnsets: targetOnsets,
                    currentOnsets: startingOnsets,
                    direction: targetOnsets > startingOnsets ? 1 : -1,
                    currentStep: -1, // Start at -1 so first press shows full list
                    transformations: []
                };
                
                // Generate all transformations from the current pattern
                const result = EuclideanTransformer.generateProgressive(
                    startingPattern,
                    params.targetOnsets,
                    {
                        operation: params.operation,
                        mode: params.mode
                    }
                );
                
                this.euclideanProgressiveState.transformations = result.variations.map((variation, index) => ({
                    transformedPattern: variation.transformed,
                    originalPattern: variation.original,
                    parameters: params,
                    name: variation.description,
                    type: 'euclidean_transformed',
                    metadata: variation.metadata
                }));
                
                if (this.euclideanProgressiveState.transformations.length === 0) {
                    showNotification('No transformations needed - already at target onsets.', 'info');
                    return;
                }
            }
            
            // Handle progressive stepping
            this.euclideanProgressiveState.currentStep++;
            console.log(`👆 Step incremented to: ${this.euclideanProgressiveState.currentStep}, total transformations: ${this.euclideanProgressiveState.transformations.length}`);
            
            if (this.euclideanProgressiveState.currentStep === 0) {
                // First press: show full list and immediately apply first transformation
                console.log(`📋 First press - showing full list of ${this.euclideanProgressiveState.transformations.length} transformations and applying first one`);
                this.displayEuclideanResults(
                    this.euclideanProgressiveState.transformations, 
                    `Progressive: ${this.euclideanProgressiveState.currentOnsets} → ${targetOnsets} onsets`
                );
                
                // Immediately apply the first transformation
                if (this.euclideanProgressiveState.transformations.length > 0) {
                    const firstTransformation = this.euclideanProgressiveState.transformations[0];
                    this.selectEuclideanTransformation(firstTransformation, 0);
                    this.loadEuclideanTransformedPatternAsInput(firstTransformation);
                    
                    // Shorten the list by removing the completed step
                    const remainingTransformations = this.euclideanProgressiveState.transformations.slice(1);
                    if (remainingTransformations.length > 0) {
                        this.displayEuclideanResults(
                            remainingTransformations, 
                            `Progressive: ${remainingTransformations.length} steps remaining`
                        );
                    }
                    
                    showNotification(`Step 1/${this.euclideanProgressiveState.transformations.length}: ${firstTransformation.name}`, 'info');
                    this.euclideanProgressiveState.currentStep = 1; // Mark that we've applied the first step
                } else {
                    showNotification(`Generated ${this.euclideanProgressiveState.transformations.length} progressive transformations. Press Enter again to step through them.`, 'info');
                }
            } else if (this.euclideanProgressiveState.currentStep <= this.euclideanProgressiveState.transformations.length) {
                // Subsequent presses: step through the list
                const transformationIndex = this.euclideanProgressiveState.currentStep - 1;
                const transformation = this.euclideanProgressiveState.transformations[transformationIndex];
                
                // Auto-select the current transformation
                this.selectEuclideanTransformation(transformation, transformationIndex);
                
                // Load the transformed pattern as the new current pattern (like Barlow does)
                this.loadEuclideanTransformedPatternAsInput(transformation);
                
                showNotification(`Step ${this.euclideanProgressiveState.currentStep}/${this.euclideanProgressiveState.transformations.length}: ${transformation.name}`, 'info');
                
                // Shorten the list by removing the completed step and redisplay
                const remainingTransformations = this.euclideanProgressiveState.transformations.slice(this.euclideanProgressiveState.currentStep);
                if (remainingTransformations.length > 0) {
                    this.displayEuclideanResults(
                        remainingTransformations, 
                        `Progressive: ${remainingTransformations.length} steps remaining`
                    );
                }
            } else {
                showNotification('Reached end of progressive sequence. Parse a new pattern to restart.', 'info');
            }
            
        } catch (error) {
            console.error('❌ Progressive Euclidean error:', error);
            showNotification('Error generating progressive transformations: ' + error.message, 'error');
        }
    }
    
    /**
     * Display Euclidean transformation results
     */
    displayEuclideanResults(transformations, title) {
        const resultsDiv = document.getElementById('euclideanResults');
        if (!resultsDiv) return;
        
        resultsDiv.style.display = 'block';
        
        const html = `
            <div class="stochastic-results-header">
                <h4>${title}</h4>
            </div>
            <div class="stochastic-variations">
                ${transformations.map((transformation, index) => this.renderEuclideanTransformationCompact(transformation, index)).join('')}
            </div>
        `;
        
        resultsDiv.innerHTML = html;
        
        // Add click handlers for selection
        transformations.forEach((transformation, index) => {
            const variationDiv = resultsDiv.querySelector(`[data-euclidean-index="${index}"]`);
            if (variationDiv) {
                variationDiv.addEventListener('click', () => this.selectEuclideanTransformation(transformation, index));
            }
        });
        
        // Auto-select first transformation only if there's just one
        if (transformations.length === 1) {
            this.selectEuclideanTransformation(transformations[0], 0);
        }
    }
    
    /**
     * Render a single Euclidean transformation (compact version)
     */
    renderEuclideanTransformationCompact(transformation, index) {
        const binary = PatternConverter.toBinary(transformation.transformedPattern, transformation.transformedPattern.length);
        const onsetCount = transformation.transformedPattern.filter(step => step).length;
        
        return `
            <div class="stochastic-variation" data-euclidean-index="${index}">
                <div class="variation-header">
                    <span class="variation-name">${transformation.name || `Transform ${index + 1}`}</span>
                    <span class="variation-complexity">${onsetCount} onsets</span>
                </div>
                <div class="variation-pattern">
                    <span class="pattern-binary">${binary}</span>
                </div>
            </div>
        `;
    }
    
    /**
     * Select a specific Euclidean transformation
     */
    selectEuclideanTransformation(transformation, index) {
        this.selectedEuclideanTransformation = transformation;
        
        // Update UI to show selection
        const resultsDiv = document.getElementById('euclideanResults');
        if (resultsDiv) {
            resultsDiv.querySelectorAll('.stochastic-variation').forEach((div, i) => {
                div.classList.toggle('selected', i === index);
            });
        }
        
        // Enable add to database button
        const addBtn = document.getElementById('addEuclideanBtn');
        if (addBtn) {
            addBtn.disabled = false;
        }
        
        showNotification(`Selected Euclidean transformation: ${transformation.name}`, 'success');
    }
    
    /**
     * Add selected Euclidean transformation to database
     */
    addEuclideanTransformationToDatabase() {
        if (!this.selectedEuclideanTransformation) {
            showNotification('No Euclidean transformation selected', 'warning');
            return;
        }
        
        try {
            const transformation = this.selectedEuclideanTransformation;
            
            const patternData = {
                steps: transformation.transformedPattern,
                stepCount: transformation.transformedPattern.length,
                name: transformation.name || 'Euclidean Transformation',
                expression: this.generateEuclideanTransformationExpression(transformation),
                isEuclideanTransformed: true,
                originalPattern: transformation.originalPattern,
                transformation: transformation.parameters,
                metadata: transformation.metadata
            };
            
            if (this.database.addPattern(patternData)) {
                showNotification('Euclidean transformation added to database!', 'success');
                this.updateDatabaseDisplay();
                this.resetEuclideanControls();
            } else {
                showNotification('Pattern already exists in database', 'info');
            }
            
        } catch (error) {
            console.error('❌ Error adding Euclidean pattern to database:', error);
            showNotification('Error adding pattern to database: ' + error.message, 'error');
        }
    }
    
    /**
     * Reset Euclidean controls to default state
     */
    resetEuclideanControls() {
        const resultsDiv = document.getElementById('euclideanResults');
        if (resultsDiv) {
            resultsDiv.style.display = 'none';
            resultsDiv.innerHTML = '';
        }
        
        const addBtn = document.getElementById('addEuclideanBtn');
        if (addBtn) {
            addBtn.disabled = true;
        }
        
        this.selectedEuclideanTransformation = null;
        this.euclideanProgressiveState = null; // Reset progressive state
    }
    
    /**
     * Reset Progressive Offsets controls to default state
     */
    resetProgressiveOffsetsControls() {
        const resultsDiv = document.getElementById('progressiveOffsetsResults');
        if (resultsDiv) {
            resultsDiv.style.display = 'none';
            resultsDiv.innerHTML = '';
        }
        
        const addBtn = document.getElementById('addOffsetsBtn');
        if (addBtn) {
            addBtn.disabled = true;
        }
        
        // Reset input fields to defaults
        const forwardStepInput = document.getElementById('forwardOffsetStep');
        const backwardStepInput = document.getElementById('backwardOffsetStep');
        
        if (forwardStepInput) forwardStepInput.value = '1';
        if (backwardStepInput) backwardStepInput.value = '1';
        
        // Reset state if no progressive sequence in progress
        if (!this.progressiveOffsetsState.originalPattern) {
            this.progressiveOffsetsState = {
                originalPattern: null,
                currentOffset: 0,
                forwardStep: 1,
                backwardStep: 1
            };
        }
        
        this.updateCurrentOffsetDisplay();
    }
    
    /**
     * Reset Progressive Lengthening controls to default state
     */
    resetProgressiveLengtheningControls() {
        const resultsDiv = document.getElementById('progressiveLengtheningResults');
        if (resultsDiv) {
            resultsDiv.style.display = 'none';
            resultsDiv.innerHTML = '';
        }
        
        const addBtn = document.getElementById('addLengtheningBtn');
        if (addBtn) {
            addBtn.disabled = true;
        }
        
        // Reset input fields to defaults
        const stepsToAddInput = document.getElementById('stepsToAdd');
        if (stepsToAddInput) stepsToAddInput.value = '4';
        
        // Reset state if no progressive sequence in progress
        if (!this.progressiveLengtheningState.originalPattern) {
            this.progressiveLengtheningState = {
                originalPattern: null,
                extensionCount: 0,
                stepsToAdd: 4
            };
        }
        
        this.updateExtensionCountDisplay();
    }
    
    /**
     * Load a Euclidean transformed pattern as the new input pattern
     */
    loadEuclideanTransformedPatternAsInput(transformation) {
        try {
            // Create pattern data for the transformed pattern
            const transformedPatternData = {
                steps: transformation.transformedPattern,
                stepCount: transformation.transformedPattern.length,
                name: transformation.name || 'Euclidean Transformed Pattern',
                expression: this.generateEuclideanTransformationExpression(transformation),
                isEuclideanTransformed: true,
                originalPattern: transformation.originalPattern,
                transformation: transformation.parameters,
                metadata: transformation.metadata
            };
            
            // Update the universal input field
            const universalInput = document.getElementById('universalInput');
            if (universalInput) {
                universalInput.value = transformedPatternData.expression;
            }
            
            // Set as current pattern
            this.currentPattern = transformedPatternData;
            
            // Update UI components without full analysis (to preserve results display)
            this.updateOriginalPatternDisplay(transformedPatternData);
            this.updateCurrentEuclideanOnsetsDisplay(transformedPatternData);
            
            // Load into sequencer for preview (if available)
            if (this.sequencer) {
                this.sequencer.updatePattern(transformedPatternData);
            }
            
            console.log('✅ Euclidean transformed pattern loaded as input:', transformedPatternData.expression);
            
        } catch (error) {
            console.error('❌ Error loading Euclidean transformed pattern as input:', error);
            showNotification('Error loading transformed pattern: ' + error.message, 'error');
        }
    }
    
    /**
     * Generate a parsable expression for a Euclidean transformation
     */
    generateEuclideanTransformationExpression(transformation) {
        const stepCount = transformation.transformedPattern.length;
        
        // Convert to parsable format (binary with step count)
        const binary = PatternConverter.toBinary(transformation.transformedPattern, stepCount);
        return `b${binary}:${stepCount}`;
    }
    
    /**
     * Generate a parsable expression for a generated pattern
     */
    generateGenerationExpression(performance) {
        const stepCount = performance.performedPattern.length;
        
        // Convert to parsable format (binary with step count)
        const binary = PatternConverter.toBinary(performance.performedPattern, stepCount);
        return `b${binary}:${stepCount}`;
    }
    
    /**
     * Setup progressive offsets event listeners
     */
    setupProgressiveOffsetsEvents() {
        const forwardStepInput = document.getElementById('forwardOffsetStep');
        const backwardStepInput = document.getElementById('backwardOffsetStep');
        const resetBtn = document.getElementById('resetOffsetsBtn');
        const manualBtn = document.getElementById('manualOffsetBtn');
        const addBtn = document.getElementById('addOffsetsBtn');
        
        // Forward step input - Cross-platform Enter key for progressive forward offset
        if (forwardStepInput) {
            platformUtils.addKeyListener(forwardStepInput, () => {
                this.progressiveOffsetsState.forwardStep = parseInt(forwardStepInput.value) || 1;
                this.applyProgressiveOffset('forward');
            }, { key: 'Enter' });
            
            forwardStepInput.addEventListener('input', (e) => {
                this.progressiveOffsetsState.forwardStep = parseInt(e.target.value) || 1;
            });
        }
        
        // Backward step input - Cross-platform Enter key for progressive backward offset
        if (backwardStepInput) {
            platformUtils.addKeyListener(backwardStepInput, () => {
                this.progressiveOffsetsState.backwardStep = parseInt(backwardStepInput.value) || 1;
                this.applyProgressiveOffset('backward');
            }, { key: 'Enter' });
            
            backwardStepInput.addEventListener('input', (e) => {
                this.progressiveOffsetsState.backwardStep = parseInt(e.target.value) || 1;
            });
        }
        
        // Reset button
        if (resetBtn) {
            resetBtn.addEventListener('click', () => this.resetProgressiveOffsets());
        }
        
        // Manual offset button  
        if (manualBtn) {
            manualBtn.addEventListener('click', () => this.applyManualOffset());
        }
        
        // Add to database button
        if (addBtn) {
            addBtn.addEventListener('click', () => this.addOffsetPatternToDatabase());
        }
    }
    
    /**
     * Apply progressive offset in specified direction
     */
    applyProgressiveOffset(direction) {
        if (!this.currentPattern) {
            showNotification('No pattern loaded. Parse a pattern first.', 'warning');
            return;
        }
        
        // Initialize original pattern if not set
        if (!this.progressiveOffsetsState.originalPattern) {
            this.progressiveOffsetsState.originalPattern = {
                steps: [...this.currentPattern.steps],
                stepCount: this.currentPattern.stepCount,
                name: this.currentPattern.name || 'Original Pattern'
            };
            this.progressiveOffsetsState.currentOffset = 0;
        }
        
        // Calculate new offset
        const stepSize = direction === 'forward' ? this.progressiveOffsetsState.forwardStep : this.progressiveOffsetsState.backwardStep;
        const offsetDelta = direction === 'forward' ? stepSize : -stepSize;
        this.progressiveOffsetsState.currentOffset += offsetDelta;
        
        // Apply offset to original pattern
        const offsetPattern = this.rotatePattern(
            this.progressiveOffsetsState.originalPattern.steps,
            this.progressiveOffsetsState.currentOffset
        );
        
        // Create offset pattern data
        const offsetPatternData = {
            steps: offsetPattern,
            stepCount: this.progressiveOffsetsState.originalPattern.stepCount,
            name: `${this.progressiveOffsetsState.originalPattern.name} (offset ${this.progressiveOffsetsState.currentOffset})`,
            expression: this.generateOffsetExpression(offsetPattern, this.progressiveOffsetsState.currentOffset),
            isOffset: true,
            originalPattern: this.progressiveOffsetsState.originalPattern,
            offset: this.progressiveOffsetsState.currentOffset
        };
        
        // Update UI and load pattern
        this.loadOffsetPatternAsInput(offsetPatternData);
        
        console.log(`✅ Applied ${direction} offset: ${this.progressiveOffsetsState.currentOffset}`);
    }
    
    /**
     * Apply manual offset using current step values
     */
    applyManualOffset() {
        if (!this.currentPattern) {
            showNotification('No pattern loaded. Parse a pattern first.', 'warning');
            return;
        }
        
        const forwardStep = this.progressiveOffsetsState.forwardStep;
        const backwardStep = this.progressiveOffsetsState.backwardStep;
        const netOffset = forwardStep - backwardStep;
        
        // Apply offset to current pattern
        const offsetPattern = this.rotatePattern(this.currentPattern.steps, netOffset);
        
        // Create offset pattern data
        const offsetPatternData = {
            steps: offsetPattern,
            stepCount: this.currentPattern.stepCount,
            name: `${this.currentPattern.name || 'Pattern'} (manual offset ${netOffset})`,
            expression: this.generateOffsetExpression(offsetPattern, netOffset),
            isOffset: true,
            originalPattern: this.currentPattern,
            offset: netOffset
        };
        
        // Update UI and load pattern
        this.loadOffsetPatternAsInput(offsetPatternData);
        
        console.log(`✅ Applied manual offset: ${netOffset}`);
    }
    
    /**
     * Reset progressive offsets to original pattern
     */
    resetProgressiveOffsets() {
        if (!this.progressiveOffsetsState.originalPattern) {
            showNotification('No progressive offsets in progress', 'info');
            return;
        }
        
        // Reset to original pattern
        this.progressiveOffsetsState.currentOffset = 0;
        this.loadOffsetPatternAsInput(this.progressiveOffsetsState.originalPattern);
        
        // Update current offset display
        this.updateCurrentOffsetDisplay();
        
        console.log('✅ Reset to original pattern');
    }
    
    /**
     * Rotate pattern by specified offset
     */
    rotatePattern(steps, offset) {
        if (!steps || steps.length === 0) return [];
        
        const stepCount = steps.length;
        // Normalize offset to be within pattern length
        const normalizedOffset = ((offset % stepCount) + stepCount) % stepCount;
        
        // Rotate the pattern
        return [...steps.slice(normalizedOffset), ...steps.slice(0, normalizedOffset)];
    }
    
    /**
     * Load offset pattern as input
     */
    loadOffsetPatternAsInput(offsetPatternData) {
        try {
            // Update universal input field
            const universalInput = document.getElementById('universalInput');
            if (universalInput) {
                universalInput.value = offsetPatternData.expression;
            }
            
            // Set as current pattern
            this.currentPattern = offsetPatternData;
            
            // Update UI displays
            this.updateCurrentOffsetsPatternDisplay(offsetPatternData);
            this.updateCurrentOffsetDisplay();
            
            // Load into sequencer for preview
            if (this.sequencer) {
                this.sequencer.updatePattern(offsetPatternData);
            }
            
            // Enable add button
            const addBtn = document.getElementById('addOffsetsBtn');
            if (addBtn) addBtn.disabled = false;
            
            console.log('✅ Offset pattern loaded as input:', offsetPatternData.expression);
            
        } catch (error) {
            console.error('❌ Error loading offset pattern as input:', error);
            showNotification('Error loading offset pattern: ' + error.message, 'error');
        }
    }
    
    /**
     * Update current offsets pattern display
     */
    updateCurrentOffsetsPatternDisplay(pattern) {
        const display = document.getElementById('currentOffsetsPattern');
        if (display && pattern && pattern.steps) {
            const binary = PatternConverter.toBinary(pattern.steps, pattern.stepCount);
            const onsetCount = pattern.steps.filter(step => step).length;
            display.textContent = `${binary} (${onsetCount} onsets)`;
            display.classList.add('pattern-loaded');
        }
    }
    
    /**
     * Update current offset display
     */
    updateCurrentOffsetDisplay() {
        const display = document.getElementById('currentOffsetValue');
        if (display) {
            display.textContent = this.progressiveOffsetsState.currentOffset.toString();
        }
    }
    
    /**
     * Generate expression for offset pattern
     */
    generateOffsetExpression(steps, offset) {
        const stepCount = steps.length;
        const binary = PatternConverter.toBinary(steps, stepCount);
        return `b${binary}:${stepCount}`;
    }
    
    /**
     * Add offset pattern to database
     */
    addOffsetPatternToDatabase() {
        if (!this.currentPattern || !this.currentPattern.isOffset) {
            showNotification('No offset pattern to add', 'warning');
            return;
        }
        
        try {
            // Create pattern data for database
            const patternData = {
                steps: this.currentPattern.steps,
                stepCount: this.currentPattern.stepCount,
                name: this.currentPattern.name,
                isOffset: true,
                originalPattern: this.currentPattern.originalPattern,
                offset: this.currentPattern.offset
            };
            
            // Generate analyses
            const analyses = this.generateComprehensiveAnalysis(patternData);
            
            // Create database pattern
            const dbPattern = createDatabasePattern(patternData, {
                perfectBalance: analyses.balanceAnalysis,
                syncopation: analyses.syncopationAnalysis
            });
            
            // Add offset-specific metadata
            dbPattern.isOffset = true;
            dbPattern.originalPattern = patternData.originalPattern;
            dbPattern.offset = patternData.offset;
            
            // Add to database
            const patternId = this.database.add(dbPattern);
            if (patternId) {
                showNotification('Offset pattern added to database!', 'success');
                this.updatePatternList();
                this.updateDatabaseStats();
            } else {
                showNotification('Pattern already exists in database', 'info');
            }
            
        } catch (error) {
            console.error('❌ Error adding offset pattern:', error);
            showNotification('Error adding offset pattern to database: ' + error.message, 'error');
        }
    }
    
    /**
     * Setup progressive lengthening event listeners
     */
    setupProgressiveLengtheningEvents() {
        const stepsToAddInput = document.getElementById('stepsToAdd');
        const resetBtn = document.getElementById('resetLengtheningBtn');
        const manualBtn = document.getElementById('manualLengtheningBtn');
        const addBtn = document.getElementById('addLengtheningBtn');
        
        // Steps to add input - Cross-platform Enter key for progressive lengthening
        if (stepsToAddInput) {
            platformUtils.addKeyListener(stepsToAddInput, () => {
                this.progressiveLengtheningState.stepsToAdd = parseInt(stepsToAddInput.value) || 4;
                this.applyProgressiveLengthening();
            }, { key: 'Enter' });
            
            stepsToAddInput.addEventListener('input', (e) => {
                this.progressiveLengtheningState.stepsToAdd = parseInt(e.target.value) || 4;
            });
        }
        
        // Reset button
        if (resetBtn) {
            resetBtn.addEventListener('click', () => this.resetProgressiveLengthening());
        }
        
        // Manual lengthening button
        if (manualBtn) {
            manualBtn.addEventListener('click', () => this.applyManualLengthening());
        }
        
        // Add to database button
        if (addBtn) {
            addBtn.addEventListener('click', () => this.addLengtheningPatternToDatabase());
        }
    }
    
    /**
     * Apply progressive lengthening by appending random steps
     */
    applyProgressiveLengthening() {
        if (!this.currentPattern) {
            showNotification('No pattern loaded. Parse a pattern first.', 'warning');
            return;
        }
        
        // Initialize original pattern if not set
        if (!this.progressiveLengtheningState.originalPattern) {
            this.progressiveLengtheningState.originalPattern = {
                steps: [...this.currentPattern.steps],
                stepCount: this.currentPattern.stepCount,
                name: this.currentPattern.name || 'Original Pattern'
            };
            this.progressiveLengtheningState.extensionCount = 0;
        }
        
        // Generate random extension using bell curve distribution
        const extensionSteps = this.progressiveLengtheningState.stepsToAdd;
        const randomOnsetCount = MathUtils.randomOnsetCount(extensionSteps);
        
        // Create random pattern for extension
        const extensionPattern = this.generateRandomExtension(extensionSteps, randomOnsetCount);
        
        // Get current extended pattern or original if first extension
        const currentExtended = this.progressiveLengtheningState.extensionCount === 0 
            ? this.progressiveLengtheningState.originalPattern.steps
            : this.currentPattern.steps;
        
        // Append extension to current pattern
        const extendedSteps = [...currentExtended, ...extensionPattern];
        const newStepCount = extendedSteps.length;
        
        // Increment extension count
        this.progressiveLengtheningState.extensionCount++;
        
        // Create extended pattern data
        const extendedPatternData = {
            steps: extendedSteps,
            stepCount: newStepCount,
            name: `${this.progressiveLengtheningState.originalPattern.name} (${this.progressiveLengtheningState.extensionCount}x extended)`,
            expression: this.generateLengtheningExpression(extendedSteps),
            isExtended: true,
            originalPattern: this.progressiveLengtheningState.originalPattern,
            extensionCount: this.progressiveLengtheningState.extensionCount,
            lastExtension: extensionPattern
        };
        
        // Update UI and load pattern
        this.loadLengtheningPatternAsInput(extendedPatternData);
        
        console.log(`✅ Applied progressive lengthening: +${extensionSteps} steps (${randomOnsetCount} onsets)`);
    }
    
    /**
     * Generate random pattern for extension using bell curve distribution
     */
    generateRandomExtension(steps, onsetCount) {
        const extension = new Array(steps).fill(false);
        const onsetPositions = new Set();
        
        // Generate random onset positions
        while (onsetPositions.size < onsetCount) {
            const randomPos = Math.floor(Math.random() * steps);
            onsetPositions.add(randomPos);
        }
        
        // Set onsets in extension
        onsetPositions.forEach(pos => {
            extension[pos] = true;
        });
        
        return extension;
    }
    
    /**
     * Apply manual lengthening (one-time extension)
     */
    applyManualLengthening() {
        if (!this.currentPattern) {
            showNotification('No pattern loaded. Parse a pattern first.', 'warning');
            return;
        }
        
        // Set up state for manual lengthening if needed
        if (!this.progressiveLengtheningState.originalPattern) {
            this.progressiveLengtheningState.originalPattern = {
                steps: [...this.currentPattern.steps],
                stepCount: this.currentPattern.stepCount,
                name: this.currentPattern.name || 'Original Pattern'
            };
            this.progressiveLengtheningState.extensionCount = 0;
        }
        
        this.applyProgressiveLengthening();
    }
    
    /**
     * Reset progressive lengthening to original pattern
     */
    resetProgressiveLengthening() {
        if (!this.progressiveLengtheningState.originalPattern) {
            showNotification('No original pattern stored', 'warning');
            return;
        }
        
        // Reset to original pattern
        const originalPatternData = {
            steps: [...this.progressiveLengtheningState.originalPattern.steps],
            stepCount: this.progressiveLengtheningState.originalPattern.stepCount,
            name: this.progressiveLengtheningState.originalPattern.name
        };
        
        // Reset state
        this.progressiveLengtheningState.extensionCount = 0;
        
        // Load original pattern
        this.loadLengtheningPatternAsInput(originalPatternData);
        
        console.log('✅ Reset to original pattern length');
    }
    
    /**
     * Load lengthening pattern as input and update UI
     */
    loadLengtheningPatternAsInput(patternData) {
        try {
            // Generate comprehensive analysis
            const analyses = this.generateComprehensiveAnalysis(patternData);
            
            // Set all required properties
            patternData.expression = patternData.expression || this.generateLengtheningExpression(patternData.steps);
            patternData.metadata = {
                timestamp: Date.now(),
                type: patternData.isExtended ? 'extended' : 'original',
                isGenerated: false
            };
            
            // Add analysis results
            patternData.analysis = analyses;
            
            // Set as current pattern
            this.currentPattern = patternData;
            
            // Update UI displays
            this.updateCurrentLengtheningPatternDisplay(patternData);
            this.updateExtensionCountDisplay();
            this.updatePatternLengthDisplay();
            
            // Load into sequencer for preview
            if (this.sequencer) {
                this.sequencer.updatePattern(patternData);
            }
            
            // Enable add button
            const addBtn = document.getElementById('addLengtheningBtn');
            if (addBtn) addBtn.disabled = false;
            
            console.log('✅ Lengthening pattern loaded as input:', patternData.expression);
            
        } catch (error) {
            console.error('❌ Error loading lengthening pattern as input:', error);
            showNotification('Error loading lengthening pattern: ' + error.message, 'error');
        }
    }
    
    /**
     * Update current lengthening pattern display
     */
    updateCurrentLengtheningPatternDisplay(pattern) {
        const display = document.getElementById('currentLengtheningPattern');
        if (display && pattern && pattern.steps) {
            const binary = PatternConverter.toBinary(pattern.steps, pattern.stepCount);
            const onsetCount = pattern.steps.filter(step => step).length;
            display.textContent = `${binary} (${onsetCount} onsets)`;
            display.classList.add('pattern-loaded');
        }
    }
    
    /**
     * Update extension count display
     */
    updateExtensionCountDisplay() {
        const display = document.getElementById('extensionCount');
        if (display) {
            display.textContent = this.progressiveLengtheningState.extensionCount.toString();
        }
    }
    
    /**
     * Update pattern length display
     */
    updatePatternLengthDisplay() {
        const display = document.getElementById('currentPatternLength');
        if (display && this.currentPattern) {
            display.textContent = this.currentPattern.stepCount.toString();
        }
    }
    
    /**
     * Generate expression for lengthening pattern
     */
    generateLengtheningExpression(steps) {
        try {
            const binary = PatternConverter.toBinary(steps, steps.length);
            return `b${binary}`;
        } catch (error) {
            console.warn('Error generating lengthening expression:', error);
            return '[Extended Pattern]';
        }
    }
    
    /**
     * Add lengthening pattern to database
     */
    addLengtheningPatternToDatabase() {
        if (!this.currentPattern || !this.currentPattern.isExtended) {
            showNotification('No extended pattern to add', 'warning');
            return;
        }
        
        try {
            // Create pattern data for database
            const patternData = {
                steps: this.currentPattern.steps,
                stepCount: this.currentPattern.stepCount,
                name: this.currentPattern.name,
                isExtended: true,
                originalPattern: this.currentPattern.originalPattern,
                extensionCount: this.currentPattern.extensionCount
            };
            
            // Generate analyses
            const analyses = this.generateComprehensiveAnalysis(patternData);
            
            // Create database pattern
            const dbPattern = createDatabasePattern(patternData, {
                perfectBalance: analyses.balanceAnalysis,
                syncopation: analyses.syncopationAnalysis
            });
            
            // Add extension-specific metadata
            dbPattern.isExtended = true;
            dbPattern.originalPattern = patternData.originalPattern;
            dbPattern.extensionCount = patternData.extensionCount;
            
            // Add to database
            const patternId = this.database.add(dbPattern);
            if (patternId) {
                showNotification('Extended pattern added to database!', 'success');
                this.updatePatternList();
                this.updateDatabaseStats();
            } else {
                showNotification('Pattern already exists in database', 'info');
            }
            
        } catch (error) {
            console.error('❌ Error adding extended pattern:', error);
            showNotification('Error adding extended pattern to database: ' + error.message, 'error');
        }
    }
    
    /**
     * Handle progressive transformer patterns from UPI notation
     */
    handleProgressiveTransformerPattern(pattern) {
        // console.log('🔄 Handling progressive transformer pattern:', pattern);
        
        // Set current pattern to base pattern for analysis
        this.currentPattern = pattern.basePattern;
        this.displayPatternAnalysis(this.currentPattern);
        this.showCompactOutput(this.currentPattern);
        this.updateButtonStates();
        
        // Generate progressive transformation sequence
        const transformations = this.generateUPIProgressiveSequence(pattern);
        
        if (transformations && transformations.length > 0) {
            // Set up simple stepping without displaying results list
            this.setupUPIProgressiveSteppingSimple(transformations, pattern);
            
            // Show notification about progressive mode
            const transformerName = pattern.transformerType === 'B' ? 'Barlow' :
                                   pattern.transformerType === 'W' ? 'Wolrab' :
                                   pattern.transformerType === 'E' ? 'Euclidean' : 'Dilcue';
            
            showNotification(`Progressive ${transformerName} mode active!\nPress Enter repeatedly to step through ${transformations.length} transformations.`, 'success');
        } else {
            showNotification('No progressive transformations could be generated', 'error');
        }
    }
    
    /**
     * Generate progressive transformation sequence for UPI notation
     */
    generateUPIProgressiveSequence(pattern) {
        // Check if required transformer classes are available
        if (typeof BarlowTransformer === 'undefined') {
            console.error('❌ BarlowTransformer class not available');
            return [];
        }
        if (typeof EuclideanTransformer === 'undefined') {
            console.error('❌ EuclideanTransformer class not available');
            return [];
        }
        
        const baseOnsetCount = pattern.basePattern.steps.filter(s => s).length;
        const targetOnsets = pattern.targetOnsets;
        
        if (baseOnsetCount === targetOnsets) {
            showNotification('Current and target onset counts are the same', 'info');
            return [];
        }
        
        const transformations = [];
        const direction = targetOnsets > baseOnsetCount ? 1 : -1;
        let currentPattern = [...pattern.basePattern.steps];
        
        // Generate each step in the sequence
        for (let onsets = baseOnsetCount + direction; 
             direction > 0 ? onsets <= targetOnsets : onsets >= targetOnsets; 
             onsets += direction) {
             
            let transformedPattern;
            
            switch (pattern.transformerType) {
                case 'B':
                    transformedPattern = this.generateBarlowTransformation(currentPattern, onsets, false);
                    break;
                case 'W':
                    transformedPattern = this.generateBarlowTransformation(currentPattern, onsets, true);
                    break;
                case 'E':
                    transformedPattern = this.generateEuclideanTransformation(currentPattern, onsets, false);
                    break;
                case 'D':
                    transformedPattern = this.generateEuclideanTransformation(currentPattern, onsets, true);
                    break;
                default:
                    console.error(`❌ Unknown transformer type: ${pattern.transformerType}`);
                    return [];
            }
            
            if (transformedPattern) {
                transformations.push({
                    pattern: transformedPattern,
                    onsetCount: onsets,
                    stepNumber: Math.abs(onsets - baseOnsetCount),
                    transformerType: pattern.transformerType
                });
                currentPattern = transformedPattern;
            } else {
                console.error(`❌ Transformation failed for ${onsets} onsets, stopping generation`);
                break;
            }
        }
        
        return transformations;
    }
    
    /**
     * Generate Barlow transformation for UPI progressive sequence
     */
    generateBarlowTransformation(currentSteps, targetOnsets, wolrabMode = false) {
        try {
            const currentOnsets = currentSteps.filter(s => s).length;
            
            if (currentOnsets === targetOnsets) {
                return currentSteps;
            }
            
            // Use existing BarlowTransformer
            const result = BarlowTransformer.transformPattern(
                currentSteps,
                targetOnsets,
                {
                    wolrabMode: wolrabMode,
                    preserveDownbeat: true,
                    minimumIndispensability: 0.0
                }
            );
            
            return result && result.pattern ? result.pattern : null;
            
        } catch (error) {
            console.error('❌ Error in Barlow transformation:', error);
            return null;
        }
    }
    
    /**
     * Generate Euclidean transformation for UPI progressive sequence
     */
    generateEuclideanTransformation(currentSteps, targetOnsets, antiMode = false) {
        try {
            const currentOnsets = currentSteps.filter(s => s).length;
            
            if (currentOnsets === targetOnsets) {
                return currentSteps;
            }
            
            // Use existing EuclideanTransformer
            const result = EuclideanTransformer.transform(
                currentSteps,
                targetOnsets,
                {
                    operation: targetOnsets > currentOnsets ? 'concentrate' : 'dilute',
                    mode: antiMode ? 'anti' : 'normal'
                }
            );
            
            return result && result.transformed ? result.transformed : null;
            
        } catch (error) {
            console.error('❌ Error in Euclidean transformation:', error);
            return null;
        }
    }
    
    
    /**
     * Set up simple Enter key handling for UPI progressive stepping
     */
    setupUPIProgressiveSteppingSimple(transformations, pattern) {
        // Initialize stepping state
        const upiInput = document.getElementById('universalInput');
        this.upiProgressiveState = {
            transformations: transformations,
            currentIndex: -1,
            pattern: pattern,
            originalUPIContent: upiInput ? upiInput.value.trim() : ''
        };
        
    }
    
    /**
     * Step through UPI progressive transformations (simple version)
     */
    stepThroughUPIProgressiveSimple() {
        if (!this.upiProgressiveState || !this.upiProgressiveState.transformations) {
            return;
        }
        
        const state = this.upiProgressiveState;
        state.currentIndex++;
        
        if (state.currentIndex >= state.transformations.length) {
            // Reached the end
            showNotification('Reached end of progressive sequence', 'info');
            return;
        }
        
        const transformation = state.transformations[state.currentIndex];
        
        // Update current pattern
        this.currentPattern = {
            steps: transformation.pattern,
            stepCount: transformation.pattern.length,
            name: `${state.pattern.formula} Step ${transformation.stepNumber}`,
            formula: state.pattern.formula,
            isUPIProgressive: true
        };
        
        // Update analysis and display WITHOUT updating UPI
        this.displayPatternAnalysis(this.currentPattern);
        this.showCompactOutput(this.currentPattern);
        this.updateButtonStates();
        
        // console.log(`✅ UPI Progressive Step ${transformation.stepNumber}: ${transformation.onsetCount} onsets`);
    }
    
    // ================================================
    // PATTERN CREATION MODULE
    // ================================================
    //
    // This module provides an intuitive interface for creating rhythm patterns
    // using four different algorithmic approaches:
    // 
    // 1. Euclidean: Even distribution of onsets (Godfried Toussaint algorithm)
    // 2. Dilcue: Anti-Euclidean using complement patterns for syncopation
    // 3. Barlow: Indispensability theory for intelligent onset placement
    // 4. Wolrab: Anti-Barlow with reversed indispensability for groove patterns
    // 
    // Features:
    // - Streamlined inline parameter controls (onsets, steps, offset, target)
    // - Auto-progressive mode when target is specified
    // - Enter key to generate/step through sequences
    // - Cmd/Ctrl+Enter to add patterns to database
    // - Real-time validation and state management
    
    /**
     * Initialize Pattern Creation Module
     * 
     * Sets up the streamlined interface for creating rhythmic patterns using
     * algorithmic approaches. Handles DOM element references, event listeners,
     * and initial state configuration.
     * 
     * Key Features:
     * - Radio button mode selection (Euclidean, Dilcue, Barlow, Wolrab)
     * - Inline parameter controls with real-time validation
     * - Auto-progressive mode activation when target is specified
     * - Unified keyboard interaction (Enter/Cmd+Enter)
     */
    initializeTransformerModule() {
        // Get transformer elements
        this.transformerElements = {
            modeRadios: document.querySelectorAll('input[name="transformerMode"]'),
            onsets: document.getElementById('transformerOnsets'),
            steps: document.getElementById('transformerSteps'),
            offset: document.getElementById('transformerOffset'),
            target: document.getElementById('transformerTarget'),
            // Random mode specific elements
            randomOnsets: document.getElementById('randomOnsets'),
            randomSteps: document.getElementById('randomSteps'),
            randomDistribution: document.getElementById('randomDistribution'),
            randomIntensity: document.getElementById('randomIntensity'),
            randomIntensityValue: document.getElementById('randomIntensityValue'),
            quickInput: document.getElementById('transformerQuickInput'),
            addBtn: document.getElementById('transformerAddBtn'),
            progress: document.getElementById('transformerProgress'),
            progressText: document.getElementById('transformerProgressText'),
            progressBar: document.getElementById('transformerProgressBar'),
            module: document.querySelector('.transformer-input-module')
        };
        
        // Initialize transformer state
        this.transformerState = {
            isActive: false,
            transformations: [],
            currentIndex: -1,
            basePattern: null,
            mode: 'euclidean'
        };
        
        // Set up event listeners
        this.setupTransformerEventListeners();
        
        // Update mode styling
        this.updateTransformerModeDisplay();
    }
    
    /**
     * Set up transformer event listeners
     */
    setupTransformerEventListeners() {
        const elements = this.transformerElements;
        
        // Mode change (radio buttons)
        elements.modeRadios.forEach(radio => {
            radio.addEventListener('change', () => {
                if (radio.checked) {
                    this.transformerState.mode = radio.value;
                    this.updateTransformerModeDisplay();
                    this.resetTransformerState();
                }
            });
        });
        
        // Input validation for standard parameters
        [elements.onsets, elements.steps, elements.offset, elements.target].forEach(input => {
            input.addEventListener('input', () => {
                this.resetTransformerState();
                this.validateTransformerInputs();
                this.updateTransformerButtonStates();
                this.updateProgressiveMode();
            });
            input.addEventListener('change', () => {
                this.resetTransformerState();
                this.validateTransformerInputs();
                this.updateTransformerButtonStates();
                this.updateProgressiveMode();
            });
        });
        
        // Input validation for random parameters
        [elements.randomOnsets, elements.randomSteps, elements.randomDistribution, elements.randomIntensity].forEach(input => {
            if (input) {
                input.addEventListener('input', () => {
                    this.resetTransformerState();
                    this.validateTransformerInputs();
                    this.updateTransformerButtonStates();
                    this.updateRandomIntensityDisplay();
                });
                input.addEventListener('change', () => {
                    this.resetTransformerState();
                    this.validateTransformerInputs();
                    this.updateTransformerButtonStates();
                    this.updateRandomIntensityDisplay();
                });
            }
        });
        
        // Quick input field with Enter key support
        elements.quickInput.addEventListener('keydown', (event) => {
            if (event.key === 'Enter') {
                event.preventDefault();
                if (event.metaKey || event.ctrlKey) {
                    // Cmd/Ctrl+Enter: Add to database
                    this.addTransformerPatternToDatabase();
                } else {
                    // Enter: Generate/Step
                    this.handleTransformerGenerate();
                }
            }
        });
        
        // Button actions
        elements.addBtn.addEventListener('click', () => this.addTransformerPatternToDatabase());
        
        // Global keyboard events
        document.addEventListener('keydown', (event) => this.handleTransformerKeyboard(event));
    }
    
    /**
     * Update transformer mode display styling
     */
    updateTransformerModeDisplay() {
        const mode = this.transformerState.mode;
        this.transformerElements.module.setAttribute('data-mode', mode);
    }
    
    /**
     * Validate transformer inputs
     */
    validateTransformerInputs() {
        const elements = this.transformerElements;
        const mode = this.transformerState.mode;
        
        if (mode === 'random') {
            // Validate random mode parameters
            const randomOnsetsValue = elements.randomOnsets.value.trim().toLowerCase();
            const randomSteps = parseInt(elements.randomSteps.value) || 1;
            
            let isValidOnsets = false;
            if (randomOnsetsValue === 'r') {
                isValidOnsets = true; // 'r' is always valid
            } else {
                const randomOnsets = parseInt(randomOnsetsValue) || 0;
                isValidOnsets = randomOnsets >= 0 && randomOnsets <= randomSteps;
            }
            
            const isValid = 
                randomSteps >= 1 && randomSteps <= 32 && isValidOnsets;
            
            return isValid;
        } else {
            // Validate standard mode parameters
            const onsets = parseInt(elements.onsets.value) || 0;
            const steps = parseInt(elements.steps.value) || 1;
            const offset = parseInt(elements.offset.value) || 0;
            const target = parseInt(elements.target.value) || null;
            
            const isValid = 
                steps >= 1 && steps <= 32 &&
                onsets >= 0 && onsets <= steps &&
                offset >= 0 && offset < steps &&
                (!target || (target >= 0 && target <= steps));
            
            // Update constraints dynamically
            elements.onsets.max = steps;
            elements.offset.max = steps - 1;
            if (elements.target.value) {
                elements.target.max = steps;
            }
            
            return isValid;
        }
    }
    
    /**
     * Update transformer button states
     */
    updateTransformerButtonStates() {
        const elements = this.transformerElements;
        const isValid = this.validateTransformerInputs();
        const hasPattern = !!this.currentPattern;
        
        // Enable add button if we have a current pattern
        elements.addBtn.disabled = !hasPattern;
    }
    
    /**
     * Update Progressive Mode Automatically
     * 
     * Automatically enables progressive mode when a target onset count is specified.
     * This provides intuitive behavior - when users fill in a target, they expect
     * to step through a sequence from current to target onset count.
     * 
     * Progressive mode allows stepping through transformations with Enter key,
     * creating smooth transitions between onset densities.
     */
    updateProgressiveMode() {
        const elements = this.transformerElements;
        const hasTarget = elements.target.value && parseInt(elements.target.value) > 0;
        
        // Auto-enable progressive mode when target is filled
        this.transformerState.isProgressive = hasTarget;
    }
    
    /**
     * Update random intensity display value
     */
    updateRandomIntensityDisplay() {
        const elements = this.transformerElements;
        if (elements.randomIntensity && elements.randomIntensityValue) {
            elements.randomIntensityValue.textContent = elements.randomIntensity.value + '%';
        }
    }
    
    /**
     * Handle transformer generate/step action
     */
    handleTransformerGenerate() {
        if (this.transformerState.isActive && this.transformerState.transformations.length > 0) {
            // If we're in progressive mode, step to next
            this.stepTransformerPattern();
        } else {
            // Generate new pattern
            this.generateTransformerPattern();
        }
    }
    
    /**
     * Generate transformer pattern
     */
    generateTransformerPattern() {
        try {
            const elements = this.transformerElements;
            const mode = this.transformerState.mode;
            
            if (mode === 'random') {
                // Handle random pattern generation
                return this.generateRandomPattern();
            }
            
            // Handle standard pattern generation
            const onsets = parseInt(elements.onsets.value) || 0;
            const steps = parseInt(elements.steps.value) || 1;
            const offset = parseInt(elements.offset.value) || 0;
            const target = parseInt(elements.target.value) || null;
            const isProgressive = this.transformerState.isProgressive;
            
            if (!this.validateTransformerInputs()) {
                showNotification('Please check your input values', 'warning');
                return;
            }
            
            // Generate base pattern based on mode
            let basePattern;
            switch (mode) {
                case 'euclidean':
                case 'dilcue':
                    basePattern = this.generateEuclideanBasePattern(onsets, steps, offset, mode === 'dilcue');
                    break;
                case 'barlow':
                case 'wolrab':
                    basePattern = this.generateBarlowBasePattern(onsets, steps, offset);
                    break;
            }
            
            if (!basePattern) {
                showNotification('Failed to generate base pattern', 'error');
                return;
            }
            
            // Set as current pattern
            this.currentPattern = basePattern;
            this.transformerState.basePattern = basePattern;
            
            // Generate progressive transformations if requested
            if (isProgressive && target !== null && target !== onsets) {
                this.generateTransformerSequence(basePattern, target, mode);
                this.transformerState.isActive = true;
                this.updateTransformerProgress();
                showNotification(`Progressive ${mode} mode active! Use Step Through or Enter to continue.`, 'success');
            } else {
                this.transformerState.isActive = false;
                this.resetTransformerState();
                showNotification(`${mode.charAt(0).toUpperCase() + mode.slice(1)} pattern generated!`, 'success');
            }
            
            // Update display
            this.displayPatternAnalysis(this.currentPattern);
            this.showCompactOutput(this.currentPattern);
            this.updateButtonStates();
            this.updateTransformerButtonStates();
            
        } catch (error) {
            console.error('Error generating transformer pattern:', error);
            showNotification('Error generating pattern: ' + error.message, 'error');
        }
    }
    
    /**
     * Generate Random Pattern
     * 
     * Creates random patterns using various distribution strategies.
     * Each generation produces a different pattern, perfect for creative exploration.
     * 
     * Distribution Types:
     * - Uniform: Equal probability for all positions
     * - Bell Curve: Favors middle positions (musically balanced)
     * - Syncopated: Emphasizes off-beat positions
     * - Balanced: Even spacing with controlled randomness
     * - Clustered: Groups onsets together with varying density
     */
    generateRandomPattern() {
        try {
            const elements = this.transformerElements;
            const onsetsValue = elements.randomOnsets.value.trim().toLowerCase();
            const steps = parseInt(elements.randomSteps.value) || 1;
            const distribution = elements.randomDistribution.value || 'bell';
            const intensity = parseInt(elements.randomIntensity.value) || 50;
            
            // Handle random onset count
            let onsets;
            if (onsetsValue === 'r') {
                // Generate random onset count using bell curve distribution
                onsets = this.generateRandomOnsetCount(steps);
            } else {
                onsets = parseInt(onsetsValue) || 0;
            }
            
            if (!this.validateTransformerInputs()) {
                showNotification('Please check your random parameters', 'warning');
                return;
            }
            
            // Generate random pattern based on distribution
            const randomSteps = this.createRandomDistribution(onsets, steps, distribution, intensity);
            
            const pattern = {
                steps: randomSteps,
                stepCount: steps,
                name: `Random(${onsetsValue === 'r' ? 'r→' + onsets : onsets},${steps},${distribution})`,
                formula: `Random(${onsetsValue === 'r' ? 'r→' + onsets : onsets},${steps},${distribution},${intensity}%)`,
                isRandom: true,
                distribution: distribution,
                intensity: intensity,
                randomOnsets: onsetsValue === 'r'
            };
            
            // Set as current pattern
            this.currentPattern = pattern;
            
            // Update display
            this.displayPatternAnalysis(pattern);
            this.updateButtonStates();
            this.updateTransformerButtonStates();
            
            showNotification(`Random ${distribution} pattern generated! Press Enter for another variation.`, 'success');
            
        } catch (error) {
            console.error('Error generating random pattern:', error);
            showNotification('Error generating random pattern: ' + error.message, 'error');
        }
    }
    
    /**
     * Generate Random Onset Count
     * 
     * Creates a random onset count using bell curve distribution to favor
     * musically interesting densities (avoiding extremes of 0 or maximum).
     */
    generateRandomOnsetCount(steps) {
        // Use bell curve distribution favoring middle densities
        const center = steps / 2;
        const spread = steps / 4;
        
        // Generate random value using Box-Muller transform for Gaussian distribution
        const u1 = Math.random();
        const u2 = Math.random();
        const z = Math.sqrt(-2 * Math.log(u1)) * Math.cos(2 * Math.PI * u2);
        
        // Scale and shift to desired range
        let randomOnsets = Math.round(center + spread * z);
        
        // Clamp to valid range (1 to steps-1 to avoid empty or full patterns)
        randomOnsets = Math.max(1, Math.min(steps - 1, randomOnsets));
        
        return randomOnsets;
    }
    
    /**
     * Create Random Distribution
     * 
     * Generates onset patterns using different probability distributions
     * to create musically interesting and varied rhythmic structures.
     */
    createRandomDistribution(onsets, steps, distribution, intensity) {
        const positions = Array(steps).fill(false);
        
        // Convert intensity (0-100) to probability modifier (0.1-2.0)
        const intensityFactor = 0.1 + (intensity / 100) * 1.9;
        
        switch (distribution) {
            case 'uniform':
                return this.createUniformRandom(onsets, steps);
                
            case 'bell':
                return this.createBellCurveRandom(onsets, steps, intensityFactor);
                
            case 'syncopated':
                return this.createSyncopatedRandom(onsets, steps, intensityFactor);
                
            case 'balanced':
                return this.createBalancedRandom(onsets, steps, intensityFactor);
                
            case 'clustered':
                return this.createClusteredRandom(onsets, steps, intensityFactor);
                
            default:
                return this.createBellCurveRandom(onsets, steps, intensityFactor);
        }
    }
    
    /**
     * Create uniform random distribution
     */
    createUniformRandom(onsets, steps) {
        const positions = Array(steps).fill(false);
        const availablePositions = [...Array(steps).keys()];
        
        for (let i = 0; i < onsets && availablePositions.length > 0; i++) {
            const randomIndex = Math.floor(Math.random() * availablePositions.length);
            const position = availablePositions.splice(randomIndex, 1)[0];
            positions[position] = true;
        }
        
        return positions;
    }
    
    /**
     * Create bell curve distribution (favors middle positions)
     */
    createBellCurveRandom(onsets, steps, intensityFactor) {
        const positions = Array(steps).fill(false);
        const center = (steps - 1) / 2;
        
        // Create weighted probabilities using bell curve
        const weights = [];
        for (let i = 0; i < steps; i++) {
            const distance = Math.abs(i - center) / (steps / 2);
            const weight = Math.exp(-Math.pow(distance * intensityFactor, 2));
            weights.push(weight);
        }
        
        return this.selectWeightedPositions(positions, weights, onsets);
    }
    
    /**
     * Create syncopated distribution (emphasizes off-beats)
     */
    createSyncopatedRandom(onsets, steps, intensityFactor) {
        const positions = Array(steps).fill(false);
        const weights = [];
        
        for (let i = 0; i < steps; i++) {
            // Favor off-beat positions (odd indices, positions after strong beats)
            const isOffBeat = (i % 2 === 1) || (i % 4 === 2) || (i % 4 === 3);
            const weight = isOffBeat ? intensityFactor : (2 - intensityFactor);
            weights.push(Math.max(0.1, weight));
        }
        
        return this.selectWeightedPositions(positions, weights, onsets);
    }
    
    /**
     * Create balanced distribution (even spacing with controlled randomness)
     */
    createBalancedRandom(onsets, steps, intensityFactor) {
        const positions = Array(steps).fill(false);
        
        if (onsets === 0) return positions;
        
        // Start with even spacing
        const baseSpacing = steps / onsets;
        const weights = Array(steps).fill(0.1);
        
        // Add probability peaks around ideal positions
        for (let i = 0; i < onsets; i++) {
            const idealPos = Math.round(i * baseSpacing);
            const spread = Math.max(1, Math.round(baseSpacing * (2 - intensityFactor) / 2));
            
            for (let j = Math.max(0, idealPos - spread); j <= Math.min(steps - 1, idealPos + spread); j++) {
                const distance = Math.abs(j - idealPos);
                weights[j] += intensityFactor * Math.exp(-distance / spread);
            }
        }
        
        return this.selectWeightedPositions(positions, weights, onsets);
    }
    
    /**
     * Create clustered distribution (groups onsets together)
     */
    createClusteredRandom(onsets, steps, intensityFactor) {
        const positions = Array(steps).fill(false);
        
        if (onsets === 0) return positions;
        
        // Determine number of clusters (1-3 based on intensity)
        const numClusters = Math.max(1, Math.min(3, Math.round(onsets / (4 - intensityFactor))));
        const onsetsPerCluster = Math.ceil(onsets / numClusters);
        
        let remainingOnsets = onsets;
        
        for (let cluster = 0; cluster < numClusters && remainingOnsets > 0; cluster++) {
            // Pick random center for this cluster
            const clusterCenter = Math.floor(Math.random() * steps);
            const clusterSize = Math.min(remainingOnsets, onsetsPerCluster);
            const clusterSpread = Math.max(1, Math.round(steps / (8 * intensityFactor)));
            
            // Place onsets around cluster center
            for (let i = 0; i < clusterSize; i++) {
                let attempts = 0;
                let placed = false;
                
                while (!placed && attempts < 50) {
                    const offset = Math.round((Math.random() - 0.5) * clusterSpread * 2);
                    const pos = (clusterCenter + offset + steps) % steps;
                    
                    if (!positions[pos]) {
                        positions[pos] = true;
                        placed = true;
                        remainingOnsets--;
                    }
                    attempts++;
                }
            }
        }
        
        return positions;
    }
    
    /**
     * Select positions based on weighted probabilities
     */
    selectWeightedPositions(positions, weights, onsets) {
        const available = weights.map((weight, index) => ({ index, weight }));
        
        for (let i = 0; i < onsets && available.length > 0; i++) {
            // Calculate total weight
            const totalWeight = available.reduce((sum, item) => sum + item.weight, 0);
            
            // Pick random position based on weights
            let random = Math.random() * totalWeight;
            let selectedIndex = 0;
            
            for (let j = 0; j < available.length; j++) {
                random -= available[j].weight;
                if (random <= 0) {
                    selectedIndex = j;
                    break;
                }
            }
            
            // Place onset and remove from available positions
            const selectedItem = available.splice(selectedIndex, 1)[0];
            positions[selectedItem.index] = true;
        }
        
        return positions;
    }
    
    /**
     * Generate Euclidean base pattern
     */
    generateEuclideanBasePattern(onsets, steps, offset, antiMode = false) {
        try {
            let euclideanSteps;
            if (antiMode) {
                // Dilcue: use complement
                const normalPattern = EuclideanGenerator.generate(steps - onsets, steps, 0);
                euclideanSteps = normalPattern.map(step => !step);
            } else {
                euclideanSteps = EuclideanGenerator.generate(onsets, steps, offset);
            }
            
            return {
                steps: euclideanSteps,
                stepCount: steps,
                name: `${antiMode ? 'Dilcue' : 'Euclidean'}(${onsets},${steps}${offset ? `,${offset}` : ''})`,
                formula: `${antiMode ? 'Dilcue' : 'Euclidean'}(${onsets},${steps}${offset ? `,${offset}` : ''})`,
                isEuclidean: true,
                isDilcue: antiMode,
                beats: onsets,
                offset: offset
            };
        } catch (error) {
            console.error('Error generating Euclidean pattern:', error);
            return null;
        }
    }
    
    /**
     * Generate Barlow base pattern (starts with Euclidean, then applies Barlow logic)
     */
    generateBarlowBasePattern(onsets, steps, offset) {
        try {
            // Start with Euclidean distribution
            const euclideanSteps = EuclideanGenerator.generate(onsets, steps, offset);
            
            return {
                steps: euclideanSteps,
                stepCount: steps,
                name: `Barlow(${onsets},${steps}${offset ? `,${offset}` : ''})`,
                formula: `Barlow(${onsets},${steps}${offset ? `,${offset}` : ''})`,
                isBarlow: true,
                beats: onsets,
                offset: offset
            };
        } catch (error) {
            console.error('Error generating Barlow pattern:', error);
            return null;
        }
    }
    
    /**
     * Generate transformer sequence for progressive mode
     */
    generateTransformerSequence(basePattern, targetOnsets, mode) {
        const transformations = [];
        const baseOnsets = basePattern.steps.filter(s => s).length;
        const direction = targetOnsets > baseOnsets ? 1 : -1;
        let currentPattern = [...basePattern.steps];
        
        for (let onsets = baseOnsets + direction; 
             direction > 0 ? onsets <= targetOnsets : onsets >= targetOnsets; 
             onsets += direction) {
             
            let transformedPattern;
            
            switch (mode) {
                case 'euclidean':
                    transformedPattern = this.generateEuclideanTransformation(currentPattern, onsets, false);
                    break;
                case 'dilcue':
                    transformedPattern = this.generateEuclideanTransformation(currentPattern, onsets, true);
                    break;
                case 'barlow':
                    transformedPattern = this.generateBarlowTransformation(currentPattern, onsets, false);
                    break;
                case 'wolrab':
                    transformedPattern = this.generateBarlowTransformation(currentPattern, onsets, true);
                    break;
            }
            
            if (transformedPattern) {
                transformations.push({
                    pattern: transformedPattern,
                    onsetCount: onsets,
                    stepNumber: Math.abs(onsets - baseOnsets),
                    mode: mode
                });
                currentPattern = transformedPattern;
            }
        }
        
        this.transformerState.transformations = transformations;
        this.transformerState.currentIndex = -1;
    }
    
    /**
     * Step through transformer pattern
     */
    stepTransformerPattern() {
        if (!this.transformerState.isActive || !this.transformerState.transformations.length) {
            return;
        }
        
        this.transformerState.currentIndex++;
        
        if (this.transformerState.currentIndex >= this.transformerState.transformations.length) {
            showNotification('Reached end of transformation sequence', 'info');
            return;
        }
        
        const transformation = this.transformerState.transformations[this.transformerState.currentIndex];
        const mode = this.transformerState.mode;
        
        // Update current pattern
        this.currentPattern = {
            steps: transformation.pattern,
            stepCount: transformation.pattern.length,
            name: `${mode.charAt(0).toUpperCase() + mode.slice(1)} Step ${transformation.stepNumber}`,
            formula: `${mode.charAt(0).toUpperCase() + mode.slice(1)} Step ${transformation.stepNumber}`,
            isTransformerGenerated: true,
            transformerMode: mode
        };
        
        // Update display
        this.displayPatternAnalysis(this.currentPattern);
        this.showCompactOutput(this.currentPattern);
        this.updateButtonStates();
        this.updateTransformerButtonStates();
        this.updateTransformerProgress();
        
        showNotification(`Step ${transformation.stepNumber}: ${transformation.onsetCount} onsets`, 'info');
    }
    
    /**
     * Add transformer pattern to database
     */
    addTransformerPatternToDatabase() {
        if (!this.currentPattern) {
            showNotification('No pattern to add to database', 'warning');
            return;
        }
        
        try {
            // Create descriptive name
            const mode = this.transformerState.mode;
            const onsetCount = this.currentPattern.steps.filter(s => s).length;
            const stepCount = this.currentPattern.stepCount;
            
            let descriptiveName = `${mode.charAt(0).toUpperCase() + mode.slice(1)}-Generated(${onsetCount},${stepCount})`;
            
            // Create pattern object for database
            const patternData = {
                steps: this.currentPattern.steps,
                stepCount: this.currentPattern.stepCount,
                name: descriptiveName,
                isTransformerGenerated: true,
                transformerMode: mode,
                generatedBy: 'transformer-module'
            };
            
            // Generate analyses for the new pattern
            const analyses = this.generateComprehensiveAnalysis(patternData);
            
            // Create database pattern
            const dbPattern = createDatabasePattern(patternData, analyses);
            
            // Add to database
            const patternId = this.database.add(dbPattern);
            
            if (patternId) {
                showNotification(`${mode.charAt(0).toUpperCase() + mode.slice(1)} pattern added to database!`, 'success');
                this.updatePatternList();
                this.updateDatabaseStats();
            } else {
                showNotification('Pattern already exists in database', 'info');
            }
            
        } catch (error) {
            console.error('Error adding transformer pattern to database:', error);
            showNotification('Error adding pattern to database: ' + error.message, 'error');
        }
    }
    
    /**
     * Reset transformer module
     */
    resetTransformerModule() {
        // Reset form values
        this.transformerElements.onsets.value = '3';
        this.transformerElements.steps.value = '8';
        this.transformerElements.offset.value = '0';
        this.transformerElements.target.value = '';
        this.transformerElements.progressive.checked = false;
        this.transformerElements.mode.value = 'euclidean';
        
        // Reset state
        this.resetTransformerState();
        this.transformerState.mode = 'euclidean';
        
        // Update display
        this.updateTransformerModeDisplay();
        this.updateTransformerButtonStates();
        
        showNotification('Transformer module reset', 'info');
    }
    
    /**
     * Reset transformer state
     */
    resetTransformerState() {
        this.transformerState.isActive = false;
        this.transformerState.transformations = [];
        this.transformerState.currentIndex = -1;
        this.transformerState.basePattern = null;
        
        // Hide progress
        this.transformerElements.progress.style.display = 'none';
        
        this.updateTransformerButtonStates();
    }
    
    /**
     * Update transformer progress display
     */
    updateTransformerProgress() {
        const elements = this.transformerElements;
        const state = this.transformerState;
        
        if (!state.isActive || !state.transformations.length) {
            elements.progress.style.display = 'none';
            return;
        }
        
        const total = state.transformations.length;
        const current = Math.max(0, state.currentIndex);
        const progress = total > 0 ? (current / total) * 100 : 0;
        
        const currentTransformation = state.transformations[current];
        const progressText = currentTransformation 
            ? `Step ${current + 1} of ${total}: ${currentTransformation.onsetCount} onsets`
            : `Ready to step through ${total} transformations`;
        
        elements.progressText.textContent = progressText;
        elements.progressBar.style.width = `${progress}%`;
        elements.progress.style.display = 'block';
    }
    
    /**
     * Handle transformer keyboard events
     */
    handleTransformerKeyboard(event) {
        // Only handle if transformer module is focused or has focus within
        const isTransformerFocused = this.transformerElements.module.contains(document.activeElement);
        
        if (!isTransformerFocused) return;
        
        if (event.key === 'Enter') {
            event.preventDefault();
            
            if (event.metaKey || event.ctrlKey) {
                // Cmd/Ctrl+Enter: Add to database
                this.addTransformerPatternToDatabase();
            } else {
                // Enter: Generate or step
                if (this.transformerState.isActive && this.transformerState.transformations.length > 0) {
                    this.stepTransformerPattern();
                } else {
                    this.generateTransformerPattern();
                }
            }
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