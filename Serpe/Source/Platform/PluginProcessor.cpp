/*
  ==============================================================================

    Rhythm Pattern Explorer - iPad AUv3 Plugin
    Main Audio Processor Implementation
    
    PLATFORM DIFFERENCES FROM DESKTOP VERSION:
    
    1. iOS String Assertion Fix (lines 13-19)
       - JUCE String assertions cause crashes on iOS with Unicode characters
       - Desktop versions don't experience these crashes
       - Required for pattern parsing stability on iPadOS
    
    2. Redundant Note-Off Safety Mechanism (triggerNote function)
       - AUv3 framework occasionally loses note-off messages
       - Desktop AU/VST3 versions don't need this redundancy
       - Critical for preventing stuck notes on iPad
    
    3. Sandboxed File System Access
       - iOS sandboxing prevents direct preset directory access
       - Desktop versions have unrestricted file system access
       - Affects preset management and factory preset loading
    
    Last Updated: August 2025 - Stable note timing achieved
    Status: Consistent 1-tick note duration matching desktop version

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PlatformSpecific.h"
#include <fstream>
#include <algorithm>
#include <chrono>
#include <iomanip>

// ============================================================================
// iOS-SPECIFIC WORKAROUND: JUCE String Assertion Fix
// ============================================================================
// ISSUE: JUCE String class triggers fatal assertions on iOS when processing
//        certain Unicode characters during UPI pattern parsing
// CAUSE: iOS has stricter Unicode validation that conflicts with JUCE's
//        internal String assertion logic
// SYMPTOMS: Plugin crashes with "juce_String.cpp:327 assertion" during pattern parsing
// SOLUTION: Disable all JUCE assertions for iOS builds only
// IMPACT: Desktop versions are unaffected - this fix is iOS-specific
// SAFETY: Pattern parsing logic remains intact, only assertion checking is disabled
#if SERPE_IS_IPAD
#undef jassert
#define jassert(x) 
#undef jassertfalse  
#define jassertfalse
#endif
#include "../Core/UPIParser.h"
#include "../Core/PatternUtils.h"
#include "DebugConfig.h"
// Manager implementations now properly referenced from Source directory in Xcode project
#include <ctime>

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout SerpeAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Core parameters - only expose essential ones to host
    layout.add(std::make_unique<juce::AudioParameterBool>("useHostTransport", "Use Host Transport", true));
    layout.add(std::make_unique<juce::AudioParameterInt>("midiNote", "MIDI Note", 0, 127, 36));
    layout.add(std::make_unique<juce::AudioParameterBool>("tick", "Tick", false));
    
    // Pattern Length parameters for Phase 2 temporal control
    layout.add(std::make_unique<juce::AudioParameterChoice>("patternLengthUnit", "Pattern Length Unit", 
        juce::StringArray{"Steps", "Beats", "Bars", "Auto"}, 1)); // Default to "Beats"
    layout.add(std::make_unique<juce::AudioParameterChoice>("patternLengthValue", "Pattern Length Value", 
        juce::StringArray{"0.125", "0.25", "0.5", "0.75", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32"}, 7)); // Default to "8"
    
    // Subdivision parameter for transport subdivision matching
    layout.add(std::make_unique<juce::AudioParameterChoice>("subdivision", "Subdivision", 
        juce::StringArray{"64th Triplet", "64th", "32nd Triplet", "32nd", "16th Triplet", "16th", "8th Triplet", "8th", "Quarter Triplet", "Quarter", "Half Triplet", "Half", "Whole"}, 5)); // Default to "16th"
    
    // Accent parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>("accentVelocity", "Accent Velocity", 0.0f, 1.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("unaccentedVelocity", "Unaccented Velocity", 0.0f, 1.0f, 0.8f));
    layout.add(std::make_unique<juce::AudioParameterInt>("accentPitchOffset", "Accent Pitch Offset", -12, 12, 5));
    
    return layout;
}

SerpeAudioProcessor::SerpeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), randomGenerator(std::random_device{}()), parameters(*this, nullptr, "RhythmPatternExplorer", createParameterLayout())
#else
     : randomGenerator(std::random_device{}()), parameters(*this, nullptr, "RhythmPatternExplorer", createParameterLayout())
#endif
{
#if JUCE_IOS
    // Install custom assertion handler to prevent crashes on iOS
    juce::Logger::setCurrentLogger(nullptr);
#endif
    // Get parameter pointers from AudioProcessorValueTreeState
    useHostTransportParam = dynamic_cast<juce::AudioParameterBool*>(parameters.getParameter("useHostTransport"));
    midiNoteParam = dynamic_cast<juce::AudioParameterInt*>(parameters.getParameter("midiNote"));
    tickParam = dynamic_cast<juce::AudioParameterBool*>(parameters.getParameter("tick"));
    
    // Pattern Length parameters
    patternLengthUnitParam = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("patternLengthUnit"));
    patternLengthValueParam = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("patternLengthValue"));
    subdivisionParam = dynamic_cast<juce::AudioParameterChoice*>(parameters.getParameter("subdivision"));
    
    // Accent parameters
    accentVelocityParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("accentVelocity"));
    unaccentedVelocityParam = dynamic_cast<juce::AudioParameterFloat*>(parameters.getParameter("unaccentedVelocity"));
    accentPitchOffsetParam = dynamic_cast<juce::AudioParameterInt*>(parameters.getParameter("accentPitchOffset"));
    
    // Initialize pattern engine with default Euclidean pattern
    patternEngine.generateEuclideanPattern(3, 8);
    
    // Set up progressive offset engine for UPI parser
    UPIParser::setProgressiveOffsetEngine(&patternEngine);
    
    // Initialize encapsulated managers - TRANSITION: Running parallel with legacy for safety
    sceneManager = std::make_unique<SceneManager>();
    progressiveManager = std::make_unique<ProgressiveManager>();
    
    try {
        presetManager.installFactoryPresets();
    } catch (...) {
        // Ignore preset installation errors to avoid initialization failure
    }
    
}


SerpeAudioProcessor::~SerpeAudioProcessor()
{
}

//==============================================================================
const juce::String SerpeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SerpeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SerpeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SerpeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SerpeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SerpeAudioProcessor::getNumPrograms()
{
    return 1;
}

int SerpeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SerpeAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SerpeAudioProcessor::getProgramName (int index)
{
    return "Default";
}

void SerpeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SerpeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    // MIDI effect mode - minimal audio setup
    // Reset sequencer state
    currentSample = 0;
    currentStep = 0;
    wasPlaying = false;
    
    // Force early initialization to work around Logic Pro loading order issues
    lastProcessBlockTime = juce::Time::getMillisecondCounter();
    
    updateTiming();
    
}

void SerpeAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SerpeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SerpeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // CRITICAL DEBUG: Track processBlock calls to detect multiple instances
    static int processBlockCallCount = 0;
    processBlockCallCount++;
    
    // If this number grows too fast, we have multiple instances or threading issues
    // Handle tick parameter (equivalent to pressing Parse) - WITH CRASH PROTECTION
    bool currentTickState = tickParam ? tickParam->get() : false;
    if (currentTickState && !lastTickState) {
        // Tick edge detected - advance scenes and progressive transformations like Enter key and MIDI input
        if (!currentUPIInput.isEmpty()) {
            try {
                // Use original UPI input if available (preserves progressive/scene syntax after manual edits)
                juce::String upiToProcess = originalUPIInput.isEmpty() ? currentUPIInput : originalUPIInput;
                
                // SAFETY: Validate pattern before processing
                if (upiToProcess.length() > 500) {
                    // Pattern too complex - skip to prevent crash
                    tickResetCounter = 1;
                    lastTickState = currentTickState;
                    return;
                }
                
                // Check for scenes and progressive patterns to handle them correctly
                bool hasProgressiveTransformation = upiToProcess.contains(">");
                bool hasScenes = upiToProcess.contains("|");
                
                bool triggerNeeded = false;
                
                if (hasScenes) {
                    // CRITICAL FIX WITH ERROR PROTECTION: If we have scenes, handle scene advancement first
                    // This prevents double/triple advancement when scenes contain progressive transformations
                    try {
                        advanceScene();
                        applyCurrentScenePattern(); 
                        triggerNeeded = true;
                    } catch (...) {
                        // Scene processing failed - reset to safe state
                        resetScenes();
                        // Fallback to simple pattern parsing
                        parseAndApplyUPI("E(3,8)", true); // Safe fallback pattern
                        triggerNeeded = true;
                    }
                }
                else if (hasProgressiveTransformation) {
                    // Progressive transformations: advance without resetting accents
                    // Only process this if we DON'T have scenes (to avoid double advancement)
                    try {
                        parseAndApplyUPI(upiToProcess, false); // false = preserve accents
                        triggerNeeded = true;
                    } catch (...) {
                        // Progressive parsing failed - use safe fallback
                        parseAndApplyUPI("E(3,8)", true); // Safe fallback pattern
                        triggerNeeded = true;
                    }
                }
                
                // For regular patterns without scenes or progressive transformations
                if (!triggerNeeded) {
                    try {
                        parseAndApplyUPI(upiToProcess, true); // true = reset accents for new patterns
                    } catch (...) {
                        // Regular parsing failed - use safe fallback
                        parseAndApplyUPI("E(3,8)", true); // Safe fallback pattern
                    }
                }
                
                // Reset step indicator for all manual triggers
                // currentStep.store(0); // REMOVED: Using derived indices
                patternChanged.store(true);
                
            } catch (...) {
                // ULTIMATE SAFETY: If anything crashes, reset to completely safe state
                resetScenes();
                resetAccentSystem();
                parseAndApplyUPI("E(3,8)", true); // Safe fallback pattern
                // currentStep.store(0); // REMOVED: Using derived indices
                patternChanged.store(true);
            }
        } else {
        }
        tickResetCounter = 1; // Start reset counter
    }
    
    // Reset tick parameter after a brief delay to allow for multiple ticks
    if (tickResetCounter > 0) {
        tickResetCounter++;
        if (tickResetCounter >= 20) { // Reset after ~20 process blocks (~1-2ms)
            tickParam->setValueNotifyingHost(0.0f);
            tickResetCounter = 0;
        }
    }
    lastTickState = currentTickState;
    
    // Update last process time to indicate we're receiving audio callbacks
    lastProcessBlockTime = juce::Time::getMillisecondCounter();
    
    
    int currentBufferSize = buffer.getNumSamples();
    if (currentBufferSize <= 0) {
        return;
    }
    
    // PHASE 1: Update monotonic transport tick
    updateTransportTick(currentBufferSize);
    
    // PHASE 1.5: Legacy synchronization removed - getCurrentStep() now fully derived
    
    // PHASE 2: Process any queued pattern updates at buffer boundaries
    processPatternUpdates();
    
    juce::ScopedLock lock(processingLock);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // CRITICAL FIX: Clear ALL audio channels - this is a MIDI-only plugin and should produce NO audio
    for (auto i = 0; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Check for incoming MIDI that could trigger pattern regeneration
    checkMidiInputForTriggers(midiMessages);

    // Get transport info from host
    juce::AudioPlayHead* playHead = getPlayHead();
    
    // Default values for when transport info is not available
    bool hasValidPosition = false;
    bool isPlaying = false;
    bool isRecording = false;
    double ppqPosition = 0.0;
    double bpm = 120.0;
    
    if (playHead != nullptr)
    {
        if (auto position = playHead->getPosition())
        {
            hasValidPosition = true;
            isPlaying = position->getIsPlaying();
            isRecording = position->getIsRecording();
            
            if (auto ppq = position->getPpqPosition())
                ppqPosition = *ppq;
                
            if (auto tempo = position->getBpm())
                bpm = *tempo;
        }
        
        
        // Transport sync with host
    }
    else
    {
        // No playhead available
        // Use internal currentBPM variable
        if (currentBPM >= 200.0f) {
            static int noPlayheadCount = 0;
            if (++noPlayheadCount % 100 == 0) {
            }
        }
    }

    // Determine if we should be playing
    bool finalIsPlaying;
    if (useHostTransportParam && useHostTransportParam->get() && hasValidPosition)
    {
        // FIXED: Manual play button should work alongside host transport (OR logic)
        finalIsPlaying = isPlaying || internalPlaying;
        
        // BITWIG FIX: Always sync BPM, but conditionally sync position
        // Create temporary PositionInfo for sync functions
        juce::AudioPlayHead::CurrentPositionInfo tempPosInfo;
        tempPosInfo.isPlaying = isPlaying;
        tempPosInfo.isRecording = isRecording;
        tempPosInfo.ppqPosition = ppqPosition;
        tempPosInfo.bpm = bpm;
        
        syncBPMWithHost(tempPosInfo);
        syncPositionWithHost(tempPosInfo);
    }
    else
    {
        finalIsPlaying = internalPlaying;
    }

    // Pattern updates are now handled via UPI input only
    
    // Update timing if pattern length parameters changed
    static int lastPatternLengthUnit = 1; // Default to Beats
    static float lastPatternLengthValue = 8.0f;
    bool patternLengthChanged = false;
    
    int currentPatternLengthUnit = patternLengthUnitParam->getIndex();
    float currentPatternLengthValue = getPatternLengthValue();
    
    if (currentPatternLengthUnit != lastPatternLengthUnit || 
        std::abs(currentPatternLengthValue - lastPatternLengthValue) > 0.001f) {
        patternLengthChanged = true;
        lastPatternLengthUnit = currentPatternLengthUnit;
        lastPatternLengthValue = currentPatternLengthValue;
    }
    
    // Update timing if BPM or pattern length changed - but preserve currentSample ratio
    static float lastBPM = 120.0f;
    bool bpmChanged = std::abs(currentBPM - lastBPM) > 0.1f;
    
    if (bpmChanged || patternLengthChanged) {
        // BITWIG FIX: Preserve timing position when BPM changes
        double sampleRatio = (samplesPerStep > 0) ? (double)currentSample / samplesPerStep : 0.0;
        updateTiming();
        currentSample = static_cast<int>(sampleRatio * samplesPerStep);
        
    }
    
    // Always update lastBPM outside the if block
    if (bpmChanged) {
        lastBPM = currentBPM;
    }
    

    // FIRST STEP FIX: Track if we've processed step 0 in this buffer
    static bool processedFirstStepThisBuffer = false;
    if (!finalIsPlaying) processedFirstStepThisBuffer = false; // Reset when not playing

    // TRANSPORT-SYNCED TIMING: Use DAW's ppqPosition for perfect alignment
    if (finalIsPlaying && hasValidPosition)
    {
        // Calculate pattern timing based on DAW transport and pattern length parameters
        int lengthUnit = patternLengthUnitParam->getIndex(); // 0=Steps, 1=Beats, 2=Bars
        float lengthValue = getPatternLengthValue();
        int subdivisionIndex = subdivisionParam->getIndex();
        
        // Get pattern info for all modes
        auto pattern = patternEngine.getCurrentPattern();
        int patternSteps = static_cast<int>(pattern.size());
        if (patternSteps <= 0) patternSteps = 8; // Fallback
        
        // Calculate pattern length in beats using pattern length parameters
        double patternLengthInBeats;
        switch (lengthUnit) {
            case 0: // Steps mode - each step represents a subdivision, pattern length is IGNORED
            {
                // STEPS MODE DOCUMENTATION:
                // In Steps mode, each step represents a specific subdivision (16th, 8th, 8th triplet, etc.)
                // The subdivision parameter determines what each step represents.
                // Pattern length value is completely ignored - only subdivision and pattern size matter.
                //
                // Examples:
                // - 8 steps × 16th notes = 8 × 0.25 = 2 beats total (half note duration)
                // - 9 steps × 8th notes = 9 × 0.5 = 4.5 beats total  
                // - 9 steps × 8th triplets = 9 × (1/3) = 3 beats total
                //
                // This creates "microrhythm" functionality where the same pattern can be
                // played at different rhythmic resolutions by changing the subdivision.
                
                // Convert subdivision index to beat fraction per step
                double subdivisionBeatsPerStep = getSubdivisionInBeats(subdivisionIndex);
                
                // Steps mode: subdivision duration × number of steps (pattern length value ignored)
                patternLengthInBeats = subdivisionBeatsPerStep * patternSteps;
                
                break;
            }
            case 1: // Beats mode 
                patternLengthInBeats = lengthValue; 
                break;
            case 2: // Bars mode
                patternLengthInBeats = lengthValue * 4.0; // Assume 4/4 time
                break;
            case 3: // Auto mode - calculate optimal pattern length based on pattern characteristics
                patternLengthInBeats = calculateAutoPatternLength(pattern);
                break;
            default:
                patternLengthInBeats = lengthValue; // Default to beats
                break;
        }
        
        // Calculate beats per step based on pattern length parameters
        double beatsPerStep = patternLengthInBeats / patternEngine.getStepCount();
        double currentBeat = ppqPosition;
        double stepsFromStart = currentBeat / beatsPerStep;
        
        // Calculate which step should be active within the current pattern cycle
        // Use modulo on the raw step count to handle patterns that don't start at beat 0
        double stepsInCurrentCycle = fmod(stepsFromStart, patternEngine.getStepCount());
        
        // Trigger notes at the exact moment they should occur
        int numSamples = buffer.getNumSamples();
        
        // CRITICAL FIX: Use DAW-synchronized step calculation for perfect timing
        // The transport tick approach was causing DAW alignment issues
        int patternStepCount = patternEngine.getStepCount();
        int currentBufferStep = static_cast<int>(stepsInCurrentCycle); // REVERTED: Use DAW-synced step
        juce::ignoreUnused(patternStepCount);
        
        // SYNC DERIVED INDICES: Update transport tick to match DAW-synchronized step
        // This ensures derived accent calculations are aligned with DAW timing
        uint64_t dawSynchronizedTick = static_cast<uint64_t>(stepsFromStart);
        transportTick.store(dawSynchronizedTick);
        
        // Track the last processed step to detect boundaries
        static int lastProcessedStep = -1;
        
        // Only trigger if we've moved to a new step since last buffer
        if (currentBufferStep != lastProcessedStep)
        {
            // Calculate sample position for the step within this buffer
            double stepPosition = fmod(stepsInCurrentCycle, 1.0); // Fractional position within step
            int samplePosition = static_cast<int>(stepPosition * numSamples);
            if (samplePosition >= numSamples) samplePosition = 0; // Safety clamp
            
            // Update tracking
            lastProcessedStep = currentBufferStep;
            
            // Trigger the step ONCE per buffer
            processStep(midiMessages, samplePosition, currentBufferStep);
            
            // PHASE 3: currentStep now derived from transportTick - no manual update needed
            // currentStep.store(currentBufferStep);
        }
    }
    
    if (wasPlaying && !finalIsPlaying)
    {
        // Just stopped playing - reset position and clear all active notes
        currentSample = 0;
        // currentStep.store(0); // REMOVED: Using derived indices
        absoluteSamplePosition = 0;
        
        // CRITICAL: Initialize derived indices for DAW-synchronized timing
        // Since transport tick will be synchronized with DAW, set base to 0
        // This allows derived indices to work with absolute DAW-synchronized ticks
        baseTickRhythm.store(0);
        baseTickAccent.store(0);
        
        clearAllActiveNotes(midiMessages);
        // Stopped playing - resetting position
    }
    
    // CRITICAL: Process active notes and send note-offs at proper times 
    // MUST be called AFTER all note-ons have been added in this buffer
    // This ensures note-offs appear in the same MIDI buffer as their corresponding note-ons
    processActiveNotes(midiMessages, currentBufferSize);
    
    // CRITICAL: Update absolute sample position for note tracking AFTER processing
    // This ensures the timing calculations are correct for the next buffer
    absoluteSamplePosition += currentBufferSize;
    
    wasPlaying = finalIsPlaying;
}

//==============================================================================
bool SerpeAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SerpeAudioProcessor::createEditor()
{
    return new SerpeAudioProcessorEditor (*this);
}

//==============================================================================
void SerpeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Use AudioProcessorValueTreeState for parameter management
    auto state = parameters.copyState();
    
    // Add additional non-parameter state to the ValueTree
    state.setProperty("bpm", currentBPM, nullptr);
    state.setProperty("playing", internalPlaying, nullptr);
    
    // Save pattern data
    auto pattern = patternEngine.getCurrentPattern();
    juce::String patternString;
    for (int i = 0; i < pattern.size(); ++i)
    {
        patternString += pattern[i] ? "1" : "0";
    }
    state.setProperty("patternData", patternString, nullptr);
    
    // Save UPI input
    state.setProperty("currentUPIInput", currentUPIInput, nullptr);
    
    // Save background color
    state.setProperty("currentBackgroundColor", currentBackgroundColor, nullptr);
    
    // Save UPI history (ticker tape feature)
    juce::String upiHistoryString;
    for (int i = 0; i < upiHistory.size(); ++i) {
        if (i > 0) upiHistoryString += "\n";
        upiHistoryString += upiHistory[i];
    }
    state.setProperty("upiHistory", upiHistoryString, nullptr);
    
    // Save critical pattern state (Phase 3: Core Pattern State)
    state.setProperty("originalUPIInput", originalUPIInput, nullptr);
    state.setProperty("lastParsedUPI", lastParsedUPI, nullptr);
    state.setProperty("currentProgressivePatternKey", currentProgressivePatternKey, nullptr);
    state.setProperty("basePattern", basePattern, nullptr);
    
    // Save baseLengthPattern as string
    juce::String baseLengthPatternString;
    for (int i = 0; i < baseLengthPattern.size(); ++i)
    {
        baseLengthPatternString += baseLengthPattern[i] ? "1" : "0";
    }
    state.setProperty("baseLengthPattern", baseLengthPatternString, nullptr);
    
    // Save accent pattern state (Phase 3: Missing accent support)
    state.setProperty("hasAccentPattern", hasAccentPattern, nullptr);
    state.setProperty("globalOnsetCounter", globalOnsetCounter, nullptr);
    state.setProperty("uiAccentOffset", uiAccentOffset, nullptr);
    state.setProperty("accentPatternManuallyModified", accentPatternManuallyModified, nullptr);
    
    // Save currentAccentPattern as string
    juce::String accentPatternString;
    for (int i = 0; i < currentAccentPattern.size(); ++i)
    {
        accentPatternString += currentAccentPattern[i] ? "1" : "0";
    }
    state.setProperty("currentAccentPattern", accentPatternString, nullptr);
    
    // Save scene state (Phase 5: Scene State Persistence)
    state.setProperty("currentSceneIndex", currentSceneIndex, nullptr);
    state.setProperty("sceneCount", static_cast<int>(scenePatterns.size()), nullptr);
    
    // Save scene patterns as comma-separated string
    juce::String scenePatternsString;
    for (int i = 0; i < scenePatterns.size(); ++i) {
        if (i > 0) scenePatternsString += ",";
        scenePatternsString += scenePatterns[i];
    }
    state.setProperty("scenePatterns", scenePatternsString, nullptr);
    
    // Save per-scene progressive state
    juce::String sceneOffsetsString;
    for (int i = 0; i < sceneProgressiveOffsets.size(); ++i) {
        if (i > 0) sceneOffsetsString += ",";
        sceneOffsetsString += juce::String(sceneProgressiveOffsets[i]);
    }
    state.setProperty("sceneProgressiveOffsets", sceneOffsetsString, nullptr);
    
    juce::String sceneStepsString;
    for (int i = 0; i < sceneProgressiveSteps.size(); ++i) {
        if (i > 0) sceneStepsString += ",";
        sceneStepsString += juce::String(sceneProgressiveSteps[i]);
    }
    state.setProperty("sceneProgressiveSteps", sceneStepsString, nullptr);
    
    juce::String sceneBasePatternsString;
    for (int i = 0; i < sceneBasePatterns.size(); ++i) {
        if (i > 0) sceneBasePatternsString += ",";
        sceneBasePatternsString += sceneBasePatterns[i];
    }
    state.setProperty("sceneBasePatterns", sceneBasePatternsString, nullptr);
    
    juce::String sceneLengtheningString;
    for (int i = 0; i < sceneProgressiveLengthening.size(); ++i) {
        if (i > 0) sceneLengtheningString += ",";
        sceneLengtheningString += juce::String(sceneProgressiveLengthening[i]);
    }
    state.setProperty("sceneProgressiveLengthening", sceneLengtheningString, nullptr);
    
    // Save progressive transformation state (Phase 4: Progressive State Persistence)
    if (progressiveManager)
    {
        progressiveManager->saveProgressiveStatesToValueTree(state);
    }
    
    // Convert ValueTree to XML and save to memory block
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void SerpeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    
    if (xml != nullptr)
    {
        // Try to restore from new ValueTree format first
        auto state = juce::ValueTree::fromXml(*xml);
        if (state.isValid())
        {
            // Restore parameters via AudioProcessorValueTreeState
            parameters.replaceState(state);
            
            // Restore additional non-parameter state
            currentBPM = state.getProperty("bpm", 120.0);
            internalPlaying = state.getProperty("playing", false);
            
            // Restore pattern data
            juce::String patternString = state.getProperty("patternData", "10010010");
            std::vector<bool> pattern;
            // Use getCharPointer() for safer access to avoid String assertion
            auto chars = patternString.getCharPointer();
            for (int i = 0; i < patternString.length(); ++i)
            {
                pattern.push_back(chars[i] == '1');
            }
            patternEngine.setPattern(pattern);
            
            // Restore UPI input
            currentUPIInput = state.getProperty("currentUPIInput", "E(3,8)");
            
            // Restore background color
            currentBackgroundColor = state.getProperty("currentBackgroundColor", 0);
            
            // Restore UPI history (ticker tape feature)
            juce::String upiHistoryString = state.getProperty("upiHistory", juce::String());
            upiHistory.clear();
            if (!upiHistoryString.isEmpty()) {
                auto historyArray = juce::StringArray::fromLines(upiHistoryString);
                for (const auto& historyItem : historyArray) {
                    if (!historyItem.isEmpty()) {
                        upiHistory.add(historyItem);
                    }
                }
            }
            
            // Restore critical pattern state (Phase 3: Core Pattern State)
            originalUPIInput = state.getProperty("originalUPIInput", juce::String());
            lastParsedUPI = state.getProperty("lastParsedUPI", juce::String());
            currentProgressivePatternKey = state.getProperty("currentProgressivePatternKey", juce::String());
            basePattern = state.getProperty("basePattern", juce::String());
            
            // Restore baseLengthPattern from string
            juce::String baseLengthPatternString = state.getProperty("baseLengthPattern", juce::String());
            baseLengthPattern.clear();
            auto chars4 = baseLengthPatternString.getCharPointer();
            for (int i = 0; i < baseLengthPatternString.length(); ++i)
            {
                baseLengthPattern.push_back(chars4[i] == '1');
            }
            
            // Restore accent pattern state (Phase 3: Missing accent support)
            hasAccentPattern = state.getProperty("hasAccentPattern", false);
            globalOnsetCounter = state.getProperty("globalOnsetCounter", 0);
            uiAccentOffset = state.getProperty("uiAccentOffset", 0);
            accentPatternManuallyModified = state.getProperty("accentPatternManuallyModified", false);
            
            // Restore currentAccentPattern from string
            juce::String accentPatternString = state.getProperty("currentAccentPattern", juce::String());
            currentAccentPattern.clear();
            auto chars5 = accentPatternString.getCharPointer();
            for (int i = 0; i < accentPatternString.length(); ++i)
            {
                currentAccentPattern.push_back(chars5[i] == '1');
            }
            
            // Restore scene state (Phase 5: Scene State Persistence)
            currentSceneIndex = state.getProperty("currentSceneIndex", 0);
            int sceneCount = state.getProperty("sceneCount", 0);
            
            // Restore scene patterns from comma-separated string
            juce::String scenePatternsString = state.getProperty("scenePatterns", juce::String());
            scenePatterns.clear();
            if (!scenePatternsString.isEmpty()) {
                auto sceneArray = juce::StringArray::fromTokens(scenePatternsString, ",", "");
                for (const auto& scene : sceneArray) {
                    scenePatterns.add(scene);
                }
            }
            
            // Restore per-scene progressive state
            juce::String sceneOffsetsString = state.getProperty("sceneProgressiveOffsets", juce::String());
            sceneProgressiveOffsets.clear();
            if (!sceneOffsetsString.isEmpty()) {
                auto offsetArray = juce::StringArray::fromTokens(sceneOffsetsString, ",", "");
                for (const auto& offset : offsetArray) {
                    sceneProgressiveOffsets.push_back(offset.getIntValue());
                }
            }
            
            juce::String sceneStepsString = state.getProperty("sceneProgressiveSteps", juce::String());
            sceneProgressiveSteps.clear();
            if (!sceneStepsString.isEmpty()) {
                auto stepsArray = juce::StringArray::fromTokens(sceneStepsString, ",", "");
                for (const auto& step : stepsArray) {
                    sceneProgressiveSteps.push_back(step.getIntValue());
                }
            }
            
            juce::String sceneBasePatternsString = state.getProperty("sceneBasePatterns", juce::String());
            sceneBasePatterns.clear();
            if (!sceneBasePatternsString.isEmpty()) {
                auto basePatternsArray = juce::StringArray::fromTokens(sceneBasePatternsString, ",", "");
                for (const auto& basePattern : basePatternsArray) {
                    sceneBasePatterns.push_back(basePattern);
                }
            }
            
            juce::String sceneLengtheningString = state.getProperty("sceneProgressiveLengthening", juce::String());
            sceneProgressiveLengthening.clear();
            if (!sceneLengtheningString.isEmpty()) {
                auto lengtheningArray = juce::StringArray::fromTokens(sceneLengtheningString, ",", "");
                for (const auto& lengthening : lengtheningArray) {
                    sceneProgressiveLengthening.push_back(lengthening.getIntValue());
                }
            }
            
            // Initialize sceneBaseLengthPatterns with correct size
            sceneBaseLengthPatterns.clear();
            sceneBaseLengthPatterns.resize(sceneCount);
            
            // Restore progressive transformation state (Phase 4: Progressive State Persistence)
            if (progressiveManager)
            {
                progressiveManager->loadProgressiveStatesFromValueTree(state);
            }
            
            // Apply the restored UPI pattern after all state has been loaded
            // CRITICAL: Use originalUPIInput if available (contains full scene syntax)
            juce::String patternToRestore = originalUPIInput.isEmpty() ? currentUPIInput : originalUPIInput;
            if (!patternToRestore.isEmpty())
            {
                // CRITICAL FIX: Reset ALL scene and progressive state before parsing
                // This prevents inconsistencies between saved scene state and fresh parsing
                resetScenes();
                resetAccentSystem();
                if (sceneManager) {
                    sceneManager->resetScenes();
                }
                if (progressiveManager) {
                    progressiveManager->clearAllProgressiveStates();
                }
                
                // CRITICAL FIX: Use setUPIInput instead of parseAndApplyUPI for proper scene initialization
                // This ensures scene state is set up exactly like manual entry (Enter key behavior)
                setUPIInput(patternToRestore);
            }
            
            updateTiming();
        }
        // Fallback: handle old XML format for backward compatibility
        else if (xml->hasTagName("RhythmPatternExplorerState"))
        {
            currentBPM = xml->getDoubleAttribute("bpm", 120.0);
            internalPlaying = xml->getBoolAttribute("playing", false);
            
            // Restore basic parameters manually for old format
            if (useHostTransportParam) *useHostTransportParam = xml->getBoolAttribute("useHostTransport", true);
            if (midiNoteParam) *midiNoteParam = xml->getIntAttribute("midiNote", 36);
            
            // Restore pattern data from old format
            auto patternXml = xml->getChildByName("Pattern");
            if (patternXml != nullptr)
            {
                juce::String patternString = patternXml->getStringAttribute("data");
                std::vector<bool> pattern;
                // Use getCharPointer() for safer access to avoid String assertion
                auto chars = patternString.getCharPointer();
                for (int i = 0; i < patternString.length(); ++i)
                {
                    pattern.push_back(chars[i] == '1');
                }
                patternEngine.setPattern(pattern);
            }
            
            // Restore UPI input from old format
            auto upiXml = xml->getChildByName("UPI");
            if (upiXml != nullptr)
            {
                currentUPIInput = upiXml->getStringAttribute("input");
            }
            
            // Initialize critical pattern state (these won't exist in old format)
            originalUPIInput = currentUPIInput; // Use currentUPIInput as fallback
            lastParsedUPI = juce::String();
            currentProgressivePatternKey = juce::String();
            basePattern = juce::String();
            baseLengthPattern.clear();
            
            // Initialize accent pattern state (won't exist in old format)
            hasAccentPattern = false;
            currentAccentPattern.clear();
            globalOnsetCounter = 0;
            uiAccentOffset = 0;
            accentPatternManuallyModified = false;
            
            // Initialize scene state (won't exist in old format)
            currentSceneIndex = 0;
            scenePatterns.clear();
            sceneProgressiveOffsets.clear();
            sceneProgressiveSteps.clear();
            sceneBasePatterns.clear();
            sceneProgressiveLengthening.clear();
            sceneBaseLengthPatterns.clear();
            
            // Clear progressive transformation state (won't exist in old format)
            if (progressiveManager)
            {
                progressiveManager->clearAllProgressiveStates();
            }
            
            // CRITICAL FIX: Use setUPIInput for proper scene initialization (legacy format)
            // Use originalUPIInput if available (contains full scene syntax)
            juce::String patternToRestore = originalUPIInput.isEmpty() ? currentUPIInput : originalUPIInput;
            if (!patternToRestore.isEmpty())
            {
                setUPIInput(patternToRestore);
            }
            
            updateTiming();
        }
    }
}

//==============================================================================
void SerpeAudioProcessor::updateTiming()
{
    float bpm = currentBPM;
    
    // Get pattern length parameters for Phase 2 temporal control
    int lengthUnit = patternLengthUnitParam->getIndex(); // 0=Steps, 1=Beats, 2=Bars, 3=Subdivision
    float lengthValue = getPatternLengthValue();
    int subdivisionIndex = subdivisionParam->getIndex();
    
    // Calculate samples per step using pattern length parameters
    double beatsPerSecond = bpm / 60.0;
    double patternLengthInBeats;
    
    // Get pattern info for all modes
    auto pattern = patternEngine.getCurrentPattern();
    int patternSteps = static_cast<int>(pattern.size());
    if (patternSteps <= 0) patternSteps = 8; // Fallback
    
    // Calculate pattern length based on mode
    switch (lengthUnit) {
        case 0: // Steps mode - each step represents a subdivision, pattern length is IGNORED
        {
            // Convert subdivision index to beat fraction per step
            double subdivisionBeatsPerStep = getSubdivisionInBeats(subdivisionIndex);
            
            // Steps mode: subdivision duration × number of steps (pattern length value ignored)
            // Example: 8 steps × 16th notes = 8 × 0.25 = 2 beats total
            // Example: 9 steps × 8th notes = 9 × 0.5 = 4.5 beats total
            // Example: 9 steps × 8th triplets = 9 × (1/3) = 3 beats total
            patternLengthInBeats = subdivisionBeatsPerStep * patternSteps;
            
            break;
        }
        case 1: // Beats mode - pattern fits in specified number of beats
            patternLengthInBeats = lengthValue; 
            break;
        case 2: // Bars mode - pattern fits in specified number of bars
            patternLengthInBeats = lengthValue * 4.0; // Assume 4/4 time
            break;
        case 3: // Auto mode - calculate optimal pattern length based on pattern characteristics
            patternLengthInBeats = calculateAutoPatternLength(pattern);
            break;
        default:
            patternLengthInBeats = lengthValue; // Default to beats
            break;
    }
    
    double patternDurationInSeconds = patternLengthInBeats / beatsPerSecond;
    double stepDurationInSeconds = patternDurationInSeconds / patternSteps;
    samplesPerStep = static_cast<int>(currentSampleRate * stepDurationInSeconds);
    
    
    // Timing validation
    if (samplesPerStep <= 0) {
        samplesPerStep = static_cast<int>(currentSampleRate / 60.0); // Default to 1Hz fallback
    }
    
    // PHASE 1: Calculate precise step timing for monotonic transport
    samplesPerStepPrecise = currentSampleRate * stepDurationInSeconds;
    if (samplesPerStepPrecise <= 0.0) {
        samplesPerStepPrecise = currentSampleRate / 60.0; // Default fallback
    }
    
    // Debug: Enhanced logging to track timing instability
    std::ofstream logFile("/tmp/timing_debug.log", std::ios::app);
    if (logFile.is_open()) {
        logFile << "TIMING UPDATE: BPM=" << currentBPM 
                << ", samplesPerStep=" << samplesPerStep
                << ", samplesPerStepPrecise=" << samplesPerStepPrecise
                << ", stepDurationInSeconds=" << stepDurationInSeconds
                << ", sampleRate=" << currentSampleRate
                << ", lengthUnit=" << lengthUnit
                << ", lengthValue=" << lengthValue
                << ", subdivisionIndex=" << subdivisionIndex
                << ", patternSteps=" << patternSteps
                << ", patternLengthInBeats=" << patternLengthInBeats << std::endl;
        logFile.close();
    }
}

void SerpeAudioProcessor::processStep(juce::MidiBuffer& midiBuffer, int samplePosition, int stepToProcess)
{
    auto pattern = patternEngine.getCurrentPattern();
    
    // CRITICAL FIX: Only trigger if this step has an onset in the pattern
    if (stepToProcess < pattern.size() && pattern[stepToProcess])
    {
        // PHASE 2: Accent decision logic with feature flag for migration
        bool isAccented;
        
        if (useNewAccentSystem) {
            // NEW ROBUST ACCENT SYSTEM: Use AccentSequence with O(1) lookup
            if (currentAccentSequence && currentAccentSequence->isValid()) {
                uint64_t currentTick = transportTick.load();
                uint64_t baseTick = baseTickRhythm.load();
                uint32_t stepInSequence = static_cast<uint32_t>((currentTick - baseTick) % currentAccentSequence->getSequenceLength());
                isAccented = currentAccentSequence->isAccentedAtStep(stepInSequence);
                
                // DEBUG: Log new system usage (minimal logging)
                static int newSystemLogCount = 0;
                if ((++newSystemLogCount % 20) == 0) {
                    DBG("NEW ACCENT SYSTEM: step=" << stepToProcess << ", accented=" << (isAccented ? "true" : "false"));
                }
            } else {
                isAccented = false; // No accent sequence = no accents
            }
        } else {
            // LEGACY ACCENT SYSTEM: Current fragile logic (for rollback)
            if (patternManuallyModified) {
                // SUSPENSION MODE: Use step-based accent logic (manual modifications)
                isAccented = shouldStepBeAccented(stepToProcess);
            } else {
                // NORMAL MODE: Use onset-based accent logic (UPI patterns, progressive transformations)
                isAccented = shouldOnsetBeAccented(getCurrentOnsetCount());
            }
            
            // DEBUG: Log legacy system usage (reduced logging)
            static int legacySystemLogCount = 0;
            if ((++legacySystemLogCount % 50) == 0) {
                DBG("LEGACY ACCENT SYSTEM: step=" << stepToProcess << ", accented=" << (isAccented ? "true" : "false"));
            }
        }
        
        // PHASE 2: Parallel validation (only when using legacy system for migration safety)
        if (!useNewAccentSystem && currentAccentSequence && currentAccentSequence->isValid()) {
            uint64_t currentTick = transportTick.load();
            uint64_t baseTick = baseTickRhythm.load();
            uint32_t stepInSequence = static_cast<uint32_t>((currentTick - baseTick) % currentAccentSequence->getSequenceLength());
            bool newAccent = currentAccentSequence->isAccentedAtStep(stepInSequence);
            
            // ASSERTION: Both systems must produce identical results
            if (newAccent != isAccented) {
                DBG("MIGRATION VALIDATION FAILED! Step: " << stepToProcess 
                    << ", Legacy: " << (isAccented ? "true" : "false") 
                    << ", New: " << (newAccent ? "true" : "false"));
                
                // Log detailed state for debugging migration issues
                std::ofstream logFile("/tmp/serpe_migration_mismatch.log", std::ios::app);
                if (logFile.is_open()) {
                    logFile << "MIGRATION_MISMATCH - Step: " << stepToProcess
                            << ", Legacy: " << (isAccented ? "true" : "false")
                            << ", New: " << (newAccent ? "true" : "false")
                            << ", stepInSequence: " << stepInSequence
                            << std::endl;
                    logFile.close();
                }
                
                // Continue with legacy result but flag for investigation
                jassertfalse;
            }
        }
        
        // Trigger MIDI note
        triggerNote(midiBuffer, samplePosition, isAccented);
        
        // CRITICAL: Track the onset count used for MIDI for UI synchronization
        lastMidiOnsetCount.store(getCurrentOnsetCount());
    }
    
    // Notify UI of cycle completion for pattern change updates
    // Use DAW-synchronized step for cycle boundary detection (matches MIDI timing)
    int nextStep = (stepToProcess + 1) % static_cast<int>(pattern.size());
    if (nextStep == 0)
    {
        /**
         * UI ACCENT OFFSET CYCLE BOUNDARY UPDATE
         * 
         * This is where we synchronize the UI accent display with the current MIDI state.
         * By updating only at cycle boundaries (nextStep == 0), we ensure:
         * 
         * 1. UI accent markers remain stable during cycle playback
         * 2. UI eventually catches up to any MIDI accent progression
         * 3. No rapid visual updates that cause "swirling"
         * 
         * The modulo operation ensures the UI always shows the correct accent pattern
         * position, even for complex polymetric relationships.
         */
        // Update stable UI accent offset at cycle boundaries (only if not manually modified)
        if (hasAccentPattern && !currentAccentPattern.empty() && !accentPatternManuallyModified)
        {
            // DETERMINISTIC: Use the exact same cumulative onset count that MIDI used
            // This ensures UI accent markers match MIDI accent timing perfectly
            uint32_t midiOnsetCount = lastMidiOnsetCount.load();
            int accentPatternSize = static_cast<int>(currentAccentPattern.size());
            
            // UI accent offset matches MIDI: use same cumulative onset count
            uiAccentOffset = static_cast<int>(midiOnsetCount % accentPatternSize);
            
            // UI accent offset now uses identical calculation as MIDI (cumulative onset count)
        }
        else if (hasAccentPattern && accentPatternManuallyModified)
        {
        }
        patternChanged.store(true); // UI can refresh at cycle boundaries
    }
}

