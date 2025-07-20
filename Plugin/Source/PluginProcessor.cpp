/*
  ==============================================================================

    Rhythm Pattern Explorer - AUv3 Plugin
    Main Audio Processor Implementation (Fixed)

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UPIParser.h"
#include <ctime>
#include <fstream>

// Debug output disabled for production performance
#ifdef DEBUG
    // Keep debug output in debug builds
#else
    #undef DBG
    #define DBG(textToWrite) do { } while (false)
#endif

//==============================================================================
RhythmPatternExplorerAudioProcessor::RhythmPatternExplorerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), randomGenerator(std::random_device{}())
#endif
{
    // Initialize parameters - only expose essential ones to host
    addParameter(useHostTransportParam = new juce::AudioParameterBool("useHostTransport", "Use Host Transport", true));
    addParameter(midiNoteParam = new juce::AudioParameterInt("midiNote", "MIDI Note", 0, 127, 36));
    addParameter(tickParam = new juce::AudioParameterBool("tick", "Tick", false));
    addParameter(accentPitchOffsetParam = new juce::AudioParameterInt("accentPitchOffset", "Accent Pitch Offset", -12, 12, 0));
    addParameter(accentVelocityParam = new juce::AudioParameterFloat("accentVelocity", "Accent Velocity", 0.0f, 1.0f, 1.0f));
    addParameter(unaccentedVelocityParam = new juce::AudioParameterFloat("unaccentedVelocity", "Unaccented Velocity", 0.0f, 1.0f, 0.69f));
    
    // Pattern Length parameters for Phase 2 temporal control
    addParameter(patternLengthUnitParam = new juce::AudioParameterChoice("patternLengthUnit", "Pattern Length Unit", 
        juce::StringArray{"Steps", "Beats", "Bars"}, 1)); // Default to "Beats"
    addParameter(patternLengthValueParam = new juce::AudioParameterChoice("patternLengthValue", "Pattern Length Value", 
        juce::StringArray{"0.125", "0.25", "0.5", "0.75", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32"}, 7)); // Default to "8"
    
    // Subdivision parameter for transport subdivision matching
    addParameter(subdivisionParam = new juce::AudioParameterChoice("subdivision", "Subdivision", 
        juce::StringArray{"64th Triplet", "64th", "32nd Triplet", "32nd", "16th Triplet", "16th", "8th Triplet", "8th", "Quarter Triplet", "Quarter", "Half Triplet", "Half", "Whole"}, 5)); // Default to "16th"
    
    // Initialize pattern engine with default Euclidean pattern
    patternEngine.generateEuclideanPattern(3, 8);
    
    // Set up progressive offset engine for UPI parser
    UPIParser::setProgressiveOffsetEngine(&patternEngine);
    
    DBG("RhythmPatternExplorer: Plugin initialized");
    
    // Debug logging for plugin initialization
    logDebug(DebugCategory::BITWIG_INIT, "Plugin constructor called! Debug version v0.03a4.DBG active.");
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
    globalAccentPosition = 0;
    wasPlaying = false;
    
    // Force early initialization to work around Logic Pro loading order issues
    lastProcessBlockTime = juce::Time::getMillisecondCounter();
    
    updateTiming();
    
    DBG("RhythmPatternExplorer: Prepared to play at " << sampleRate << " Hz");
}

void RhythmPatternExplorerAudioProcessor::releaseResources()
{
    DBG("RhythmPatternExplorer: Released resources");
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
        // Tick edge detected - reparse current UPI pattern
        DBG("RhythmPatternExplorer: Tick parameter activated! UPI input: '" << currentUPIInput << "'");
        if (!currentUPIInput.isEmpty()) {
            setUPIInput(currentUPIInput);
            DBG("RhythmPatternExplorer: Pattern reparsed via tick");
        } else {
            DBG("RhythmPatternExplorer: No UPI pattern to tick - currentUPIInput is empty");
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
    
    // BITWIG 210 BPM DEBUG: Focus on the specific problem
    static int debugCallCount = 0;
    static int lastLoggedBPM = 0;
    
    int currentBufferSize = buffer.getNumSamples();
    if (currentBufferSize <= 0) {
        return;
    }
    
    // Log every 50 calls or when BPM changes, with focus on 200+ BPM
    debugCallCount++;
    int currentBPMInt = static_cast<int>(currentBPM);
    
    // More frequent logging at high BPMs
    bool shouldLog = false;
    if (currentBPM >= 200) {
        shouldLog = (debugCallCount % 25 == 0); // Every 25 calls at high BPM
    } else {
        shouldLog = (debugCallCount % 100 == 0); // Every 100 calls normally
    }
    
    if (shouldLog || std::abs(currentBPM - lastLoggedBPM) > 2) {
        juce::String message = juce::String::formatted("BPM=%.1f, SamplesPerStep=%d, BufferSize=%d, CurrentSample=%d, CurrentStep=%d, SampleRate=%.0f, CallCount=%d",
            currentBPM, samplesPerStep, currentBufferSize, currentSample, currentStep.load(), currentSampleRate, debugCallCount);
        logDebug(DebugCategory::BITWIG_PROCESS, message);
        lastLoggedBPM = currentBPM;
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
    juce::AudioPlayHead::CurrentPositionInfo posInfo;
    bool hasValidPosition = false;
    
    if (playHead != nullptr)
    {
        hasValidPosition = playHead->getCurrentPosition(posInfo);
        
        // BITWIG TRANSPORT DEBUG: Log transport details
        // Use internal currentBPM variable
        if (currentBPM >= 200.0f) {
            static int transportDebugCount = 0;
            if (++transportDebugCount % 100 == 0) {
                juce::String message = juce::String::formatted("hasValidPosition=%s, isPlaying=%s, isRecording=%s, ppqPosition=%.3f, bpm=%.1f",
                    hasValidPosition ? "TRUE" : "FALSE",
                    posInfo.isPlaying ? "TRUE" : "FALSE", 
                    posInfo.isRecording ? "TRUE" : "FALSE",
                    posInfo.ppqPosition, posInfo.bpm);
                logDebug(DebugCategory::TRANSPORT, message);
            }
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
                logDebug(DebugCategory::TRANSPORT, "Bitwig not providing transport info");
            }
        }
    }

    // Determine if we should be playing
    bool isPlaying;
    if (useHostTransportParam && useHostTransportParam->get() && hasValidPosition)
    {
        // FIXED: Manual play button should work alongside host transport (OR logic)
        isPlaying = posInfo.isPlaying || internalPlaying;
        
        // BITWIG FIX: Always sync BPM, but conditionally sync position
        syncBPMWithHost(posInfo);
        syncPositionWithHost(posInfo);
    }
    else
    {
        isPlaying = internalPlaying;
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
        
        // Log BPM changes for debugging
        if (bpmChanged && currentBPM >= 180.0f) {
            juce::String message = juce::String::formatted("%.1f->%.1f, sampleRatio=%.3f, newCurrentSample=%d, newSamplesPerStep=%d",
                lastBPM, currentBPM, sampleRatio, currentSample, samplesPerStep);
            logDebug(DebugCategory::BPM_SYNC, message);
        }
    }
    
    // Always update lastBPM outside the if block
    if (bpmChanged) {
        lastBPM = currentBPM;
    }
    
    // Clean production version - no aggressive debugging

    // BITWIG PLAYING STATE DEBUG: Log playing detection
    if (currentBPM >= 180.0f) {
        static int playingDebugCount = 0;
        if (++playingDebugCount % 50 == 0) {
            logDebug(DebugCategory::BITWIG_PROCESS, 
                "PLAYING DEBUG: isPlaying=" + juce::String(isPlaying ? "TRUE" : "FALSE") +
                ", hostIsPlaying=" + juce::String(hostIsPlaying ? "TRUE" : "FALSE") + 
                ", internalPlaying=" + juce::String(internalPlaying ? "TRUE" : "FALSE") +
                ", useHostTransport=" + juce::String(useHostTransportParam->get() ? "TRUE" : "FALSE"));
        }
    }

    // FIRST STEP FIX: Track if we've processed step 0 in this buffer
    static bool processedFirstStepThisBuffer = false;
    if (!isPlaying) processedFirstStepThisBuffer = false; // Reset when not playing

    // TRANSPORT-SYNCED TIMING: Use DAW's ppqPosition for perfect alignment
    if (isPlaying && hasValidPosition)
    {
        // Calculate pattern timing based on DAW transport and pattern length parameters
        int lengthUnit = patternLengthUnitParam->getIndex(); // 0=Steps, 1=Beats, 2=Bars
        float lengthValue = getPatternLengthValue();
        
        // Calculate pattern length in beats using pattern length parameters
        double patternLengthInBeats;
        switch (lengthUnit) {
            case 0: // Steps mode - use 16th note subdivisions
                patternLengthInBeats = lengthValue / 4.0; // Steps are 16th notes
                break;
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
        double currentBeat = posInfo.ppqPosition;
        double stepsFromStart = currentBeat / beatsPerStep;
        
        // Calculate which step should be active
        int targetStep = static_cast<int>(stepsFromStart) % patternEngine.getStepCount();
        
        // Calculate the exact fractional position within the current step
        double stepFraction = stepsFromStart - std::floor(stepsFromStart);
        
        // Trigger notes at the exact moment they should occur
        int numSamples = buffer.getNumSamples();
        double samplesPerBeat = getSampleRate() * 60.0 / posInfo.bpm;
        double samplesPerStep = samplesPerBeat * beatsPerStep;
        
        // Track the last processed step to detect boundaries
        static int lastProcessedStep = -1;
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Calculate the ppqPosition for this sample
            double sampleBeat = currentBeat + (sample / samplesPerBeat);
            double sampleStepsFromStart = sampleBeat / beatsPerStep;
            int sampleStep = static_cast<int>(sampleStepsFromStart) % patternEngine.getStepCount();
            
            // Check if we're crossing a step boundary (only trigger once per step)
            if (sampleStep != lastProcessedStep)
            {
                // Update target step
                targetStep = sampleStep;
                lastProcessedStep = sampleStep;
                
                // Trigger the step at this exact sample position
                processStep(midiMessages, sample, targetStep);
                
                // Update current step
                currentStep.store(targetStep);
                
                // Handle accent pattern updates at cycle boundaries
                if (targetStep == 0 && hasAccentPattern) {
                    uiAccentOffset = globalAccentPosition % currentAccentPattern.size();
                    patternChanged.store(true);
                    DBG("RhythmPatternExplorer: Cycle boundary - UI accent offset set to " << uiAccentOffset);
                }
                
                // DEBUG: Log transport-synced step progression
                {
                    std::ofstream debugFile("/tmp/rhythm_timing_debug.txt", std::ios::app);
                    debugFile << "TRANSPORT-SYNCED STEP: triggeredStep=" << targetStep 
                              << ", sampleBeat=" << sampleBeat
                              << ", sampleOffset=" << sample
                              << ", ppqPosition=" << posInfo.ppqPosition
                              << ", beatsPerStep=" << beatsPerStep << std::endl;
                    debugFile.close();
                }
            }
        }
    }
    
    if (wasPlaying && !isPlaying)
    {
        // Just stopped playing - reset position
        currentSample = 0;
        currentStep.store(0);
        globalAccentPosition = 0;
        // Stopped playing - resetting position
    }
    
    wasPlaying = isPlaying;
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
    // Create an XML representation of our state
    auto xml = std::make_unique<juce::XmlElement>("RhythmPatternExplorerState");
    
    xml->setAttribute("bpm", currentBPM);
    xml->setAttribute("playing", internalPlaying);
    xml->setAttribute("useHostTransport", useHostTransportParam->get());
    xml->setAttribute("midiNote", midiNoteParam->get());
    
    // Save pattern data
    auto patternXml = xml->createNewChildElement("Pattern");
    auto pattern = patternEngine.getCurrentPattern();
    juce::String patternString;
    for (int i = 0; i < pattern.size(); ++i)
    {
        patternString += pattern[i] ? "1" : "0";
    }
    patternXml->setAttribute("data", patternString);
    
    // Save UPI input
    auto upiXml = xml->createNewChildElement("UPI");
    upiXml->setAttribute("input", currentUPIInput);
    
    copyXmlToBinary(*xml, destData);
}

void RhythmPatternExplorerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    
    if (xml != nullptr && xml->hasTagName("RhythmPatternExplorerState"))
    {
        currentBPM = xml->getDoubleAttribute("bpm", 120.0);
        internalPlaying = xml->getBoolAttribute("playing", false);
        *useHostTransportParam = xml->getBoolAttribute("useHostTransport", true);
        *midiNoteParam = xml->getIntAttribute("midiNote", 36);
        
        // Restore pattern data if available
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
        
        // Restore UPI input
        auto upiXml = xml->getChildByName("UPI");
        if (upiXml != nullptr)
        {
            currentUPIInput = upiXml->getStringAttribute("input");
        }
        
        updateTiming();
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
    
    // Check if we're in subdivision mode 
    // For now, use subdivision mode when pattern length unit is "Steps" (index 0)
    bool useSubdivisionMode = (lengthUnit == 0); // Steps mode uses subdivision parameter
    
    if (useSubdivisionMode) {
        // Subdivision mode: each step matches a transport subdivision
        auto pattern = patternEngine.getCurrentPattern();
        int patternSteps = static_cast<int>(pattern.size());
        if (patternSteps <= 0) patternSteps = 8; // Fallback
        
        // Convert subdivision index to beat fraction
        double subdivisionBeats = getSubdivisionInBeats(subdivisionIndex);
        patternLengthInBeats = subdivisionBeats * patternSteps;
    } else {
        // Pattern length modes
        switch (lengthUnit) {
            case 0: // Steps mode - use 16th note subdivisions (default behavior)
                patternLengthInBeats = lengthValue / 4.0; // Steps are 16th notes
                break;
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
    }
    
    // Calculate timing for entire pattern, then divide by number of steps
    auto pattern = patternEngine.getCurrentPattern();
    int patternSteps = static_cast<int>(pattern.size());
    if (patternSteps <= 0) patternSteps = 8; // Fallback
    
    double patternDurationInSeconds = patternLengthInBeats / beatsPerSecond;
    double stepDurationInSeconds = patternDurationInSeconds / patternSteps;
    samplesPerStep = static_cast<int>(currentSampleRate * stepDurationInSeconds);
    
    // Timing validation
    if (samplesPerStep <= 0) {
        samplesPerStep = static_cast<int>(currentSampleRate / 60.0); // Default to 1Hz fallback
    }
        
    DBG("RhythmPatternExplorer: Updated timing - BPM: " << bpm << ", Samples per step: " << samplesPerStep);
}

void RhythmPatternExplorerAudioProcessor::processStep(juce::MidiBuffer& midiBuffer, int samplePosition, int stepToProcess)
{
    auto pattern = patternEngine.getCurrentPattern();
    
    // BITWIG 210 DEBUG: Log pattern state at high BPMs
    // Use internal currentBPM variable
    static int stepCallCount = 0;
    
#ifdef DEBUG
    if (currentBPM >= 200.0f && ++stepCallCount % 3 == 0) {
        std::cout << "PROCESS STEP: BPM=" << currentBPM << 
            ", Step=" << stepToProcess << 
            ", PatternSize=" << pattern.size() << 
            ", HasOnset=" << (stepToProcess < pattern.size() && pattern[stepToProcess] ? "YES" : "NO") << 
            ", SamplePos=" << samplePosition << std::endl;
    }
#endif
    
    // DEBUG: Log pattern check
    {
        std::ofstream debugFile("/tmp/rhythm_timing_debug.txt", std::ios::app);
        debugFile << "PATTERN CHECK: step=" << stepToProcess 
                  << ", pattern[" << stepToProcess << "]=" << (stepToProcess < pattern.size() ? (pattern[stepToProcess] ? "1" : "0") : "OUT_OF_BOUNDS")
                  << ", willTriggerNote=" << (stepToProcess < pattern.size() && pattern[stepToProcess] ? "YES" : "NO");
        // Also log the complete pattern for verification
        debugFile << ", fullPattern=";
        for (size_t i = 0; i < pattern.size(); ++i) {
            debugFile << (pattern[i] ? "1" : "0");
        }
        debugFile << std::endl;
        debugFile.close();
    }
    
    if (stepToProcess < pattern.size() && pattern[stepToProcess])
    {
        // Check if this step should be accented
        bool shouldAccent = false;
        if (hasAccentPattern && !currentAccentPattern.empty())
        {
            // Use global accent position (persists across pattern cycles)
            int accentStep = globalAccentPosition % currentAccentPattern.size();
            shouldAccent = currentAccentPattern[accentStep];
            
            // Increment global accent position for next onset
            globalAccentPosition++;
            
            // Debug accent detection
            DBG("ACCENT DEBUG: Step=" << stepToProcess << 
                ", GlobalAccentPos=" << (globalAccentPosition - 1) <<
                ", AccentPatternSize=" << currentAccentPattern.size() <<
                ", AccentStep=" << accentStep <<
                ", ShouldAccent=" << (shouldAccent ? "YES" : "NO"));
        }
        else
        {
            DBG("ACCENT DEBUG: No accent pattern - hasAccentPattern=" << (hasAccentPattern ? "true" : "false") <<
                ", accentPatternSize=" << currentAccentPattern.size());
        }
        
        triggerNote(midiBuffer, samplePosition, shouldAccent);
        
#ifdef DEBUG
        // Log note triggers at high BPM
        if (currentBPM >= 200.0f) {
            std::cout << "NOTE TRIGGERED: Step=" << currentStep.load() << 
                ", BPM=" << currentBPM << 
                ", Accented=" << (shouldAccent ? "YES" : "NO") <<
                ", SamplePos=" << samplePosition << std::endl;
        }
#endif
    }
}

void RhythmPatternExplorerAudioProcessor::triggerNote(juce::MidiBuffer& midiBuffer, int samplePosition, bool isAccented)
{
    // Get base MIDI note number from parameter (set by incoming MIDI)
    int baseNoteNumber = midiNoteParam ? midiNoteParam->get() : 36; // Default kick drum note
    
    // Apply accent modulation based on parameters
    int noteNumber = baseNoteNumber;
    float velocity;
    
    if (isAccented)
    {
        // Apply accent pitch offset from parameter (default 0)
        int pitchOffset = accentPitchOffsetParam ? accentPitchOffsetParam->get() : 0;
        noteNumber = juce::jlimit(0, 127, baseNoteNumber + pitchOffset); // Clamped to MIDI range
        
        // Apply accent velocity from parameter (default 1.0)
        velocity = accentVelocityParam ? accentVelocityParam->get() : 1.0f;
    }
    else
    {
        // Apply unaccented velocity from parameter (default 0.69)
        velocity = unaccentedVelocityParam ? unaccentedVelocityParam->get() : 0.69f;
    }
    
    // Send MIDI note
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, noteNumber, velocity);
    juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, noteNumber, 0.0f);
    
    midiBuffer.addEvent(noteOn, samplePosition);
    midiBuffer.addEvent(noteOff, samplePosition + 100); // 100 samples note duration
    
    // MIDI effect mode - no audio synthesis
    
    DBG("RhythmPatternExplorer: " << (isAccented ? "ACCENTED " : "REGULAR ") << "Note " << noteNumber << 
        " (base: " << baseNoteNumber << ") triggered at step " << currentStep.load() << " with velocity " << velocity <<
        " (isAccented=" << (isAccented ? "true" : "false") << ")");
}

void RhythmPatternExplorerAudioProcessor::syncBPMWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // ALWAYS sync BPM - this should never be disabled
    if (posInfo.bpm > 0)
    {
        float hostBPM = static_cast<float>(posInfo.bpm);
        float currentBPMValue = currentBPM;
        
        // BITWIG BPM SYNC DEBUG: Log BPM sync attempts  
        if (hostBPM >= 200.0f) {
            logDebug(DebugCategory::BPM_SYNC, 
                "BPM SYNC: hostBPM=" + juce::String(hostBPM, 1) + 
                ", currentBPM=" + juce::String(currentBPMValue, 1) + 
                ", diff=" + juce::String(std::abs(currentBPMValue - hostBPM), 3));
        }
        
        // Update our internal BPM to match host
        if (std::abs(currentBPMValue - hostBPM) > 0.1f)
        {
            currentBPM = hostBPM;
            updateTiming();
            
            // Log successful BPM updates
            if (hostBPM >= 200.0f) {
                logDebug(DebugCategory::BPM_SYNC, 
                    "BPM UPDATED: " + juce::String(currentBPMValue, 1) + "->" + 
                    juce::String(hostBPM, 1) + ", newSamplesPerStep=" + juce::String(samplesPerStep));
            }
        }
    }
}

void RhythmPatternExplorerAudioProcessor::syncPositionWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // Calculate pattern position based on host timeline
    if (posInfo.ppqPosition >= 0.0)
    {
        // Calculate which step we should be on based on timeline position
        // For tresillo: 8 steps over 4 beats = 0.5 beats per step
        double beatsPerStep = 4.0 / patternEngine.getStepCount(); // Dynamic based on pattern length
        double currentBeat = posInfo.ppqPosition;
        double stepsFromStart = currentBeat / beatsPerStep;
        
        int targetStep = static_cast<int>(stepsFromStart) % patternEngine.getStepCount();
        
        // CRITICAL FIX: Re-enable position sync for perfect DAW timing alignment
        // Use internal currentBPM variable
        bool allowPositionSync = true;  // ENABLED: Position sync needed for tick 1 precision
        
        // CRITICAL FIX: Only sync position when significantly out of sync (not every block)
        int stepDifference = std::abs(targetStep - currentStep.load());
        if (allowPositionSync && stepDifference > 2)  // Increased threshold from 1 to 2
        {
            currentStep.store(targetStep);
            currentSample = static_cast<int>((stepsFromStart - std::floor(stepsFromStart)) * samplesPerStep);
            DBG("RhythmPatternExplorer: Synced to host position - Step: " << targetStep << ", Sample: " << currentSample);
        }
        else if (!allowPositionSync && currentBPM >= 180.0f)
        {
            // Log that we're skipping position sync at high BPM
            logDebug(DebugCategory::POSITION_SYNC, 
                "POSITION SYNC DISABLED: BPM=" + juce::String(currentBPM, 1) + 
                ", targetStep=" + juce::String(targetStep) + 
                ", currentStep=" + juce::String(currentStep.load()));
        }
    }
    
    hostIsPlaying = posInfo.isPlaying;
    lastHostPosition = posInfo.ppqPosition;
}

void RhythmPatternExplorerAudioProcessor::setUPIInput(const juce::String& upiPattern)
{
    juce::ScopedLock lock(processingLock);
    
    // Reset global accent position when setting new UPI input
    globalAccentPosition = 0;
    uiAccentOffset = 0;
    
    DBG("RhythmPatternExplorerAudioProcessor::setUPIInput called with: '" << upiPattern << "'");
    
    // Check for progressive syntax: scenes first (pattern|pattern|pattern), then pattern+N (offset), pattern*N (lengthening)
    juce::String pattern = upiPattern.trim();
    bool isScenes = pattern.contains("|");
    
    // Check if + is followed by a number (progressive offset) vs pattern (combination)
    bool isProgressiveOffset = false;
    if (!isScenes && pattern.contains("+") && pattern.lastIndexOf("+") > 0) {
        int lastPlusIndex = pattern.lastIndexOf("+");
        juce::String afterPlus = pattern.substring(lastPlusIndex + 1).trim();
        // Progressive offset if what follows + is purely numeric
        isProgressiveOffset = afterPlus.containsOnly("0123456789");
    }
    
    bool isProgressiveLengthening = !isScenes && pattern.contains("*") && pattern.lastIndexOf("*") > 0;
    
    if (isScenes)
    {
        // Handle scene cycling: pattern1|pattern2|pattern3
        // CRITICAL: Split scenes BEFORE any UPI parsing to preserve individual accent patterns
        auto scenes = juce::StringArray::fromTokens(pattern, "|", "");
        
        // Log scene cycling detection
        juce::String sceneList;
        for (int i = 0; i < scenes.size(); ++i) {
            sceneList += juce::String::formatted("  Scene %d: %s\n", i, scenes[i].trim().toRawUTF8());
        }
        logDebug(DebugCategory::SCENE_CYCLING, juce::String::formatted("Scene cycling detected: %d scenes\n%s", static_cast<int>(scenes.size()), sceneList.toRawUTF8()));
        
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
            
            // Debug log advancement
            logDebug(DebugCategory::SCENE_CYCLING, 
                "Advanced to scene " + juce::String(currentSceneIndex) + ": " + scenePatterns[currentSceneIndex]);
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
            
            // Debug log reset
            juce::String scenesInfo = "New scene sequence - parsed " + juce::String(scenePatterns.size()) + " scenes, starting with scene 0: " + scenePatterns[0];
            for (int i = 0; i < scenePatterns.size(); ++i) {
                scenesInfo += "\n  Scene " + juce::String(i) + ": " + scenePatterns[i] + " (base: " + sceneBasePatterns[i] + ", step: " + juce::String(sceneProgressiveSteps[i]) + ")";
            }
            logDebug(DebugCategory::SCENE_CYCLING, scenesInfo);
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
        logDebug(DebugCategory::PROGRESSIVE_OFFSET, 
            "Progressive offset detected: " + pattern + " (base: " + newBasePattern + ", step: +" + juce::String(newStep) + ")");
        
        // If same base pattern, advance offset; if different, reset
        if (basePattern == newBasePattern && progressiveStep == newStep)
        {
            advanceProgressiveOffset();
            
            // Notify UI that pattern has changed for accent map updates
            patternChanged.store(true);
            
            // Debug log advancement
            logDebug(DebugCategory::PROGRESSIVE_OFFSET, 
                "Advanced offset to: " + juce::String(progressiveOffset));
        }
        else
        {
            // New progressive pattern - reset and start
            basePattern = newBasePattern;
            progressiveStep = newStep;
            progressiveOffset = newStep; // Start with first offset
            
            // Debug log reset
            logDebug(DebugCategory::PROGRESSIVE_OFFSET, 
                "New progressive pattern - reset offset to: " + juce::String(progressiveOffset));
        }
        
        // Parse the base pattern first, then apply progressive offset via rotation
        parseAndApplyUPI(basePattern);
        
        // Apply progressive offset by rotating the generated pattern
        if (progressiveOffset != 0)
        {
            auto currentPattern = patternEngine.getCurrentPattern();
            auto rotatedPattern = rotatePatternBySteps(currentPattern, progressiveOffset);
            patternEngine.setPattern(rotatedPattern);
            
            // Debug log rotation
            logDebug(DebugCategory::PROGRESSIVE_OFFSET, 
                "Applied rotation: offset=" + juce::String(progressiveOffset));
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
        logDebug(DebugCategory::PROGRESSIVE_LENGTHENING, 
            "Progressive lengthening detected: " + pattern + " (base: " + newBasePattern + ", add: *" + juce::String(newLengthening) + ")");
        
        // If same base pattern, advance lengthening; if different, reset
        if (basePattern == newBasePattern && progressiveLengthening == newLengthening)
        {
            advanceProgressiveLengthening();
            
            // Notify UI that pattern has changed for accent map updates
            patternChanged.store(true);
            
            // Debug log advancement
            logDebug(DebugCategory::PROGRESSIVE_LENGTHENING, 
                "Advanced lengthening - pattern now has " + juce::String(static_cast<int>(baseLengthPattern.size())) + " steps");
        }
        else
        {
            // New progressive lengthening pattern - reset and start
            basePattern = newBasePattern;
            progressiveLengthening = newLengthening;
            
            // Parse base pattern and store for lengthening
            parseAndApplyUPI(basePattern);
            baseLengthPattern = patternEngine.getCurrentPattern();
            
            // Debug log reset
            logDebug(DebugCategory::PROGRESSIVE_LENGTHENING, 
                "New progressive lengthening - starting with " + juce::String(static_cast<int>(baseLengthPattern.size())) + " steps");
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
        parseAndApplyUPI(pattern, true);
        currentStep.store(0); // Reset step indicator to beginning
    }
    
    currentUPIInput = upiPattern;
}

void RhythmPatternExplorerAudioProcessor::parseAndApplyUPI(const juce::String& upiPattern, bool resetAccentPosition)
{
    if (upiPattern.isEmpty())
        return;
    
    // Only reset global accent position for truly new patterns
    if (resetAccentPosition) {
        globalAccentPosition = 0;
        uiAccentOffset = 0;
    }
    
    DBG("parseAndApplyUPI called with: '" + upiPattern + "'");
    
    // For progressive patterns, always re-parse to advance state
    bool isProgressive = upiPattern.contains("#");
    DBG("   Is progressive: " + juce::String(isProgressive ? "YES" : "NO"));
    
    // Check if this is a progressive transformation with ">" syntax
    bool isProgressiveTransformation = upiPattern.contains(">");
    if (isProgressiveTransformation) {
        // Store the pattern key for step tracking
        currentProgressivePatternKey = upiPattern;
    }
    
    auto parseResult = UPIParser::parse(upiPattern);
    
    if (parseResult.isValid())
    {
        DBG("Parse successful in processor");
        DBG("   Pattern: " + UPIParser::patternToBinary(parseResult.pattern));
        DBG("   Name: " + parseResult.patternName);
        
        // Apply the parsed pattern to the engine
        patternEngine.setPattern(parseResult.pattern);
        
        // Set up progressive offset if present
        if (parseResult.hasProgressiveOffset)
        {
            patternEngine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
            DBG("   Progressive offset enabled: initial=" << parseResult.initialOffset 
                << ", progressive=" << parseResult.progressiveOffset);
                
            // Store progressive pattern key for step tracking
            if (!parseResult.progressivePatternKey.isEmpty())
            {
                currentProgressivePatternKey = parseResult.progressivePatternKey;
                DBG("   Progressive pattern key stored: " << currentProgressivePatternKey);
            }
        }
        else
        {
            patternEngine.setProgressiveOffset(false);
            currentProgressivePatternKey.clear(); // Clear for non-progressive patterns
        }
        
        // Set up accent pattern if present
        if (parseResult.hasAccentPattern)
        {
            hasAccentPattern = true;
            currentAccentPattern = parseResult.accentPattern;
            currentAccentPatternName = parseResult.accentPatternName;
            DBG("   Accent pattern enabled: " << currentAccentPatternName << 
                " (" << UPIParser::patternToBinary(currentAccentPattern) << ")");
        }
        else
        {
            hasAccentPattern = false;
            currentAccentPattern.clear();
            currentAccentPatternName.clear();
        }
        
        // Update parameters to reflect the new pattern
        int onsets = UPIParser::countOnsets(parseResult.pattern);
        int steps = static_cast<int>(parseResult.pattern.size());
        
        DBG("   Onsets: " + juce::String(onsets) + ", Steps: " + juce::String(steps));
        
        // Pattern applied successfully via UPI
        
        updateTiming();
        
        // Only cache non-progressive patterns to prevent re-parsing
        if (!isProgressive)
        {
            lastParsedUPI = upiPattern;
        }
        
        DBG("RhythmPatternExplorer: Parsed UPI pattern '" << upiPattern << "' -> " 
            << onsets << " onsets in " << steps << " steps");
        DBG("Binary: " << UPIParser::patternToBinary(parseResult.pattern));
    }
    else
    {
        DBG("RhythmPatternExplorer: Failed to parse UPI pattern '" << upiPattern 
            << "' - " << parseResult.errorMessage);
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
                DBG("RhythmPatternExplorer: Captured MIDI note " << noteNumber << " for output");
            }
            
            // Any MIDI note input triggers pattern regeneration for progressive/random patterns
            if (!currentUPIInput.isEmpty())
            {
                // Check for progressive patterns and scenes
                bool hasProgressiveOffset = currentUPIInput.contains("+") && currentUPIInput.lastIndexOf("+") > 0;
                bool hasProgressiveLengthening = currentUPIInput.contains("*") && currentUPIInput.lastIndexOf("*") > 0;
                bool hasOldProgressiveOffset = currentUPIInput.contains("#");
                bool hasProgressiveTransformation = currentUPIInput.contains(">"); // New progressive syntax
                bool hasScenes = currentUPIInput.contains("|");
                
                // MIDI triggers manual advancement - handle progressive transformations and scenes together when both present
                bool triggerNeeded = false;
                
                if (hasProgressiveTransformation && !hasScenes)
                {
                    DBG("RhythmPatternExplorer: MIDI triggered progressive transformation (no scenes)");
                    // Trigger progressive transformation manually with accent reset
                    parseAndApplyUPI(currentUPIInput, true);
                    triggerNeeded = true;
                }
                else if (hasProgressiveOffset || hasProgressiveLengthening || hasOldProgressiveOffset)
                {
                    DBG("RhythmPatternExplorer: MIDI triggered progressive pattern advancement");
                    if (hasProgressiveOffset || hasOldProgressiveOffset) {
                        if (patternEngine.hasProgressiveOffsetEnabled()) {
                            patternEngine.triggerProgressiveOffset();
                        }
                    }
                    if (hasProgressiveLengthening) {
                        advanceProgressiveLengthening();
                        patternEngine.setPattern(baseLengthPattern);
                    }
                    parseAndApplyUPI(currentUPIInput, true);
                    triggerNeeded = true;
                }
                
                // Handle scenes - can occur together with progressive transformations
                if (hasScenes)
                {
                    DBG("RhythmPatternExplorer: MIDI triggered scene advancement");
                    advanceScene();
                    applyCurrentScenePattern();
                    triggerNeeded = true;
                }
                
                if (!triggerNeeded)
                {
                    // Re-parse other UPI patterns to trigger regeneration with accent reset
                    parseAndApplyUPI(currentUPIInput, true);
                }
                
                // Reset step indicator and notify UI for any trigger
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
                    DBG("RhythmPatternExplorer: CC input ignored for progressive transformation - cycle boundary advancement only");
                    // Don't re-parse - progressive transformations advance at cycle boundaries
                }
                else if (hasProgressiveOffset || hasProgressiveLengthening || hasOldProgressiveOffset || hasScenes)
                {
                    DBG("RhythmPatternExplorer: CC input ignored for progressive/scene patterns - cycle boundary advancement only");
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

std::vector<bool> RhythmPatternExplorerAudioProcessor::rotatePatternBySteps(const std::vector<bool>& pattern, int steps)
{
    if (pattern.empty()) 
        return pattern;
    
    std::vector<bool> rotated(pattern.size());
    int size = static_cast<int>(pattern.size());
    
    // Normalize steps to be within pattern size
    steps = steps % size;
    if (steps < 0) steps += size;
    
    for (int i = 0; i < size; ++i)
    {
        int newIndex = (i + steps) % size;
        rotated[newIndex] = pattern[i];
    }
    
    return rotated;
}

void RhythmPatternExplorerAudioProcessor::advanceProgressiveLengthening()
{
    if (progressiveLengthening > 0 && !baseLengthPattern.empty())
    {
        // Generate random steps using bell curve distribution
        auto randomSteps = generateBellCurveRandomSteps(progressiveLengthening);
        
        // Append the random steps to the pattern
        baseLengthPattern.insert(baseLengthPattern.end(), randomSteps.begin(), randomSteps.end());
        
        logDebug(DebugCategory::PROGRESSIVE_LENGTHENING, 
            "Added " + juce::String(progressiveLengthening) + " random steps, total length now: " + 
            juce::String(static_cast<int>(baseLengthPattern.size())));
    }
}

std::vector<bool> RhythmPatternExplorerAudioProcessor::generateBellCurveRandomSteps(int numSteps)
{
    std::vector<bool> randomSteps(numSteps, false);
    
    if (numSteps <= 0) return randomSteps;
    
    // Use bell curve distribution to determine number of onsets (avoid extremes)
    std::normal_distribution<double> distribution(numSteps / 2.0, (numSteps - 1) / 6.0);
    int onsets = static_cast<int>(std::round(distribution(randomGenerator)));
    
    // Clamp to valid range [1, numSteps-1] to avoid empty or full patterns
    onsets = juce::jmax(1, juce::jmin(numSteps - 1, onsets));
    
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
    if (!scenePatterns.isEmpty() && currentSceneIndex < static_cast<int>(sceneProgressiveSteps.size()))
    {
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
        
        juce::String sceneInfo = "Advanced to scene " + juce::String(currentSceneIndex) + "/" + 
            juce::String(static_cast<int>(scenePatterns.size())) + ": " + scenePatterns[currentSceneIndex];
        if (currentSceneIndex < static_cast<int>(sceneProgressiveSteps.size())) {
            sceneInfo += "\n  Scene progressive state - offset: " + juce::String(sceneProgressiveOffsets[currentSceneIndex]) + 
                        ", lengthening: " + juce::String(sceneProgressiveLengthening[currentSceneIndex]);
        }
        logDebug(DebugCategory::SCENE_CYCLING, sceneInfo);
    }
}

void RhythmPatternExplorerAudioProcessor::applyCurrentScenePattern()
{
    // Apply the current scene pattern using per-scene progressive state
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
        auto rotatedPattern = rotatePatternBySteps(currentPattern, progressiveOffset);
        patternEngine.setPattern(rotatedPattern);
        
        logDebug(DebugCategory::SCENE_CYCLING, 
            "Applied scene " + juce::String(currentSceneIndex) + " progressive offset: " + juce::String(progressiveOffset));
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
        
        logDebug(DebugCategory::SCENE_CYCLING, 
            "Applied scene " + juce::String(currentSceneIndex) + " progressive lengthening: " + juce::String(progressiveLengthening) + " steps");
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
// Accent Map Generation

std::vector<bool> RhythmPatternExplorerAudioProcessor::getCurrentAccentMap() const
{
    // Show stable accent map using UI accent offset (updates only at cycle boundaries)
    auto pattern = patternEngine.getCurrentPattern();
    std::vector<bool> accentMap(pattern.size(), false);

    if (!hasAccentPattern || currentAccentPattern.empty()) {
        return accentMap;
    }

    // Apply accents starting from stable UI accent offset
    // This provides stable display that only changes at cycle boundaries
    int accentLen = static_cast<int>(currentAccentPattern.size());
    int accentCounter = 0;
    
    for (size_t i = 0; i < pattern.size(); ++i)
    {
        if (pattern[i])
        {
            // Calculate accent position based on stable UI accent offset
            int accentStep = (uiAccentOffset + accentCounter) % accentLen;
            accentMap[i] = currentAccentPattern[accentStep];
            accentCounter++;
        }
    }
    
    return accentMap;
}

//==============================================================================
// Centralized Debug Logging Utility

void RhythmPatternExplorerAudioProcessor::logDebug(DebugCategory category, const juce::String& message) {
    // Debug logging disabled for production performance
    juce::ignoreUnused(category, message);
}

const char* RhythmPatternExplorerAudioProcessor::getCategoryName(DebugCategory category) {
    switch (category) {
        case DebugCategory::BITWIG_INIT: return "BITWIG_INIT";
        case DebugCategory::BITWIG_PROCESS: return "BITWIG_PROCESS";
        case DebugCategory::TRANSPORT: return "TRANSPORT";
        case DebugCategory::BPM_SYNC: return "BPM_SYNC";
        case DebugCategory::STEP_TRIGGER: return "STEP_TRIGGER";
        case DebugCategory::POSITION_SYNC: return "POSITION_SYNC";
        case DebugCategory::SCENE_CYCLING: return "SCENE_CYCLING";
        case DebugCategory::PROGRESSIVE_OFFSET: return "PROGRESSIVE_OFFSET";
        case DebugCategory::PROGRESSIVE_LENGTHENING: return "PROGRESSIVE_LENGTHENING";
        default: return "UNKNOWN";
    }
}

const char* RhythmPatternExplorerAudioProcessor::getLogFile(DebugCategory category) {
    switch (category) {
        case DebugCategory::SCENE_CYCLING:
        case DebugCategory::PROGRESSIVE_OFFSET:
        case DebugCategory::PROGRESSIVE_LENGTHENING:
            return "/tmp/rhythm_progressive_debug.log";
        default:
            return "/tmp/bitwig_debug.log";
    }
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
    // Convert subdivision choice to beat fractions
    // Subdivision choices: {"64th Triplet", "64th", "32nd Triplet", "32nd", "16th Triplet", "16th", "8th Triplet", "8th", "Quarter Triplet", "Quarter", "Half Triplet", "Half", "Whole"}
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
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RhythmPatternExplorerAudioProcessor();
}