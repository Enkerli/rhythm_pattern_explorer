/*
  ==============================================================================

    Rhythm Pattern Explorer - AUv3 Plugin
    Main Audio Processor Implementation (Fixed)

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UPIParser.h"
#include "PatternUtils.h"
#include "DebugConfig.h"
// Manager implementations now properly referenced from Source directory in Xcode project
#include <ctime>

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout RhythmPatternExplorerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Core parameters - only expose essential ones to host
    layout.add(std::make_unique<juce::AudioParameterBool>("useHostTransport", "Use Host Transport", true));
    layout.add(std::make_unique<juce::AudioParameterInt>("midiNote", "MIDI Note", 0, 127, 36));
    layout.add(std::make_unique<juce::AudioParameterBool>("tick", "Tick", false));
    
    // Pattern Length parameters for Phase 2 temporal control
    layout.add(std::make_unique<juce::AudioParameterChoice>("patternLengthUnit", "Pattern Length Unit", 
        juce::StringArray{"Steps", "Beats", "Bars"}, 1)); // Default to "Beats"
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

RhythmPatternExplorerAudioProcessor::RhythmPatternExplorerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters(*this, nullptr, "RhythmPatternExplorer", createParameterLayout()), randomGenerator(std::random_device{}())
#else
     : parameters(*this, nullptr, "RhythmPatternExplorer", createParameterLayout()), randomGenerator(std::random_device{}())
#endif
{
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
    
    // Install factory presets on first run
    presetManager.installFactoryPresets();
    
}


RhythmPatternExplorerAudioProcessor::~RhythmPatternExplorerAudioProcessor()
{
}

//==============================================================================
const juce::String RhythmPatternExplorerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RhythmPatternExplorerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RhythmPatternExplorerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RhythmPatternExplorerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RhythmPatternExplorerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RhythmPatternExplorerAudioProcessor::getNumPrograms()
{
    return 1;
}

int RhythmPatternExplorerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RhythmPatternExplorerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RhythmPatternExplorerAudioProcessor::getProgramName (int index)
{
    return "Default";
}

void RhythmPatternExplorerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RhythmPatternExplorerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
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

void RhythmPatternExplorerAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RhythmPatternExplorerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void RhythmPatternExplorerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Handle tick parameter (equivalent to pressing Parse)
    bool currentTickState = tickParam ? tickParam->get() : false;
    if (currentTickState && !lastTickState) {
        // Tick edge detected - advance scenes and progressive transformations like Enter key and MIDI input
        if (!currentUPIInput.isEmpty()) {
            // Use original UPI input if available (preserves progressive/scene syntax after manual edits)
            juce::String upiToProcess = originalUPIInput.isEmpty() ? currentUPIInput : originalUPIInput;
            
            // Check for scenes and progressive patterns to handle them correctly
            bool hasProgressiveTransformation = upiToProcess.contains(">");
            bool hasScenes = upiToProcess.contains("|");
            
            bool triggerNeeded = false;
            
            if (hasScenes) {
                // CRITICAL FIX: If we have scenes, handle scene advancement first
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
            
            // Reset step indicator for all manual triggers
            currentStep.store(0);
            patternChanged.store(true);
            
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
    
    // FIRST: Always log that processBlock is being called
    static int processBlockCallCount = 0;
    processBlockCallCount++;
    
    // Update last process time to indicate we're receiving audio callbacks
    lastProcessBlockTime = juce::Time::getMillisecondCounter();
    
    
    int currentBufferSize = buffer.getNumSamples();
    if (currentBufferSize <= 0) {
        return;
    }
    
    juce::ScopedLock lock(processingLock);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
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
        int targetStep = static_cast<int>(stepsInCurrentCycle);
        
        
        // Trigger notes at the exact moment they should occur
        int numSamples = buffer.getNumSamples();
        double samplesPerBeat = getSampleRate() * 60.0 / bpm;
        
        // Track the last processed step to detect boundaries
        static int lastProcessedStep = -1;
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Calculate the ppqPosition for this sample
            double sampleBeat = currentBeat + (sample / samplesPerBeat);
            double sampleStepsFromStart = sampleBeat / beatsPerStep;
            int patternStepCount = patternEngine.getStepCount();
            int sampleStep = static_cast<int>(sampleStepsFromStart) % patternStepCount;
            
            // For single-step patterns, also check pattern cycle boundaries
            int patternCycle = static_cast<int>(sampleStepsFromStart / patternStepCount);
            static int lastProcessedCycle = -1;
            
            // Check if we're crossing a step boundary OR pattern cycle boundary
            bool stepBoundary = (sampleStep != lastProcessedStep);
            bool cycleBoundary = (patternStepCount == 1 && patternCycle != lastProcessedCycle);
            
            if (stepBoundary || cycleBoundary)
            {
                // Update target step
                targetStep = sampleStep;
                lastProcessedStep = sampleStep;
                lastProcessedCycle = patternCycle;
                
                // Trigger the step at this exact sample position
                processStep(midiMessages, sample, targetStep);
                
                // Update current step
                currentStep.store(targetStep);
                
                
            }
        }
    }
    
    if (wasPlaying && !finalIsPlaying)
    {
        // Just stopped playing - reset position
        currentSample = 0;
        currentStep.store(0);
        // Stopped playing - resetting position
    }
    
    wasPlaying = finalIsPlaying;
}

//==============================================================================
bool RhythmPatternExplorerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* RhythmPatternExplorerAudioProcessor::createEditor()
{
    return new RhythmPatternExplorerAudioProcessorEditor (*this);
}

//==============================================================================
void RhythmPatternExplorerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
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

void RhythmPatternExplorerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
            for (int i = 0; i < patternString.length(); ++i)
            {
                pattern.push_back(patternString[i] == '1');
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
            for (int i = 0; i < baseLengthPatternString.length(); ++i)
            {
                baseLengthPattern.push_back(baseLengthPatternString[i] == '1');
            }
            
            // Restore accent pattern state (Phase 3: Missing accent support)
            hasAccentPattern = state.getProperty("hasAccentPattern", false);
            globalOnsetCounter = state.getProperty("globalOnsetCounter", 0);
            uiAccentOffset = state.getProperty("uiAccentOffset", 0);
            accentPatternManuallyModified = state.getProperty("accentPatternManuallyModified", false);
            
            // Restore currentAccentPattern from string
            juce::String accentPatternString = state.getProperty("currentAccentPattern", juce::String());
            currentAccentPattern.clear();
            for (int i = 0; i < accentPatternString.length(); ++i)
            {
                currentAccentPattern.push_back(accentPatternString[i] == '1');
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
            if (!currentUPIInput.isEmpty())
            {
                parseAndApplyUPI(currentUPIInput);
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
                for (int i = 0; i < patternString.length(); ++i)
                {
                    pattern.push_back(patternString[i] == '1');
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
            
            updateTiming();
        }
    }
}

//==============================================================================
void RhythmPatternExplorerAudioProcessor::updateTiming()
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
        
}

void RhythmPatternExplorerAudioProcessor::processStep(juce::MidiBuffer& midiBuffer, int samplePosition, int stepToProcess)
{
    auto pattern = patternEngine.getCurrentPattern();
    
    
    
    if (stepToProcess < pattern.size() && pattern[stepToProcess])
    {
        // This step has an onset - determine if it should be accented
        // Use appropriate accent logic based on suspension mode
        bool isAccented;
        if (patternManuallyModified) {
            // SUSPENSION MODE: Use step-based accent logic (manual modifications)
            isAccented = shouldStepBeAccented(stepToProcess);
        } else {
            // NORMAL MODE: Use onset-based accent logic (UPI patterns, progressive transformations)
            isAccented = shouldOnsetBeAccented(globalOnsetCounter);
        }
        
        // Trigger MIDI note
        triggerNote(midiBuffer, samplePosition, isAccented);
        
        // Advance the global onset counter (single source of truth)
        globalOnsetCounter++;
        
    }
    
    // Notify UI of cycle completion for pattern change updates
    int nextStep = (stepToProcess + 1) % static_cast<int>(pattern.size());
    if (nextStep == 0)
    {
        // Update stable UI accent offset at cycle boundaries (only if not manually modified)
        if (hasAccentPattern && !currentAccentPattern.empty() && !accentPatternManuallyModified)
        {
            uiAccentOffset = globalOnsetCounter % static_cast<int>(currentAccentPattern.size());
        }
        else if (hasAccentPattern && accentPatternManuallyModified)
        {
        }
        patternChanged.store(true); // UI can refresh at cycle boundaries
    }
}

void RhythmPatternExplorerAudioProcessor::triggerNote(juce::MidiBuffer& midiBuffer, int samplePosition, bool isAccented)
{
    // Get base MIDI note number from parameter
    int baseNoteNumber = midiNoteParam ? midiNoteParam->get() : 36; // Default kick drum note
    
    // Apply accent parameters if accented
    int noteNumber = baseNoteNumber;
    float velocity = 0.8f; // Default unaccented velocity
    
    if (isAccented && hasAccentPattern)
    {
        // Use accent parameters
        velocity = accentVelocityParam ? accentVelocityParam->get() : 1.0f;
        int pitchOffset = accentPitchOffsetParam ? accentPitchOffsetParam->get() : 5;
        noteNumber = baseNoteNumber + pitchOffset;
    }
    else
    {
        // Use unaccented velocity parameter
        velocity = unaccentedVelocityParam ? unaccentedVelocityParam->get() : 0.8f;
    }
    
    // Send MIDI note
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, noteNumber, velocity);
    juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, noteNumber, 0.0f);
    
    midiBuffer.addEvent(noteOn, samplePosition);
    midiBuffer.addEvent(noteOff, samplePosition + 100); // 100 samples note duration
    
    
    // MIDI effect mode - no audio synthesis
    
}

void RhythmPatternExplorerAudioProcessor::syncBPMWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
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

void RhythmPatternExplorerAudioProcessor::syncPositionWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // Phase 3: Advanced Host Sync - Loop point detection
    if (enableLoopSync && posInfo.isLooping)
    {
        bool wasLooping = hostIsLooping;
        hostIsLooping = true;
        
        // Detect loop restart (position jumped backward significantly)
        if (wasLooping && posInfo.ppqPosition < lastHostPosition - 0.1)
        {
            // Loop restarted - reset pattern timing and accents
            currentStep.store(0);
            currentSample = 0;
            globalOnsetCounter = 0;
            uiAccentOffset = 0;
            resetAccentSystem();
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
        int stepDifference = std::abs(targetStep - currentStep.load());
        if (allowPositionSync && stepDifference > 2)  // Increased threshold from 1 to 2
        {
            currentStep.store(targetStep);
            currentSample = static_cast<int>((stepsFromStart - std::floor(stepsFromStart)) * samplesPerStep);
        }
    }
    
    hostIsPlaying = posInfo.isPlaying;
    lastHostPosition = posInfo.ppqPosition;
}

void RhythmPatternExplorerAudioProcessor::setUPIInput(const juce::String& upiPattern)
{
    juce::ScopedLock lock(processingLock);
    
    // Add to history when pattern is entered (not when restored from state)
    addToUPIHistory(upiPattern.trim());
    
    // Check for progressive syntax: scenes first (pattern|pattern|pattern), then pattern+N (offset), pattern*N (lengthening)
    juce::String pattern = upiPattern.trim();
    
    // Store original UPI input if it contains progressive/scene syntax for later Tick/MIDI advancement
    bool hasProgressiveTransformation = pattern.contains(">");
    bool hasScenes = pattern.contains("|");
    if (hasProgressiveTransformation || hasScenes) {
        originalUPIInput = pattern;
    } else {
        originalUPIInput.clear(); // Clear if no progressive syntax
    }
    
    // Check if + is followed by a number (progressive offset) vs pattern (combination)
    bool isProgressiveOffset = false;
    if (!hasScenes && pattern.contains("+") && pattern.lastIndexOf("+") > 0) {
        int lastPlusIndex = pattern.lastIndexOf("+");
        juce::String afterPlus = pattern.substring(lastPlusIndex + 1).trim();
        // Progressive offset if what follows + is purely numeric (including negative numbers)
        isProgressiveOffset = afterPlus.containsOnly("0123456789-") && afterPlus.isNotEmpty();
    }
    
    bool isProgressiveLengthening = !hasScenes && pattern.contains("*") && pattern.lastIndexOf("*") > 0;
    
    if (hasScenes)
    {
        // Handle scene cycling: pattern1|pattern2|pattern3
        // CRITICAL: Split scenes BEFORE any UPI parsing to preserve individual accent patterns
        auto scenes = juce::StringArray::fromTokens(pattern, "|", "");
        
        // Log scene cycling detection
        juce::String sceneList;
        for (int i = 0; i < scenes.size(); ++i) {
            sceneList += juce::String::formatted("  Scene %d: %s\n", i, scenes[i].trim().toRawUTF8());
        }
        
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
                if (scenePattern.contains("+") && scenePattern.lastIndexOf("+") > 0) {
                    int lastPlusIndex = scenePattern.lastIndexOf("+");
                    juce::String afterPlus = scenePattern.substring(lastPlusIndex + 1).trim();
                    hasProgressiveOffset = afterPlus.containsOnly("0123456789");
                }
                bool hasProgressiveLengthening = scenePattern.contains("*") && scenePattern.lastIndexOf("*") > 0;
                
                if (hasProgressiveOffset) {
                    // Parse offset syntax: pattern+N
                    int plusIndex = scenePattern.lastIndexOf("+");
                    juce::String basePattern = scenePattern.substring(0, plusIndex).trim();
                    juce::String offsetStr = scenePattern.substring(plusIndex + 1).trim();
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
        // Handle progressive offset: pattern+N
        int plusIndex = pattern.lastIndexOf("+");
        juce::String newBasePattern = pattern.substring(0, plusIndex).trim();
        juce::String stepStr = pattern.substring(plusIndex + 1).trim();
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
        currentStep.store(0); // Reset step indicator to beginning
    }
    
    currentUPIInput = upiPattern;
}

//==============================================================================
// Interactive Pattern Modification Methods
//==============================================================================

void RhythmPatternExplorerAudioProcessor::togglePatternStep(int stepIndex)
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
        
        // Apply the modified pattern
        patternEngine.setPattern(currentPattern);
        
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
        
        // Apply the modified pattern
        patternEngine.setPattern(currentPattern);
        
        // Resize accent pattern to match new pattern size
        if (currentAccentPattern.size() != currentPattern.size()) {
            currentAccentPattern.resize(currentPattern.size(), false);
            suspendedAccentPattern = currentAccentPattern;
        }
    }
    
    // Update step indicator to current position for immediate feedback
    currentStep.store(stepIndex);
    
    // Update UPI display to reflect the manual modification
    updateUPIFromCurrentPattern();
    
    // Notify UI of pattern change
    patternChanged.store(true);
    
}

void RhythmPatternExplorerAudioProcessor::toggleAccentAtStep(int stepIndex)
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
        // Pattern size mismatch - resize and toggle
        currentAccentPattern.resize(currentPattern.size(), false);
        currentAccentPattern[stepIndex] = !currentAccentPattern[stepIndex];
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

bool RhythmPatternExplorerAudioProcessor::isValidStepIndex(int stepIndex) const
{
    auto currentPattern = patternEngine.getCurrentPattern();
    return stepIndex >= 0 && stepIndex < static_cast<int>(currentPattern.size());
}

void RhythmPatternExplorerAudioProcessor::updateUPIFromCurrentPattern()
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

void RhythmPatternExplorerAudioProcessor::parseAndApplyUPI(const juce::String& upiPattern, bool resetAccentPosition)
{
    if (upiPattern.isEmpty())
        return;
    
    
    
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
        
        // Apply the parsed pattern to the engine
        patternEngine.setPattern(parseResult.pattern);
        
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
        }
        else
        {
            hasAccentPattern = false;
            currentAccentPattern.clear();
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
    }
}

void RhythmPatternExplorerAudioProcessor::checkMidiInputForTriggers(juce::MidiBuffer& midiMessages)
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
                
                if (hasScenes) {
                    // CRITICAL FIX: If we have scenes, handle scene advancement first
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
                
                // Reset step indicator for all manual triggers
                currentStep.store(0);
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
                bool hasProgressiveOffset = currentUPIInput.contains("+") && currentUPIInput.lastIndexOf("+") > 0;
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


void RhythmPatternExplorerAudioProcessor::advanceProgressiveLengthening()
{
    if (progressiveLengthening > 0 && !baseLengthPattern.empty())
    {
        // Generate random steps using bell curve distribution
        auto randomSteps = generateBellCurveRandomSteps(progressiveLengthening);
        
        // Append the random steps to the pattern
        baseLengthPattern.insert(baseLengthPattern.end(), randomSteps.begin(), randomSteps.end());
    }
}

std::vector<bool> RhythmPatternExplorerAudioProcessor::generateBellCurveRandomSteps(int numSteps)
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

std::vector<bool> RhythmPatternExplorerAudioProcessor::lengthenPattern(const std::vector<bool>& pattern, int additionalSteps)
{
    auto lengthened = pattern;
    auto randomSteps = generateBellCurveRandomSteps(additionalSteps);
    lengthened.insert(lengthened.end(), randomSteps.begin(), randomSteps.end());
    return lengthened;
}

void RhythmPatternExplorerAudioProcessor::advanceScene()
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

void RhythmPatternExplorerAudioProcessor::applyCurrentScenePattern()
{
    // TRANSITION: Use SceneManager data if available, fall back to legacy
    if (sceneManager && sceneManager->hasScenes())
    {
        // NEW: Use SceneManager for scene pattern application
        juce::String basePattern = sceneManager->getCurrentSceneBasePattern();
        int progressiveOffset = sceneManager->getCurrentSceneProgressiveOffset();
        int progressiveLengthening = sceneManager->getCurrentSceneProgressiveLengthening();
        
        // Parse the base pattern first
        parseAndApplyUPI(basePattern, true);
        currentStep.store(0); // Reset step indicator to beginning
        
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
        
        // Parse the base pattern first
        parseAndApplyUPI(basePattern, true);
        currentStep.store(0); // Reset step indicator to beginning
        
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

int RhythmPatternExplorerAudioProcessor::getProgressiveTriggerCount() const
{
    // For progressive transformations with ">" syntax, get step count from UPIParser
    if (!currentProgressivePatternKey.isEmpty())
    {
        return UPIParser::getProgressiveStepCount(currentProgressivePatternKey);
    }
    
    // Fall back to PatternEngine's trigger count for old "@#" syntax
    return patternEngine.getProgressiveTriggerCount();
}



//==============================================================================
// Helper function to convert pattern length choice to float value
float RhythmPatternExplorerAudioProcessor::getPatternLengthValue() const
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

double RhythmPatternExplorerAudioProcessor::getSubdivisionInBeats(int subdivisionIndex) const
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

bool RhythmPatternExplorerAudioProcessor::shouldOnsetBeAccented(int onsetNumber) const
{
    if (!hasAccentPattern || currentAccentPattern.empty())
        return false;
    
    // DEPRECATED: onset-based logic (kept for compatibility during transition)
    int accentPosition = onsetNumber % static_cast<int>(currentAccentPattern.size());
    return currentAccentPattern[accentPosition];
}

bool RhythmPatternExplorerAudioProcessor::shouldStepBeAccented(int stepIndex) const
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

std::vector<bool> RhythmPatternExplorerAudioProcessor::getCurrentAccentMap() const
{
    // For UI visualization - use appropriate accent mapping based on suspension mode
    std::vector<bool> accentMap;
    auto currentPattern = patternEngine.getCurrentPattern();
    
    if (!hasAccentPattern || currentAccentPattern.empty())
    {
        // No accents - return all false
        accentMap.resize(currentPattern.size(), false);
        return accentMap;
    }
    
    accentMap.resize(currentPattern.size(), false);
    
    if (patternManuallyModified) {
        // SUSPENSION MODE: Use step-based accent mapping (manual modifications)
        // Accents appear exactly where the user clicked
        for (int stepIndex = 0; stepIndex < static_cast<int>(currentPattern.size()); ++stepIndex)
        {
            if (currentPattern[stepIndex] && stepIndex < static_cast<int>(currentAccentPattern.size()))
            {
                accentMap[stepIndex] = currentAccentPattern[stepIndex];
            }
        }
    } else {
        // NORMAL MODE: Use onset-based accent mapping (UPI patterns, progressive transformations)
        // Calculate which onsets will occur in this cycle and their accent status
        // When stopped, always start from position 0 to show accent pattern from beginning
        int onsetNumber = isCurrentlyPlaying() ? uiAccentOffset : 0;
        
        for (int stepInCycle = 0; stepInCycle < static_cast<int>(currentPattern.size()); ++stepInCycle)
        {
            if (currentPattern[stepInCycle])
            {
                // This step will have an onset - will it be accented?
                accentMap[stepInCycle] = shouldOnsetBeAccented(onsetNumber);
                onsetNumber++;
            }
            // else: No onset at this step, accentMap[stepInCycle] remains false
        }
    }
    
    return accentMap;
}

bool RhythmPatternExplorerAudioProcessor::checkPatternChanged()
{
    // Check and reset the pattern changed flag atomically
    return patternChanged.exchange(false);
}

void RhythmPatternExplorerAudioProcessor::resetAccentSystem()
{
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

void RhythmPatternExplorerAudioProcessor::addToUPIHistory(const juce::String& upiPattern)
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
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RhythmPatternExplorerAudioProcessor();
}