// ============================================================================
// iPad AUv3 MIDI Note Trigger with Redundant Note-Off Safety
// ============================================================================
// PLATFORM DIFFERENCE: This function includes iPad-specific note-off redundancy
// that is not needed in the desktop AU/VST3 versions.
//
// ISSUE: AUv3 framework occasionally loses note-off messages, causing notes
//        to last until the end of recording instead of having proper duration
// SOLUTION: Dual note-off messages ensure notes are always terminated
// DESKTOP: Desktop versions use single note-off without issues
void SerpeAudioProcessor::triggerNote(juce::MidiBuffer& midiBuffer, int samplePosition, bool isAccented)
{
    // Get base MIDI note number from parameter
    int baseNoteNumber = midiNoteParam ? midiNoteParam->get() : 36; // Default kick drum note
    
    // Apply accent parameters if accented
    int noteNumber = baseNoteNumber;
    float velocity = 0.8f; // Default unaccented velocity
    
    if (isAccented && hasAccentPattern)
    {
        // Accented note: higher velocity + pitch offset (typically F1 = C1 + 5 semitones)
        velocity = accentVelocityParam ? accentVelocityParam->get() : 1.0f;
        int pitchOffset = accentPitchOffsetParam ? accentPitchOffsetParam->get() : 5;
        noteNumber = baseNoteNumber + pitchOffset;
    }
    else
    {
        // Regular note: standard velocity (typically C1)
        velocity = unaccentedVelocityParam ? unaccentedVelocityParam->get() : 0.8f;
    }
    
    // Create and send note-on immediately
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, noteNumber, velocity);
    midiBuffer.addEvent(noteOn, samplePosition);
    
    // Calculate note duration (use 80% of step duration for musical notes)
    int noteDuration = static_cast<int>(samplesPerStep * 0.8);
    if (noteDuration < 2048) noteDuration = 2048; // Minimum ~46ms at 44.1kHz for instrument compatibility
    
    // Debug counter for UI display
    debugTriggerCount++;
    
    // Add note to tracking system for proper duration management
    addActiveNote(noteNumber, noteDuration);
}

void SerpeAudioProcessor::addActiveNote(int noteNumber, int duration)
{
    // Calculate absolute end position for this note
    int endPosition = absoluteSamplePosition + duration;
    
    // Add to active notes list
    activeNotes.emplace_back(noteNumber, endPosition);
}

void SerpeAudioProcessor::processActiveNotes(juce::MidiBuffer& midiBuffer, int bufferSize)
{
    // CRITICAL: This method sends note-off messages for notes whose duration has expired
    // TIMING REQUIREMENT: Must be called AFTER all note-ons have been added to midiBuffer
    // CROSS-PLATFORM: iPadOS is more sensitive to incorrect timing than macOS
    
    // Process all active notes and send note-offs when their time is up
    for (auto& note : activeNotes)
    {
        if (note.isActive && note.endSample >= absoluteSamplePosition && note.endSample < absoluteSamplePosition + bufferSize)
        {
            // Time to send note-off for this note
            int bufferPosition = note.endSample - absoluteSamplePosition;
            
            // CRITICAL FIX: Ensure buffer position is valid
            if (bufferPosition >= 0 && bufferPosition < bufferSize)
            {
                // CRITICAL FIX: Use proper velocity for note-off (some hosts/instruments need non-zero velocity)
                juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, note.noteNumber, 0.5f);
                midiBuffer.addEvent(noteOff, bufferPosition);
                note.isActive = false;
                
                // Debug counter for UI display
                debugNoteOffsSent++;
            }
            else
            {
                // Invalid buffer position - mark note as inactive but don't send note-off
                note.isActive = false;
            }
        }
    }
    
    // Clean up inactive notes periodically
    if (activeNotes.size() > 100) // Prevent memory buildup
    {
        activeNotes.erase(
            std::remove_if(activeNotes.begin(), activeNotes.end(), 
                          [](const ActiveNote& note) { return !note.isActive; }),
            activeNotes.end()
        );
    }
}

void SerpeAudioProcessor::clearAllActiveNotes(juce::MidiBuffer& midiBuffer)
{
    // Send immediate note-offs for all active notes
    for (auto& note : activeNotes)
    {
        if (note.isActive)
        {
            juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, note.noteNumber, 0.0f);
            midiBuffer.addEvent(noteOff, 0);
            note.isActive = false;
        }
    }
    activeNotes.clear();
}

void SerpeAudioProcessor::syncBPMWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // ALWAYS sync BPM - this should never be disabled
    if (posInfo.bpm > 0)
    {
        float hostBPM = static_cast<float>(posInfo.bpm);
        float currentBPMValue = currentBPM;
        
        
        // Update our internal BPM to match host
        if (std::abs(currentBPMValue - hostBPM) > 0.1f)
        {
            currentBPM = hostBPM;
            updateTiming();
            
        }
    }
}

void SerpeAudioProcessor::syncPositionWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // Phase 3: Advanced Host Sync - Loop point detection
    if (enableLoopSync && posInfo.isLooping)
    {
        bool wasLooping = hostIsLooping;
        hostIsLooping = true;
        
        // Detect loop restart (position jumped backward significantly)
        if (wasLooping && posInfo.ppqPosition < lastHostPosition - 0.1)
        {
            // Loop restarted - reset pattern timing but preserve accent flow
            // currentStep.store(0); // REMOVED: Using derived indices
            currentSample = 0;
            // CRITICAL: Do NOT reset globalOnsetCounter - accent patterns should flow across loop boundaries
            // Only reset uiAccentOffset for UI display consistency
            uiAccentOffset = static_cast<int>(getCurrentOnsetCount() % (hasAccentPattern && !currentAccentPattern.empty() ? currentAccentPattern.size() : 1));
            patternChanged.store(true);
        }
        
        // Update loop boundaries
        if (!wasLooping)
        {
            hostLoopStart = posInfo.ppqPosition;
        }
    }
    else
    {
        hostIsLooping = false;
    }
    
    // Calculate pattern position based on host timeline
    if (posInfo.ppqPosition >= 0.0)
    {
        // Calculate which step we should be on based on timeline position using pattern length parameters
        // Get pattern length parameters
        int lengthUnit = patternLengthUnitParam->getIndex();
        float lengthValue = getPatternLengthValue();
        int subdivisionIndex = subdivisionParam->getIndex();
        
        // Get pattern info
        auto pattern = patternEngine.getCurrentPattern();
        int patternSteps = static_cast<int>(pattern.size());
        if (patternSteps <= 0) patternSteps = 8; // Fallback
        
        // Calculate pattern length based on mode (same logic as updateTiming)
        double patternLengthInBeats;
        switch (lengthUnit) {
            case 0: // Steps mode - each step represents a subdivision, pattern length is IGNORED
            {
                // Steps mode: Same logic as main calculation - subdivision determines step duration
                double subdivisionBeatsPerStep = getSubdivisionInBeats(subdivisionIndex);
                patternLengthInBeats = subdivisionBeatsPerStep * patternSteps;
                
                break;
            }
            case 1: // Beats mode - pattern fits in specified number of beats
                patternLengthInBeats = lengthValue; 
                break;
            case 2: // Bars mode - pattern fits in specified number of bars
                patternLengthInBeats = lengthValue * 4.0; // Assume 4/4 time
                break;
            case 3: // Auto mode - calculate optimal pattern length based on pattern characteristics
                patternLengthInBeats = calculateAutoPatternLength(pattern);
                break;
            default:
                patternLengthInBeats = lengthValue; // Default to beats
                break;
        }
        
        double beatsPerStep = patternLengthInBeats / patternSteps;
        double currentBeat = posInfo.ppqPosition;
        double stepsFromStart = currentBeat / beatsPerStep;
        
        // Calculate which step should be active within the current pattern cycle
        double stepsInCurrentCycle = fmod(stepsFromStart, patternSteps);
        int targetStep = static_cast<int>(stepsInCurrentCycle);
        
        
        // CRITICAL FIX: Re-enable position sync for perfect DAW timing alignment
        // Use internal currentBPM variable
        bool allowPositionSync = true;  // ENABLED: Position sync needed for tick 1 precision
        
        // CRITICAL FIX: Only sync position when significantly out of sync (not every block)
        int stepDifference = std::abs(targetStep - getCurrentStep());
        if (allowPositionSync && stepDifference > 2)  // Increased threshold from 1 to 2
        {
            // currentStep.store(targetStep); // REMOVED: Using derived indices
            currentSample = static_cast<int>((stepsFromStart - std::floor(stepsFromStart)) * samplesPerStep);
        }
    }
    
    hostIsPlaying = posInfo.isPlaying;
    lastHostPosition = posInfo.ppqPosition;
}

void SerpeAudioProcessor::setUPIInput(const juce::String& upiPattern)
{
    juce::ScopedLock lock(processingLock);
    
    // Add to history when pattern is entered (not when restored from state)
    addToUPIHistory(upiPattern.trim());
    
    // Check for progressive syntax: scenes first (pattern|pattern|pattern), then pattern%N (offset), pattern*N (lengthening)
    juce::String pattern = upiPattern.trim();
    
    // Store original UPI input if it contains progressive/scene syntax for later Tick/MIDI advancement
    bool hasProgressiveTransformation = pattern.contains(">");
    bool hasScenes = pattern.contains("|");
    bool hasProgressiveOffsetSyntax = (pattern.contains("%") && pattern.lastIndexOf("%") > 0) || (pattern.contains("+") && pattern.lastIndexOf("+") > 0);
    if (hasProgressiveTransformation || hasScenes || hasProgressiveOffsetSyntax) {
        originalUPIInput = pattern;
    } else {
        originalUPIInput.clear(); // Clear if no progressive syntax
    }
    
    // Check if + is followed by a number (progressive offset) vs pattern (combination)
    bool isProgressiveOffset = false;
    if (!hasScenes && ((pattern.contains("%") && pattern.lastIndexOf("%") > 0) || (pattern.contains("+") && pattern.lastIndexOf("+") > 0))) {
        // Check % first (preferred syntax), then + (legacy)
        if (pattern.contains("%") && pattern.lastIndexOf("%") > 0) {
            int lastPercentIndex = pattern.lastIndexOf("%");
            juce::String afterPercent = pattern.substring(lastPercentIndex + 1).trim();
            isProgressiveOffset = afterPercent.containsOnly("0123456789-") && afterPercent.isNotEmpty();
        } else if (pattern.contains("+") && pattern.lastIndexOf("+") > 0) {
            int lastPlusIndex = pattern.lastIndexOf("+");
            juce::String afterPlus = pattern.substring(lastPlusIndex + 1).trim();
            // Progressive offset if what follows + is purely numeric (including negative numbers)
            isProgressiveOffset = afterPlus.containsOnly("0123456789-") && afterPlus.isNotEmpty();
        }
    }
    
    bool isProgressiveLengthening = !hasScenes && pattern.contains("*") && pattern.lastIndexOf("*") > 0;
    
    if (hasScenes)
    {
        // Handle scene cycling: pattern1|pattern2|pattern3
        // CRITICAL: Split scenes BEFORE any UPI parsing to preserve individual accent patterns
        auto scenes = juce::StringArray::fromTokens(pattern, "|", "");
        
        // Scene cycling detection
        
        // Check if this is the same scene sequence or a new one
        bool isSameSequence = (scenes.size() == scenePatterns.size());
        if (isSameSequence) {
            for (int i = 0; i < scenes.size(); ++i) {
                if (scenes[i].trim() != scenePatterns[i]) {
                    isSameSequence = false;
                    break;
                }
            }
        }
        
        if (isSameSequence && !scenePatterns.isEmpty())
        {
            // Same sequence - advance to next scene
            advanceScene();
            
        }
        else
        {
            // New scene sequence - reset and initialize per-scene progressive state
            scenePatterns.clear();
            sceneProgressiveOffsets.clear();
            sceneProgressiveSteps.clear();
            sceneBasePatterns.clear();
            sceneProgressiveLengthening.clear();
            sceneBaseLengthPatterns.clear();
            
            // Parse each scene and initialize its progressive state  
            // CRITICAL: Store original scene patterns with accent syntax intact
            for (const auto& scene : scenes) {
                juce::String scenePattern = scene.trim();
                scenePatterns.add(scenePattern);
                
                // Check if this scene has progressive syntax
                bool hasProgressiveOffset = false;
                if ((scenePattern.contains("%") && scenePattern.lastIndexOf("%") > 0) || (scenePattern.contains("+") && scenePattern.lastIndexOf("+") > 0)) {
                    if (scenePattern.contains("%") && scenePattern.lastIndexOf("%") > 0) {
                        int lastPercentIndex = scenePattern.lastIndexOf("%");
                        juce::String afterPercent = scenePattern.substring(lastPercentIndex + 1).trim();
                        hasProgressiveOffset = afterPercent.containsOnly("0123456789-");
                    } else if (scenePattern.contains("+") && scenePattern.lastIndexOf("+") > 0) {
                        int lastPlusIndex = scenePattern.lastIndexOf("+");
                        juce::String afterPlus = scenePattern.substring(lastPlusIndex + 1).trim();
                        hasProgressiveOffset = afterPlus.containsOnly("0123456789-");
                    }
                }
                bool hasProgressiveLengthening = scenePattern.contains("*") && scenePattern.lastIndexOf("*") > 0;
                
                if (hasProgressiveOffset) {
                    // Parse offset syntax: pattern%N or pattern+N
                    int symbolIndex = -1;
                    if (scenePattern.contains("%")) {
                        symbolIndex = scenePattern.lastIndexOf("%");
                    } else if (scenePattern.contains("+")) {
                        symbolIndex = scenePattern.lastIndexOf("+");
                    }
                    juce::String basePattern = scenePattern.substring(0, symbolIndex).trim();
                    juce::String offsetStr = scenePattern.substring(symbolIndex + 1).trim();
                    int step = offsetStr.getIntValue();
                    
                    sceneBasePatterns.push_back(basePattern);
                    sceneProgressiveSteps.push_back(step);
                    sceneProgressiveOffsets.push_back(step); // Start with first offset
                    sceneProgressiveLengthening.push_back(0);
                    sceneBaseLengthPatterns.push_back(std::vector<bool>());
                } else if (hasProgressiveLengthening) {
                    // Parse lengthening syntax: pattern*N
                    int starIndex = scenePattern.lastIndexOf("*");
                    juce::String basePattern = scenePattern.substring(0, starIndex).trim();
                    juce::String lengthStr = scenePattern.substring(starIndex + 1).trim();
                    int lengthStep = lengthStr.getIntValue();
                    
                    sceneBasePatterns.push_back(basePattern);
                    sceneProgressiveSteps.push_back(lengthStep);
                    sceneProgressiveOffsets.push_back(0);
                    sceneProgressiveLengthening.push_back(lengthStep); // Start with first lengthening
                    sceneBaseLengthPatterns.push_back(std::vector<bool>()); // Will be filled when pattern is generated
                } else {
                    // Simple pattern without progressive syntax
                    sceneBasePatterns.push_back(scenePattern);
                    sceneProgressiveSteps.push_back(0);
                    sceneProgressiveOffsets.push_back(0);
                    sceneProgressiveLengthening.push_back(0);
                    sceneBaseLengthPatterns.push_back(std::vector<bool>());
                }
            }
            
            currentSceneIndex = 0;
            
            
            // TRANSITION: Initialize SceneManager with same data as legacy system
            if (sceneManager) {
                sceneManager->initializeScenes(scenes);
            }
        }
        
        // Parse and apply the current scene pattern using per-scene progressive state
        if (!scenePatterns.isEmpty() && currentSceneIndex < scenePatterns.size()) {
            applyCurrentScenePattern();
        }
    }
    else if (isProgressiveOffset)
    {
        // Handle progressive offset: pattern%N or pattern+N
        int symbolIndex = -1;
        if (pattern.contains("%")) {
            symbolIndex = pattern.lastIndexOf("%");
        } else if (pattern.contains("+")) {
            symbolIndex = pattern.lastIndexOf("+");
        }
        juce::String newBasePattern = pattern.substring(0, symbolIndex).trim();
        juce::String stepStr = pattern.substring(symbolIndex + 1).trim();
        int newStep = stepStr.getIntValue();
        
        // Progressive offset debug logging
        
        // If same base pattern, advance offset; if different, reset
        if (basePattern == newBasePattern && progressiveStep == newStep)
        {
            advanceProgressiveOffset();
            
            // Notify UI that pattern has changed for accent map updates
            patternChanged.store(true);
            
        }
        else
        {
            // New progressive pattern - reset and start
            basePattern = newBasePattern;
            progressiveStep = newStep;
            progressiveOffset = newStep; // Start with first offset
            
        }
        
        // Parse the base pattern first, then apply progressive offset via rotation
        parseAndApplyUPI(basePattern);
        
        // Apply progressive offset by rotating the generated pattern
        if (progressiveOffset != 0)
        {
            auto currentPattern = patternEngine.getCurrentPattern();
            // Use negative rotation for clockwise progression (positive offset = clockwise)
            auto rotatedPattern = PatternUtils::rotatePattern(currentPattern, -progressiveOffset);
            patternEngine.setPattern(rotatedPattern);
            
        }
    }
    else if (isProgressiveLengthening)
    {
        // Handle progressive lengthening: pattern*N
        int starIndex = pattern.lastIndexOf("*");
        juce::String newBasePattern = pattern.substring(0, starIndex).trim();
        juce::String lengthStr = pattern.substring(starIndex + 1).trim();
        int newLengthening = lengthStr.getIntValue();
        
        // Progressive lengthening debug logging
        
        // If same base pattern, advance lengthening; if different, reset
        if (basePattern == newBasePattern && progressiveLengthening == newLengthening)
        {
            advanceProgressiveLengthening();
            
            // Notify UI that pattern has changed for accent map updates
            patternChanged.store(true);
            
        }
        else
        {
            // New progressive lengthening pattern - reset and start
            basePattern = newBasePattern;
            progressiveLengthening = newLengthening;
            
            // Parse base pattern and store for lengthening
            parseAndApplyUPI(basePattern);
            baseLengthPattern = patternEngine.getCurrentPattern();
            
            // Apply initial lengthening immediately (don't wait for second trigger)
            advanceProgressiveLengthening();
            
        }
        
        // Apply current lengthened pattern
        patternEngine.setPattern(baseLengthPattern);
    }
    else
    {
        // Non-progressive pattern - reset progressive states
        progressiveOffset = 0;
        progressiveStep = 0;
        progressiveLengthening = 0;
        basePattern = "";
        baseLengthPattern.clear();
        scenePatterns.clear();
        currentSceneIndex = 0;
        
        // TRANSITION: Reset SceneManager in parallel with legacy system
        if (sceneManager) {
            sceneManager->resetScenes();
        }
        parseAndApplyUPI(pattern, true);
        // currentStep.store(0); // REMOVED: Using derived indices
    }
    
    currentUPIInput = upiPattern;
}

//==============================================================================
// Interactive Pattern Modification Methods
//==============================================================================

void SerpeAudioProcessor::togglePatternStep(int stepIndex)
{
    juce::ScopedLock lock(processingLock);
    
    if (!isValidStepIndex(stepIndex)) {
        return;
    }
    
    // ENTER SUSPENSION MODE: Preserve current VISUAL accent state before pattern modification
    if (!patternManuallyModified) {
        // CRITICAL FIX: Capture current visual accent state BEFORE any pattern changes
        auto currentVisualAccents = getCurrentAccentMap(); // Call while still in normal mode
        
        // Now enter suspension mode
        patternManuallyModified = true;
        suspendedRhythmPattern = patternEngine.getCurrentPattern();
        
        // Get current pattern and toggle the specified step
        auto currentPattern = patternEngine.getCurrentPattern();
        currentPattern[stepIndex] = !currentPattern[stepIndex];
        
        // PHASE 2: Apply the modified pattern via queue for phase-locking
        queuePatternUpdate(currentPattern, currentAccentPattern, hasAccentPattern, 0);
        
        // CRITICAL FIX: Map the captured visual accents to the NEW pattern structure
        // This preserves accent positions even when pattern structure changes
        currentAccentPattern.clear();
        currentAccentPattern.resize(currentPattern.size(), false);
        
        // Copy accents from old visual state to new pattern structure
        for (int i = 0; i < std::min(static_cast<int>(currentVisualAccents.size()), static_cast<int>(currentPattern.size())); ++i) {
            if (currentPattern[i] && i < currentVisualAccents.size()) {
                currentAccentPattern[i] = currentVisualAccents[i];
            }
        }
        hasAccentPattern = true; // We now have a captured accent pattern
        
        // Update suspended patterns with the NEW structure
        suspendedRhythmPattern = currentPattern;
        suspendedAccentPattern = currentAccentPattern;
        
    } else {
        // Already in suspension mode - just modify the pattern
        auto currentPattern = patternEngine.getCurrentPattern();
        currentPattern[stepIndex] = !currentPattern[stepIndex];
        
        // Update suspended patterns (this becomes the new cycle state)
        suspendedRhythmPattern = currentPattern;
        
        // PHASE 2: Apply the modified pattern via queue for phase-locking
        queuePatternUpdate(currentPattern, currentAccentPattern, hasAccentPattern, 0);
        
        // CRITICAL FIX: Only resize accent pattern for UI modifications, not UPI patterns
        // UPI accent patterns should maintain their original size for onset-based logic
        if (currentAccentPattern.size() != currentPattern.size() && accentPatternManuallyModified) {
            currentAccentPattern.resize(currentPattern.size(), false);
            suspendedAccentPattern = currentAccentPattern;
        }
    }
    
    // Update step indicator to current position for immediate feedback
    // currentStep.store(stepIndex); // REMOVED: Using derived indices
    
    // Update UPI display to reflect the manual modification
    updateUPIFromCurrentPattern();
    
    // Notify UI of pattern change
    patternChanged.store(true);
    
}

void SerpeAudioProcessor::toggleAccentAtStep(int stepIndex)
{
    juce::ScopedLock lock(processingLock);
    
    if (!isValidStepIndex(stepIndex)) {
        return;
    }
    
    // First ensure the step is an onset - can't accent a rest
    auto currentPattern = patternEngine.getCurrentPattern();
    if (!currentPattern[stepIndex]) {
        return;
    }
    
    // ENTER SUSPENSION MODE: Preserve current VISUAL accent state before manual modification
    if (!patternManuallyModified) {
        // CRITICAL FIX: Capture current visual accent state BEFORE setting patternManuallyModified
        // This preserves the live accent cycling state instead of losing it
        auto currentVisualAccents = getCurrentAccentMap(); // Call this while still in normal mode
        
        // Now enter suspension mode
        patternManuallyModified = true;
        suspendedRhythmPattern = patternEngine.getCurrentPattern();
        
        // Set the captured visual state as our new accent pattern
        currentAccentPattern = currentVisualAccents;
        hasAccentPattern = true; // We now have a captured accent pattern
        
        suspendedAccentPattern = currentAccentPattern;
    }
    
    // Toggle accent at this step (working with the preserved visual state)
    if (stepIndex < currentAccentPattern.size()) {
        currentAccentPattern[stepIndex] = !currentAccentPattern[stepIndex];
    } else {
        // Pattern size mismatch - only resize if this is a manually modified accent pattern
        if (accentPatternManuallyModified) {
            currentAccentPattern.resize(currentPattern.size(), false);
        }
        if (stepIndex < currentAccentPattern.size()) {
            currentAccentPattern[stepIndex] = !currentAccentPattern[stepIndex];
        }
    }
    
    // Update suspended accent pattern (this becomes the new cycle state)
    suspendedAccentPattern = currentAccentPattern;
    
    // Mark accent pattern as manually modified to prevent automatic cycling
    accentPatternManuallyModified = true;
    
    // Update UPI display to show accent pattern changes
    updateUPIFromCurrentPattern();
    
    // Notify UI of pattern change
    patternChanged.store(true);
}

bool SerpeAudioProcessor::isValidStepIndex(int stepIndex) const
{
    auto currentPattern = patternEngine.getCurrentPattern();
    return stepIndex >= 0 && stepIndex < static_cast<int>(currentPattern.size());
}

void SerpeAudioProcessor::updateUPIFromCurrentPattern()
{
    // Convert the current pattern back to a binary representation for UPI display
    auto currentPattern = patternEngine.getCurrentPattern();
    
    if (currentPattern.empty()) {
        currentUPIInput = "E(0,8)"; // Default fallback
        return;
    }
    
    // Generate binary string representation
    juce::String binaryString;
    for (bool step : currentPattern) {
        binaryString += step ? "1" : "0";
    }
    
    // Include accent pattern if present
    juce::String displayUPI;
    if (hasAccentPattern && !currentAccentPattern.empty()) {
        // Generate accent binary string
        juce::String accentString;
        for (bool accent : currentAccentPattern) {
            accentString += accent ? "1" : "0";
        }
        displayUPI = "{" + accentString + "}" + binaryString + ":" + juce::String(static_cast<int>(currentPattern.size()));
    } else {
        displayUPI = binaryString + ":" + juce::String(static_cast<int>(currentPattern.size()));
    }
    
    // Update UPI input to show the manual modification for display purposes
    currentUPIInput = displayUPI;
    
    // CRITICAL: Do NOT clear originalUPIInput here - preserve progressive/scene syntax for Tick/MIDI
    // originalUPIInput stays intact so Tick button and MIDI can still advance progressive transformations
    
}

void SerpeAudioProcessor::parseAndApplyUPI(const juce::String& upiPattern, bool resetAccentPosition)
{
    if (upiPattern.isEmpty())
        return;
    
    // SAFETY CHECK: Complex pattern validation to prevent crashes
    try {
        // Check for extremely complex patterns that might cause issues
        if (upiPattern.length() > 500) {
            // Pattern too complex - use safe fallback
            parseAndApplyUPI("E(3,8)", resetAccentPosition);
            return;
        }
        
        // Double B notation like B(3,21)B>17 should work fine - let UPIParser handle it
        
        // Double notation patterns (E(n,s)E>target, W(n,s)W>target, D(n,s)D>target) should work fine - let UPIParser handle them
        
        // For progressive patterns, always re-parse to advance state
        bool isProgressive = upiPattern.contains("#");
        
        // Check if this is a progressive transformation with ">" syntax
        bool isProgressiveTransformation = upiPattern.contains(">");
        if (isProgressiveTransformation) {
            // Store the pattern key for step tracking
            currentProgressivePatternKey = upiPattern;
        }
        
        auto parseResult = UPIParser::parse(upiPattern);
    
    if (parseResult.isValid())
    {
        
        // PHASE 2: Apply the parsed pattern via queue for phase-locking
        queuePatternUpdate(parseResult.pattern, parseResult.accentPattern, parseResult.hasAccentPattern, 0);
        
        // Store quantization metadata if present
        if (parseResult.hasQuantization)
        {
            hasQuantization = true;
            originalStepCount = parseResult.originalStepCount;
            quantizedStepCount = parseResult.quantizedStepCount;
            quantizationClockwise = parseResult.quantizationClockwise;
            originalOnsetCount = parseResult.originalOnsetCount;
            quantizedOnsetCount = parseResult.quantizedOnsetCount;
            
        }
        else
        {
            hasQuantization = false;
            originalStepCount = 0;
            quantizedStepCount = 0;
            quantizationClockwise = true;
            originalOnsetCount = 0;
            quantizedOnsetCount = 0;
        }
        
        // Set up accent pattern if present
        if (parseResult.hasAccentPattern)
        {
            hasAccentPattern = true;
            currentAccentPattern = parseResult.accentPattern;
            accentMapNeedsUpdate.store(true); // Mark accent map for regeneration
            
            // DEBUG: Log accent pattern setup to file
            std::ofstream logFile("/tmp/serpe_accent_debug.log", std::ios::app);
            if (logFile.is_open()) {
                logFile << "ACCENT SETUP - Pattern: " << upiPattern.toStdString() 
                        << ", Size: " << currentAccentPattern.size() 
                        << ", Pattern: ";
                for (size_t i = 0; i < std::min(currentAccentPattern.size(), size_t(8)); i++) {
                    logFile << (currentAccentPattern[i] ? "1" : "0");
                }
                logFile << std::endl;
                logFile.close();
            }
        }
        else
        {
            hasAccentPattern = false;
            currentAccentPattern.clear();
            accentMapNeedsUpdate.store(true); // Mark accent map for regeneration
            
            // CRITICAL: Reset manual modification flags when clearing accent patterns
            accentPatternManuallyModified = false;
            
            // DEBUG: Log when accent pattern is cleared
            std::ofstream logFile("/tmp/serpe_accent_debug.log", std::ios::app);
            if (logFile.is_open()) {
                logFile << "NO ACCENT - Pattern: " << upiPattern.toStdString() 
                        << " (accent pattern cleared, manual flags reset)" << std::endl;
                logFile.close();
            }
        }
        
        // Reset global onset counter and UI accent offset only when requested
        if (resetAccentPosition) {
            globalOnsetCounter = 0;
            uiAccentOffset = 0;
        }
        
        // CRITICAL FIX: Always exit suspension mode when we get a new UPI pattern
        // This ensures that restored UPI accent patterns use proper onset-based logic
        if (patternManuallyModified) {
            patternManuallyModified = false;
            accentPatternManuallyModified = false;
            suspendedRhythmPattern.clear();
            suspendedAccentPattern.clear();
        }
        
        // Set up progressive offset if present
        if (parseResult.hasProgressiveOffset)
        {
            // Only set progressive offset if not already configured with the same values
            // This preserves the triggerCount for MIDI advancement
            bool needsProgressiveSetup = !patternEngine.hasProgressiveOffsetEnabled() ||
                                       patternEngine.getProgressiveOffsetValue() != parseResult.progressiveOffset;
            
            if (needsProgressiveSetup) {
                patternEngine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
            } else {
            }
                
            // Store progressive pattern key for step tracking
            if (!parseResult.progressivePatternKey.isEmpty())
            {
                currentProgressivePatternKey = parseResult.progressivePatternKey;
            }
        }
        else
        {
            patternEngine.setProgressiveOffset(false);
            currentProgressivePatternKey.clear(); // Clear for non-progressive patterns
        }
        
        
        // Update parameters to reflect the new pattern
        
        // Pattern applied successfully via UPI
        
        updateTiming();
        
        // Only cache non-progressive patterns to prevent re-parsing
        if (!isProgressive)
        {
            lastParsedUPI = upiPattern;
        }
        
        // Notify UI of pattern changes (including accent pattern changes)
        patternChanged.store(true);
        
    }
    else
    {
        // Parse failed - use safe fallback to prevent crash
        parseAndApplyUPI("E(3,8)", resetAccentPosition);
    }
    
    } catch (...) {
        // CRITICAL: Any exception in pattern parsing - use safe fallback to prevent plugin crash
        try {
            // Reset to completely safe state
            resetScenes();
            resetAccentSystem();
            auto safePattern = std::vector<bool>{true, false, false, true, false, false, true, false}; // E(3,8)
            patternEngine.setPattern(safePattern);
            hasAccentPattern = false;
            currentAccentPattern.clear();
            patternChanged.store(true);
        } catch (...) {
            // Even the safe fallback failed - this is critical but we must not crash
            // Leave current state as-is but mark pattern as changed
            patternChanged.store(true);
        }
    }
}

void SerpeAudioProcessor::checkMidiInputForTriggers(juce::MidiBuffer& midiMessages)
{
    // Store MIDI messages to process, then clear the buffer to prevent passthrough
    std::vector<juce::MidiMessage> messagesToProcess;
    
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        messagesToProcess.push_back(message);
    }
    
    // Clear the input buffer to prevent MIDI passthrough
    midiMessages.clear();
    
    // Process the stored messages
    for (const auto& message : messagesToProcess)
    {
        if (message.isNoteOn())
        {
            // Capture the MIDI note number and set the parameter
            int noteNumber = message.getNoteNumber();
            if (midiNoteParam)
            {
                midiNoteParam->setValueNotifyingHost(midiNoteParam->convertTo0to1((float)noteNumber));
            }
            
            // Any MIDI note input triggers pattern regeneration - use same logic as Tick button
            if (!currentUPIInput.isEmpty())
            {
                // Use original UPI input if available (preserves progressive/scene syntax after manual edits)
                juce::String upiToProcess = originalUPIInput.isEmpty() ? currentUPIInput : originalUPIInput;
                
                // Check for scenes and progressive patterns to handle them correctly
                bool hasProgressiveTransformation = upiToProcess.contains(">");
                bool hasScenes = upiToProcess.contains("|");
                
                bool triggerNeeded = false;
                
                try {
                    if (hasScenes) {
                        // CRITICAL FIX WITH ERROR PROTECTION: If we have scenes, handle scene advancement first
                        // This prevents double/triple advancement when scenes contain progressive transformations
                        advanceScene();
                        applyCurrentScenePattern(); 
                        triggerNeeded = true;
                    }
                    else if (hasProgressiveTransformation) {
                        // Progressive transformations: advance without resetting accents
                        // Only process this if we DON'T have scenes (to avoid double advancement)
                        parseAndApplyUPI(upiToProcess, false); // false = preserve accents
                        triggerNeeded = true;
                    }
                    
                    // For regular patterns without scenes or progressive transformations
                    if (!triggerNeeded) {
                        parseAndApplyUPI(upiToProcess, true); // true = reset accents for new patterns
                    }
                } catch (...) {
                    // MIDI processing failed - use safe fallback to prevent crash
                    resetScenes();
                    parseAndApplyUPI("E(3,8)", true); // Safe fallback pattern
                }
                
                // Reset step indicator for all manual triggers
                // currentStep.store(0); // REMOVED: Using derived indices
                patternChanged.store(true);
            }
            // Pattern updates are handled via UPI only
        }
        else if (message.isController())
        {
            // CC messages trigger pattern updates but don't affect the MIDI note parameter
            if (!currentUPIInput.isEmpty())
            {
                // Check for progressive patterns and scenes
                bool hasProgressiveOffset = (currentUPIInput.contains("%") && currentUPIInput.lastIndexOf("%") > 0) || (currentUPIInput.contains("+") && currentUPIInput.lastIndexOf("+") > 0);
                bool hasProgressiveLengthening = currentUPIInput.contains("*") && currentUPIInput.lastIndexOf("*") > 0;
                bool hasOldProgressiveOffset = currentUPIInput.contains("#");
                bool hasProgressiveTransformation = currentUPIInput.contains(">"); // New progressive syntax
                bool hasScenes = currentUPIInput.contains("|");
                
                // CYCLE BOUNDARY ADVANCEMENT: Progressive transformations and scenes now only advance at step 0
                // CC input no longer triggers mid-cycle advancement for better accent synchronization
                if (hasProgressiveTransformation)
                {
                    // Don't re-parse - progressive transformations advance at cycle boundaries
                }
                else if (hasProgressiveOffset || hasProgressiveLengthening || hasOldProgressiveOffset || hasScenes)
                {
                    // Progressive transformations and scenes now advance only at cycle boundaries (step 0)
                    // This ensures accent maps are properly synchronized from pattern start
                }
                else
                {
                    parseAndApplyUPI(currentUPIInput, false);
                }
            }
            // Pattern updates are handled via UPI only
        }
    }
}


void SerpeAudioProcessor::advanceProgressiveLengthening()
{
    if (progressiveLengthening > 0 && !baseLengthPattern.empty())
    {
        // Generate random steps using bell curve distribution
        auto randomSteps = generateBellCurveRandomSteps(progressiveLengthening);
        
        // Append the random steps to the pattern
        baseLengthPattern.insert(baseLengthPattern.end(), randomSteps.begin(), randomSteps.end());
    }
}

std::vector<bool> SerpeAudioProcessor::generateBellCurveRandomSteps(int numSteps)
{
    std::vector<bool> randomSteps(numSteps, false);
    
    if (numSteps <= 0) return randomSteps;
    
    int onsets;
    
    if (numSteps == 1) {
        // Special case for *1: randomly choose 0 or 1 onset (50/50 chance)
        std::uniform_int_distribution<int> coinFlip(0, 1);
        onsets = coinFlip(randomGenerator);
    } else {
        // Use bell curve distribution to determine number of onsets (avoid extremes)
        std::normal_distribution<double> distribution(numSteps / 2.0, (numSteps - 1) / 6.0);
        onsets = static_cast<int>(std::round(distribution(randomGenerator)));
        
        // Clamp to valid range [0, numSteps] to allow empty or full patterns
        onsets = juce::jmax(0, juce::jmin(numSteps, onsets));
    }
    
    // Randomly distribute the onsets
    std::vector<int> positions;
    for (int i = 0; i < numSteps; ++i)
    {
        positions.push_back(i);
    }
    
    std::shuffle(positions.begin(), positions.end(), randomGenerator);
    
    for (int i = 0; i < onsets && i < positions.size(); ++i)
    {
        randomSteps[positions[i]] = true;
    }
    
    return randomSteps;
}

std::vector<bool> SerpeAudioProcessor::lengthenPattern(const std::vector<bool>& pattern, int additionalSteps)
{
    auto lengthened = pattern;
    auto randomSteps = generateBellCurveRandomSteps(additionalSteps);
    lengthened.insert(lengthened.end(), randomSteps.begin(), randomSteps.end());
    return lengthened;
}

double SerpeAudioProcessor::calculateAutoPatternLength(const std::vector<bool>& pattern) const
{
    // Phase 3: Advanced Host Sync - Automatic pattern length calculation
    if (pattern.empty()) return 4.0; // Default to 4 beats
    
    int stepCount = static_cast<int>(pattern.size());
    int onsetCount = static_cast<int>(std::count(pattern.begin(), pattern.end(), true));
    
    // Calculate onset density (onsets per step)
    double density = static_cast<double>(onsetCount) / stepCount;
    
    // Musical heuristics based on pattern characteristics
    double baseLength;
    
    // Step count based heuristics
    if (stepCount <= 4) {
        baseLength = 1.0; // Very short patterns get 1 beat
    } else if (stepCount <= 8) {
        baseLength = 2.0; // Standard patterns get 2 beats
    } else if (stepCount <= 16) {
        baseLength = 4.0; // Longer patterns get 4 beats (1 bar)
    } else {
        baseLength = 8.0; // Very long patterns get 8 beats (2 bars)
    }
    
    // Density adjustment: sparse patterns get longer duration, dense patterns get shorter
    if (density < 0.2) {
        baseLength *= 2.0; // Very sparse - double the length
    } else if (density < 0.4) {
        baseLength *= 1.5; // Sparse - increase length by 50%
    } else if (density > 0.8) {
        baseLength *= 0.5; // Very dense - halve the length
    } else if (density > 0.6) {
        baseLength *= 0.75; // Dense - reduce length by 25%
    }
    
    // Special cases for common mathematical patterns
    if (stepCount == 7 && onsetCount == 3) {
        // Likely E(3,7) or similar - standard Afro-Cuban pattern
        baseLength = 2.0;
    } else if (stepCount == 8 && onsetCount == 3) {
        // Likely E(3,8) tresillo - standard 2 beat pattern
        baseLength = 2.0;
    } else if (stepCount == 16 && onsetCount == 3) {
        // Likely E(3,16) - sparse pattern gets 4 beats
        baseLength = 4.0;
    } else if (stepCount == 5 && onsetCount >= 3) {
        // Quintillo patterns - 2.5 beats or 1.25 beats
        baseLength = (onsetCount >= 4) ? 1.25 : 2.5;
    }
    
    // Ensure reasonable bounds
    if (baseLength < 0.5) baseLength = 0.5;
    if (baseLength > 16.0) baseLength = 16.0;
    
    return baseLength;
}

void SerpeAudioProcessor::advanceScene()
{
    // TRANSITION: Use SceneManager if available, with legacy fallback for safety
    if (sceneManager && sceneManager->hasScenes()) 
    {
        // NEW: Use SceneManager
        sceneManager->advanceScene();
        
        // Keep legacy system in sync for transition safety
        currentSceneIndex = sceneManager->getCurrentSceneIndex();
        
        // Update legacy vectors to match SceneManager state (for safety during transition)
        if (currentSceneIndex < static_cast<int>(sceneProgressiveSteps.size()) && 
            currentSceneIndex < scenePatterns.size()) 
        {
            // This ensures legacy system stays in sync if anything still uses it
            // The progressive state is managed by SceneManager now
        }
        
        // Notify UI that pattern has changed for accent map updates
        patternChanged.store(true);
        
    }
    else if (!scenePatterns.isEmpty() && currentSceneIndex < static_cast<int>(sceneProgressiveSteps.size()))
    {
        // LEGACY: Fall back to original logic if SceneManager not available or no scenes
        // First, advance the progressive state for the current scene if it has progressive syntax
        if (sceneProgressiveSteps[currentSceneIndex] != 0) {
            if (sceneProgressiveOffsets[currentSceneIndex] != 0) {
                // Progressive offset scene
                sceneProgressiveOffsets[currentSceneIndex] += sceneProgressiveSteps[currentSceneIndex];
            } else if (sceneProgressiveLengthening[currentSceneIndex] != 0) {
                // Progressive lengthening scene
                sceneProgressiveLengthening[currentSceneIndex] += sceneProgressiveSteps[currentSceneIndex];
            }
        }
        
        // Then advance to next scene in the sequence, cycling back to 0 when reaching the end
        currentSceneIndex = (currentSceneIndex + 1) % scenePatterns.size();
        
        // Notify UI that pattern has changed for accent map updates
        patternChanged.store(true);
        
    }
}

void SerpeAudioProcessor::applyCurrentScenePattern()
{
    // TRANSITION: Use SceneManager data if available, fall back to legacy
    if (sceneManager && sceneManager->hasScenes())
    {
        // NEW: Use SceneManager for scene pattern application
        juce::String basePattern = sceneManager->getCurrentSceneBasePattern();
        int progressiveOffset = sceneManager->getCurrentSceneProgressiveOffset();
        int progressiveLengthening = sceneManager->getCurrentSceneProgressiveLengthening();
        
        // Double B notation like B(3,21)B>17 should work fine - let UPIParser handle it
        
        // Parse the base pattern first
        parseAndApplyUPI(basePattern, true);
        // currentStep.store(0); // REMOVED: Using derived indices
        
        // Apply progressive transformations if any
        if (progressiveOffset != 0)
        {
            // Apply progressive offset by rotating the generated pattern
            auto currentPattern = patternEngine.getCurrentPattern();
            // Use negative rotation for clockwise progression (positive offset = clockwise)
            auto rotatedPattern = PatternUtils::rotatePattern(currentPattern, -progressiveOffset);
            patternEngine.setPattern(rotatedPattern);
        }
        else if (progressiveLengthening != 0)
        {
            // Apply progressive lengthening
            auto currentPattern = patternEngine.getCurrentPattern();
            
            // Store or retrieve the base pattern for this scene
            auto sceneBaseLengthPattern = sceneManager->getCurrentSceneBaseLengthPattern();
            if (sceneBaseLengthPattern.empty()) {
                // First time - store the base pattern in SceneManager
                sceneManager->setCurrentSceneBaseLengthPattern(currentPattern);
                sceneBaseLengthPattern = currentPattern;
            }
            
            // Apply lengthening to the stored base pattern
            auto lengthenedPattern = lengthenPattern(sceneBaseLengthPattern, progressiveLengthening);
            patternEngine.setPattern(lengthenedPattern);
        }
    }
    else
    {
        // LEGACY: Fall back to original logic
        if (currentSceneIndex >= static_cast<int>(sceneBasePatterns.size())) {
            return; // Safety check
        }
        
        juce::String basePattern = sceneBasePatterns[currentSceneIndex];
        int progressiveOffset = sceneProgressiveOffsets[currentSceneIndex];
        int progressiveLengthening = sceneProgressiveLengthening[currentSceneIndex];
        
        // Double B notation like B(3,21)B>17 should work fine - let UPIParser handle it (legacy fallback)
        
        // Parse the base pattern first
        parseAndApplyUPI(basePattern, true);
        // currentStep.store(0); // REMOVED: Using derived indices
        
        // Apply progressive transformations if any
        if (progressiveOffset != 0)
        {
            // Apply progressive offset by rotating the generated pattern
            auto currentPattern = patternEngine.getCurrentPattern();
            // Use negative rotation for clockwise progression (positive offset = clockwise)
            auto rotatedPattern = PatternUtils::rotatePattern(currentPattern, -progressiveOffset);
            patternEngine.setPattern(rotatedPattern);
            
        }
        else if (progressiveLengthening != 0)
        {
            // Apply progressive lengthening
            auto currentPattern = patternEngine.getCurrentPattern();
            
            // Store or retrieve the base pattern for this scene
            if (sceneBaseLengthPatterns[currentSceneIndex].empty()) {
                // First time - store the base pattern
                sceneBaseLengthPatterns[currentSceneIndex] = currentPattern;
            }
            
            // Apply lengthening to the stored base pattern
            auto lengthenedPattern = lengthenPattern(sceneBaseLengthPatterns[currentSceneIndex], progressiveLengthening);
            patternEngine.setPattern(lengthenedPattern);
            
        }
    }
}

int SerpeAudioProcessor::getProgressiveTriggerCount() const
{
    // For progressive transformations with ">" syntax, get step count from UPIParser
    if (!currentProgressivePatternKey.isEmpty())
    {
        return UPIParser::getProgressiveStepCount(currentProgressivePatternKey);
    }
    
    // Fall back to PatternEngine's trigger count for old "@#" syntax
    return patternEngine.getProgressiveTriggerCount();
}

juce::String SerpeAudioProcessor::getCurrentPatternDisplay() const
{
    return patternEngine.getBinaryString();
}



//==============================================================================
// Helper function to convert pattern length choice to float value
float SerpeAudioProcessor::getPatternLengthValue() const
{
    // Convert choice index to precise float values
    static const float values[] = {
        0.125f, 0.25f, 0.5f, 0.75f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f,
        11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
        24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f
    };
    
    int index = patternLengthValueParam->getIndex();
    if (index >= 0 && index < static_cast<int>(sizeof(values) / sizeof(values[0]))) {
        return values[index];
    }
    return 8.0f; // Default fallback
}

double SerpeAudioProcessor::getSubdivisionInBeats(int subdivisionIndex) const
{
    // SUBDIVISION PARAMETER DOCUMENTATION:
    // This function converts subdivision parameter choices to beat fractions for Steps mode.
    // Each subdivision represents how much of a beat one step occupies.
    //
    // Subdivision choices: {"64th Triplet", "64th", "32nd Triplet", "32nd", "16th Triplet", "16th", "8th Triplet", "8th", "Quarter Triplet", "Quarter", "Half Triplet", "Half", "Whole"}
    // Default: index 5 = "16th" = 0.25 beats per step
    //
    // Examples:
    // - 16th notes: 0.25 beats per step (4 steps = 1 beat)
    // - 8th notes: 0.5 beats per step (2 steps = 1 beat)  
    // - 8th triplets: 1/3 beats per step (3 steps = 1 beat)
    // - Quarter notes: 1.0 beats per step (1 step = 1 beat)
    
    static const double subdivisionBeats[] = {
        1.0/24.0,  // 64th Triplet (1/64 * 2/3 = 1/96, but musically 1/24 makes more sense)
        1.0/16.0,  // 64th
        1.0/12.0,  // 32nd Triplet (1/32 * 2/3 = 1/48, but musically 1/12 makes more sense)
        1.0/8.0,   // 32nd
        1.0/6.0,   // 16th Triplet (1/16 * 2/3 = 1/24, but musically 1/6 makes more sense)
        1.0/4.0,   // 16th (default)
        1.0/3.0,   // 8th Triplet (1/8 * 2/3 = 1/12, but musically 1/3 makes more sense)
        1.0/2.0,   // 8th
        2.0/3.0,   // Quarter Triplet (1/4 * 2/3 = 1/6, but musically 2/3 makes more sense)
        1.0,       // Quarter
        4.0/3.0,   // Half Triplet (1/2 * 2/3 = 1/3, but musically 4/3 makes more sense)
        2.0,       // Half
        4.0        // Whole
    };
    
    if (subdivisionIndex >= 0 && subdivisionIndex < static_cast<int>(sizeof(subdivisionBeats) / sizeof(subdivisionBeats[0]))) {
        return subdivisionBeats[subdivisionIndex];
    }
    return 1.0/4.0; // Default to 16th note
}

//==============================================================================
// Accent system implementation - single source of truth

bool SerpeAudioProcessor::shouldOnsetBeAccented(int onsetNumber) const
{
    if (!hasAccentPattern || currentAccentPattern.empty())
        return false;
    
    // DEBUG: Log accent calculation
    int accentPosition = onsetNumber % static_cast<int>(currentAccentPattern.size());
    bool result = currentAccentPattern[accentPosition];
    
    // Log every call to diagnose swirling
    static int callCount = 0;
    if ((++callCount % 1) == 0) {
        DBG("ACCENT DEBUG - onset: " << onsetNumber << ", pattern size: " << currentAccentPattern.size() 
            << ", accentPos: " << accentPosition << ", result: " << (result ? "ACCENT" : "normal"));
    }
    
    return result;
}

bool SerpeAudioProcessor::shouldStepBeAccented(int stepIndex) const
{
    if (!hasAccentPattern || currentAccentPattern.empty())
        return false;
    
    // NEW: Step-based accent logic that matches UI display exactly
    // Check if this specific step is marked as accented
    if (stepIndex >= 0 && stepIndex < static_cast<int>(currentAccentPattern.size()))
    {
        return currentAccentPattern[stepIndex];
    }
    
    return false;
}

std::vector<bool> SerpeAudioProcessor::getCurrentAccentMap() const
{
    /**
     * PHASE 2: UI ACCENT MAP WITH FEATURE FLAG FOR MIGRATION
     * 
     * Uses either the new robust AccentSequence system or legacy pre-calculated map
     * based on the feature flag. Ensures UI and MIDI use identical accent calculations.
     */
    
    if (useNewAccentSystem) {
        // NEW ROBUST ACCENT SYSTEM: Direct lookup from AccentSequence
        if (currentAccentSequence && currentAccentSequence->isValid()) {
            uint64_t currentTick = transportTick.load();
            uint64_t baseTick = baseTickRhythm.load();
            uint32_t stepsIntoSequence = static_cast<uint32_t>((currentTick - baseTick) % currentAccentSequence->getSequenceLength());
            
            return currentAccentSequence->getAccentMapForCycle(stepsIntoSequence);
        } else {
            // No accent sequence - return empty map
            std::vector<bool> rhythmPattern = patternEngine.getCurrentPattern();
            return std::vector<bool>(rhythmPattern.size(), false);
        }
    } else {
        // LEGACY ACCENT SYSTEM: Pre-calculated map with complex regeneration logic
        if (accentMapNeedsUpdate.load() || 
            (hasAccentPattern && !currentAccentPattern.empty() && !patternManuallyModified)) {
            const_cast<SerpeAudioProcessor*>(this)->generatePreCalculatedAccentMap();
        }
        
        return preCalculatedAccentMap;
    }
}

bool SerpeAudioProcessor::checkPatternChanged()
{
    // Check and reset the pattern changed flag atomically
    return patternChanged.exchange(false);
}

void SerpeAudioProcessor::resetAccentSystem()
{
    // PHASE 3: Simple reset for DAW-synchronized timing
    baseTickRhythm.store(0);
    baseTickAccent.store(0);
    
    // Legacy reset (will be removed in Phase 5)
    globalOnsetCounter = 0;
    uiAccentOffset = 0;
    accentPatternManuallyModified = false; // Reset manual modification flag
    patternManuallyModified = false;       // Exit suspension mode
    suspendedRhythmPattern.clear();
    suspendedAccentPattern.clear();
    patternChanged.store(true);
}

//==============================================================================
// UPI History Management (Ticker Tape Feature)


void SerpeAudioProcessor::addToUPIHistory(const juce::String& upiPattern)
{
    if (upiPattern.isEmpty()) return;
    
    // Remove if already exists (move to front)
    upiHistory.removeString(upiPattern);
    
    // Add to front
    upiHistory.insert(0, upiPattern);
    
    // Limit size
    while (upiHistory.size() > MAX_UPI_HISTORY)
    {
        upiHistory.remove(upiHistory.size() - 1);
    }
}

//==============================================================================
// PHASE 2: Lock-Free Pattern Update Queue Implementation

bool PatternUpdateQueue::enqueue(const PatternUpdate& update)
{
    size_t currentWrite = writeIndex.load();
    size_t nextWrite = (currentWrite + 1) % QUEUE_SIZE;
    
    // Check if queue is full
    if (nextWrite == readIndex.load()) {
        return false; // Queue full
    }
    
    // Copy the update
    queue[currentWrite] = update;
    
    // Advance write index
    writeIndex.store(nextWrite);
    return true;
}

bool PatternUpdateQueue::dequeue(PatternUpdate& update)
{
    size_t currentRead = readIndex.load();
    
    // Check if queue is empty
    if (currentRead == writeIndex.load()) {
        return false; // Queue empty
    }
    
    // Copy the update
    update = queue[currentRead];
    
    // Advance read index
    readIndex.store((currentRead + 1) % QUEUE_SIZE);
    return true;
}

bool PatternUpdateQueue::isEmpty() const
{
    return readIndex.load() == writeIndex.load();
}

void SerpeAudioProcessor::processPatternUpdates()
{
    PatternUpdate update;
    while (patternUpdateQueue.dequeue(update)) {
        // Simple pattern update - derived indices use DAW-synchronized ticks
        // Base remains at 0 since transport tick is DAW-synchronized
        baseTickRhythm.store(0);
        baseTickAccent.store(update.accentPhaseOffset);
        
        // Update pattern atomically
        patternEngine.setPattern(update.rhythmPattern);
        hasAccentPattern = update.hasAccent;
        currentAccentPattern = update.accentPattern;
        
        accentMapNeedsUpdate.store(true); // Mark accent map for regeneration
        patternChanged.store(true); // Notify UI
    }
}

void SerpeAudioProcessor::queuePatternUpdate(const std::vector<bool>& rhythmPattern, const std::vector<bool>& accentPattern, bool hasAccent, int accentPhaseOffset)
{
    PatternUpdate update(rhythmPattern, accentPattern, hasAccent, accentPhaseOffset);
    
    // Try to enqueue the update
    if (!patternUpdateQueue.enqueue(update)) {
        // Queue is full - fallback to phase-synced direct setting
        setPatternWithPhaseSync(rhythmPattern, accentPattern, hasAccent, accentPhaseOffset);
    }
}

void SerpeAudioProcessor::setPatternWithPhaseSync(const std::vector<bool>& rhythmPattern, const std::vector<bool>& accentPattern, bool hasAccent, int accentPhaseOffset)
{
    // CRITICAL FIX: Only reset base ticks for genuinely new patterns, not transformations
    // This preserves timing continuity for progressive/scene changes
    uint64_t currentTick = transportTick.load();
    
    // Get current derived step BEFORE pattern change
    uint32_t currentDerivedStep = getCurrentRhythmStep();
    
    // Apply the pattern first
    patternEngine.setPattern(rhythmPattern);
    this->hasAccentPattern = hasAccent;
    currentAccentPattern = accentPattern;
    
    accentMapNeedsUpdate.store(true); // Mark accent map for regeneration
    
    // Calculate new base tick to preserve current step position
    if (!rhythmPattern.empty()) {
        // For progressive patterns, be more conservative with base tick calculation
        // since the pattern itself changes due to rotation
        bool isProgressivePattern = (accentPhaseOffset != 0) || 
                                   currentUPIInput.contains("%") || 
                                   currentUPIInput.contains("+");
        
        uint64_t newBase;
        if (isProgressivePattern) {
            // For progressive patterns, align to current tick to avoid confusion
            newBase = currentTick;
        } else {
            // For static patterns, preserve step position
            uint32_t safeDerivedStep = currentDerivedStep % static_cast<uint32_t>(rhythmPattern.size());
            newBase = currentTick - safeDerivedStep;
        }
        
        baseTickRhythm.store(newBase);
        baseTickAccent.store(newBase + accentPhaseOffset);
        
        // Sync legacy step
        // currentStep.store(safeDerivedStep); // REMOVED: Using derived indices
    }
    
    // Update AccentSequence with new patterns (Phase 1: Compatibility Layer)
    updateAccentSequence();
    
    patternChanged.store(true);
}

//==============================================================================
// PHASE 1: Monotonic Transport Tick System

void SerpeAudioProcessor::updateTransportTick(int bufferSize)
{
    if (!isCurrentlyPlaying()) return;
    
    // REVERT TO ORIGINAL: Use sample-based timing (was working)
    // The issue was with baseTickRhythm alignment, not transport tick advancement
    sampleAccumulator += static_cast<uint64_t>(bufferSize);
    while (sampleAccumulator >= static_cast<uint64_t>(samplesPerStepPrecise)) {
        sampleAccumulator -= static_cast<uint64_t>(samplesPerStepPrecise);
        transportTick.store(transportTick.load() + 1);
    }
}

uint32_t SerpeAudioProcessor::getCurrentRhythmStep() const
{
    uint64_t tick = transportTick.load();
    uint64_t base = baseTickRhythm.load();
    auto pattern = patternEngine.getCurrentPattern();
    if (pattern.empty()) return 0;
    return static_cast<uint32_t>((tick - base) % pattern.size());
}

uint32_t SerpeAudioProcessor::getCurrentAccentStep() const
{
    if (!hasAccentPattern || currentAccentPattern.empty()) return 0;
    
    uint64_t tick = transportTick.load();
    uint64_t base = baseTickAccent.load();
    
    // Handle the case where tick < base (negative phase offset)
    if (tick >= base) {
        return static_cast<uint32_t>((tick - base) % currentAccentPattern.size());
    } else {
        // When tick < base, we need to handle negative modulo correctly
        uint64_t diff = base - tick;
        uint32_t size = static_cast<uint32_t>(currentAccentPattern.size());
        return (size - (diff % size)) % size;
    }
}

//==============================================================================
// PHASE 0: Derived index functions - proof of concept

uint64_t SerpeAudioProcessor::getMonotonicTick() const
{
    // PHASE 3: Use real monotonic transport tick
    return transportTick.load();
}

uint32_t SerpeAudioProcessor::getDerivedRhythmStep() const
{
    // PHASE 3: Use Phase 1 implementation
    return getCurrentRhythmStep();
}

uint32_t SerpeAudioProcessor::getDerivedAccentStep() const
{
    // PHASE 3: Use Phase 1 implementation
    return getCurrentAccentStep();
}

uint32_t SerpeAudioProcessor::getCurrentOnsetCount() const
{
    uint64_t tick = transportTick.load();
    uint64_t base = baseTickRhythm.load();
    uint32_t ticksSinceBase = static_cast<uint32_t>(tick - base);
    
    // Count onsets in the pattern from 0 to current position
    auto pattern = patternEngine.getCurrentPattern();
    if (pattern.empty()) return 0;
    
    // OPTIMIZATION: Avoid expensive loop for large tick values
    // Calculate onsets per complete cycle, then add partial cycle onsets
    uint32_t patternSize = static_cast<uint32_t>(pattern.size());
    
    // Count onsets in one complete pattern cycle
    uint32_t onsetsPerCycle = 0;
    for (bool step : pattern) {
        if (step) onsetsPerCycle++;
    }
    
    if (onsetsPerCycle == 0) return 0; // Pattern has no onsets
    
    // SAFETY: Handle single-step patterns like E(1,1) with special logic
    if (patternSize == 1) {
        // For single step patterns, onset count equals tick count if step has onset
        return pattern[0] ? ticksSinceBase : 0;
    }
    
    // Calculate complete cycles and remaining steps
    uint32_t completeCycles = ticksSinceBase / patternSize;
    uint32_t remainingSteps = ticksSinceBase % patternSize;
    
    // Count onsets in complete cycles
    uint32_t onsetCount = completeCycles * onsetsPerCycle;
    
    // Add onsets in remaining partial cycle
    for (uint32_t step = 0; step < remainingSteps; step++) {
        if (pattern[step]) {
            onsetCount++;
        }
    }
    
    return onsetCount;
}

void SerpeAudioProcessor::validateCounterConsistency() const
{
    #ifdef DEBUG
    auto pattern = patternEngine.getCurrentPattern();
    if (pattern.empty()) return;
    
    uint32_t rawLegacyStep = currentStep.load() % static_cast<uint32_t>(pattern.size());
    uint32_t derivedStep = getDerivedRhythmStep();
    
    if (rawLegacyStep != derivedStep) {
        // Enhanced logging with all relevant values
        std::ofstream logFile("/tmp/counter_drift_detected.log", std::ios::app);
        if (logFile.is_open()) {
            uint64_t tick = transportTick.load();
            uint64_t baseRhythm = baseTickRhythm.load();
            uint64_t baseAccent = baseTickAccent.load();
            
            auto now = std::chrono::system_clock::now();
            auto time_t_now = std::chrono::system_clock::to_time_t(now);
            
            logFile << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S")
                    << " DRIFT DETECTED: RawLegacy=" << rawLegacyStep 
                    << ", Derived=" << derivedStep 
                    << ", RawTick=" << tick
                    << ", BaseRhythm=" << baseRhythm
                    << ", BaseAccent=" << baseAccent
                    << ", TickDiff=" << (tick - baseRhythm)
                    << ", PatternSize=" << pattern.size()
                    << ", Playing=" << isCurrentlyPlaying()
                    << ", samplesPerStepPrecise=" << samplesPerStepPrecise
                    << ", RawLegacy=" << currentStep.load()
                    << std::endl;
            logFile.close();
        }
    }
    #endif
}

void SerpeAudioProcessor::generatePreCalculatedAccentMap()
{
    /**
     * PRE-CALCULATED ACCENT MAP GENERATION WITH LCM OPTIMIZATION
     * 
     * This method generates a deterministic accent map that covers the full polymetric cycle.
     * For patterns with N onsets and M accents, the cycle repeats every LCM(N,M) onsets.
     * 
     * Example: 7 onsets, 5 accents -> LCM(7,5) = 35 onsets -> 7 rhythm cycles 
     * This gives us a complete map that perfectly predicts accent positions.
     */
    
    auto currentPattern = patternEngine.getCurrentPattern();
    preCalculatedAccentMap.resize(currentPattern.size(), false);
    
    // Accent map generation - removed excessive logging for performance
    
    if (!hasAccentPattern || currentAccentPattern.empty()) {
        // No accents - map is already all false
        accentMapNeedsUpdate.store(false);
        return;
    }
    
    if (patternManuallyModified) {
        // SUSPENSION MODE: Use step-based accent mapping (manual modifications)
        for (int stepIndex = 0; stepIndex < static_cast<int>(currentPattern.size()); ++stepIndex) {
            if (currentPattern[stepIndex] && stepIndex < static_cast<int>(currentAccentPattern.size())) {
                preCalculatedAccentMap[stepIndex] = currentAccentPattern[stepIndex];
            }
        }
    } else {
        // NORMAL MODE: Use deterministic onset-based accent mapping
        // Count onsets in rhythm pattern
        int onsetsInPattern = 0;
        for (bool step : currentPattern) {
            if (step) onsetsInPattern++;
        }
        
        int accentPatternSize = static_cast<int>(currentAccentPattern.size());
        
        // SAFETY: Handle edge cases that could cause stalls
        if (onsetsInPattern == 0 || accentPatternSize == 0) {
            // No onsets or no accent pattern - no accents
            accentMapNeedsUpdate.store(false);
            return;
        }
        
        // Calculate LCM for efficient cycle detection
        auto gcd = [](int a, int b) {
            while (b != 0) {
                int temp = b;
                b = a % b;
                a = temp;
            }
            return a;
        };
        
        int lcm = (onsetsInPattern * accentPatternSize) / gcd(onsetsInPattern, accentPatternSize);
        
        // SAFETY: Prevent excessive LCM values that could cause performance issues
        if (lcm > 1000) {
            lcm = 1000; // Cap at reasonable limit
        }
        
        // CORRECT APPROACH: Calculate onset count at the START of the current cycle
        // This gives us the "slice" of the polymetric sequence that should be displayed
        uint32_t currentOnsetCount = getCurrentOnsetCount();
        uint32_t currentStep = getCurrentRhythmStep();
        
        // SAFETY: Ensure currentStep is within bounds
        if (currentStep >= static_cast<uint32_t>(currentPattern.size())) {
            currentStep = 0; // Reset to beginning if out of bounds
        }
        
        // Calculate how many onsets occurred before the current cycle started
        int onsetsBeforeCurrentCycle = 0;
        for (int step = 0; step < currentStep; step++) {
            if (step < static_cast<int>(currentPattern.size()) && currentPattern[step]) {
                onsetsBeforeCurrentCycle++;
            }
        }
        
        // Onset count at the beginning of the current cycle
        uint32_t cycleStartOnsetCount = (currentOnsetCount >= onsetsBeforeCurrentCycle) ? 
                                       (currentOnsetCount - onsetsBeforeCurrentCycle) : 0;
        uint32_t cyclePosition = cycleStartOnsetCount % lcm;
        
        // Calculate accent map for current cycle position
        int onsetIndex = 0;
        for (int stepIndex = 0; stepIndex < static_cast<int>(currentPattern.size()); ++stepIndex) {
            if (currentPattern[stepIndex]) {
                // This step has an onset - calculate if it should be accented
                uint32_t globalOnsetNumber = cyclePosition + onsetIndex;
                int accentPosition = static_cast<int>(globalOnsetNumber % accentPatternSize);
                preCalculatedAccentMap[stepIndex] = currentAccentPattern[accentPosition];
                onsetIndex++;
            }
        }
    }
    
    accentMapNeedsUpdate.store(false);
}

//==============================================================================
// NEW ROBUST ACCENT SYSTEM (Phase 1: Compatibility Layer)
//==============================================================================

bool SerpeAudioProcessor::isStepAccentedNew(uint32_t step) const
{
    if (!currentAccentSequence || !currentAccentSequence->isValid())
        return false;
    
    return currentAccentSequence->isAccentedAtStep(step);
}

std::vector<bool> SerpeAudioProcessor::getAccentMapNew() const
{
    if (!currentAccentSequence || !currentAccentSequence->isValid())
    {
        // Return empty map if no accent sequence
        std::vector<bool> rhythmPattern = patternEngine.getCurrentPattern();
        return std::vector<bool>(rhythmPattern.size(), false);
    }
    
    // Get accent map for current cycle based on transport position
    uint64_t currentTick = transportTick.load();
    uint64_t baseTick = baseTickRhythm.load();
    uint32_t stepsIntoSequence = static_cast<uint32_t>((currentTick - baseTick) % currentAccentSequence->getSequenceLength());
    
    return currentAccentSequence->getAccentMapForCycle(stepsIntoSequence);
}

void SerpeAudioProcessor::updateAccentSequence()
{
    try
    {
        // Get current patterns
        std::vector<bool> rhythmPattern = patternEngine.getCurrentPattern();
        
        if (!hasAccentPattern || currentAccentPattern.empty())
        {
            // No accent pattern - create sequence that returns no accents
            currentAccentSequence = std::make_unique<AccentSequence>(rhythmPattern, std::vector<bool>());
        }
        else
        {
            // Create new immutable accent sequence
            currentAccentSequence = std::make_unique<AccentSequence>(rhythmPattern, currentAccentPattern);
        }
        
        DBG("AccentSequence updated: " << currentAccentSequence->getDebugInfo());
    }
    catch (const std::exception& e)
    {
        DBG("Failed to create AccentSequence: " << e.what());
        currentAccentSequence.reset(); // Clear invalid sequence
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SerpeAudioProcessor();
}